#include "pch.h"

#include "PathSystem.h"
#include "core/auxiliary/StringAux.h"

#include <filesystem>


namespace fs = std::filesystem;

namespace Assets
{
	inline bool PathSystem::SetCurrentDir(const std::string& path) const
	{
		std::error_code error;
		fs::current_path(path, error);
		return !error;
	}

	void PathSystem::GatherSubDirectories(const std::string& directoryPath,
		std::vector<std::string>& dirList) const
	{
		dirList.push_back(directoryPath);
		RT_XENGINE_LOG_TRACE("\'{}\'", directoryPath);
		for (const auto& entry : fs::directory_iterator(directoryPath))
		{
			if (fs::is_directory(entry.status()))
			{
				GatherSubDirectories(entry.path().string(), dirList);
			}
		}

	}

	bool PathSystem::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
	{
		RT_XENGINE_LOG_INFO("Initializing Paths System");

		auto appParentPath = fs::path(applicationPath);

		RT_XENGINE_ASSERT_RETURN_FALSE(appParentPath.has_parent_path(), "Couldn't retrieve application's parent directory!");

		appParentPath = appParentPath.parent_path();
		RT_XENGINE_ASSERT_RETURN_FALSE(SetCurrentDir(appParentPath.string()), "Couldn't set work directory!");

		RT_XENGINE_LOG_DEBUG("Searching for directory: \'{}\'", dataDirectoryName); // search recursively for the data folder
		bool found = false;
		for (auto currPath = fs::current_path(); !found && currPath != fs::current_path().parent_path(); fs::
			current_path(fs::current_path().parent_path()), currPath = fs::current_path())
		{
			RT_XENGINE_LOG_TRACE("searching in: \'{}\'", currPath.string());
			for (const auto& entry : fs::directory_iterator(currPath))
			{
				if (fs::is_directory(entry.status()))
				{
					auto dataPath = entry.path().parent_path();
					dataPath += fs::path("\\" + dataDirectoryName);
					if (dataPath == entry.path())
					{
						RT_XENGINE_LOG_DEBUG("found in: \'{}\'", entry.path().string());
						m_assetsRootPath = entry.path().string();
						found = true;
						break;
					}
				}
			}
		}

		RT_XENGINE_ASSERT_RETURN_FALSE(!m_assetsRootPath.empty(), "Couldn't locate assets root directory!");
		RT_XENGINE_ASSERT_RETURN_FALSE(SetCurrentDir(m_assetsRootPath), "Couldn't locate assets root directory!");
		RT_XENGINE_LOG_TRACE("Assets Sub-dirs:");
		GatherSubDirectories(m_assetsRootPath, m_assetsPaths);

		m_shadersRootPath = m_assetsRootPath + "\\shaders";
		RT_XENGINE_ASSERT_RETURN_FALSE(SetCurrentDir(m_shadersRootPath), "Couldn't locate shaders root directory!");
		RT_XENGINE_LOG_TRACE("Shaders Sub-dirs:");
		GatherSubDirectories(m_shadersRootPath, m_shadersPaths);

		m_scenesRootPath = m_assetsRootPath + "\\scenes";
		RT_XENGINE_ASSERT_RETURN_FALSE(SetCurrentDir(m_scenesRootPath), "Couldn't locate scenes root directory!");
		RT_XENGINE_LOG_TRACE("Scenes Sub-dirs:");
		GatherSubDirectories(m_scenesRootPath, m_scenesPaths);

		RT_XENGINE_LOG_INFO("Assets root directory: \'{}\'", m_assetsRootPath);
		RT_XENGINE_LOG_INFO("Shaders root directory: \'{}\'", m_shadersRootPath);
		RT_XENGINE_LOG_INFO("Scenes root directory: \'{}\'", m_scenesRootPath);

		return true;
	}

	std::string PathSystem::SearchAssetInShadersDirectories(const std::string& relativeAssetPath) const
	{
		for (auto& path : m_shadersPaths)
		{
			auto fsPath = fs::path(path + "\\" + relativeAssetPath);

			std::error_code error;
			fsPath = fs::absolute(fsPath, error);

			if (!error && fs::exists(fsPath))
				return fsPath.string();
		}

		RT_XENGINE_LOG_WARN("Couldn't locate asset in shaders sub-dirs, asset: \'{}\'", relativeAssetPath);

		return "";
	}

	std::string PathSystem::SearchAssetInScenesDirectories(const std::string& relativeAssetPath) const
	{
		for (auto& path : m_scenesPaths)
		{
			auto fsPath = fs::path(path + "\\" + relativeAssetPath);

			std::error_code error;
			fsPath = fs::absolute(fsPath, error);

			if (!error && fs::exists(fsPath))
				return fsPath.string();
		}

		RT_XENGINE_LOG_WARN("Couldn't locate asset in scenes sub-dirs, asset: \'{}\'", relativeAssetPath);

		return "";
	}

