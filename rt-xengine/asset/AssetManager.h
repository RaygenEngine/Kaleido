#pragma once

#include "system/EngineComponent.h"
#include "system/reflection/Reflector.h"
#include "system/Engine.h"
#include "asset/PathSystem.h"
#include "asset/Asset.h"
constexpr auto __default__imageWhite = "__default__image-white.jpg";
constexpr auto __default__imageMissing = "__default__image-missing.jpg";

constexpr auto __default__texture = "#__default__texture";
constexpr auto __default__material = "#__default__material";

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager
{

	std::unordered_map<size_t, AssetPod*> m_uidToPod;
	std::unordered_map<size_t, std::string> m_uidToPath;
	
	std::unordered_map<std::string, size_t> m_pathToUid;

private:
	template<typename PodType>
	std::string GetPodPath(size_t podId)
	{
		return m_uidToPath.at(podId);
	}

	void DetachOldPod(const fs::path& path)
	{
		// TODO: Copy old pod
		m_pathToUid.erase(path.string());
	}


	template<typename PodType>
	PodType* FindPod(size_t podId)
	{
		return dynamic_cast<PodType*>(m_uidToPod.at(podId));
	}

	template<typename PodType>
	PodType* RefreshPod(size_t podId)
	{
		auto it = m_uidToPod.find(podId);
		if (it == m_uidToPod.end())
		{
			return dynamic_cast<PodType*>(it->second);
		}

		auto& podPath = GetPodPath(podId);

		PodType* pod = new PodType();
		PodType::Load(pod, podPath);
		
		m_uidToPod.insert({ podId, podPath });
		
		return pod;
	}

	template<typename PodType>
	PodType* ReplaceInto(size_t podId, const fs::path& path)
	{
		assert(false && "Incorrect implementation, implement cache hits in replace");
		auto it = m_uidToPod.find(podId);

		if (it != m_uidToPod.end())
		{
			DetachOldPod(m_uidToPath[podId]);
			PodType*& pod = it->second;
			delete pod;
			pod = new PodType();

			PodType::Load(pod, path);

			m_uidToPath[podId] = path;
			return pod;
		}

		PodType* pod = new PodType();

		PodType::Load(pod, path);

		m_uidToPath.insert({ podId, path });
		m_uidToPod.insert({ podId, pod });
		return pod;
	}

protected:
	template<typename PodType>
	void DeletePod(size_t podId)
	{
		auto pod = FindPod<PodType>(podId);
		delete pod;
		m_uidToPod.erase(podId);
	}



private:
	template<typename PodHandle>
	static auto RefreshPod(const PodHandle& handle) -> typename PodHandle::PodType
	{
		return Engine::GetAssetManager()->RefreshPod(handle.podId);
	}

	template<typename PodHandle>
	static auto FindPod(const PodHandle& handle) -> typename PodHandle::PodType
	{
		return Engine::GetAssetManager()->FindPod(handle.podId);
	}
	

	static size_t NextHandle;
public:
	template<typename PodType>
	static PodHandle<PodType> GetOrCreate(const fs::path& path)
	{
		auto am = Engine::GetAssetManager();
		// Path Code


		fs::path p;
		if (IsCpuPath(path))
		{
			p = path;
		}
		else
		{
			p = m_pathSystem.SearchAssetPath(path);
			assert(!p.empty());
		}

		auto it = m_pathToUid.find(p);
		if (it != m_pathToUid.end())
		{
			PodHandle<PodType> result;
			result.podId = it->second;
			return result;
		}

		// Generate
		size_t newHandle = NextHandle++;
		PodHandle<PodType> result;
		result.podId = newHandle;
		
		m_pathToUid.insert({ path.string(), newHandle });

		PodType* pod = new PodType();

		PodType::Load(pod, path);

		m_uidToPath.insert({ podId, path });
		m_uidToPod.insert({ podId, pod });

		return pod;
	}

	//template<typename PodHandle>
	//static auto ReplaceInto(const PodHandle& handle, const fs::path& path) -> typename PodHandle::PodType
	//{
	//	return Engine::GetAssetManager()->ReplaceInto(handle.podId);
	//}




	static bool IsCpuPath(const fs::path& path)
	{
		if (path.filename().string()[0] == '#') 
			return true;
		return false;
	}

	//template<typename ...Args>
	//bool LoadAssetList(Args... args)
	//{
	//	using namespace std;
	//	//static_assert(conjunction_v< (conjunction_v < is_pointer_v<Args>, is_base_of_v<std::remove_pointer_t<Args>, Asset>), ... >, "Not all argument types are pointers of assets.");

	//	return ((Load(args) && ...));
	//}

	////template<typename AssetT>
	////AssetT* GenerateAndLoad(const fs::path& path)
	////{
	////	auto r = GenerateAsset(path);
	////	Load(r);
	////	return r;
	////}
	PathSystem m_pathSystem;
	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);
};
