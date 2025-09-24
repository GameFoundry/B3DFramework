//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsUUIDRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
#include "Scene/BsSceneObjectHierarchyDelta.h"
#include "Serialization/BsSerializedObject.h"
#include "Scene/BsGameObjectManager.h"
#include "Serialization/BsBinarySerializer.h"
#include "Utility/BsUtility.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT SceneObjectHierarchyDeltaObjectRTTI : public TRTTIType<SceneObjectHierarchyDeltaObject, IReflectable, SceneObjectHierarchyDeltaObjectRTTI>
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

	class B3D_CORE_EXPORT SceneObjectHierarchyDeltaRTTI : public TRTTIType<SceneObjectHierarchyDelta, IReflectable, SceneObjectHierarchyDeltaRTTI>
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
