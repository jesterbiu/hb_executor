// Codes in this header has been tested!

#pragma once

#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <numeric>
#include <cmath>
#include <cstdio>

using coord = std::pair<double, double>;

std::vector<std::string> split(const std::string& str, char spliter)
{
	std::vector<std::string> vs;
	if (str.empty() || spliter == '\0')
	{
		return vs;
	}

	for (auto i1 = str.cbegin(); i1 != str.cend(); i1++)
	{
		// Look for spliter
		auto i2 = i1;
		for (; i2 != str.cend() && *i2 != spliter; i2++)
		{
		}

		// Split and store and substring
		vs.push_back(std::string(i1, i2));

		if (i2 != str.cend())
			i1 = i2;		// Continue
		else
			break;			// End fo the string
	}

	return vs;
}

void transform_coord_format(const std::string& fname)
{
	// input format: 1 565.0 575.0
	std::ifstream ifs(fname);
	std::ofstream ofs(fname + "_rf");
	assert(ifs.is_open() && ofs.is_open());

	std::string buf;
	while (!ifs.eof()) {
		std::getline(ifs, buf);
		auto strvec = split(buf, ' ');
		strvec[1] = "{ " + strvec[1] + ", ";
		strvec[2] += " },\n";
		for (auto i = 1u; i < strvec.size(); ++i) ofs << strvec[i];
	}			
}

double tsp(const std::vector<int>& pos, const coord* map)
{
	const auto size = pos.size() + 1; // 1 more element for trip back to the start point
	std::vector<double> diff_vec(size);

	// Calculate length of the route
	auto distance = [](const coord& c1, const coord& c2) {
		return std::sqrt(std::pow(c1.first - c2.first,  2) + std::pow(c1.second - c2.second, 2));
	};
	std::adjacent_difference(pos.cbegin(), pos.cend(), diff_vec.begin(),
		[&](auto p2, auto p1) { return distance(map[p2], map[p1]); });	
	
	// The length going back to start point
	diff_vec.back() = distance(map[pos.front()], map[pos.back()]);

	// Skip the first element
	return std::accumulate(diff_vec.cbegin() + 1, diff_vec.cend(), 0.); 
}

struct tsp_solve
{
	const coord* m_map;

	tsp_solve(const coord* ptr_map) : m_map(ptr_map) {}
	int operator()(const discrete_particle::position_type& input)
	{
		// Implicityly convert float point to integer
		return static_cast<int>(tsp(input, m_map));
	}
};