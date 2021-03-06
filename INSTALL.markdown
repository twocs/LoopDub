
Installing LoopDub
==================

The bootstrap.sh script is made to conveniently download and compile
dependent libraries if they are not commonly available for your
system.  So, when connected to the internet, the first step is to
execute it:

    $ ./bootstrap.sh

Now, to compile LoopDub:

    $ ./configure
    $ make

LoopDub is pretty-much self-contained and can run from the source
directory after compiling.

    $ ./loopdub /path/to/loops


Running LoopDub
===============

The program takes one optional argument, which is the folder where
your loops are stored.  For quick access, I keep mine right in the
root folder, at `/loops/` This is probably bad practise.. ;-) But I
execute it like this:

    $ loopdub /loops

If you keep your loops elsewhere, run it with a different path
argument.  The path may contain an optional file called `programs.ini`
which informs LoopDub about how you want to respond to program-change
MIDI messages.  It gives each program a name, a folder (which should
be relative to the loops folder), and a list of loops associated with
that program.  (For now, up to 8 loops per program.)

See the included file, `programs.ini.example` for details.

There is also a configuration file called ".loopdub.midi.conf" which
should go into your home folder.  It contains mapping information
between MIDI controls and what GUI control they modify.  In fact, this
can easily be generated at runtime by clicking on "Learn", however the
mode buttons cannot be configured automatically.  See the included
example file.  (It is a dot-file so if you don't see it, try "ls -a".)
