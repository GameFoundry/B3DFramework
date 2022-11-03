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

	class B3D_CORE_EXPORT PrefabComponentDiffRTTI : public RTTIType<PrefabComponentDiff, IReflectable, PrefabComponentDiffRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Id, 0)
			B3D_RTTI_MEMBER_REFLPTR(Data, 1)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "PrefabComponentDiff";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_PrefabComponentDiff;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<PrefabComponentDiff>();
		}
	};

	class B3D_CORE_EXPORT PrefabObjectDiffRTTI : public RTTIType<PrefabObjectDiff, IReflectable, PrefabObjectDiffRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Id, 0)
			B3D_RTTI_MEMBER_PLAIN(Name, 1)

			B3D_RTTI_MEMBER_REFLPTR_ARRAY(ComponentDiffs, 2)
			B3D_RTTI_MEMBER_PLAIN_ARRAY(RemovedComponents, 3)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(AddedComponents, 4)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(ChildDiffs, 5)

			B3D_RTTI_MEMBER_PLAIN_ARRAY(RemovedChildren, 6)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(AddedChildren, 7)

			B3D_RTTI_MEMBER_PLAIN(Position, 8)
			B3D_RTTI_MEMBER_PLAIN(Rotation, 9)
			B3D_RTTI_MEMBER_PLAIN(Scale, 10)
			B3D_RTTI_MEMBER_PLAIN(IsActive, 11)
			B3D_RTTI_MEMBER_PLAIN(SoFlags, 12)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "PrefabObjectDiff";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_PrefabObjectDiff;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PrefabObjectDiff>();
		}
	};

	class B3D_CORE_EXPORT PrefabDiffRTTI : public RTTIType<PrefabDiff, IReflectable, PrefabDiffRTTI>
	{
		/**	Contains data about a game object handle serialized in a prefab diff.  */
		struct SerializedHandle
		{
			SPtr<SerializedObject> Object;
			SPtr<GameObjectHandleBase> Handle;
		};

	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFLPTR(mRoot, 0)
		B3D_RTTI_END_MEMBERS
	public:
		void OnDeserializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			B3D_ASSERT(context != nullptr && B3DRTTIIsOfType<CoreSerializationContext>(context));
			auto coreContext = static_cast<CoreSerializationContext*>(context);

			if(coreContext->GoState)
			{
				coreContext->GoState->RegisterOnDeserializationEndCallback(
					std::bind(&PrefabDiffRTTI::DelayedOnDeserializationEnded, prefabDiff));
			}
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			B3D_ASSERT(context != nullptr && B3DRTTIIsOfType<CoreSerializationContext>(context));
			const auto coreContext = static_cast<CoreSerializationContext*>(context);
			B3D_ASSERT(coreContext->GoState);

			// Make sure to deserialize all game object handles since their IDs need to be updated. Normally they are
			// updated automatically upon deserialization but since we store them in intermediate form we need to manually
			// deserialize and reserialize them in order to update their IDs.
			PrefabDiff* prefabDiff = static_cast<PrefabDiff*>(obj);

			Stack<SPtr<PrefabObjectDiff>> todo;

			if(prefabDiff->mRoot != nullptr)
				todo.push(prefabDiff->mRoot);

			UnorderedSet<SPtr<SerializedObject>> handleObjects;

			while(!todo.empty())
			{
				SPtr<PrefabObjectDiff> current = todo.top();
				todo.pop();

				for(auto& component : current->AddedComponents)
					FindGameObjectHandles(component, handleObjects);

				for(auto& child : current->AddedChildren)
					FindGameObjectHandles(child, handleObjects);

				for(auto& component : current->ComponentDiffs)
					FindGameObjectHandles(component->Data, handleObjects);

				for(auto& child : current->ChildDiffs)
					todo.push(child);
			}

			Vector<SerializedHandle> handleData(handleObjects.size());

			u32 idx = 0;
			for(auto& handleObject : handleObjects)
			{
				SerializedHandle& handle = handleData[idx];

				handle.Object = handleObject;
				handle.Handle = std::static_pointer_cast<GameObjectHandleBase>(handleObject->Decode(context));

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
			Vector<SerializedHandle>& handleData = AnyCastRef<Vector<SerializedHandle>>(prefabDiff->mRTTIData);

			for(auto& serializedHandle : handleData)
			{
				if(serializedHandle.Handle != nullptr)
					*serializedHandle.Object = *SerializedObject::Create(*serializedHandle.Handle);
			}

			prefabDiff->mRTTIData = nullptr;
		}

		/**	Scans the entire hierarchy and find all serialized GameObjectHandle objects. */
		static void FindGameObjectHandles(const SPtr<SerializedObject>& serializedObject, UnorderedSet<SPtr<SerializedObject>>& handleObjects)
		{
			for(auto& subObject : serializedObject->SubObjects)
			{
				RTTITypeBase* rtti = IReflectable::GetRttifromTypeIdInternal(subObject.TypeId);
				if(rtti == nullptr)
					continue;

				if(rtti->GetRttiId() == TID_GameObjectHandleBase)
				{
					handleObjects.insert(serializedObject);
					return;
				}

				for(auto& child : subObject.Entries)
				{
					RTTIField* curGenericField = rtti->FindField(child.second.FieldId);
					if(curGenericField == nullptr)
						continue;

					SPtr<SerializedInstance> entryData = child.second.Serialized;
					if(entryData == nullptr)
						continue;

					if(B3DRTTIIsOfType<SerializedArray>(entryData))
					{
						SPtr<SerializedArray> arrayData = std::static_pointer_cast<SerializedArray>(entryData);

						for(auto& arrayElem : arrayData->Entries)
						{
							if(arrayElem.second.Serialized != nullptr && B3DRTTIIsOfType<SerializedObject>(arrayElem.second.Serialized))
							{
								SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.Serialized);
								FindGameObjectHandles(arrayElemData, handleObjects);
							}
						}
					}
					else if(B3DRTTIIsOfType<SerializedObject>(entryData))
					{
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
						FindGameObjectHandles(fieldObjectData, handleObjects);
					}
				}
			}
		}

		const String& GetRttiName() override
		{
			static String name = "PrefabDiff";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_PrefabDiff;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PrefabDiff>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
