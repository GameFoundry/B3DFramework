//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DReflectionProbe.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "Renderer/B3DRenderer.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ReflectionProbeRTTI : public TRTTIType<ReflectionProbe, Component, ReflectionProbeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mType, 0)
			B3D_RTTI_MEMBER(mRadius, 1)
			B3D_RTTI_MEMBER(mExtents, 2)
			B3D_RTTI_MEMBER(mTransitionDistance, 3)
			B3D_RTTI_MEMBER(mCustomTexture, 4)
			B3D_RTTI_MEMBER(mFilteredTexture, 5)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ReflectionProbe& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				// Force the renderer task to complete, so the filtered texture is up to date
				if(object.mRendererTask != nullptr)
					object.mRendererTask->Wait();
			}
		}

		const String& GetRttiName() override
		{
			static String name = "ReflectionProbe";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ReflectionProbe;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<ReflectionProbe>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
