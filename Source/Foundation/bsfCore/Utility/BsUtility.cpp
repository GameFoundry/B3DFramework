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

	void findResourceDependenciesInternal(IReflectable& obj, FrameAlloc& alloc, bool recursive, 
		Map<UUID, ResourceDependency>& dependencies)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		RTTITypeBase* rtti = obj.getRTTI();
		do {
			RTTITypeBase* rttiInstance = rtti->_clone(alloc);
			rttiInstance->onSerializationStarted(&obj, dummyParams);

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
							const UINT32 numElements = reflectableField->getArraySize(rttiInstance, &obj);
							for (UINT32 j = 0; j < numElements; j++)
							{
								HResource resource = (HResource&)reflectableField->getArrayValue(rttiInstance, &obj, j);
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
							HResource resource = (HResource&)reflectableField->getValue(rttiInstance, &obj);
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
								const UINT32 numElements = reflectableField->getArraySize(rttiInstance, &obj);
								for (UINT32 j = 0; j < numElements; j++)
								{
									IReflectable& childObj = reflectableField->getArrayValue(rttiInstance, &obj, j);
									findResourceDependenciesInternal(childObj, alloc, true, dependencies);
								}
							}
							else
							{
								IReflectable& childObj = reflectableField->getValue(rttiInstance, &obj);
								findResourceDependenciesInternal(childObj, alloc, true, dependencies);
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
							const UINT32 numElements = reflectablePtrField->getArraySize(rttiInstance, &obj);
							for (UINT32 j = 0; j < numElements; j++)
							{
								const SPtr<IReflectable>& childObj = 
									reflectablePtrField->getArrayValue(rttiInstance, &obj, j);

								if (childObj != nullptr)
									findResourceDependenciesInternal(*childObj, alloc, true, dependencies);
							}
						}
						else
						{
							const SPtr<IReflectable>& childObj = reflectablePtrField->getValue(rttiInstance, &obj);

							if (childObj != nullptr)
								findResourceDependenciesInternal(*childObj, alloc, true, dependencies);
						}
					}
				}
			}

			rttiInstance->onSerializationEnded(&obj, dummyParams);
			alloc.destruct(rttiInstance);

			rtti = rtti->getBaseClass();
		} while(rtti != nullptr);
	}

	Vector<ResourceDependency> Utility::findResourceDependencies(IReflectable& obj, bool recursive)
	{
		gFrameAlloc().markFrame();

		Map<UUID, ResourceDependency> dependencies;
		findResourceDependenciesInternal(obj, gFrameAlloc(), recursive, dependencies);

		gFrameAlloc().clear();

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
}