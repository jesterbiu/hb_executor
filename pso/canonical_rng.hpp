#pragma once
#include <random>
inline double canonical_rng()
{
	static std::mt19937 generator(std::random_device{}()); //Standard mersenne_twister_engine seeded with rd()
	static std::uniform_real_distribution<> distribute(0., 1.);
	return distribute(generator);
}