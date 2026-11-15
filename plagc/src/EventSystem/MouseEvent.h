#pragma once
#include "Event.h"

// ==========================================
	// MOUSE EVENTS
	// ==========================================

namespace plagc
{
	class MouseButtonPressedEvent final : public Event
	{
	public:
		MouseButtonPressedEvent(int button, int mods) noexcept
			: Button(button), Mods(mods) {
		}

		static constexpr EventType StaticType =
			EventType::MouseButtonPressed;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int Button, Mods;
	};


	class MouseButtonReleasedEvent final : public Event
	{
	public:
		MouseButtonReleasedEvent(int button, int mods) noexcept
			: Button(button), Mods(mods) {
		}

		static constexpr EventType StaticType =
			EventType::MouseButtonReleased;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int Button, Mods;
	};


	class MouseMovedEvent final : public Event
	{
	public:
		MouseMovedEvent(double xpos, double ypos) noexcept
			: XPos(xpos), YPos(ypos) {
		}

		static constexpr EventType StaticType = EventType::MouseMoved;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		double XPos, YPos;
	};


	class MouseEnteredEvent final : public Event
	{
	public:
		MouseEnteredEvent() noexcept = default;

		static constexpr EventType StaticType = EventType::MouseEntered;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class MouseEscapedEvent final : public Event
	{
	public:
		MouseEscapedEvent() noexcept = default;

		static constexpr EventType StaticType = EventType::MouseEscaped;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class MouseWheelScrolledEvent final : public Event
	{
	public:
		MouseWheelScrolledEvent(double xoffset, double yoffset) noexcept
			: XOffset(xoffset), YOffset(yoffset) {
		}

		static constexpr EventType StaticType =
			EventType::MouseWheelScrolled;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		double XOffset, YOffset;
	};
}
