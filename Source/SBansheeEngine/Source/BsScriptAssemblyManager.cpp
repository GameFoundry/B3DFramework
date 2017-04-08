//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"
#include "BsMonoMethod.h"
#include "BsMonoProperty.h"
#include "BsScriptManagedResource.h"
#include "BsScriptComponent.h"
#include "BsScriptTexture.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptMaterial.h"
#include "BsScriptMesh.h"
#include "BsScriptFont.h"
#include "BsScriptShader.h"
#include "BsScriptShaderInclude.h"
#include "BsScriptPlainText.h"
#include "BsScriptScriptCode.h"
#include "BsScriptStringTable.h"
#include "BsScriptGUISkin.h"
#include "BsScriptPhysicsMaterial.h"
#include "BsScriptPhysicsMesh.h"
#include "BsScriptAudioClip.h"
#include "BsScriptPrefab.h"
#include "BsScriptAnimationClip.h"
#include "BsBuiltinComponentLookup.h"

namespace bs
{
	ScriptAssemblyManager::ScriptAssemblyManager()
		: mBaseTypesInitialized(false), mSystemArrayClass(nullptr), mSystemGenericListClass(nullptr)
		, mSystemGenericDictionaryClass(nullptr), mSystemTypeClass(nullptr), mComponentClass(nullptr)
		, mManagedComponentClass(nullptr), mSceneObjectClass(nullptr), mMissingComponentClass(nullptr)
		, mSerializeObjectAttribute(nullptr), mDontSerializeFieldAttribute(nullptr), mSerializeFieldAttribute(nullptr)
		, mHideInInspectorAttribute(nullptr), mShowInInspectorAttribute(nullptr), mRangeAttribute(nullptr)
		, mStepAttribute(nullptr)
	{

	}

	ScriptAssemblyManager::~ScriptAssemblyManager()
	{

	}

	Vector<String> ScriptAssemblyManager::getScriptAssemblies() const
	{
		Vector<String> initializedAssemblies;
		for (auto& assemblyPair : mAssemblyInfos)
			initializedAssemblies.push_back(assemblyPair.first);

		return initializedAssemblies;
	}

	void ScriptAssemblyManager::loadAssemblyInfo(const String& assemblyName)
	{
		if(!mBaseTypesInitialized)
			initializeBaseTypes();

		initializeBuiltinComponentInfos();

		// Process all classes and fields
		UINT32 mUniqueTypeId = 1;

		MonoAssembly* curAssembly = MonoManager::instance().getAssembly(assemblyName);
		if(curAssembly == nullptr)
			return;

		SPtr<ManagedSerializableAssemblyInfo> assemblyInfo = bs_shared_ptr_new<ManagedSerializableAssemblyInfo>();
		assemblyInfo->mName = assemblyName;

		mAssemblyInfos[assemblyName] = assemblyInfo;

		MonoClass* managedResourceClass = ScriptManagedResource::getMetaData()->scriptClass;

		// Populate class data
		const Vector<MonoClass*>& allClasses = curAssembly->getAllClasses();
		for(auto& curClass : allClasses)
		{
			if ((curClass->isSubClassOf(mManagedComponentClass) || curClass->isSubClassOf(managedResourceClass) ||
				curClass->hasAttribute(mSerializeObjectAttribute)) && curClass != mManagedComponentClass && 
				curClass != managedResourceClass)
			{
				SPtr<ManagedSerializableTypeInfoObject> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoObject>();
				typeInfo->mTypeNamespace = curClass->getNamespace();
				typeInfo->mTypeName = curClass->getTypeName();
				typeInfo->mTypeId = mUniqueTypeId++;

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

				SPtr<ManagedSerializableFieldInfo> fieldInfo = bs_shared_ptr_new<ManagedSerializableFieldInfo>();
				fieldInfo->mFieldId = mUniqueFieldId++;
				fieldInfo->mName = field->getName();
				fieldInfo->mMonoField = field;
				fieldInfo->mTypeInfo = typeInfo;
				fieldInfo->mParentTypeId = objInfo->mTypeInfo->mTypeId;
				
				MonoMemberVisibility visibility = field->getVisibility();
				if (visibility == MonoMemberVisibility::Public)
				{
					if (!field->hasAttribute(mDontSerializeFieldAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Serializable;

					if (!field->hasAttribute(mHideInInspectorAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Inspectable;

					fieldInfo->mFlags |= ScriptFieldFlag::Animable;
				}
				else
				{
					if (field->hasAttribute(mSerializeFieldAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Serializable;

					if (field->hasAttribute(mShowInInspectorAttribute))
						fieldInfo->mFlags |= ScriptFieldFlag::Inspectable;
				}

				if (field->hasAttribute(mRangeAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::Range;

				if (field->hasAttribute(mStepAttribute))
					fieldInfo->mFlags |= ScriptFieldFlag::Step;

				objInfo->mFieldNameToId[fieldInfo->mName] = fieldInfo->mFieldId;
				objInfo->mFields[fieldInfo->mFieldId] = fieldInfo;
			}

			const Vector<MonoProperty*>& properties = objInfo->mMonoClass->getAllProperties();
			for (auto& property : properties)
			{
				SPtr<ManagedSerializableTypeInfo> typeInfo = getTypeInfo(property->getReturnType());
				if (typeInfo == nullptr)
					continue;

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

					if (property->hasAttribute(mSerializeFieldAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Serializable;

					if (property->hasAttribute(mShowInInspectorAttribute))
						propertyInfo->mFlags |= ScriptFieldFlag::Inspectable;
				}

				if (property->hasAttribute(mRangeAttribute))
					propertyInfo->mFlags |= ScriptFieldFlag::Range;

				if (property->hasAttribute(mStepAttribute))
					propertyInfo->mFlags |= ScriptFieldFlag::Step;

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
		switch(monoPrimitiveType)
		{
		case MonoPrimitiveType::Boolean:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Bool;
				return typeInfo;
			}
		case MonoPrimitiveType::Char:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Char;
				return typeInfo;
			}
		case MonoPrimitiveType::I8:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I8;
				return typeInfo;
			}
		case MonoPrimitiveType::U8:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U8;
				return typeInfo;
			}
		case MonoPrimitiveType::I16:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I16;
				return typeInfo;
			}
		case MonoPrimitiveType::U16:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U16;
				return typeInfo;
			}
		case MonoPrimitiveType::I32:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I32;
				return typeInfo;
			}
		case MonoPrimitiveType::U32:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U32;
				return typeInfo;
			}
		case MonoPrimitiveType::I64:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I64;
				return typeInfo;
			}
		case MonoPrimitiveType::U64:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U64;
				return typeInfo;
			}
		case MonoPrimitiveType::String:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::String;
				return typeInfo;
			}
		case MonoPrimitiveType::R32:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Float;
				return typeInfo;
			}
		case MonoPrimitiveType::R64:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Double;
				return typeInfo;
			}
		case MonoPrimitiveType::Class:
			if(monoClass->isSubClassOf(ScriptResource::getMetaData()->scriptClass)) // Resource
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				typeInfo->mTypeNamespace = monoClass->getNamespace();
				typeInfo->mTypeName = monoClass->getTypeName();

