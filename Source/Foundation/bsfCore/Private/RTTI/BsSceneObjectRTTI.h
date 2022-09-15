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
#include "Scene/BsPrefabDiff.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT SceneObjectRTTI : public RTTIType<SceneObject, GameObject, SceneObjectRTTI>
	{
	private:
		Transform& GetTransform(SceneObject* obj) { return obj->mWorldTfrm; }
		void SetTransform(SceneObject* obj, Transform& value) { obj->mWorldTfrm = value; }

		Transform& GetLocalTransform(SceneObject* obj) { return obj->mLocalTfrm; }
		void SetLocalTransform(SceneObject* obj, Transform& value) { obj->mLocalTfrm = value; }

		bool& GetActive(SceneObject* obj) { return obj->mActiveSelf; }
		void SetActive(SceneObject* obj, bool& value) { obj->mActiveSelf = value; }

		SPtr<SceneObject> GetChild(SceneObject* obj, UINT32 idx) { return obj->mChildren[idx].GetInternalPtr(); }
		void SetChild(SceneObject* obj, UINT32 idx, SPtr<SceneObject> param)
		{
			// It's important that child indices remain the same after deserialization, as some systems (like SO
			// record/restore) depend on it
			if(idx >= mChildren.size())
				mChildren.resize(idx + 1);

			mChildren[idx] = param;
		}

		UINT32 GetNumChildren(SceneObject* obj) { return (UINT32)obj->mChildren.size(); }
		void SetNumChildren(SceneObject* obj, UINT32 size) { /* DO NOTHING */ }

		// NOTE - These can only be set sequentially, specific array index is ignored
		SPtr<Component> GetComponent(SceneObject* obj, UINT32 idx) { return obj->mComponents[idx].GetInternalPtr(); }
		void SetComponent(SceneObject* obj, UINT32 idx, SPtr<Component> param)
		{
			// It's important that child indices remain the same after deserialization, as some systems (like SO
			// record/restore) depend on it
			if(idx >= mComponents.size())
				mComponents.resize(idx + 1);

			mComponents[idx] = param;
		}
		UINT32 GetNumComponents(SceneObject* obj) { return (UINT32)obj->mComponents.size(); }
		void SetNumComponents(SceneObject* obj, UINT32 size) { /* DO NOTHING */ }

		UUID& GetPrefabLink(SceneObject* obj) { return obj->mPrefabLinkUUID; }
		void SetPrefabLink(SceneObject* obj, UUID& value) { obj->mPrefabLinkUUID = value; }

		SPtr<PrefabDiff> GetPrefabDiff(SceneObject* obj) { return obj->mPrefabDiff; }
		void SetPrefabDiff(SceneObject* obj, SPtr<PrefabDiff> value) { obj->mPrefabDiff = value; }

		UINT32& GetFlags(SceneObject* obj) { return obj->mFlags; }
		void SetFlags(SceneObject* obj, UINT32& value) { obj->mFlags = value; }

		UINT32& GetPrefabHash(SceneObject* obj) { return obj->mPrefabHash; }
		void SetPrefabHash(SceneObject* obj, UINT32& value) { obj->mPrefabHash = value; }

		ObjectMobility& GetMobility(SceneObject* obj) { return obj->mMobility; }
		void SetMobility(SceneObject* obj, ObjectMobility& value) { obj->mMobility = value; }
	public:
		SceneObjectRTTI()
		{
			AddReflectablePtrArrayField("mChildren", 0, &SceneObjectRTTI::GetChild,
				&SceneObjectRTTI::GetNumChildren, &SceneObjectRTTI::SetChild, &SceneObjectRTTI::SetNumChildren);
			AddReflectablePtrArrayField("mComponents", 1, &SceneObjectRTTI::GetComponent,
				&SceneObjectRTTI::GetNumComponents, &SceneObjectRTTI::SetComponent, &SceneObjectRTTI::SetNumComponents);
			AddPlainField("mPrefabLink", 2, &SceneObjectRTTI::GetPrefabLink, &SceneObjectRTTI::SetPrefabLink);
			AddPlainField("mFlags", 3, &SceneObjectRTTI::GetFlags, &SceneObjectRTTI::SetFlags);
			AddReflectablePtrField("mPrefabDiff", 4, &SceneObjectRTTI::GetPrefabDiff, &SceneObjectRTTI::SetPrefabDiff);
			AddPlainField("mPrefabHash", 5, &SceneObjectRTTI::GetPrefabHash, &SceneObjectRTTI::SetPrefabHash);
			AddPlainField("mActiveSelf", 9, &SceneObjectRTTI::GetActive, &SceneObjectRTTI::SetActive);
			AddPlainField("mMobility", 10, &SceneObjectRTTI::GetMobility, &SceneObjectRTTI::SetMobility);

			AddReflectableField("mWorldTfrm", 11, &SceneObjectRTTI::GetTransform, &SceneObjectRTTI::SetTransform);
			AddReflectableField("mLocalTfrm", 12, &SceneObjectRTTI::GetLocalTransform, &SceneObjectRTTI::SetLocalTransform);
		}

		void OnDeserializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			// If this is the root scene object we're deserializing, activate game object deserialization so the system
			// can resolve deserialized handles to the newly created objects
			SceneObject* so = static_cast<SceneObject*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new scene objects).
			if (so->mRTTIData.Empty())
				return;

			if(context == nullptr || !rtti_is_of_type<CoreSerializationContext>(context))
				return;

			auto coreContext = static_cast<CoreSerializationContext*>(context);
			if(!coreContext->goDeserializationActive)
			{
				if (!coreContext->goState)
					coreContext->goState = bs_shared_ptr_new<GameObjectDeserializationState>();

				mIsDeserializationParent = true;
				coreContext->goDeserializationActive = true;
			}
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			SceneObject* so = static_cast<SceneObject*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new scene objects).
			if (so->mRTTIData.Empty())
				return;

			BS_ASSERT(context != nullptr && rtti_is_of_type<CoreSerializationContext>(context));
			auto coreContext = static_cast<CoreSerializationContext*>(context);

			GODeserializationData& goDeserializationData = any_cast_ref<GODeserializationData>(so->mRTTIData);

			// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
			// deserialized handles pointing to this object can be resolved.
			SPtr<SceneObject> soPtr = std::static_pointer_cast<SceneObject>(goDeserializationData.ptr);

			HSceneObject soHandle = SceneObject::CreateInternal(soPtr);
			coreContext->goState->RegisterObject(goDeserializationData.originalId, soHandle);

			// We stored all components and children in a temporary structure because they rely on the SceneObject being
			// initialized with the GameObjectManager. Now that it is, we add them.
			for (auto& component : mComponents)
				so->AddComponentInternal(component);

			for (auto& child : mChildren)
			{
				if(child != nullptr)
					child->SetParentInternal(so->mThisHandle, false);
			}

			if(so->mUUID.Empty() || coreContext->goState->GetUseNewUuiDs())
				so->mUUID = UUIDGenerator::GenerateRandom();

			// If this is the deserialization parent, end deserialization (which resolves all game object handles, if we
			// provided valid IDs), and instantiate (i.e. activate) the deserialized hierarchy.
			if (mIsDeserializationParent)
			{
				coreContext->goState->Resolve();
				coreContext->goDeserializationActive = false;

				bool parentActive = true;
				if (so->GetParent() != nullptr)
					parentActive = so->GetParent()->GetActive();

				so->SetActiveHierarchy(parentActive, false);

				if ((so->mFlags & SOF_DontInstantiate) == 0)
					so->InstantiateInternal();
			}

			so->mRTTIData = nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "SceneObject";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SceneObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			SPtr<SceneObject> sceneObject = SPtr<SceneObject>(new (bs_alloc<SceneObject>()) SceneObject("", SOF_DontInstantiate),
				&bs_delete<SceneObject>, StdAlloc<SceneObject>());
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
}
