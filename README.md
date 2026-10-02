# sdltrs3
Port of sdltrs 1.1.0 to SDL3/emscripten

The original sdltrs can be found here: https://sdltrs.sourceforge.net/

In it's current state, this this a direct/lazy port.  Rather than reworking the optimizations made for SDL1, I've focused on making the minimal changes required to get it to run with SDL3 and with a new emscripten target.  This means that the screen rendering code is doing some silly things now.  I haven't made any great attempt to fix bugs in the original code, however getting a clean and working compile to emscripten required a good bit of code clean-up.  Additionally, there were quite a few tweaks for SDL1 running on MacOS X that I've left in.  I have not even attempted to compile on MacOS X, and I imagine that they will now cause more problems than they fix with SDL3.

To build for Linux and execute Meteor Mission 2:
```
cd src/linux
make
./sdltrs -model3 -romfile3 ../../test/model3.rom -disk0 ../../metmis2a.dsk
```

To build for emscripten and execute Meteor Missions 2:
```
cd src/emscripten
cp ../../test/* data
export EMARGS="-model3 -romfile3 ../../test/model3.rom -disk0 ../../metmis2a.dsk"
make
python3 -m http.server
[open http://localhost:8000/sdltrs.html]
```
