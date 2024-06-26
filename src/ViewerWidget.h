#pragma once

#include <QtWidgets>
#include <random>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/Core>
#include <omp.h>
#include <iomanip>

using namespace Eigen;

class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize size = QSize(0, 0);
	QImage* img = nullptr;
	QPainter* painter = nullptr;
	uchar* data = nullptr;

	VectorXi imagePGM;
	size_t width;
	size_t height;
	size_t max_val;

public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	void setPixel(int x, int y, uchar gray);
	void setPixel(int i, uchar gray);

	bool isInside(int x, int y) { return (x > 0 && y > 0 && x < img->width() && y < img->height()) ? true : false; }
	bool isInside(QPoint p) { return (p.x() > 0 && p.y() > 0 && p.x() < img->width() && p.y() < img->height()) ? true : false; }

	//Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img->bits(); }
	void setPainter() { painter = new QPainter(img); }

	int getImgWidth() { return img->width(); };
	int getImgHeight() { return img->height(); };

	void clear();

	//PGM functions
	void loadPGM(QString filename);
	void removePgmPixels(size_t percent);
	void restorePgmPixels();
	void savePGM(QString filename);
	void EOC(int n);
	VectorXd restore(int n);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};