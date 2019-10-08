#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/MaterialPod.h"

// TODO: create custom asset generator

constexpr auto __default__imageWhite = "/default-data/__default__image-white.jpg";
constexpr auto __default__imageMissing = "/default-data/__default__image-missing.jpg";
constexpr auto __default__imageNormal = "/default-data/__default__image-normal.jpg";

// For now: #custom-type#param0#param1...

// TEMPORARY IMPL
namespace CustomLoader {
template<typename PodType>
inline PodHandle<PodType> GetCustom(const uri::Uri& info)
{
	return AssetManager::GetOrCreate<PodType>(info);
}

// TODO: expand variadic correctly
#define GET_CUSTOM_POD(type, ...) CustomLoader::GetCustom<type>(std::string(__VA_ARGS__) + #type)

// #TexturePod#image_path
inline bool Load(TexturePod* pod, const uri::Uri& path)
{
	// WIP:
	// auto str = "";//path.string().find_last_of('#');
	// auto imgPath = "";//path.string().substr(str+1);
	//
	pod->images.push_back(AssetManager::GetOrCreate<ImagePod>(__default__imageWhite));
	return true;
}

inline bool Load(MaterialPod* pod, const uri::Uri& path)
{
	pod->baseColorTexture = GET_CUSTOM_POD(TexturePod, __default__imageWhite);
	pod->normalTexture = GET_CUSTOM_POD(TexturePod, __default__imageNormal);
	pod->emissiveTexture = GET_CUSTOM_POD(TexturePod, __default__imageWhite);
	pod->metallicRoughnessTexture = GET_CUSTOM_POD(TexturePod, __default__imageWhite);
	pod->occlusionTexture = GET_CUSTOM_POD(TexturePod, __default__imageWhite);
	return true;
}
} // namespace CustomLoader
