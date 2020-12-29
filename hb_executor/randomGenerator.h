#pragma once

#include <random>
typedef unsigned range;

class randomGenerator
{
public:
	randomGenerator() {}
	~randomGenerator() {}
	randomGenerator(const randomGenerator&) = delete;
	randomGenerator& operator=(const randomGenerator&) = delete;

	//get numbers
	unsigned int get_uniform()
	{
		return uni_disb(engine);
	}
	unsigned get_uniform(range r)
	{
		return get_uniform((range)0, r);
	}
	unsigned get_uniform(range l, range h)//range: [l, h)
	{
		std::uniform_int_distribution<unsigned>::param_type p(l, h - 1);
		uni_disb.param(p);
		unsigned rdn = uni_disb(engine);
		reset_param();
		return rdn;
	}

	void get_uniform(unsigned* a, int sz)
	{
		get_uniform(a, sz, UINT_MAX);
	}
	void get_uniform(unsigned* a, int sz, range r)
	{
		get_uniform(a, sz, range(0), r);
	}
	void get_uniform(unsigned* a, int sz, range l, range h)
	{
		for (int i = 0; i != sz; i++)
			a[i] = uni_disb(engine);
	}

private:
	std::default_random_engine engine;
	std::uniform_int_distribution<unsigned> uni_disb;
	void reset_param()
	{
		std::uniform_int_distribution<unsigned>::param_type p(0, UINT_MAX - 1);
		uni_disb.param(p);
	}
};