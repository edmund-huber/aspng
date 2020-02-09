aspng: application-specific PNG
-------------------------------

Have you ever wanted to design a CPU with Paint? Well now you can!

`aspng` ("application-specific PNG") lets you draw out transistor-level digital
circuits using pixels, and then simulate (and interact with!) those circuits.

Have a look at some <a href="">interactive examples</a>.

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

Goals of `aspng`
----------------

A digital ASIC is a careful arrangement of n- and/or p-channel MOSFETs,
pullups, and pulldowns, implementing arbitrarily complicated relationships
between input and output pins.

In the early days, all decisions about the logic and the layout of these
circuits were made by humans, and big masks were drawn out by hand.
Microprocessors like the Intel 4004 and the MOS 6502 were designed this way.

Today, digital ASICs are much too large and complicated to be designed by
humans. Instead, hardware description languages like Verilog or VHDL are used
to describe the behavior, and then very heavily researched computer programs
figure out the exact layout of physical devices to realize the design ("logic
synthesis").

I'm a simple person. I feel drawn to those "early days", with the intimate and
basically artisanal relationship between the designer and their circuit. Every
chip was bespoke, and had only one or a few minds behind it.

But at the same time, I don't want to get bogged down in managing power
consumption, switching delays, parasitic capacitances, capacitance delays,
leakage, and so on that true ASIC designers have to consider. I'm lazy! I just
want the fun parts!

So for the sake of keeping "ASIC design" fun (?) and feasibly simple for a
non-expert who just wants to see some blinkenlights, `aspng` greatly simplifies
all electrical properties:

* *resistance* does not exist.
* *capacitance* only exists in CapacitorDevices, which charge instantly and
  don't lose charge over time.
* *inductance* does not exist.

Related projects / inspiration
------------------------------

<http://www.visual6502.org>
<https://github.com/onidev/TICS>
<http://www.simplecpudesign.com>
<https://monster6502.com>
