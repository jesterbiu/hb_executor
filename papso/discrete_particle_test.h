#pragma once
#include <random>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdio>
#include "discrete_particle.h"
#include "canonical_rng.h"

using pos_t = discrete_particle::position_type;
using vel_t = discrete_particle::velocity_type;
typedef unsigned long long size_t;
static constexpr size_t DIM = 52u;

void vel_mult_test()
{	
	dpso_velocity_multiplies_perm multiplies;
	canonical_rng rng;
	const vel_t vel(DIM, { 0, 0 });
	for (auto i = 0u; i < 10; ++i) {
		const double r = rng();
		vel_t vel_ = multiplies(r, vel);
		printf_s("DIM: %llu; r: %lf; sz: %llu\n", DIM, r, vel_.size());
	}
}

void pos_minus_plus()
{
	std::default_random_engine e(std::random_device{}());
	pos_t pos(DIM, 0);
	std::iota(pos.begin(), pos.end(), 0);
	dpso_position_minus_perm minus;
	dpso_position_plus_perm plus;
	for (auto i = 0u; i < 100; ++i) {
		pos_t rand_pos = pos;
		std::shuffle(rand_pos.begin(), rand_pos.end(), e);
		const vel_t vel = minus(pos, rand_pos);
		rand_pos = plus(std::move(rand_pos), vel);
		printf_s("vel.size(): %llu\n", vel.size());
		assert(rand_pos == pos);
	}
}


void discrete_particle_test()
{
	vel_mult_test();
	pos_minus_plus();
}