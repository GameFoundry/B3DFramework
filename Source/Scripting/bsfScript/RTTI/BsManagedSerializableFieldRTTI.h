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

	class BS_SCR_BE_EXPORT ManagedSerializableFieldKeyRTTI : public RTTIType<ManagedSerializableFieldKey, IReflectable, ManagedSerializableFieldKeyRTTI>
	{
	private:
		UINT16& GetTypeId(ManagedSerializableFieldKey* obj) { return obj->mTypeId; }
		void SetTypeId(ManagedSerializableFieldKey* obj, UINT16& val) { obj->mTypeId = val; }

		UINT16& GetFieldId(ManagedSerializableFieldKey* obj) { return obj->mFieldId; }
		void SetFieldId(ManagedSerializableFieldKey* obj, UINT16& val) { obj->mFieldId = val; }

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

		UINT32 GetRttiId() 
		{
			return TID_SerializableFieldKey;
		}

		SPtr<IReflectable> NewRttiObject() 
		{
			return bs_shared_ptr_new<ManagedSerializableFieldKey>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataRTTI : public RTTIType<ManagedSerializableFieldData, IReflectable, ManagedSerializableFieldDataRTTI>
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

		UINT32 GetRttiId() 
		{
			return TID_SerializableFieldData;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			BS_EXCEPT(InvalidStateException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataEntryRTTI : public RTTIType<ManagedSerializableFieldDataEntry, IReflectable, ManagedSerializableFieldDataEntryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldKey> GetKey(ManagedSerializableFieldDataEntry* obj)	{ return obj->mKey; }
		void SetKey(ManagedSerializableFieldDataEntry* obj, SPtr<ManagedSerializableFieldKey> val) { obj->mKey = val; }

		SPtr<ManagedSerializableFieldData> GetValue(ManagedSerializableFieldDataEntry* obj) { return obj->mValue; }
		void SetValue(ManagedSerializableFieldDataEntry* obj, SPtr<ManagedSerializableFieldData> val) { obj->mValue = val; }

	public:
		ManagedSerializableFieldDataEntryRTTI()
		{
			AddReflectablePtrField("mKey", 0, &ManagedSerializableFieldDataEntryRTTI::getKey, &ManagedSerializableFieldDataEntryRTTI::setKey);
			AddReflectablePtrField("mValue", 1, &ManagedSerializableFieldDataEntryRTTI::getValue, &ManagedSerializableFieldDataEntryRTTI::setValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataEntry";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataEntry>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataBoolRTTI : public RTTIType<ManagedSerializableFieldDataBool, ManagedSerializableFieldData, ManagedSerializableFieldDataBoolRTTI>
	{
	private:
		bool& GetValue(ManagedSerializableFieldDataBool* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataBool* obj, bool& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataBool;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataBool>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataCharRTTI : public RTTIType<ManagedSerializableFieldDataChar, ManagedSerializableFieldData, ManagedSerializableFieldDataCharRTTI>
	{
	private:
		wchar_t& GetValue(ManagedSerializableFieldDataChar* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataChar* obj, wchar_t& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataCharRTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataCharRTTI::GetValue, &ManagedSerializableFieldDataCharRTTI::SetValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataChar";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataChar;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataBool>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI8RTTI : public RTTIType<ManagedSerializableFieldDataI8, ManagedSerializableFieldData, ManagedSerializableFieldDataI8RTTI>
	{
	private:
		INT8& GetValue(ManagedSerializableFieldDataI8* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataI8* obj, INT8& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataI8;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI8>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU8RTTI : public RTTIType<ManagedSerializableFieldDataU8, ManagedSerializableFieldData, ManagedSerializableFieldDataU8RTTI>
	{
	private:
		UINT8& GetValue(ManagedSerializableFieldDataU8* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataU8* obj, UINT8& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataU8;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU8>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI16RTTI : public RTTIType<ManagedSerializableFieldDataI16, ManagedSerializableFieldData, ManagedSerializableFieldDataI16RTTI>
	{
	private:
		INT16& GetValue(ManagedSerializableFieldDataI16* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataI16* obj, INT16& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataI16;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI16>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU16RTTI : public RTTIType<ManagedSerializableFieldDataU16, ManagedSerializableFieldData, ManagedSerializableFieldDataU16RTTI>
	{
	private:
		UINT16& GetValue(ManagedSerializableFieldDataU16* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataU16* obj, UINT16& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataU16;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU16>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI32RTTI : public RTTIType<ManagedSerializableFieldDataI32, ManagedSerializableFieldData, ManagedSerializableFieldDataI32RTTI>
	{
	private:
		INT32& GetValue(ManagedSerializableFieldDataI32* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataI32* obj, INT32& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataI32;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI32>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU32RTTI : public RTTIType<ManagedSerializableFieldDataU32, ManagedSerializableFieldData, ManagedSerializableFieldDataU32RTTI>
	{
	private:
		UINT32& GetValue(ManagedSerializableFieldDataU32* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataU32* obj, UINT32& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataU32;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU32>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI64RTTI : public RTTIType<ManagedSerializableFieldDataI64, ManagedSerializableFieldData, ManagedSerializableFieldDataI64RTTI>
	{
	private:
		INT64& GetValue(ManagedSerializableFieldDataI64* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataI64* obj, INT64& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataI64;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI64>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU64RTTI : public RTTIType<ManagedSerializableFieldDataU64, ManagedSerializableFieldData, ManagedSerializableFieldDataU64RTTI>
	{
	private:
		UINT64& GetValue(ManagedSerializableFieldDataU64* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataU64* obj, UINT64& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataU64;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU64>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataFloatRTTI : public RTTIType<ManagedSerializableFieldDataFloat, ManagedSerializableFieldData, ManagedSerializableFieldDataFloatRTTI>
	{
	private:
		float& GetValue(ManagedSerializableFieldDataFloat* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataFloat* obj, float& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataFloat;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataFloat>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataDoubleRTTI : public RTTIType<ManagedSerializableFieldDataDouble, ManagedSerializableFieldData, ManagedSerializableFieldDataDoubleRTTI>
	{
	private:
		double& GetValue(ManagedSerializableFieldDataDouble* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataDouble* obj, double& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataDouble;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataDouble>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataStringRTTI : public RTTIType<ManagedSerializableFieldDataString, ManagedSerializableFieldData, ManagedSerializableFieldDataStringRTTI>
	{
	private:
		WString& GetValue(ManagedSerializableFieldDataString* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataString* obj, WString& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataStringRTTI()
		{
			AddPlainField("mValue", 0, &ManagedSerializableFieldDataStringRTTI::GetValue, &ManagedSerializableFieldDataStringRTTI::SetValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataString";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataString;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataString>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataResourceRefRTTI : public RTTIType<ManagedSerializableFieldDataResourceRef, ManagedSerializableFieldData, ManagedSerializableFieldDataResourceRefRTTI>
	{
	private:
		HResource& GetValue(ManagedSerializableFieldDataResourceRef* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataResourceRef* obj, HResource& val) { obj->value = HResource(val); }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataResourceRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataResourceRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataGameObjectRefRTTI : public RTTIType<ManagedSerializableFieldDataGameObjectRef, ManagedSerializableFieldData, ManagedSerializableFieldDataGameObjectRefRTTI>
	{
	private:
		HGameObject& GetValue(ManagedSerializableFieldDataGameObjectRef* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataGameObjectRef* obj, HGameObject& val) { obj->value = val; }

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

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataGameObjectRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataGameObjectRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataReflectableRefRTTI : public RTTIType<ManagedSerializableFieldDataReflectableRef, ManagedSerializableFieldData, ManagedSerializableFieldDataReflectableRefRTTI>
	{
	private:
		SPtr<IReflectable> GetValue(ManagedSerializableFieldDataReflectableRef* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataReflectableRef* obj, SPtr<IReflectable> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataReflectableRefRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataReflectableRefRTTI::getValue, &ManagedSerializableFieldDataReflectableRefRTTI::setValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataReflectableRef";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataReflectableRef;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataReflectableRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataObjectRTTI : public RTTIType<ManagedSerializableFieldDataObject, ManagedSerializableFieldData, ManagedSerializableFieldDataObjectRTTI>
	{
	private:
		SPtr<ManagedSerializableObject> GetValue(ManagedSerializableFieldDataObject* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataObject* obj, SPtr<ManagedSerializableObject> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataObjectRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataObjectRTTI::getValue, &ManagedSerializableFieldDataObjectRTTI::setValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataObject";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataObject>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataArrayRTTI : public RTTIType<ManagedSerializableFieldDataArray, ManagedSerializableFieldData, ManagedSerializableFieldDataArrayRTTI>
	{
	private:
		SPtr<ManagedSerializableArray> GetValue(ManagedSerializableFieldDataArray* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataArray* obj, SPtr<ManagedSerializableArray> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataArrayRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataArrayRTTI::getValue, &ManagedSerializableFieldDataArrayRTTI::setValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataArray";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataArray>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataListRTTI : public RTTIType<ManagedSerializableFieldDataList, ManagedSerializableFieldData, ManagedSerializableFieldDataListRTTI>
	{
	private:
		SPtr<ManagedSerializableList> GetValue(ManagedSerializableFieldDataList* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataList* obj, SPtr<ManagedSerializableList> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataListRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataListRTTI::getValue, &ManagedSerializableFieldDataListRTTI::setValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataList";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataList;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataList>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataDictionaryRTTI : public RTTIType<ManagedSerializableFieldDataDictionary, ManagedSerializableFieldData, ManagedSerializableFieldDataDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableDictionary> GetValue(ManagedSerializableFieldDataDictionary* obj) { return obj->value; }
		void SetValue(ManagedSerializableFieldDataDictionary* obj, SPtr<ManagedSerializableDictionary> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataDictionaryRTTI()
		{
			AddReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataDictionaryRTTI::getValue, &ManagedSerializableFieldDataDictionaryRTTI::setValue);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializableFieldDataDictionary";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_SerializableFieldDataDictionary;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataDictionary>();
		}
	};

	/** @} */
	/** @endcond */
}
