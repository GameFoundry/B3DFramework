//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2023 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Resources/B3DPackage.h"
#include "RTTI/B3DPackageRTTI.h"
#include "B3DResource.h"
#include "B3DResources.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "Serialization/B3DBinaryCloner.h"
#include "Serialization/B3DBinarySerializer.h"
#include "Serialization/B3DFileSerializer.h"
#include "Utility/B3DScopeGuard.h"
#include "Utility/B3DUtility.h"

using namespace b3d;

/** Resource header serialized within the serialized package, letting us know where in the serialized data stream to find the resource data. */
struct SerializedResourceHeader
{
	UUID Id = UUID::kEmpty;
	u64 OffsetInDataStream = 0; /**< Offset at which the resource begins in the data stream (in bytes). */
	u64 SizeInDataStream = 0; /**< Size of the resources when serialized to the data stream (in bytes). */
};

RTTIType* PackageResourceMetaData::GetRttiStatic()
{
	return PackageResourceMetaDataRTTI::Instance();
}

RTTIType* PackageResourceMetaData::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* PackageResourceUserMetaData::GetRttiStatic()
{
	return PackageResourceUserMetaDataRTTI::Instance();
}

RTTIType* PackageResourceUserMetaData::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* PackageMetaData::GetRttiStatic()
{
	return PackageMetaDataRTTI::Instance();
}

RTTIType* PackageMetaData::GetRtti() const
{
	return GetRttiStatic();
}

PackageHierarchy::Entry::~Entry()
{
	for (auto& entry : Children)
	{
		if(B3D_ENSURE(entry != nullptr))
			B3DPoolDelete(entry);
	}
}

PackageHierarchy::Entry* PackageHierarchy::FindEntry(const Path& path) const
{
	auto fnFindEntryRecursive = [](Entry* parent, const Path& path, u32 pathEntryIndex, u32 pathEntryCount, const auto& fnFindEntryRecursive) -> Entry*
	{
		if(!B3D_ENSURE(parent))
			return nullptr;

		const String& pathEntryName = pathEntryIndex < path.GetDirectoryCount() ? path.GetDirectory(pathEntryIndex) : path.GetFilename();

		Entry* childEntry = nullptr;
		for (auto& child : parent->Children)
		{
			if (child->Name == pathEntryName)
			{
				childEntry = child;
				break;
			}
		}

		if (childEntry == nullptr)
			return nullptr;

		const bool isLastEntry = (pathEntryIndex + 1) == pathEntryCount;
		if (isLastEntry)
			return childEntry;

		if (childEntry->Type == EntryType::Resource)
			return nullptr;

		return fnFindEntryRecursive(childEntry, path, pathEntryIndex + 1, pathEntryCount, fnFindEntryRecursive);
	};

	const u32 pathEntryCount = path.GetDirectoryCount() + (path.IsFile() ? 1 : 0);
	return fnFindEntryRecursive(Root, path, 0, pathEntryCount, fnFindEntryRecursive);
}

Package::Package(String name, const UUID& id) :
mName(std::move(name)), mId(id)
{
	if (mId.Empty())
		mId = UUIDGenerator::GenerateRandom();
}

void Package::SetPackageId(const UUID& id)
{
	Lock lock(mMetaDataMutex);
	mId = id;
}

void Package::SetPackageName(const String& name)
{
	Lock lock(mMetaDataMutex);
	mName = name;
}

const SPtr<PackageMetaData>& Package::GetPackageMetaData() const
{
	Lock lock(mMetaDataMutex);
	return mPackageMetaData;
}

void Package::SetPackageMetaData(const SPtr<PackageMetaData>& metaData)
{
	Lock lock(mMetaDataMutex);
	mPackageMetaData = metaData;
}

bool Package::IsEmpty() const
{
	Lock lock(mMetaDataMutex);
	return mResourceInformationByUUID.empty();
}

Vector<UUID> Package::CreateResourceIdList() const
{
	Vector<UUID> output;

	Lock lock(mMetaDataMutex);
	output.reserve(mResourceInformationByUUID.size());

	for (const auto& entry : mResourceInformationByUUID)
		output.push_back(entry.first);

	return output;
}

PackageHierarchy Package::CreateHierarchy() const
{
	PackageHierarchy output;
	output.Root = B3DPoolNew<PackageHierarchy::Entry>(PackageHierarchy::EntryType::Folder);

	auto fnCreateEntriesRecursive = [](PackageHierarchy::Entry* parent, const Path& path, u32 pathEntryIndex, u32 pathEntryCount, const SPtr<PackageResourceMetaData>& metaData, const auto& fnCreateEntriesRecursive)
	{
		if(!B3D_ENSURE(parent))
			return;

		const String& pathEntryName = pathEntryIndex < path.GetDirectoryCount() ? path.GetDirectory(pathEntryIndex) : path.GetFilename();

		PackageHierarchy::Entry* foundChild = nullptr;
		for (const auto& child : parent->Children)
		{
			if (child->Name == pathEntryName)
			{
				foundChild = child;
				break;
			}
		}

		const bool isLastEntry = (pathEntryIndex + 1) == pathEntryCount;
		const bool isFolder = !isLastEntry || metaData == nullptr || metaData->Flags.IsSet(PackageResourceFlag::Folder);
		if (!foundChild)
		{
			Path entryPath = path.GetSubPath(pathEntryIndex + 1);
			if (pathEntryIndex == path.GetDirectoryCount())
				entryPath.SetFilename(path.GetFilename());

			foundChild = B3DPoolNew<PackageHierarchy::Entry>(isFolder ? PackageHierarchy::EntryType::Folder : PackageHierarchy::EntryType::Resource, pathEntryName, entryPath, metaData);
			parent->Children.push_back(foundChild);
		}

		if (foundChild->Type == PackageHierarchy::EntryType::Resource)
		{
			B3D_ASSERT(!isFolder);
			return;
		}

		if (!isLastEntry)
			fnCreateEntriesRecursive(foundChild, path, pathEntryIndex + 1, pathEntryCount, metaData, fnCreateEntriesRecursive);
	};

	Lock lock(mMetaDataMutex);
	{
		for (const auto& entry : mResourceInformationByUUID)
		{
			const SPtr<PackageResourceMetaData>& metaData = entry.second->MetaData;

			const u32 pathEntryCount = metaData->Path.GetDirectoryCount() + (metaData->Path.IsFile() ? 1 : 0);
			fnCreateEntriesRecursive(output.Root, metaData->Path, 0, pathEntryCount, metaData, fnCreateEntriesRecursive);
		}
	}

	return output;
}

