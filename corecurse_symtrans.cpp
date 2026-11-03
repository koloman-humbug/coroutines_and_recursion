#include <iostream>
#include <coroutine>
#include <cstddef>

// symmetric transfer

class Context {
private:
	class FinalAwaiter
	{
	public:
		// The FinalAwaiter gets the caller's coroutine handle
		FinalAwaiter(std::coroutine_handle<> caller_h): _caller_h{caller_h} {}

		bool await_ready() noexcept
		{
			return false;
		}

		// Return the caller's handle so the caller is resumed
		// (symmetric transfer). The outermost coroutine has no caller;
		// noop_coroutine() makes resume() in run() return.
		std::coroutine_handle<> await_suspend([[maybe_unused]] std::coroutine_handle<> my_h) noexcept
		{
			if (_caller_h) {
				return _caller_h;
			}

			return std::noop_coroutine();
		}

		void await_resume() noexcept
		{
		}

	private:
		std::coroutine_handle<> _caller_h{};
	};

	class Promise {
	public:
		Context get_return_object()
		{
			return {std::coroutine_handle<Promise>::from_promise(*this)};
		}

		// Start suspended so the body doesn't run inside the caller's
		// call.
		// It is started by Context::await_suspend, after the caller
		// has been recorded.
		std::suspend_always initial_suspend() {return {};}

		// FinalAwaiter is invoked when the coroutine finishes
		FinalAwaiter final_suspend() noexcept
		{
			return {_caller_h};
		}

		void unhandled_exception() {std::terminate();}
		void return_value(unsigned long v) {_v = v;}

		unsigned long value()
		{
			return _v;
		}

		void caller(std::coroutine_handle<> caller_h)
		{
			_caller_h = caller_h;
		}

	private:
		unsigned long _v = 0;
		std::coroutine_handle<> _caller_h;
	};

public:
	using promise_type = Promise;

	unsigned long run()
	{
		// resume: main() calls run() and via symmetric transfer
		// the whole recursion is executed
		_my_h();
		// and return the result co_returned eventually
		return _my_h.promise().value();
	}

	~Context()
	{
		if (_my_h) {
			_my_h.destroy();
		}
	}

	Context(Context const&) = delete;
	Context& operator=(Context const&) = delete;

	Context(Context&&) = delete;
	Context& operator=(Context&&) = delete;

	bool await_ready()
	{
		return false;
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_h)
	{
		// store caller's coroutine handle which will be
		// resumed by our FinalAwaiter
		_my_h.promise().caller(caller_h);

		// Return the child's handle: the caller stays suspended and
		// the child is resumed as a tail call.
		return _my_h;
	}

	unsigned long await_resume()
	{
		return _my_h.promise().value();
	}

private:
	Context(std::coroutine_handle<promise_type> h): _my_h{h} {}

	std::coroutine_handle<promise_type> _my_h;
};


Context recurse(unsigned long *cnt)
{
	constexpr std::size_t sz = 1024 / sizeof(unsigned long);
	unsigned long a[sz];
	for (std::size_t i = 0; i < sz; ++i) {
		a[i] = 1 + *cnt;
	}

	if (a[sz - 1] >= 1000000) {
		// this terminates the recursion
		co_return a[sz - 1];
	}

	// Context's awaiter interface is invoked here
	unsigned long level = co_await recurse(a);
	co_return level;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char const* const argv[])
{
	unsigned long cnt = 0;
	Context ctx = recurse(&cnt);

	// start the recursion
	unsigned long level = ctx.run();
	std::cout << level << " recursive calls" << std::endl;
}
