# Hitachi HD44780 LCD Controller — Hardware Specification

## Overview

The HD44780 is a dot-matrix LCD controller/driver capable of displaying alphanumeric characters and symbols. It has a simple parallel bus interface suitable for direct connection to a Z80 or similar 8-bit CPU.

## Bus Interface

### Address Decoding (Memory-Mapped or I/O-Mapped)

The HD44780 uses a single address bit (offset bit 0, typically directly mapped to the **RS** pin) to select between two registers:

| Offset (A0) | RS | R/W=0 (Write) | R/W=1 (Read) |
|---|---|---|---|
| 0 | 0 | Instruction Register (IR) — command write | Busy Flag (BF) + Address Counter (AC) read |
| 1 | 1 | Data Register (DR) — data write to RAM | Data read from RAM |

In the FB-01, these are mapped to Z80 I/O ports `0x30` (RS=0) and `0x31` (RS=1).

### Control Signals

| Pin | Direction | Function |
|---|---|---|
| RS | Input | Register Select: 0 = Instruction, 1 = Data |
| R/W | Input | Read/Write: 0 = Write, 1 = Read |
| E | Input | Enable (active-high strobe, data latched on falling edge) |
| DB7–DB0 | Bidirectional | 8-bit data bus (or DB7–DB4 only in 4-bit mode) |

### Bus Timing (directly from E pin)

- Data is **written** on the **falling edge** of E (high→low transition).
- Data is **read** while E is high (directly from internal registers).

### Data Bus Width Modes

- **8-bit mode** (default after reset): All 8 data lines (DB7–DB0) used per transfer.
- **4-bit mode**: Only DB7–DB4 used; each byte requires two E-strobes (high nibble first, then low nibble).

## Internal Clock

- **Oscillator:** Internal RC oscillator, frequency determined by external resistor Rf.
- **Typical Rf:** 91 kΩ → **~270 kHz** clock.
- **Formula (5V):** `f = 1 / (2 × π × Rf × 6.5pF)`
- **Formula (3V):** `f = 1 / (2 × π × Rf × 7.86pF)`
- **Alternate Rf:** 200 kΩ → ~122.4 kHz (used in some Hitachi LCD modules).

## Internal Memory

### DDRAM (Display Data RAM)

- **Size:** 128 bytes (`0x00`–`0x7F`)
- **Purpose:** Stores character codes for display.
- **1-line mode:** 80 usable positions (`0x00`–`0x4F`), wraps around.
- **2-line mode:**
  - Line 1: addresses `0x00`–`0x27` (40 positions)
  - Line 2: addresses `0x40`–`0x67` (40 positions)
- **Reset state:** All filled with `0x20` (space character).

### CGRAM (Character Generator RAM)

- **Size:** 64 bytes (`0x00`–`0x3F`)
- **Purpose:** User-defined character patterns.
- **5×8 font:** 8 custom characters (8 bytes each, addressed as characters `0x00`–`0x07`).
- **5×10 font:** 4 custom characters (16 bytes each, addressed as characters `0x00`–`0x03`, using bit 2:1 for selection).
- **Reset state:** All zeros.

### CGROM (Character Generator ROM)

- **Size:** 4096 bytes (4 KB)
- **Purpose:** Built-in character font bitmaps (192+ predefined characters).
- **Indexing:** Character code × 16 bytes per entry (5 bits wide per row).
- **Variants:** HD44780 A00, HD44780U A00, HD44780U A02, and others.

## Instruction Set

All instructions are written to offset 0 (RS=0, R/W=0). The instruction is decoded by the highest set bit:

| Bit Pattern | Instruction | Busy Time (cycles) |
|---|---|---|
| `0000 0001` | Clear Display | 410 |
| `0000 001x` | Return Home | 410 |
| `0000 01DS` | Entry Mode Set | 10 |
| `0000 1DCB` | Display On/Off Control | 10 |
| `0001 SRxx` | Cursor/Display Shift | 10 |
| `001D NF xx` | Function Set | 10 |
| `01AA AAAA` | Set CGRAM Address | 10 |
| `1AAA AAAA` | Set DDRAM Address | 10 |
| — | Data Write (to either RAM) | 10 |
| — | Data Read (from either RAM) | 10 |

### Instruction Bit Definitions

**Entry Mode Set** (`0000 01DS`):

- D (bit 1): Increment (1) / Decrement (0) address counter after data R/W.
- S (bit 0): Shift entire display (1) / Don't shift (0) on data write.

**Display On/Off Control** (`0000 1DCB`):

