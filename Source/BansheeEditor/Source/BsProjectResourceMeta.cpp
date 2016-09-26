//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsProjectResourceMeta.h"
#include "BsProjectResourceMetaRTTI.h"

namespace BansheeEngine
{
	ProjectResourceMeta::ProjectResourceMeta(const ConstructPrivately& dummy)
		:mTypeId(0)
	{

	}

	SPtr<ProjectResourceMeta> ProjectResourceMeta::create(const WString& name, const String& uuid, UINT32 typeId,
		const SPtr<ResourceMetaData>& resourceMetaData)
	{
		SPtr<ProjectResourceMeta> meta = bs_shared_ptr_new<ProjectResourceMeta>(ConstructPrivately());
		meta->mName = name;
		meta->mUUID = uuid;
		meta->mTypeId = typeId;
		meta->mResourceMeta = resourceMetaData;

		return meta;
	}

	SPtr<ProjectResourceMeta> ProjectResourceMeta::createEmpty()
	{
		return bs_shared_ptr_new<ProjectResourceMeta>(ConstructPrivately());
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* ProjectResourceMeta::getRTTIStatic()
	{
		return ProjectResourceMetaRTTI::instance();
	}

	RTTITypeBase* ProjectResourceMeta::getRTTI() const
	{
		return ProjectResourceMeta::getRTTIStatic();
	}

	ProjectFileMeta::ProjectFileMeta(const ConstructPrivately& dummy)
		:mIncludeInBuild(false)
	{

	}

	SPtr<ProjectFileMeta> ProjectFileMeta::create(const SPtr<ImportOptions>& importOptions)
	{
		SPtr<ProjectFileMeta> meta = bs_shared_ptr_new<ProjectFileMeta>(ConstructPrivately());
		meta->mImportOptions = importOptions;
		meta->mIncludeInBuild = false;

		return meta;
	}

	void ProjectFileMeta::add(const SPtr<ProjectResourceMeta>& resourceMeta)
	{
		mResourceMetaData.push_back(resourceMeta);
	}

	void ProjectFileMeta::addInactive(const SPtr<ProjectResourceMeta>& resourceMeta)
	{
		mInactiveResourceMetaData.push_back(resourceMeta);
	}

	Vector<SPtr<ProjectResourceMeta>> ProjectFileMeta::getAllResourceMetaData() const
	{
		Vector<SPtr<ProjectResourceMeta>> output(mResourceMetaData);

		for (auto& entry : mInactiveResourceMetaData)
			output.push_back(entry);

		return output;
	}

	bool ProjectFileMeta::hasTypeId(UINT32 typeId) const
	{
		for(auto& entry : mResourceMetaData)
		{
			if (entry->getTypeID() == typeId)
				return true;
		}

		return false;
	}

	bool ProjectFileMeta::hasUUID(const String& uuid) const
	{
		for (auto& entry : mResourceMetaData)
		{
			if (entry->getUUID() == uuid)
				return true;
		}

		return false;
	}

	SPtr<ProjectFileMeta> ProjectFileMeta::createEmpty()
	{
		return bs_shared_ptr_new<ProjectFileMeta>(ConstructPrivately());
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* ProjectFileMeta::getRTTIStatic()
	{
		return ProjectFileMetaRTTI::instance();
	}

	RTTITypeBase* ProjectFileMeta::getRTTI() const
	{
		return ProjectFileMeta::getRTTIStatic();
	}
}