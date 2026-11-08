#pragma once
#include "Event.h"

// ==========================================
//             WINDOW EVENTS
// ==========================================

namespace plagc
{
	class WindowMovedEvent final : public Event
	{
	public:
		WindowMovedEvent(int xpos, int ypos) noexcept
			: XPos(xpos), YPos(ypos) {
		}

		static constexpr EventType StaticType = EventType::WindowMoved;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int XPos, YPos;
	};


	class WindowResizedEvent final : public Event
	{
	public:
		WindowResizedEvent(int width, int height) noexcept
			: Width(width), Height(height) {
		}

		static constexpr EventType StaticType = EventType::WindowResized;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int Width, Height;
	};


	class WindowClosedEvent final : public Event
	{
	public:
		WindowClosedEvent() noexcept = default;

		static constexpr EventType StaticType = EventType::WindowClosed;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};

	class WindowCloseRequestEvent final : public Event
	{
	public:
		WindowCloseRequestEvent() noexcept = default;

		static constexpr EventType StaticType = EventType::WindowCloseRequest;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowRefreshedEvent final : public Event
	{
	public:
		WindowRefreshedEvent() noexcept = default;

		static constexpr EventType StaticType = EventType::WindowRefreshed;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowFocusGainedEvent final : public Event
	{
	public:
		WindowFocusGainedEvent() noexcept = default;

		static constexpr EventType StaticType =
			EventType::WindowFocusGained;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowFocusLostEvent final : public Event
	{
	public:
		WindowFocusLostEvent() noexcept = default;

		static constexpr EventType StaticType =
			EventType::WindowFocusLost;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowIconifiedEvent final : public Event
	{
	public:
		WindowIconifiedEvent() noexcept = default;

		static constexpr EventType StaticType =
			EventType::WindowIconified;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowMinimizedEvent final : public Event
	{
	public:
		WindowMinimizedEvent() noexcept = default;

		static constexpr EventType StaticType =
			EventType::WindowMinimized;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowMaximizedEvent final : public Event
	{
	public:
		WindowMaximizedEvent() noexcept = default;

		static constexpr EventType StaticType =
			EventType::WindowMaximized;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowWindowedEvent final : public Event
	{
	public:
		WindowWindowedEvent() noexcept = default;

		static constexpr EventType StaticType =
			EventType::WindowWindowed;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}
	};


	class WindowFramebufferResizedEvent final : public Event
	{
	public:
		WindowFramebufferResizedEvent(int width, int height) noexcept
			: Width(width), Height(height) {
		}

		static constexpr EventType StaticType =
			EventType::WindowFramebufferResized;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int Width, Height;
	};


	class WindowContentScaleEvent final : public Event
	{
	public:
		WindowContentScaleEvent(float xscale, float yscale) noexcept
			: XScale(xscale), YScale(yscale) {
		}

		static constexpr EventType StaticType =
			EventType::WindowContentScale;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		float XScale, YScale;
	};
}
