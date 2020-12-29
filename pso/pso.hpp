#pragma once
#include <vector>
#include <random>
#include <array>
#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <iterator>
#include <type_traits>
#define CRAND 0
#if CRAND
#include <ctime>
#include <cstdlib>
#endif

#define DEBUG 0
#if DEBUG
#include <cstdio>
#endif
namespace hungbiu
{

#define ALIGN_REQ alignas(64)
	template <size_t D>
	struct ALIGN_REQ particle
	{
		using array_t = std::array<double, D>;
		using lock_t = std::mutex;
		std::mutex mtx;
		std::atomic<double> best_fitness{ std::numeric_limits<double>::max() };
		double fitness{ std::numeric_limits<double>::max() };
		array_t velocity, position, personal_best;
		// Neighbor
	};

	struct uniform_rand_real_num
	{
		using RealType = double;
		static constexpr auto Precision = 10;
		std::default_random_engine e;
		RealType operator()()
		{
			return std::generate_canonical<RealType, Precision>(e);
		}
		RealType operator()(RealType lo, RealType hi)
		{
			return std::uniform_real_distribution<RealType>(lo, hi)(e);
		}
	};
	
	struct worker_handle
	{
		template <typename F>
		void submit(F&& f);
		void done();
	};
	
	template <
		typename Func,
		size_t Dimensions,
		size_t SwarmSize = 40u,
		size_t NeighborSize = 16u,
		size_t IterationSize = 10000u>
	struct pso
	{
		static constexpr double Chi = 0.7298;
		static constexpr double Phi = 1.49618;

		Func obj_func;
		const double precision;
		uniform_rand_real_num rng;
		
		std::mutex mtx;
		std::atomic<bool> optimization_success{ false };
		std::atomic<double> global_best{ std::numeric_limits<double>::max() };
		std::vector<double> global_best_pos;
		
		using particle_t = particle<Dimensions>;
		std::vector<particle_t> particles;
		std::vector<double> Xmin, Xmax;

		struct subpso
		{
			pso* parent_pso;
			size_t first, last;
			uniform_rand_real_num rng;

			subpso(pso* parent, size_t fst, size_t lst) :
				parent_pso(parent), first(fst), last(lst), rng() 
			{}
			subpso(const subpso& oth) :
				parent_pso(oth.pso_parent), first(oth.first), 
				last(oth.last), rng() 
			{}
			subpso& operator=(const subpso& rhs)
			{
				if (this != &rhs) {
					parent_pso = rhs.pso_parent;
					first = rhs.first;
					last = rhs.last;
				}
				return *this;
			}

			size_t select_particle(size_t i)
			{
				return rng() * SwarmSize;				
			}

			bool evaluate(particle_t& p)
			{
				p.fitness = parent_pso->obj_func(p.position.data(), Dimensions);
				
				// Update oneself's fitness
				if (p.fitness <= p.best_fitness.load()) {
					std::lock_guard lk{ p.mtx };
					p.best_fitness.store(p.fitness);
					for (size_t i = 0; i < Dimensions; ++i) p.personal_best[i] = p.position[i];
				}

				// Update global fitness
				if (p.fitness <= parent_pso->global_best.load()) {
					parent_pso->update_global(p.fitness, p.position);
					
					// Check convergence
					if (p.fitness <= parent_pso->precision) {
						parent_pso->optimization_success.store(true);
						return true;
					}
				}
				return false;
			}

			// vel[i] = -0.5 * vel[i]
			// (vel[i] < Xmin[i] or vel[i] > Xmax[i]), SPSO 2011
			void clamp_position(particle_t& p)
			{
				auto& position = p.position;
				auto& velocity = p.velocity;
				for (size_t i = 0; i < Dimensions; ++i) {
					if (position[i] < parent_pso->Xmin[i]) {
						position[i] = parent_pso->Xmin[i];
						velocity[i] *= -0.5;
					}
					else if (position[i] > parent_pso->Xmax[i]) {
						position[i] = parent_pso->Xmax[i];
						velocity[i] *= -0.5;
					}
					else continue;
				}
			}

