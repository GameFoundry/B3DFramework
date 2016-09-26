//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsException.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableAssemblyInfoRTTI : public RTTIType<ManagedSerializableAssemblyInfo, IReflectable, ManagedSerializableAssemblyInfoRTTI>
	{
	private:
		String& getName(ManagedSerializableAssemblyInfo* obj)
		{
			return obj->mName;
		}

		void setName(ManagedSerializableAssemblyInfo* obj, String& val)
		{
			obj->mName = val;
		}

		SPtr<ManagedSerializableObjectInfo> getSerializableObjectInfo(ManagedSerializableAssemblyInfo* obj, UINT32 idx) 
		{ 
			auto iter = obj->mObjectInfos.begin();
			for(UINT32 i = 0; i < idx; i++)
				iter++;

			return iter->second;
		}

		void setSerializableObjectInfo(ManagedSerializableAssemblyInfo* obj, UINT32 idx, SPtr<ManagedSerializableObjectInfo> val) 
		{ 
			obj->mTypeNameToId[val->getFullTypeName()] = val->mTypeInfo->mTypeId;
			obj->mObjectInfos[val->mTypeInfo->mTypeId] = val;
		}
		
		UINT32 getSerializableObjectInfoArraySize(ManagedSerializableAssemblyInfo* obj) { return (UINT32)obj->mObjectInfos.size(); }
		void setSerializableObjectInfoArraySize(ManagedSerializableAssemblyInfo* obj, UINT32 size) {  }

	public:
		ManagedSerializableAssemblyInfoRTTI()
		{
			addPlainField("mName", 0, &ManagedSerializableAssemblyInfoRTTI::getName, &ManagedSerializableAssemblyInfoRTTI::setName);
			addReflectablePtrArrayField("mObjectInfos", 1, &ManagedSerializableAssemblyInfoRTTI::getSerializableObjectInfo, 
				&ManagedSerializableAssemblyInfoRTTI::getSerializableObjectInfoArraySize, &ManagedSerializableAssemblyInfoRTTI::setSerializableObjectInfo, 
				&ManagedSerializableAssemblyInfoRTTI::setSerializableObjectInfoArraySize);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableAssemblyInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableAssemblyInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableAssemblyInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableObjectInfoRTTI : public RTTIType<ManagedSerializableObjectInfo, IReflectable, ManagedSerializableObjectInfoRTTI>
	{
	private:
		using RTTIType<ManagedSerializableObjectInfo, IReflectable, ManagedSerializableObjectInfoRTTI>::getBaseClass;

		SPtr<ManagedSerializableTypeInfoObject> getTypeInfo(ManagedSerializableObjectInfo* obj)
		{
			return obj->mTypeInfo;
		}

		void setTypeInfo(ManagedSerializableObjectInfo* obj, SPtr<ManagedSerializableTypeInfoObject> val)
		{
			obj->mTypeInfo = val;
		}

		SPtr<ManagedSerializableObjectInfo> getBaseClass(ManagedSerializableObjectInfo* obj)
		{
			return obj->mBaseClass;
		}

		void setBaseClass(ManagedSerializableObjectInfo* obj, SPtr<ManagedSerializableObjectInfo> val)
		{
			obj->mBaseClass = val;
		}

		SPtr<ManagedSerializableMemberInfo> getSerializableFieldInfo(ManagedSerializableObjectInfo* obj, UINT32 idx) 
		{ 
			auto iter = obj->mFields.begin();
			for(UINT32 i = 0; i < idx; i++)
				iter++;

			return iter->second;
		}

		void setSerializableFieldInfo(ManagedSerializableObjectInfo* obj, UINT32 idx, SPtr<ManagedSerializableMemberInfo> val) 
		{ 
			obj->mFieldNameToId[val->mName] = val->mFieldId;
			obj->mFields[val->mFieldId] = val;
		}

		UINT32 getSerializableFieldInfoArraySize(ManagedSerializableObjectInfo* obj) { return (UINT32)obj->mFields.size(); }
		void setSerializableFieldInfoArraySize(ManagedSerializableObjectInfo* obj, UINT32 size) {  }

	public:
		ManagedSerializableObjectInfoRTTI()
		{
			addReflectablePtrField("mTypeInfo", 0, &ManagedSerializableObjectInfoRTTI::getTypeInfo, &ManagedSerializableObjectInfoRTTI::setTypeInfo);
			addReflectablePtrField("mBaseClass", 2, &ManagedSerializableObjectInfoRTTI::getBaseClass, &ManagedSerializableObjectInfoRTTI::setBaseClass);

			addReflectablePtrArrayField("mFields", 3, &ManagedSerializableObjectInfoRTTI::getSerializableFieldInfo, 
				&ManagedSerializableObjectInfoRTTI::getSerializableFieldInfoArraySize, &ManagedSerializableObjectInfoRTTI::setSerializableFieldInfo, 
				&ManagedSerializableObjectInfoRTTI::setSerializableFieldInfoArraySize);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableObjectInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableObjectInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableObjectInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableMemberInfoRTTI : public RTTIType<ManagedSerializableMemberInfo, IReflectable, ManagedSerializableMemberInfoRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mName, 0)
			BS_RTTI_MEMBER_REFLPTR(mTypeInfo, 1)
			BS_RTTI_MEMBER_PLAIN(mFieldId, 2)
			BS_RTTI_MEMBER_PLAIN(mFlags, 3)
			BS_RTTI_MEMBER_PLAIN(mParentTypeId, 4)
		BS_END_RTTI_MEMBERS
			
	public:
		ManagedSerializableMemberInfoRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableMemberInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableMemberInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			// This is an abstract class, but it wasn't always. For compatibility sake we return an object instance so old
			// data can still be properly read.
			return bs_shared_ptr_new<ManagedSerializableFieldInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldInfoRTTI : public RTTIType<ManagedSerializableFieldInfo, ManagedSerializableMemberInfo, ManagedSerializableFieldInfoRTTI>
	{
	private:

	public:
		ManagedSerializableFieldInfoRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableFieldInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializablePropertyInfoRTTI : public RTTIType<ManagedSerializablePropertyInfo, ManagedSerializableMemberInfo, ManagedSerializablePropertyInfoRTTI>
	{
	private:

	public:
		ManagedSerializablePropertyInfoRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializablePropertyInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializablePropertyInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializablePropertyInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRTTI : public RTTIType<ManagedSerializableTypeInfo, IReflectable, ManagedSerializableTypeInfoRTTI>
	{
	private:

	public:
		ManagedSerializableTypeInfoRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InvalidStateException, "Cannot instantiate an abstract class");
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoPrimitiveRTTI : public RTTIType<ManagedSerializableTypeInfoPrimitive, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoPrimitiveRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mType, 0)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoPrimitiveRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoPrimitive";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoPrimitive;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRefRTTI : public RTTIType<ManagedSerializableTypeInfoRef, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoRefRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mType, 0)
			BS_RTTI_MEMBER_PLAIN(mTypeName, 1)
			BS_RTTI_MEMBER_PLAIN(mTypeNamespace, 2)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoRefRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoRef";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoRef;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoObjectRTTI : public RTTIType<ManagedSerializableTypeInfoObject, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoObjectRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mTypeName, 0)
			BS_RTTI_MEMBER_PLAIN(mTypeNamespace, 1)
			BS_RTTI_MEMBER_PLAIN(mValueType, 2)
			BS_RTTI_MEMBER_PLAIN(mTypeId, 4)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoObjectRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoObject>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoArrayRTTI : public RTTIType<ManagedSerializableTypeInfoArray, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoArrayRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mElementType, 0)
			BS_RTTI_MEMBER_PLAIN(mRank, 1)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoArrayRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoArray";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoArray;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoArray>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoListRTTI : public RTTIType<ManagedSerializableTypeInfoList, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoListRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mElementType, 0)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoListRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoList";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoList;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoList>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoDictionaryRTTI : public RTTIType<ManagedSerializableTypeInfoDictionary, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoDictionaryRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mKeyType, 0)
			BS_RTTI_MEMBER_REFLPTR(mValueType, 1)
		BS_END_RTTI_MEMBERS


	public:
		ManagedSerializableTypeInfoDictionaryRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoDictionary";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoDictionary;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoDictionary>();
		}
	};

	/** @} */
	/** @endcond */
}