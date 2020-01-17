aspng: application-specific PNG
-------------------------------

Have you ever wanted to design a CPU with Paint? Well now you can!

`aspng` allows you to draw out transistor-level digital circuits using pixels,
and then simulate (and interact with!) those circuits.

Design guide
------------
[0, 0, 0]        background
[255, 255, 255]  source
#595652          sink
copper 1, 2      copper  (@runtime, either light gray or dark gray, 2x for different types)
copper 3         bridge  (stays copper @ runtime)
_purple_         transistor
_yellow_ _source_   pullup
_yellow_ _sink_   pulldown
red               capacitor
green            template: LED, clock (also color for bonding)

recommended: aseprite. If you use aseprite, you can use palette.aseprite . It
has the above palette, laid out in order.

specific components i want to be able to do:
gates - in NMOS style, in CMOS style
sram
dram
mask rom
adder
shift register
latch
flipflop
clock divider


Goals of `aspng`
----------------

As the name `aspng` suggests, I want to draw a parallel to ASIC (or VLSI)
design. An ASIC implementing digital logic does so through a careful
arrangement of n- or p-channel MOSFETs, and pullups or pulldowns implemented
through depletion-mode MOSFETS (for e.g. NMOS or PMOS logic), or both n- and
p-channel MOSFETS (for e.g. CMOS logic). 

All logical gates, SRAM, adders, muxers, etc. are built with these primitives.
For the same reasons that I didn't find building a circuit in Verilog engaging,
I didn't find it engaging to design circuits at the level of boolean logic, and
so I decided I would build a transistor-level simulator that I could use and
understand.

Yet there is a deeper, darker magical art of understanding and managing power
consumption, switching delays, parasitic capacitances, capacitance delays,
leakage, and so on that true VLSI designers must practice. For the sake of
keeping `aspng` fun and instructive, (and because I'm incapable of writing a
program to simulate those effects), `aspng` ignores those effects.

`aspng` is mostly a toy, but one you can learn some low-level principles from,
because it does force you to think about:
* voltages - whether high, low, or floating,
* propagation delay,
* layout.

Related projects / inspiration
------------------------------

<http://www.visual6502.org>
<https://github.com/onidev/TICS>
<http://www.simplecpudesign.com>
<https://monster6502.com>
