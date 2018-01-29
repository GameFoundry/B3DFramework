//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsPrefabDiff.h"
#include "Serialization/BsSerializedObject.h"
#include "Scene/BsGameObjectManager.h"
#include "Serialization/BsBinarySerializer.h"

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
		PrefabComponentDiffRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "PrefabComponentDiff";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PrefabComponentDiff;
		}

		SPtr<IReflectable> newRTTIObject() override
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
		PrefabObjectDiffRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "PrefabObjectDiff";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PrefabObjectDiff;
		}

		SPtr<IReflectable> newRTTIObject() override
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
		PrefabDiffRTTI()
			:mInitMembers(this)
		{ }

		void onDeserializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			if (GameObjectManager::instance().isGameObjectDeserializationActive())
				GameObjectManager::instance().registerOnDeserializationEndCallback(std::bind(&PrefabDiffRTTI::delayedOnDeserializationEnded, prefabDiff));
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			assert(GameObjectManager::instance().isGameObjectDeserializationActive());

			// Make sure to deserialize all game object handles since their IDs need to be updated. Normally they are
			// updated automatically upon deserialization but since we store them in intermediate form we need to manually
			// deserialize and reserialize them in order to update their IDs.
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			Stack<SPtr<PrefabObjectDiff>> todo;

			if (prefabDiff->mRoot != nullptr)
				todo.push(prefabDiff->mRoot);

			UnorderedSet<SPtr<SerializedObject>> handleObjects;

			while (!todo.empty())
			{
				SPtr<PrefabObjectDiff> current = todo.top();
				todo.pop();

				for (auto& component : current->addedComponents)
					findGameObjectHandles(component, handleObjects);

				for (auto& child : current->addedChildren)
					findGameObjectHandles(child, handleObjects);

				for (auto& component : current->componentDiffs)
					findGameObjectHandles(component->data, handleObjects);

				for (auto& child : current->childDiffs)
					todo.push(child);
			}

			Vector<SerializedHandle> handleData(handleObjects.size());

			UINT32 idx = 0;
			BinarySerializer bs;
			for (auto& handleObject : handleObjects)
			{
				SerializedHandle& handle = handleData[idx];

				handle.object = handleObject;
				handle.handle = std::static_pointer_cast<GameObjectHandleBase>(bs._decodeFromIntermediate(handleObject));

				idx++;
			}

			prefabDiff->mRTTIData = handleData;
		}

		/**
		 * Decodes GameObjectHandles from their binary format, because during deserialization GameObjectManager will update
		 * all object IDs and we want to keep the handles up to date.So we deserialize them and allow them to be updated
		 * before storing them back into binary format.
		 */
		static void delayedOnDeserializationEnded(PrefabDiff* prefabDiff)
		{
			Vector<SerializedHandle>& handleData = any_cast_ref<Vector<SerializedHandle>>(prefabDiff->mRTTIData);

			BinarySerializer bs;
			for (auto& serializedHandle : handleData)
			{
				if (serializedHandle.handle != nullptr)
					*serializedHandle.object = *bs._encodeToIntermediate(serializedHandle.handle.get());
			}

			prefabDiff->mRTTIData = nullptr;
		}

		/**	Scans the entire hierarchy and find all serialized GameObjectHandle objects. */
		static void findGameObjectHandles(const SPtr<SerializedObject>& serializedObject, UnorderedSet<SPtr<SerializedObject>>& handleObjects)
		{
			for (auto& subObject : serializedObject->subObjects)
			{
				RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
				if (rtti == nullptr)
					continue;

				if (rtti->getRTTIId() == TID_GameObjectHandleBase)
				{
					handleObjects.insert(serializedObject);
					return;
				}

				for (auto& child : subObject.entries)
				{
					RTTIField* curGenericField = rtti->findField(child.second.fieldId);
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

		const String& getRTTIName() override
		{
			static String name = "PrefabDiff";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PrefabDiff;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<PrefabDiff>();
		}
	};

	/** @} */
	/** @endcond */
}