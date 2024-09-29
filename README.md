State of the project:<br />
It will remain dormant. Was intended as 1-2 weeks project to check the viability of the approach. <br />
Constraints: Only position, rotation and scale can be modified on images. <br />
Results: It does generate image similar to the given one, results can only represent general form but not finer details. Perhaps having non-rectangular images will help, but that tends to make generation time too long. I also tried using edge similarity as a metric, however that made generation time slow again. Both ideas were not explored thouroughly. <br />
There a bunch of parameters to tweak and my methods are by no way optimal. However, I do not believe that improving on those will fix the details issue. I am not sure if that issue is solvable with fast runtimes. <br />

I do not have a great dataset to show off an image. This is the results generated in 5 minutes on cat dataset(bad dataset, as color palette is not big, and backgrounds which usually dominate the images have even smaller palette). You can kinda see the algorithm tried but only got general shapes and general color clusters inside of the shape, but with no opacity achieving detailed pictures will be hard.
![Sample Image](https://github.com/508312/public-files/blob/master/genetic_images/image.png)

Resources used:<br />
https://github.com/openlab-vn-ua/ImageRotation/tree/master <br />
https://github.com/m3y54m/sobel-simd-opencv (function present, but not being used) <br />

Concept is based on:<br />
https://users.cg.tuwien.ac.at/zsolnai/gfx/mona_lisa_parallel_genetic_algorithm/ <br />
and Gabriele Greco implementation at http://www.ggsoft.org/archives/genetic.zip
