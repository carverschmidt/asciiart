# asciiart
A commandline asciiart generator written in C. I wrote this for fun to gain experience with libjpeg and libpng. Therefore, the program only works with jpeg and png files.

Usage
-----
```
gcc -lm -lpng -ljpeg -o asciiart asciiart.c
# then...
asciiart image [max_width]
```

Implementation
-----
This ascii art generator makes use of the libpng and libjpeg libraries to process png and jpeg image files into grayscale. The grayscale value for each pixel is then output as some character that corresponds to the grayscale value of the pixel. The output asciiart is intended to be viewed with a dark background and light text (ideally black background and white text).

