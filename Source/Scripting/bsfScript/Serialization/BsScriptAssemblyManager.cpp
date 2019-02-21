//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"
#include "BsMonoMethod.h"
#include "BsMonoProperty.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs
{
	BuiltinTypeMappings BuiltinTypeMappings::EMPTY;

	Vector<String> ScriptAssemblyManager::getScriptAssemblies() const
	{
		Vector<String> initializedAssemblies;
		for (auto& assemblyPair : mAssemblyInfos)
			initializedAssemblies.push_back(assemblyPair.first);

		return initializedAssemblies;
	}

	void ScriptAssemblyManager::loadAssemblyInfo(const String& assemblyName, const BuiltinTypeMappings& typeMappings)
	{
		if(!mBaseTypesInitialized)
			initializeBaseTypes();

		// Process all classes and fields
		UINT32 mUniqueTypeId = 1;

		MonoAssembly* curAssembly = MonoManager::instance().getAssembly(assemblyName);
		if(curAssembly == nullptr)
			return;

		loadTypeMappings(*curAssembly, typeMappings);

		SPtr<ManagedSerializableAssemblyInfo> assemblyInfo = bs_shared_ptr_new<ManagedSerializableAssemblyInfo>();
		assemblyInfo->mName = assemblyName;

		mAssemblyInfos[assemblyName] = assemblyInfo;

		MonoClass* resourceClass = ScriptResource::getMetaData()->scriptClass;
		MonoClass* managedResourceClass = ScriptManagedResource::getMetaData()->scriptClass;

		// Populate class data
		const Vector<MonoClass*>& allClasses = curAssembly->getAllClasses();
		for(auto& curClass : allClasses)
		{
			const bool isSerializable = 
				curClass->isSubClassOf(mBuiltin.componentClass) || 
				curClass->isSubClassOf(resourceClass) ||
				curClass->hasAttribute(mBuiltin.serializeObjectAttribute);

			const bool isInspectable =
				curClass->hasAttribute(mBuiltin.showInInspectorAttribute);

			if ((isSerializable || isInspectable) &&
				curClass != mBuiltin.componentClass && curClass != resourceClass &&
				curClass != mBuiltin.managedComponentClass && curClass != managedResourceClass)
			{
				SPtr<ManagedSerializableTypeInfoObject> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoObject>();
				typeInfo->mTypeNamespace = curClass->getNamespace();
				typeInfo->mTypeName = curClass->getTypeName();
				typeInfo->mTypeId = mUniqueTypeId++;

				if(isSerializable)
					typeInfo->mFlags |= ScriptTypeFlag::Serializable;

				if(isSerializable || isInspectable)
					typeInfo->mFlags |= ScriptTypeFlag::Inspectable;

				MonoPrimitiveType monoPrimitiveType = MonoUtil::getPrimitiveType(curClass->_getInternalClass());

				if(monoPrimitiveType == MonoPrimitiveType::ValueType)
					typeInfo->mValueType = true;
				else
					typeInfo->mValueType = false;

				SPtr<ManagedSerializableObjectInfo> objInfo = bs_shared_ptr_new<ManagedSerializableObjectInfo>();

				objInfo->mTypeInfo = typeInfo;
				objInfo->mMonoClass = curClass;

				assemblyInfo->mTypeNameToId[objInfo->getFullTypeName()] = typeInfo->mTypeId;
				assemblyInfo->mObjectInfos[typeInfo->mTypeId] = objInfo;
			}
		}

		// Populate field & property data
		for(auto& curClassInfo : assemblyInfo->mObjectInfos)
		{
			SPtr<ManagedSerializableObjectInfo> objInfo = curClassInfo.second;

			UINT32 mUniqueFieldId = 1;

			const Vector<MonoField*>& fields = objInfo->mMonoClass->getAllFields();
			for(auto& field : fields)
			{
				if(field->isStatic())
					continue;

				SPtr<ManagedSerializableTypeInfo> typeInfo = getTypeInfo(field->getType());
				if (typeInfo == nullptr)
					continue;

				bool typeIsSerializable = true;
				bool typeIsInspectable = true;

				if(const auto* objTypeInfo = rtti_cast<ManagedSerializableTypeInfoObject>(typeInfo.get()))
				{
					typeIsSerializable = objTypeInfo->mFlags.isSet(ScriptTypeFlag::Serializable);
					typeIsInspectable = typeIsSerializable || objTypeInfo->mFlags.isSet(ScriptTypeFlag::Inspectable);
				}

				SPtr<ManagedSerializableFieldInfo> fieldInfo = bs_shared_ptr_new<ManagedSerializableFieldInfo>();
				fieldInfo->mFieldId = mUniqueFieldId++;
				fieldInfo->mName = field->getName();
				fieldInfo->mMonoField = field;
				fieldInfo->mTypeInfo = typeInfo;
				fieldInfo->mParentTypeId = objInfo->mTypeInfo->mTypeId;
				
				MonoMemberVisibility visibility = field->getVisibility();
				if (visibility == MonoMemberVisibility::Public)
				{
					if (typeIsSerializable && !field->hasAttribute(mBuiltin.dontSerializeFieldAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Serializable;

					if (typeIsInspectable && !field->hasAttribute(mBuiltin.hideInInspectorAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Inspectable;

					fieldInfo->mFlags |= ScriptFieldFlag::Animable;
				}
				else
				{
					if (typeIsSerializable && field->hasAttribute(mBuiltin.serializeFieldAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Serializable;

					if (typeIsInspectable && field->hasAttribute(mBuiltin.showInInspectorAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Inspectable;
				}

				if (field->hasAttribute(mBuiltin.rangeAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::Range;

				if (field->hasAttribute(mBuiltin.stepAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::Step;

				if (field->hasAttribute(mBuiltin.layerMaskAttribute))
				{
					// Layout mask attribute is only relevant for 64-bit integer types
					if (const auto* primTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
					{
						if (primTypeInfo->mType == ScriptPrimitiveType::I64 ||
							primTypeInfo->mType == ScriptPrimitiveType::U64)
						{
							fieldInfo->mFlags |= ScriptFieldFlag::AsLayerMask;
						}
					}
				}

				if (field->hasAttribute(mBuiltin.asQuaternionAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::AsQuaternion;

				if(field->hasAttribute(mBuiltin.notNullAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::NotNull;

				if(field->hasAttribute(mBuiltin.categoryAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::Category;

				if(field->hasAttribute(mBuiltin.orderAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::Order;

				if(field->hasAttribute(mBuiltin.inlineAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::Inline;

				objInfo->mFieldNameToId[fieldInfo->mName] = fieldInfo->mFieldId;
				objInfo->mFields[fieldInfo->mFieldId] = fieldInfo;
			}

			const Vector<MonoProperty*>& properties = objInfo->mMonoClass->getAllProperties();
			for (auto& property : properties)
			{
				SPtr<ManagedSerializableTypeInfo> typeInfo = getTypeInfo(property->getReturnType());
				if (typeInfo == nullptr)
					continue;

				bool typeIsSerializable = true;
				bool typeIsInspectable = true;

				if(const auto* objTypeInfo = rtti_cast<ManagedSerializableTypeInfoObject>(typeInfo.get()))
				{
					typeIsSerializable = objTypeInfo->mFlags.isSet(ScriptTypeFlag::Serializable);
					typeIsInspectable = typeIsSerializable || objTypeInfo->mFlags.isSet(ScriptTypeFlag::Inspectable);
				}

				SPtr<ManagedSerializablePropertyInfo> propertyInfo = bs_shared_ptr_new<ManagedSerializablePropertyInfo>();
				propertyInfo->mFieldId = mUniqueFieldId++;
				propertyInfo->mName = property->getName();
				propertyInfo->mMonoProperty = property;
				propertyInfo->mTypeInfo = typeInfo;
				propertyInfo->mParentTypeId = objInfo->mTypeInfo->mTypeId;

				if (!property->isIndexed())
				{
					MonoMemberVisibility visibility = property->getVisibility();
					if (visibility == MonoMemberVisibility::Public)
						propertyInfo->mFlags |= ScriptFieldFlag::Animable;

					if (typeIsSerializable && property->hasAttribute(mBuiltin.serializeFieldAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Serializable;

					if (typeIsInspectable && property->hasAttribute(mBuiltin.showInInspectorAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Inspectable;

					if (property->hasAttribute(mBuiltin.rangeAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Range;

					if (property->hasAttribute(mBuiltin.stepAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Step;

					if (property->hasAttribute(mBuiltin.layerMaskAttribute))
					{
						// Layout mask attribute is only relevant for 64-bit integer types
						if (const auto* primTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
						{
							if (primTypeInfo->mType == ScriptPrimitiveType::I64 ||
								primTypeInfo->mType == ScriptPrimitiveType::U64)
							{
								propertyInfo->mFlags |= ScriptFieldFlag::AsLayerMask;
							}
						}
					}

					if (property->hasAttribute(mBuiltin.asQuaternionAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::AsQuaternion;

					if (property->hasAttribute(mBuiltin.notNullAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::NotNull;

					if (property->hasAttribute(mBuiltin.passByCopyAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::PassByCopy;

					if (property->hasAttribute(mBuiltin.applyOnDirtyAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::ApplyOnDirty;

					if (property->hasAttribute(mBuiltin.nativeWrapperAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::NativeWrapper;

					if (property->hasAttribute(mBuiltin.categoryAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Category;

					if (property->hasAttribute(mBuiltin.orderAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Order;

					if (property->hasAttribute(mBuiltin.inlineAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Inline;
				}

				objInfo->mFieldNameToId[propertyInfo->mName] = propertyInfo->mFieldId;
				objInfo->mFields[propertyInfo->mFieldId] = propertyInfo;
			}
		}

		// Form parent/child connections
		for(auto& curClass : assemblyInfo->mObjectInfos)
		{
			MonoClass* base = curClass.second->mMonoClass->getBaseClass();
			while(base != nullptr)
			{
				SPtr<ManagedSerializableObjectInfo> baseObjInfo;
				if(getSerializableObjectInfo(base->getNamespace(), base->getTypeName(), baseObjInfo))
				{
					curClass.second->mBaseClass = baseObjInfo;
					baseObjInfo->mDerivedClasses.push_back(curClass.second);

					break;
				}

				base = base->getBaseClass();
			}
		}
	}

	void ScriptAssemblyManager::clearAssemblyInfo()
	{
		clearScriptObjects();
		mAssemblyInfos.clear();

		mBuiltinComponentInfos.clear();
		mBuiltinComponentInfosByTID.clear();

		mBuiltinResourceInfos.clear();
		mBuiltinResourceInfosByTID.clear();
		mBuiltinResourceInfosByType.clear();

		mReflectableTypeInfos.clear();
		mReflectableTypeInfosByTID.clear();
	}

	SPtr<ManagedSerializableTypeInfo> ScriptAssemblyManager::getTypeInfo(MonoClass* monoClass)
	{
		if(!mBaseTypesInitialized)
			BS_EXCEPT(InvalidStateException, "Calling getTypeInfo without previously initializing base types.");

		MonoPrimitiveType monoPrimitiveType = MonoUtil::getPrimitiveType(monoClass->_getInternalClass());
		
		// If enum get the enum base data type
		bool isEnum = MonoUtil::isEnum(monoClass->_getInternalClass());
		if (isEnum)
			monoPrimitiveType = MonoUtil::getEnumPrimitiveType(monoClass->_getInternalClass());

		//  Determine field type
		//// Check for simple types or enums first
		ScriptPrimitiveType scriptPrimitiveType = ScriptPrimitiveType::U32;
		bool isSimpleType = false;
		switch(monoPrimitiveType)
		{
		case MonoPrimitiveType::Boolean:
			scriptPrimitiveType = ScriptPrimitiveType::Bool;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::Char:
			scriptPrimitiveType = ScriptPrimitiveType::Char;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::I8:
			scriptPrimitiveType = ScriptPrimitiveType::I8;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::U8:
			scriptPrimitiveType = ScriptPrimitiveType::U8;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::I16:
			scriptPrimitiveType = ScriptPrimitiveType::I16;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::U16:
			scriptPrimitiveType = ScriptPrimitiveType::U16;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::I32:
			scriptPrimitiveType = ScriptPrimitiveType::I32;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::U32:
			scriptPrimitiveType = ScriptPrimitiveType::U32;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::I64:
			scriptPrimitiveType = ScriptPrimitiveType::I64;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::U64:
			scriptPrimitiveType = ScriptPrimitiveType::U64;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::String:
			scriptPrimitiveType = ScriptPrimitiveType::String;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::R32:
			scriptPrimitiveType = ScriptPrimitiveType::Float;
			isSimpleType = true;
			break;
		case MonoPrimitiveType::R64:
			scriptPrimitiveType = ScriptPrimitiveType::Double;
			isSimpleType = true;
			break;
		default:
			break;
		};

		if(isSimpleType)
		{
			if(!isEnum)
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = scriptPrimitiveType;
				return typeInfo;
			}
			else
			{
				SPtr<ManagedSerializableTypeInfoEnum> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoEnum>();
				typeInfo->mUnderlyingType = scriptPrimitiveType;
				typeInfo->mTypeNamespace = monoClass->getNamespace();
				typeInfo->mTypeName = monoClass->getTypeName();
				return typeInfo;
			}
		}

		//// Check complex types
		switch(monoPrimitiveType)
		{
		case MonoPrimitiveType::Class:
			if(monoClass->isSubClassOf(ScriptResource::getMetaData()->scriptClass)) // Resource
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				typeInfo->mTypeNamespace = monoClass->getNamespace();
				typeInfo->mTypeName = monoClass->getTypeName();
				typeInfo->mRTIITypeId = 0;

				if(monoClass == ScriptResource::getMetaData()->scriptClass)
					typeInfo->mType = ScriptReferenceType::BuiltinResourceBase;
				else if (monoClass == ScriptManagedResource::getMetaData()->scriptClass)
					typeInfo->mType = ScriptReferenceType::ManagedResourceBase;
				else if (monoClass->isSubClassOf(ScriptManagedResource::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::ManagedResource;
				else if (monoClass->isSubClassOf(ScriptResource::getMetaData()->scriptClass))
				{
					typeInfo->mType = ScriptReferenceType::BuiltinResource;

					::MonoReflectionType* type = MonoUtil::getType(monoClass->_getInternalClass());
					BuiltinResourceInfo* builtinInfo = getBuiltinResourceInfo(type);
					if (builtinInfo == nullptr)
					{
						assert(false && "Unable to find information about a built-in resource. Did you update BuiltinResourceTypes list?");
						return nullptr;
					}

					typeInfo->mRTIITypeId = builtinInfo->typeId;
				}

				return typeInfo;
			}
			else if(monoClass == ScriptRRefBase::getMetaData()->scriptClass) // Resource reference
				return bs_shared_ptr_new<ManagedSerializableTypeInfoRRef>();
			else if (monoClass->isSubClassOf(mBuiltin.sceneObjectClass) || monoClass->isSubClassOf(mBuiltin.componentClass)) // Game object
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				typeInfo->mTypeNamespace = monoClass->getNamespace();
				typeInfo->mTypeName = monoClass->getTypeName();
				typeInfo->mRTIITypeId = 0;

				if (monoClass == mBuiltin.componentClass)
					typeInfo->mType = ScriptReferenceType::BuiltinComponentBase;
				else if (monoClass == mBuiltin.managedComponentClass)
					typeInfo->mType = ScriptReferenceType::ManagedComponentBase;
				else if (monoClass->isSubClassOf(mBuiltin.sceneObjectClass))
					typeInfo->mType = ScriptReferenceType::SceneObject;
				else if (monoClass->isSubClassOf(mBuiltin.managedComponentClass))
					typeInfo->mType = ScriptReferenceType::ManagedComponent;
				else if (monoClass->isSubClassOf(mBuiltin.componentClass))
				{
					typeInfo->mType = ScriptReferenceType::BuiltinComponent;

					::MonoReflectionType* type = MonoUtil::getType(monoClass->_getInternalClass());
					BuiltinComponentInfo* builtinInfo = getBuiltinComponentInfo(type);
					if(builtinInfo == nullptr)
					{
						assert(false && "Unable to find information about a built-in component. Did you update BuiltinComponents list?");
						return nullptr;
					}

					typeInfo->mRTIITypeId = builtinInfo->typeId;
				}

				return typeInfo;
			}
			else
			{
				::MonoReflectionType* type = MonoUtil::getType(monoClass->_getInternalClass());

				// Is this a wrapper for some reflectable type?
				//ReflectableTypeInfo* reflTypeInfo = getReflectableTypeInfo(type);
				//if(reflTypeInfo != nullptr)
				//{
				//	SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				//	typeInfo->mTypeNamespace = monoClass->getNamespace();
				//	typeInfo->mTypeName = monoClass->getTypeName();
				//	typeInfo->mRTIITypeId = reflTypeInfo->typeId;
				//	typeInfo->mType = ScriptReferenceType::ReflectableObject;

				//	return typeInfo;
				//}
				//else
				{
					// Finally, it's either a normal managed object, or a non-reflectable type wrapper
					SPtr<ManagedSerializableObjectInfo> objInfo;
					if (getSerializableObjectInfo(monoClass->getNamespace(), monoClass->getTypeName(), objInfo))
						return objInfo->mTypeInfo;
				}
			}

			break;
		case MonoPrimitiveType::ValueType:
			{
				SPtr<ManagedSerializableObjectInfo> objInfo;
				if (getSerializableObjectInfo(monoClass->getNamespace(), monoClass->getTypeName(), objInfo))
					return objInfo->mTypeInfo;
			}

			break;
		case MonoPrimitiveType::Generic:
			if(monoClass->getFullName() == mBuiltin.systemGenericListClass->getFullName()) // Full name is part of CIL spec, so it is just fine to compare like this
			{
				SPtr<ManagedSerializableTypeInfoList> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoList>();

				MonoProperty* itemProperty = monoClass->getProperty("Item");
				MonoClass* itemClass = itemProperty->getReturnType();

				if (itemClass != nullptr)
					typeInfo->mElementType = getTypeInfo(itemClass);
				
				if (typeInfo->mElementType == nullptr)
					return nullptr;

				return typeInfo;
			}
			else if(monoClass->getFullName() == mBuiltin.systemGenericDictionaryClass->getFullName())
			{
				SPtr<ManagedSerializableTypeInfoDictionary> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoDictionary>();

				MonoMethod* getEnumerator = monoClass->getMethod("GetEnumerator");
				MonoClass* enumClass = getEnumerator->getReturnType();

				MonoProperty* currentProp = enumClass->getProperty("Current");
				MonoClass* keyValuePair = currentProp->getReturnType();

				MonoProperty* keyProperty = keyValuePair->getProperty("Key");
				MonoProperty* valueProperty = keyValuePair->getProperty("Value");

				MonoClass* keyClass = keyProperty->getReturnType();
				if(keyClass != nullptr)
					typeInfo->mKeyType = getTypeInfo(keyClass);

				MonoClass* valueClass = valueProperty->getReturnType();
				if(valueClass != nullptr)
					typeInfo->mValueType = getTypeInfo(valueClass);

				if (typeInfo->mKeyType == nullptr || typeInfo->mValueType == nullptr)
					return nullptr;

				return typeInfo;
			}
			else if(monoClass->getFullName() == mBuiltin.genericRRefClass->getFullName())
			{
				SPtr<ManagedSerializableTypeInfoRRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRRef>();
				
				MonoProperty* itemProperty = monoClass->getProperty("Value");
				MonoClass* itemClass = itemProperty->getReturnType();

				if (itemClass != nullptr)
					typeInfo->mResourceType = getTypeInfo(itemClass);
				
				if (typeInfo->mResourceType == nullptr)
					return nullptr;

				return typeInfo;
			}
			break;
		case MonoPrimitiveType::Array:
			{
				SPtr<ManagedSerializableTypeInfoArray> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoArray>();

				::MonoClass* elementClass = ScriptArray::getElementClass(monoClass->_getInternalClass());
				if(elementClass != nullptr)
				{
					MonoClass* monoElementClass = MonoManager::instance().findClass(elementClass);
					if(monoElementClass != nullptr)
						typeInfo->mElementType = getTypeInfo(monoElementClass);
				}

				if (typeInfo->mElementType == nullptr)
					return nullptr;

				typeInfo->mRank = ScriptArray::getRank(monoClass->_getInternalClass());

				return typeInfo;
			}
		default:
			break;
		}

		return nullptr;
	}

	void ScriptAssemblyManager::clearScriptObjects()
	{
		mBaseTypesInitialized = false;
		mBuiltin = BuiltinScriptClasses();
	}

	void ScriptAssemblyManager::initializeBaseTypes()
	{
		// Get necessary classes for detecting needed class & field information
		MonoAssembly* corlib = MonoManager::instance().getAssembly("corlib");
		if(corlib == nullptr)
			BS_EXCEPT(InvalidStateException, "corlib assembly is not loaded.");

		MonoAssembly* engineAssembly = MonoManager::instance().getAssembly(ENGINE_ASSEMBLY);
		if(engineAssembly == nullptr)
			BS_EXCEPT(InvalidStateException, String(ENGINE_ASSEMBLY) +  " assembly is not loaded.");

		mBuiltin.systemArrayClass = corlib->getClass("System", "Array");
		if(mBuiltin.systemArrayClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find System.Array managed class.");

		mBuiltin.systemGenericListClass = corlib->getClass("System.Collections.Generic", "List`1");
		if(mBuiltin.systemGenericListClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find List<T> managed class.");

		mBuiltin.systemGenericDictionaryClass = corlib->getClass("System.Collections.Generic", "Dictionary`2");
		if(mBuiltin.systemGenericDictionaryClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Dictionary<TKey, TValue> managed class.");

		mBuiltin.systemTypeClass = corlib->getClass("System", "Type");
		if (mBuiltin.systemTypeClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Type managed class.");

		mBuiltin.serializeObjectAttribute = engineAssembly->getClass(ENGINE_NS, "SerializeObject");
		if(mBuiltin.serializeObjectAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SerializableObject managed class.");

		mBuiltin.dontSerializeFieldAttribute = engineAssembly->getClass(ENGINE_NS, "DontSerializeField");
		if(mBuiltin.dontSerializeFieldAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find DontSerializeField managed class.");

		mBuiltin.rangeAttribute = engineAssembly->getClass(ENGINE_NS, "Range");
		if (mBuiltin.rangeAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Range managed class.");

		mBuiltin.stepAttribute = engineAssembly->getClass(ENGINE_NS, "Step");
		if (mBuiltin.stepAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Step managed class.");

		mBuiltin.layerMaskAttribute = engineAssembly->getClass(ENGINE_NS, "LayerMask");
		if (mBuiltin.layerMaskAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find LayerMask managed class.");

		mBuiltin.asQuaternionAttribute = engineAssembly->getClass(ENGINE_NS, "AsQuaternion");
		if (mBuiltin.asQuaternionAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find AsQuaternion managed class.");

		mBuiltin.nativeWrapperAttribute = engineAssembly->getClass(ENGINE_NS, "NativeWrapper");
		if (mBuiltin.nativeWrapperAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find NativeWrapper managed class.");

		mBuiltin.notNullAttribute = engineAssembly->getClass(ENGINE_NS, "NotNull");
		if (mBuiltin.notNullAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find NotNull managed class.");

		mBuiltin.passByCopyAttribute = engineAssembly->getClass(ENGINE_NS, "PassByCopy");
		if (mBuiltin.passByCopyAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find PassByCopy managed class.");

		mBuiltin.applyOnDirtyAttribute = engineAssembly->getClass(ENGINE_NS, "ApplyOnDirty");
		if (mBuiltin.applyOnDirtyAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find ApplyOnDirty managed class.");

		mBuiltin.componentClass = engineAssembly->getClass(ENGINE_NS, "Component");
		if(mBuiltin.componentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Component managed class.");

		mBuiltin.managedComponentClass = engineAssembly->getClass(ENGINE_NS, "ManagedComponent");
		if (mBuiltin.managedComponentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find ManagedComponent managed class.");

		mBuiltin.missingComponentClass = engineAssembly->getClass(ENGINE_NS, "MissingComponent");
		if (mBuiltin.missingComponentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find MissingComponent managed class.");

		mBuiltin.sceneObjectClass = engineAssembly->getClass(ENGINE_NS, "SceneObject");
		if(mBuiltin.sceneObjectClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SceneObject managed class.");

		mBuiltin.rrefBaseClass = engineAssembly->getClass(ENGINE_NS, "RRefBase");
		if(mBuiltin.rrefBaseClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find RRefBase managed class.");

		mBuiltin.genericRRefClass = engineAssembly->getClass(ENGINE_NS, "RRef`1");
		if(mBuiltin.genericRRefClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find RRef<T> managed class.");

		mBuiltin.serializeFieldAttribute = engineAssembly->getClass(ENGINE_NS, "SerializeField");
		if(mBuiltin.serializeFieldAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SerializeField managed class.");

		mBuiltin.hideInInspectorAttribute = engineAssembly->getClass(ENGINE_NS, "HideInInspector");
		if(mBuiltin.hideInInspectorAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find HideInInspector managed class.");

		mBuiltin.showInInspectorAttribute = engineAssembly->getClass(ENGINE_NS, "ShowInInspector");
		if (mBuiltin.showInInspectorAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find ShowInInspector managed class.");

		mBuiltin.categoryAttribute = engineAssembly->getClass(ENGINE_NS, "Category");
		if (mBuiltin.categoryAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Category managed class.");

		mBuiltin.orderAttribute = engineAssembly->getClass(ENGINE_NS, "Order");
		if (mBuiltin.orderAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Order managed class.");

		mBuiltin.inlineAttribute = engineAssembly->getClass(ENGINE_NS, "Inline");
		if (mBuiltin.inlineAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Inline managed class.");

		mBaseTypesInitialized = true;
	}

	void ScriptAssemblyManager::loadTypeMappings(MonoAssembly& assembly, const BuiltinTypeMappings& mapping)
	{
		for(auto& entry : mapping.components)
		{
			BuiltinComponentInfo info = entry;
			info.monoClass = assembly.getClass(entry.metaData->ns, entry.metaData->name);

			::MonoReflectionType* type = MonoUtil::getType(info.monoClass->_getInternalClass());

			mBuiltinComponentInfos[type] = info;
			mBuiltinComponentInfosByTID[info.typeId] = info;
		}

		for (auto& entry : mapping.resources)
		{
			BuiltinResourceInfo info = entry;
			info.monoClass = assembly.getClass(entry.metaData->ns, entry.metaData->name);

			::MonoReflectionType* type = MonoUtil::getType(info.monoClass->_getInternalClass());

			mBuiltinResourceInfos[type] = info;
			mBuiltinResourceInfosByTID[info.typeId] = info;
			mBuiltinResourceInfosByType[(UINT32)info.resType] = info;
		}

		for(auto& entry : mapping.reflectableObjects)
		{
			ReflectableTypeInfo info = entry;
			info.monoClass = assembly.getClass(entry.metaData->ns, entry.metaData->name);

			::MonoReflectionType* type = MonoUtil::getType(info.monoClass->_getInternalClass());

			mReflectableTypeInfos[type] = info;
			mReflectableTypeInfosByTID[info.typeId] = info;
		}
	}

	BuiltinComponentInfo* ScriptAssemblyManager::getBuiltinComponentInfo(::MonoReflectionType* type)
	{
		auto iterFind = mBuiltinComponentInfos.find(type);
		if (iterFind == mBuiltinComponentInfos.end())
			return nullptr;

		return &(iterFind->second);
	}

	BuiltinComponentInfo* ScriptAssemblyManager::getBuiltinComponentInfo(UINT32 rttiTypeId)
	{
		auto iterFind = mBuiltinComponentInfosByTID.find(rttiTypeId);
		if (iterFind == mBuiltinComponentInfosByTID.end())
			return nullptr;

		return &(iterFind->second);
	}

	BuiltinResourceInfo* ScriptAssemblyManager::getBuiltinResourceInfo(::MonoReflectionType* type)
	{
		auto iterFind = mBuiltinResourceInfos.find(type);
		if (iterFind == mBuiltinResourceInfos.end())
			return nullptr;

		return &(iterFind->second);
	}

	BuiltinResourceInfo* ScriptAssemblyManager::getBuiltinResourceInfo(UINT32 rttiTypeId)
	{
		auto iterFind = mBuiltinResourceInfosByTID.find(rttiTypeId);
		if (iterFind == mBuiltinResourceInfosByTID.end())
			return nullptr;

		return &(iterFind->second);
	}

	BuiltinResourceInfo* ScriptAssemblyManager::getBuiltinResourceInfo(ScriptResourceType type)
	{
		auto iterFind = mBuiltinResourceInfosByType.find((UINT32)type);
		if (iterFind == mBuiltinResourceInfosByType.end())
			return nullptr;

		return &(iterFind->second);
	}

	ReflectableTypeInfo* ScriptAssemblyManager::getReflectableTypeInfo(::MonoReflectionType* type)
	{
		auto iterFind = mReflectableTypeInfos.find(type);
		if (iterFind == mReflectableTypeInfos.end())
			return nullptr;

		return &(iterFind->second);
	}

	ReflectableTypeInfo* ScriptAssemblyManager::getReflectableTypeInfo(uint32_t rttiTypeId)
	{
		auto iterFind = mReflectableTypeInfosByTID.find(rttiTypeId);
		if (iterFind == mReflectableTypeInfosByTID.end())
			return nullptr;

		return &(iterFind->second);
	}

	bool ScriptAssemblyManager::getSerializableObjectInfo(const String& ns, const String& typeName, SPtr<ManagedSerializableObjectInfo>& outInfo)
	{
		String fullName = ns + "." + typeName;
		for(auto& curAssembly : mAssemblyInfos)
		{
			if (curAssembly.second == nullptr)
				continue;

			auto iterFind = curAssembly.second->mTypeNameToId.find(fullName);
			if(iterFind != curAssembly.second->mTypeNameToId.end())
			{
				outInfo = curAssembly.second->mObjectInfos[iterFind->second];

				return true;
			}
		}

		return false;
	}

	bool ScriptAssemblyManager::hasSerializableObjectInfo(const String& ns, const String& typeName)
	{
		String fullName = ns + "." + typeName;
		for(auto& curAssembly : mAssemblyInfos)
		{
			auto iterFind = curAssembly.second->mTypeNameToId.find(fullName);
			if(iterFind != curAssembly.second->mTypeNameToId.end())
				return true;
		}

		return false;
	}
}