bool Package::BreakCombinedPackagePath(const Path& combinedPath, Path& outPathToPackage, Path& outPathToResource)
{
	outPathToPackage.Clear();
	outPathToResource.Clear();

	const u32 directoryCount = combinedPath.GetDirectoryCount();

	for(u32 directoryIndex = 0; directoryIndex < directoryCount; directoryIndex++)
	{
		const String& directory = combinedPath.GetDirectory(directoryIndex);

		if(StringUtil::EndsWith(directory, kPackageExtension))
		{
			outPathToPackage = combinedPath.GetSubPath(directoryIndex);
			outPathToPackage.SetFilename(directory);

			continue;
		}

		if(!outPathToPackage.IsEmpty())
			outPathToResource.PushDirectory(directory);
	}

	if(combinedPath.IsFile())
	{
		if(!outPathToPackage.IsEmpty())
		{
			outPathToResource.SetFilename(combinedPath.GetFilename());
		}
	}

	return !outPathToResource.IsEmpty();
}

bool Package::Contains(const UUID& id) const
{
	Lock lock(mMetaDataMutex);
	return GetResourceInformation(id, false) != nullptr;
}

bool Package::Contains(const Path& path) const
{
	Lock lock(mMetaDataMutex);
	return GetResourceInformation(path, false) != nullptr;
}

SPtr<const PackageResourceMetaData> Package::GetResourceMetaData(const UUID& id) const
{
	Lock lock(mMetaDataMutex);
	const ResourceInformation* const resourceInformation = GetResourceInformation(id);

	return resourceInformation ? resourceInformation->MetaData : nullptr;
}

SPtr<const PackageResourceMetaData> Package::GetResourceMetaData(const Path& path) const
{
	Lock lock(mMetaDataMutex);
	const ResourceInformation* const resourceInformation = GetResourceInformation(path);

	return resourceInformation ? resourceInformation->MetaData : nullptr;
}

void Package::SetResourceMetaData(const UUID& id, const SPtr<PackageResourceUserMetaData>& data)
{
	Lock lock(mMetaDataMutex);
	ResourceInformation* const resourceInformation = GetResourceInformation(id);

	if (resourceInformation)
	{
		// Always make a copy and user might be reading from the meta-data, and we cannot modify it in a thread safe way
		const SPtr<PackageResourceMetaData> metaDataCopy = B3DRTTIClone(resourceInformation->MetaData);

		metaDataCopy->AdditionalMetaData = data;
		resourceInformation->MetaData = metaDataCopy;
	}
}

void Package::SetResourceMetaData(const Path& path, const SPtr<PackageResourceUserMetaData>& data)
{
	Lock lock(mMetaDataMutex);
	ResourceInformation* const resourceInformation = GetResourceInformation(path);

	if (resourceInformation)
	{
		// Always make a copy and user might be reading from the meta-data, and we cannot modify it in a thread safe way
		const SPtr<PackageResourceMetaData> metaDataCopy = B3DRTTIClone(resourceInformation->MetaData);

		metaDataCopy->AdditionalMetaData = data;
		resourceInformation->MetaData = metaDataCopy;
	}
}

PackageResourceLoadState Package::GetResourceLoadState(const UUID& id) const
{
	Lock lock(mMetaDataMutex);
	const ResourceInformation* const resourceInformation= GetResourceInformation(id);

	return resourceInformation ? resourceInformation->LoadState : PackageResourceLoadState::Unloaded;
}

float Package::GetResourceLoadProgress(const UUID& id) const
{
	Lock lock(mMetaDataMutex);
	const ResourceInformation* const resourceInformation= GetResourceInformation(id);

	return resourceInformation ? resourceInformation->LoadProgress.load(std::memory_order_relaxed) : 0.0f;
}

void Package::AddResource(const Path& path, const HResource& resource)
{
	B3D_ASSERT(!resource.IsLoaded(false) || resource->GetId() == resource.GetId());
	AddResource(path, resource.GetShared());
}

void Package::AddResource(const Path& path, const SPtr<Resource>& resource)
{
	UUID id;
	if (resource != nullptr)
	{
		id = resource->GetId();

		if (id.Empty())
		{
			B3D_LOG(Error, LogResources, "Cannot register resource in package {0} ({1}) at path {2}. Provided resource id is empty.", mName, mAssociatedPackageFilePath, path);
			return;
		}
	}
	else
	{
		id = UUIDGenerator::GenerateRandom();
	}

	if (path.IsAbsolute())
	{
		B3D_LOG(Error, LogResources, "Cannot register resource in package {0} ({1}) at path {2}. Provided path must be relative. ", mName, mAssociatedPackageFilePath, path);
		return;
	}

	if (path.IsEmpty())
	{
		B3D_LOG(Error, LogResources, "Cannot register resource in package {0} ({1}). Provided path must not be empty. ", mName, mAssociatedPackageFilePath);
		return;
	}

	SPtr<PackageResourceMetaData> metaData = B3DMakeShared<PackageResourceMetaData>();
	metaData->Path = path;
	metaData->Id = id;
	metaData->TypeId = resource != nullptr ? resource->GetTypeId() : 0;
	metaData->ResourceMetaData = resource != nullptr ? resource->GetMetaData() : nullptr;

	if(resource != nullptr)
	{
		const Vector<ResourceDependency> resourceDependencies = Utility::FindResourceDependencies(*resource);
		metaData->Dependencies.reserve(resourceDependencies.size());

		for(const auto& entry : resourceDependencies)
			metaData->Dependencies.push_back(entry.Resource.GetId());
	}
	else
	{
		metaData->Flags.Set(PackageResourceFlag::Folder);
	}

	UPtr<ResourceInformation> resourceInformation = B3DMakeUnique<ResourceInformation>();
	resourceInformation->LoadedResource = resource;
	resourceInformation->MetaData = metaData;
	resourceInformation->LoadState = PackageResourceLoadState::Loaded;
	resourceInformation->LoadProgress = 1.0f;
	resourceInformation->IsLoadedResourceDirty = true;

	Lock lock(mMetaDataMutex);

	if (mResourceInformationByPath.find(path) != mResourceInformationByPath.end())
	{
		B3D_LOG(Error, LogResources, "Cannot register resource in package {0} ({1}) at path {2}. Resource at path already exists. ", mName, mAssociatedPackageFilePath, path);
		return;
	}

	if (mResourceInformationByUUID.find(id) != mResourceInformationByUUID.end())
	{
		B3D_LOG(Error, LogResources, "Cannot register resource in package {0} ({1}) at path {2}. Resource with UUID {3} already exists. ", mName, mAssociatedPackageFilePath, path, id);
	}

	mResourceInformationByPath[path] = resourceInformation.get();
	mResourceInformationByUUID[id] = std::move(resourceInformation);
}

