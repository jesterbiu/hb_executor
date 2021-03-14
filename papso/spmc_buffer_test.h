#pragma once
#include <thread>
#include <algorithm>
#include <numeric>
#include <random>
#include <mutex>
#include <array>
#include <future>
#include <iostream>
#include <shared_mutex>
#include "spmc_buffer.h"

using vec_t = std::array<int, 40>;
static constexpr int LoopSize = 50;
static constexpr int OpSize = 20;
int reader_operation (const vec_t& v, int n) {
	int result = 0;
	for (int i = 0; i < n; ++i) {
		result += std::accumulate(v.cbegin(), v.cend(), 0);
	}
	return result;
}

template <typename BufferT>
void reader(BufferT* pb, int n, std::promise<int>* p) {
	int sum = 0;
	for (int i = 0; i < LoopSize; ++i) {
		auto view = pb->get();
		int tmp = reader_operation(*view, n);
		sum += tmp;
		//std::cout << "Reader Op (" << i << "): " << "+" << tmp << '\n';
	}
	p->set_value(sum);
}

void writer_operation(vec_t& v, int n) {
	static std::default_random_engine rng;
	for (int i = 0; i < n; ++i) {
		std::shuffle(v.begin(), v.end(), rng);
	}
}

template <typename BufferT>
void writer(BufferT* pb, vec_t* pv, int n, std::promise<int>* p) {
	for (int i = 0; i < LoopSize; ++i) {
		std::cout << "Writer Op (" << i << ")\n";
		writer_operation(*pv, n);
		pb->put(*pv);
	}
	int sum = std::accumulate(pv->cbegin(), pv->cend(), 0);
	p->set_value(sum);
}

template <typename BufferT>
void spmc_buffer_test() {
	vec_t v = {};
	std::iota(v.begin(), v.end(), 1);
		
	BufferT buf;
	buf.put(v);

	std::promise<int> p1;
	std::promise<int> p2;
	std::promise<int> pw;


	std::thread r0{ reader<BufferT>, &buf, OpSize, &p1 };
	std::thread r1{ reader<BufferT>, &buf, OpSize, &p2 };
	std::thread w { writer<BufferT>, &buf, &v, OpSize, &pw };

	int i1 = p1.get_future().get();
	int i2 = p2.get_future().get();
	int i3 = pw.get_future().get();
	
	std::cout << "readers: "<< i1 << " " << i2 << '\n';
	std::cout << "expected: " <<  i3 * LoopSize * OpSize << '\n';

	r0.join();
	r1.join();
	w.join();
}
