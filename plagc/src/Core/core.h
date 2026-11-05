#pragma once

#include <memory>
#include <utility>

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Owner = std::unique_ptr<T>;

template<typename T, typename... Args>
Ref<T> MakeRef(Args&&... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
Owner<T> MakeUnique(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}