void Package::RemoveResource(const Path& path, bool recursive)
{
	Lock lock(mMetaDataMutex);

	if(!recursive)
	{
		if(const auto found = mResourceInformationByPath.find(path); found != mResourceInformationByPath.end())
		{
			const UUID& id = found->second->MetaData->Id;

			mResourceInformationByPath.erase(found);
			mResourceInformationByUUID.erase(id);
		}
	}
	else
	{
		for (auto it = mResourceInformationByPath.begin(); it != mResourceInformationByPath.end();)
		{
			if (path.Includes(it->first, true))
			{
				const UUID& id = it->second->MetaData->Id;

				it = mResourceInformationByPath.erase(it);
				mResourceInformationByUUID.erase(id);
			}
			else
			{
				++it;
			}
		}
	}
}

void Package::RemoveResource(const UUID& id, bool recursive)
{
	Lock lock(mMetaDataMutex);

	if (const auto found = mResourceInformationByUUID.find(id); found != mResourceInformationByUUID.end())
	{
		const Path& path = found->second->MetaData->Path;

		if(!recursive)
		{
			mResourceInformationByPath.erase(path);
			mResourceInformationByUUID.erase(found);
		}
		else
		{
			RemoveResource(path, recursive);
		}

		return;
	}

	B3D_LOG(Warning, LogResources, "Cannot remove resource {0} from package {1} ({2}). Provided resource is not part of this package.", id, mName, mAssociatedPackageFilePath);
}

void Package::SetResource(const SPtr<Resource>& resource, bool markAsDirty)
{
	if(resource == nullptr)
	{
		B3D_LOG(Error, LogResources, "Cannot change resource in package {0} ({1}). Provided resource is null.", mName, mAssociatedPackageFilePath);
		return;
	}

	const UUID& id = resource->GetId();
	if (id.Empty())
	{
		B3D_LOG(Error, LogResources, "Cannot change resource in package {0} ({1}). Provided resource id is empty.", mName, mAssociatedPackageFilePath);
		return;
	}

	Lock lock(mMetaDataMutex);
	ResourceInformation* const resourceInformation = GetResourceInformation(id);

	if (!resourceInformation)
		return;

	// Wait if load in progress
	resourceInformation->LoadSignal.Wait(lock, [resourceInformation] { return resourceInformation->LoadState != PackageResourceLoadState::InProgress; });

	// Always make a copy and user might be reading from the meta-data, and we cannot modify it in a thread safe way
	const SPtr<PackageResourceMetaData> metaDataCopy = B3DRTTIClone(resourceInformation->MetaData);
	metaDataCopy->TypeId = resource->GetTypeId();
	metaDataCopy->ResourceMetaData = resource->GetMetaData();

	const Vector<ResourceDependency> resourceDependencies = Utility::FindResourceDependencies(*resource);
	metaDataCopy->Dependencies.reserve(resourceDependencies.size());

	for(const auto& entry : resourceDependencies)
		metaDataCopy->Dependencies.push_back(entry.Resource.GetId());

	resourceInformation->LoadedResource = resource;
	resourceInformation->MetaData = metaDataCopy;
	resourceInformation->LoadState = PackageResourceLoadState::Loaded;
	resourceInformation->LoadProgress = 1.0f;
	resourceInformation->IsLoadedResourceDirty = resourceInformation->IsLoadedResourceDirty || markAsDirty;
}

bool Package::SetResourcePath(const UUID& id, const Path& path, bool recursive)
{
	if (id.Empty())
	{
		B3D_LOG(Error, LogResources, "Unable to set resource path '{2}' in package {0} ({1}). Provided resource id is empty.", mName, mAssociatedPackageFilePath, path);
		return false;
	}

	if(path.IsAbsolute())
	{
		B3D_LOG(Error, LogResources, "Unable to set resource path '{2}' in package {0} ({1}). Provided path must be relative. ", mName, mAssociatedPackageFilePath, path);
		return false;
	}

	if(path.IsEmpty())
	{
		B3D_LOG(Error, LogResources, "Unable to set resource path in package {0} ({1}). Provided path must not be empty. ", mName, mAssociatedPackageFilePath);
		return false;
	}

	Lock lock(mMetaDataMutex);
	const ResourceInformation* const resourceInformation = GetResourceInformation(id);

	if (!resourceInformation)
		return false;

	if (resourceInformation->MetaData->Path == path)
		return true;

	if (const auto found = mResourceInformationByPath.find(path); found != mResourceInformationByPath.end())
	{
		B3D_LOG(Error, LogResources, "Unable to set resource path '{2}' in package {0} ({1}). Resource at path already exists. ", mName, mAssociatedPackageFilePath, path);
		return false;
	}

	return SetResourcePath(id, path, recursive);
}

