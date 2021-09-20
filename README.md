# createetest
Create exposure test file for the Sparkmaker FHD

## NOTE: This currently creates a print file with multiple [move Z axis to top] commands at the end, causing the Sparkmaker to keep hitting the top of the Z axis.
If you need to use the program, edit out those MOVE commands before printing!

## How to use this
Here's an example, assuming you want to test eight different exposure times from 8s to 15s:

1. Load your test shape in Chitubox.
2. Make a total of eight copies, and distribute them on the build plate (if you keep them tied together, it'll be easier to keep track of which one is which afterwards).
3. Slice this for an exposure time of 8s, and save to a numbered file, as _etest_01.fhd_ (or _etest_08.fhd_, if you prefer that).
4. Remove one copy - the one to receive the shortest exposure time.
5. Slice again, and save as number _etest_02.fhd_ (or _etest_09.fhd_) - the exposure time is not relevant here.
6. Redo 4-5 until only one copy of the shape is left.
7. Now you should have eight files, which when listed ends up in numerical order
8. Now run the program, and generate the test file!

```
./createetest -b 8 -d 1 -o etest.fhd etest_??.fhd
```

Run the program without parameters to see usage info.

## TODO
Currently, the program assumes a lot. It assumes that the structure of a single layer, apart from Z axis movement, is fixed:

```
;dataSize:19354
{{
<PNG data>
}}
M106 S255;
G4 S100;
;L:2;
M106 S0;
```

If Chitubox starts producing different G-code, this program will have to be modified to accommodate that.

Additionally, it blindly assumes that all the files have exactly the same structure.
If you follow the instructions, that's not going to be a problem.
However, we really only need the image blobs from all the input files apart from the first one, so we could
just as easily scan those files until the ";dataSize" header appears and use that to extract the PNG, and
ignore all other rows, copying the necessary commands from the first file instead of reading them from each file.
