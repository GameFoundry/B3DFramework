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
#include "BsScriptReflectableWrapper.h"

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
	InitializeScriptWrapperMetaDataLookup(*curAssembly);

	SPtr<ManagedSerializableAssemblyInfo> assemblyInfo = B3DMakeShared<ManagedSerializableAssemblyInfo>();
	assemblyInfo->Name = assemblyName;

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
			SPtr<ManagedSerializableTypeInfoObject> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoObject>();
			typeInfo->TypeNamespace = curClass->GetNamespace();
			typeInfo->TypeName = curClass->GetTypeName();
			typeInfo->TypeId = mUniqueTypeId++;

			if(isSerializable)
				typeInfo->Flags |= ScriptTypeFlag::Serializable;

			if(isSerializable || isInspectable)
				typeInfo->Flags |= ScriptTypeFlag::Inspectable;

			MonoPrimitiveType monoPrimitiveType = MonoUtil::GetPrimitiveType(curClass->GetInternalClass());

			if(monoPrimitiveType == MonoPrimitiveType::ValueType)
				typeInfo->ValueType = true;
			else
				typeInfo->ValueType = false;

			::MonoReflectionType* type = MonoUtil::GetType(curClass->GetInternalClass());

			// Is this a wrapper for some reflectable type?
			const ScriptWrapperObjectMetaData* const scriptWrapperObjectMetaData = GetScriptWrapperMetaData(type);
			if(scriptWrapperObjectMetaData != nullptr)
				typeInfo->RttiTypeId = scriptWrapperObjectMetaData->TypeId;
			else
				typeInfo->RttiTypeId = 0;

			SPtr<ManagedSerializableObjectInfo> objInfo = B3DMakeShared<ManagedSerializableObjectInfo>();

			objInfo->TypeInfo = typeInfo;
			objInfo->ScriptClass = curClass;

			assemblyInfo->TypeNameToId[objInfo->GetFullTypeName()] = typeInfo->TypeId;
			assemblyInfo->ObjectInfos[typeInfo->TypeId] = objInfo;
		}
	}

	// Populate field & property data
	for(auto& curClassInfo : assemblyInfo->ObjectInfos)
	{
		SPtr<ManagedSerializableObjectInfo> objInfo = curClassInfo.second;

		u32 mUniqueFieldId = 1;

		const Vector<MonoField*>& fields = objInfo->ScriptClass->GetAllFields();
		for(auto& field : fields)
		{
			if(field->IsStatic())
				continue;

			SPtr<ManagedSerializableTypeInfo> typeInfo = GetTypeInfo(field->GetType());
			if(typeInfo == nullptr)
				continue;

			bool typeIsSerializable = true;
			bool typeIsInspectable = true;

			if(const auto* objTypeInfo = B3DRTTICast<ManagedSerializableTypeInfoObject>(typeInfo.get()))
			{
				typeIsSerializable = objTypeInfo->Flags.IsSet(ScriptTypeFlag::Serializable);
				typeIsInspectable = typeIsSerializable || objTypeInfo->Flags.IsSet(ScriptTypeFlag::Inspectable);
			}

			SPtr<ManagedSerializableFieldInfo> fieldInfo = B3DMakeShared<ManagedSerializableFieldInfo>();
			fieldInfo->FieldId = mUniqueFieldId++;
			fieldInfo->Name = field->GetName();
			fieldInfo->ScriptField = field;
			fieldInfo->TypeInfo = typeInfo;
			fieldInfo->ParentTypeId = objInfo->TypeInfo->TypeId;

			MonoMemberVisibility visibility = field->GetVisibility();
			if(visibility == MonoMemberVisibility::Public)
			{
				if(typeIsSerializable && !field->HasAttribute(mBuiltin.DontSerializeFieldAttribute))
					fieldInfo->Flags |= ScriptFieldFlag::Serializable;

				if(typeIsInspectable && !field->HasAttribute(mBuiltin.HideInInspectorAttribute))
					fieldInfo->Flags |= ScriptFieldFlag::Inspectable;

				fieldInfo->Flags |= ScriptFieldFlag::Animable;
			}
			else
			{
				if(typeIsSerializable && field->HasAttribute(mBuiltin.SerializeFieldAttribute))
					fieldInfo->Flags |= ScriptFieldFlag::Serializable;

				if(typeIsInspectable && field->HasAttribute(mBuiltin.ShowInInspectorAttribute))
					fieldInfo->Flags |= ScriptFieldFlag::Inspectable;
			}

			if(field->HasAttribute(mBuiltin.RangeAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::Range;

			if(field->HasAttribute(mBuiltin.StepAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::Step;

			if(field->HasAttribute(mBuiltin.LayerMaskAttribute))
			{
				// Layout mask attribute is only relevant for 64-bit integer types
				if(const auto* primTypeInfo = B3DRTTICast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
				{
					if(primTypeInfo->Type == ScriptPrimitiveType::I64 ||
					   primTypeInfo->Type == ScriptPrimitiveType::U64)
					{
						fieldInfo->Flags |= ScriptFieldFlag::AsLayerMask;
					}
				}
			}

			if(field->HasAttribute(mBuiltin.AsQuaternionAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::AsQuaternion;

			if(field->HasAttribute(mBuiltin.NotNullAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::NotNull;

			if(field->HasAttribute(mBuiltin.CategoryAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::Category;

			if(field->HasAttribute(mBuiltin.OrderAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::Order;

			if(field->HasAttribute(mBuiltin.InlineAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::Inline;

			if(field->HasAttribute(mBuiltin.LoadOnAssignAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::LoadOnAssign;

			if(field->HasAttribute(mBuiltin.HdrAttribute))
				fieldInfo->Flags |= ScriptFieldFlag::HDR;

			objInfo->FieldNameToId[fieldInfo->Name] = fieldInfo->FieldId;
			objInfo->Fields[fieldInfo->FieldId] = fieldInfo;
		}

		const Vector<MonoProperty*>& properties = objInfo->ScriptClass->GetAllProperties();
		for(auto& property : properties)
		{
			SPtr<ManagedSerializableTypeInfo> typeInfo = GetTypeInfo(property->GetReturnType());
			if(typeInfo == nullptr)
				continue;

			bool typeIsSerializable = true;
			bool typeIsInspectable = true;

			if(const auto* objTypeInfo = B3DRTTICast<ManagedSerializableTypeInfoObject>(typeInfo.get()))
			{
				typeIsSerializable = objTypeInfo->Flags.IsSet(ScriptTypeFlag::Serializable);
				typeIsInspectable = typeIsSerializable || objTypeInfo->Flags.IsSet(ScriptTypeFlag::Inspectable);
			}

			SPtr<ManagedSerializablePropertyInfo> propertyInfo = B3DMakeShared<ManagedSerializablePropertyInfo>();
			propertyInfo->FieldId = mUniqueFieldId++;
			propertyInfo->Name = property->GetName();
			propertyInfo->ScriptProperty = property;
			propertyInfo->TypeInfo = typeInfo;
			propertyInfo->ParentTypeId = objInfo->TypeInfo->TypeId;

			if(!property->IsIndexed())
			{
				MonoMemberVisibility visibility = property->GetVisibility();
				if(visibility == MonoMemberVisibility::Public)
					propertyInfo->Flags |= ScriptFieldFlag::Animable;

				if(typeIsSerializable && property->HasAttribute(mBuiltin.SerializeFieldAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::Serializable;

				if(typeIsInspectable && property->HasAttribute(mBuiltin.ShowInInspectorAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::Inspectable;

				if(property->HasAttribute(mBuiltin.RangeAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::Range;

				if(property->HasAttribute(mBuiltin.StepAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::Step;

				if(property->HasAttribute(mBuiltin.LayerMaskAttribute))
				{
					// Layout mask attribute is only relevant for 64-bit integer types
					if(const auto* primTypeInfo = B3DRTTICast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
					{
						if(primTypeInfo->Type == ScriptPrimitiveType::I64 ||
						   primTypeInfo->Type == ScriptPrimitiveType::U64)
						{
							propertyInfo->Flags |= ScriptFieldFlag::AsLayerMask;
						}
					}
				}

				if(property->HasAttribute(mBuiltin.AsQuaternionAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::AsQuaternion;

				if(property->HasAttribute(mBuiltin.NotNullAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::NotNull;

				if(property->HasAttribute(mBuiltin.PassByCopyAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::PassByCopy;

				if(property->HasAttribute(mBuiltin.ApplyOnDirtyAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::ApplyOnDirty;

				if(property->HasAttribute(mBuiltin.NativeWrapperAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::NativeWrapper;

				if(property->HasAttribute(mBuiltin.CategoryAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::Category;

				if(property->HasAttribute(mBuiltin.OrderAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::Order;

				if(property->HasAttribute(mBuiltin.InlineAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::Inline;

				if(property->HasAttribute(mBuiltin.LoadOnAssignAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::LoadOnAssign;

				if(property->HasAttribute(mBuiltin.HdrAttribute))
					propertyInfo->Flags |= ScriptFieldFlag::HDR;
			}

			objInfo->FieldNameToId[propertyInfo->Name] = propertyInfo->FieldId;
			objInfo->Fields[propertyInfo->FieldId] = propertyInfo;
		}
	}

	// Form parent/child connections
	for(auto& curClass : assemblyInfo->ObjectInfos)
	{
		MonoClass* base = curClass.second->ScriptClass->GetBaseClass();
		while(base != nullptr)
		{
			SPtr<ManagedSerializableObjectInfo> baseObjInfo;
			if(GetSerializableObjectInfo(base->GetNamespace(), base->GetTypeName(), baseObjInfo))
			{
				curClass.second->BaseClass = baseObjInfo;
				baseObjInfo->DerivedClasses.push_back(curClass.second);

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

	mBuiltinResourceInfos.clear();
	mBuiltinResourceInfosByTID.clear();
	mBuiltinResourceInfosByType.clear();

	mScriptWrapperMetaDataByTypeId.clear();
	mScriptWrapperMetaDataByScriptClass.clear();
}

SPtr<ManagedSerializableTypeInfo> ScriptAssemblyManager::GetTypeInfo(MonoClass* monoClass)
{
	if(!mBaseTypesInitialized)
		B3D_EXCEPT(InvalidStateException, "Calling getTypeInfo without previously initializing base types.");

	MonoPrimitiveType monoPrimitiveType = MonoUtil::GetPrimitiveType(monoClass->GetInternalClass());

	// If enum get the enum base data type
	bool isEnum = MonoUtil::IsEnum(monoClass->GetInternalClass());
	if(isEnum)
		monoPrimitiveType = MonoUtil::GetEnumPrimitiveType(monoClass->GetInternalClass());

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
			SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoPrimitive>();
			typeInfo->Type = scriptPrimitiveType;
			return typeInfo;
		}
		else
		{
			SPtr<ManagedSerializableTypeInfoEnum> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoEnum>();
			typeInfo->UnderlyingType = scriptPrimitiveType;
			typeInfo->TypeNamespace = monoClass->GetNamespace();
			typeInfo->TypeName = monoClass->GetTypeName();
			return typeInfo;
		}
	}

	//// Check complex types
	switch(monoPrimitiveType)
	{
	case MonoPrimitiveType::Class:
		if(monoClass->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass)) // Resource
		{
			SPtr<ManagedSerializableTypeInfoRef> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoRef>();
			typeInfo->TypeNamespace = monoClass->GetNamespace();
			typeInfo->TypeName = monoClass->GetTypeName();
			typeInfo->RtiiTypeId = 0;

			if(monoClass == ScriptResource::GetMetaData()->ScriptClass)
				typeInfo->Type = ScriptReferenceType::BuiltinResourceBase;
			else if(monoClass == ScriptManagedResource::GetMetaData()->ScriptClass)
				typeInfo->Type = ScriptReferenceType::ManagedResourceBase;
			else if(monoClass->IsSubClassOf(ScriptManagedResource::GetMetaData()->ScriptClass))
				typeInfo->Type = ScriptReferenceType::ManagedResource;
			else if(monoClass->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass))
			{
				typeInfo->Type = ScriptReferenceType::BuiltinResource;

				::MonoReflectionType* type = MonoUtil::GetType(monoClass->GetInternalClass());
				BuiltinResourceInfo* builtinInfo = GetBuiltinResourceInfo(type);
				if(builtinInfo == nullptr)
				{
					B3D_ASSERT(false && "Unable to find information about a built-in resource. Did you update BuiltinResourceTypes list?");
					return nullptr;
				}

				typeInfo->RtiiTypeId = builtinInfo->TypeId;
			}

			return typeInfo;
		}
		else if(monoClass == ScriptRRefBase::GetMetaData()->ScriptClass) // Resource reference
			return B3DMakeShared<ManagedSerializableTypeInfoRRef>();
		else if(monoClass->IsSubClassOf(mBuiltin.SceneObjectClass) || monoClass->IsSubClassOf(mBuiltin.ComponentClass)) // Game object
		{
			SPtr<ManagedSerializableTypeInfoRef> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoRef>();
			typeInfo->TypeNamespace = monoClass->GetNamespace();
			typeInfo->TypeName = monoClass->GetTypeName();
			typeInfo->RtiiTypeId = 0;

			if(monoClass == mBuiltin.ComponentClass)
				typeInfo->Type = ScriptReferenceType::BuiltinComponentBase;
			else if(monoClass == mBuiltin.ManagedComponentClass)
				typeInfo->Type = ScriptReferenceType::ManagedComponentBase;
			else if(monoClass->IsSubClassOf(mBuiltin.SceneObjectClass))
				typeInfo->Type = ScriptReferenceType::SceneObject;
			else if(monoClass->IsSubClassOf(mBuiltin.ManagedComponentClass))
				typeInfo->Type = ScriptReferenceType::ManagedComponent;
			else if(monoClass->IsSubClassOf(mBuiltin.ComponentClass))
			{
				typeInfo->Type = ScriptReferenceType::BuiltinComponent;

				::MonoReflectionType* type = MonoUtil::GetType(monoClass->GetInternalClass());
				const ScriptWrapperObjectMetaData* const scriptWrapperObjectMetaData = GetScriptWrapperMetaData(type);
				if(scriptWrapperObjectMetaData == nullptr)
				{
					B3D_ASSERT(false && "Unable to find information about a built-in component. Is it script exported?");
					return nullptr;
				}

				typeInfo->RtiiTypeId = scriptWrapperObjectMetaData->TypeId;
			}

			return typeInfo;
		}
		else
		{
			::MonoReflectionType* type = MonoUtil::GetType(monoClass->GetInternalClass());

			// Is this a wrapper for some reflectable type?
			const ScriptWrapperObjectMetaData* const scriptWrapperObjectMetaData = GetScriptWrapperMetaData(type);
			if(scriptWrapperObjectMetaData != nullptr && scriptWrapperObjectMetaData->TypeId != ~0u)
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoRef>();
				typeInfo->TypeNamespace = monoClass->GetNamespace();
				typeInfo->TypeName = monoClass->GetTypeName();
				typeInfo->RtiiTypeId = scriptWrapperObjectMetaData->TypeId;
				typeInfo->Type = ScriptReferenceType::ReflectableObject;

				return typeInfo;
			}
			else
			{
				// Finally, it's either a normal managed object, or a non-reflectable type wrapper
				SPtr<ManagedSerializableObjectInfo> objInfo;
				if(GetSerializableObjectInfo(monoClass->GetNamespace(), monoClass->GetTypeName(), objInfo))
					return objInfo->TypeInfo;
			}
		}

		break;
	case MonoPrimitiveType::ValueType:
		{
			SPtr<ManagedSerializableObjectInfo> objInfo;
			if(GetSerializableObjectInfo(monoClass->GetNamespace(), monoClass->GetTypeName(), objInfo))
				return objInfo->TypeInfo;
		}

		break;
	case MonoPrimitiveType::Generic:
		if(monoClass->GetFullName() == mBuiltin.SystemGenericListClass->GetFullName()) // Full name is part of CIL spec, so it is just fine to compare like this
		{
			SPtr<ManagedSerializableTypeInfoList> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoList>();

			MonoProperty* itemProperty = monoClass->GetProperty("Item");
			MonoClass* itemClass = itemProperty->GetReturnType();

			if(itemClass != nullptr)
				typeInfo->ElementType = GetTypeInfo(itemClass);

			if(typeInfo->ElementType == nullptr)
				return nullptr;

			return typeInfo;
		}
		else if(monoClass->GetFullName() == mBuiltin.SystemGenericDictionaryClass->GetFullName())
		{
			SPtr<ManagedSerializableTypeInfoDictionary> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoDictionary>();

			MonoMethod* getEnumerator = monoClass->GetMethod("GetEnumerator");
			MonoClass* enumClass = getEnumerator->GetReturnType();

			MonoProperty* currentProp = enumClass->GetProperty("Current");
			MonoClass* keyValuePair = currentProp->GetReturnType();

			MonoProperty* keyProperty = keyValuePair->GetProperty("Key");
			MonoProperty* valueProperty = keyValuePair->GetProperty("Value");

			MonoClass* keyClass = keyProperty->GetReturnType();
			if(keyClass != nullptr)
				typeInfo->KeyType = GetTypeInfo(keyClass);

			MonoClass* valueClass = valueProperty->GetReturnType();
			if(valueClass != nullptr)
				typeInfo->ValueType = GetTypeInfo(valueClass);

			if(typeInfo->KeyType == nullptr || typeInfo->ValueType == nullptr)
				return nullptr;

			return typeInfo;
		}
		else if(monoClass->GetFullName() == mBuiltin.GenericRRefClass->GetFullName())
		{
			SPtr<ManagedSerializableTypeInfoRRef> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoRRef>();

			MonoProperty* itemProperty = monoClass->GetProperty("Value");
			MonoClass* itemClass = itemProperty->GetReturnType();

			if(itemClass != nullptr)
				typeInfo->ResourceType = GetTypeInfo(itemClass);

			if(typeInfo->ResourceType == nullptr)
				return nullptr;

			return typeInfo;
		}
		break;
	case MonoPrimitiveType::Array:
		{
			SPtr<ManagedSerializableTypeInfoArray> typeInfo = B3DMakeShared<ManagedSerializableTypeInfoArray>();

			::MonoClass* elementClass = ScriptArray::GetElementClass(monoClass->GetInternalClass());
			if(elementClass != nullptr)
			{
				MonoClass* monoElementClass = MonoManager::Instance().FindClass(elementClass);
				if(monoElementClass != nullptr)
					typeInfo->ElementType = GetTypeInfo(monoElementClass);
			}

			if(typeInfo->ElementType == nullptr)
				return nullptr;

			typeInfo->Rank = ScriptArray::GetRank(monoClass->GetInternalClass());

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
		B3D_EXCEPT(InvalidStateException, "corlib assembly is not loaded.");

	MonoAssembly* engineAssembly = MonoManager::Instance().GetAssembly(kEngineAssembly);
	if(engineAssembly == nullptr)
		B3D_EXCEPT(InvalidStateException, String(kEngineAssembly) + " assembly is not loaded.");

	mBuiltin.SystemArrayClass = corlib->GetClass("System", "Array");
	if(mBuiltin.SystemArrayClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find System.Array managed class.");

	mBuiltin.SystemGenericListClass = corlib->GetClass("System.Collections.Generic", "List`1");
	if(mBuiltin.SystemGenericListClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find List<T> managed class.");

	mBuiltin.SystemGenericDictionaryClass = corlib->GetClass("System.Collections.Generic", "Dictionary`2");
	if(mBuiltin.SystemGenericDictionaryClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Dictionary<TKey, TValue> managed class.");

	mBuiltin.SystemTypeClass = corlib->GetClass("System", "Type");
	if(mBuiltin.SystemTypeClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Type managed class.");

	mBuiltin.SerializeObjectAttribute = engineAssembly->GetClass(kEngineNs, "SerializeObject");
	if(mBuiltin.SerializeObjectAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find SerializableObject managed class.");

	mBuiltin.DontSerializeFieldAttribute = engineAssembly->GetClass(kEngineNs, "DontSerializeField");
	if(mBuiltin.DontSerializeFieldAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find DontSerializeField managed class.");

	mBuiltin.RangeAttribute = engineAssembly->GetClass(kEngineNs, "Range");
	if(mBuiltin.RangeAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Range managed class.");

	mBuiltin.StepAttribute = engineAssembly->GetClass(kEngineNs, "Step");
	if(mBuiltin.StepAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Step managed class.");

	mBuiltin.LayerMaskAttribute = engineAssembly->GetClass(kEngineNs, "LayerMask");
	if(mBuiltin.LayerMaskAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find LayerMask managed class.");

	mBuiltin.AsQuaternionAttribute = engineAssembly->GetClass(kEngineNs, "AsQuaternion");
	if(mBuiltin.AsQuaternionAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find AsQuaternion managed class.");

	mBuiltin.NativeWrapperAttribute = engineAssembly->GetClass(kEngineNs, "NativeWrapper");
	if(mBuiltin.NativeWrapperAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find NativeWrapper managed class.");

	mBuiltin.NotNullAttribute = engineAssembly->GetClass(kEngineNs, "NotNull");
	if(mBuiltin.NotNullAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find NotNull managed class.");

	mBuiltin.PassByCopyAttribute = engineAssembly->GetClass(kEngineNs, "PassByCopy");
	if(mBuiltin.PassByCopyAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find PassByCopy managed class.");

	mBuiltin.ApplyOnDirtyAttribute = engineAssembly->GetClass(kEngineNs, "ApplyOnDirty");
	if(mBuiltin.ApplyOnDirtyAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find ApplyOnDirty managed class.");

	mBuiltin.ComponentClass = engineAssembly->GetClass(kEngineNs, "Component");
	if(mBuiltin.ComponentClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Component managed class.");

	mBuiltin.ManagedComponentClass = engineAssembly->GetClass(kEngineNs, "ManagedComponent");
	if(mBuiltin.ManagedComponentClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find ManagedComponent managed class.");

	mBuiltin.MissingComponentClass = engineAssembly->GetClass(kEngineNs, "MissingComponent");
	if(mBuiltin.MissingComponentClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find MissingComponent managed class.");

	mBuiltin.SceneObjectClass = engineAssembly->GetClass(kEngineNs, "SceneObject");
	if(mBuiltin.SceneObjectClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find SceneObject managed class.");

	mBuiltin.RrefBaseClass = engineAssembly->GetClass(kEngineNs, "RRefBase");
	if(mBuiltin.RrefBaseClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find RRefBase managed class.");

	mBuiltin.GenericRRefClass = engineAssembly->GetClass(kEngineNs, "RRef`1");
	if(mBuiltin.GenericRRefClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find RRef<T> managed class.");

	mBuiltin.GenericAsyncOpClass = engineAssembly->GetClass(kEngineNs, "AsyncOp`1");
	if(mBuiltin.GenericAsyncOpClass == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find AsyncOp<T> managed class.");

	mBuiltin.SerializeFieldAttribute = engineAssembly->GetClass(kEngineNs, "SerializeField");
	if(mBuiltin.SerializeFieldAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find SerializeField managed class.");

	mBuiltin.HideInInspectorAttribute = engineAssembly->GetClass(kEngineNs, "HideInInspector");
	if(mBuiltin.HideInInspectorAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find HideInInspector managed class.");

	mBuiltin.ShowInInspectorAttribute = engineAssembly->GetClass(kEngineNs, "ShowInInspector");
	if(mBuiltin.ShowInInspectorAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find ShowInInspector managed class.");

	mBuiltin.CategoryAttribute = engineAssembly->GetClass(kEngineNs, "Category");
	if(mBuiltin.CategoryAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Category managed class.");

	mBuiltin.OrderAttribute = engineAssembly->GetClass(kEngineNs, "Order");
	if(mBuiltin.OrderAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Order managed class.");

	mBuiltin.InlineAttribute = engineAssembly->GetClass(kEngineNs, "Inline");
	if(mBuiltin.InlineAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find Inline managed class.");

	mBuiltin.LoadOnAssignAttribute = engineAssembly->GetClass(kEngineNs, "LoadOnAssign");
	if(mBuiltin.LoadOnAssignAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find LoadOnAssign managed class.");

	mBuiltin.HdrAttribute = engineAssembly->GetClass(kEngineNs, "HDR");
	if(mBuiltin.HdrAttribute == nullptr)
		B3D_EXCEPT(InvalidStateException, "Cannot find HDR managed class.");

	mBaseTypesInitialized = true;
}

void ScriptAssemblyManager::LoadTypeMappings(MonoAssembly& assembly, const BuiltinTypeMappings& mapping)
{
	for(auto& entry : mapping.Resources)
	{
		BuiltinResourceInfo info = entry;
		info.MonoClass = assembly.GetClass(entry.MetaData->Namespace, entry.MetaData->Name);

		::MonoReflectionType* type = MonoUtil::GetType(info.MonoClass->GetInternalClass());

		mBuiltinResourceInfos[type] = info;
		mBuiltinResourceInfosByTID[info.TypeId] = info;
		mBuiltinResourceInfosByType[(u32)info.ResType] = info;
	}
}

void ScriptAssemblyManager::InitializeScriptWrapperMetaDataLookup(MonoAssembly& assembly)
{
	const UnorderedMap<String, Vector<RegisteredScriptWrapperTypeInformation>> scriptWrapperTypeInformationMap = MonoManager::GetScriptWrapperTypeInformation();
	const auto found = scriptWrapperTypeInformationMap.find(assembly.GetName());
	if(found == scriptWrapperTypeInformationMap.end())
		return;

	for(const auto& entry : found->second)
	{
		if(!B3D_ENSURE(entry.MetaData))
			continue;

		::MonoReflectionType* type = MonoUtil::GetType(entry.MetaData->ScriptClass->GetInternalClass());
		mScriptWrapperMetaDataByScriptClass[type] = entry.MetaData;

		if(entry.MetaData->TypeId != ~0u)
			mScriptWrapperMetaDataByTypeId[entry.MetaData->TypeId] = entry.MetaData;
	}
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

const ScriptWrapperObjectMetaData* ScriptAssemblyManager::GetScriptWrapperMetaData(u32 typeId) const
{
	if(auto found = mScriptWrapperMetaDataByTypeId.find(typeId); found != mScriptWrapperMetaDataByTypeId.end())
		return found->second;

	return nullptr;
}

const ScriptWrapperObjectMetaData* ScriptAssemblyManager::GetScriptWrapperMetaData(::MonoReflectionType* type) const
{
	if(auto found = mScriptWrapperMetaDataByScriptClass.find(type); found != mScriptWrapperMetaDataByScriptClass.end())
		return found->second;

	return nullptr;
}

bool ScriptAssemblyManager::GetSerializableObjectInfo(const String& ns, const String& typeName, SPtr<ManagedSerializableObjectInfo>& outInfo)
{
	String fullName = ns + "." + typeName;
	for(auto& curAssembly : mAssemblyInfos)
	{
		if(curAssembly.second == nullptr)
			continue;

		auto iterFind = curAssembly.second->TypeNameToId.find(fullName);
		if(iterFind != curAssembly.second->TypeNameToId.end())
		{
			outInfo = curAssembly.second->ObjectInfos[iterFind->second];

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
		auto iterFind = curAssembly.second->TypeNameToId.find(fullName);
		if(iterFind != curAssembly.second->TypeNameToId.end())
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
			B3D_LOG(Warning, Script, "Unsupported type provided.");
			return nullptr;
		}

		MonoPrimitiveType monoPrimitiveType = MonoUtil::GetPrimitiveType(klass);
		if(monoPrimitiveType != MonoPrimitiveType::Class && monoPrimitiveType != MonoPrimitiveType::ValueType)
		{
			B3D_LOG(Warning, Script, "Unsupported type provided.");
			return nullptr;
		}

		const ScriptWrapperObjectMetaData* managedResourceMeta = ScriptManagedResource::GetMetaData();
		const ScriptWrapperObjectMetaData* managedComponentMeta = ScriptManagedComponent::GetMetaData();

		if(monoClass->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass)) // Resource
		{
			if(monoClass == ScriptResource::GetMetaData()->ScriptClass ||
			   monoClass == ScriptManagedResource::GetMetaData()->ScriptClass)
			{
				B3D_LOG(Warning, Script, "Unsupported type provided.");
				return nullptr;
			}

			if(monoClass->IsSubClassOf(managedResourceMeta->ScriptClass))
			{
				ScriptManagedResource* scriptResource = nullptr;
				managedResourceMeta->ScriptObjectWrapperPointerField->Get(value, &scriptResource);

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
					B3D_ASSERT(false && "Unable to find information about a built-in resource. Did you update BuiltinResourceTypes list?");
					return nullptr;
				}

				ScriptResourceBase* scriptResource = nullptr;
				builtinInfo->MetaData->ScriptObjectWrapperPointerField->Get(value, &scriptResource);

				HResource handle = scriptResource->GetGenericHandle();
				if(!handle.IsLoaded(false))
					return nullptr;

				return handle.GetShared();
			}
		}
		else if(monoClass->IsSubClassOf(mBuiltin.ComponentClass)) // Component
		{
			if(monoClass == mBuiltin.ComponentClass || monoClass == mBuiltin.ManagedComponentClass)
			{
				B3D_LOG(Warning, Script, "Unsupported type provided.");
				return nullptr;
			}

			if(monoClass->IsSubClassOf(mBuiltin.ManagedComponentClass))
			{
				ScriptManagedComponent* scriptComponent = ScriptManagedComponent::GetScriptObjectWrapper(value);

				HManagedComponent component = scriptComponent->GetNativeObjectAsHandle();
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
				const ScriptWrapperObjectMetaData* const scriptWrapperObjectMetaData = GetScriptWrapperMetaData(type);
				if(scriptWrapperObjectMetaData == nullptr)
				{
					B3D_ASSERT(false && "Unable to find information about a built-in component. Is it script exported?");
					return nullptr;
				}

				ScriptGameObjectWrapper* const scriptGameObjectWrapper = ScriptGameObjectWrapper::GetScriptObjectWrapper(*scriptWrapperObjectMetaData, value);

				HComponent handle = B3DStaticGameObjectCast<Component>(scriptGameObjectWrapper->GetBaseNativeObjectAsHandle());
				if(handle.IsDestroyed())
					return nullptr;

				return handle.GetShared();
			}
		}

		// Generic class or value type
		String elementNs;
		String elementTypeName;
		MonoUtil::GetClassName(value, elementNs, elementTypeName);

		SPtr<ManagedSerializableObjectInfo> objInfo;
		if(!Instance().GetSerializableObjectInfo(elementNs, elementTypeName, objInfo))
		{
			B3D_LOG(Error, Script, "Object has no serialization meta-data.");
			return nullptr;
		}

		if(objInfo->TypeInfo->RttiTypeId != 0)
		{
			::MonoClass* monoClass = MonoUtil::GetClass(value);
			::MonoReflectionType* monoType = MonoUtil::GetType(monoClass);

			const ScriptWrapperObjectMetaData* const scriptWrapperObjectMetaData = GetScriptWrapperMetaData(monoType);
			if(!B3D_ENSURE(scriptWrapperObjectMetaData != nullptr))
				return nullptr;

			const ScriptReflectableWrapper* const scriptReflectableWrapper = ScriptReflectableWrapper::GetScriptObjectWrapper(*scriptWrapperObjectMetaData, value);
			if(!B3D_ENSURE(scriptReflectableWrapper != nullptr))
				return nullptr;

			return scriptReflectableWrapper->GetBaseNativeObjectAsShared();
		}
		else
		{
			SPtr<ManagedSerializableObject> managedObj = ManagedSerializableObject::CreateFromExisting(value);
			if(!managedObj)
			{
				B3D_LOG(Error, Script, "Object failed to serialize due to an internal error.");
				return nullptr;
			}

			managedObj->Serialize();
			return managedObj;
		}
	}

	return nullptr;
}
