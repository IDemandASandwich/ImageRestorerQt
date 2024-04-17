#pragma once

#include <iostream>
#include <fstream>
#include <random>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/Core>
#include <omp.h>

using namespace std;
using namespace Eigen;

class Image
{
private:
	VectorXi original;
	VectorXi removed;
	VectorXi restored;
	size_t width;
	size_t height;
	size_t max_val;

public:
	Image(string filename);
	void remove(size_t percent);
	void saveRemoved();
	void restore();
	void saveRestored();

};

