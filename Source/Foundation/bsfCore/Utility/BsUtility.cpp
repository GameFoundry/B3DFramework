//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsUtility.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsSceneObject.h"

namespace bs
{
	Vector<ResourceDependency> Utility::findResourceDependencies(IReflectable& obj, bool recursive)
	{
		Map<UUID, ResourceDependency> dependencies;
		findResourceDependenciesInternal(obj, recursive, dependencies);

		Vector<ResourceDependency> dependencyList(dependencies.size());
		UINT32 i = 0;
		for (auto& entry : dependencies)
		{
			dependencyList[i] = entry.second;
			i++;
		}

		return dependencyList;
	}

	UINT32 Utility::getSceneObjectDepth(const HSceneObject& so)
	{
		HSceneObject parent = so->getParent();
		
		UINT32 depth = 0;
		while (parent != nullptr)
		{
			depth++;
			parent = parent->getParent();
		}

		return depth;
	}

	void Utility::findResourceDependenciesInternal(IReflectable& obj, bool recursive, Map<UUID, ResourceDependency>& dependencies)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		RTTITypeBase* rtti = obj.getRTTI();
		do {
			rtti->onSerializationStarted(&obj, dummyParams);

			const UINT32 numFields = rtti->getNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* field = rtti->getField(i);
				if ((field->getFlags() & RTTI_Flag_SkipInReferenceSearch) != 0)
					continue;

				if (field->isReflectableType())
				{
					auto reflectableField = static_cast<RTTIReflectableFieldBase*>(field);

					if (reflectableField->getType()->getRTTIId() == TID_ResourceHandle)
					{
						if (reflectableField->isArray())
						{
							const UINT32 numElements = reflectableField->getArraySize(rtti, &obj);
							for (UINT32 j = 0; j < numElements; j++)
							{
								HResource resource = (HResource&)reflectableField->getArrayValue(rtti, &obj, j);
								if (!resource.getUUID().empty())
								{
									ResourceDependency& dependency = dependencies[resource.getUUID()];
									dependency.resource = resource;
									dependency.numReferences++;
								}
							}
						}
						else
						{
							HResource resource = (HResource&)reflectableField->getValue(rtti, &obj);
							if (!resource.getUUID().empty())
							{
								ResourceDependency& dependency = dependencies[resource.getUUID()];
								dependency.resource = resource;
								dependency.numReferences++;
							}
						}
					}
					else if (recursive)
					{
						// Optimization, no need to retrieve its value and go deeper if it has no 
						// reflectable children that may hold the reference.
						if (hasReflectableChildren(reflectableField->getType()))
						{
							if (reflectableField->isArray())
							{
								const UINT32 numElements = reflectableField->getArraySize(rtti, &obj);
								for (UINT32 j = 0; j < numElements; j++)
								{
									IReflectable& childObj = reflectableField->getArrayValue(rtti, &obj, j);
									findResourceDependenciesInternal(childObj, true, dependencies);
								}
							}
							else
							{
								IReflectable& childObj = reflectableField->getValue(rtti, &obj);
								findResourceDependenciesInternal(childObj, true, dependencies);
							}
						}
					}
				}
				else if (field->isReflectablePtrType() && recursive)
				{
					auto reflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(field);

					// Optimization, no need to retrieve its value and go deeper if it has no 
					// reflectable children that may hold the reference.
					if (hasReflectableChildren(reflectablePtrField->getType()))
					{
						if (reflectablePtrField->isArray())
						{
							const UINT32 numElements = reflectablePtrField->getArraySize(rtti, &obj);
							for (UINT32 j = 0; j < numElements; j++)
							{
								const SPtr<IReflectable>& childObj = reflectablePtrField->getArrayValue(rtti, &obj, j);

								if (childObj != nullptr)
									findResourceDependenciesInternal(*childObj, true, dependencies);
							}
						}
						else
						{
							const SPtr<IReflectable>& childObj = reflectablePtrField->getValue(rtti, &obj);

							if (childObj != nullptr)
								findResourceDependenciesInternal(*childObj, true, dependencies);
						}
					}
				}
			}

			rtti->onSerializationEnded(&obj, dummyParams);

			rtti = rtti->getBaseClass();
		} while(rtti != nullptr);
	}

	Vector<HComponent> Utility::findComponents(const HSceneObject& object, UINT32 typeId)
	{
		Vector<HComponent> output;

		Stack<HSceneObject> todo;
		todo.push(object);

		while(!todo.empty())
		{
			HSceneObject curSO = todo.top();
			todo.pop();

			const Vector<HComponent>& components = curSO->getComponents();
			for(auto& entry : components)
			{
				if (entry->getRTTI()->getRTTIId() == typeId)
					output.push_back(entry);
			}

			UINT32 numChildren = curSO->getNumChildren();
			for (UINT32 i = 0; i < numChildren; i++)
				todo.push(curSO->getChild(i));
		}

		return output;
	}

	bool Utility::hasReflectableChildren(RTTITypeBase* type)
	{
		UINT32 numFields = type->getNumFields();
		for (UINT32 i = 0; i < numFields; i++)
		{
			RTTIField* field = type->getField(i);
			if (field->isReflectableType() || field->isReflectablePtrType())
				return true;
		}

		const Vector<RTTITypeBase*>& derivedClasses = type->getDerivedClasses();
		for (auto& derivedClass : derivedClasses)
		{
			numFields = derivedClass->getNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* field = derivedClass->getField(i);
				if (field->isReflectableType() || field->isReflectablePtrType())
					return true;
			}
		}

		return false;
	}
}