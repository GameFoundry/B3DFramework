//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "Serialization/B3DManagedTypeInfo.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedAssemblyInfoRTTI : public TRTTIType<ManagedAssemblyInfo, IReflectable, ManagedAssemblyInfoRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Name, 0)
			B3D_RTTI_MEMBER_CONTAINER(ObjectInfos, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(ManagedAssemblyInfo& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.TypeNameToId.clear();
				for(const auto& pair : object.ObjectInfos)
				{
					if(!B3D_ENSURE(pair.second != nullptr))
						continue;

					B3D_ENSURE(pair.first == pair.second->TypeInfo->TypeId);
					object.TypeNameToId[pair.second->GetFullTypeName()] = pair.second->TypeInfo->TypeId;
				}
			}
		}

		const String& GetRttiName()
		{
			static String name = "ManagedAssemblyInfo";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedAssemblyInfo;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedAssemblyInfo>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedObjectInfoRTTI : public TRTTIType<ManagedObjectInfo, IReflectable, ManagedObjectInfoRTTI>
	{
	private:
		using TRTTIType<ManagedObjectInfo, IReflectable, ManagedObjectInfoRTTI>::GetBaseClass;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(TypeInfo, 0)
			B3D_RTTI_MEMBER(BaseClass, 2)
			B3D_RTTI_MEMBER_CONTAINER(Members, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(ManagedObjectInfo& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.MemberNameToIndex.clear();

				for(u32 memberIndex = 0; memberIndex < (u32)object.Members.size(); ++memberIndex)
				{
					const SPtr<ManagedMemberInfo>& memberInfo = object.Members[memberIndex];
					if(!B3D_ENSURE(memberInfo != nullptr))
						continue;

					object.MemberNameToIndex[memberInfo->Name] = memberIndex;
				}
			}
		}

		const String& GetRttiName() override
		{
			static String name = "ManagedObjectInfo";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedObjectInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedObjectInfo>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedMemberInfoRTTI : public TRTTIType<ManagedMemberInfo, IReflectable, ManagedMemberInfoRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Name, 0)
			B3D_RTTI_MEMBER(TypeInfo, 1)
			B3D_RTTI_MEMBER(FieldId, 2)
			B3D_RTTI_MEMBER(MetaDataFlags, 3)
			B3D_RTTI_MEMBER(ParentTypeId, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedMemberInfo";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedMemberInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			// This is an abstract class, but it wasn't always. For compatibility sake we return an object instance so old
			// data can still be properly read.
			return B3DMakeShared<ManagedFieldInfo>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedFieldInfoRTTI : public TRTTIType<ManagedFieldInfo, ManagedMemberInfo, ManagedFieldInfoRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedFieldInfo";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedFieldInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedFieldInfo>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedPropertyInfoRTTI : public TRTTIType<ManagedPropertyInfo, ManagedMemberInfo, ManagedPropertyInfoRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedPropertyInfo";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedPropertyInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedPropertyInfo>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoRTTI : public TRTTIType<ManagedTypeInfo, IReflectable, ManagedTypeInfoRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfo";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoPrimitiveRTTI : public TRTTIType<ManagedTypeInfoPrimitive, ManagedTypeInfo, ManagedTypeInfoPrimitiveRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(PrimitiveType, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoPrimitive";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoPrimitive;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoPrimitive>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoEnumRTTI : public TRTTIType<ManagedTypeInfoEnum, ManagedTypeInfo, ManagedTypeInfoEnumRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(UnderlyingType, 0)
			B3D_RTTI_MEMBER(TypeNamespace, 1)
			B3D_RTTI_MEMBER(TypeName, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoEnum";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoEnum;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoEnum>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoReferenceRTTI : public TRTTIType<ManagedTypeInfoReference, ManagedTypeInfo, ManagedTypeInfoReferenceRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(ReferenceType, 0)
			B3D_RTTI_MEMBER(TypeName, 1)
			B3D_RTTI_MEMBER(TypeNamespace, 2)
			B3D_RTTI_MEMBER(TypeRTTIId, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoReference";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoReference;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoReference>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoResourceReferenceRTTI : public TRTTIType<ManagedTypeInfoResourceReference, ManagedTypeInfo, ManagedTypeInfoResourceReferenceRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(ResourceType, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoResourceReference";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoResourceReference;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoResourceReference>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoObjectRTTI : public TRTTIType<ManagedTypeInfoObject, ManagedTypeInfo, ManagedTypeInfoObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(TypeName, 0)
			B3D_RTTI_MEMBER(TypeNamespace, 1)
			B3D_RTTI_MEMBER(IsValueType, 2)
			B3D_RTTI_MEMBER(TypeId, 4)
			B3D_RTTI_MEMBER(MetaDataFlags, 5)
			B3D_RTTI_MEMBER(TypeRTTIId, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoObject>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoArrayRTTI : public TRTTIType<ManagedTypeInfoArray, ManagedTypeInfo, ManagedTypeInfoArrayRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(ElementType, 0)
			B3D_RTTI_MEMBER(Rank, 1)
		B3D_RTTI_END_MEMBERS

	public:
		ManagedTypeInfoArrayRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoArray";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoArray>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoListRTTI : public TRTTIType<ManagedTypeInfoList, ManagedTypeInfo, ManagedTypeInfoListRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(ElementType, 0)
		B3D_RTTI_END_MEMBERS

	public:
		ManagedTypeInfoListRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoList";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoList;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoList>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoDictionaryRTTI : public TRTTIType<ManagedTypeInfoDictionary, ManagedTypeInfo, ManagedTypeInfoDictionaryRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(KeyType, 0)
			B3D_RTTI_MEMBER(ValueType, 1)
		B3D_RTTI_END_MEMBERS

	public:
		ManagedTypeInfoDictionaryRTTI()
			: mInitMembers(this)
		{}

		const String& GetRttiName() override
		{
			static String name = "ManagedTypeInfoDictionary";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedTypeInfoDictionary;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedTypeInfoDictionary>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
