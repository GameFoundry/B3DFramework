//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
#include "BsManagedSerializableField.h"
#include "BsException.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableFieldKeyRTTI : public RTTIType<ManagedSerializableFieldKey, IReflectable, ManagedSerializableFieldKeyRTTI>
	{
	private:
		UINT16& getTypeId(ManagedSerializableFieldKey* obj) { return obj->mTypeId; }
		void setTypeId(ManagedSerializableFieldKey* obj, UINT16& val) { obj->mTypeId = val; }

		UINT16& getFieldId(ManagedSerializableFieldKey* obj) { return obj->mFieldId; }
		void setFieldId(ManagedSerializableFieldKey* obj, UINT16& val) { obj->mFieldId = val; }

	public:
		ManagedSerializableFieldKeyRTTI()
		{
			addPlainField("mTypeId", 0, &ManagedSerializableFieldKeyRTTI::getTypeId, &ManagedSerializableFieldKeyRTTI::setTypeId);
			addPlainField("mFieldId", 1, &ManagedSerializableFieldKeyRTTI::getFieldId, &ManagedSerializableFieldKeyRTTI::setFieldId);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldKey";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldKey;
		}

		SPtr<IReflectable> newRTTIObject() override
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

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InvalidStateException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataEntryRTTI : public RTTIType<ManagedSerializableFieldDataEntry, IReflectable, ManagedSerializableFieldDataEntryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldKey> getKey(ManagedSerializableFieldDataEntry* obj)	{ return obj->mKey; }
		void setKey(ManagedSerializableFieldDataEntry* obj, SPtr<ManagedSerializableFieldKey> val) { obj->mKey = val; }

		SPtr<ManagedSerializableFieldData> getValue(ManagedSerializableFieldDataEntry* obj) { return obj->mValue; }
		void setValue(ManagedSerializableFieldDataEntry* obj, SPtr<ManagedSerializableFieldData> val) { obj->mValue = val; }

	public:
		ManagedSerializableFieldDataEntryRTTI()
		{
			addReflectablePtrField("mKey", 0, &ManagedSerializableFieldDataEntryRTTI::getKey, &ManagedSerializableFieldDataEntryRTTI::setKey);
			addReflectablePtrField("mValue", 1, &ManagedSerializableFieldDataEntryRTTI::getValue, &ManagedSerializableFieldDataEntryRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataEntry";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataEntry;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataEntry>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataBoolRTTI : public RTTIType<ManagedSerializableFieldDataBool, ManagedSerializableFieldData, ManagedSerializableFieldDataBoolRTTI>
	{
	private:
		bool& getValue(ManagedSerializableFieldDataBool* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataBool* obj, bool& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataBoolRTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataBoolRTTI::getValue, &ManagedSerializableFieldDataBoolRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataBool";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataBool;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataBool>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataCharRTTI : public RTTIType<ManagedSerializableFieldDataChar, ManagedSerializableFieldData, ManagedSerializableFieldDataCharRTTI>
	{
	private:
		wchar_t& getValue(ManagedSerializableFieldDataChar* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataChar* obj, wchar_t& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataCharRTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataCharRTTI::getValue, &ManagedSerializableFieldDataCharRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataChar";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataChar;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataBool>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI8RTTI : public RTTIType<ManagedSerializableFieldDataI8, ManagedSerializableFieldData, ManagedSerializableFieldDataI8RTTI>
	{
	private:
		INT8& getValue(ManagedSerializableFieldDataI8* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataI8* obj, INT8& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataI8RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataI8RTTI::getValue, &ManagedSerializableFieldDataI8RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataI8";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataI8;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI8>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU8RTTI : public RTTIType<ManagedSerializableFieldDataU8, ManagedSerializableFieldData, ManagedSerializableFieldDataU8RTTI>
	{
	private:
		UINT8& getValue(ManagedSerializableFieldDataU8* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataU8* obj, UINT8& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataU8RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataU8RTTI::getValue, &ManagedSerializableFieldDataU8RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataU8";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataU8;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU8>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI16RTTI : public RTTIType<ManagedSerializableFieldDataI16, ManagedSerializableFieldData, ManagedSerializableFieldDataI16RTTI>
	{
	private:
		INT16& getValue(ManagedSerializableFieldDataI16* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataI16* obj, INT16& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataI16RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataI16RTTI::getValue, &ManagedSerializableFieldDataI16RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataI16";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataI16;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI16>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU16RTTI : public RTTIType<ManagedSerializableFieldDataU16, ManagedSerializableFieldData, ManagedSerializableFieldDataU16RTTI>
	{
	private:
		UINT16& getValue(ManagedSerializableFieldDataU16* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataU16* obj, UINT16& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataU16RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataU16RTTI::getValue, &ManagedSerializableFieldDataU16RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataU16";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataU16;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU16>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI32RTTI : public RTTIType<ManagedSerializableFieldDataI32, ManagedSerializableFieldData, ManagedSerializableFieldDataI32RTTI>
	{
	private:
		INT32& getValue(ManagedSerializableFieldDataI32* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataI32* obj, INT32& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataI32RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataI32RTTI::getValue, &ManagedSerializableFieldDataI32RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataI32";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataI32;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI32>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU32RTTI : public RTTIType<ManagedSerializableFieldDataU32, ManagedSerializableFieldData, ManagedSerializableFieldDataU32RTTI>
	{
	private:
		UINT32& getValue(ManagedSerializableFieldDataU32* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataU32* obj, UINT32& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataU32RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataU32RTTI::getValue, &ManagedSerializableFieldDataU32RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataU32";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataU32;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU32>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI64RTTI : public RTTIType<ManagedSerializableFieldDataI64, ManagedSerializableFieldData, ManagedSerializableFieldDataI64RTTI>
	{
	private:
		INT64& getValue(ManagedSerializableFieldDataI64* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataI64* obj, INT64& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataI64RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataI64RTTI::getValue, &ManagedSerializableFieldDataI64RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataI64";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataI64;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataI64>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU64RTTI : public RTTIType<ManagedSerializableFieldDataU64, ManagedSerializableFieldData, ManagedSerializableFieldDataU64RTTI>
	{
	private:
		UINT64& getValue(ManagedSerializableFieldDataU64* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataU64* obj, UINT64& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataU64RTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataU64RTTI::getValue, &ManagedSerializableFieldDataU64RTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataU64";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataU64;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataU64>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataFloatRTTI : public RTTIType<ManagedSerializableFieldDataFloat, ManagedSerializableFieldData, ManagedSerializableFieldDataFloatRTTI>
	{
	private:
		float& getValue(ManagedSerializableFieldDataFloat* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataFloat* obj, float& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataFloatRTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataFloatRTTI::getValue, &ManagedSerializableFieldDataFloatRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataFloat";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataFloat;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataFloat>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataDoubleRTTI : public RTTIType<ManagedSerializableFieldDataDouble, ManagedSerializableFieldData, ManagedSerializableFieldDataDoubleRTTI>
	{
	private:
		double& getValue(ManagedSerializableFieldDataDouble* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataDouble* obj, double& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataDoubleRTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataDoubleRTTI::getValue, &ManagedSerializableFieldDataDoubleRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataDouble";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataDouble;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataDouble>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataStringRTTI : public RTTIType<ManagedSerializableFieldDataString, ManagedSerializableFieldData, ManagedSerializableFieldDataStringRTTI>
	{
	private:
		WString& getValue(ManagedSerializableFieldDataString* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataString* obj, WString& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataStringRTTI()
		{
			addPlainField("mValue", 0, &ManagedSerializableFieldDataStringRTTI::getValue, &ManagedSerializableFieldDataStringRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataString";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataString;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataString>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataResourceRefRTTI : public RTTIType<ManagedSerializableFieldDataResourceRef, ManagedSerializableFieldData, ManagedSerializableFieldDataResourceRefRTTI>
	{
	private:
		HResource& getValue(ManagedSerializableFieldDataResourceRef* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataResourceRef* obj, HResource& val) { obj->value = HResource(val); }

	public:
		ManagedSerializableFieldDataResourceRefRTTI()
		{
			addReflectableField("mValue", 0, &ManagedSerializableFieldDataResourceRefRTTI::getValue, &ManagedSerializableFieldDataResourceRefRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataResourceRef";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataResourceRef;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataResourceRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataGameObjectRefRTTI : public RTTIType<ManagedSerializableFieldDataGameObjectRef, ManagedSerializableFieldData, ManagedSerializableFieldDataGameObjectRefRTTI>
	{
	private:
		HGameObject& getValue(ManagedSerializableFieldDataGameObjectRef* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataGameObjectRef* obj, HGameObject& val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataGameObjectRefRTTI()
		{
			addReflectableField("mValue", 0, &ManagedSerializableFieldDataGameObjectRefRTTI::getValue, &ManagedSerializableFieldDataGameObjectRefRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataGameObjectRef";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataGameObjectRef;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataGameObjectRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataObjectRTTI : public RTTIType<ManagedSerializableFieldDataObject, ManagedSerializableFieldData, ManagedSerializableFieldDataObjectRTTI>
	{
	private:
		SPtr<ManagedSerializableObject> getValue(ManagedSerializableFieldDataObject* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataObject* obj, SPtr<ManagedSerializableObject> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataObjectRTTI()
		{
			addReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataObjectRTTI::getValue, &ManagedSerializableFieldDataObjectRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataObject>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataArrayRTTI : public RTTIType<ManagedSerializableFieldDataArray, ManagedSerializableFieldData, ManagedSerializableFieldDataArrayRTTI>
	{
	private:
		SPtr<ManagedSerializableArray> getValue(ManagedSerializableFieldDataArray* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataArray* obj, SPtr<ManagedSerializableArray> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataArrayRTTI()
		{
			addReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataArrayRTTI::getValue, &ManagedSerializableFieldDataArrayRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataArray";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataArray;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataArray>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataListRTTI : public RTTIType<ManagedSerializableFieldDataList, ManagedSerializableFieldData, ManagedSerializableFieldDataListRTTI>
	{
	private:
		SPtr<ManagedSerializableList> getValue(ManagedSerializableFieldDataList* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataList* obj, SPtr<ManagedSerializableList> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataListRTTI()
		{
			addReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataListRTTI::getValue, &ManagedSerializableFieldDataListRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataList";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataList;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataList>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataDictionaryRTTI : public RTTIType<ManagedSerializableFieldDataDictionary, ManagedSerializableFieldData, ManagedSerializableFieldDataDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableDictionary> getValue(ManagedSerializableFieldDataDictionary* obj) { return obj->value; }
		void setValue(ManagedSerializableFieldDataDictionary* obj, SPtr<ManagedSerializableDictionary> val) { obj->value = val; }

	public:
		ManagedSerializableFieldDataDictionaryRTTI()
		{
			addReflectablePtrField("mValue", 0, &ManagedSerializableFieldDataDictionaryRTTI::getValue, &ManagedSerializableFieldDataDictionaryRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializableFieldDataDictionary";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldDataDictionary;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldDataDictionary>();
		}
	};

	/** @} */
	/** @endcond */
}