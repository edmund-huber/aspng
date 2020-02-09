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

Why `aspng`?
------------

A digital ASIC is a careful arrangement of n- and/or p-channel MOSFETs,
pullups, pulldowns, clocks, latches, etc., implementing arbitrarily complicated
relationships between input and output pins.

![MOS 6502 designers](/readme/mos6502mask.jpg)

In the early days, all decisions about the logic and the layout of these
circuits were made by humans, and the lithography masks were drawn out by hand.
Microprocessors like the Intel 4004 and the MOS 6502 (pictured above) were
designed this way.

Today, digital ASICs are much too large and complicated to be designed by
humans. Instead, hardware description languages like Verilog or VHDL are used
to describe the behavior, and then very heavily researched computer programs
figure out the exact layout of physical devices to realize the design ("logic
synthesis").

I'm a simple person. I romanticize those early days, with the intimate and
artisanal relationship between the designer and their circuit. `aspng` exists
so that I can capture a fraction of that feeling of satisfaction when a
carefully designed digital circuit does the right thing.

At the same time, I don't have a lifetime of experience managing power
consumption, switching delays, parasitic capacitances, capacitance delays,
leakage, and so on that true ASIC designers must have to make a functioning
circuit. I'm lazy! I just want the fun parts that make me feel smart. So on to
design decisions..

Design decisions
----------------

For the sake of keeping "ASIC design" fun (?) and feasibly simple for a
non-expert whose main motivation is seeing some blinkenlights, `aspng` greatly
simplifies all electrical properties:

* *Resistance* does not exist.
* *Capacitance* only exists in CapacitorDevices, which charge instantly and
  don't lose charge over time.
* *Inductance* does not exist.
* *Voltage* is either floating, low, or high.

Unlike very similar projects (that I know of) such as
[TICS]<https://github.com/onidev/TICS>, `aspng` doesn't reduce down to logical
networks of "true" or "false". To me, logical networks feel so far removed from
the reality of MOS technology and how electrical devices work, that I don't
feel compelled to write a simulator that functions at that level of
abstraction. If designing an ASIC in `aspng` is science fiction, then I view
designing a logical network as science fantasy -- interesting, but not remotely
authentic.

Related projects / inspiration
------------------------------

<http://www.visual6502.org>
<http://www.simplecpudesign.com>
<https://monster6502.com>
