//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "Serialization/BsManagedSerializableField.h"
#include "Error/BsException.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldKeyRTTI : public RTTIType<ManagedSerializableFieldKey, IReflectable, ManagedSerializableFieldKeyRTTI>
	{
	private:
		u16& GetTypeId(ManagedSerializableFieldKey* obj) { return obj->MTypeId; }

		void SetTypeId(ManagedSerializableFieldKey* obj, u16& val) { obj->MTypeId = val; }

		u16& GetFieldId(ManagedSerializableFieldKey* obj) { return obj->MFieldId; }

		void SetFieldId(ManagedSerializableFieldKey* obj, u16& val) { obj->MFieldId = val; }

	public:
		ManagedSerializableFieldKeyRTTI()
		{
			AddPlainField("mTypeId", 0, &ManagedSerializableFieldKeyRTTI::GetTypeId, &ManagedSerializableFieldKeyRTTI::SetTypeId);
			AddPlainField("mFieldId", 1, &ManagedSerializableFieldKeyRTTI::GetFieldId, &ManagedSerializableFieldKeyRTTI::SetFieldId);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataRTTI : public RTTIType<ManagedSerializableFieldData, IReflectable, ManagedSerializableFieldDataRTTI>
	{
	private:
	public:
		ManagedSerializableFieldDataRTTI()
		{
		}

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
			B3D_EXCEPT(InvalidStateException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataEntryRTTI : public RTTIType<ManagedSerializableFieldDataEntry, IReflectable, ManagedSerializableFieldDataEntryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldKey> GetKey(ManagedSerializableFieldDataEntry* obj) { return obj->MKey; }

		void SetKey(ManagedSerializableFieldDataEntry* obj, SPtr<ManagedSerializableFieldKey> val) { obj->MKey = val; }

		SPtr<ManagedSerializableFieldData> GetValue(ManagedSerializableFieldDataEntry* obj) { return obj->MValue; }

		void SetValue(ManagedSerializableFieldDataEntry* obj, SPtr<ManagedSerializableFieldData> val) { obj->MValue = val; }

	public:
		ManagedSerializableFieldDataEntryRTTI()
		{
			AddReflectablePtrField("mKey", 0, &ManagedSerializableFieldDataEntryRTTI::GetKey, &ManagedSerializableFieldDataEntryRTTI::SetKey);
			AddReflectablePtrField("mValue", 1, &ManagedSerializableFieldDataEntryRTTI::GetValue, &ManagedSerializableFieldDataEntryRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataBoolRTTI : public RTTIType<ManagedSerializableFieldDataBool, ManagedSerializableFieldData, ManagedSerializableFieldDataBoolRTTI>
	{
	private:
		bool& GetValue(ManagedSerializableFieldDataBool* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataBool* obj, bool& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataBoolRTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataBoolRTTI::GetValue, &ManagedSerializableFieldDataBoolRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataCharRTTI : public RTTIType<ManagedSerializableFieldDataChar, ManagedSerializableFieldData, ManagedSerializableFieldDataCharRTTI>
	{
	private:
		u32& GetValue(ManagedSerializableFieldDataChar* obj) { return obj->Value32; }
		void SetValue(ManagedSerializableFieldDataChar* obj, u32& val) { obj->Value32 = val; }

	public:
		ManagedSerializableFieldDataCharRTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataCharRTTI::GetValue, &ManagedSerializableFieldDataCharRTTI::SetValue);
		}

		void OnDeserializationEnded(IReflectable* object, RTTIOperationContext* context) override
		{
			ManagedSerializableFieldDataChar* const fieldData = static_cast<ManagedSerializableFieldDataChar*>(object);
			fieldData->Value = (wchar_t)fieldData->Value32;
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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI8RTTI : public RTTIType<ManagedSerializableFieldDataI8, ManagedSerializableFieldData, ManagedSerializableFieldDataI8RTTI>
	{
	private:
		i8& GetValue(ManagedSerializableFieldDataI8* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataI8* obj, i8& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataI8RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataI8RTTI::GetValue, &ManagedSerializableFieldDataI8RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU8RTTI : public RTTIType<ManagedSerializableFieldDataU8, ManagedSerializableFieldData, ManagedSerializableFieldDataU8RTTI>
	{
	private:
		u8& GetValue(ManagedSerializableFieldDataU8* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataU8* obj, u8& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataU8RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataU8RTTI::GetValue, &ManagedSerializableFieldDataU8RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI16RTTI : public RTTIType<ManagedSerializableFieldDataI16, ManagedSerializableFieldData, ManagedSerializableFieldDataI16RTTI>
	{
	private:
		i16& GetValue(ManagedSerializableFieldDataI16* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataI16* obj, i16& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataI16RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataI16RTTI::GetValue, &ManagedSerializableFieldDataI16RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU16RTTI : public RTTIType<ManagedSerializableFieldDataU16, ManagedSerializableFieldData, ManagedSerializableFieldDataU16RTTI>
	{
	private:
		u16& GetValue(ManagedSerializableFieldDataU16* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataU16* obj, u16& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataU16RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataU16RTTI::GetValue, &ManagedSerializableFieldDataU16RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI32RTTI : public RTTIType<ManagedSerializableFieldDataI32, ManagedSerializableFieldData, ManagedSerializableFieldDataI32RTTI>
	{
	private:
		i32& GetValue(ManagedSerializableFieldDataI32* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataI32* obj, i32& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataI32RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataI32RTTI::GetValue, &ManagedSerializableFieldDataI32RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU32RTTI : public RTTIType<ManagedSerializableFieldDataU32, ManagedSerializableFieldData, ManagedSerializableFieldDataU32RTTI>
	{
	private:
		u32& GetValue(ManagedSerializableFieldDataU32* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataU32* obj, u32& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataU32RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataU32RTTI::GetValue, &ManagedSerializableFieldDataU32RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI64RTTI : public RTTIType<ManagedSerializableFieldDataI64, ManagedSerializableFieldData, ManagedSerializableFieldDataI64RTTI>
	{
	private:
		i64& GetValue(ManagedSerializableFieldDataI64* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataI64* obj, i64& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataI64RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataI64RTTI::GetValue, &ManagedSerializableFieldDataI64RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU64RTTI : public RTTIType<ManagedSerializableFieldDataU64, ManagedSerializableFieldData, ManagedSerializableFieldDataU64RTTI>
	{
	private:
		u64& GetValue(ManagedSerializableFieldDataU64* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataU64* obj, u64& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataU64RTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataU64RTTI::GetValue, &ManagedSerializableFieldDataU64RTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataFloatRTTI : public RTTIType<ManagedSerializableFieldDataFloat, ManagedSerializableFieldData, ManagedSerializableFieldDataFloatRTTI>
	{
	private:
		float& GetValue(ManagedSerializableFieldDataFloat* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataFloat* obj, float& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataFloatRTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataFloatRTTI::GetValue, &ManagedSerializableFieldDataFloatRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataDoubleRTTI : public RTTIType<ManagedSerializableFieldDataDouble, ManagedSerializableFieldData, ManagedSerializableFieldDataDoubleRTTI>
	{
	private:
		double& GetValue(ManagedSerializableFieldDataDouble* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataDouble* obj, double& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataDoubleRTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataDoubleRTTI::GetValue, &ManagedSerializableFieldDataDoubleRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataStringRTTI : public RTTIType<ManagedSerializableFieldDataString, ManagedSerializableFieldData, ManagedSerializableFieldDataStringRTTI>
	{
	private:
		U32String& GetValue(ManagedSerializableFieldDataString* obj) { return obj->Value32; }

		void SetValue(ManagedSerializableFieldDataString* obj, U32String& val) { obj->Value32 = val; }

	public:
		ManagedSerializableFieldDataStringRTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataStringRTTI::GetValue, &ManagedSerializableFieldDataStringRTTI::SetValue);
		}

		void OnDeserializationEnded(IReflectable* object, RTTIOperationContext* context) override
		{
			ManagedSerializableFieldDataString* const fieldData = static_cast<ManagedSerializableFieldDataString*>(object);

			fieldData->Value = WString(fieldData->Value32.size(), '0');
			for(size_t i = 0; i < fieldData->Value32.size(); ++i)
				fieldData->Value[i] = (wchar_t)fieldData->Value32[i];
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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataResourceRefRTTI : public RTTIType<ManagedSerializableFieldDataResourceRef, ManagedSerializableFieldData, ManagedSerializableFieldDataResourceRefRTTI>
	{
	private:
		HResource& GetValue(ManagedSerializableFieldDataResourceRef* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataResourceRef* obj, HResource& val) { obj->Value = HResource(val); }

	public:
		ManagedSerializableFieldDataResourceRefRTTI()
		{
			AddReflectableField("mValue", 0, &ManagedSerializableFieldDataResourceRefRTTI::GetValue, &ManagedSerializableFieldDataResourceRefRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataGameObjectRefRTTI : public RTTIType<ManagedSerializableFieldDataGameObjectRef, ManagedSerializableFieldData, ManagedSerializableFieldDataGameObjectRefRTTI>
	{
	private:
		HGameObject& GetValue(ManagedSerializableFieldDataGameObjectRef* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataGameObjectRef* obj, HGameObject& val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataGameObjectRefRTTI()
		{
			AddReflectableField("mValue", 0, &ManagedSerializableFieldDataGameObjectRefRTTI::GetValue, &ManagedSerializableFieldDataGameObjectRefRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataReflectableRefRTTI : public RTTIType<ManagedSerializableFieldDataReflectableRef, ManagedSerializableFieldData, ManagedSerializableFieldDataReflectableRefRTTI>
	{
	private:
		SPtr<IReflectable> GetValue(ManagedSerializableFieldDataReflectableRef* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataReflectableRef* obj, SPtr<IReflectable> val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataReflectableRefRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataReflectableRefRTTI::GetValue, &ManagedSerializableFieldDataReflectableRefRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataObjectRTTI : public RTTIType<ManagedSerializableFieldDataObject, ManagedSerializableFieldData, ManagedSerializableFieldDataObjectRTTI>
	{
	private:
		SPtr<ManagedSerializableObject> GetValue(ManagedSerializableFieldDataObject* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataObject* obj, SPtr<ManagedSerializableObject> val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataObjectRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataObjectRTTI::GetValue, &ManagedSerializableFieldDataObjectRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataArrayRTTI : public RTTIType<ManagedSerializableFieldDataArray, ManagedSerializableFieldData, ManagedSerializableFieldDataArrayRTTI>
	{
	private:
		SPtr<ManagedSerializableArray> GetValue(ManagedSerializableFieldDataArray* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataArray* obj, SPtr<ManagedSerializableArray> val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataArrayRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataArrayRTTI::GetValue, &ManagedSerializableFieldDataArrayRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataListRTTI : public RTTIType<ManagedSerializableFieldDataList, ManagedSerializableFieldData, ManagedSerializableFieldDataListRTTI>
	{
	private:
		SPtr<ManagedSerializableList> GetValue(ManagedSerializableFieldDataList* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataList* obj, SPtr<ManagedSerializableList> val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataListRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataListRTTI::GetValue, &ManagedSerializableFieldDataListRTTI::SetValue);
		}

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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataDictionaryRTTI : public RTTIType<ManagedSerializableFieldDataDictionary, ManagedSerializableFieldData, ManagedSerializableFieldDataDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableDictionary> GetValue(ManagedSerializableFieldDataDictionary* obj) { return obj->Value; }

		void SetValue(ManagedSerializableFieldDataDictionary* obj, SPtr<ManagedSerializableDictionary> val) { obj->Value = val; }

	public:
		ManagedSerializableFieldDataDictionaryRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataDictionaryRTTI::GetValue, &ManagedSerializableFieldDataDictionaryRTTI::SetValue);
		}

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
} // namespace bs
