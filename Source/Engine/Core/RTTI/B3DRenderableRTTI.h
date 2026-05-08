//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIECSField.h"
#include "Components/B3DRenderable.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d::ecs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ECSRenderableRTTI : public TRTTIType<Renderable, IReflectable, ECSRenderableRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Mesh, 0)
			B3D_RTTI_MEMBER(Layer, 1)
			B3D_RTTI_MEMBER_CONTAINER(Materials, 2)
			B3D_RTTI_MEMBER(CullDistanceFactor, 3)
			B3D_RTTI_MEMBER(WriteVelocity, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ECSRenderable";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ECSRenderable;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<Renderable>();
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

	class B3D_EXPORT RenderableRTTI : public TRTTIType<Renderable, Component, RenderableRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_ECS(Renderable, 0)
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

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Renderable>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
