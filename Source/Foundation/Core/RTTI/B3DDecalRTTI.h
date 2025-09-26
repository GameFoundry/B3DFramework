//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DDecal.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT DecalRTTI : public TRTTIType<Decal, Component, DecalRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mSize, 0)
			B3D_RTTI_MEMBER(mMaxDistance, 1)
			B3D_RTTI_MEMBER(mMaterial, 2)
			B3D_RTTI_MEMBER(mLayer, 3)
			B3D_RTTI_MEMBER(mLayerMask, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "Decal";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Decal;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Decal>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
