# Perfboard layout: Neopixel Christmas light (2 x 60 LEDs, 5V 5A)

Board: 70 x 91 mm, perforated area about 61 x 77 mm, 24 columns x 30 rows (2.54 mm pitch, plain board with individual pads).
Diagrams: `perfboard-layout.jpg` (top view, component side) and `perfboard-layout-bottom.jpg` (bottom view, solder side, mirrored left-right so column 1 is at the right; column numbers are labeled the same as the top view). Row 1 = top in both. Holes are written (column,row).

The Nano sits in the middle of the board, USB end at the top. J2 (strip 1) is at the top near the USB end with its wire entry facing up, away from the Nano. J1 (power in) is at the bottom with its wire entry facing down, away from the Nano. J3 (strip 2 power, optional) is next to J2 at the top.

## Parts
| Ref | Part | Holes |
|---|---|---|
| N1 | Arduino Nano in two 15-pin female sockets | left pins col 9, right pins col 15, rows 9-23 (USB end at row 9) |
| J1 | 2-position screw terminal (5.08 mm), wire entry facing down | (11,29) +5V, (13,29) GND |
| F1 | 3 A fuse in an inline holder on the +5V wire BEFORE J1 (not on the board, not a PTC) | off-board |
| C1 | 1000 uF 6.3 V+ electrolytic, next to J1 | (9,27) +, (9,25) - |
| J2 | 3-position screw terminal for strip 1, wire entry facing up | (13,3) DIN, (15,3) 5V, (17,3) GND |
| J3 (optional) | 2-position screw terminal for strip 2 power, wire entry facing up. Leave it out and put strip 2's power wires under J2's 5V and GND screws instead (two wires per screw) | (5,3) GND, (7,3) 5V |
| R1 | 330 ohm resistor, standing vertically | (17,13) and (17,9) |
| P1 | 3-pin header for the speed pot | (6,11) 5V, (6,12) wiper, (6,13) GND |
| SW1 | 2-pin header for the button | (17,19) D2 side, (17,20) GND side |

Use sockets for the Nano so you can remove it. Use header pins or screw terminals for the pot, button and strips so they stay off-board.

## Wires (solder side, insulated; 22 AWG for power, 26-30 AWG for signals)
| Net | Wire |
|---|---|
| +5V power | J1+ (11,29) -> (11,27) -> west along row 27 to (7,27) -> up column 7 to J3 5V (7,3). C1+ sits at (9,27) on this wire |
| J2 5V | tee at (7,5) -> east along row 5 to (15,5) -> up to J2 5V (15,3) |
| Nano 5V | (9,20) -> (8,20) -> (7,20) on the column 7 wire |
| P1 5V | (6,11) -> (7,11) on the column 7 wire |
| GND power | J1 GND (13,29) -> up to (13,25) -> west along row 25 to (5,25) and east to (19,25). C1- sits at (9,25) on this wire |
| Left GND riser | (5,25) -> up column 5 to J3 GND (5,3) |
| P1 GND | (6,13) -> (5,13) on the column 5 wire |
| Right GND riser | (19,25) -> up column 19 to (19,3) -> west to J2 GND (17,3) |
| Nano GND / button | Nano GND (15,20) -> SW1 (17,20) -> (19,20) on the column 19 wire |
| Speed pot wiper | P1 (6,12) -> Nano A0 (9,12) |
| LED data | Nano D6 (15,15) -> (16,15) -> up column 16 to (16,13) -> R1 (17,13); R1 (17,9) -> up to (17,6) -> west along row 6 to (13,6) -> up to J2 DIN (13,3) |
| Button | Nano D2 (15,19) -> SW1 (17,19) |

Off-board: strip 1 DO connects to strip 2 DIN.

Without J3: skip the wires to J3 (5V riser top at (7,3) and GND riser top at (5,3)). Strip 2's 5V and GND wires then go under J2's 5V and GND screws together with strip 1's.

Wires cross at (7,12), (7,25) and (13,5). That is fine with insulated wire. Keep the joints of the different nets apart.

## Why it is wired this way
- The strips' current runs J1 -> rails -> J2/J3 and never through the Nano.
- The button uses the Nano's GND pin right next to D2 (it only carries a tiny current).
- Power wires should be thicker than perfboard pads can carry well. Don't rely on the solder pads for the strip current.
- Both terminals' wire openings face away from the Nano and out toward the board edges, so cables don't cross the Nano.

## Build order
1. Solder the Nano sockets, headers, terminals, R1 and C1 (check the capacitor's polarity).
2. Run the power rails and the Nano power wires first, then the signal wires.
3. Check **before inserting the Nano**: use a meter in continuity mode and confirm the 5V net is **not** connected to the GND net, and every wire above does connect.
4. Insert the Nano, power J1 with the strips connected, and check the voltage at the Nano's 5V pin before uploading anything.
5. Do not plug USB into the Nano while J1 is powered.

Free area: the left and right edges, rows 14-23, have room for a 0.1 uF capacitor from A0 to GND if the speed knob jitters.

Pinout reference: Last Minute Engineers (Arduino Nano).