bool Package::SetResourcePath(const Path& path, const Path& newPath, bool recursive)
{
	if(path.IsAbsolute())
	{
		B3D_LOG(Error, LogResources, "Unable to set resource path '{2}' in package {0} ({1}). Provided path must be relative. ", mName, mAssociatedPackageFilePath, path);
		return false;
	}

	if(path.IsEmpty())
	{
		B3D_LOG(Error, LogResources, "Unable to set resource path in package {0} ({1}). Provided path must not be empty. ", mName, mAssociatedPackageFilePath);
		return false;
	}

	if(path == newPath)
		return true;

	Lock lock(mMetaDataMutex);

	auto fnChangePath = [this](ResourceInformation* const resourceInformation, const Path& oldPath, const Path& newPath) -> bool
	{
		if (const auto foundNew = mResourceInformationByPath.find(newPath); foundNew != mResourceInformationByPath.end())
		{
			B3D_LOG(Error, LogResources, "Unable to set resource path '{2}' in package {0} ({1}). Resource at path already exists. ", mName, mAssociatedPackageFilePath, newPath);
			return false;
		}

		// Always make a copy and user might be reading from the meta-data, and we cannot modify it in a thread safe way
		const SPtr<PackageResourceMetaData> metaDataCopy = B3DRTTIClone(resourceInformation->MetaData);
		metaDataCopy->Path = newPath;

		resourceInformation->MetaData = metaDataCopy;
		if(resourceInformation->LoadedResource != nullptr)
		{
			resourceInformation->LoadedResource->SetName(metaDataCopy->GetResourceName());
		}

		mResourceInformationByPath[newPath] = resourceInformation; 
		mResourceInformationByPath.erase(oldPath);

		return true;
	};

	if(!recursive)
	{
		if(const auto foundExisting = mResourceInformationByPath.find(path); foundExisting != mResourceInformationByPath.end())
		{
			ResourceInformation* const resourceInformation = foundExisting->second;

			return fnChangePath(resourceInformation, path, newPath);
		}
	}
	else
	{
		UnorderedMap<Path, ResourceInformation*, PathHashFunction<true>, PathEqualsFunction<true>> resourceInformationCopy = mResourceInformationByPath;
		for(const auto& resourceInformationPair : resourceInformationCopy)
		{
			if(!path.Includes(resourceInformationPair.first, true))
				continue;

			const Path newResourcePath = resourceInformationPair.first.GetRelative(path).GetAbsolute(newPath);
			fnChangePath(resourceInformationPair.second, resourceInformationPair.first, newResourcePath);
		}
	}

	return true;
}

SPtr<Resource> Package::LoadResource(const Path& path)
{
	UUID id;
	{
		Lock lock(mMetaDataMutex);

		if(const auto found = mResourceInformationByPath.find(path); found != mResourceInformationByPath.end())
			id = found->second->MetaData->Id;
		else
		{
			B3D_LOG(Warning, LogResources, "Failed to load resource at path '{2}' from package {0} ({1}). Resource is not found in the package.", mName, mAssociatedPackageFilePath, path);
			return nullptr;
		}
	}

	return LoadResource(id);
}

SPtr<Resource> Package::LoadResource(const UUID& id)
{
	u64 offsetInDataStream = 0;
	u64 sizeInDataStream = 0;
	CompressionType compressionType = CompressionType::Uncompressed;
	bool canStartLoad = false;
	std::atomic<float>* progress = nullptr;
	{
		Lock lock(mMetaDataMutex);

		ResourceInformation* resourceInformation = GetResourceInformation(id);
		if (!resourceInformation)
			return nullptr;

		// Already loaded
		if(resourceInformation->LoadState == PackageResourceLoadState::Loaded)
		{
			B3D_ENSURE(resourceInformation->LoadedResource != nullptr);
			B3D_ENSURE(!resourceInformation->LoadedResource->IsDestroyed()); // Package should be responsible for destroying the resource

			return resourceInformation->LoadedResource;
		}

		// Start load if unloaded
		if (resourceInformation->LoadState == PackageResourceLoadState::Unloaded)
		{
			resourceInformation->LoadProgress.store(0.0f, std::memory_order_relaxed);
			resourceInformation->LoadState = PackageResourceLoadState::InProgress;

			offsetInDataStream = resourceInformation->OffsetInDataStream;
			sizeInDataStream = resourceInformation->SizeInDataStream;
			compressionType = resourceInformation->MetaData->CompressionType;
			progress = &resourceInformation->LoadProgress;

			canStartLoad = true;
		}
		else
		{
			// Wait if in progress
			resourceInformation->LoadSignal.Wait(lock, [resourceInformation] { return resourceInformation->LoadState != PackageResourceLoadState::InProgress; });

			if(resourceInformation->LoadState == PackageResourceLoadState::Loaded)
			{
				B3D_ENSURE(resourceInformation->LoadedResource != nullptr);
				B3D_ENSURE(!resourceInformation->LoadedResource->IsDestroyed()); // Package should be responsible for destroying the resource

				return resourceInformation->LoadedResource;
			}
		}
	}

	if (canStartLoad)
	{
		// Note: It's important that associated ResourceInformation is not destroyed while load is in progress, as we're referencing the progress field
		const SPtr<Resource> resource = LoadAndDeserializeResource(id, offsetInDataStream, sizeInDataStream, compressionType, *progress);

		Lock lock(mMetaDataMutex);

		ResourceInformation* resourceInformation = GetResourceInformation(id);
		if (!resourceInformation)
		{
			// Resource was likely removed from the package by another thread
			return nullptr;
		}

		if (resource == nullptr)
		{
			// Load error occurred, reset the state
			B3D_ASSERT(resourceInformation->LoadState == PackageResourceLoadState::InProgress);
			resourceInformation->LoadState = PackageResourceLoadState::Unloaded;
			resourceInformation->LoadProgress.store(0.0f, std::memory_order_relaxed);

			resourceInformation->LoadSignal.NotifyAll();
			return nullptr;
		}

		B3D_ASSERT(resourceInformation->MetaData != nullptr);
		resource->SetName(resourceInformation->MetaData->GetResourceName());

		B3D_ASSERT(resourceInformation->LoadState == PackageResourceLoadState::InProgress);
		resourceInformation->LoadState = PackageResourceLoadState::Loaded;
		resourceInformation->LoadProgress.store(1.0f, std::memory_order_relaxed);
		resourceInformation->LoadedResource = resource;

		resourceInformation->LoadSignal.NotifyAll();

		return resourceInformation->LoadedResource;
	}

	// Resource was unloaded by another thread, try the load again.
	return LoadResource(id);
}

