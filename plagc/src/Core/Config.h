#pragma once

enum class Platform
{
	None = 0,
	Windows,
	Linux,
	Android,
	MacOS
};

enum class RenderingAPI
{
	None = 0,
	DirectX,
	Vulkan,
	OpenGL,
	Metal
};

#ifdef _WIN32

#define SET_PLATFROM() ::Platform::Windows
#define SET_API() ::RenderingAPI::DirectX

#elif LINUX

#define SET_PLATFROM() ::Platform::Linux
#define SET_API() ::RenderingAPI::Vulkan

#elif ANDROID

#define SET_PLATFROM() ::Platform::Android
#define SET_API() ::RenderingAPI::OpenGL

#elif MAC

#define SET_PLATFROM() ::Platform::MacOS
#define SET_API() ::RenderingAPI::Metal

#endif // _WIN32

struct Config
{
	static Config& Get()
	{
		static Config cfg;
		return cfg;
	}

	Platform PLATFROM;
	RenderingAPI RENDERING_API;

private:
	Config()
	{
		PLATFROM = SET_PLATFROM();
		RENDERING_API = SET_API();
	}
};


#define GET_PLATFROM() ::Config::Get().PLATFROM
#define GET_RENDERING_API() ::Config::Get().RENDERING_API
