#include "pch.h"

#include "asset/assets/ImageAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "stb_image/stb_image.h"

bool ImageAsset::Load(ImagePod* pod, const fs::path& path)
{
	auto finalPath = path;

	pod->hdr = stbi_is_hdr(finalPath.string().c_str()) == 1;

	if (!pod->hdr)
		pod->data = stbi_load(finalPath.string().c_str(), &pod->width, &pod->height, &pod->components, STBI_rgb_alpha);
	else
		pod->data = stbi_loadf(finalPath.string().c_str(), &pod->width, &pod->height, &pod->components, STBI_rgb_alpha);

	if (!pod->data || (pod->width == 0) || (pod->height == 0))
	{
		LOG_WARN("TextureAsset loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
				 static_cast<bool>(pod->data), pod->width, pod->height);

		return false;
	}

	return true;

}