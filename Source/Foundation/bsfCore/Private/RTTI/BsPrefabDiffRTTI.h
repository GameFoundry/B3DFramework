//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "Scene/BsPrefabDiff.h"
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

	class BS_CORE_EXPORT PrefabComponentDiffRTTI : public RTTIType < PrefabComponentDiff, IReflectable, PrefabComponentDiffRTTI >
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(id, 0)
			BS_RTTI_MEMBER_REFLPTR(data, 1)
		BS_END_RTTI_MEMBERS
	public:
		const String& GetRTTIName() override
		{
			static String name = "PrefabComponentDiff";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_PrefabComponentDiff;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<PrefabComponentDiff>();
		}
	};

	class BS_CORE_EXPORT PrefabObjectDiffRTTI : public RTTIType < PrefabObjectDiff, IReflectable, PrefabObjectDiffRTTI >
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(id, 0)
			BS_RTTI_MEMBER_PLAIN(name, 1)

			BS_RTTI_MEMBER_REFLPTR_ARRAY(componentDiffs, 2)
			BS_RTTI_MEMBER_PLAIN_ARRAY(removedComponents, 3)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(addedComponents, 4)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(childDiffs, 5)

			BS_RTTI_MEMBER_PLAIN_ARRAY(removedChildren, 6)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(addedChildren, 7)

			BS_RTTI_MEMBER_PLAIN(position, 8)
			BS_RTTI_MEMBER_PLAIN(rotation, 9)
			BS_RTTI_MEMBER_PLAIN(scale, 10)
			BS_RTTI_MEMBER_PLAIN(isActive, 11)
			BS_RTTI_MEMBER_PLAIN(soFlags, 12)
		BS_END_RTTI_MEMBERS
	public:
		const String& GetRTTIName() override
		{
			static String name = "PrefabObjectDiff";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_PrefabObjectDiff;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<PrefabObjectDiff>();
		}
	};

	class BS_CORE_EXPORT PrefabDiffRTTI : public RTTIType < PrefabDiff, IReflectable, PrefabDiffRTTI >
	{
		/**	Contains data about a game object handle serialized in a prefab diff.  */
		struct SerializedHandle
		{
			SPtr<SerializedObject> object;
			SPtr<GameObjectHandleBase> handle;
		};

	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mRoot, 0)
		BS_END_RTTI_MEMBERS
	public:
		void OnDeserializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			BS_ASSERT(context != nullptr && rtti_is_of_type<CoreSerializationContext>(context));
			auto coreContext = static_cast<CoreSerializationContext*>(context);

			if (coreContext->goState)
			{
				coreContext->goState->registerOnDeserializationEndCallback(
					std::bind(&PrefabDiffRTTI::delayedOnDeserializationEnded, prefabDiff));
			}
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			BS_ASSERT(context != nullptr && rtti_is_of_type<CoreSerializationContext>(context));
			const auto coreContext = static_cast<CoreSerializationContext*>(context);
			BS_ASSERT(coreContext->goState);

			// Make sure to deserialize all game object handles since their IDs need to be updated. Normally they are
			// updated automatically upon deserialization but since we store them in intermediate form we need to manually
			// deserialize and reserialize them in order to update their IDs.
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			Stack<SPtr<PrefabObjectDiff>> todo;

			if (prefabDiff->mRoot != nullptr)
				todo.Push(prefabDiff->mRoot);

			UnorderedSet<SPtr<SerializedObject>> handleObjects;

			while (!todo.empty())
			{
				SPtr<PrefabObjectDiff> current = todo.Top();
				todo.pop();

				for (auto& component : current->addedComponents)
					findGameObjectHandles(component, handleObjects);

				for (auto& child : current->addedChildren)
					findGameObjectHandles(child, handleObjects);

				for (auto& component : current->componentDiffs)
					findGameObjectHandles(component->Data, handleObjects);

				for (auto& child : current->childDiffs)
					todo.Push(child);
			}

			Vector<SerializedHandle> HandleData(handleObjects.size());

			UINT32 idx = 0;
			for (auto& handleObject : handleObjects)
			{
				SerializedHandle& handle = handleData[idx];

				handle.object = handleObject;
				handle.handle = std::static_pointer_cast<GameObjectHandleBase>(handleObject->Decode(context));

				idx++;
			}

			prefabDiff->mRTTIData = handleData;
		}

		/**
		 * Decodes GameObjectHandles from their binary format, because during deserialization GameObjectManager will update
		 * all object IDs and we want to keep the handles up to date.So we deserialize them and allow them to be updated
		 * before storing them back into binary format.
		 */
		static void DelayedOnDeserializationEnded(PrefabDiff* prefabDiff)
		{
			Vector<SerializedHandle>& handleData = any_cast_ref<Vector<SerializedHandle>>(prefabDiff->mRTTIData);

			for (auto& serializedHandle : handleData)
			{
				if (serializedHandle.handle != nullptr)
					*serializedHandle.object = *SerializedObject::create(*serializedHandle.handle);
			}

			prefabDiff->mRTTIData = nullptr;
		}

		/**	Scans the entire hierarchy and find all serialized GameObjectHandle objects. */
		static void FindGameObjectHandles(const SPtr<SerializedObject>& serializedObject, UnorderedSet<SPtr<SerializedObject>>& handleObjects)
		{
			for (auto& subObject : serializedObject->subObjects)
			{
				RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
				if (rtti == nullptr)
					continue;

				if (rtti->GetRTTIId() == TID_GameObjectHandleBase)
				{
					handleObjects.Insert(serializedObject);
					return;
				}

				for (auto& child : subObject.entries)
				{
					RTTIField* curGenericField = rtti->FindField(child.second.fieldId);
					if (curGenericField == nullptr)
						continue;

					SPtr<SerializedInstance> entryData = child.second.serialized;
					if (entryData == nullptr)
						continue;

					if (rtti_is_of_type<SerializedArray>(entryData))
					{
						SPtr<SerializedArray> arrayData = std::static_pointer_cast<SerializedArray>(entryData);
						
						for (auto& arrayElem : arrayData->entries)
						{
							if (arrayElem.second.serialized != nullptr && rtti_is_of_type<SerializedObject>(arrayElem.second.serialized))
							{
								SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);
								findGameObjectHandles(arrayElemData, handleObjects);
							}
						}
					}
					else if(rtti_is_of_type<SerializedObject>(entryData))
					{
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
						findGameObjectHandles(fieldObjectData, handleObjects);
					}
				}
			}
		}

		const String& GetRTTIName() override
		{
			static String name = "PrefabDiff";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_PrefabDiff;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<PrefabDiff>();
		}
	};

	/** @} */
	/** @endcond */
}
