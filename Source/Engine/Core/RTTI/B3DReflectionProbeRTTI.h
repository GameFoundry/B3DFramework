//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIECSField.h"
#include "Components/B3DReflectionProbe.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "Renderer/B3DRenderer.h"
#include "ECS/B3DRegistry.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d::ecs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ECSReflectionProbeRTTI : public TRTTIType<ReflectionProbe, IReflectable, ECSReflectionProbeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Type, 0)
			B3D_RTTI_MEMBER(Radius, 1)
			B3D_RTTI_MEMBER(Extents, 2)
			B3D_RTTI_MEMBER(TransitionDistance, 3)
			B3D_RTTI_MEMBER(FilteredTexture, 4)
			B3D_RTTI_MEMBER(CustomTexture, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ECSReflectionProbe";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ECSReflectionProbe;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ReflectionProbe>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d::ecs

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
			B3D_RTTI_MEMBER_ECS(ReflectionProbe, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ReflectionProbe& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				// Force any pending capture task to complete, so the filtered texture is up to date
				ecs::Registry* registry = object.GetECSRegistry();
				ecs::Entity entity = object.GetECSEntity();

				if(registry->HasAllOf<ecs::ReflectionProbe>(entity))
				{
					ecs::ReflectionProbe& fragment = registry->GetComponents<ecs::ReflectionProbe>(entity);
					if(fragment.PendingTask != nullptr)
						fragment.PendingTask->Wait();
				}
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
