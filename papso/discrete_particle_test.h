#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <unordered_map>
#include <cassert>
#include <cstdio>

using velocity_type = std::pair<size_t, size_t>;
std::vector<velocity_type> calc_vel(const std::vector<int>& pos_dst, const std::vector<int>& pos_src)
{
	const auto D = pos_dst.size();
	std::unordered_map<int, size_t> map_src_idx;
	for (size_t i = 0; i < D; ++i) {
		map_src_idx[pos_src[i]] = i;
	}

	std::vector<int> src_mutable = pos_src;
	std::vector<velocity_type> vel;
	vel.reserve(D);
	for (size_t i = 0; i < D; ++i) {
		const auto dst_val = pos_dst[i];
		if (src_mutable[i] == dst_val) continue;
		else {
			// Update velocity
			const auto src_idx = map_src_idx[dst_val];
			vel.emplace_back(i, src_idx);

			// Swap element to the right position
			std::swap(src_mutable[i], src_mutable[src_idx]);

			// Update lookup table
			map_src_idx[src_mutable[i]] = i;
			map_src_idx[src_mutable[src_idx]] = src_idx;
		}
	} // end of for
	return vel;
}

void update_pos(std::vector<int>& pos, const std::vector<velocity_type>& vel)
{
	for (const auto& v : vel) {
		std::swap(pos[v.first], pos[v.second]);
	}
}

void discrete_particle_test()
{
	// Init a sequence
	static constexpr auto Dimension = 20u;
	std::vector<int> og_pos(Dimension);
	std::iota(og_pos.begin(), og_pos.end(), 0);	

	// Shuffle it
	static constexpr auto TestSz = 10u;
	std::vector<std::vector<int>> test_set(TestSz, og_pos);	
	auto g = std::default_random_engine{};
	for (auto& vec : test_set) {
		std::shuffle(vec.begin(), vec.end(), g);
		for (auto i : vec) printf_s("%d ", i);
		printf_s("\n");
	}
		

	// Calculate undo ops
	std::vector<std::vector<velocity_type>> undos;
	undos.reserve(TestSz);
	for (const auto& pos : test_set) {
		undos.push_back(calc_vel(og_pos, pos));
	}

	// Undo
	std::vector<std::vector<int>> recovers(TestSz);
	std::transform(
		test_set.cbegin(), test_set.cend(), // Input sequence 1
		undos.cbegin(),						// Input sequence 2
		recovers.begin(),					// Output sequence
		[](auto& pos, auto& vel) {			// Binary functor to apply
			auto new_pos = pos;
			update_pos(new_pos, vel);
			return new_pos;
		});

	// Assert equal
	printf_s("\n");
	for (const auto& pos : recovers) {
		for (auto i : pos) printf_s("%d ", i);
		printf_s("\n");
		assert(pos == og_pos);
	}
	printf_s("perm recover success\n");
}