	std::string PathSystem::SearchAssetInAssetsDirectories(const std::string& relativeAssetPath, const std::string& absoluteHintMask) const
	{
		for (auto& path : m_assetsPaths)
		{
			// a hint was given
			if (!absoluteHintMask.empty())
			{
				auto maskPart = path.substr(0, absoluteHintMask.size());

				// skip based on mask
				if (!Core::CaseInsensitiveCompare(maskPart, absoluteHintMask))
				{
					continue;
				}
			}

			auto fsPath = fs::path(path + "\\" + relativeAssetPath);

			std::error_code error;
			fsPath = fs::absolute(fsPath, error);

			if (!error && fs::exists(fsPath))
				return fsPath.string();
		}

		if (absoluteHintMask.empty())
			RT_XENGINE_LOG_WARN("Couldn't locate asset in any asset sub-dirs, asset: \'{}\'", relativeAssetPath);
		if (!absoluteHintMask.empty())
			RT_XENGINE_LOG_WARN("Couldn't locate asset in any asset sub-dirs with absolute mask, asset: \'{}\', mask: \'{}\'", relativeAssetPath, absoluteHintMask);

		return "";
	}

	std::string PathSystem::SearchAsset(const std::string& relativeAssetPath, const std::string& pathHint) const
	{
		// Trim
		auto pcopy = relativeAssetPath;
		Core::Trim(pcopy);

		RT_XENGINE_LOG_TRACE("Searching for asset, given path: \'{}\', given path hint: \'{}\'", pcopy, pathHint);

		auto path = fs::path(pcopy);
		
		// if path is absolute shorten it and return it if it is valid
		if (path.is_absolute())
		{
			std::error_code error;
			auto shortenAbsPath = fs::absolute(path, error);

			// if path exists return it 
			if (!error && fs::exists(shortenAbsPath))
				return shortenAbsPath.string();

			RT_XENGINE_LOG_WARN("Couldn't locate asset given in absolute path, path: \'{}\'", pcopy);

			// otherwise we won't find it anywhere
			return "";
		}

		// if it is relative

		// search by hint
		// path hint only works for relative hints (root is scenes folder)
		if (!pathHint.empty() && fs::path(pathHint).is_relative())
		{
			std::string finalHint = pathHint;

			finalHint.erase(std::remove_if(finalHint.end()-1, finalHint.end(),
				[](byte x) {return x == '\\'; }), finalHint.end());

			const auto pathHintMask = fs::path(m_assetsRootPath + "\\" + finalHint).string();
			
			auto res = SearchAssetInAssetsDirectories(pcopy, pathHintMask);

			// if found in scenes
			if (!res.empty())
				return res;
		}

		// search by extension
		const auto extension = path.extension().string();

		// search based on shader extensions
		if (m_shaderExtensions.find(Core::ToLower(extension)) != m_shaderExtensions.end())
		{
			auto res = SearchAssetInShadersDirectories(pcopy);

			// if found in shaders
			if(!res.empty())
				return res;
		} 
		
		// search based on scene extensions
		if (m_sceneExtensions.find(Core::ToLower(extension)) != m_sceneExtensions.end())
		{
			auto res = SearchAssetInScenesDirectories(pcopy);

			// if found in scenes
			if (!res.empty())
				return res;
		}

		RT_XENGINE_LOG_WARN("Couldn't locate asset searching specific sub-dirs by extension, searching in every subdirectory of assets root, path: \'{}\'", relativeAssetPath);
		// search everywhere
		return SearchAssetInAssetsDirectories(pcopy);
	}

	bool PathSystem::FileExists(const std::string& filepath)
	{
		return fs::exists(filepath);
	}

	std::string PathSystem::GetNameWithExtension(const std::string& filepath)
	{
		return fs::path(filepath).filename().string();
	}

	std::string PathSystem::GetNameWithoutExtension(const std::string& filepath)
	{
		return fs::path(filepath).stem().string();
	}

	std::string PathSystem::GetRootDirectory(const std::string& filepath)
	{
		return fs::path(filepath).root_directory().string();
	}

	std::string PathSystem::GetParentPath(const std::string& filepath)
	{
		return fs::path(filepath).parent_path().string();
	}

	std::string PathSystem::GetPathWithoutExtension(const std::string& filepath)
	{
		return GetParentPath(filepath) + "\\" + GetNameWithoutExtension(filepath);
	}

	std::string PathSystem::GetExtension(const std::string& filepath)
	{
		return fs::path(filepath).extension().string();
	}
}
