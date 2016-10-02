#include <chrono>

template<typename F>
double functionTimer(F f)
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	f();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::chrono::nanoseconds result = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	return result.count();
}