			// ring topology
			void update_best_neighbor(size_t pidx, std::array<double, Dimensions>& n_best_pos)
			{
				// Initliaze with particles[pidx] itself
				auto& particles = parent_pso->particles;
				double best_fitness = particles[pidx].best_fitness;
				size_t best_idx = pidx;
				for (size_t i = 0; i < Dimensions; ++i) n_best_pos[i] = particles[pidx].position[i];

				// Find its best neighbor in ring topology
				int start = pidx - NeighborSize / 2; // ...
				int end = pidx + NeighborSize / 2;	 // Use signed integer!
				for (; start <= end; ++start) {
					auto neighbor_idx = start;
					if (neighbor_idx < 0)  neighbor_idx += NeighborSize;
					else if (neighbor_idx == pidx) continue;
					else if (neighbor_idx >= NeighborSize) neighbor_idx %= NeighborSize;
					else ;
					
					particle_t& neighbor = particles[neighbor_idx];
					if (neighbor.best_fitness.load() < best_fitness) {
						std::lock_guard lk{ neighbor.mtx };
						best_idx = neighbor_idx;
						best_fitness = neighbor.best_fitness.load();
						for (size_t i = 0; i < Dimensions; ++i) n_best_pos[i] = neighbor.personal_best[i];
					}
				}
			}

			void update_velocity(particle_t& p, size_t idx)
			{
				// Produce r1, r2
				std::array<double, Dimensions> r1, r2;
				for (size_t i = 0; i < Dimensions; ++i) {
					if constexpr (CRAND) {
						r1[i] = rand() / std::numeric_limits<double>::max();
						r2[i] = rand() / std::numeric_limits<double>::max();
					}
					else {
						r1[i] = rng.operator()();
						r2[i] = rng.operator()();
					}
				}

				// Compute new velocity			
				std::array<double, Dimensions> neighbor_best_pos;
				update_best_neighbor(idx, neighbor_best_pos);

				// ---------------------------------------------------------------------
				std::lock_guard lk{ p.mtx };
				for (size_t i = 0; i < Dimensions; ++i) {
					p.velocity[i] += Chi * p.velocity[i]
									+ Phi * r1[i] * (p.personal_best[i] - p.position[i])
									+ Phi * r2[i] * (neighbor_best_pos[i] - p.position[i]);
				}								
			}

			void update_position(particle_t& p)
			{
				std::lock_guard lk{ p.mtx };
				for (size_t i = 0; i < Dimensions; ++i) {
					p.position[i] += p.velocity[i];
					clamp_position(p);
				}
			}

			// random async
//			void operator()()
//			{
//#if DEBUG
//
//				FILE* fp;
//				try {
//					if (fopen_s(&fp, "output.csv", "w")) throw "fopen_s failed\n";
//				}
//				catch (const char* str) {
//					printf_s("%s\n", str);
//					return;
//				}
//				
//				fprintf_s(fp, "ITERATION,PARTICLE,");
//				for (size_t i = 0; i < Dimensions; ++i) fprintf_s(fp, "V_%llu,", i);
//				for (size_t i = 0; i < Dimensions; ++i) fprintf_s(fp, "X_%llu,", i);
//				fprintf_s(fp, "FITNESS\n");
//#endif
//				auto& particles = parent_pso->particles;
//				for (size_t i = 0; i < IterationSize; ++i) { // Iteration
//					for (size_t j = first; j < last; ++j) { // Swarm			
//						auto idx = select_particle(j);
//						auto& p = particles[idx];
//						const auto target = evaluate(p);
//#if DEBUG
//						// Print particle
//						fprintf_s(fp, "%llu,%llu,", i, j);
//						for (const auto v : p.velocity) fprintf_s(fp, "%.4lf,", v);
//						for (const auto x : p.position) fprintf_s(fp, "%.4lf,", x);
//						fprintf_s(fp, "%lf\n", p.fitness);
//#endif
//						if (target) return;
//						update_velocity(p, idx);
//						update_position(p);
//					} // end of swarm loop
//				} // end of iteration
//			}

