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

		Event() noexcept {}
		virtual ~Event() noexcept = default;

		virtual constexpr EventType GetType() const { return EventType::None; }

	private:

	};

	class WindowCloseEvent : public Event
	{
	public:

		WindowCloseEvent() noexcept{}

		constexpr EventType GetType() const override{ return m_Type; }

	private:

		EventType m_Type = EventType::WindowClose;

	};

}