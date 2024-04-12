//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsGameObjectHandle.h"
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsComponent.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "Scene/BsGameObjectCollection.h"
#include "Scene/BsSceneObjectHierarchyDelta.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT SceneObjectRTTI : public RTTIType<SceneObject, GameObject, SceneObjectRTTI>
	{
	private:
		Transform& GetTransform(SceneObject* obj) { return obj->mWorldTfrm; }

		void SetTransform(SceneObject* obj, Transform& value) { obj->mWorldTfrm = value; }

		Transform& GetLocalTransform(SceneObject* obj) { return obj->mLocalTfrm; }

		void SetLocalTransform(SceneObject* obj, Transform& value) { obj->mLocalTfrm = value; }

		bool& GetActive(SceneObject* obj) { return obj->mActiveSelf; }

		void SetActive(SceneObject* obj, bool& value) { obj->mActiveSelf = value; }

		SPtr<SceneObject> GetChild(SceneObject* obj, u32 idx) { return obj->mChildren[idx].GetShared(); }

		void SetChild(SceneObject* obj, u32 idx, SPtr<SceneObject> param)
		{
			// It's important that child indices remain the same after deserialization, as some systems (like SO
			// record/restore) depend on it
			if(idx >= mChildren.size())
				mChildren.resize(idx + 1);

			mChildren[idx] = param;
		}

		u32 GetNumChildren(SceneObject* obj) { return (u32)obj->mChildren.size(); }

		void SetNumChildren(SceneObject* obj, u32 size)
		{ /* DO NOTHING */
		}

		// NOTE - These can only be set sequentially, specific array index is ignored
		SPtr<Component> GetComponent(SceneObject* obj, u32 idx) { return obj->mComponents[idx].GetShared(); }

		void SetComponent(SceneObject* obj, u32 idx, SPtr<Component> param)
		{
			// It's important that child indices remain the same after deserialization, as some systems (like SO
			// record/restore) depend on it
			if(idx >= mComponents.size())
				mComponents.resize(idx + 1);

			mComponents[idx] = param;
		}

		u32 GetNumComponents(SceneObject* obj) { return (u32)obj->mComponents.size(); }

		void SetNumComponents(SceneObject* obj, u32 size)
		{ /* DO NOTHING */
		}

		SPtr<SceneObjectHierarchyDelta> GetPrefabDelta(SceneObject* obj) { return obj->mPrefabDelta; }

		void SetPrefabDelta(SceneObject* obj, SPtr<SceneObjectHierarchyDelta> value) { obj->mPrefabDelta = value; }

		u32& GetFlags(SceneObject* obj) { return obj->mFlags; }

		void SetFlags(SceneObject* obj, u32& value) { obj->mFlags = value; }

		UUID& GetPrefabVersion(SceneObject* obj) { return obj->mPrefabVersion; }

		void SetPrefabVersion(SceneObject* obj, UUID& value) { obj->mPrefabVersion = value; }

		ObjectMobility& GetMobility(SceneObject* obj) { return obj->mMobility; }

		void SetMobility(SceneObject* obj, ObjectMobility& value) { obj->mMobility = value; }

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_INFO(mPrefabResourceId, 13, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare | RTTIFieldFlag::SkipInDeltaCopy))
		B3D_RTTI_END_MEMBERS

	public:
		SceneObjectRTTI()
		{
			AddReflectablePtrArrayField("mChildren", 0, &SceneObjectRTTI::GetChild, &SceneObjectRTTI::GetNumChildren, &SceneObjectRTTI::SetChild, &SceneObjectRTTI::SetNumChildren, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare | RTTIFieldFlag::SkipInDeltaCopy));
			AddReflectablePtrArrayField("mComponents", 1, &SceneObjectRTTI::GetComponent, &SceneObjectRTTI::GetNumComponents, &SceneObjectRTTI::SetComponent, &SceneObjectRTTI::SetNumComponents, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare | RTTIFieldFlag::SkipInDeltaCopy));
			//AddPlainField("mPrefabLink", 2, &SceneObjectRTTI::GetPrefabLink, &SceneObjectRTTI::SetPrefabLink);
			AddPlainField("mFlags", 3, &SceneObjectRTTI::GetFlags, &SceneObjectRTTI::SetFlags);
			AddReflectablePtrField("mPrefabDelta", 4, &SceneObjectRTTI::GetPrefabDelta, &SceneObjectRTTI::SetPrefabDelta, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare | RTTIFieldFlag::SkipInDeltaCopy));
			AddPlainField("mPrefabVersion", 5, &SceneObjectRTTI::GetPrefabVersion, &SceneObjectRTTI::SetPrefabVersion, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare | RTTIFieldFlag::SkipInDeltaCopy));
			AddPlainField("mActiveSelf", 9, &SceneObjectRTTI::GetActive, &SceneObjectRTTI::SetActive);
			AddPlainField("mMobility", 10, &SceneObjectRTTI::GetMobility, &SceneObjectRTTI::SetMobility);

			AddReflectableField("mWorldTfrm", 11, &SceneObjectRTTI::GetTransform, &SceneObjectRTTI::SetTransform);
			AddReflectableField("mLocalTfrm", 12, &SceneObjectRTTI::GetLocalTransform, &SceneObjectRTTI::SetLocalTransform);
		}

		void OnDeserializationStarted(IReflectable* object, SerializationContext* context) override
		{
			// If this is the root scene object we're deserializing, activate game object deserialization so the system
			// can resolve deserialized handles to the newly created objects
			SceneObject* const sceneObject = static_cast<SceneObject*>(object);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new scene objects).
			if(sceneObject->mRTTIData.Empty())
				return;

			CoreSerializationContext* const serializationContext = B3DRTTICast<CoreSerializationContext>(context);
			if(serializationContext == nullptr)
				return;

			if(!serializationContext->IsGameObjectDeserializationActive)
			{
				mIsDeserializationParent = true;
				serializationContext->IsGameObjectDeserializationActive = true;

				if(serializationContext->GameObjectCollection != nullptr)
					serializationContext->GameObjectCollection->BeginHandleResolve();
			}
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			SceneObject* sceneObject = static_cast<SceneObject*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new scene objects).
			if(sceneObject->mRTTIData.Empty())
				return;

			CoreSerializationContext* serializationContext = B3DRTTICast<CoreSerializationContext>(context);
			B3D_ASSERT(serializationContext != nullptr);

			GODeserializationData& goDeserializationData = AnyCastRef<GODeserializationData>(sceneObject->mRTTIData);

			// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
			// deserialized handles pointing to this object can be resolved.
			SPtr<SceneObject> sceneObjectShared = std::static_pointer_cast<SceneObject>(goDeserializationData.Ptr);

			if(sceneObject->mId.Empty() || !serializationContext->PreserveGameObjectIds)
			{
				const UUID oldId = sceneObject->mId;
				sceneObject->mId = UUIDGenerator::GenerateRandom();

				if(!oldId.Empty())
				{
					if(serializationContext->GameObjectCollection != nullptr)
						serializationContext->GameObjectCollection->RegisterUnresolvedHandleIdRemapping(oldId, sceneObject->mId);
				}
			}

			HSceneObject sceneObjectHandle = SceneObject::CreateInternal(serializationContext->GameObjectCollection, sceneObjectShared);

			// We stored all components and children in a temporary structure because they rely on the SceneObject being
			// initialized with the GameObjectManager. Now that it is, we add them.
			for(auto& component : mComponents)
				sceneObject->InternalAddComponent(component, false);

			for(auto& child : mChildren)
			{
				if(child != nullptr)
					child->SetParentInternal(sceneObject->GetHandle(), false);
			}

			// If this is the deserialization parent, end deserialization (which resolves all game object handles, if we
			// provided valid IDs), and instantiate (i.e. activate) the deserialized hierarchy.
			if(mIsDeserializationParent)
			{
				if(serializationContext->GameObjectCollection != nullptr)
					serializationContext->GameObjectCollection->EndHandleResolve();

				serializationContext->IsGameObjectDeserializationActive = false;

				bool parentActive = true;
				if(sceneObject->GetParent() != nullptr)
					parentActive = sceneObject->GetParent()->GetActive();

				sceneObject->SetActiveHierarchy(parentActive, false);

				if((sceneObject->mFlags & SOF_DontInstantiate) == 0)
					sceneObject->InstantiateInternal();
			}

			sceneObject->mRTTIData = nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "SceneObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SceneObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			SPtr<SceneObject> sceneObject = SPtr<SceneObject>(new(B3DAllocate<SceneObject>()) SceneObject("", SOF_DontInstantiate), &B3DDelete<SceneObject>, StdAlloc<SceneObject>());
			sceneObject->mRTTIData = sceneObject;

			return sceneObject;
		}

	private:
		bool mIsDeserializationParent = false;
		Vector<SPtr<SceneObject>> mChildren;
		Vector<SPtr<Component>> mComponents;
	};

	/** @} */
	/** @endcond */
} // namespace bs
