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
#include "Wrappers/BsScriptReflectable.h"
#include "BsManagedSerializableObject.h"
#include "BsManagedResource.h"
#include "Wrappers/BsScriptManagedComponent.h"
#include "BsManagedComponent.h"

using namespace bs;
const BuiltinTypeMappings BuiltinTypeMappings::kEmpty;

Vector<String> ScriptAssemblyManager::GetScriptAssemblies() const
{
	Vector<String> initializedAssemblies;
	for(auto& assemblyPair : mAssemblyInfos)
		initializedAssemblies.push_back(assemblyPair.first);

	return initializedAssemblies;
}

void ScriptAssemblyManager::LoadAssemblyInfo(const String& assemblyName, const BuiltinTypeMappings& typeMappings)
{
	if(!mBaseTypesInitialized)
		InitializeBaseTypes();

	// Process all classes and fields
	u32 mUniqueTypeId = 1;

	MonoAssembly* curAssembly = MonoManager::Instance().GetAssembly(assemblyName);
	if(curAssembly == nullptr)
		return;

	LoadTypeMappings(*curAssembly, typeMappings);

	SPtr<ManagedSerializableAssemblyInfo> assemblyInfo = bs_shared_ptr_new<ManagedSerializableAssemblyInfo>();
	assemblyInfo->MName = assemblyName;

	mAssemblyInfos[assemblyName] = assemblyInfo;

	MonoClass* resourceClass = ScriptResource::GetMetaData()->ScriptClass;
	MonoClass* managedResourceClass = ScriptManagedResource::GetMetaData()->ScriptClass;

	// Populate class data
	const Vector<MonoClass*>& allClasses = curAssembly->GetAllClasses();
	for(auto& curClass : allClasses)
	{
		const bool isSerializable =
			curClass->IsSubClassOf(mBuiltin.ComponentClass) ||
			curClass->IsSubClassOf(resourceClass) ||
			curClass->HasAttribute(mBuiltin.SerializeObjectAttribute);

		const bool isInspectable =
			curClass->HasAttribute(mBuiltin.ShowInInspectorAttribute);

		if((isSerializable || isInspectable) &&
		   curClass != mBuiltin.ComponentClass && curClass != resourceClass &&
		   curClass != mBuiltin.ManagedComponentClass && curClass != managedResourceClass)
		{
			SPtr<ManagedSerializableTypeInfoObject> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoObject>();
			typeInfo->MTypeNamespace = curClass->GetNamespace();
			typeInfo->MTypeName = curClass->GetTypeName();
			typeInfo->MTypeId = mUniqueTypeId++;

			if(isSerializable)
				typeInfo->MFlags |= ScriptTypeFlag::Serializable;

			if(isSerializable || isInspectable)
				typeInfo->MFlags |= ScriptTypeFlag::Inspectable;

			MonoPrimitiveType monoPrimitiveType = MonoUtil::GetPrimitiveType(curClass->GetInternalClassInternal());

			if(monoPrimitiveType == MonoPrimitiveType::ValueType)
				typeInfo->MValueType = true;
			else
				typeInfo->MValueType = false;

			::MonoReflectionType* type = MonoUtil::GetType(curClass->GetInternalClassInternal());

			// Is this a wrapper for some reflectable type?
			ReflectableTypeInfo* reflTypeInfo = GetReflectableTypeInfo(type);
			if(reflTypeInfo != nullptr)
				typeInfo->MRtiiTypeId = reflTypeInfo->TypeId;
			else
				typeInfo->MRtiiTypeId = 0;

			SPtr<ManagedSerializableObjectInfo> objInfo = bs_shared_ptr_new<ManagedSerializableObjectInfo>();

			objInfo->MTypeInfo = typeInfo;
			objInfo->MMonoClass = curClass;

			assemblyInfo->MTypeNameToId[objInfo->GetFullTypeName()] = typeInfo->MTypeId;
			assemblyInfo->MObjectInfos[typeInfo->MTypeId] = objInfo;
		}
	}

	// Populate field & property data
	for(auto& curClassInfo : assemblyInfo->MObjectInfos)
	{
		SPtr<ManagedSerializableObjectInfo> objInfo = curClassInfo.second;

		u32 mUniqueFieldId = 1;

		const Vector<MonoField*>& fields = objInfo->MMonoClass->GetAllFields();
		for(auto& field : fields)
		{
			if(field->IsStatic())
				continue;

			SPtr<ManagedSerializableTypeInfo> typeInfo = GetTypeInfo(field->GetType());
			if(typeInfo == nullptr)
				continue;

			bool typeIsSerializable = true;
			bool typeIsInspectable = true;

			if(const auto* objTypeInfo = rtti_cast<ManagedSerializableTypeInfoObject>(typeInfo.get()))
			{
				typeIsSerializable = objTypeInfo->MFlags.IsSet(ScriptTypeFlag::Serializable);
				typeIsInspectable = typeIsSerializable || objTypeInfo->MFlags.IsSet(ScriptTypeFlag::Inspectable);
			}

			SPtr<ManagedSerializableFieldInfo> fieldInfo = bs_shared_ptr_new<ManagedSerializableFieldInfo>();
			fieldInfo->MFieldId = mUniqueFieldId++;
			fieldInfo->MName = field->GetName();
			fieldInfo->MMonoField = field;
			fieldInfo->MTypeInfo = typeInfo;
			fieldInfo->MParentTypeId = objInfo->MTypeInfo->MTypeId;

			MonoMemberVisibility visibility = field->GetVisibility();
			if(visibility == MonoMemberVisibility::Public)
			{
				if(typeIsSerializable && !field->HasAttribute(mBuiltin.DontSerializeFieldAttribute))
					fieldInfo->MFlags |= ScriptFieldFlag::Serializable;

				if(typeIsInspectable && !field->HasAttribute(mBuiltin.HideInInspectorAttribute))
					fieldInfo->MFlags |= ScriptFieldFlag::Inspectable;

				fieldInfo->MFlags |= ScriptFieldFlag::Animable;
			}
			else
			{
				if(typeIsSerializable && field->HasAttribute(mBuiltin.SerializeFieldAttribute))
					fieldInfo->MFlags |= ScriptFieldFlag::Serializable;

				if(typeIsInspectable && field->HasAttribute(mBuiltin.ShowInInspectorAttribute))
					fieldInfo->MFlags |= ScriptFieldFlag::Inspectable;
			}

			if(field->HasAttribute(mBuiltin.RangeAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::Range;

			if(field->HasAttribute(mBuiltin.StepAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::Step;

			if(field->HasAttribute(mBuiltin.LayerMaskAttribute))
			{
				// Layout mask attribute is only relevant for 64-bit integer types
				if(const auto* primTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
				{
					if(primTypeInfo->MType == ScriptPrimitiveType::I64 ||
					   primTypeInfo->MType == ScriptPrimitiveType::U64)
					{
						fieldInfo->MFlags |= ScriptFieldFlag::AsLayerMask;
					}
				}
			}

			if(field->HasAttribute(mBuiltin.AsQuaternionAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::AsQuaternion;

			if(field->HasAttribute(mBuiltin.NotNullAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::NotNull;

			if(field->HasAttribute(mBuiltin.CategoryAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::Category;

			if(field->HasAttribute(mBuiltin.OrderAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::Order;

			if(field->HasAttribute(mBuiltin.InlineAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::Inline;

			if(field->HasAttribute(mBuiltin.LoadOnAssignAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::LoadOnAssign;

			if(field->HasAttribute(mBuiltin.HdrAttribute))
				fieldInfo->MFlags |= ScriptFieldFlag::HDR;

			objInfo->MFieldNameToId[fieldInfo->MName] = fieldInfo->MFieldId;
			objInfo->MFields[fieldInfo->MFieldId] = fieldInfo;
		}

		const Vector<MonoProperty*>& properties = objInfo->MMonoClass->GetAllProperties();
		for(auto& property : properties)
		{
			SPtr<ManagedSerializableTypeInfo> typeInfo = GetTypeInfo(property->GetReturnType());
			if(typeInfo == nullptr)
				continue;

			bool typeIsSerializable = true;
			bool typeIsInspectable = true;

			if(const auto* objTypeInfo = rtti_cast<ManagedSerializableTypeInfoObject>(typeInfo.get()))
			{
				typeIsSerializable = objTypeInfo->MFlags.IsSet(ScriptTypeFlag::Serializable);
				typeIsInspectable = typeIsSerializable || objTypeInfo->MFlags.IsSet(ScriptTypeFlag::Inspectable);
			}

			SPtr<ManagedSerializablePropertyInfo> propertyInfo = bs_shared_ptr_new<ManagedSerializablePropertyInfo>();
			propertyInfo->MFieldId = mUniqueFieldId++;
			propertyInfo->MName = property->GetName();
			propertyInfo->MMonoProperty = property;
			propertyInfo->MTypeInfo = typeInfo;
			propertyInfo->MParentTypeId = objInfo->MTypeInfo->MTypeId;

			if(!property->IsIndexed())
			{
				MonoMemberVisibility visibility = property->GetVisibility();
				if(visibility == MonoMemberVisibility::Public)
					propertyInfo->MFlags |= ScriptFieldFlag::Animable;

				if(typeIsSerializable && property->HasAttribute(mBuiltin.SerializeFieldAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::Serializable;

				if(typeIsInspectable && property->HasAttribute(mBuiltin.ShowInInspectorAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::Inspectable;

				if(property->HasAttribute(mBuiltin.RangeAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::Range;

				if(property->HasAttribute(mBuiltin.StepAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::Step;

				if(property->HasAttribute(mBuiltin.LayerMaskAttribute))
				{
					// Layout mask attribute is only relevant for 64-bit integer types
					if(const auto* primTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
					{
						if(primTypeInfo->MType == ScriptPrimitiveType::I64 ||
						   primTypeInfo->MType == ScriptPrimitiveType::U64)
						{
							propertyInfo->MFlags |= ScriptFieldFlag::AsLayerMask;
						}
					}
				}

				if(property->HasAttribute(mBuiltin.AsQuaternionAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::AsQuaternion;

				if(property->HasAttribute(mBuiltin.NotNullAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::NotNull;

				if(property->HasAttribute(mBuiltin.PassByCopyAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::PassByCopy;

				if(property->HasAttribute(mBuiltin.ApplyOnDirtyAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::ApplyOnDirty;

				if(property->HasAttribute(mBuiltin.NativeWrapperAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::NativeWrapper;

				if(property->HasAttribute(mBuiltin.CategoryAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::Category;

				if(property->HasAttribute(mBuiltin.OrderAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::Order;

				if(property->HasAttribute(mBuiltin.InlineAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::Inline;

				if(property->HasAttribute(mBuiltin.LoadOnAssignAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::LoadOnAssign;

				if(property->HasAttribute(mBuiltin.HdrAttribute))
					propertyInfo->MFlags |= ScriptFieldFlag::HDR;
			}

			objInfo->MFieldNameToId[propertyInfo->MName] = propertyInfo->MFieldId;
			objInfo->MFields[propertyInfo->MFieldId] = propertyInfo;
		}
	}

	// Form parent/child connections
	for(auto& curClass : assemblyInfo->MObjectInfos)
	{
		MonoClass* base = curClass.second->MMonoClass->GetBaseClass();
		while(base != nullptr)
		{
			SPtr<ManagedSerializableObjectInfo> baseObjInfo;
			if(GetSerializableObjectInfo(base->GetNamespace(), base->GetTypeName(), baseObjInfo))
			{
				curClass.second->MBaseClass = baseObjInfo;
				baseObjInfo->MDerivedClasses.push_back(curClass.second);

				break;
			}

			base = base->GetBaseClass();
		}
	}
}

void ScriptAssemblyManager::ClearAssemblyInfo()
{
	ClearScriptObjects();
	mAssemblyInfos.clear();

	mBuiltinComponentInfos.clear();
	mBuiltinComponentInfosByTID.clear();

	mBuiltinResourceInfos.clear();
	mBuiltinResourceInfosByTID.clear();
	mBuiltinResourceInfosByType.clear();

	mReflectableTypeInfos.clear();
	mReflectableTypeInfosByTID.clear();
}

SPtr<ManagedSerializableTypeInfo> ScriptAssemblyManager::GetTypeInfo(MonoClass* monoClass)
{
	if(!mBaseTypesInitialized)
		BS_EXCEPT(InvalidStateException, "Calling getTypeInfo without previously initializing base types.");

	MonoPrimitiveType monoPrimitiveType = MonoUtil::GetPrimitiveType(monoClass->GetInternalClassInternal());

	// If enum get the enum base data type
	bool isEnum = MonoUtil::IsEnum(monoClass->GetInternalClassInternal());
	if(isEnum)
		monoPrimitiveType = MonoUtil::GetEnumPrimitiveType(monoClass->GetInternalClassInternal());

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
			typeInfo->MType = scriptPrimitiveType;
			return typeInfo;
		}
		else
		{
			SPtr<ManagedSerializableTypeInfoEnum> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoEnum>();
			typeInfo->MUnderlyingType = scriptPrimitiveType;
			typeInfo->MTypeNamespace = monoClass->GetNamespace();
			typeInfo->MTypeName = monoClass->GetTypeName();
			return typeInfo;
		}
	}

	//// Check complex types
	switch(monoPrimitiveType)
	{
	case MonoPrimitiveType::Class:
		if(monoClass->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass)) // Resource
		{
			SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
			typeInfo->MTypeNamespace = monoClass->GetNamespace();
			typeInfo->MTypeName = monoClass->GetTypeName();
			typeInfo->MRtiiTypeId = 0;

			if(monoClass == ScriptResource::GetMetaData()->ScriptClass)
				typeInfo->MType = ScriptReferenceType::BuiltinResourceBase;
			else if(monoClass == ScriptManagedResource::GetMetaData()->ScriptClass)
				typeInfo->MType = ScriptReferenceType::ManagedResourceBase;
			else if(monoClass->IsSubClassOf(ScriptManagedResource::GetMetaData()->ScriptClass))
				typeInfo->MType = ScriptReferenceType::ManagedResource;
			else if(monoClass->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass))
			{
				typeInfo->MType = ScriptReferenceType::BuiltinResource;

				::MonoReflectionType* type = MonoUtil::GetType(monoClass->GetInternalClassInternal());
				BuiltinResourceInfo* builtinInfo = GetBuiltinResourceInfo(type);
				if(builtinInfo == nullptr)
				{
					assert(false && "Unable to find information about a built-in resource. Did you update BuiltinResourceTypes list?");
					return nullptr;
				}

				typeInfo->MRtiiTypeId = builtinInfo->TypeId;
			}

			return typeInfo;
		}
		else if(monoClass == ScriptRRefBase::GetMetaData()->ScriptClass) // Resource reference
			return bs_shared_ptr_new<ManagedSerializableTypeInfoRRef>();
		else if(monoClass->IsSubClassOf(mBuiltin.SceneObjectClass) || monoClass->IsSubClassOf(mBuiltin.ComponentClass)) // Game object
		{
			SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
			typeInfo->MTypeNamespace = monoClass->GetNamespace();
			typeInfo->MTypeName = monoClass->GetTypeName();
			typeInfo->MRtiiTypeId = 0;

			if(monoClass == mBuiltin.ComponentClass)
				typeInfo->MType = ScriptReferenceType::BuiltinComponentBase;
			else if(monoClass == mBuiltin.ManagedComponentClass)
				typeInfo->MType = ScriptReferenceType::ManagedComponentBase;
			else if(monoClass->IsSubClassOf(mBuiltin.SceneObjectClass))
				typeInfo->MType = ScriptReferenceType::SceneObject;
			else if(monoClass->IsSubClassOf(mBuiltin.ManagedComponentClass))
				typeInfo->MType = ScriptReferenceType::ManagedComponent;
			else if(monoClass->IsSubClassOf(mBuiltin.ComponentClass))
			{
				typeInfo->MType = ScriptReferenceType::BuiltinComponent;

				::MonoReflectionType* type = MonoUtil::GetType(monoClass->GetInternalClassInternal());
				BuiltinComponentInfo* builtinInfo = GetBuiltinComponentInfo(type);
				if(builtinInfo == nullptr)
				{
					assert(false && "Unable to find information about a built-in component. Did you update BuiltinComponents list?");
					return nullptr;
				}

				typeInfo->MRtiiTypeId = builtinInfo->TypeId;
			}

			return typeInfo;
		}
		else
		{
			::MonoReflectionType* type = MonoUtil::GetType(monoClass->GetInternalClassInternal());

			// Is this a wrapper for some reflectable type?
			ReflectableTypeInfo* reflTypeInfo = GetReflectableTypeInfo(type);
			if(reflTypeInfo != nullptr)
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				typeInfo->MTypeNamespace = monoClass->GetNamespace();
				typeInfo->MTypeName = monoClass->GetTypeName();
				typeInfo->MRtiiTypeId = reflTypeInfo->TypeId;
				typeInfo->MType = ScriptReferenceType::ReflectableObject;

				return typeInfo;
			}
			else
			{
				// Finally, it's either a normal managed object, or a non-reflectable type wrapper
				SPtr<ManagedSerializableObjectInfo> objInfo;
				if(GetSerializableObjectInfo(monoClass->GetNamespace(), monoClass->GetTypeName(), objInfo))
					return objInfo->MTypeInfo;
			}
		}

		break;
	case MonoPrimitiveType::ValueType:
		{
			SPtr<ManagedSerializableObjectInfo> objInfo;
			if(GetSerializableObjectInfo(monoClass->GetNamespace(), monoClass->GetTypeName(), objInfo))
				return objInfo->MTypeInfo;
		}

		break;
	case MonoPrimitiveType::Generic:
		if(monoClass->GetFullName() == mBuiltin.SystemGenericListClass->GetFullName()) // Full name is part of CIL spec, so it is just fine to compare like this
		{
			SPtr<ManagedSerializableTypeInfoList> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoList>();

			MonoProperty* itemProperty = monoClass->GetProperty("Item");
			MonoClass* itemClass = itemProperty->GetReturnType();

			if(itemClass != nullptr)
				typeInfo->MElementType = GetTypeInfo(itemClass);

			if(typeInfo->MElementType == nullptr)
				return nullptr;

			return typeInfo;
		}
		else if(monoClass->GetFullName() == mBuiltin.SystemGenericDictionaryClass->GetFullName())
		{
			SPtr<ManagedSerializableTypeInfoDictionary> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoDictionary>();

			MonoMethod* getEnumerator = monoClass->GetMethod("GetEnumerator");
			MonoClass* enumClass = getEnumerator->GetReturnType();

			MonoProperty* currentProp = enumClass->GetProperty("Current");
			MonoClass* keyValuePair = currentProp->GetReturnType();

			MonoProperty* keyProperty = keyValuePair->GetProperty("Key");
			MonoProperty* valueProperty = keyValuePair->GetProperty("Value");

			MonoClass* keyClass = keyProperty->GetReturnType();
			if(keyClass != nullptr)
				typeInfo->MKeyType = GetTypeInfo(keyClass);

			MonoClass* valueClass = valueProperty->GetReturnType();
			if(valueClass != nullptr)
				typeInfo->MValueType = GetTypeInfo(valueClass);

			if(typeInfo->MKeyType == nullptr || typeInfo->MValueType == nullptr)
				return nullptr;

			return typeInfo;
		}
		else if(monoClass->GetFullName() == mBuiltin.GenericRRefClass->GetFullName())
		{
			SPtr<ManagedSerializableTypeInfoRRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRRef>();

			MonoProperty* itemProperty = monoClass->GetProperty("Value");
			MonoClass* itemClass = itemProperty->GetReturnType();

			if(itemClass != nullptr)
				typeInfo->MResourceType = GetTypeInfo(itemClass);

			if(typeInfo->MResourceType == nullptr)
				return nullptr;

			return typeInfo;
		}
		break;
	case MonoPrimitiveType::Array:
		{
			SPtr<ManagedSerializableTypeInfoArray> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoArray>();

			::MonoClass* elementClass = ScriptArray::GetElementClass(monoClass->GetInternalClassInternal());
			if(elementClass != nullptr)
			{
				MonoClass* monoElementClass = MonoManager::Instance().FindClass(elementClass);
				if(monoElementClass != nullptr)
					typeInfo->MElementType = GetTypeInfo(monoElementClass);
			}

			if(typeInfo->MElementType == nullptr)
				return nullptr;

			typeInfo->MRank = ScriptArray::GetRank(monoClass->GetInternalClassInternal());

			return typeInfo;
		}
	default:
		break;
	}

	return nullptr;
}

void ScriptAssemblyManager::ClearScriptObjects()
{
	mBaseTypesInitialized = false;
	mBuiltin = BuiltinScriptClasses();
}

void ScriptAssemblyManager::InitializeBaseTypes()
{
	// Get necessary classes for detecting needed class & field information
	MonoAssembly* corlib = MonoManager::Instance().GetAssembly("corlib");
	if(corlib == nullptr)
		BS_EXCEPT(InvalidStateException, "corlib assembly is not loaded.");

	MonoAssembly* engineAssembly = MonoManager::Instance().GetAssembly(kEngineAssembly);
	if(engineAssembly == nullptr)
		BS_EXCEPT(InvalidStateException, String(kEngineAssembly) + " assembly is not loaded.");

	mBuiltin.SystemArrayClass = corlib->GetClass("System", "Array");
	if(mBuiltin.SystemArrayClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find System.Array managed class.");

	mBuiltin.SystemGenericListClass = corlib->GetClass("System.Collections.Generic", "List`1");
	if(mBuiltin.SystemGenericListClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find List<T> managed class.");

	mBuiltin.SystemGenericDictionaryClass = corlib->GetClass("System.Collections.Generic", "Dictionary`2");
	if(mBuiltin.SystemGenericDictionaryClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Dictionary<TKey, TValue> managed class.");

	mBuiltin.SystemTypeClass = corlib->GetClass("System", "Type");
	if(mBuiltin.SystemTypeClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Type managed class.");

	mBuiltin.SerializeObjectAttribute = engineAssembly->GetClass(kEngineNs, "SerializeObject");
	if(mBuiltin.SerializeObjectAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find SerializableObject managed class.");

	mBuiltin.DontSerializeFieldAttribute = engineAssembly->GetClass(kEngineNs, "DontSerializeField");
	if(mBuiltin.DontSerializeFieldAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find DontSerializeField managed class.");

	mBuiltin.RangeAttribute = engineAssembly->GetClass(kEngineNs, "Range");
	if(mBuiltin.RangeAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Range managed class.");

	mBuiltin.StepAttribute = engineAssembly->GetClass(kEngineNs, "Step");
	if(mBuiltin.StepAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Step managed class.");

	mBuiltin.LayerMaskAttribute = engineAssembly->GetClass(kEngineNs, "LayerMask");
	if(mBuiltin.LayerMaskAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find LayerMask managed class.");

	mBuiltin.AsQuaternionAttribute = engineAssembly->GetClass(kEngineNs, "AsQuaternion");
	if(mBuiltin.AsQuaternionAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find AsQuaternion managed class.");

	mBuiltin.NativeWrapperAttribute = engineAssembly->GetClass(kEngineNs, "NativeWrapper");
	if(mBuiltin.NativeWrapperAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find NativeWrapper managed class.");

	mBuiltin.NotNullAttribute = engineAssembly->GetClass(kEngineNs, "NotNull");
	if(mBuiltin.NotNullAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find NotNull managed class.");

	mBuiltin.PassByCopyAttribute = engineAssembly->GetClass(kEngineNs, "PassByCopy");
	if(mBuiltin.PassByCopyAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find PassByCopy managed class.");

	mBuiltin.ApplyOnDirtyAttribute = engineAssembly->GetClass(kEngineNs, "ApplyOnDirty");
	if(mBuiltin.ApplyOnDirtyAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find ApplyOnDirty managed class.");

	mBuiltin.ComponentClass = engineAssembly->GetClass(kEngineNs, "Component");
	if(mBuiltin.ComponentClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Component managed class.");

	mBuiltin.ManagedComponentClass = engineAssembly->GetClass(kEngineNs, "ManagedComponent");
	if(mBuiltin.ManagedComponentClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find ManagedComponent managed class.");

	mBuiltin.MissingComponentClass = engineAssembly->GetClass(kEngineNs, "MissingComponent");
	if(mBuiltin.MissingComponentClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find MissingComponent managed class.");

	mBuiltin.SceneObjectClass = engineAssembly->GetClass(kEngineNs, "SceneObject");
	if(mBuiltin.SceneObjectClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find SceneObject managed class.");

	mBuiltin.RrefBaseClass = engineAssembly->GetClass(kEngineNs, "RRefBase");
	if(mBuiltin.RrefBaseClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find RRefBase managed class.");

	mBuiltin.GenericRRefClass = engineAssembly->GetClass(kEngineNs, "RRef`1");
	if(mBuiltin.GenericRRefClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find RRef<T> managed class.");

	mBuiltin.GenericAsyncOpClass = engineAssembly->GetClass(kEngineNs, "AsyncOp`1");
	if(mBuiltin.GenericAsyncOpClass == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find AsyncOp<T> managed class.");

	mBuiltin.SerializeFieldAttribute = engineAssembly->GetClass(kEngineNs, "SerializeField");
	if(mBuiltin.SerializeFieldAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find SerializeField managed class.");

	mBuiltin.HideInInspectorAttribute = engineAssembly->GetClass(kEngineNs, "HideInInspector");
	if(mBuiltin.HideInInspectorAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find HideInInspector managed class.");

	mBuiltin.ShowInInspectorAttribute = engineAssembly->GetClass(kEngineNs, "ShowInInspector");
	if(mBuiltin.ShowInInspectorAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find ShowInInspector managed class.");

	mBuiltin.CategoryAttribute = engineAssembly->GetClass(kEngineNs, "Category");
	if(mBuiltin.CategoryAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Category managed class.");

	mBuiltin.OrderAttribute = engineAssembly->GetClass(kEngineNs, "Order");
	if(mBuiltin.OrderAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Order managed class.");

	mBuiltin.InlineAttribute = engineAssembly->GetClass(kEngineNs, "Inline");
	if(mBuiltin.InlineAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find Inline managed class.");

	mBuiltin.LoadOnAssignAttribute = engineAssembly->GetClass(kEngineNs, "LoadOnAssign");
	if(mBuiltin.LoadOnAssignAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find LoadOnAssign managed class.");

	mBuiltin.HdrAttribute = engineAssembly->GetClass(kEngineNs, "HDR");
	if(mBuiltin.HdrAttribute == nullptr)
		BS_EXCEPT(InvalidStateException, "Cannot find HDR managed class.");

	mBaseTypesInitialized = true;
}

void ScriptAssemblyManager::LoadTypeMappings(MonoAssembly& assembly, const BuiltinTypeMappings& mapping)
{
	for(auto& entry : mapping.Components)
	{
		BuiltinComponentInfo info = entry;
		info.MonoClass = assembly.GetClass(entry.MetaData->Ns, entry.MetaData->Name);

		::MonoReflectionType* type = MonoUtil::GetType(info.MonoClass->GetInternalClassInternal());

		mBuiltinComponentInfos[type] = info;
		mBuiltinComponentInfosByTID[info.TypeId] = info;
	}

	for(auto& entry : mapping.Resources)
	{
		BuiltinResourceInfo info = entry;
		info.MonoClass = assembly.GetClass(entry.MetaData->Ns, entry.MetaData->Name);

		::MonoReflectionType* type = MonoUtil::GetType(info.MonoClass->GetInternalClassInternal());

		mBuiltinResourceInfos[type] = info;
		mBuiltinResourceInfosByTID[info.TypeId] = info;
		mBuiltinResourceInfosByType[(u32)info.ResType] = info;
	}

	for(auto& entry : mapping.ReflectableObjects)
	{
		ReflectableTypeInfo info = entry;
		info.MonoClass = assembly.GetClass(entry.MetaData->Ns, entry.MetaData->Name);

		::MonoReflectionType* type = MonoUtil::GetType(info.MonoClass->GetInternalClassInternal());

		mReflectableTypeInfos[type] = info;
		mReflectableTypeInfosByTID[info.TypeId] = info;
	}
}

BuiltinComponentInfo* ScriptAssemblyManager::GetBuiltinComponentInfo(::MonoReflectionType* type)
{
	auto iterFind = mBuiltinComponentInfos.find(type);
	if(iterFind == mBuiltinComponentInfos.end())
		return nullptr;

	return &(iterFind->second);
}

BuiltinComponentInfo* ScriptAssemblyManager::GetBuiltinComponentInfo(u32 rttiTypeId)
{
	auto iterFind = mBuiltinComponentInfosByTID.find(rttiTypeId);
	if(iterFind == mBuiltinComponentInfosByTID.end())
		return nullptr;

	return &(iterFind->second);
}

BuiltinResourceInfo* ScriptAssemblyManager::GetBuiltinResourceInfo(::MonoReflectionType* type)
{
	auto iterFind = mBuiltinResourceInfos.find(type);
	if(iterFind == mBuiltinResourceInfos.end())
		return nullptr;

	return &(iterFind->second);
}

BuiltinResourceInfo* ScriptAssemblyManager::GetBuiltinResourceInfo(u32 rttiTypeId)
{
	auto iterFind = mBuiltinResourceInfosByTID.find(rttiTypeId);
	if(iterFind == mBuiltinResourceInfosByTID.end())
		return nullptr;

	return &(iterFind->second);
}

BuiltinResourceInfo* ScriptAssemblyManager::GetBuiltinResourceInfo(ScriptResourceType type)
{
	auto iterFind = mBuiltinResourceInfosByType.find((u32)type);
	if(iterFind == mBuiltinResourceInfosByType.end())
		return nullptr;

	return &(iterFind->second);
}

ReflectableTypeInfo* ScriptAssemblyManager::GetReflectableTypeInfo(::MonoReflectionType* type)
{
	auto iterFind = mReflectableTypeInfos.find(type);
	if(iterFind == mReflectableTypeInfos.end())
		return nullptr;

	return &(iterFind->second);
}

ReflectableTypeInfo* ScriptAssemblyManager::GetReflectableTypeInfo(uint32_t rttiTypeId)
{
	auto iterFind = mReflectableTypeInfosByTID.find(rttiTypeId);
	if(iterFind == mReflectableTypeInfosByTID.end())
		return nullptr;

	return &(iterFind->second);
}

bool ScriptAssemblyManager::GetSerializableObjectInfo(const String& ns, const String& typeName, SPtr<ManagedSerializableObjectInfo>& outInfo)
{
	String fullName = ns + "." + typeName;
	for(auto& curAssembly : mAssemblyInfos)
	{
		if(curAssembly.second == nullptr)
			continue;

		auto iterFind = curAssembly.second->MTypeNameToId.find(fullName);
		if(iterFind != curAssembly.second->MTypeNameToId.end())
		{
			outInfo = curAssembly.second->MObjectInfos[iterFind->second];

			return true;
		}
	}

	return false;
}

bool ScriptAssemblyManager::HasSerializableObjectInfo(const String& ns, const String& typeName)
{
	String fullName = ns + "." + typeName;
	for(auto& curAssembly : mAssemblyInfos)
	{
		auto iterFind = curAssembly.second->MTypeNameToId.find(fullName);
		if(iterFind != curAssembly.second->MTypeNameToId.end())
			return true;
	}

	return false;
}

SPtr<IReflectable> ScriptAssemblyManager::GetReflectableFromManagedObject(MonoObject* value)
{
	if(value != nullptr)
	{
		::MonoClass* klass = MonoUtil::GetClass(value);
		MonoClass* monoClass = MonoManager::Instance().FindClass(klass);

		if(MonoUtil::IsEnum(klass))
		{
			BS_LOG(Warning, Script, "Unsupported type provided.");
			return nullptr;
		}

		MonoPrimitiveType monoPrimitiveType = MonoUtil::GetPrimitiveType(klass);
		if(monoPrimitiveType != MonoPrimitiveType::Class && monoPrimitiveType != MonoPrimitiveType::ValueType)
		{
			BS_LOG(Warning, Script, "Unsupported type provided.");
			return nullptr;
		}

		const ScriptMeta* managedResourceMeta = ScriptManagedResource::GetMetaData();
		const ScriptMeta* managedComponentMeta = ScriptManagedComponent::GetMetaData();

		if(monoClass->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass)) // Resource
		{
			if(monoClass == ScriptResource::GetMetaData()->ScriptClass ||
			   monoClass == ScriptManagedResource::GetMetaData()->ScriptClass)
			{
				BS_LOG(Warning, Script, "Unsupported type provided.");
				return nullptr;
			}

			if(monoClass->IsSubClassOf(managedResourceMeta->ScriptClass))
			{
				ScriptManagedResource* scriptResource = nullptr;
				managedResourceMeta->ThisPtrField->Get(value, &scriptResource);

				HManagedResource resource = scriptResource->GetHandle();
				if(!resource.IsLoaded(false))
					return nullptr;

				MonoObject* managedInstance = resource->GetManagedInstance();
				SPtr<ManagedSerializableObject> serializedObject = ManagedSerializableObject::CreateFromExisting(managedInstance);
				if(serializedObject == nullptr)
					return nullptr;

				serializedObject->Serialize();
				return serializedObject;
			}
			else
			{
				::MonoReflectionType* type = MonoUtil::GetType(klass);
				BuiltinResourceInfo* builtinInfo = GetBuiltinResourceInfo(type);
				if(builtinInfo == nullptr)
				{
					assert(false && "Unable to find information about a built-in resource. Did you update BuiltinResourceTypes list?");
					return nullptr;
				}

				ScriptResourceBase* scriptResource = nullptr;
				builtinInfo->MetaData->ThisPtrField->Get(value, &scriptResource);

				HResource handle = scriptResource->GetGenericHandle();
				if(!handle.IsLoaded(false))
					return nullptr;

				return handle.GetInternalPtr();
			}
		}
		else if(monoClass->IsSubClassOf(mBuiltin.ComponentClass)) // Component
		{
			if(monoClass == mBuiltin.ComponentClass || monoClass == mBuiltin.ManagedComponentClass)
			{
				BS_LOG(Warning, Script, "Unsupported type provided.");
				return nullptr;
			}

			if(monoClass->IsSubClassOf(mBuiltin.ManagedComponentClass))
			{
				ScriptManagedComponent* scriptComponent = nullptr;
				managedComponentMeta->ThisPtrField->Get(value, &scriptComponent);

				HManagedComponent component = scriptComponent->GetHandle();
				if(component.IsDestroyed())
					return nullptr;

				MonoObject* managedInstance = component->GetManagedInstance();
				SPtr<ManagedSerializableObject> serializedObject = ManagedSerializableObject::CreateFromExisting(managedInstance);
				if(serializedObject == nullptr)
					return nullptr;

				serializedObject->Serialize();
				return serializedObject;
			}
			else
			{
				::MonoReflectionType* type = MonoUtil::GetType(klass);
				BuiltinComponentInfo* builtinInfo = GetBuiltinComponentInfo(type);
				if(builtinInfo == nullptr)
				{
					assert(false && "Unable to find information about a built-in component. Did you update BuiltinComponents list?");
					return nullptr;
				}

				ScriptComponentBase* scriptComponent = nullptr;
				builtinInfo->MetaData->ThisPtrField->Get(value, &scriptComponent);

				HComponent handle = scriptComponent->GetComponent();
				if(handle.IsDestroyed())
					return nullptr;

				return handle.GetInternalPtr();
			}
		}

		// Generic class or value type
		String elementNs;
		String elementTypeName;
		MonoUtil::GetClassName(value, elementNs, elementTypeName);

		SPtr<ManagedSerializableObjectInfo> objInfo;
		if(!Instance().GetSerializableObjectInfo(elementNs, elementTypeName, objInfo))
		{
			BS_LOG(Error, Script, "Object has no serialization meta-data.");
			return nullptr;
		}

		if(objInfo->MTypeInfo->MRtiiTypeId != 0)
		{
			::MonoClass* monoClass = MonoUtil::GetClass(value);
			::MonoReflectionType* monoType = MonoUtil::GetType(monoClass);

			const ReflectableTypeInfo* reflTypeInfo = Instance().GetReflectableTypeInfo(monoType);
			assert(reflTypeInfo);

			ScriptReflectableBase* scriptReflectable = nullptr;

			if(reflTypeInfo->MetaData->ThisPtrField != nullptr)
				reflTypeInfo->MetaData->ThisPtrField->Get(value, &scriptReflectable);

			return scriptReflectable->GetReflectable();
		}
		else
		{
			SPtr<ManagedSerializableObject> managedObj = ManagedSerializableObject::CreateFromExisting(value);
			if(!managedObj)
			{
				BS_LOG(Error, Script, "Object failed to serialize due to an internal error.");
				return nullptr;
			}

			managedObj->Serialize();
			return managedObj;
		}
	}

	return nullptr;
}
