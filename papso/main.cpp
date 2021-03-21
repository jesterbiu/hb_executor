#include "papso2_test.h"

int main(int argc, const char* argv[]) {
	size_t fork_count = 0;
	if (argc > 1) {
		fork_count = std::stoul(std::string{ argv[1] });
	}
	else {
		return -1;
	}
	speedup_benchmark(fork_count);
}


