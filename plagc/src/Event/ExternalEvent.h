#pragma once
#include "Event.h"

namespace plagc
{
	// ==========================================
	// FILE DROP EVENT
	// ==========================================

	class FileDroppedEvent final : public Event
	{
	public:
		explicit FileDroppedEvent(std::vector<std::string> paths)
			: Paths(std::move(paths)) {
		}

		static constexpr EventType StaticType = EventType::FileDropped;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		std::vector<std::string> Paths;
	};

}