SPtr<Resource> Package::DeserializeResource(const Path& path) const
{
	UUID id;
	{
		Lock lock(mMetaDataMutex);

		if(const auto found = mResourceInformationByPath.find(path); found != mResourceInformationByPath.end())
			id = found->second->MetaData->Id;
		else
		{
			B3D_LOG(Warning, LogResources, "Failed to deserialize resource at path '{2}' from package {0} ({1}). Resource is not found in the package.", mName, mAssociatedPackageFilePath, path);
			return nullptr;
		}
	}

	return DeserializeResource(id);
}

SPtr<Resource> Package::DeserializeResource(const UUID& id) const
{
	u64 offsetInDataStream;
	u64 sizeInDataStream;
	CompressionType compressionMethod;
	{
		Lock lock(mMetaDataMutex);

		ResourceInformation* resourceInformation = GetResourceInformation(id);
		if (!resourceInformation)
			return nullptr;

		offsetInDataStream = resourceInformation->OffsetInDataStream;
		sizeInDataStream = resourceInformation->SizeInDataStream;
		compressionMethod = resourceInformation->MetaData->CompressionType;
	}

	std::atomic<float> progress;
	SPtr<Resource> resource = LoadAndDeserializeResource(id, offsetInDataStream, sizeInDataStream, compressionMethod, progress);

	if (resource == nullptr)
		return nullptr;

	Lock lock(mMetaDataMutex);

	ResourceInformation* const resourceInformation = GetResourceInformation(id);

	if(!resourceInformation)
	{
		// Resource was likely removed from the package by another thread
		return nullptr;
	}

	B3D_ASSERT(resourceInformation->MetaData != nullptr);
	resource->SetName(resourceInformation->MetaData->GetResourceName());

	return resource;
}

SPtr<Resource> Package::GetResource(const Path& path) const
{
	UUID id;
	{
		Lock lock(mMetaDataMutex);

		if (const auto found = mResourceInformationByPath.find(path); found == mResourceInformationByPath.end())
			id = found->second->MetaData->Id;
		else
		{
			B3D_LOG(Warning, LogResources, "Failed to get resource at path '{2}' from package {0} ({1}). Resource is not found in the package.", mName, mAssociatedPackageFilePath, path);
			return nullptr;
		}
	}

	return GetResource(id);
}

SPtr<Resource> Package::GetResource(const UUID& id) const
{
	Lock lock(mMetaDataMutex);

	ResourceInformation* resourceInformation = GetResourceInformation(id);
	if (!resourceInformation)
	{
		B3D_LOG(Warning, LogResources, "Failed to get resource with id '{2}' from package {0} ({1}). Resource is not found in the package.", mName, mAssociatedPackageFilePath, id);
		return nullptr;
	}

	if (resourceInformation->LoadState == PackageResourceLoadState::Loaded)
	{
		B3D_ENSURE(resourceInformation->LoadedResource != nullptr);
		B3D_ENSURE(!resourceInformation->LoadedResource->IsDestroyed()); // Package should be responsible for destroying the resource

		return resourceInformation->LoadedResource;
	}

	return nullptr;
}

void Package::UnloadResource(const UUID& id)
{
	if(id.Empty())
	{
		B3D_LOG(Warning, LogResources, "Failed to unload resource with from package {0} ({1}). Provided id is empty.", mName, mAssociatedPackageFilePath, id);
		return;
	}

	{
		Lock lock(mMetaDataMutex);

		if(const auto found = mResourceInformationByUUID.find(id); found != mResourceInformationByUUID.end())
		{
			ResourceInformation* resourceInformation = found->second.get();

			// Wait for the load to complete if in progress, we don't have a mechanism to abort the load
			resourceInformation->LoadSignal.Wait(lock, [resourceInformation] { return resourceInformation->LoadState != PackageResourceLoadState::InProgress; });

			if(resourceInformation->LoadState == PackageResourceLoadState::Loaded)
				UnloadResource(resourceInformation);

			return;
		}
	}

	B3D_LOG(Warning, LogResources, "Failed to unload resource with id '{2}' from package {0} ({1}). Resource is not found in the package.", mName, mAssociatedPackageFilePath, id);
}

void Package::UnloadAllResources()
{
	Lock lock(mMetaDataMutex);

	for (const auto& entryPair : mResourceInformationByUUID)
	{
		ResourceInformation *const resourceInformation= entryPair.second.get();

		// Wait for the load to complete if in progress, we don't have a mechanism to abort the load
		resourceInformation->LoadSignal.Wait(lock, [resourceInformation] { return resourceInformation->LoadState != PackageResourceLoadState::InProgress; });

		if (resourceInformation->LoadState == PackageResourceLoadState::Loaded)
			UnloadResource(resourceInformation);
	}
}

size_t Package::GetResourceSizeInDataStream(const UUID& id) const
{
	Lock lock(mMetaDataMutex);
	const ResourceInformation* const resourceInformation= GetResourceInformation(id);

	return resourceInformation ? resourceInformation->SizeInDataStream : 0;
}

