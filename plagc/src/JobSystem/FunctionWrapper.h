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
			: pImpl(MakeUnique<Impl<std::decay_t<F>>>(
				std::forward<F>(f)))
		{
		}

		template<JobCallable<FuncWrapper> F>
		FuncWrapper& operator=(F&& f)
		{
			pImpl = MakeUnique<Impl<std::decay_t<F>>>(std::forward<F>(f));
			return *this;
		}

		FuncWrapper(FuncWrapper&& other) noexcept = default;

		FuncWrapper& operator=(FuncWrapper&& other) noexcept = default;

		FuncWrapper() = default;

		FuncWrapper(const FuncWrapper&) = delete;

		FuncWrapper& operator=(const FuncWrapper&) = delete;

		
		void operator() ()
		{
			pImpl->call();
		}

	private:

		Owner<BaseImpl> pImpl;

	};
}
