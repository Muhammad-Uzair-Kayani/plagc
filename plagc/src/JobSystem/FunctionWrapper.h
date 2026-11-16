#pragma once
#include <type_traits>
#include <concepts>


namespace plagc
{

	template<typename T, typename Self>
	concept JobCallable = !std::same_as<std::remove_cvref_t<T>, Self>&&
		std::invocable<std::decay_t<T>&>&&
		std::constructible_from<std::decay_t<T>, T>;

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

			Impl(F&& _f) : fn(std::forward<F>(_f)) {}

			void call() override
			{
				fn();
			}

		private:

			F fn;

		};

	public:


		template<JobCallable<FuncWrapper> F>
		FuncWrapper(F&& f)
			: pImpl(MakeUnique<Impl<std::decay_t<F>>>(std::forward<F>(f)))
		{
		}

		template<typename F>
		FuncWrapper(FuncWrapper&& other) noexcept : pImpl(std::forward<F>(other.pImpl)) {}

		FuncWrapper& operator=(FuncWrapper&& other) noexcept
		{
			pImpl = std::move(other.pImpl);
			return *this;
		}

		FuncWrapper() = default;

		template<typename F>
		FuncWrapper(const F& f) = delete;
		FuncWrapper(const FuncWrapper& other) = delete;

		void operator() ()
		{
			pImpl->call();
		}

	private:

		Owner<BaseImpl> pImpl;

	};
}
