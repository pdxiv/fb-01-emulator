# Intel 8251 (µPD71051) USART — Hardware Specification

## Overview

The i8251 is a Universal Synchronous/Asynchronous Receiver/Transmitter (USART). It provides a full-duplex serial interface with programmable baud rate division, character length, parity, and stop bits.

## Bus Interface

### Address Decoding

The i8251 uses a single address bit (offset bit 0, directly mapped to the **C/D̄** pin) to select between two register ports:

| Offset (A0) | C/D̄ | Read | Write |
|---|---|---|---|
| 0 | 0 | Data Register (received data) | Data Register (transmit data) |
| 1 | 1 | Status Register | Control Register (mode or command) |

In the FB-01, these are mapped to Z80 I/O ports `0x10` (data) and `0x11` (control/status).

### Control Pins

| Pin | Direction | Function |
|---|---|---|
| D7–D0 | Bidirectional | 8-bit data bus |
| C/D̄ | Input | Control/Data select (directly from A0) |
| RD̄ | Input | Read strobe |
| WR̄ | Input | Write strobe |
| CS̄ | Input | Chip select (directly from address decode logic) |
| CLK | Input | System clock |
| TxC | Input | Transmitter clock |
| RxC | Input | Receiver clock |
| TxD | Output | Transmit data (serial out) |
| RxD | Input | Receive data (serial in) |
| RTS̄ | Output | Request to Send (active low) |
| DTR̄ | Output | Data Terminal Ready (active low) |
| CTS̄ | Input | Clear to Send (active low) |
| DSR̄ | Input | Data Set Ready (active low) |
| TXRDY | Output | Transmit buffer ready (active high) |
| RXRDY | Output | Receive data ready (active high) |
| TxEMPTY | Output | Transmit shift register empty |
| SYNDET | I/O | Sync detect (output in internal sync, input in external sync) |

## Reset State

After reset:

- TxD output = 1 (marking state).
- RTS̄ output = 1 (inactive).
- DTR̄ output = 1 (inactive).
- Status = `0x05` (TX_EMPTY | TX_READY).
- Next write to control port expects a **mode byte**.
- Baud rate factor = 1 (×1).

## Programming Sequence

After reset, the control port accepts writes in this order:

1. **Mode Byte** — configures async/sync, character format, baud rate factor.
2. **Sync Byte(s)** — (synchronous mode only) 1 or 2 sync characters.
3. **Command Byte** — enables/disables TX/RX, controls modem signals.

After the initial mode + command sequence, subsequent writes to the control port are interpreted as **command bytes**. Writing a command with bit 6 set (Internal Reset) returns the device to expecting a new mode byte.

## Mode Byte Format

### Asynchronous Mode (bits 1:0 ≠ 0)

| Bit(s) | Field | Values |
|---|---|---|
| 7:6 | Stop Bits | `00` = inhibit, `01` = 1 bit, `10` = 1.5 bits, `11` = 2 bits |
| 5 | Parity Type | `0` = odd, `1` = even |
| 4 | Parity Enable | `0` = disabled, `1` = enabled |
| 3:2 | Character Length | `00` = 5, `01` = 6, `10` = 7, `11` = 8 bits |
| 1:0 | Baud Rate Factor | `01` = ×1, `10` = ×16, `11` = ×64 |

### Synchronous Mode (bits 1:0 = 0)

| Bit(s) | Field | Values |
|---|---|---|
| 7 | Sync Char Count | `0` = 1 sync character, `1` = 2 sync characters |
| 6 | Sync Mode | `0` = internal sync (SYNDET is output), `1` = external sync (SYNDET is input) |
| 5 | Parity Type | `0` = odd, `1` = even |
| 4 | Parity Enable | `0` = disabled, `1` = enabled |
| 3:2 | Character Length | `00` = 5, `01` = 6, `10` = 7, `11` = 8 bits |
| 1:0 | Must be `00` | Selects synchronous mode |

## Command Byte Format

Written to the control port (offset 1) after the mode byte has been accepted:

| Bit | Field | `0` | `1` |
|---|---|---|---|
| 0 | TxEN | Transmit disable | Transmit enable |
| 1 | DTR | DTR̄ pin = 1 (inactive) | DTR̄ pin = 0 (active) |
| 2 | RxEN | Receive disable | Receive enable |
| 3 | SBRK | Normal operation | Send break (force TxD low) |
| 4 | ER | Normal operation | Reset error flags (PE, OE, FE) |
| 5 | RTS | RTS̄ pin = 1 (inactive) | RTS̄ pin = 0 (active) |
| 6 | IR | Normal operation | Internal Reset (returns to mode format) |
| 7 | EH | Normal operation | Enter Hunt Mode (sync only) |

