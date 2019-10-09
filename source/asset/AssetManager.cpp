#include "pch/pch.h"

#include "asset/AssetManager.h"
#include "core/reflection/PodTools.h"
#include "asset/loaders/GltfFileLoader.h"

bool AssetManager::Init(const fs::path& assetPath)
{
	m_pods.push_back(std::make_unique<PodEntry>());

	fs::current_path(fs::current_path() / assetPath);

	LOG_REPORT("Current working dir: {}", fs::current_path());

	return true;
}

void AssetManager::PreloadGltf(const uri::Uri& gltfModelPath)
{
	auto pParent = AssetManager::GetOrCreate<GltfFilePod>(gltfModelPath);

	const tinygltf::Model& file = pParent->data;

	for (auto& gltfImage : file.images) {
		GetOrCreateFromParent<ImagePod>(gltfImage.uri, pParent);
	}
}

void PodDeleter::operator()(AssetPod* p)
{
	podtools::VisitPod(p, [](auto* pod) {
		static_assert(!std::is_same_v<decltype(pod), AssetPod*>,
			"This should not ever instantiate with AssetPod*. Pod tools has internal error.");
		delete pod;
	});
}

void Code()
{
	podtools::ForEachPodType([](auto p) {
		using PodType = std::remove_pointer_t<decltype(p)>;
		PodHandle<PodType> a;
		a._Debug();
	});
}
