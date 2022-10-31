//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsResourceManifest.h"
#include "Private/RTTI/BsResourceManifestRTTI.h"
#include "Serialization/BsFileSerializer.h"
#include "Error/BsException.h"

using namespace bs;

ResourceManifest::ResourceManifest(const ConstructPrivately& dummy)
{
}

ResourceManifest::ResourceManifest(const String& name)
	: mName(name)
{
}

SPtr<ResourceManifest> ResourceManifest::Create(const String& name)
{
	return B3DMakeShared<ResourceManifest>(name);
}

SPtr<ResourceManifest> ResourceManifest::CreateEmpty()
{
	return B3DMakeShared<ResourceManifest>(ConstructPrivately());
}

void ResourceManifest::RegisterResource(const UUID& uuid, const Path& filePath)
{
	auto iterFind = mUUIDToFilePath.find(uuid);

	if(iterFind != mUUIDToFilePath.end())
	{
		if(iterFind->second != filePath)
		{
			mFilePathToUUID.erase(iterFind->second);

			mUUIDToFilePath[uuid] = filePath;
			mFilePathToUUID[filePath] = uuid;
		}
	}
	else
	{
		auto iterFind2 = mFilePathToUUID.find(filePath);
		if(iterFind2 != mFilePathToUUID.end())
			mUUIDToFilePath.erase(iterFind2->second);

		mUUIDToFilePath[uuid] = filePath;
		mFilePathToUUID[filePath] = uuid;
	}
}

void ResourceManifest::UnregisterResource(const UUID& uuid)
{
	auto iterFind = mUUIDToFilePath.find(uuid);

	if(iterFind != mUUIDToFilePath.end())
	{
		mFilePathToUUID.erase(iterFind->second);
		mUUIDToFilePath.erase(uuid);
	}
}

bool ResourceManifest::UuidToFilePath(const UUID& uuid, Path& filePath) const
{
	auto iterFind = mUUIDToFilePath.find(uuid);

	if(iterFind != mUUIDToFilePath.end())
	{
		filePath = iterFind->second;
		return true;
	}
	else
	{
		filePath = Path::kBlank;
		return false;
	}
}

bool ResourceManifest::FilePathToUuid(const Path& filePath, UUID& outUUID) const
{
	auto iterFind = mFilePathToUUID.find(filePath);

	if(iterFind != mFilePathToUUID.end())
	{
		outUUID = iterFind->second;
		return true;
	}
	else
	{
		outUUID = UUID::kEmpty;
		return false;
	}
}

bool ResourceManifest::UuidExists(const UUID& uuid) const
{
	auto iterFind = mUUIDToFilePath.find(uuid);

	return iterFind != mUUIDToFilePath.end();
}

bool ResourceManifest::FilePathExists(const Path& filePath) const
{
	auto iterFind = mFilePathToUUID.find(filePath);

	return iterFind != mFilePathToUUID.end();
}

void ResourceManifest::Save(const SPtr<ResourceManifest>& manifest, const Path& path, const Path& relativePath)
{
	if(relativePath.IsEmpty())
	{
		FileEncoder fs(path);
		fs.Encode(manifest.get());
	}
	else
	{
		SPtr<ResourceManifest> copy = Create(manifest->mName);

		for(auto& elem : manifest->mFilePathToUUID)
		{
			if(!relativePath.Includes(elem.first))
			{
				BS_EXCEPT(InvalidStateException, "Path in resource manifest cannot be made relative to: \"" + relativePath.ToString() + "\". Path: \"" + elem.first.ToString() + "\"");
			}

			Path elementRelativePath = elem.first.GetRelative(relativePath);

			copy->mFilePathToUUID[elementRelativePath] = elem.second;
		}

		for(auto& elem : manifest->mUUIDToFilePath)
		{
			if(!relativePath.Includes(elem.second))
			{
				BS_EXCEPT(InvalidStateException, "Path in resource manifest cannot be made relative to: \"" + relativePath.ToString() + "\". Path: \"" + elem.second.ToString() + "\"");
			}

			Path elementRelativePath = elem.second.GetRelative(relativePath);

			copy->mUUIDToFilePath[elem.first] = elementRelativePath;
		}

		FileEncoder fs(path);
		fs.Encode(copy.get());
	}
}

SPtr<ResourceManifest> ResourceManifest::Load(const Path& path, const Path& relativePath)
{
	FileDecoder fs(path);
	SPtr<ResourceManifest> manifest = std::static_pointer_cast<ResourceManifest>(fs.Decode());

	if(relativePath.IsEmpty())
		return manifest;

	SPtr<ResourceManifest> copy = Create(manifest->mName);

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

RTTITypeBase* ResourceManifest::GetRttiStatic()
{
	return ResourceManifestRTTI::Instance();
}

RTTITypeBase* ResourceManifest::GetRtti() const
{
	return ResourceManifest::GetRttiStatic();
}
