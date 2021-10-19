
#pragma once

#include "concurrentqueue.h"
#include <array>
#include <functional>
#include <coroutine>

namespace jobs {
	using counter = std::atomic<int>;
	using job = void (*)(void*);

	struct job_decl {
		job call;
		void* data;
		const char* name;
	};

	struct queued_job {
		job_decl decl;
		counter* cnt;
	};

	class context {
	public:
		context() = default;
		~context();

		void run_blocking();

	private:
		std::vector<std::thread> _threads;
	};




	void run_all(const job_decl* decls, size_t num, counter* cnt);
	template<size_t N>
	void run_all(const std::array<job_decl, N>& decls, counter* cnt) {
		run_all(decls.data(), decls.size(), cnt);
	}

	/// Used to synchronize jobs.
	void wait_for_counter(counter* cnt);
	void _wait_for_counter_no_fiber(counter* cnt);

	std::string read_file(std::string path);
}