#pragma once

#include "renderer/renderers/contextual/d3d11/asset/D3D11Asset.h"

class D3D11Graphics;

class D3D11AssetManager {
	std::unordered_map<size_t, std::unique_ptr<D3D11AssetBase>> m_assetMap;
	D3D11Graphics& m_gfx;

public:
	D3D11AssetManager(D3D11Graphics& gfx)
		: m_gfx(gfx)
	{
	}

	template<typename AssetT>
	AssetT* GpuGetOrCreate(PodHandle<typename AssetT::PodType> handle)
	{
		size_t uid = handle.podId;
		auto it = m_assetMap.find(uid);
		if (it != m_assetMap.end()) {
			return dynamic_cast<AssetT*>(it->second.get());
		}

		AssetT* result = new AssetT(handle);
		m_assetMap.emplace(uid, result);
		result->FriendLoad(m_gfx);
		return result;
	}

	template<typename AssetT>
	AssetT* GenerateFromPodPath(const uri::Uri& path)
	{
		return GpuGetOrCreate<AssetT>(AssetManager::GetOrCreate<typename AssetT::PodType>(path));
	}
};
