#pragma once

namespace PropertyFlags {
using Type = uint64;

constexpr Type NoSave = (1 << 0);
constexpr Type NoLoad = (1 << 1);
constexpr Type NoCopy = (1 << 2);

// == NoSave | NoLoad | NoCopy, should probably be used everywhere instead of NoSave/NoLoad
constexpr Type Transient = NoSave | NoLoad | NoCopy;

constexpr Type NoEdit = (1 << 8);

constexpr Type Color = (1 << 9);
constexpr Type Multiline = (1 << 10);

template<typename... Flags>
constexpr PropertyFlags::Type Pack(Flags... f)
{
	static_assert((std::is_same_v<PropertyFlags::Type, Flags> && ...));
	return (f | ...);
}

[[maybe_unused]] static PropertyFlags::Type Pack()
{
	return 0;
}
} // namespace PropertyFlags