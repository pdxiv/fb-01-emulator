#include "fb01.h"
#include "scheduler.h"
#include "i8251.h"
#include "hd44780.h"

namespace FB01 {

FB01Emulator::FB01Emulator()
    : m_rom(32768, 0xFF)
    , m_ram(16384, 0x00)
    , m_buttons(0xFF)  // all buttons released (active-low)
    , m_rom_loaded(false)
{
    m_z80 = new z80();
    m_opm = new opm_t();
    m_usart = new I8251();
    m_lcd = new HD44780();
    m_scheduler = new Scheduler(*this);

    z80_init(m_z80);
    m_z80->read_byte = z80ReadByte;
    m_z80->write_byte = z80WriteByte;
    m_z80->port_in = z80PortIn;
    m_z80->port_out = z80PortOut;
    m_z80->userdata = this;

    OPM_Reset(m_opm, opm_flags_ym2164);
}

FB01Emulator::~FB01Emulator() {
    delete m_z80;
    delete m_opm;
    delete m_usart;
    delete m_lcd;
    delete m_scheduler;
}

bool FB01Emulator::loadROM(const uint8_t* data, size_t size) {
    if (size != 32768)
        return false;
    std::copy(data, data + size, m_rom.begin());
    m_rom_loaded = true;
    return true;
}

void FB01Emulator::reset() {
    std::fill(m_ram.begin(), m_ram.end(), 0x00);
    z80_init(m_z80);
    m_z80->read_byte = z80ReadByte;
    m_z80->write_byte = z80WriteByte;
    m_z80->port_in = z80PortIn;
    m_z80->port_out = z80PortOut;
    m_z80->userdata = this;

    OPM_Reset(m_opm, opm_flags_ym2164);
    m_usart->reset();
    m_lcd->reset();
    m_scheduler->reset();
    m_buttons = 0xFF;
}

void FB01Emulator::sendMidiByte(uint8_t byte) {
    m_usart->receiveData(byte);
    updateInterrupts();
}

bool FB01Emulator::receiveMidiByte(uint8_t& byte) {
    return m_usart->transmitData(byte);
}

void FB01Emulator::setButtonState(uint8_t buttons) {
    m_buttons = buttons;
}

const char* FB01Emulator::getLCDText() const {
    return m_lcd->getDisplayText();
}

const uint8_t* FB01Emulator::getCGRAM() const {
    return m_lcd->getCGRAM();
}

void FB01Emulator::generateSample(float& outL, float& outR, double sampleRate) {
    m_scheduler->advanceOneSample(outL, outR, sampleRate);
}

// Z80 memory read: 0x0000-0x7FFF = ROM, 0x8000-0xFFFF = RAM (16KB, mirrored)
uint8_t FB01Emulator::z80ReadByte(void* userdata, uint16_t addr) {
    auto* self = static_cast<FB01Emulator*>(userdata);
    if (addr < 0x8000) {
        return self->m_rom[addr];
    } else {
        return self->m_ram[addr & 0x3FFF];
    }
}

void FB01Emulator::z80WriteByte(void* userdata, uint16_t addr, uint8_t val) {
    auto* self = static_cast<FB01Emulator*>(userdata);
    if (addr >= 0x8000) {
        self->m_ram[addr & 0x3FFF] = val;
    }
    // Writes to ROM space are ignored
}

uint8_t FB01Emulator::z80PortIn(struct z80* z, uint8_t port) {
    auto* self = static_cast<FB01Emulator*>(z->userdata);
    uint8_t masked = port & 0xFF;

    switch (masked & 0xF0) {
        case 0x00: // OPM
            if (masked & 0x01) {
                return OPM_Read(self->m_opm, 1); // Status register
            }
            return 0xFF; // Address port is write-only
        case 0x10: // USART
            if (masked & 0x01) {
                return self->m_usart->readStatus();
            } else {
                uint8_t val = self->m_usart->readData();
                self->updateInterrupts();
                return val;
            }
        case 0x20: // Panel switches
            return self->m_buttons;
        case 0x30: // LCD
            return self->m_lcd->read(masked & 0x01);
        default:
            return 0xFF; // Unmapped
    }
}

void FB01Emulator::z80PortOut(struct z80* z, uint8_t port, uint8_t val) {
    auto* self = static_cast<FB01Emulator*>(z->userdata);
    uint8_t masked = port & 0xFF;

    switch (masked & 0xF0) {
        case 0x00: // OPM
            OPM_Write(self->m_opm, masked & 0x01, val);
            break;
        case 0x10: // USART
            if (masked & 0x01) {
                self->m_usart->writeControl(val);
            } else {
                self->m_usart->writeData(val);
            }
            self->updateInterrupts();
            break;
        case 0x30: // LCD
            self->m_lcd->write(masked & 0x01, val);
            break;
        default:
            break; // Unmapped
    }
}

void FB01Emulator::updateInterrupts() {
    // OR-combined: OPM IRQ, USART TXRDY, USART RXRDY
    bool opm_irq = OPM_ReadIRQ(m_opm) != 0;
    bool usart_irq = m_usart->getTxReady() || m_usart->getRxReady();

    if (opm_irq || usart_irq) {
        z80_gen_int(m_z80, 0xFF); // IM1 mode uses RST 38h vector
    }
}

} // namespace FB01
