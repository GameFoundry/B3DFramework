//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsUUIDRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObjectHierarchyDelta.h"
#include "Serialization/BsSerializedObject.h"
#include "Scene/BsGameObjectManager.h"
#include "Serialization/BsBinarySerializer.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ComponentDeltaRTTI : public RTTIType<ComponentDelta, IReflectable, ComponentDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Id, 0)
			B3D_RTTI_MEMBER_REFLPTR(Data, 1)
			B3D_RTTI_MEMBER_PLAIN(ParentId, 2)
			B3D_RTTI_MEMBER_PLAIN(PrefabObjectId, 3)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "ComponentDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ComponentDelta;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ComponentDelta>();
		}
	};

	class B3D_CORE_EXPORT SceneObjectDeltaRTTI : public RTTIType<SceneObjectDelta, IReflectable, SceneObjectDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Id, 0)
			B3D_RTTI_MEMBER_PLAIN(Name, 1)

			B3D_RTTI_MEMBER_REFLPTR_ARRAY(ComponentDeltas, 2)
			B3D_RTTI_MEMBER_PLAIN_ARRAY(RemovedComponents, 3)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(AddedComponents, 4)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(ChildDeltas, 5)

			B3D_RTTI_MEMBER_PLAIN_ARRAY(RemovedChildren, 6)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(AddedChildren, 7)

			B3D_RTTI_MEMBER_PLAIN(Position, 8)
			B3D_RTTI_MEMBER_PLAIN(Rotation, 9)
			B3D_RTTI_MEMBER_PLAIN(Scale, 10)
			B3D_RTTI_MEMBER_PLAIN(IsActive, 11)
			B3D_RTTI_MEMBER_PLAIN(SoFlags, 12)

			B3D_RTTI_MEMBER_PLAIN(ParentId, 13)
			B3D_RTTI_MEMBER_PLAIN(PrefabObjectId, 14)
			B3D_RTTI_MEMBER_PLAIN(PrefabResourceId, 15)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "SceneObjectDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SceneObjectDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SceneObjectDelta>();
		}
	};

	class B3D_CORE_EXPORT SceneObjectHierarchyDeltaRTTI : public RTTIType<SceneObjectHierarchyDelta, IReflectable, SceneObjectHierarchyDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFLPTR(mRoot, 0)
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
} // namespace bs
