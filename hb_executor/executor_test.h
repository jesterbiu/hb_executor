#pragma once
#include <iostream>
#include "executor.h"
void executor_test() {
	std::chrono::seconds periods[] = {
		std::chrono::seconds{ 1 }
		, std::chrono::seconds{ 4 }
		, std::chrono::seconds{ 8 }
	};

	for (auto s : periods) {
		hungbiu::hb_executor etor(4);
		etor.execute([](auto& handle) {std::cout << "wake up sunshine\n"; });
		etor.execute([](auto& handle) {std::cout << "wake up sunshine\n"; });
		etor.execute([](auto& handle) {std::cout << "wake up sunshine\n"; });
		std::this_thread::sleep_for(s);
		etor.done();
	}
}