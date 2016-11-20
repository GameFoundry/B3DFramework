//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsSceneObject.h"
#include "BsGameObjectHandle.h"
#include "BsGameObjectManager.h"
#include "BsComponent.h"
#include "BsGameObjectRTTI.h"
#include "BsPrefabDiff.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	/** Provides temporary storage for data used during SceneObject deserialization. */
	struct SODeserializationData
	{
		Vector<SPtr<SceneObject>> children;
		Vector<SPtr<Component>> components;
	};

	class BS_CORE_EXPORT SceneObjectRTTI : public RTTIType<SceneObject, GameObject, SceneObjectRTTI>
	{
	private:
		Vector3& getPosition(SceneObject* obj) { return obj->mPosition; }
		void setPosition(SceneObject* obj, Vector3& value) { obj->mPosition = value; }

		Quaternion& getRotation(SceneObject* obj) { return obj->mRotation; }
		void setRotation(SceneObject* obj, Quaternion& value) { obj->mRotation = value; }

		Vector3& getScale(SceneObject* obj) { return obj->mScale; }
		void setScale(SceneObject* obj, Vector3& value) { obj->mScale = value; }

		bool& getActive(SceneObject* obj) { return obj->mActiveSelf; }
		void setActive(SceneObject* obj, bool& value) { obj->mActiveSelf = value; }

		// NOTE - These can only be set sequentially, specific array index is ignored
		SPtr<SceneObject> getChild(SceneObject* obj, UINT32 idx) { return obj->mChildren[idx].getInternalPtr(); }
		void setChild(SceneObject* obj, UINT32 idx, SPtr<SceneObject> param)
		{
			SceneObject* so = static_cast<SceneObject*>(obj);
			GODeserializationData& goDeserializationData = any_cast_ref<GODeserializationData>(so->mRTTIData);
			SODeserializationData& soDeserializationData = any_cast_ref<SODeserializationData>(goDeserializationData.moreData);

			soDeserializationData.children.push_back(param);
		} 

		UINT32 getNumChildren(SceneObject* obj) { return (UINT32)obj->mChildren.size(); }
		void setNumChildren(SceneObject* obj, UINT32 size) { /* DO NOTHING */ }

		// NOTE - These can only be set sequentially, specific array index is ignored
		SPtr<Component> getComponent(SceneObject* obj, UINT32 idx) { return obj->mComponents[idx].getInternalPtr(); }
		void setComponent(SceneObject* obj, UINT32 idx, SPtr<Component> param)
		{
			SceneObject* so = static_cast<SceneObject*>(obj);
			GODeserializationData& goDeserializationData = any_cast_ref<GODeserializationData>(so->mRTTIData);
			SODeserializationData& soDeserializationData = any_cast_ref<SODeserializationData>(goDeserializationData.moreData);

			soDeserializationData.components.push_back(param);
		}
		UINT32 getNumComponents(SceneObject* obj) { return (UINT32)obj->mComponents.size(); }
		void setNumComponents(SceneObject* obj, UINT32 size) { /* DO NOTHING */ }

		String& getPrefabLink(SceneObject* obj) { return obj->mPrefabLinkUUID; }
		void setPrefabLink(SceneObject* obj, String& value) { obj->mPrefabLinkUUID = value; }

		SPtr<PrefabDiff> getPrefabDiff(SceneObject* obj) { return obj->mPrefabDiff; }
		void setPrefabDiff(SceneObject* obj, SPtr<PrefabDiff> value) { obj->mPrefabDiff = value; }

		UINT32& getFlags(SceneObject* obj) { return obj->mFlags; }
		void setFlags(SceneObject* obj, UINT32& value) { obj->mFlags = value; }

		UINT32& getPrefabHash(SceneObject* obj) { return obj->mPrefabHash; }
		void setPrefabHash(SceneObject* obj, UINT32& value) { obj->mPrefabHash = value; }
	public:
		SceneObjectRTTI()
		{
			addReflectablePtrArrayField("mChildren", 0, &SceneObjectRTTI::getChild, 
				&SceneObjectRTTI::getNumChildren, &SceneObjectRTTI::setChild, &SceneObjectRTTI::setNumChildren);
			addReflectablePtrArrayField("mComponents", 1, &SceneObjectRTTI::getComponent, 
				&SceneObjectRTTI::getNumComponents, &SceneObjectRTTI::setComponent, &SceneObjectRTTI::setNumComponents);
			addPlainField("mPrefabLink", 2, &SceneObjectRTTI::getPrefabLink, &SceneObjectRTTI::setPrefabLink);
			addPlainField("mFlags", 3, &SceneObjectRTTI::getFlags, &SceneObjectRTTI::setFlags);
			addReflectablePtrField("mPrefabDiff", 4, &SceneObjectRTTI::getPrefabDiff, &SceneObjectRTTI::setPrefabDiff);
			addPlainField("mPrefabHash", 5, &SceneObjectRTTI::getPrefabHash, &SceneObjectRTTI::setPrefabHash);
			addPlainField("mPosition", 6, &SceneObjectRTTI::getPosition, &SceneObjectRTTI::setPosition);
			addPlainField("mRotation", 7, &SceneObjectRTTI::getRotation, &SceneObjectRTTI::setRotation);
			addPlainField("mScale", 8, &SceneObjectRTTI::getScale, &SceneObjectRTTI::setScale);
			addPlainField("mActiveSelf", 9, &SceneObjectRTTI::getActive, &SceneObjectRTTI::setActive);
		}

		void onDeserializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			// If this is the root scene object we're deserializing, activate game object deserialization so the system
			// can resolve deserialized handles to the newly created objects
			SceneObject* so = static_cast<SceneObject*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new scene objects).
			if (so->mRTTIData.empty())
				return;

			// Every GameObject must store GODeserializationData in its RTTI data field during deserialization
			GODeserializationData& deserializationData = any_cast_ref<GODeserializationData>(so->mRTTIData);

			// We delay adding children/components and instead store them here
			deserializationData.moreData = SODeserializationData();

			if (!GameObjectManager::instance().isGameObjectDeserializationActive())
			{
				GameObjectManager::instance().startDeserialization();
				
				// Mark it as the object that started the GO deserialization so it knows to end it
				deserializationData.isDeserializationParent = true;
			}
			else
				deserializationData.isDeserializationParent = false;
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			SceneObject* so = static_cast<SceneObject*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new scene objects).
			if (so->mRTTIData.empty())
				return;

			GODeserializationData& goDeserializationData = any_cast_ref<GODeserializationData>(so->mRTTIData);

			// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
			// deserialized handles pointing to this object can be resolved.
			SPtr<SceneObject> soPtr = std::static_pointer_cast<SceneObject>(goDeserializationData.ptr);
			SceneObject::createInternal(soPtr, goDeserializationData.originalId);

			// We stored all components and children in a temporary structure because they rely on the SceneObject being
			// initialized with the GameObjectManager. Now that it is, we add them.
			SODeserializationData& soDeserializationData = any_cast_ref<SODeserializationData>(goDeserializationData.moreData);

			for (auto& component : soDeserializationData.components)
				so->addComponentInternal(component);

			for (auto& child : soDeserializationData.children)
				child->_setParent(so->mThisHandle, false);

			// If this is the deserialization parent, end deserialization (which resolves all game object handles, if we 
			// provided valid IDs), and instantiate (i.e. activate) the deserialized hierarchy.
			if (goDeserializationData.isDeserializationParent)
			{
				GameObjectManager::instance().endDeserialization();

				bool parentActive = true;
				if (so->getParent() != nullptr)
					parentActive = so->getParent()->getActive();

				so->setActiveHierarchy(parentActive, false);

				if ((so->mFlags & SOF_DontInstantiate) == 0)
					so->_instantiate();
			}

			so->mRTTIData = nullptr;
		}

		const String& getRTTIName() override
		{
			static String name = "SceneObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SceneObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			SPtr<SceneObject> sceneObject = SPtr<SceneObject>(new (bs_alloc<SceneObject>()) SceneObject("", SOF_DontInstantiate),
				&bs_delete<SceneObject>, StdAlloc<SceneObject>());
			sceneObject->mRTTIData = sceneObject;

			return sceneObject;
		}
	};

	/** @} */
	/** @endcond */
}