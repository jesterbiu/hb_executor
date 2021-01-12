/*
* Random number generator with normal distribution within [0, 1)
* Change this to a object?
*/
#pragma once
#include <random>
#include <concepts>
struct canonical_rng
{
	std::mt19937 generator_; //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<double> real_distribute;

	canonical_rng() :
		generator_(std::random_device{}()), real_distribute(0., 1.) {}
	canonical_rng(const canonical_rng& oth) :
		generator_(oth.generator_), real_distribute(0., 1.) {}
	~canonical_rng() {}

	// Return real number randomly distrubuted from [0, 1)
	inline double operator()()
	{
		return real_distribute(generator_);
	}
};