SPtr<Resource> Package::LoadAndDeserializeResource(const UUID& id, size_t offsetInStream, size_t sizeInStream, CompressionType compressionType, std::atomic<float>& outProgress) const
{
	SharedLock dataLock(mDataMutex);

	const SPtr<DataStream> dataStream = FileSystem::OpenFile(mAssociatedPackageFilePath);
	if (!dataStream)
	{
		B3D_LOG(Error, LogResources, "Cannot deserialize package resource with id '{2}' in package {0} ({1}). The package has not been serialized or the package file is missing.", mName, mAssociatedPackageFilePath, id, mAssociatedPackageFilePath);
		return nullptr;
	}

	if (!dataStream->IsReadable())
	{
		B3D_LOG(Error, LogResources, "Cannot deserialize package resource with id '{2}' in package {0} ({1}). The data stream from package file is not readable.", mName, mAssociatedPackageFilePath, id);
		return nullptr;
	}

	RTTIOperationEngineContext rttiOperationContext;

	// Read resource data
	SPtr<IReflectable> loadedData;
	dataStream->Seek(offsetInStream);

	if (dataStream->Eof())
	{
		B3D_LOG(Error, LogResources, "Cannot deserialize package resource with id '{2}' in package {0} ({1}). Data stream from package file reached end prematurely.", mName, mAssociatedPackageFilePath, id);
		return nullptr;
	}
	else
	{
		if (compressionType == CompressionType::Uncompressed)
		
		{
			BinarySerializer binarySerializer;
			loadedData = binarySerializer.Decode(dataStream, (u32)sizeInStream, rttiOperationContext, BinarySerializerFlag::None,
				[&outProgress](float progress) { outProgress.exchange(progress, std::memory_order_relaxed); });
		}
		else
		{
			constexpr float kCompressionProgressWeight = 0.9f; // Assuming compression will take 90% of the deserialization time.
			const SPtr<MemoryDataStream> uncompressedStream = B3DMakeShared<MemoryDataStream>();

			const bool decompressionSuccessful = Compression::Decompress(*dataStream, *uncompressedStream, sizeInStream, compressionType, [&outProgress, kCompressionProgressWeight](float progress) {
				outProgress.exchange(progress * kCompressionProgressWeight, std::memory_order_relaxed);
			});

			uncompressedStream->Seek(0);

			if (decompressionSuccessful)
			{
				BinarySerializer bs;
				loadedData = bs.Decode(uncompressedStream, (u32)uncompressedStream->Size(), rttiOperationContext, BinarySerializerFlag::None,
									   [&outProgress, kCompressionProgressWeight](float progress) {
										   outProgress.exchange(kCompressionProgressWeight + progress * (1.0f - kCompressionProgressWeight), std::memory_order_relaxed);
									   });
			}
			else
			{
				B3D_LOG(Error, LogResources, "Cannot deserialize package resource with id '{2}' in package {0} ({1}). Data decompression failed.", mName, mAssociatedPackageFilePath, id);
				return nullptr;
			}
		}
	}

	if (loadedData == nullptr)
	{
		B3D_LOG(Error, LogResources, "Cannot deserialize package resource with id '{2}' in package {0} ({1}). Unknown deserialization error.", mName, mAssociatedPackageFilePath, id);
		return nullptr;
	}
	else
	{
		if (!loadedData->IsDerivedFrom(Resource::GetRttiStatic()))
		{
			B3D_LOG(Error, LogResources, "Cannot deserialize package resource with id '{2}' in package {0} ({1}). Deserialized object is not a Resource type.", mName, mAssociatedPackageFilePath, id);
			return nullptr;
		}
	}

	return std::static_pointer_cast<Resource>(loadedData);
}

void Package::UnloadResource(ResourceInformation* resourceInformation)
{
	if (resourceInformation->SizeInDataStream == 0)
		B3D_LOG(Error, LogResources, "Resource with id '{2}' in package {0} ({1}) is being unloaded but it was never saved. You will not be able to reload this resource.", mName, mAssociatedPackageFilePath, resourceInformation->MetaData->Id);

	if (resourceInformation->IsLoadedResourceDirty)
		B3D_LOG(Error, LogResources, "Resource with id '{2}' in package {0} ({1}) is being unloaded but it has changes that were never saved. You will not be able to reload this resource.", mName, mAssociatedPackageFilePath, resourceInformation->MetaData->Id);

	resourceInformation->LoadedResource = nullptr;
	resourceInformation->LoadProgress.store(0.0f, std::memory_order_relaxed);
	resourceInformation->LoadState = PackageResourceLoadState::Unloaded;
	resourceInformation->LoadSignal.NotifyAll();
}

SPtr<Package> Package::Create(const String& name, const UUID& id)
{
	return B3DMakeShared<Package>(name, id);
}

