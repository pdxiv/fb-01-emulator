# Yamaha FB-01 Hardware Specifications

## CPU

- **Processor:** Zilog Z80
- **Crystal:** 12 MHz (XTAL), divided by 2 → **6 MHz CPU clock**

## Memory Address Map (Z80 address space)

| Address Range | Size | Description |
|---|---|---|
| `0x0000`–`0x7FFF` | 32 KB | ROM (firmware, NEC D27C256C-15 EPROM) |
| `0x8000`–`0xBFFF` | 16 KB | SRAM (battery-backed NVRAM, 2 × 8 KB) |
| `0xC000`–`0xFFFF` | 16 KB | Mirror of SRAM (`0x8000`–`0xBFFF`) |

## I/O Address Map (8-bit I/O space, partial decoding via upper nibble A4–A7)

Only bits A4–A7 select the device; bits A0–A3 select sub-registers within each device.
Unmapped I/O addresses read as `0xFF`.

| Port | R/W | Device | Function |
|---|---|---|---|
| `0x00` | W | YM2164 (OPM) | Address write |
| `0x01` | R | YM2164 (OPM) | Status read |
| `0x01` | W | YM2164 (OPM) | Data write |
| `0x10` | R/W | µPD71051C (i8251-compatible USART) | Data register |
| `0x11` | R/W | µPD71051C (i8251-compatible USART) | Control/status register |
| `0x20` | R | Panel switches | 8-bit active-low button input |
| `0x30` | R/W | HD44780A LCD controller | Instruction/data (RS=0) |
| `0x31` | R/W | HD44780A LCD controller | Instruction/data (RS=1) |

## Sound Chip

- **Chip:** Yamaha YM2164 (OPM variant)
- **Clock:** 4 MHz (dedicated XTAL)
- **Output:** Stereo (Left + Right routed to a 2-channel speaker at full volume)
- **IRQ:** Connected to Z80 INT line (active high, OR-combined with USART signals)

## MIDI / Serial Interface

- **USART:** NEC µPD71051C (i8251-compatible)
- **USART base clock:** 4 MHz XTAL
- **Baud rate clock:** 4 MHz ÷ 8 = **500 kHz** (TX and RX clock)
- **MIDI ports:** MIDI IN, MIDI OUT, MIDI THRU (THRU is directly wired from IN)
- **Standard:** 31.25 kbaud MIDI (500 kHz ÷ 16 = 31,250 baud, typical i8251 ÷16 mode)

## Interrupts

- **Mode:** IM1 (Interrupt Mode 1) — Z80 jumps to `0x0038` on INT assertion
- **Z80 INT (active-low accent):** Directly asserted when *any* of these signals are active:
  - YM2164 IRQ (timer interrupt from the sound chip)
  - µPD71051 TXRDY (transmit buffer ready)
  - µPD71051 RXRDY (receive data ready)
- These are OR-combined in hardware (active-high into Z80 INT); no dedicated interrupt controller.

## Display

- **LCD Controller:** Hitachi HD44780A (clock ~270 kHz, typical from datasheet)
- **LCD Size:** 2×8 characters, displayed as a single 1×16 line
- **LCD Palette:** 2 colors (dark red background `rgb(30,0,0)`, bright red foreground `rgb(150,0,0)`)
- **Refresh:** 50 Hz

## Front Panel Buttons (active-low, port `0x20`)

| Bit | Button |
|---|---|
| 0 | System Set Up |
| 1 | Inst Select |
| 2 | Inst Assign |
| 3 | Inst Function |
| 4 | Voice Function |
| 5 | Voice Select |
| 6 | −1 / No |
| 7 | +1 / Yes |

## ROM

- **Size:** 32 KB (27C256 one-time-programmable EPROM)
- **Label:** `NEC -011 XB712C0 8709EX700 D27C256C-15`
- **CRC32:** `0x7357E9A4`
- **SHA1:** `049c482d6c91b7e2846757dd0f5138e0d8b687f0`

## General

- **Year of manufacture:** 1986
- **Manufacturer:** Yamaha
- **Full name:** FB-01 FM Sound Generator
- **Architecture summary:** Simple Z80 system with no banking — 32 KB ROM + 16 KB SRAM (mirrored), four peripherals on the I/O bus (OPM, USART, panel switches, LCD).
