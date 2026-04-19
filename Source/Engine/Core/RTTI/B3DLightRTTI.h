//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIECSField.h"
#include "Components/B3DLight.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DColorRTTI.h"

namespace b3d::ecs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ECSLightRTTI : public TRTTIType<Light, IReflectable, ECSLightRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Type, 0)
			B3D_RTTI_MEMBER(CastsShadows, 1)
			B3D_RTTI_MEMBER(LightColor, 2)
			B3D_RTTI_MEMBER(AttRadius, 3)
			B3D_RTTI_MEMBER(Intensity, 4)
			B3D_RTTI_MEMBER(SpotAngle, 5)
			B3D_RTTI_MEMBER(SpotFalloffAngle, 6)
			B3D_RTTI_MEMBER(AutoAttenuation, 7)
			B3D_RTTI_MEMBER(SourceRadius, 8)
			B3D_RTTI_MEMBER(ShadowBias, 9)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ECSLight";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ECSLight;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<Light>();
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

	class B3D_EXPORT LightRTTI : public TRTTIType<Light, Component, LightRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_ECS(Light, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "Light";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Light;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Light>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
