#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/util/GltfAux.h"
#include "asset/loaders/DummyLoader.h"

#include "tinygltf/tiny_gltf.h"

namespace GltfMaterialLoader
{
	inline bool Load(MaterialPod* pod, const fs::path& path)
	{
		const auto pPath = path.parent_path();
		auto pParent = AssetManager::GetOrCreate<GltfFilePod>(pPath);

		const auto info = path.filename();
		const auto ext = std::stoi(&info.extension().string()[1]);

		tinygltf::Model& model = pParent->data;

		auto& gltfMaterial = model.materials.at(ext);

		// factors
		auto bFactor = gltfMaterial.pbrMetallicRoughness.baseColorFactor;
		pod->baseColorFactor = { bFactor[0], bFactor[1], bFactor[2], bFactor[3] };
		pod->metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
		pod->roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
		auto eFactor = gltfMaterial.emissiveFactor;
		pod->emissiveFactor = { eFactor[0], eFactor[1], eFactor[2] };

		// scales/strenghts
		pod->normalScale = static_cast<float>(gltfMaterial.normalTexture.scale);
		pod->occlusionStrength = static_cast<float>(gltfMaterial.occlusionTexture.strength);

		// alpha
		pod->alphaMode = GltfAux::GetAlphaMode(gltfMaterial.alphaMode);

		pod->alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
		// doublesided-ness
		pod->doubleSided = gltfMaterial.doubleSided;

		enum class DefType { Missing, White, Normal };
		
		auto LoadTexture = [&](auto textureInfo, PodHandle<TexturePod>& sampler, int32& textCoordIndex, DefType defType)
		{
			if (textureInfo.index != -1)
			{
				tinygltf::Texture& gltfTexture = model.textures.at(textureInfo.index);

				auto textPath = pPath / ("#texture." + std::to_string(textureInfo.index));

				sampler = AssetManager::GetOrCreate<TexturePod>(textPath);

				textCoordIndex = textureInfo.texCoord;
			}
			else
			{
				switch (defType)
				{
				case DefType::Missing:
					sampler = GET_CUSTOM_POD(TexturePod, __default__imageMissing);
					break;
				case DefType::White:
					sampler = GET_CUSTOM_POD(TexturePod, __default__imageWhite);
					break;
				case DefType::Normal:
					sampler = GET_CUSTOM_POD(TexturePod, __default__imageNormal);
					break;
				}	
			}

			return true;
		};

		// samplers
		auto& baseColorTextureInfo = gltfMaterial.pbrMetallicRoughness.baseColorTexture;
		LoadTexture(baseColorTextureInfo, pod->baseColorTexture, pod->baseColorTexCoordIndex, DefType::White);

		auto& emissiveTextureInfo = gltfMaterial.emissiveTexture;
		LoadTexture(emissiveTextureInfo, pod->emissiveTexture, pod->emissiveTexCoordIndex, DefType::White);

		auto& normalTextureInfo = gltfMaterial.normalTexture;
		LoadTexture(normalTextureInfo, pod->normalTexture, pod->normalTexCoordIndex, DefType::Normal);

		// TODO: pack if different
		auto& metallicRougnessTextureInfo = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture;
		LoadTexture(metallicRougnessTextureInfo, pod->metallicRoughnessTexture, pod->metallicRoughnessTexCoordIndex, DefType::White);
		
		auto& occlusionTextureInfo = gltfMaterial.occlusionTexture;
		LoadTexture(occlusionTextureInfo, pod->occlusionTexture, pod->occlusionTexCoordIndex, DefType::White);

		return true;
	}
};