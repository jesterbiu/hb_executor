#pragma once
#include <cmath>
#include <numeric>
#include <algorithm>
#include <cstddef>
#include <vector>
// 16 digits Pi
static constexpr double Pi = 245850922.0 / 78256779.0;

using iter = std::vector<double>::const_iterator;

// Class 1

// f1
inline double sphere(iter beg, iter end) {	
	double sum = 0;
	while (beg != end) {
		sum += std::pow(*beg, 2);
		beg++;
	}
	return sum;
}

// f2 
inline double schwefel_12(iter beg, iter end) {
	double sum = 0;	
	iter next = beg + 1;

	// next : [beg, end)
	while (next != end) {
		iter it = beg;

		// first : [beg, next)
		while (it != next) {
			sum += std::pow(*it, 2);
			it++;
		}

		next++;
	}
	return sum;
}

// f3
inline double rosenbrock(iter beg, iter end) {
	double sum = 0;
	iter next = beg + 1;
	while (next != end) {
		sum += 100 * std::pow(*next - std::pow(*beg, 2), 2)
			+ std::pow(*beg, 2);
		beg++;
		next++;
	}
	return sum;
}

// Class 2

// f4
//inline double (const double* vec, size_t dim)
inline double schwefel_26(iter beg, iter end) {		
	double sum = 0;
	while (beg != end) {
		sum -= *beg * std::sin(std::sqrt(*beg));
		beg++;
	}
	return sum;
}

// f5
inline double rastrigin(iter beg, iter end) {
	double sum = 0;
	while (beg != end) {
		sum += std::pow(*beg, 2)
			- 10 * std::cos(2 * Pi * (*beg))
			+ 10;

		beg++;
	}
	return sum;
}

// f6
inline double ackley(iter beg, iter end)
{
	double square_sum = 0;
	double cos_sum = 0;
	auto dim = end - beg;
	while (beg != end) {
		square_sum  += std::pow(*beg, 2);
		cos_sum		+= std::cos(2 * Pi * (*beg));
		beg++;
	}
	return -20 * std::exp(-0.2 * std::sqrt(square_sum / dim))
		- std::exp(cos_sum / dim)
		+ 22.718282;
}

// f7
inline double griewank(iter beg, iter end) {

	double sum = 0.;
	double product = 1.;
	iter it = beg;
	while (it != end) {
		sum += pow(*it, 2);
		product *= cos(*it / sqrt((it - beg) + 1));
		++it;
	}

	return sum / 4000. - product + 1.;
}

// f8

// f9
