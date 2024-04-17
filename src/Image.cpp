#include "Image.h"
#include <iomanip>

Image::Image(string filename) :removed(), restored() {
	std::cout << "Loading image...\n";

	ifstream file("images/in/" + filename);
	if (!file) {
		throw runtime_error("Unable to open file in Image constructor");
	}

	string header;
	file >> header;
	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  

	if (header != "P2") {
		throw runtime_error("Invalid PGM file");
	}

	while (file.peek() == '#') {
		file.ignore(numeric_limits<streamsize>::max(), '\n');
	}
	
	file >> width >> height >> max_val;
	original.resize(width * height);
	for (int i = 0; i < width * height; i++) {
		file >> original(i);
	}

	file.close();

	std::cout << "Image loaded, dimensions: " << width << "x" << height << std::endl;
}

void Image::remove(size_t percent) {
	std::cout << "Removing " << percent << "%...\n";

	if (width <= 2 || height <= 2) {
		throw runtime_error("Image width and height must be greater than 2");
	}
	else if(width > numeric_limits<int>::max() || height > numeric_limits<int>::max()) {
		throw runtime_error("Image dimensions are too large");
	}

	removed = original;
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> X(1, static_cast<int>(width - 2));
	uniform_int_distribution<> Y(1, static_cast<int>(height - 2));

	size_t n = static_cast<size_t>((width - 2) * (height - 2) * percent / 100);

	for (size_t i = 0; i < n; i++) {
		int& pixel = removed[width * Y(gen) + X(gen)];

		if (pixel == 0) {
			i--;
			continue;
		}

		pixel = 0;
	}
}
void Image::saveRemoved() {
	if (removed.size() == 0) {
		throw runtime_error("Removed image cannot be empty");
	}

	ofstream file("images/out/imageRemovedPixels.pgm");
	if (!file) {
		throw runtime_error("Unable to open file in saveRemoved");
	}

	file << "P2\n" << "# pixels removed\n" << width << " " << height << endl << max_val << endl;

	for (int i = 0; i < width * height; i++) {
		file << removed(i) << endl;
	}
}

void Image::restore() {
	std::cout << "\nImage restoration started\n";
	std::cout << "Creating sparse matrix...\n";
	size_t size = width * height;
	SparseMatrix<double, RowMajor> m(size, size);
	m.reserve(5 * size);
	
	for (size_t i = 0; i < size; i++) {
		if (removed(i) == 0) {
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
		ostringstream oss;
		oss << fixed << setprecision(2) << progress << "%";
		printf("\33[2K\r%s", oss.str().c_str());
	}

	Eigen::initParallel();
	int n = 8;
	omp_set_num_threads(n);
	Eigen::setNbThreads(n);
	
	BiCGSTAB <SparseMatrix<double,RowMajor>> solver;
	std::cout << "\nComputing BiCGSTAB...\n";
	solver.setMaxIterations(100000);
	solver.setTolerance(1E-8);
	solver.compute(m);

	std::cout << "Solving...\n";
	VectorXd removed_d = removed.cast<double>();
	VectorXd restored_d = solver.solve(removed_d);

	std::cout << "# Iterations:	" << solver.iterations() << std::endl;
	std::cout << "Estimated error: " << solver.error() << std::endl;

	restored = restored_d.cast<int>();
}
void Image::saveRestored() {
	std::cout << "\nSaving restored image...";

	if (restored.size() == 0) {
		throw runtime_error("Restored image cannot be empty");
	}

	ofstream file("images/out/imageRestoredPixels.pgm");
	if (!file) {
		throw runtime_error("Unable to open file in saveRestored");
	}

	file << "P2\n" << "# pixels restored\n" << width << " " << height << endl << max_val << endl;

	for (int i = 0; i < width * height; i++) {
		file << restored(i) << endl;
	}
}