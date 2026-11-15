#pragma once
class FuncWrapper
{

private:

	class BaseImpl
	{
	public:

		virtual ~BaseImpl() = default;

		virtual void call() = 0;

	private:

	};

	template<typename F>
	class Impl : public BaseImpl
	{
	public:

		Impl(F&& _f) : fn(std::move(_f)) {}

		void call() override
		{
			fn();
		}

	private:

		F fn;

	};

public:

	template<typename F>
	FucnWrapper(F&& f) : pImpl(MakeUnique<Impl<F>>(std::move(f))) {}

	FucnWrapper(FucnWrapper&& other) noexcept : pImpl(std::move(other.pImpl)) {}

	FucnWrapper& operator=(FucnWrapper&& other) noexcept
	{
		pImpl = std::move(other.pImpl);
		return *this;
	}

	FucnWrapper() = default;

	template<typename F>
	FucnWrapper(const F& f) = delete;
	FucnWrapper(const FucnWrapper& other) = delete;

	void operator() ()
	{
		pImpl->call();
	}

private:

	Owner<BaseImpl> pImpl;

};