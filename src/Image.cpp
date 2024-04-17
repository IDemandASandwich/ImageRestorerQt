#include "Image.h"

Image::Image():height(0), width(0), max_val(0) {};
Image::Image(QString filename):view(view) {
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
	imageData.resize(width * height);

	for (int i = 0; i < width * height; i++) {
		in >> imageData(i);
	}

	file.close();

	qDebug() << "Image loaded, dimensions:" << width << "x" << height;
}

void Image::remove(size_t percent) {
	if (imageData.size() == 0) {
		qDebug() << "No image";
		return;
	}

	qDebug() << "Removing " << percent << "%...";

	if (width <= 2 || height <= 2) {
		throw runtime_error("Image width and height must be greater than 2");
	}
	else if(width > numeric_limits<int>::max() || height > numeric_limits<int>::max()) {
		throw runtime_error("Image dimensions are too large");
	}

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> X(1, static_cast<int>(width - 2));
	uniform_int_distribution<> Y(1, static_cast<int>(height - 2));

	size_t n = static_cast<size_t>((width - 2) * (height - 2) * percent / 100);

	for (size_t i = 0; i < n; i++) {
		int& pixel = imageData(width * Y(gen) + X(gen));

		if (pixel == 0) {
			i--;
			continue;
		}

		pixel = 0;
	}

	qDebug() << "Done";
}
void Image::restore() {
	qDebug() << "\nImage restoration started";
	qDebug() << "Creating sparse matrix...";
	size_t size = width * height;
	SparseMatrix<double, RowMajor> m(size, size);
	m.reserve(5 * size);
	
	for (size_t i = 0; i < size; i++) {
		if (imageData(i) == 0) {
			m.insert(i, i) = 4.;

			if (i - 1 >= 0) {
				m.insert(i , i - 1) = -1.;
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
		QString progressStr = QString("Progress: %1%").arg(progress, 0, 'f', 2);
		qDebug().noquote().nospace() << "\r" << progressStr;
	}

	Eigen::initParallel();
	int n = 8;
	omp_set_num_threads(n);
	Eigen::setNbThreads(n);
	
	BiCGSTAB <SparseMatrix<double,RowMajor>> solver;
	qDebug() << "\nComputing BiCGSTAB...\n";
	solver.setMaxIterations(100000);
	solver.setTolerance(1E-8);
	solver.compute(m);

	qDebug() << "Solving...\n";
	VectorXd removed_d = imageData.cast<double>();
	VectorXd restored_d = solver.solve(removed_d);

	qDebug() << "# Iterations:	" << solver.iterations();
	qDebug() << "Estimated error: " << solver.error();

	imageData = restored_d.cast<int>();
}

void Image::save(QString filename) {
	qDebug() << "Saving image...";

	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		throw std::runtime_error("Unable to open file for writing");
	}

	QTextStream out(&file);

	// Write the header
	out << "P2\n";
	out << "# Created by ImageRestorer - restored by Laplace interpolation\n";
	out << width << " " << height << "\n";
	out << max_val << "\n";

	// Write the pixel data
	for (int i = 0; i < width * height; i++) {
		out << imageData(i) << " ";
		if ((i + 1) % width == 0) { // end of a row
			out << "\n";
		}
	}

	file.close();

	qDebug() << "Done";
}