bool Package::Save(const SPtr<DataStream>& stream, const SavePackageOptions& options)
{
	Lock metaDataLock(mMetaDataMutex);

	const u32 resourceCount = (u32)mResourceInformationByUUID.size();
	CompressionType* savedCompressionTypesPerResource = B3DStackAllocate<CompressionType>(resourceCount);

	auto fnCleanupStack = [savedCompressionTypesPerResource]()
	{
		B3DStackFree(savedCompressionTypesPerResource);
	};

	ScopeGuard cleanupStack(fnCleanupStack);

	constexpr CompressionType kDefaultCompressionType = CompressionType::Default;

	u32 resourceIndex = 0;
	for (auto& entryPair : mResourceInformationByUUID)
	{
		ResourceInformation* const resourceInformation = entryPair.second.get();
		const SPtr<PackageResourceMetaData>& metaData = resourceInformation->MetaData;

		savedCompressionTypesPerResource[resourceIndex] = metaData->CompressionType;

		// Always make a copy and user might be reading from the meta-data, and we cannot modify it in a thread safe way
		const SPtr<PackageResourceMetaData> metaDataCopy = B3DMakeShared<PackageResourceMetaData>();
		*metaDataCopy = *resourceInformation->MetaData;

		metaDataCopy->CompressionType = options.CompressResources ? kDefaultCompressionType : CompressionType::Uncompressed;
		entryPair.second->MetaData = metaDataCopy;

		resourceIndex++;
	}

	if(options.SaveMetaDataOnly)
	{
		// Check if the new meta-data fits
		SPtr<MemoryDataStream> metaDataStream = B3DMakeShared<MemoryDataStream>();
		FileEncoder metaDataEncoder(metaDataStream);
		metaDataEncoder.Encode(this);	

		const u64 metaDataSize = (u64)metaDataStream->Tell();
		if(metaDataSize > mSerializedMetaDataEnd)
			return false;

		mMetaDataPaddingByteCount = mSerializedMetaDataEnd - metaDataSize;
	}
	else
		mMetaDataPaddingByteCount = options.MetaDataPaddingByteCount;

	// Package structure:
	// 1. Meta data
	// 2. Per-resource headers (uuid, offset, size)
	// 3. Per-resource data

	FileEncoder fileEncoder(stream);
	fileEncoder.Encode(this);

	if(mMetaDataPaddingByteCount > 0)
		B3D_ENSURE(stream->Skip(mMetaDataPaddingByteCount) == mMetaDataPaddingByteCount);

	if (options.SaveMetaDataOnly)
		return true;

	BinarySerializer serializer;

	const size_t dataBlockOffset = stream->Tell(); // After meta-data (plus optional padding)
	const size_t resourceHeaderSize = resourceCount * sizeof(SerializedResourceHeader);

	stream->Skip(resourceHeaderSize);

	const SPtr<DataStream> existingPackageFileStream = !mAssociatedPackageFilePath.IsEmpty() ? FileSystem::OpenFile(mAssociatedPackageFilePath) : nullptr;

	FrameAllocatorScope frameScope;
	FrameVector<SerializedResourceHeader> resourceHeaders(resourceCount);

	resourceIndex = 0;
	for (auto& entry : mResourceInformationByUUID)
	{
		const SPtr<Resource>& loadedResource = entry.second->LoadedResource;
		const CompressionType compressionType = options.CompressResources ? kDefaultCompressionType : CompressionType::Uncompressed;

		resourceHeaders[resourceIndex].Id = entry.first;
		resourceHeaders[resourceIndex].OffsetInDataStream = stream->Tell();

		if (loadedResource != nullptr && (entry.second->IsLoadedResourceDirty || entry.second->SizeInDataStream == 0 || existingPackageFileStream == nullptr))
		{
			if (compressionType != CompressionType::Uncompressed)
			{
				SPtr<MemoryDataStream> serializedResourceStream = B3DMakeShared<MemoryDataStream>();
				serializer.Encode(loadedResource.get(), serializedResourceStream);
				serializedResourceStream->Seek(0);

				resourceHeaders[resourceIndex].SizeInDataStream = Compression::Compress(*serializedResourceStream, *stream, 0, compressionType);
			}
			else
			{
				serializer.Encode(loadedResource.get(), stream);
				resourceHeaders[resourceIndex].SizeInDataStream = stream->Tell() - resourceHeaders[resourceIndex].OffsetInDataStream;
			}
		}
		else
		{
			SharedLock dataLock(mDataMutex);

			if (!existingPackageFileStream || entry.second->SizeInDataStream == 0)
			{
				B3D_LOG(Error, LogResources, "Cannot save resource with id {2} to package {0} ({1}). Original data for resource cannot be found in the associated package file.", mName, mAssociatedPackageFilePath, entry.second->MetaData->Id);
				resourceHeaders[resourceIndex].SizeInDataStream = 0;
			}
			else
			{
				if (savedCompressionTypesPerResource[resourceIndex] == compressionType)
				{
					existingPackageFileStream->Seek(entry.second->OffsetInDataStream);

					static constexpr u32 kBufferSize = 32768;
					u8* chunkBuffer = (u8*)B3DStackAllocate(kBufferSize);

					u64 remainingByteCount = entry.second->SizeInDataStream;
					while(remainingByteCount > 0)
					{
						const u64 bytesToRead = Math::Min(kBufferSize, remainingByteCount);
						const u64 readByteCount = existingPackageFileStream->Read(chunkBuffer, bytesToRead);
						B3D_ENSURE(bytesToRead == readByteCount);

						stream->Write(chunkBuffer, readByteCount);
						remainingByteCount -= bytesToRead;
					}

					B3DStackFree(chunkBuffer);

					resourceHeaders[resourceIndex].SizeInDataStream = entry.second->SizeInDataStream;
				}
				else
				{
					existingPackageFileStream->Seek(entry.second->OffsetInDataStream);

					u64 uncompressedDataSize;
					SPtr<DataStream> uncompressedDataStream;
					if(savedCompressionTypesPerResource[resourceIndex] != CompressionType::Uncompressed)
					{
						uncompressedDataStream = B3DMakeShared<MemoryDataStream>();
						if(!Compression::Decompress(*existingPackageFileStream, *uncompressedDataStream, entry.second->SizeInDataStream, savedCompressionTypesPerResource[resourceIndex]))
						{
							B3D_LOG(Error, LogResources, "Cannot save resource with id {2} to package {0} ({1}). Decompression failed for original resource data.", mName, mAssociatedPackageFilePath, entry.second->MetaData->Id);
							continue;
							
						}

						uncompressedDataStream->Seek(0);
						uncompressedDataSize = uncompressedDataStream->Size();
					}
					else
					{
						uncompressedDataStream = existingPackageFileStream;
						uncompressedDataSize = entry.second->SizeInDataStream;
					}

					if (compressionType != CompressionType::Uncompressed)
					{
						resourceHeaders[resourceIndex].SizeInDataStream = Compression::Compress(*uncompressedDataStream, *stream, uncompressedDataSize, compressionType);
					}
					else
					{
						B3D_ASSERT(existingPackageFileStream != uncompressedDataStream && !uncompressedDataStream->IsFile()); // Must be a temporary uncompressed memory stream

						const SPtr<MemoryDataStream> uncompressedMemoryDataStream = std::static_pointer_cast<MemoryDataStream>(uncompressedDataStream);
						stream->Write(uncompressedMemoryDataStream->Data(), uncompressedMemoryDataStream->Size());
						resourceHeaders[resourceIndex].SizeInDataStream = uncompressedMemoryDataStream->Size();
					}
				}
			}
		}

		entry.second->OffsetInDataStream = resourceHeaders[resourceIndex].OffsetInDataStream;
		entry.second->SizeInDataStream = resourceHeaders[resourceIndex].SizeInDataStream;
		entry.second->IsLoadedResourceDirty = false;

		resourceIndex++;
	}

	if(existingPackageFileStream != nullptr)
		existingPackageFileStream->Close();

	stream->Seek(dataBlockOffset);
	stream->Write(resourceHeaders.data(), resourceHeaders.size() * sizeof(SerializedResourceHeader));

	return true;
}

