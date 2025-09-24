//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPhysicsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "BsNullPhysicsMesh.h"
#include "FileSystem/BsDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-NullPhysics
	 *  @{
	 */

	class FNullPhysicsMeshRTTI : public RTTIType<FNullPhysicsMesh, FPhysicsMesh, FNullPhysicsMeshRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "FNullPhysicsMesh";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_FNullPhysicsMesh;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FNullPhysicsMesh>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
