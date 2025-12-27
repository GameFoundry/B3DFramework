//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "Scene/B3DSceneObjectHierarchyDelta.h"
#include "Serialization/B3DSerializedObject.h"
#include "Scene/B3DGameObjectManager.h"
#include "Serialization/B3DBinarySerializer.h"
#include "Utility/B3DUtility.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SceneObjectHierarchyDeltaObjectRTTI : public TRTTIType<SceneObjectHierarchyDeltaObject, IReflectable, SceneObjectHierarchyDeltaObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Id, 0)
			B3D_RTTI_MEMBER(ParentId, 1)
			B3D_RTTI_MEMBER(PrefabObjectId, 2)
			B3D_RTTI_MEMBER(PrefabResourceId, 3)
			B3D_RTTI_MEMBER(Data, 4)
			B3D_RTTI_MEMBER(Flags, 5)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "SceneObjectHierarchyDeltaObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SceneObjectHierarchyDeltaObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SceneObjectHierarchyDeltaObject>();
		}
	};

	class B3D_EXPORT SceneObjectHierarchyDeltaRTTI : public TRTTIType<SceneObjectHierarchyDelta, IReflectable, SceneObjectHierarchyDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Objects, 0)
			B3D_RTTI_MEMBER_CONTAINER(AddedSceneObjects, 1)
			B3D_RTTI_MEMBER_CONTAINER(RemovedSceneObjects, 2)
			B3D_RTTI_MEMBER_CONTAINER(AddedComponents, 3)
			B3D_RTTI_MEMBER_CONTAINER(RemovedComponents, 4)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "SceneObjectHierarchyDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SceneObjectHierarchyDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SceneObjectHierarchyDelta>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
