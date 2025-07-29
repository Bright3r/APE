#pragma once

#include <chrono>
#include <thread>
#include <type_traits>

namespace APE {
namespace Timing {

// Type Aliases
using seconds = std::chrono::duration<double>;
using millis = std::chrono::duration<double, std::milli>;

// Utility Functions
template <typename F, typename Rep, typename Period>
decltype(auto) timeFunctionCall(F&& f, std::chrono::duration<Rep, Period>& execution_time)
{
	// Time function execution time
	auto start_time = std::chrono::high_resolution_clock::now();

	if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
		// Return execution time
		f();
		auto end_time = std::chrono::high_resolution_clock::now();
		execution_time = end_time - start_time;
	}
	else {
		// Return execution time and function result
		auto res = f();
		auto end_time = std::chrono::high_resolution_clock::now();
		execution_time = end_time - start_time;
		return res;
	}
}

template <typename Rep, typename Period>
void spinWait(const std::chrono::duration<Rep, Period>& wait_time)
{
	using namespace std::chrono;

	if (wait_time <= milliseconds(0))
		return;

	auto start_time = high_resolution_clock::now();
	while ((high_resolution_clock::now() - start_time) < wait_time) { }
}

template <typename Rep,  typename Period>
void waitFor(const std::chrono::duration<Rep, Period>& wait_time)
{
	using namespace std::chrono;

	if (wait_time <= duration<Rep, Period>::zero())
		return;

	auto start_time = high_resolution_clock::now();

	// Wait until 1ms before end of wait
	if (wait_time > milliseconds(1)) {
		auto coarse_delay_time = wait_time - milliseconds(1);
		std::this_thread::sleep_for(coarse_delay_time);
	}

	// Spin wait for final 1ms of wait
	auto time_elapsed = high_resolution_clock::now() - start_time;
	auto duration_remaining = wait_time - time_elapsed;
	spinWait(duration_remaining);
}

};

};
