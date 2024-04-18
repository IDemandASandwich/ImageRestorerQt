#include   "ViewerWidget.h"

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent), width(0), height(0), max_val(0)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img != nullptr) {
		delete painter;
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}
bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete painter;
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
{
	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = b;
	data[startbyte + 1] = g;
	data[startbyte + 2] = r;
	data[startbyte + 3] = a;
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(255 * valB);
	data[startbyte + 1] = static_cast<uchar>(255 * valG);
	data[startbyte + 2] = static_cast<uchar>(255 * valR);
	data[startbyte + 3] = static_cast<uchar>(255 * valA);
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		size_t startbyte = y * img->bytesPerLine() + x * 4;

		data[startbyte] = color.blue();
		data[startbyte + 1] = color.green();
		data[startbyte + 2] = color.red();
		data[startbyte + 3] = color.alpha();
	}
}
void ViewerWidget::setPixel(int x, int y, uchar gray)
{
	size_t index = y * img->bytesPerLine() + x;
	data[index] = gray;
}
void ViewerWidget::setPixel(int i, uchar gray)
{
	int x = i % img->width();
	int y = i / img->width();
	size_t index = y * img->bytesPerLine() + x;
	data[index] = gray;
}

void ViewerWidget::clear()
{
	img->fill(Qt::white);
	update();
}

//PGM functions
void ViewerWidget::loadPGM(QString filename) {
	qDebug() << "Loading image...";

	height = 0; width = 0; max_val = 0;

	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Unable to open file in Image constructor";
		return;
	}

	QTextStream in(&file);

	if (in.readLine() != "P2") {
		qDebug() << "Invalid PGM file";
		return;
	}

	qint64 pos = in.pos();
	if (!in.readLine().startsWith('#'))
		in.seek(pos);

	in >> width >> height >> max_val;
	imagePGM.resize(width * height);

	for (int i = 0; i < width * height; i++) {
		in >> imagePGM(i);
	}

	file.close();

	qDebug() << "Image loaded, dimensions:" << width << "x" << height;
}
void ViewerWidget::savePGM(QString filename) {
	qDebug() << "Saving image...";

	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Unable to open file for writing";
		return;
	}

	QTextStream out(&file);

	// Write the header
	out << "P2\n";
	out << "# Created by ImageRestorer - restored by Laplace interpolation\n";
	out << width << " " << height << "\n";
	out << max_val << "\n";

	// Write the pixel data
	for (int i = 0; i < width * height; i++) {
		out << imagePGM(i) << " ";
		if ((i + 1) % width == 0) { // end of a row
			out << "\n";
		}
	}

	file.close();

	qDebug() << "Done";
}

void ViewerWidget::removePgmPixels(size_t percent) {
	if (imagePGM.size() == 0) {
		qDebug() << "No image";
		return;
	}

	qDebug() << "Removing " << percent << "%...";

	if (width <= 2 || height <= 2) {
		qDebug() << "Image width and height must be greater than 2";
	}
	else if (width > std::numeric_limits<int>::max() || height > std::numeric_limits<int>::max()) {
		qDebug() << "Image dimensions are too large";
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> X(1, static_cast<int>(width - 2));
	std::uniform_int_distribution<> Y(1, static_cast<int>(height - 2));

	size_t n = static_cast<size_t>((width - 2) * (height - 2) * percent / 100);
	size_t maxIterations = 2 * width * height;

	for (size_t i = 0; i < n; i++) {
		int y = Y(gen);
		int x = X(gen);
		int& pixel = imagePGM(width * y + x);

		if (pixel == 0) {
			i--;
			maxIterations--;

			if (maxIterations == 0)
				break;
			else
				continue;
		}

		pixel = 0;
		setPixel(x, y, 0);
	}
	update();
	qDebug() << "Done";
}
void ViewerWidget::restorePgmPixels() {
	if (imagePGM.size() == 0) {
		qDebug() << "No image";
		return;
	}

	qDebug() << "\nImage restoration started";
	qDebug() << "Creating sparse matrix...";
	size_t size = width * height;
	Eigen::SparseMatrix<double, RowMajor> m(size, size);
	m.reserve(5 * size);

	for (size_t i = 0; i < size; i++) {
		if (imagePGM(i) == 0) {
			m.insert(i, i) = 4.;

			if (i - 1 >= 0) {
				m.insert(i, i - 1) = -1.;
			}
			if (i + 1 < size) {
				m.insert(i, i + 1) = -1.;
			}
			if (i - width >= 0) {
				m.insert(i, i - width) = -1.;
			}
			if (i + width < size) {
				m.insert(i, i + width) = -1.;
			}
		}
		else {
			m.insert(i, i) = 1.;
		}

		double progress = static_cast<double>(i + 1) / size * 100;
		printf("\r%.2f%%", progress);
		fflush(stdout);
	}

	initParallel();
	int n = 8;
	omp_set_num_threads(n);
	Eigen::setNbThreads(n);

	BiCGSTAB <SparseMatrix<double, RowMajor>> solver;
	qDebug() << "Computing BiCGSTAB...";
	solver.setMaxIterations(100000);
	solver.setTolerance(1E-8);
	solver.compute(m);

	qDebug() << "Solving...\n";
	VectorXd removed_d = imagePGM.cast<double>();
	VectorXd restored_d = solver.solve(removed_d);

	qDebug() << "# Iterations:	" << solver.iterations();
	qDebug() << "Estimated error: " << solver.error() << "-----------------\n";

	imagePGM = restored_d.cast<int>();

	for (int i = 0; i < imagePGM.size(); i++) {
		setPixel(i, imagePGM(i));
	}
	update();
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}