- D (bit 2): Display on (1) / off (0).
- C (bit 1): Cursor on (1) / off (0).
- B (bit 0): Blink on (1) / off (0).

**Cursor/Display Shift** (`0001 SRxx`):

- S (bit 3): Display shift (1) / Cursor move (0).
- R (bit 2): Shift right (1) / Shift left (0).

**Function Set** (`001D NF xx`):

- D (bit 4): 8-bit interface (1) / 4-bit interface (0).
- N (bit 3): 2-line display (1) / 1-line display (0).
- F (bit 2): 5×10 font (1) / 5×8 font (0).

**Set CGRAM Address** (`01AA AAAA`):

- A (bits 5–0): 6-bit CGRAM address (`0x00`–`0x3F`).

**Set DDRAM Address** (`1AAA AAAA`):

- A (bits 6–0): 7-bit DDRAM address (valid range depends on line mode).

## Busy Flag & Address Counter Read

Reading offset 0 (RS=0, R/W=1) returns:

| Bit 7 | Bits 6–0 |
|---|---|
| BF (Busy Flag) | AC (Address Counter, 7-bit) |

- **BF=1:** Controller is executing an instruction; do not send new commands.
- **BF=0:** Controller is ready to accept the next instruction.

## Busy Timings (in oscillator clock cycles)

| Operation | Cycles |
|---|---|
| Clear Display | 410 |
| Return Home | 410 |
| Entry Mode Set | 10 |
| Display On/Off | 10 |
| Cursor/Display Shift | 10 |
| Function Set | 10 |
| Set CGRAM Address | 10 |
| Set DDRAM Address | 10 |
| Data Write | 10 |
| Data Read | 10 |
| Blink Inversion Rate | 102,400 (cursor blink period) |

At ~270 kHz clock: 10 cycles ≈ 37 µs, 410 cycles ≈ 1.52 ms.

## Display Shift / Wraparound Behavior

The display window can be shifted left/right through the full DDRAM using the shift instruction or auto-shift mode.

**1-line mode wrap:** `...4E 4F 00 01 02...4D 4E 4F 00 01...`

**2-line mode wrap:**

- Line 1: `...26 27 00 01 02...24 25 26 27 00 01...`
- Line 2: `...66 67 40 41 42...64 65 66 67 40 41...`

## Character Rendering

- **Character cell:** 5 pixels wide × 8 (or 10) pixels tall.
- **Display pitch:** 6 pixels horizontal per character (5 + 1 gap).
- **Line pitch:** `char_height + 1` pixels vertical (e.g. 9 pixels for 5×8 mode).
- **Cursor:** Bottom row of the character cell filled (`0x1F`).
- **Blink:** Entire character cell filled with `0x1F` at the blink rate.

## Reset State

After power-on or reset:

- DDRAM filled with `0x20` (space).
- CGRAM filled with `0x00`.
- Address counter = 0.
- Display off, cursor off, blink off.
- Data length = 8-bit.
- Number of lines = 1.
- Character size = 5×8.
- Direction = increment (+1).
- Display shift = 0.
- Busy flag set for 410 cycles.

## Initialization Sequence (for Z80 interfacing in 8-bit mode)

1. Wait >15 ms after power-on.
2. Write Function Set (`0x38` for 8-bit, 2-line, 5×8): port `0x30` ← `0x38`.
3. Wait for busy flag to clear (poll port `0x30` bit 7, or wait >40 µs).
4. Write Display On/Off (`0x0C` for display on, cursor off, blink off): port `0x30` ← `0x0C`.
5. Wait for busy flag to clear.
6. Write Clear Display (`0x01`): port `0x30` ← `0x01`.
7. Wait for busy flag to clear (~1.52 ms).
8. Write Entry Mode Set (`0x06` for increment, no shift): port `0x30` ← `0x06`.
9. Controller is now ready to accept data writes at port `0x31`.

## Compatible Clones

| Manufacturer | Device | Notes |
|---|---|---|
| Hitachi | HD44780 | Original |
| Hitachi | HD44780U | Updated (minor font differences) |
| Epson | SED1278 | Fixes HD44780 AC timing bug |
| Samsung | KS0066 / KS0066U | Pin/timing compatible |
| Samsung | S6A0069 | Equivalent to KS0066U |
| Sitronix | ST7066U | Compatible |
| Sunplus | SPLC780C | Compatible |

## Known HD44780 Bugs

- **HD44780 AC increment bug:** Busy flag is only active for 10 cycles on data read/write, but the address counter increment/decrement happens on the 11th cycle. Fixed in SED1278 and later clones. The emulation follows the SED1278 (fixed) behavior.
