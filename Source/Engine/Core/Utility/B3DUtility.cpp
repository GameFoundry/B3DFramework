//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DUtility.h"

#include "Reflection/B3DRTTIObjectWrapper.h"
#include "Reflection/B3DRTTIType.h"
#include "Scene/B3DSceneObject.h"

using namespace b3d;

/**
 * Checks if the specified type (or any of its derived classes) have any IReflectable pointer or value types as
 * their fields.
 */
bool HasReflectableChildren(RTTIType* type)
{
	{
		const u32 fieldCount = type->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			const RTTIField* const field = type->GetField(fieldIndex);

			for(const auto& fieldTypeSchema : field->Schema.FieldDataTypes)
			{
				if(fieldTypeSchema.Type == RTTIFieldDataType::Reflectable || fieldTypeSchema.Type == RTTIFieldDataType::ReflectablePointer)
					return true;
			}
		}
	}

	const Vector<RTTIType*>& derivedClasses = type->GetDerivedClasses();
	for(auto& derivedClass : derivedClasses)
	{
		const u32 fieldCount = derivedClass->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			const RTTIField* const field = derivedClass->GetField(fieldIndex);
			for(const auto& fieldTypeSchema : field->Schema.FieldDataTypes)
			{
				if(fieldTypeSchema.Type == RTTIFieldDataType::Reflectable || fieldTypeSchema.Type == RTTIFieldDataType::ReflectablePointer)
					return true;
			}
		}
	}

	return false;
}

static void FindResourceDependenciesRecursive(IReflectable& object, FrameAllocator& allocator, bool recurseChildren, UnorderedMap<UUID, ResourceDependency>& outDependencies)
{
	auto fnFieldFilter = [](const RTTIFieldSchema& fieldSchema)
		{
			if(fieldSchema.Info.Flags.IsSet(RTTIFieldFlag::SkipInReferenceSearch))
				return false;

			return true;
		};

	Function<void(const RTTIFieldDataTypeSchema&, RTTIObjectWrapper::Value<true>&)> fnProcessFieldTupleValue =
		[&outDependencies, &fnFieldFilter, &fnProcessFieldTupleValue, recurseChildren](const RTTIFieldDataTypeSchema& fieldTypeSchema, RTTIObjectWrapper::Value<true>& value)
	{
		if(fieldTypeSchema.Type == RTTIFieldDataType::Reflectable)
		{
			if(fieldTypeSchema.FieldTypeId == TID_StrongResourceHandle)
			{
				const RTTIObjectWrapper::Object<true>& handleObject = value.GetObject();
				HResource* const resource = B3DRTTICast<HResource>(handleObject.GetWrappedObject());
				if(B3D_ENSURE(resource != nullptr))
				{
					if(!resource->GetId().Empty())
					{
						ResourceDependency& dependency = outDependencies[resource->GetId()];
						dependency.Resource = *resource;
						dependency.ReferenceCount++;
					}
				}
			}
		}

		if(recurseChildren)
		{
			if(fieldTypeSchema.Type == RTTIFieldDataType::Reflectable || fieldTypeSchema.Type == RTTIFieldDataType::ReflectablePointer)
			{
				RTTIObjectWrapper::Object<true> wrappedChildObject = value.GetObject();
				RTTIObjectWrapper::IterateFieldTupleValues(wrappedChildObject, RTTIOperationType::GatherReferences, fnProcessFieldTupleValue, fnFieldFilter);
			}
		}
	};

	RTTIType* const rtti = object.GetRtti();
	RTTIObjectWrapper::Object<true> wrappedObject(&object, rtti, &allocator);
	RTTIObjectWrapper::IterateFieldTupleValues(wrappedObject, RTTIOperationType::GatherReferences, fnProcessFieldTupleValue, fnFieldFilter);
}

Vector<ResourceDependency> Utility::FindResourceDependencies(IReflectable& obj, bool recursive)
{
	GetFrameAllocator().MarkFrame();

	UnorderedMap<UUID, ResourceDependency> dependencies;
	FindResourceDependenciesRecursive(obj, GetFrameAllocator(), recursive, dependencies);

	GetFrameAllocator().Clear();

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

		u32 numChildren = curSO->GetChildCount();
		for(u32 i = 0; i < numChildren; i++)
			todo.push(curSO->GetChild(i));
	}

	return output;
}

class RTTIOperationEngineContextRTTI : public TRTTIType<RTTIOperationEngineContext, RTTIOperationContext, RTTIOperationEngineContextRTTI>
{
	const String& GetRttiName() override
	{
		static String name = "RTTIOperationEngineContext";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_RTTIOperationEngineContext;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		B3D_ASSERT(false && "Cannot instantiate an abstract class.");
		return nullptr;
	}
};

RTTIType* RTTIOperationEngineContext::GetRttiStatic()
{
	return RTTIOperationEngineContextRTTI::Instance();
}

RTTIType* RTTIOperationEngineContext::GetRtti() const
{
	return GetRttiStatic();
}

