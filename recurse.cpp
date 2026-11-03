#include <iostream>
#include <cstring>
#include <cstddef>

unsigned long recurse(unsigned long *cnt)
{
	constexpr std::size_t sz = 1024 / sizeof(unsigned long);
	unsigned long a[sz];
	for (std::size_t i = 0; i < sz; ++i) {
		a[i] = 1 + *cnt;
	}

	if (a[sz - 1] >= 1000000) {
		return a[sz - 1];
	}

	return recurse(a);
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char const* const argv[])
{
	unsigned long cnt = 0;
	unsigned long rs = recurse(&cnt);
	std::cout << rs << " recursive calls" << std::endl;

	return 0;
}
