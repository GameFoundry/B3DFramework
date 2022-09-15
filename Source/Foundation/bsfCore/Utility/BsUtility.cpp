//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsUtility.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsSceneObject.h"

namespace bs
{
	/**
	 * Checks if the specified type (or any of its derived classes) have any IReflectable pointer or value types as
	 * their fields.
	 */
	bool hasReflectableChildren(RTTITypeBase* type)
	{
		UINT32 numFields = type->GetNumFields();
		for (UINT32 i = 0; i < numFields; i++)
		{
			RTTIField* field = type->GetField(i);
			if (field->schema.type == SerializableFT_Reflectable || field->schema.type == SerializableFT_ReflectablePtr)
				return true;
		}

		const Vector<RTTITypeBase*>& derivedClasses = type->GetDerivedClasses();
		for (auto& derivedClass : derivedClasses)
		{
			numFields = derivedClass->GetNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* field = derivedClass->GetField(i);
				if (field->schema.type == SerializableFT_Reflectable || field->schema.type == SerializableFT_ReflectablePtr)
					return true;
			}
		}

		return false;
	}

	void findResourceDependenciesInternal(IReflectable& obj, FrameAlloc& alloc, bool recursive,
		Map<UUID, ResourceDependency>& dependencies)
	{
		RTTITypeBase* rtti = obj.GetRtti();
		do {
			RTTITypeBase* rttiInstance = rtti->CloneInternal(alloc);
			rttiInstance->OnSerializationStarted(&obj, nullptr);

			const UINT32 numFields = rtti->GetNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* field = rtti->GetField(i);
				if (field->schema.info.flags.IsSet(RTTIFieldFlag::SkipInReferenceSearch))
					continue;

				if (field->schema.type == SerializableFT_Reflectable)
				{
					auto reflectableField = static_cast<RTTIReflectableFieldBase*>(field);

					if (reflectableField->GetType()->GetRttiId() == TID_ResourceHandle)
					{
						if (reflectableField->schema.isArray)
						{
							const UINT32 numElements = reflectableField->GetArraySize(rttiInstance, &obj);
							for (UINT32 j = 0; j < numElements; j++)
							{
								HResource resource = (HResource&)reflectableField->GetArrayValue(rttiInstance, &obj, j);
								if (!resource.GetUuid().Empty())
								{
									ResourceDependency& dependency = dependencies[resource.GetUuid()];
									dependency.resource = resource;
									dependency.numReferences++;
								}
							}
						}
						else
						{
							HResource resource = (HResource&)reflectableField->GetValue(rttiInstance, &obj);
							if (!resource.GetUuid().Empty())
							{
								ResourceDependency& dependency = dependencies[resource.GetUuid()];
								dependency.resource = resource;
								dependency.numReferences++;
							}
						}
					}
					else if (recursive)
					{
						// Optimization, no need to retrieve its value and go deeper if it has no
						// reflectable children that may hold the reference.
						if (hasReflectableChildren(reflectableField->GetType()))
						{
							if (reflectableField->schema.isArray)
							{
								const UINT32 numElements = reflectableField->GetArraySize(rttiInstance, &obj);
								for (UINT32 j = 0; j < numElements; j++)
								{
									IReflectable& childObj = reflectableField->GetArrayValue(rttiInstance, &obj, j);
									findResourceDependenciesInternal(childObj, alloc, true, dependencies);
								}
							}
							else
							{
								IReflectable& childObj = reflectableField->GetValue(rttiInstance, &obj);
								findResourceDependenciesInternal(childObj, alloc, true, dependencies);
							}
						}
					}
				}
				else if (field->schema.type == SerializableFT_ReflectablePtr && recursive)
				{
					auto reflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(field);

					// Optimization, no need to retrieve its value and go deeper if it has no
					// reflectable children that may hold the reference.
					if (hasReflectableChildren(reflectablePtrField->GetType()))
					{
						if (reflectablePtrField->schema.isArray)
						{
							const UINT32 numElements = reflectablePtrField->GetArraySize(rttiInstance, &obj);
							for (UINT32 j = 0; j < numElements; j++)
							{
								const SPtr<IReflectable>& childObj =
									reflectablePtrField->GetArrayValue(rttiInstance, &obj, j);

								if (childObj != nullptr)
									findResourceDependenciesInternal(*childObj, alloc, true, dependencies);
							}
						}
						else
						{
							const SPtr<IReflectable>& childObj = reflectablePtrField->GetValue(rttiInstance, &obj);

							if (childObj != nullptr)
								findResourceDependenciesInternal(*childObj, alloc, true, dependencies);
						}
					}
				}
			}

			rttiInstance->OnSerializationEnded(&obj, nullptr);
			alloc.Destruct(rttiInstance);

			rtti = rtti->GetBaseClass();
		} while(rtti != nullptr);
	}

	Vector<ResourceDependency> Utility::FindResourceDependencies(IReflectable& obj, bool recursive)
	{
		gFrameAlloc().MarkFrame();

		Map<UUID, ResourceDependency> dependencies;
		findResourceDependenciesInternal(obj, gFrameAlloc(), recursive, dependencies);

		gFrameAlloc().Clear();

		Vector<ResourceDependency> dependencyList(dependencies.size());
		UINT32 i = 0;
		for (auto& entry : dependencies)
		{
			dependencyList[i] = entry.second;
			i++;
		}

		return dependencyList;
	}

	UINT32 Utility::GetSceneObjectDepth(const HSceneObject& so)
	{
		HSceneObject parent = so->GetParent();
		
		UINT32 depth = 0;
		while (parent != nullptr)
		{
			depth++;
			parent = parent->GetParent();
		}

		return depth;
	}

	Vector<HComponent> Utility::FindComponents(const HSceneObject& object, UINT32 typeId)
	{
		Vector<HComponent> output;

		Stack<HSceneObject> todo;
		todo.push(object);

		while(!todo.empty())
		{
			HSceneObject curSO = todo.top();
			todo.pop();

			const Vector<HComponent>& components = curSO->GetComponents();
			for(auto& entry : components)
			{
				if (entry->GetRtti()->GetRttiId() == typeId)
					output.push_back(entry);
			}

			UINT32 numChildren = curSO->GetNumChildren();
			for (UINT32 i = 0; i < numChildren; i++)
				todo.push(curSO->GetChild(i));
		}

		return output;
	}

	class CoreSerializationContextRTTI :
		public RTTIType<CoreSerializationContext, SerializationContext, CoreSerializationContextRTTI>
	{
		const String& GetRttiName() override
		{
			static String name = "CoreSerializationContext";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_CoreSerializationContext;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	RTTITypeBase* CoreSerializationContext::GetRttiStatic()
	{
		return CoreSerializationContextRTTI::Instance();
	}

	RTTITypeBase* CoreSerializationContext::GetRtti() const
	{
		return GetRttiStatic();
	}

}
