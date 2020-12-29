/*
* Random number generator with normal distribution within [0, 1)
* Change this to a object?
*/
#pragma once
#include <random>
struct canonical_rng
{
	std::mt19937 generator_; //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> distribute_;

	canonical_rng() :
		generator_(), distribute_(0., 1.) {}
	// Return real number randomly distrubuted from [0, 1)
	inline double operator()()
	{
		return distribute_(generator_);
	}
};