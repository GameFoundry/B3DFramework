//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsProjectResourceMeta.h"
#include "BsResourceMetaData.h"
#include "BsImportOptions.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class ProjectResourceMetaRTTI : public RTTIType<ProjectResourceMeta, IReflectable, ProjectResourceMetaRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mName, 0)
			BS_RTTI_MEMBER_PLAIN(mUUID, 1)
			BS_RTTI_MEMBER_PLAIN(mTypeId, 2)
			BS_RTTI_MEMBER_REFLPTR(mResourceMeta, 3)
			BS_RTTI_MEMBER_REFLPTR(mUserData, 4)
		BS_END_RTTI_MEMBERS
	public:
		ProjectResourceMetaRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ProjectResourceMeta";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ProjectResourceMeta;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ProjectResourceMeta::createEmpty();
		}
	};

	class ProjectFileMetaRTTI : public RTTIType<ProjectFileMeta, IReflectable, ProjectFileMetaRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mImportOptions, 1)
			BS_RTTI_MEMBER_PLAIN(mIncludeInBuild, 4)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mResourceMetaData, 5)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mInactiveResourceMetaData, 6)
		BS_END_RTTI_MEMBERS

	public:
		ProjectFileMetaRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ProjectFileMeta";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ProjectFileMeta;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ProjectFileMeta::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}