				if(monoClass == ScriptResource::getMetaData()->scriptClass)
					typeInfo->mType = ScriptReferenceType::Resource;
				else if (monoClass->isSubClassOf(ScriptTexture::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Texture;
				else if (monoClass->isSubClassOf(ScriptSpriteTexture::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::SpriteTexture;
				else if (monoClass->isSubClassOf(ScriptManagedResource::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::ManagedResource;
				else if (monoClass->isSubClassOf(ScriptShader::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Shader;
				else if (monoClass->isSubClassOf(ScriptShaderInclude::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::ShaderInclude;
				else if (monoClass->isSubClassOf(ScriptMaterial::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Material;
				else if (monoClass->isSubClassOf(ScriptMesh::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Mesh;
				else if (monoClass->isSubClassOf(ScriptPlainText::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::PlainText;
				else if (monoClass->isSubClassOf(ScriptScriptCode::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::ScriptCode;
				else if (monoClass->isSubClassOf(ScriptPrefab::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Prefab;
				else if (monoClass->isSubClassOf(ScriptFont::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Font;
				else if (monoClass->isSubClassOf(ScriptStringTable::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::StringTable;
				else if (monoClass->isSubClassOf(ScriptGUISkin::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::GUISkin;
				else if (monoClass->isSubClassOf(ScriptPhysicsMaterial::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::PhysicsMaterial;
				else if (monoClass->isSubClassOf(ScriptPhysicsMesh::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::PhysicsMesh;
				else if (monoClass->isSubClassOf(ScriptAudioClip::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::AudioClip;
				else if (monoClass->isSubClassOf(ScriptAnimationClip::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::AnimationClip;
				else
				{
					assert(false && "Unrecognized resource type");
				}

				return typeInfo;
			}
			else if (monoClass->isSubClassOf(mSceneObjectClass) || monoClass->isSubClassOf(mComponentClass)) // Game object
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				typeInfo->mTypeNamespace = monoClass->getNamespace();
				typeInfo->mTypeName = monoClass->getTypeName();
				typeInfo->mRTIITypeId = 0;

				if (monoClass == mComponentClass)
					typeInfo->mType = ScriptReferenceType::BuiltinComponentBase;
				else if (monoClass == mManagedComponentClass)
					typeInfo->mType = ScriptReferenceType::ManagedComponentBase;
				else if (monoClass->isSubClassOf(mSceneObjectClass))
					typeInfo->mType = ScriptReferenceType::SceneObject;
				else if (monoClass->isSubClassOf(mManagedComponentClass))
					typeInfo->mType = ScriptReferenceType::ManagedComponent;
				else if (monoClass->isSubClassOf(mComponentClass))
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
				SPtr<ManagedSerializableObjectInfo> objInfo;
				if (getSerializableObjectInfo(monoClass->getNamespace(), monoClass->getTypeName(), objInfo))
					return objInfo->mTypeInfo;
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
			if(monoClass->getFullName() == mSystemGenericListClass->getFullName()) // Full name is part of CIL spec, so it is just fine to compare like this
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
			else if(monoClass->getFullName() == mSystemGenericDictionaryClass->getFullName())
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

		mSystemArrayClass = nullptr;
		mSystemGenericListClass = nullptr;
		mSystemGenericDictionaryClass = nullptr;
		mSystemTypeClass = nullptr;

		mSerializeObjectAttribute = nullptr;
		mDontSerializeFieldAttribute = nullptr;

		mComponentClass = nullptr;
		mManagedComponentClass = nullptr;
		mSceneObjectClass = nullptr;
		mMissingComponentClass = nullptr;

		mSerializeFieldAttribute = nullptr;
		mHideInInspectorAttribute = nullptr;
		mShowInInspectorAttribute = nullptr;
		mRangeAttribute = nullptr;
		mStepAttribute = nullptr;
	}

	void ScriptAssemblyManager::initializeBaseTypes()
	{
		// Get necessary classes for detecting needed class & field information
		MonoAssembly* corlib = MonoManager::instance().getAssembly("corlib");
		if(corlib == nullptr)
			BS_EXCEPT(InvalidStateException, "corlib assembly is not loaded.");

		MonoAssembly* bansheeEngineAssembly = MonoManager::instance().getAssembly(ENGINE_ASSEMBLY);
		if(bansheeEngineAssembly == nullptr)
			BS_EXCEPT(InvalidStateException, String(ENGINE_ASSEMBLY) +  " assembly is not loaded.");

		mSystemArrayClass = corlib->getClass("System", "Array");
		if(mSystemArrayClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find System.Array managed class.");

		mSystemGenericListClass = corlib->getClass("System.Collections.Generic", "List`1");
		if(mSystemGenericListClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find List<T> managed class.");

		mSystemGenericDictionaryClass = corlib->getClass("System.Collections.Generic", "Dictionary`2");
		if(mSystemGenericDictionaryClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Dictionary<TKey, TValue> managed class.");

		mSystemTypeClass = corlib->getClass("System", "Type");
		if (mSystemTypeClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Type managed class.");

		mSerializeObjectAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "SerializeObject");
		if(mSerializeObjectAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SerializableObject managed class.");

		mDontSerializeFieldAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "DontSerializeField");
		if(mDontSerializeFieldAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find DontSerializeField managed class.");

		mRangeAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "Range");
		if (mRangeAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Range managed class.");

		mStepAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "Step");
		if (mStepAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Step managed class.");

		mComponentClass = bansheeEngineAssembly->getClass("BansheeEngine", "Component");
		if(mComponentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Component managed class.");

		mManagedComponentClass = bansheeEngineAssembly->getClass("BansheeEngine", "ManagedComponent");
		if (mManagedComponentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find ManagedComponent managed class.");

		mMissingComponentClass = bansheeEngineAssembly->getClass("BansheeEngine", "MissingComponent");
		if (mMissingComponentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find MissingComponent managed class.");

		mSceneObjectClass = bansheeEngineAssembly->getClass("BansheeEngine", "SceneObject");
		if(mSceneObjectClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SceneObject managed class.");

		mSerializeFieldAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "SerializeField");
		if(mSerializeFieldAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SerializeField managed class.");

		mHideInInspectorAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "HideInInspector");
		if(mHideInInspectorAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find HideInInspector managed class.");

		mShowInInspectorAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "ShowInInspector");
		if (mShowInInspectorAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find ShowInInspector managed class.");

		mBaseTypesInitialized = true;
	}

	void ScriptAssemblyManager::initializeBuiltinComponentInfos()
	{
		mBuiltinComponentInfos.clear();
		mBuiltinComponentInfosByTID.clear();

		Vector<BuiltinComponentInfo> allComponentsInfos = BuiltinComponents::getEntries();

		for(auto& entry : allComponentsInfos)
		{
			MonoAssembly* assembly = MonoManager::instance().getAssembly(entry.metaData->assembly);
			if (assembly == nullptr)
				continue;

			BuiltinComponentInfo info = entry;
			info.monoClass = assembly->getClass(entry.metaData->ns, entry.metaData->name);

			::MonoReflectionType* type = MonoUtil::getType(info.monoClass->_getInternalClass());

			mBuiltinComponentInfos[type] = info;
			mBuiltinComponentInfosByTID[info.typeId] = info;
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