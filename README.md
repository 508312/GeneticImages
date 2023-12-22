State of the project:
It will remain dormant. Was intended as 1-2 weeks project to check the viability of the approach.
Constraints: Only position, rotation and scale can be modified on images.
Results: It does generate image similar to the given one, results can only represent general form but not finer details. Perhaps having non-rectangular images will help, but that tends to make generation time too long. I also tried using edge similarity as a metric, however that made generation time slow again. Both ideas were not explored thouroughly.
There a bunch of parameters to tweak and my methods are by no way optimal. However, I do not believe that improving on those will fix the details issue. I am not sure if that issue is solvable with fast runtimes.

I do not have a great dataset to show off an image.

Resources used:
https://github.com/openlab-vn-ua/ImageRotation/tree/master
https://github.com/m3y54m/sobel-simd-opencv (function present, but not being used)

Concept is based on:
https://users.cg.tuwien.ac.at/zsolnai/gfx/mona_lisa_parallel_genetic_algorithm/
and Gabriele Greco implementation at http://www.ggsoft.org/archives/genetic.zip