			void operator()()
			{
#if DEBUG

				FILE* fp;
				try {
					if (fopen_s(&fp, "output.csv", "w")) throw "fopen_s failed\n";
				}
				catch (const char* str) {
					printf_s("%s\n", str);
					return;
				}

				fprintf_s(fp, "ITERATION,PARTICLE,");
				for (size_t i = 0; i < Dimensions; ++i) fprintf_s(fp, "V_%llu,", i);
				for (size_t i = 0; i < Dimensions; ++i) fprintf_s(fp, "X_%llu,", i);
				fprintf_s(fp, "FITNESS\n");
#endif
				auto& particles = parent_pso->particles;
				for (size_t i = 0; i < IterationSize; ++i) { // Iteration
					for (size_t j = first; j < last; ++j) { // Swarm		
						auto& p = particles[j];
						const auto target = evaluate(p);
#if DEBUG
						// Print particle
						fprintf_s(fp, "%llu,%llu,", i, j);
						for (const auto v : p.velocity) fprintf_s(fp, "%.4lf,", v);
						for (const auto x : p.position) fprintf_s(fp, "%.4lf,", x);
						fprintf_s(fp, "%lf\n", p.fitness);
#endif
						if (target) return;						
					} // end of swarm loop		
					for (size_t j = first; j < last; ++j) {
						update_velocity(particles[j], j);
						update_position(particles[j]);
					}
				} // end of iteration
			}
		};	

		//void operator()(worker_handle& h)
		//{
		//	// Init min and max velocity
		//	std::for_each(particles.begin(), particles.end(), 
		//		[](auto& p) { // Init velocity and position 
		//		});
		//	
		//	// Dispatch particles
		//	size_t particles_per_worker = SwarmSize / std::thread::hardware_concurrency();
		//	if (!particles_per_worker) particles_per_worker = 1;
		//	for (size_t i = 0; i <= SwarmSize; i += particles_per_worker) {				
		//		h.submit(subpso{ *this, i, i + particles_per_worker });
		//	}
		//}

		template <typename F, typename InputIt>
		pso(F&& f, double precision, 
			const InputIt xmin_beg, const InputIt xmin_end,
			const InputIt xmax_beg, const InputIt xmax_end) :
			obj_func(std::forward<std::decay_t<F>>(f)),
			precision(precision), rng(),
			global_best_pos(Dimensions),
			particles(SwarmSize),
			Xmin(xmin_beg, xmin_end), Xmax(xmax_beg, xmax_end) 
		{}
		pso(const pso& oth) = delete;
		pso& operator=(const pso& rhs) = delete;
		~pso() = default;

		// pos[i] = U( Xmin[i], Xmax[i] ), SPSO
		void initialize_position(particle_t& p)
		{
			for (size_t i = 0; i < Dimensions; ++i) {
				if constexpr (CRAND) {
					p.position[i] = Xmin[i] + (rand() / std::numeric_limits<double>::max()) * (Xmax[i] - Xmin[i]);
				}
				else {
					p.position[i] = Xmin[i] + rng() * (Xmax[i] - Xmin[i]);
				}				
			}
		}
		
		// vel[i] = U( Xmin[i] - pos[i], Xmax[i] - pos[i] ), SPSO 2011
		void initialize_velocity(particle_t& p)
		{
			for (size_t i = 0; i < Dimensions; ++i) {
				auto lo_diff = Xmin[i] - p.position[i];
				auto hi_diff = Xmax[i] - p.position[i];
				if constexpr (CRAND) {
					p.velocity[i] = lo_diff + (rand() / std::numeric_limits<double>::max()) * (hi_diff - lo_diff);
				}
				else {
					p.velocity[i] = lo_diff + rng() * (hi_diff - lo_diff);
				}				
			}
		}		

		void update_global(double fitness, const typename particle_t::array_t& pos)
		{
			std::lock_guard lk{ mtx };			
			if (fitness > global_best) return;			
			global_best = fitness;
			global_best_pos.assign(std::cbegin(pos), std::cend(pos));
		}

		void operator()()
		{
			if constexpr (CRAND)	std::srand(time(NULL));
			std::for_each(particles.begin(), particles.end(), [this](auto& p) {
				this->initialize_position(p);
				this->initialize_velocity(p);
				});
			subpso{ this, 0u, SwarmSize }();
		}
	};
}
