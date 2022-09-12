//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsResourceManifest.h"
#include "Private/RTTI/BsResourceManifestRTTI.h"
#include "Serialization/BsFileSerializer.h"
#include "Error/BsException.h"

namespace bs
{
	ResourceManifest::ResourceManifest(const ConstructPrivately& dummy)
	{

	}

	ResourceManifest::ResourceManifest(const String& name)
		:mName(name)
	{

	}

	SPtr<ResourceManifest> ResourceManifest::Create(const String& name)
	{
		return bs_shared_ptr_new<ResourceManifest>(name);
	}

	SPtr<ResourceManifest> ResourceManifest::CreateEmpty()
	{
		return bs_shared_ptr_new<ResourceManifest>(ConstructPrivately());
	}

	void ResourceManifest::RegisterResource(const UUID& uuid, const Path& filePath)
	{
		auto iterFind = mUUIDToFilePath.Find(uuid);

		if(iterFind != mUUIDToFilePath.End())
		{
			if (iterFind->second != filePath)
			{
				mFilePathToUUID.Erase(iterFind->second);

				mUUIDToFilePath[uuid] = filePath;
				mFilePathToUUID[filePath] = uuid;
			}
		}
		else
		{
			auto iterFind2 = mFilePathToUUID.Find(filePath);
			if (iterFind2 != mFilePathToUUID.End())
				mUUIDToFilePath.Erase(iterFind2->second);

			mUUIDToFilePath[uuid] = filePath;
			mFilePathToUUID[filePath] = uuid;
		}
	}

	void ResourceManifest::UnregisterResource(const UUID& uuid)
	{
		auto iterFind = mUUIDToFilePath.Find(uuid);

		if(iterFind != mUUIDToFilePath.End())
		{
			mFilePathToUUID.Erase(iterFind->second);
			mUUIDToFilePath.Erase(uuid);
		}
	}

	bool ResourceManifest::UuidToFilePath(const UUID& uuid, Path& filePath) const
	{
		auto iterFind = mUUIDToFilePath.Find(uuid);

		if(iterFind != mUUIDToFilePath.End())
		{
			filePath = iterFind->second;
			return true;
		}
		else
		{
			filePath = Path::BLANK;
			return false;
		}
	}

	bool ResourceManifest::FilePathToUUID(const Path& filePath, UUID& outUUID) const
	{
		auto iterFind = mFilePathToUUID.Find(filePath);

		if(iterFind != mFilePathToUUID.End())
		{
			outUUID = iterFind->second;
			return true;
		}
		else
		{
			outUUID = UUID::EMPTY;
			return false;
		}
	}

	bool ResourceManifest::UuidExists(const UUID& uuid) const
	{
		auto iterFind = mUUIDToFilePath.Find(uuid);

		return iterFind != mUUIDToFilePath.End();
	}

	bool ResourceManifest::FilePathExists(const Path& filePath) const
	{
		auto iterFind = mFilePathToUUID.Find(filePath);

		return iterFind != mFilePathToUUID.End();
	}

	void ResourceManifest::Save(const SPtr<ResourceManifest>& manifest, const Path& path, const Path& relativePath)
	{
		if(relativePath.IsEmpty())
		{
			FileEncoder Fs(path);
			fs.Encode(manifest.get());
		}
		else
		{
			SPtr<ResourceManifest> copy = create(manifest->mName);

			for (auto& elem : manifest->mFilePathToUUID)
			{
				if (!relativePath.Includes(elem.first))
				{
					BS_EXCEPT(InvalidStateException, "Path in resource manifest cannot be made relative to: \"" +
						relativePath.ToString() + "\". Path: \"" + elem.first.toString() + "\"");
				}

				Path elementRelativePath = elem.first.GetRelative(relativePath);

				copy->mFilePathToUUID[elementRelativePath] = elem.second;
			}

			for (auto& elem : manifest->mUUIDToFilePath)
			{
				if (!relativePath.Includes(elem.second))
				{
					BS_EXCEPT(InvalidStateException, "Path in resource manifest cannot be made relative to: \"" +
						relativePath.ToString() + "\". Path: \"" + elem.second.toString() + "\"");
				}

				Path elementRelativePath = elem.second.GetRelative(relativePath);

				copy->mUUIDToFilePath[elem.first] = elementRelativePath;
			}

			FileEncoder Fs(path);
			fs.Encode(copy.get());
		}
	}

	SPtr<ResourceManifest> ResourceManifest::Load(const Path& path, const Path& relativePath)
	{
		FileDecoder Fs(path);
		SPtr<ResourceManifest> manifest = std::static_pointer_cast<ResourceManifest>(fs.Decode());

		if(relativePath.IsEmpty())
			return manifest;

		SPtr<ResourceManifest> copy = create(manifest->mName);

		for(auto& elem : manifest->mFilePathToUUID)
		{
			Path absPath = elem.first.GetAbsolute(relativePath);
			copy->mFilePathToUUID[absPath] = elem.second;
		}

		for(auto& elem : manifest->mUUIDToFilePath)
		{
			Path absPath = elem.second.GetAbsolute(relativePath);
			copy->mUUIDToFilePath[elem.first] = absPath;
		}

		return copy;
	}

	RTTITypeBase* ResourceManifest::getRTTIStatic()
	{
		return ResourceManifestRTTI::Instance();
	}

	RTTITypeBase* ResourceManifest::getRTTI() const
	{
		return ResourceManifest::GetRTTIStatic();
	}
}
