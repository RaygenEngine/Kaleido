#pragma once

#include "system/EngineComponent.h"
#include "system/Engine.h"
#include "asset/PathSystem.h"
#include "asset/AssetPod.h"
#include "core/reflection/PodReflection.h"
#include "asset/UriLibrary.h"
#include "asset/PodHandle.h"
#include <future>
#include <thread>


struct PodDeleter
{
	void operator()(AssetPod* p);
};

struct PodEntry
{
	struct UnitializedPod {};

	std::unique_ptr<AssetPod, PodDeleter> ptr;
	TypeId type{ refl::GetId<UnitializedPod>() };
	size_t uid{ 0 };
	std::string path;

	std::future<AssetPod*> futureLoaded;

	static PodEntry Create(TypeId type, size_t uid, const std::string& path)
	{
		PodEntry entry;
		entry.type = type;
		entry.uid = uid;
		entry.path = path;
		return entry;
	}

	template<typename T>
	static PodEntry Create(size_t uid, const std::string& path)
	{
		static_assert(std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "PodEntry requires a pod type");
		return Create(refl::GetId<T>(), uid, path);
	}
	
	template<typename T>
	T* UnsafeGet()
	{
		static_assert(std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "Unsafe get called without a pod type");
		return static_cast<T*>(ptr.get());
	}
};

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager
{
	friend class Editor;
	friend class AssetWindow;

	// Using vector here is a bit scary because we cannot control when the actual resize is going to happen
	// Our data (PodEntry) is lightweight so this should not be too costly even for thousands of objects but
	// when having tens of thousands of objects maybe a map would be better.
	std::vector<std::unique_ptr<PodEntry>> m_pods;
	std::unordered_map<std::string, size_t> m_pathCache;

	PodEntry* GetEntryByPathFast(const std::string& path)
	{
		auto it = m_pathCache.find(path);
		if (it == m_pathCache.end())
		{
			return nullptr;
		}
		return m_pods.at(it->second).get();
	}

	template<typename T>
	void LoadEntry(PodEntry* entry)
	{
		T* ptr = new T();
		T::Load(ptr, entry->path); // WIP: handle error
		entry->ptr.reset(ptr);
	}

	// Filters a path to the internal asset manager path format
	static std::string FilterPath(const fs::path& path)
	{
		// WIP:
		if (uri::IsCpuPath(path))
		{
			return path.string();
		}

		return Engine::GetAssetManager()->m_pathSystem.SearchAssetPath(path).string();
	}

	template<typename T>
	PodEntry* CreateAndRegister(const std::string& path)
	{
		assert(m_pathCache.find(path) == m_pathCache.end() && "Path already exists");
		size_t uid = m_pods.size();

		auto& ptr = m_pods.emplace_back(std::make_unique<PodEntry>(PodEntry::Create<T>(uid, path)));
		
		PodEntry* entry = ptr.get();
		m_pathCache[path] = uid;

		return entry;
	}

	// TODO: should be static global, independent of the asset manager.
	PathSystem m_pathSystem;
public:
	// Returns a new handle from a given path.
	// * if the asset of this path is already registered in the asset list it will return a handle to the existing one
	//   requesting a different type of an existing path will (TODO: ) assert. (possibly should return invalid handle)
	// * if the asset of this path is not registered, it will become registered and associated with this type.
	//   it will also begin loading on another thread.
	template<typename PodType>
	static PodHandle<PodType> GetOrCreate(const fs::path& inPath)
	{
		auto inst = Engine::GetAssetManager();
		
		// WIP:
		std::string path = inPath.string();
		auto entry = inst->GetEntryByPathFast(path);
		if (!entry)
		{
			path = FilterPath(inPath);
			entry = inst->GetEntryByPathFast(path);
		}
		
		assert(!path.empty());

		if (entry)
		{
			CLOG_ASSERT(entry->type != refl::GetId<PodType>(),
						"Incorrect pod type on GetOrCreate:\nPath: '{}'\nPrev Type: '{}' New type: '{}'", inPath, entry->type.name(), refl::GetName<PodType>());
		}
		else 
		{
			entry = inst->CreateAndRegister<PodType>(path);
			inst->m_pathCache[inPath.string()] = entry->uid; // WIP:
		}
		
		return PodHandle<PodType>{entry->uid};
	}

	// Refreshes the underlying data of the pod.
	template<typename PodType>
	static void Reload(PodHandle<PodType> handle)
	{
		auto inst = Engine::GetAssetManager();
		inst->LoadEntry<PodType>(inst->m_pods[handle.podId].get());
	}

	// Frees the underlying cpu pod memory
	static void Unload(BasePodHandle handle)
	{
		Engine::GetAssetManager()->m_pods[handle.podId]->ptr.reset();
	}

	static fs::path GetPodPath(BasePodHandle handle)
	{
		return Engine::GetAssetManager()->m_pods[handle.podId]->path;
	}
	
	// For Internal Handle use only.
	template<typename PodType>
	PodType* _Handle_AccessPod(size_t podId)
	{
		PodEntry* entry = m_pods[podId].get();

		assert(entry->type == refl::GetId<PodType>()); // Technically this should never hit because we check during creation.

		if (!entry->ptr)
		{
			LoadEntry<PodType>(entry);
		}

		return entry->UnsafeGet<PodType>();
	}

	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);


	static void PreloadGltf(const std::string& modelPath);

private:
	// Future multithreaded loader specalizations

	template<>
	void LoadEntry<ImagePod>(PodEntry* entry)
	{
		entry->ptr.reset(entry->futureLoaded.get());
	}

	// Early load images.
	template<>
	PodEntry* CreateAndRegister<ImagePod>(const std::string& path)
	{
		assert(m_pathCache.find(path) == m_pathCache.end() && "Path already exists");
		size_t uid = m_pods.size();

		auto& ptr = m_pods.emplace_back(std::make_unique<PodEntry>(PodEntry::Create<ImagePod>(uid, path)));
		
		PodEntry* entry = ptr.get();
		m_pathCache[path] = uid;

		entry->futureLoaded = std::async(std::launch::async, [entry]() -> AssetPod* { 
			ImagePod* ptr = new ImagePod();
			ImagePod::Load(ptr, entry->path);
			return ptr;
		});

		return entry;
	}

};
