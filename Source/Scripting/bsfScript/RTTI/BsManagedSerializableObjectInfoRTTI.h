//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Error/BsException.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableAssemblyInfoRTTI : public RTTIType<ManagedSerializableAssemblyInfo, IReflectable, ManagedSerializableAssemblyInfoRTTI>
	{
	private:
		String& GetName(ManagedSerializableAssemblyInfo* obj)
		{
			return obj->MName;
		}

		void SetName(ManagedSerializableAssemblyInfo* obj, String& val)
		{
			obj->MName = val;
		}

		SPtr<ManagedSerializableObjectInfo> GetSerializableObjectInfo(ManagedSerializableAssemblyInfo* obj, u32 idx)
		{
			auto iter = obj->MObjectInfos.begin();
			for(u32 i = 0; i < idx; i++)
				iter++;

			return iter->second;
		}

		void SetSerializableObjectInfo(ManagedSerializableAssemblyInfo* obj, u32 idx, SPtr<ManagedSerializableObjectInfo> val)
		{
			obj->MTypeNameToId[val->GetFullTypeName()] = val->MTypeInfo->MTypeId;
			obj->MObjectInfos[val->MTypeInfo->MTypeId] = val;
		}

		u32 GetSerializableObjectInfoArraySize(ManagedSerializableAssemblyInfo* obj) { return (u32)obj->MObjectInfos.size(); }

		void SetSerializableObjectInfoArraySize(ManagedSerializableAssemblyInfo* obj, u32 size) {}

	public:
		ManagedSerializableAssemblyInfoRTTI()
		{
			AddPlainField("mName", 0, &ManagedSerializableAssemblyInfoRTTI::GetName, &ManagedSerializableAssemblyInfoRTTI::SetName);
			AddReflectablePtrArrayField("mObjectInfos", 1, &ManagedSerializableAssemblyInfoRTTI::GetSerializableObjectInfo, &ManagedSerializableAssemblyInfoRTTI::GetSerializableObjectInfoArraySize, &ManagedSerializableAssemblyInfoRTTI::SetSerializableObjectInfo, &ManagedSerializableAssemblyInfoRTTI::SetSerializableObjectInfoArraySize);
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableAssemblyInfo";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_SerializableAssemblyInfo;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedSerializableAssemblyInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableObjectInfoRTTI : public RTTIType<ManagedSerializableObjectInfo, IReflectable, ManagedSerializableObjectInfoRTTI>
	{
	private:
		using RTTIType<ManagedSerializableObjectInfo, IReflectable, ManagedSerializableObjectInfoRTTI>::GetBaseClass;

		SPtr<ManagedSerializableTypeInfoObject> GetTypeInfo(ManagedSerializableObjectInfo* obj)
		{
			return obj->MTypeInfo;
		}

		void SetTypeInfo(ManagedSerializableObjectInfo* obj, SPtr<ManagedSerializableTypeInfoObject> val)
		{
			obj->MTypeInfo = val;
		}

		SPtr<ManagedSerializableObjectInfo> GetBaseClass(ManagedSerializableObjectInfo* obj)
		{
			return obj->MBaseClass;
		}

		void SetBaseClass(ManagedSerializableObjectInfo* obj, SPtr<ManagedSerializableObjectInfo> val)
		{
			obj->MBaseClass = val;
		}

		SPtr<ManagedSerializableMemberInfo> GetSerializableFieldInfo(ManagedSerializableObjectInfo* obj, u32 idx)
		{
			auto iter = obj->MFields.begin();
			for(u32 i = 0; i < idx; i++)
				iter++;

			return iter->second;
		}

		void SetSerializableFieldInfo(ManagedSerializableObjectInfo* obj, u32 idx, SPtr<ManagedSerializableMemberInfo> val)
		{
			obj->MFieldNameToId[val->MName] = val->MFieldId;
			obj->MFields[val->MFieldId] = val;
		}

		u32 GetSerializableFieldInfoArraySize(ManagedSerializableObjectInfo* obj) { return (u32)obj->MFields.size(); }

		void SetSerializableFieldInfoArraySize(ManagedSerializableObjectInfo* obj, u32 size) {}

	public:
		ManagedSerializableObjectInfoRTTI()
		{
			AddReflectablePtrField("mTypeInfo", 0, &ManagedSerializableObjectInfoRTTI::GetTypeInfo, &ManagedSerializableObjectInfoRTTI::SetTypeInfo);
			AddReflectablePtrField("mBaseClass", 2, &ManagedSerializableObjectInfoRTTI::GetBaseClass, &ManagedSerializableObjectInfoRTTI::SetBaseClass);

			AddReflectablePtrArrayField("mFields", 3, &ManagedSerializableObjectInfoRTTI::GetSerializableFieldInfo, &ManagedSerializableObjectInfoRTTI::GetSerializableFieldInfoArraySize, &ManagedSerializableObjectInfoRTTI::SetSerializableFieldInfo, &ManagedSerializableObjectInfoRTTI::SetSerializableFieldInfoArraySize);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableObjectInfo";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableObjectInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableObjectInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableMemberInfoRTTI : public RTTIType<ManagedSerializableMemberInfo, IReflectable, ManagedSerializableMemberInfoRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(MName, 0)
			BS_RTTI_MEMBER_REFLPTR(MTypeInfo, 1)
			BS_RTTI_MEMBER_PLAIN(MFieldId, 2)
			BS_RTTI_MEMBER_PLAIN(MFlags, 3)
			BS_RTTI_MEMBER_PLAIN(MParentTypeId, 4)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableMemberInfoRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableMemberInfo";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableMemberInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			// This is an abstract class, but it wasn't always. For compatibility sake we return an object instance so old
			// data can still be properly read.
			return B3DMakeShared<ManagedSerializableFieldInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldInfoRTTI : public RTTIType<ManagedSerializableFieldInfo, ManagedSerializableMemberInfo, ManagedSerializableFieldInfoRTTI>
	{
	private:
	public:
		ManagedSerializableFieldInfoRTTI()
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableFieldInfo";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableFieldInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializablePropertyInfoRTTI : public RTTIType<ManagedSerializablePropertyInfo, ManagedSerializableMemberInfo, ManagedSerializablePropertyInfoRTTI>
	{
	private:
	public:
		ManagedSerializablePropertyInfoRTTI()
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializablePropertyInfo";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializablePropertyInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializablePropertyInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRTTI : public RTTIType<ManagedSerializableTypeInfo, IReflectable, ManagedSerializableTypeInfoRTTI>
	{
	private:
	public:
		ManagedSerializableTypeInfoRTTI()
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfo";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			BS_EXCEPT(InvalidStateException, "Cannot instantiate an abstract class");
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoPrimitiveRTTI : public RTTIType<ManagedSerializableTypeInfoPrimitive, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoPrimitiveRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(MType, 0)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoPrimitiveRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoPrimitive";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoPrimitive;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoPrimitive>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoEnumRTTI : public RTTIType<ManagedSerializableTypeInfoEnum, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoEnumRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(MUnderlyingType, 0)
			BS_RTTI_MEMBER_PLAIN(MTypeNamespace, 1)
			BS_RTTI_MEMBER_PLAIN(MTypeName, 2)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoEnumRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoEnum";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoEnum;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoEnum>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRefRTTI : public RTTIType<ManagedSerializableTypeInfoRef, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoRefRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(MType, 0)
			BS_RTTI_MEMBER_PLAIN(MTypeName, 1)
			BS_RTTI_MEMBER_PLAIN(MTypeNamespace, 2)
			BS_RTTI_MEMBER_PLAIN(MRtiiTypeId, 3)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoRefRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoRef";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRRefRTTI : public RTTIType<ManagedSerializableTypeInfoRRef, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoRRefRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(MResourceType, 0)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoRRefRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoRRef";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoRRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoRRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoObjectRTTI : public RTTIType<ManagedSerializableTypeInfoObject, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoObjectRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(MTypeName, 0)
			BS_RTTI_MEMBER_PLAIN(MTypeNamespace, 1)
			BS_RTTI_MEMBER_PLAIN(MValueType, 2)
			BS_RTTI_MEMBER_PLAIN(MTypeId, 4)
			BS_RTTI_MEMBER_PLAIN(MFlags, 5)
			BS_RTTI_MEMBER_PLAIN(MRtiiTypeId, 6)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoObjectRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoObject";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoObject>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoArrayRTTI : public RTTIType<ManagedSerializableTypeInfoArray, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoArrayRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(MElementType, 0)
			BS_RTTI_MEMBER_PLAIN(MRank, 1)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoArrayRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoArray";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoArray>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoListRTTI : public RTTIType<ManagedSerializableTypeInfoList, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoListRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(MElementType, 0)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoListRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoList";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoList;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoList>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoDictionaryRTTI : public RTTIType<ManagedSerializableTypeInfoDictionary, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoDictionaryRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(MKeyType, 0)
			BS_RTTI_MEMBER_REFLPTR(MValueType, 1)
		BS_END_RTTI_MEMBERS

	public:
		ManagedSerializableTypeInfoDictionaryRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableTypeInfoDictionary";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializableTypeInfoDictionary;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableTypeInfoDictionary>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
