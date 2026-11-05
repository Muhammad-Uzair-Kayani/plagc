#pragma once

// plagc precompiled header: stable, widely used standard-library facilities.
// Keep Ogre-Next, Jolt, GLFW, EnTT, Windows.h, and other third-party/platform
// headers local to the modules using them, rather than forcing them on all TUs.

// Fundamental types, diagnostics, metaprogramming
#include <any>
#include <bit>
#include <bitset>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <new>
#include <source_location>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <typeindex>
#include <utility>

// Containers, views and algorithms
#include <algorithm>
#include <array>
#include <deque>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <ranges>
#include <set>
#include <span>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Numbers and mathematics
#include <cmath>
#include <numbers>
#include <numeric>
#include <random>

// Ownership, polymorphic allocations, optional/result values
#include <expected>
#include <memory>
#include <memory_resource>
#include <optional>
#include <tuple>
#include <variant>

// Strings, serialization building blocks, streams, file paths
#include <charconv>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

// Concurrency, tasks and time
#include <atomic>
#include <barrier>
#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <future>
#include <latch>
#include <mutex>
#include <semaphore>
#include <shared_mutex>
#include <stop_token>
#include <thread>

// C interoperability and diagnostics
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>


//Engine Side Headers
#include "Core/Core.h"
#include "Core/log.h"