void Package::AssociateFileWithPackage(const Path& path)
{
	UniqueLock lock(mDataMutex);
	mAssociatedPackageFilePath = path;
}

SPtr<Package> Package::Clone() const
{
	Lock lock(mMetaDataMutex);

	SPtr<Package> output = Create(mName, mId);
	output->mAssociatedPackageFilePath = mAssociatedPackageFilePath;
	output->mPackageMetaData = B3DRTTIClone(mPackageMetaData, false);

	for (const auto& entry : mResourceInformationByUUID)
	{
		// We cannot clone a package with in-progress loads, so we need to wait for them to complete
		const ResourceInformation* const resourceInformation = entry.second.get();
		resourceInformation->LoadSignal.Wait(lock, [resourceInformation] { return resourceInformation->LoadState != PackageResourceLoadState::InProgress; });

		B3D_ASSERT(entry.second->LoadState != PackageResourceLoadState::InProgress);

		UPtr<ResourceInformation> resourceInformationClone = B3DMakeUnique<ResourceInformation>();
		resourceInformationClone->MetaData = B3DRTTIClone(resourceInformation->MetaData);
		resourceInformationClone->LoadState = resourceInformation->LoadState;
		resourceInformationClone->LoadProgress.store(resourceInformation->LoadProgress.load());
		resourceInformationClone->OffsetInDataStream = resourceInformation->OffsetInDataStream;
		resourceInformationClone->SizeInDataStream = resourceInformation->SizeInDataStream;
		resourceInformationClone->LoadedResource = resourceInformation->LoadedResource;
		resourceInformationClone->IsLoadedResourceDirty = resourceInformation->IsLoadedResourceDirty;

		output->mResourceInformationByPath[resourceInformationClone->MetaData->Path] = resourceInformationClone.get();
		output->mResourceInformationByUUID[resourceInformationClone->MetaData->Id] = std::move(resourceInformationClone);
	}

	return output;
}

void Package::CopyResourceLoadStatesFromClone(const Package& otherPackage)
{
	Lock lock(mMetaDataMutex);

	for (const auto& entry : otherPackage.mResourceInformationByUUID)
	{
		auto found = mResourceInformationByUUID.find(entry.first);
		if (found == mResourceInformationByUUID.end())
			continue;

		const UPtr<ResourceInformation>& otherResourceInfo = entry.second;
		B3D_ASSERT(otherResourceInfo != nullptr);
		B3D_ASSERT(otherResourceInfo->LoadState != PackageResourceLoadState::InProgress);

		const UPtr<ResourceInformation>& resourceInformation= found->second;
		B3D_ASSERT(resourceInformation!= nullptr);

		if (resourceInformation->LoadState != otherResourceInfo->LoadState)
		{
			if (resourceInformation->LoadState == PackageResourceLoadState::Unloaded)
			{
				B3D_ASSERT(otherResourceInfo->LoadedResource != nullptr);
				resourceInformation->LoadedResource = otherResourceInfo->LoadedResource;
			}
			else
			{
				resourceInformation->LoadedResource = nullptr;
			}

			resourceInformation->LoadState = otherResourceInfo->LoadState;
		}

		resourceInformation->LoadProgress.store(otherResourceInfo->LoadProgress.load());
	}
}

SPtr<Package> Package::Load(const Path& path)
{
	const SPtr<DataStream> stream = FileSystem::OpenFile(path);
	if (!stream)
		return nullptr;

	SPtr<Package> package = Load(stream);
	if(package)
		package->mAssociatedPackageFilePath = path;

	return package;
}

SPtr<Package> Package::Load(const SPtr<DataStream>& stream)
{
	FileDecoder fileDecoder(stream);
	SPtr<Package> package = B3DRTTICast<Package>(fileDecoder.Decode());
	if (package == nullptr)
		return nullptr;

	stream->Skip(package->mMetaDataPaddingByteCount);

	package->mSerializedMetaDataEnd = stream->Tell() + package->mMetaDataPaddingByteCount;
	const u32 resourceCount = (u32)package->mResourceInformationByUUID.size();

	Vector<SerializedResourceHeader> headers(resourceCount);
	stream->Read(headers.data(), headers.size() * sizeof(SerializedResourceHeader));

	for (const auto& entry : headers)
	{
		auto found = package->mResourceInformationByUUID.find(entry.Id);
		B3D_ASSERT(found != package->mResourceInformationByUUID.end());

		ResourceInformation* resourceInformation = found->second.get();
		resourceInformation->OffsetInDataStream = entry.OffsetInDataStream;
		resourceInformation->SizeInDataStream = entry.SizeInDataStream;
	}

	return package;
}

Package::ResourceInformation* Package::GetResourceInformation(const UUID& id, bool warnIfMissing) const
{
	if(const auto found = mResourceInformationByUUID.find(id); found != mResourceInformationByUUID.end())
		return found->second.get();

	if(warnIfMissing)
	{
		B3D_LOG(Warning, LogResources, "Cannot find information about a packaged resource with UUID {0} in package {1} ({2}). The resource is not part of this package.", id, mName, mAssociatedPackageFilePath);
	}

	return nullptr;
}

Package::ResourceInformation* Package::GetResourceInformation(const Path& path, bool warnIfMissing) const
{
	if(const auto found = mResourceInformationByPath.find(path); found != mResourceInformationByPath.end())
		return found->second;

	if(warnIfMissing)
	{
		B3D_LOG(Warning, LogResources, "Cannot find information about a packaged resource with path '{0}' in package {1} ({2}). The resource is not part of this package.", path, mName, mAssociatedPackageFilePath);
	}

	return nullptr;
}

RTTIType* Package::GetRttiStatic()
{
	return PackageRTTI::Instance();
}

RTTIType* Package::GetRtti() const
{
	return GetRttiStatic();
}
