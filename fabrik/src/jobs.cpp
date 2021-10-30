
#include "pch.h"
#include "jobs.h"
#include "utils.h"
#include "defer.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "tracy/Tracy.hpp"

namespace jobs {
	namespace {
		thread_local void* thread_main_fiber;
		thread_local void* dead_fiber;

		thread_local std::vector<std::pair<void*, counter*>> waiting_fibers;
		std::atomic<int> total_num_fibers;

		moodycamel::ConcurrentQueue<queued_job> job_queue;

		void run_queued_job(void* data) {
			auto qj = (queued_job*)data;

			qj->decl.call(qj->decl.data);
			--(*qj->cnt);


			dead_fiber = GetCurrentFiber();
			fabrik_assert(dead_fiber != nullptr);

			fabrik_assert(thread_main_fiber != nullptr);
			SwitchToFiber(thread_main_fiber);
		}

		void fiber_thread(threadpool_executor* ctx, const int thread_idx) {
			static thread_local std::string thread_name = std::format("fiber worker {}", thread_idx);
			debug::set_current_thread_name(thread_name.c_str());

			thread_main_fiber = ConvertThreadToFiber(nullptr);
			if (thread_main_fiber == nullptr) {
				panic(std::format("ConvertThreadToFiber failed: {}", GetLastError()));
			}

			moodycamel::ConsumerToken ctok{ job_queue };

			while (true) {
				// can resume?
				bool foo = false;
				for (int i = 0; i < waiting_fibers.size(); ++i) {
					if (*(waiting_fibers[i].second) == 0) {
						auto fiber_to_resume = waiting_fibers[i].first;
						fabrik_assert(fiber_to_resume != nullptr);

						std::swap(waiting_fibers[i], waiting_fibers[waiting_fibers.size() - 1]);
						waiting_fibers.pop_back();

						foo = true;
						SwitchToFiber(fiber_to_resume);

						if (dead_fiber != nullptr) {
							--total_num_fibers;
							DeleteFiber(dead_fiber);
							dead_fiber = nullptr;
						}
						break;
					}
				}

				if (foo)
					continue;

				// can spin new job?
				queued_job qj;
				if (!job_queue.try_dequeue(ctok, qj)) {
					if (thread_idx == 0)
						return;

					yield();
					continue;
				}

				const auto quit_thread = qj.decl.call == nullptr;
				if (quit_thread)
					break;

				++total_num_fibers;
				auto new_fiber = CreateFiber(0, run_queued_job, &qj);
				if (new_fiber == nullptr)
					panic("fiber null");

				SwitchToFiber(new_fiber);
			}

			if (dead_fiber != nullptr) {
				--total_num_fibers;
				DeleteFiber(dead_fiber);
				dead_fiber = nullptr;
			}
		}
	}

	void threadpool_executor::run_blocking() {
		const auto num_threads = std::thread::hardware_concurrency();

		for (size_t i = 0; i < num_threads - 1; ++i) {
			_threads.emplace_back(&fiber_thread, this, i + 1);

			uint32_t mask = 1 << (i + 1);
			::SetThreadAffinityMask(_threads[i].native_handle(), (DWORD_PTR)mask);
		}

		{
			uint32_t mask = 1 << 0;
			::SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)mask);
		}
		fiber_thread(this, 0);
	}

	threadpool_executor::~threadpool_executor() {
		for (int i = 0; i < _threads.size(); ++i) {
			job_queue.enqueue({ nullptr, nullptr });
		}

		for (auto& thread : _threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}
	}

	void run_all(const std::span<job_decl> decls, counter* cnt) {
		*cnt += (int)decls.size();

		std::vector<queued_job> qjs;
		qjs.reserve(decls.size());

		for (int i = 0; i < decls.size(); ++i) {
			qjs.emplace_back(decls[i], cnt);
		}
		job_queue.enqueue_bulk(qjs.data(), decls.size());
	}

	void wait_for_counter(counter* cnt) {
		fabrik_assert(thread_main_fiber != nullptr);

		auto current_fiber = GetCurrentFiber();
		fabrik_assert(current_fiber != nullptr);

		waiting_fibers.emplace_back(current_fiber, cnt);

		auto qj = (queued_job*)GetFiberData();
		SwitchToFiber(thread_main_fiber);

		fabrik_assert(*cnt == 0);
	}

	void _wait_for_counter_no_fiber(counter* cnt) {
		// Spinlock to victory
		while (*cnt > 0) {
			yield();
		}

		fabrik_assert(*cnt == 0);
	}

	void read_file_thread(std::string path, uint8_t** out_buffer, size_t* out_size, counter* cnt) {
		SetThreadDescription(GetCurrentThread(), L"fiber read file thread");

		auto file = CreateFileA(path.c_str(),
								GENERIC_READ,
								FILE_SHARE_READ,
								nullptr,
								OPEN_EXISTING,
								0,
								nullptr);
		defer{ CloseHandle(file); };

		LARGE_INTEGER size;
		if (!GetFileSizeEx(file, &size))
			panic("GetFileSize failed");

		if (size.HighPart > 0)
			panic("large files not implemented");

		auto buffer = new uint8_t[size.LowPart];

		DWORD read;
		if (!ReadFile(file, buffer, size.LowPart, &read, nullptr))
			panic("ReadFile failed");

		*out_buffer = buffer;
		*out_size = read;

		--(*cnt);
	}

	std::string read_file(std::string path) {
		uint8_t* buffer;
		size_t size;

#if 1
		{
			auto file = CreateFileA(path.c_str(),
									GENERIC_READ,
									FILE_SHARE_READ,
									nullptr,
									OPEN_EXISTING,
									0,
									nullptr);
			defer{ CloseHandle(file); };

			LARGE_INTEGER li;
			if (!GetFileSizeEx(file, &li))
				panic("GetFileSize failed");

			if (li.HighPart > 0)
				panic("large files not implemented");

			buffer = new uint8_t[li.LowPart];

			DWORD read;
			if (!ReadFile(file, buffer, li.LowPart, &read, nullptr))
				panic("ReadFile failed");

			size = read;
		}
#else
		{
			counter cnt2 = 1;
			std::thread thread{ read_file_thread, path, &buffer, &size, &cnt2 };
			thread.detach();
			wait_for_counter(&cnt2);
		}
#endif

		std::string result{ buffer, buffer + size };
		delete[] buffer;

		return result;
	}
}