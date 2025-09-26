//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DRenderable.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT RenderableRTTI : public TRTTIType<Renderable, Component, RenderableRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mMesh, 0)
			B3D_RTTI_MEMBER(mLayer, 1)
			B3D_RTTI_MEMBER_CONTAINER(mMaterials, 2)
			B3D_RTTI_MEMBER(mCullDistanceFactor, 3)
			B3D_RTTI_MEMBER(mWriteVelocity, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "Renderable";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Renderable;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Renderable>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
