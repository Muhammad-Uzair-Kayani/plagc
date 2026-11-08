#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace plagc
{
	enum class EventType
	{
		None = 0,

		// Window
		WindowMoved,
		WindowResized,
		WindowClosed,
		WindowCloseRequest,
		WindowRefreshed,
		WindowFocusGained,
		WindowFocusLost,
		WindowIconified,
		WindowMinimized,
		WindowMaximized,
		WindowWindowed,
		WindowFramebufferResized,
		WindowContentScale,

		// Keyboard
		KeyPressed,
		KeyReleased,
		KeyRepeated,
		CharKeyPoint,

		// Mouse
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseEntered,
		MouseEscaped,
		MouseWheelScrolled,

		// File Drop
		FileDropped
	};

	enum class EventStatus
	{
		Continue = 0,
		Consume
	};

	class Event
	{
	public:
		virtual ~Event() noexcept = default;

		virtual EventType GetType() const noexcept = 0;

		const EventStatus GetStatus() const noexcept
		{
			return m_Status;
		}

		void SetStatus(EventStatus status) noexcept
		{
			m_Status = status;
		}

	private:

		EventStatus m_Status = EventStatus::Continue;

	};

}
