#include <iostream>
#include <coroutine>
#include <cstddef>

class Context {
private:
	class Promise {
	public:
		Context get_return_object()
		{
			return {std::coroutine_handle<Promise>::from_promise(*this)};
		}

		std::suspend_never initial_suspend() {return {};}
		std::suspend_always final_suspend() noexcept {return {};}

		void unhandled_exception() {std::terminate();}
		void return_value(unsigned long v) {_v = v;}

		unsigned long value() {return _v;}
	private:
		unsigned long _v = 0;
	};

public:
	using promise_type = Promise;

	unsigned long value()
	{
		return _h.promise().value();
	}

	~Context()
	{
		if (_h) {
			_h.destroy();
		}
	}

	Context(Context const&) = delete;
	Context& operator=(Context cont&) = delete;

	Context(Context&&) = delete;
	Context& operator=(Context&&) = delete;

private:
	Context(std::coroutine_handle<promise_type> h): _h{h} {}

	std::coroutine_handle<promise_type> _h;
};


Context recurse(unsigned long *cnt)
{
	constexpr std::size_t sz = 1024 / sizeof(unsigned long);
	unsigned long a[sz];
	for (std::size_t i = 0; i < sz; ++i) {
		a[i] = 1 + *cnt;
	}

	if (a[sz - 1] >= 1000000) {
		co_return a[sz - 1];
	}

	Context ctx = recurse(a);
	co_return ctx.value();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char const* const argv[])
{
	unsigned long cnt = 0;
	Context ctx = recurse(&cnt);
	std::cout << ctx.value() << " recursive calls" << std::endl;
}
