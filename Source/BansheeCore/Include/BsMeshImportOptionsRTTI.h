//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsMeshImportOptions.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT MeshImportOptionsRTTI : public RTTIType <MeshImportOptions, ImportOptions, MeshImportOptionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mCPUReadable, 0)
			BS_RTTI_MEMBER_PLAIN(mImportNormals, 1)
			BS_RTTI_MEMBER_PLAIN(mImportTangents, 2)
			BS_RTTI_MEMBER_PLAIN(mImportBlendShapes, 3)
			BS_RTTI_MEMBER_PLAIN(mImportSkin, 4)
			BS_RTTI_MEMBER_PLAIN(mImportAnimation, 5)
			BS_RTTI_MEMBER_PLAIN(mImportScale, 6)
			BS_RTTI_MEMBER_PLAIN(mCollisionMeshType, 7)
		BS_END_RTTI_MEMBERS
	public:
		MeshImportOptionsRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "MeshImportOptions";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_MeshImportOptions;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<MeshImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
}