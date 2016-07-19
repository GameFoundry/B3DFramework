//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedSerializableObjectInfo.h"
#include "BsManagedSerializableObjectInfoRTTI.h"
#include "BsScriptRange.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoField.h"
#include "BsScriptTexture2D.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptTexture3D.h"
#include "BsScriptTextureCube.h"
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
#include "BsScriptAnimationClip.h"
#include "BsScriptPrefab.h"
#include "BsScriptManagedResource.h"
#include <BsScriptStep.h>

namespace BansheeEngine
{
	RTTITypeBase* ManagedSerializableAssemblyInfo::getRTTIStatic()
	{
		return ManagedSerializableAssemblyInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableAssemblyInfo::getRTTI() const
	{
		return ManagedSerializableAssemblyInfo::getRTTIStatic();
	}

	ManagedSerializableObjectInfo::ManagedSerializableObjectInfo()
		:mMonoClass(nullptr)
	{

	}

	SPtr<ManagedSerializableFieldInfo> ManagedSerializableObjectInfo::findMatchingField(const SPtr<ManagedSerializableFieldInfo>& fieldInfo,
		const SPtr<ManagedSerializableTypeInfo>& fieldTypeInfo) const
	{
		const ManagedSerializableObjectInfo* objInfo = this;
		while (objInfo != nullptr)
		{
			if (objInfo->mTypeInfo->matches(fieldTypeInfo))
			{
				auto iterFind = objInfo->mFieldNameToId.find(fieldInfo->mName);
				if (iterFind != objInfo->mFieldNameToId.end())
				{
					auto iterFind2 = objInfo->mFields.find(iterFind->second);
					if (iterFind2 != objInfo->mFields.end())
					{
						SPtr<ManagedSerializableFieldInfo> foundField = iterFind2->second;
						if (foundField->isSerializable())
						{
							if (fieldInfo->mTypeInfo->matches(foundField->mTypeInfo))
								return foundField;
						}
					}
				}

				return nullptr;
			}

			if (objInfo->mBaseClass != nullptr)
				objInfo = objInfo->mBaseClass.get();
			else
				objInfo = nullptr;
		}

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableObjectInfo::getRTTIStatic()
	{
		return ManagedSerializableObjectInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableObjectInfo::getRTTI() const
	{
		return ManagedSerializableObjectInfo::getRTTIStatic();
	}

	ManagedSerializableFieldInfo::ManagedSerializableFieldInfo()
		:mFieldId(0), mFlags((ScriptFieldFlags)0), mMonoField(nullptr)
	{

	}

	float ManagedSerializableFieldInfo::getRangeMinimum() const
	{
		if (((UINT32)mFlags & (UINT32)ScriptFieldFlags::Range) != 0)
		{

			MonoClass* range = ScriptAssemblyManager::instance().getRangeAttribute();
			if (range != nullptr)
			{
				float min = 0;
				ScriptRange::getMinRangeField()->getValue(mMonoField->getAttribute(range), &min);
				return min;
			}
		}
		return 0;
	}

	float ManagedSerializableFieldInfo::getRangeMaximum() const
	{
		if (((UINT32)mFlags & (UINT32)ScriptFieldFlags::Range) != 0)
		{

			MonoClass* range = ScriptAssemblyManager::instance().getRangeAttribute();
			if (range != nullptr)
			{
				float max = 0;
				ScriptRange::getMaxRangeField()->getValue(mMonoField->getAttribute(range), &max);
				return max;
			}
		}
		return 0;
	}

	bool ManagedSerializableFieldInfo::renderAsSlider() const
	{
		if (((UINT32)mFlags & (UINT32)ScriptFieldFlags::Range) != 0)
		{
			MonoClass* range = ScriptAssemblyManager::instance().getRangeAttribute();
			if (range != nullptr)
			{
				bool slider = false;
				ScriptRange::getSliderField()->getValue(mMonoField->getAttribute(range), &slider);
				return slider;
			}
		}
		return false;
	}


	float ManagedSerializableFieldInfo::getStep() const
	{
		if (((UINT32)mFlags & (UINT32)ScriptFieldFlags::Step) != 0)
		{

			MonoClass* step = ScriptAssemblyManager::instance().getStepAttribute();
			if (step != nullptr)
			{
				float value = 0;
				ScriptStep::getStepField()->getValue(mMonoField->getAttribute(step), &value);
				return value;
			}
		}
		return 0;
	}

	RTTITypeBase* ManagedSerializableFieldInfo::getRTTIStatic()
	{
		return ManagedSerializableFieldInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableFieldInfo::getRTTI() const
	{
		return ManagedSerializableFieldInfo::getRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::getRTTI() const
	{
		return ManagedSerializableTypeInfo::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoPrimitive::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoPrimitive>(typeInfo))
			return false;

		auto primTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);

		return primTypeInfo->mType == mType;
	}

	bool ManagedSerializableTypeInfoPrimitive::isTypeLoaded() const
	{
		return mType < ScriptPrimitiveType::Count; // Ignoring some removed types
	}

	::MonoClass* ManagedSerializableTypeInfoPrimitive::getMonoClass() const
	{
		switch(mType)
		{
		case ScriptPrimitiveType::Bool:
			return MonoUtil::getBoolClass();
		case ScriptPrimitiveType::Char:
			return MonoUtil::getCharClass();
		case ScriptPrimitiveType::I8:
			return MonoUtil::getSByteClass();
		case ScriptPrimitiveType::U8:
			return MonoUtil::getByteClass();
		case ScriptPrimitiveType::I16:
			return MonoUtil::getINT16Class();
		case ScriptPrimitiveType::U16:
			return MonoUtil::getUINT16Class();
		case ScriptPrimitiveType::I32:
			return MonoUtil::getINT32Class();
		case ScriptPrimitiveType::U32:
			return MonoUtil::getUINT32Class();
		case ScriptPrimitiveType::I64:
			return MonoUtil::getINT64Class();
		case ScriptPrimitiveType::U64:
			return MonoUtil::getUINT64Class();
		case ScriptPrimitiveType::Float:
			return MonoUtil::getFloatClass();
		case ScriptPrimitiveType::Double:
			return MonoUtil::getDoubleClass();
		case ScriptPrimitiveType::String:
			return MonoUtil::getStringClass();
		default:
			break;
		}

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoPrimitiveRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::getRTTI() const
	{
		return ManagedSerializableTypeInfoPrimitive::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoRef::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if (!rtti_is_of_type<ManagedSerializableTypeInfoRef>(typeInfo))
			return false;

		auto objTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

		return objTypeInfo->mTypeNamespace == mTypeNamespace && objTypeInfo->mTypeName == mTypeName;
	}

	bool ManagedSerializableTypeInfoRef::isTypeLoaded() const
	{
		switch (mType)
		{
		case ScriptReferenceType::Resource:
		case ScriptReferenceType::Texture2D:
		case ScriptReferenceType::Texture3D:
		case ScriptReferenceType::TextureCube:
		case ScriptReferenceType::SpriteTexture:
		case ScriptReferenceType::Shader:
		case ScriptReferenceType::ShaderInclude:
		case ScriptReferenceType::Material:
		case ScriptReferenceType::Mesh:
		case ScriptReferenceType::PlainText:
		case ScriptReferenceType::ScriptCode:
		case ScriptReferenceType::Prefab:
		case ScriptReferenceType::Font:
		case ScriptReferenceType::StringTable:
		case ScriptReferenceType::GUISkin:
		case ScriptReferenceType::PhysicsMaterial:
		case ScriptReferenceType::PhysicsMesh:
		case ScriptReferenceType::AudioClip:
		case ScriptReferenceType::AnimationClip:
		case ScriptReferenceType::SceneObject:
		case ScriptReferenceType::Component:
			return true;
		default:
			break;
		}

		return ScriptAssemblyManager::instance().hasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoRef::getMonoClass() const
	{
		switch (mType)
		{
		case ScriptReferenceType::Resource:
			return ScriptResource::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::Texture2D:
			return ScriptTexture2D::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::Texture3D:
			return ScriptTexture3D::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::TextureCube:
			return ScriptTextureCube::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::SpriteTexture:
			return ScriptSpriteTexture::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::Shader:
			return ScriptShader::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::ShaderInclude:
			return ScriptShaderInclude::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::Material:
			return ScriptMaterial::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::Mesh:
			return ScriptMesh::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::PlainText:
			return ScriptPlainText::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::ScriptCode:
			return ScriptScriptCode::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::Prefab:
			return ScriptPrefab::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::Font:
			return ScriptFont::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::StringTable:
			return ScriptStringTable::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::GUISkin:
			return ScriptGUISkin::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::PhysicsMaterial:
			return ScriptPhysicsMaterial::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::PhysicsMesh:
			return ScriptPhysicsMesh::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::AudioClip:
			return ScriptAudioClip::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::AnimationClip:
			return ScriptAnimationClip::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::SceneObject:
			return ScriptAssemblyManager::instance().getSceneObjectClass()->_getInternalClass();
		case ScriptReferenceType::Component:
			return ScriptAssemblyManager::instance().getComponentClass()->_getInternalClass();
		default:
			break;
		}

		// Custom component or resource
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRefRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::getRTTI() const
	{
		return ManagedSerializableTypeInfoRef::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoObject::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoObject>(typeInfo))
			return false;

		auto objTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo);

		return objTypeInfo->mTypeNamespace == mTypeNamespace && objTypeInfo->mTypeName == mTypeName && objTypeInfo->mValueType == mValueType;
	}

	bool ManagedSerializableTypeInfoObject::isTypeLoaded() const
	{
		return ScriptAssemblyManager::instance().hasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoObject::getMonoClass() const
	{
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if(!ScriptAssemblyManager::instance().getSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoObjectRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::getRTTI() const
	{
		return ManagedSerializableTypeInfoObject::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoArray::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoArray>(typeInfo))
			return false;

		auto arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(typeInfo);

		return arrayTypeInfo->mRank == mRank && arrayTypeInfo->mElementType->matches(mElementType);
	}

	bool ManagedSerializableTypeInfoArray::isTypeLoaded() const
	{
		return mElementType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoArray::getMonoClass() const
	{
		::MonoClass* elementClass = mElementType->getMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		return ScriptArray::buildArrayClass(mElementType->getMonoClass(), mRank);
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoArrayRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::getRTTI() const
	{
		return ManagedSerializableTypeInfoArray::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoList::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoList>(typeInfo))
			return false;

		auto listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(typeInfo);

		return listTypeInfo->mElementType->matches(mElementType);
	}

	bool ManagedSerializableTypeInfoList::isTypeLoaded() const
	{
		return mElementType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoList::getMonoClass() const
	{
		::MonoClass* elementClass = mElementType->getMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		MonoClass* genericListClass = ScriptAssemblyManager::instance().getSystemGenericListClass();
		::MonoClass* genParams[1] = { elementClass };

		return MonoUtil::bindGenericParameters(genericListClass->_getInternalClass(), genParams, 1);
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoListRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::getRTTI() const
	{
		return ManagedSerializableTypeInfoList::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoDictionary::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoDictionary>(typeInfo))
			return false;

		auto dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(typeInfo);

		return dictTypeInfo->mKeyType->matches(mKeyType) && dictTypeInfo->mValueType->matches(mValueType);
	}

	bool ManagedSerializableTypeInfoDictionary::isTypeLoaded() const
	{
		return mKeyType->isTypeLoaded() && mValueType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoDictionary::getMonoClass() const
	{
		::MonoClass* keyClass = mKeyType->getMonoClass();
		::MonoClass* valueClass = mValueType->getMonoClass();
		if(keyClass == nullptr || valueClass == nullptr)
			return nullptr;

		MonoClass* genericDictionaryClass = ScriptAssemblyManager::instance().getSystemGenericDictionaryClass();

		::MonoClass* params[2] = { keyClass, valueClass };
		return MonoUtil::bindGenericParameters(genericDictionaryClass->_getInternalClass(), params, 2);
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoDictionaryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::getRTTI() const
	{
		return ManagedSerializableTypeInfoDictionary::getRTTIStatic();
	}
}
