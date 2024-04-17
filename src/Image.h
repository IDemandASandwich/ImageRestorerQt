#pragma once

#include <random>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/Core>
#include <omp.h>
#include <iomanip>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "ViewerWidget.h"

using namespace std;
using namespace Eigen;

class Image
{
private:
	ViewerWidget* view;
	VectorXi imageData;
	size_t width;
	size_t height;
	size_t max_val;

public:
	Image();
	Image(QString filename);
	void remove(size_t percent);
	void restore();
	void save(QString filename);

};

