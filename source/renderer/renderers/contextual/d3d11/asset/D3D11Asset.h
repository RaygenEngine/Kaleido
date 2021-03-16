#pragma once

#include "asset/AssetPod.h"
#include "asset/PodHandle.h"

#include "system/Object.h"

class D3D11Graphics;

struct D3D11AssetBase : Object {

	D3D11AssetBase(BasePodHandle genericHandle)
		: genericPodHandle(genericHandle)
	{
	}

	BasePodHandle genericPodHandle;
};


template<typename PodTypeT>
struct D3D11Asset : D3D11AssetBase {

	using PodType = PodTypeT;

protected:
	D3D11Asset(PodHandle<PodType> handle)
		: D3D11AssetBase(handle)
		, podHandle(handle)
	{
	}

	PodHandle<PodType> podHandle;

	virtual void Load(D3D11Graphics& gfx) = 0;

private:
	friend class D3D11AssetManager;
	void FriendLoad(D3D11Graphics& gfx) { Load(gfx); }


public:
	// DOC:
	const PodType* LockData() { return podHandle.Lock(); }
};
