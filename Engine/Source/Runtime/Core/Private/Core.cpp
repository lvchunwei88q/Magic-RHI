#include "Core.h"

namespace Core
{
	// Engine Version
	constexpr const char* VERSION_STRING = MAGICTOON_VERSION;
	constexpr int VERSION[] = {
		MAGICTOON_VERSION_MAJOR,
		MAGICTOON_VERSION_FEATURE,
		MAGICTOON_VERSION_UPGRADE,
		MAGICTOON_VERSION_BUGFIX
	};

	Core::Core()
	{
	}

	Core::~Core()
	{
	}

	const char* Core::GetVersion() { return VERSION_STRING; };
}