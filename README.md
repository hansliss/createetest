# createetest
Create exposure test file for the Sparkmaker FHD

## How to use this
Here's an example, assuming you want to test eight different exposure times from 8s to 15s, with three base layers:

1. Load your test shape in Chitubox.
2. Make a total of eight copies, and distribute them on the build plate (if you keep them tied together, it'll be easier to keep track of which one is which afterwards).
3. Slice this for an exposure time of 8s, and save to a numbered file, as _etest_01.fhd_ (or _etest_08.fhd_, if you prefer that).
4. Remove one copy - the one to receive the shortest exposure time.
5. Slice again, and save as number _etest_02.fhd_ (or _etest_09.fhd_) - the exposure time is not relevant here.
6. Redo 4-5 until only one copy of the shape is left.
7. Now you should have eight files, which when listed ends up in numerical order
8. Now run the program, and generate the test file!

```
./createetest -B 3 -b 8 -d 1 -o etest.fhd etest_??.fhd
```

Run the program without parameters to see usage info.

## To build
You can either build this with Visual Studio, or in Linux or Mingw-64, using the normal Gnu autoconf method:
```
./configure
make
make install
```
