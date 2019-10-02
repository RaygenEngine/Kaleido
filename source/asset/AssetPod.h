#pragma once

#include "core/reflection/TypeId.h"

#include <filesystem>

namespace fs = std::filesystem;

struct AssetPod
{
	TypeId type;

	template<typename T>
	bool IsOfType() const 
	{ 
		static_assert(std::is_base_of_v<AssetPod, T>, "This check would always fail. T is not a child pod type");
		return type == refl::GetId<T>();
	};

protected:
	// Do not ever delete generic asset pod pointer. There is no virtual destructor,
	// always use DeletePod(AssetPod*) or cast to underlying pod type
	~AssetPod() = default;
};

struct BasePodHandle
{
	size_t podId{ 0 };
};

namespace std {
template<>
struct hash<BasePodHandle>
{
	size_t operator()(const BasePodHandle& x) const
	{
		return x.podId;
	}
};
}
