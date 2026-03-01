//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "Serialization/B3DManagedSerializableField.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldKeyRTTI : public TRTTIType<ManagedSerializableFieldKey, IReflectable, ManagedSerializableFieldKeyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(MTypeId, 0)
			B3D_RTTI_MEMBER(MFieldId, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "SerializableFieldKey";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldKey;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedSerializableFieldKey>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataRTTI : public TRTTIType<ManagedSerializableFieldData, IReflectable, ManagedSerializableFieldDataRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "SerializableFieldData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldData;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataEntryRTTI : public TRTTIType<ManagedSerializableFieldDataEntry, IReflectable, ManagedSerializableFieldDataEntryRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(MKey, 0)
			B3D_RTTI_MEMBER(MValue, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataEntry";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataEntry>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataBoolRTTI : public TRTTIType<ManagedSerializableFieldDataBool, ManagedSerializableFieldData, ManagedSerializableFieldDataBoolRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataBool";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataBool;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataBool>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataCharRTTI : public TRTTIType<ManagedSerializableFieldDataChar, ManagedSerializableFieldData, ManagedSerializableFieldDataCharRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value32, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(ManagedSerializableFieldDataChar& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.Value = (wchar_t)object.Value32;
			}
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataChar";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataChar;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataChar>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI8RTTI : public TRTTIType<ManagedSerializableFieldDataI8, ManagedSerializableFieldData, ManagedSerializableFieldDataI8RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataI8";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataI8;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataI8>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU8RTTI : public TRTTIType<ManagedSerializableFieldDataU8, ManagedSerializableFieldData, ManagedSerializableFieldDataU8RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataU8";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataU8;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataU8>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI16RTTI : public TRTTIType<ManagedSerializableFieldDataI16, ManagedSerializableFieldData, ManagedSerializableFieldDataI16RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataI16";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataI16;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataI16>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU16RTTI : public TRTTIType<ManagedSerializableFieldDataU16, ManagedSerializableFieldData, ManagedSerializableFieldDataU16RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataU16";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataU16;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataU16>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI32RTTI : public TRTTIType<ManagedSerializableFieldDataI32, ManagedSerializableFieldData, ManagedSerializableFieldDataI32RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataI32";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataI32;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataI32>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU32RTTI : public TRTTIType<ManagedSerializableFieldDataU32, ManagedSerializableFieldData, ManagedSerializableFieldDataU32RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataU32";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataU32;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataU32>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI64RTTI : public TRTTIType<ManagedSerializableFieldDataI64, ManagedSerializableFieldData, ManagedSerializableFieldDataI64RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataI64";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataI64;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataI64>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU64RTTI : public TRTTIType<ManagedSerializableFieldDataU64, ManagedSerializableFieldData, ManagedSerializableFieldDataU64RTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataU64";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataU64;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataU64>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataFloatRTTI : public TRTTIType<ManagedSerializableFieldDataFloat, ManagedSerializableFieldData, ManagedSerializableFieldDataFloatRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataFloat";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataFloat;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataFloat>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataDoubleRTTI : public TRTTIType<ManagedSerializableFieldDataDouble, ManagedSerializableFieldData, ManagedSerializableFieldDataDoubleRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataDouble";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataDouble;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataDouble>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataStringRTTI : public TRTTIType<ManagedSerializableFieldDataString, ManagedSerializableFieldData, ManagedSerializableFieldDataStringRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value32, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(ManagedSerializableFieldDataString& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.Value = WString(object.Value32.size(), '0');
				for(size_t i = 0; i < object.Value32.size(); ++i)
					object.Value[i] = (wchar_t)object.Value32[i];
			}
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataString";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataString;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataString>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataResourceRefRTTI : public TRTTIType<ManagedSerializableFieldDataResourceRef, ManagedSerializableFieldData, ManagedSerializableFieldDataResourceRefRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataResourceRef";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataResourceRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataResourceRef>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataGameObjectRefRTTI : public TRTTIType<ManagedSerializableFieldDataGameObjectRef, ManagedSerializableFieldData, ManagedSerializableFieldDataGameObjectRefRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataGameObjectRef";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataGameObjectRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataGameObjectRef>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataReflectableRefRTTI : public TRTTIType<ManagedSerializableFieldDataReflectableRef, ManagedSerializableFieldData, ManagedSerializableFieldDataReflectableRefRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataReflectableRef";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataReflectableRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataReflectableRef>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataObjectRTTI : public TRTTIType<ManagedSerializableFieldDataObject, ManagedSerializableFieldData, ManagedSerializableFieldDataObjectRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataObject>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataArrayRTTI : public TRTTIType<ManagedSerializableFieldDataArray, ManagedSerializableFieldData, ManagedSerializableFieldDataArrayRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataArray";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataArray>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataListRTTI : public TRTTIType<ManagedSerializableFieldDataList, ManagedSerializableFieldData, ManagedSerializableFieldDataListRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataList";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataList;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataList>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataDictionaryRTTI : public TRTTIType<ManagedSerializableFieldDataDictionary, ManagedSerializableFieldData, ManagedSerializableFieldDataDictionaryRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataDictionary";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializableFieldDataDictionary;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableFieldDataDictionary>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