## Status Register Format

Read from the control port (offset 1):

| Bit | Flag | Meaning |
|---|---|---|
| 0 | TXRDY | Transmit buffer ready (DB buffer empty, CTS active, TxEN set) |
| 1 | RXRDY | Receive data ready (data available & RxEN set) |
| 2 | TXEMPTY | Transmit shift register empty |
| 3 | PE | Parity Error |
| 4 | OE | Overrun Error (new char received before previous was read) |
| 5 | FE | Framing Error (async: invalid stop bit) |
| 6 | SYNDET | Sync detect (sync mode only; cleared on status read) |
| 7 | DSR | Data Set Ready (inverted DSR̄ pin state) |

## TXRDY / RXRDY Output Pin Logic

These pins drive interrupt logic (directly to the Z80 INT in the FB-01):

- **TXRDY pin** = `1` when: TX enabled (`TxEN=1`) AND CTS̄ = 0 AND status TX_READY bit is set.
- **RXRDY pin** = `1` when: RX enabled (`RxEN=1`) AND status RX_READY bit is set.

Note: Reading the data port clears RX_READY. Writing the data port clears TX_READY.

## Baud Rate Generation

The i8251 does not contain an internal baud rate generator. External clocks (TxC, RxC) must be provided.

- **×1 mode:** Clock frequency = baud rate (one bit per clock edge).
- **×16 mode:** Clock frequency = 16 × baud rate (samples at midpoint of bit).
- **×64 mode:** Clock frequency = 64 × baud rate.

### FB-01 Configuration

- TxC and RxC both driven by a 500 kHz clock (4 MHz ÷ 8).
- With ×16 mode: 500,000 ÷ 16 = **31,250 baud** (standard MIDI baud rate).

## Clock Edge Sensitivity

- **RxC:** Data sampled on the **rising edge**.
- **TxC:** Data shifted out on the **falling edge**.

## Transmit Flow

1. CPU writes data to port 0 → TX_READY clears, data stored in TX buffer.
2. When TX shift register is empty and TxEN + CTS̄ active → data moves from buffer to shift register, TX_READY re-asserts, TX_EMPTY clears.
3. Data is shifted out bit-by-bit on TxC falling edges.
4. When shift register empties → TX_EMPTY asserts.
5. In sync mode, sync characters are automatically transmitted during idle.

## Receive Flow

1. Data bits are shifted in on RxC rising edges.
2. In ×16/×64 modes, the receiver synchronizes to the start bit and samples at the midpoint (1.5× bit periods from start bit edge).
3. When a full character is assembled → RX_READY asserts, RXRDY pin goes high.
4. CPU reads data from port 0 → RX_READY clears.
5. If a new character arrives before the previous is read → Overrun Error (OE) sets.

## Error Handling

| Error | Condition | Cleared By |
|---|---|---|
| Parity Error (PE) | Received parity doesn't match expected | Command byte with ER bit (bit 4) set |
| Overrun Error (OE) | New data received while RX_READY still set | Command byte with ER bit (bit 4) set |
| Framing Error (FE) | Invalid stop bit detected (async mode) | Command byte with ER bit (bit 4) set |

## Break Detection

When the command byte has bit 3 (SBRK) set, TxD is forced low (space/break condition). 16 consecutive low RxD bits should trigger SYNDET in async mode (BRKDET), but this is not yet emulated.

## Synchronous Mode Details

- After mode byte, 1 or 2 sync characters are written to the control port.
- In Hunt Mode (command bit 7), the receiver searches the incoming stream for the sync pattern.
- Once sync is found, SYNDET asserts and character-aligned reception begins.
- During TX idle, sync characters are automatically transmitted instead of the marking state.
- SYNDET status bit is cleared on any status register read.

## Compatible Devices

| Device | Notes |
|---|---|
| Intel 8251 | Original |
| Intel 8251A | Improved timing |
| NEC µPD71051 / µPD71051C | Pin-compatible clone (used in FB-01) |
| NEC V5x SCU | Variant with split mode/command registers at 4 port addresses |
