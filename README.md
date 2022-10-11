# C++ Raytracer
Ray tracing experiments with C++

<center>
    <img src=".github/day1.png">
</center>

## Project
This is a simple implementation of a ray tracing engine in C++. This project is still a work in progress, expect bugs.

The project requires `SDL2` to handle cross platform graphics.

My intention is to register my learnings throughout this project in this repository.

## Dev log
### Day 0
I had my experiments with Ray tracing before but never really got serious. I was able to achieve something similar with AssemblyScript compiling to WASM, but the performance wasn't really what I was expecting, which led me to try with native C++ and SDL.

I started learning the basics of SDL, it has been a while since my last SDL project.
<center>
    <img src=".github/day0.png">
    <small>Here's me rendering a gradient</small>
</center>

---
### Day 1
After a few side projects in SDL, it's time to start ray tracing!

<center>
    <img src=".github/day1.png">
    <small>Some HighDPI/Retina enabled rendering</small>
<center>
</center>
    <img src=".github/day1.gif">
    <small>And some really cool realtime 120fps interaction</small>
</center>

This was basically the first day. Where I got some basic ray casting working.

It seems that doing the operations in the back buffer penalizes performance when setting pixels manually, maybe having a surface and manipulating it manually would provide a better performance, since now the actual ray tracing takes 6ms tops and the back buffer manipulation takes 40ms 😕.