This is one of the classic "Mandelbrot Set/Julia Set" programs that were so
popular - I don't know when: 1990s, I guess, or even earlier.

First off: This one isn't any different or better than the other ones.

I wrote it for two purposes, and for these, it may be interesting for you as
well:

- To learn about multithreading with workers and combining the results to a 
single unit. This application parallelizes extremely well and is thus a good
training exercise.
- To generate pictures that a larger than the screen resolution. You can
easily generate VERY large pictures with it, practically only limited by
your memory (PNG - the output format - can be 2 billion pixels on each side).
I used this to generate large high-resolution prints (posters - hence the
name!) to hang in my stairwell.

In all honesty, I haven touched this piece of software in about five years. 
There's a number of things I'd like to implement, see the corresponding 
"issues" section.
