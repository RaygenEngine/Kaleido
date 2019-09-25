#pragma once

#include "system/reflection/Reflection.h"
#include "asset/AssetPod.h"

struct StringPod : public AssetPod
{
	STATIC_REFLECTOR(StringPod)
	{
		S_REFLECT_VAR(data, PropertyFlags::Multiline);
	}

	static bool Load(StringPod* pod, const fs::path& path);

	std::string data;
};