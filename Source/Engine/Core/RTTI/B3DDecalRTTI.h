//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIECSField.h"
#include "Components/B3DDecal.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d::ecs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ECSDecalRTTI : public TRTTIType<Decal, IReflectable, ECSDecalRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Size, 0)
			B3D_RTTI_MEMBER(MaxDistance, 1)
			B3D_RTTI_MEMBER(Material, 2)
			B3D_RTTI_MEMBER(Layer, 3)
			B3D_RTTI_MEMBER(LayerMask, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ECSDecal";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ECSDecal;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<Decal>();
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

	class B3D_EXPORT DecalRTTI : public TRTTIType<Decal, Component, DecalRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_ECS(Decal, 0)
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

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Decal>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
