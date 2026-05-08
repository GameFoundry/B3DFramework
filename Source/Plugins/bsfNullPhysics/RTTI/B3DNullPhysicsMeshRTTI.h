//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DNullPhysicsMesh.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-NullPhysics
	 *  @{
	 */

	class NullPhysicsMeshImplementationRTTI : public TRTTIType<NullPhysicsMeshImplementation, IPhysicsMeshImplementation, NullPhysicsMeshImplementationRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "NullPhysicsMeshImplementation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FNullPhysicsMesh;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<NullPhysicsMeshImplementation>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
