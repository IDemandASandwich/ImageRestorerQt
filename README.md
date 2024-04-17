# Restoring images using Laplace interpolation

A short algorithm using Eigen library and its BiCGSTAB solver to restore PGM P2 images.
Should be cloned with "git clone --recurse-submodules".

How to use:
1) Drop your photos into "build/images/in/"
2) In image-restorer.cpp, change the string in constructor to fit your image
3) Output is in "build/images/out/"

TODO:
1) Add an interface