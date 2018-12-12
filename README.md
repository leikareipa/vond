# Vond
A software-based hybrid voxel/polygon 3d landscape renderer, akin in spirit to Novalogic's classic Voxel Space engine, with genuine retro visuals.

The voxel renderer operates on heightmaps and runs on the CPU (multithreaded via OpenMP). The polygon rasterizer is likewise CPU-side (single-threaded), and capable of rendering triangles with affine texture mapping.

This repo contains the source code to Vond, compilable on Windows and Linux (with Qt). Note that since the program is still in a state of relative infancy, it may undergo notable changes in the short term.

### Building
To build Vond, you'll need Qt and OpenMP.

On Linux, do ```qmake && make```, or load the .pro file in Qt Creator.

Building on Windows should be much the same, though I can't say for sure.

I use GCC and Qt 5.5 with Vond, so those might give you the fewest headaches in terms of feature compatibility and so on.

![A screenshot of VCS](http://tarpeeksihyvaesoft.com/soft/img/vond.png)
