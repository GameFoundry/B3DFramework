//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsUtility.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsSceneObject.h"

using namespace bs;

/**
 * Checks if the specified type (or any of its derived classes) have any IReflectable pointer or value types as
 * their fields.
 */
bool hasReflectableChildren(RTTITypeBase* type)
{
	u32 numFields = type->GetNumFields();
	for(u32 i = 0; i < numFields; i++)
	{
		RTTIField* field = type->GetField(i);
		if(field->Schema.Type == SerializableFT_Reflectable || field->Schema.Type == SerializableFT_ReflectablePtr)
			return true;
	}

	const Vector<RTTITypeBase*>& derivedClasses = type->GetDerivedClasses();
	for(auto& derivedClass : derivedClasses)
	{
		numFields = derivedClass->GetNumFields();
		for(u32 i = 0; i < numFields; i++)
		{
			RTTIField* field = derivedClass->GetField(i);
			if(field->Schema.Type == SerializableFT_Reflectable || field->Schema.Type == SerializableFT_ReflectablePtr)
				return true;
		}
	}

	return false;
}

void findResourceDependenciesInternal(IReflectable& obj, FrameAlloc& alloc, bool recursive, Map<UUID, ResourceDependency>& dependencies)
{
	RTTITypeBase* rtti = obj.GetRtti();
	do
	{
		RTTITypeBase* rttiInstance = rtti->CloneInternal(alloc);
		rttiInstance->OnSerializationStarted(&obj, nullptr);

		const u32 numFields = rtti->GetNumFields();
		for(u32 i = 0; i < numFields; i++)
		{
			RTTIField* field = rtti->GetField(i);
			if(field->Schema.Info.Flags.IsSet(RTTIFieldFlag::SkipInReferenceSearch))
				continue;

			if(field->Schema.Type == SerializableFT_Reflectable)
			{
				auto reflectableField = static_cast<RTTIReflectableFieldBase*>(field);

				if(reflectableField->GetType()->GetRttiId() == TID_ResourceHandle)
				{
					if(reflectableField->Schema.IsArray)
					{
						const u32 numElements = reflectableField->GetArraySize(rttiInstance, &obj);
						for(u32 j = 0; j < numElements; j++)
						{
							HResource resource = (HResource&)reflectableField->GetArrayValue(rttiInstance, &obj, j);
							if(!resource.GetUuid().Empty())
							{
								ResourceDependency& dependency = dependencies[resource.GetUuid()];
								dependency.Resource = resource;
								dependency.NumReferences++;
							}
						}
					}
					else
					{
						HResource resource = (HResource&)reflectableField->GetValue(rttiInstance, &obj);
						if(!resource.GetUuid().Empty())
						{
							ResourceDependency& dependency = dependencies[resource.GetUuid()];
							dependency.Resource = resource;
							dependency.NumReferences++;
						}
					}
				}
				else if(recursive)
				{
					// Optimization, no need to retrieve its value and go deeper if it has no
					// reflectable children that may hold the reference.
					if(hasReflectableChildren(reflectableField->GetType()))
					{
						if(reflectableField->Schema.IsArray)
						{
							const u32 numElements = reflectableField->GetArraySize(rttiInstance, &obj);
							for(u32 j = 0; j < numElements; j++)
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
			else if(field->Schema.Type == SerializableFT_ReflectablePtr && recursive)
			{
				auto reflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				// Optimization, no need to retrieve its value and go deeper if it has no
				// reflectable children that may hold the reference.
				if(hasReflectableChildren(reflectablePtrField->GetType()))
				{
					if(reflectablePtrField->Schema.IsArray)
					{
						const u32 numElements = reflectablePtrField->GetArraySize(rttiInstance, &obj);
						for(u32 j = 0; j < numElements; j++)
						{
							const SPtr<IReflectable>& childObj =
								reflectablePtrField->GetArrayValue(rttiInstance, &obj, j);

							if(childObj != nullptr)
								findResourceDependenciesInternal(*childObj, alloc, true, dependencies);
						}
					}
					else
					{
						const SPtr<IReflectable>& childObj = reflectablePtrField->GetValue(rttiInstance, &obj);

						if(childObj != nullptr)
							findResourceDependenciesInternal(*childObj, alloc, true, dependencies);
					}
				}
			}
		}

		rttiInstance->OnSerializationEnded(&obj, nullptr);
		alloc.Destruct(rttiInstance);

		rtti = rtti->GetBaseClass();
	}
	while(rtti != nullptr);
}

Vector<ResourceDependency> Utility::FindResourceDependencies(IReflectable& obj, bool recursive)
{
	gFrameAlloc().MarkFrame();

	Map<UUID, ResourceDependency> dependencies;
	findResourceDependenciesInternal(obj, gFrameAlloc(), recursive, dependencies);

	gFrameAlloc().Clear();

	Vector<ResourceDependency> dependencyList(dependencies.size());
	u32 i = 0;
	for(auto& entry : dependencies)
	{
		dependencyList[i] = entry.second;
		i++;
	}

	return dependencyList;
}

u32 Utility::GetSceneObjectDepth(const HSceneObject& so)
{
	HSceneObject parent = so->GetParent();

	u32 depth = 0;
	while(parent != nullptr)
	{
		depth++;
		parent = parent->GetParent();
	}

	return depth;
}

Vector<HComponent> Utility::FindComponents(const HSceneObject& object, u32 typeId)
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
			if(entry->GetRtti()->GetRttiId() == typeId)
				output.push_back(entry);
		}

		u32 numChildren = curSO->GetNumChildren();
		for(u32 i = 0; i < numChildren; i++)
			todo.push(curSO->GetChild(i));
	}

	return output;
}

class CoreSerializationContextRTTI : public RTTIType<CoreSerializationContext, SerializationContext, CoreSerializationContextRTTI>
{
	const String& GetRttiName() override
	{
		static String name = "CoreSerializationContext";
		return name;
	}

	u32 GetRttiId() override
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

