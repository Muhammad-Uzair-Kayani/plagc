#pragma once
#include "Event.h"

// ==========================================
// KEYBOARD EVENTS
// ==========================================

namespace plagc
{
	class KeyPressedEvent final : public Event
	{
	public:
		KeyPressedEvent(int key, int scancode, int mods) noexcept
			: Key(key), ScanCode(scancode), Mods(mods) {
		}

		static constexpr EventType StaticType = EventType::KeyPressed;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int Key, ScanCode, Mods;
	};


	class KeyReleasedEvent final : public Event
	{
	public:
		KeyReleasedEvent(int key, int scancode, int mods) noexcept
			: Key(key), ScanCode(scancode), Mods(mods) {
		}

		static constexpr EventType StaticType = EventType::KeyReleased;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int Key, ScanCode, Mods;
	};


	class KeyRepeatedEvent final : public Event
	{
	public:
		KeyRepeatedEvent(int key, int scancode, int mods) noexcept
			: Key(key), ScanCode(scancode), Mods(mods) {
		}

		static constexpr EventType StaticType = EventType::KeyRepeated;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		int Key, ScanCode, Mods;
	};


	class CharKeyPointEvent final : public Event
	{
	public:
		explicit CharKeyPointEvent(unsigned int codepoint) noexcept
			: Codepoint(codepoint) {
		}

		static constexpr EventType StaticType = EventType::CharKeyPoint;

		constexpr EventType GetType() const noexcept override
		{
			return StaticType;
		}

		unsigned int Codepoint;
	};
}
