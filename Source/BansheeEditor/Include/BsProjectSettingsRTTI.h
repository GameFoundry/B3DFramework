//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsProjectSettings.h"
#include "BsRTTIType.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class ProjectSettingsRTTI : public RTTIType <ProjectSettings, Settings, ProjectSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mLastOpenScene, 0)
		BS_END_RTTI_MEMBERS
	public:
		ProjectSettingsRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ProjectSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ProjectSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ProjectSettings>();
		}
	};

	/** @} */
	/** @endcond */
}