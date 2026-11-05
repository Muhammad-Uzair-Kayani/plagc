#pragma once

namespace plagc
{
	enum class EventType
	{
		None = 0,
		WindowClose
	};

	class Event
	{
	public:

		virtual ~Event() {}

		virtual const EventType GetType() { return EventType::None; }

	private:

	};

	class WindowCloseEvent : public Event
	{
	public:

		WindowCloseEvent() {}

		const EventType GetType() override { return m_Type; }

	private:

		EventType m_Type = EventType::WindowClose;

	};

}