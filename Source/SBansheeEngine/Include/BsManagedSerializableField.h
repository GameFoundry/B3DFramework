//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsIReflectable.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**
	 * Contains data that can be used for identifying a field in an object when cross referenced with the object type.
	 *
	 * @note	
	 * Essentially a light-weight identifier for the field so that we don't need to store entire field type for each field
	 * when serializing. Instead field types are stored separately and we just use this object for lookup.
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldKey : public IReflectable
	{
	public:
		ManagedSerializableFieldKey();
		ManagedSerializableFieldKey(UINT16 typeId, UINT16 fieldId);

		/**
		 * Creates a new field key.
		 *
		 * @param[in]	typeId	Unique ID of the parent type the field belongs to. See ManagedSerializableTypeInfoObject.
		 * @param[in]	fieldId	Unique ID of the field within its parent class. See ManagedSerializableObjectInfo.
		 */
		static SPtr<ManagedSerializableFieldKey> create(UINT16 typeId, UINT16 fieldId);

		UINT16 mTypeId;
		UINT16 mFieldId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ScriptSerializableFieldDataKeyRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains value of a single field in a managed object. This class can contain any data type and should be overridden
	 * for specific types.
	 *
	 * Stored values can be serialized and stored for later use, and deserialized back to managed objects when needed. You
	 * must call serialize() before performing RTTI serialization. After field data has been serialized you should not call
	 * any methods on it before calling deserialize().
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldData : public IReflectable
	{
	public:
		virtual ~ManagedSerializableFieldData() { }

		/**
		 * Creates a new data wrapper for some field data.
		 *
		 * @param[in]	typeInfo	Type of the data we're storing.
		 * @param[in]	value		Managed boxed value to store in the field. Value will be copied into the internal buffer
		 *							and stored.
		 */
		static SPtr<ManagedSerializableFieldData> create(const SPtr<ManagedSerializableTypeInfo>& typeInfo, MonoObject* value);

		/**
		 * Creates a new data wrapper containing default instance of the provided type.
		 *
		 * @param[in]	typeInfo	Type of the data we're storing.
		 */
		static SPtr<ManagedSerializableFieldData> createDefault(const SPtr<ManagedSerializableTypeInfo>& typeInfo);

		/**
		 * Returns the internal value.
		 *
		 * @param[in]	typeInfo	Type of the data we're looking to retrieve. This isn't required for actually retrieving
		 *							the data but is used as an extra check to ensure the field contains the data type we're
		 *							looking for.
		 * @return					Pointer to the internal serialized data. Caller must ensure the pointer is cast to the
		 *							proper type.
		 */
		virtual void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) = 0;

		/**
		 * Boxes the internal value and returns it.
		 *
		 * @param[in]	typeInfo	Type of the data we're looking to retrieve. This isn't required for actually retrieving
		 *							the data but is used as an extra check to ensure the field contains the data type we're
		 *							looking for.
		 * @return					Boxed representation of the internal value.
		 */
		virtual MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) = 0;

		/**
		 * Checks if the internal value stored in this object matches the value stored in another. Does shallow comparison
		 * for complex objects.
		 */
		virtual bool equals(const SPtr<ManagedSerializableFieldData>& other) = 0;

		/**	Returns a hash value for the internally stored value. */
		virtual size_t getHash() = 0;

		/**
		 * Serializes the internal value so that it may be stored and deserialized later.
		 *
		 * @note	
		 * This is generally only relevant for complex objects, as primitive types have their values copied and serialized
		 * automatically whenever field data is created.
		 */
		virtual void serialize() { }

		/**	Deserializes the internal value so that the managed instance can be retrieved. */
		virtual void deserialize() { }

	private:
		/**
		 * Creates a new data wrapper for some field data.
		 *
		 * @param[in]	typeInfo	Type of the data we're storing.
		 * @param[in]	value		Managed boxed value to store in the field. Value will be copied into the internal buffer
		 *							and stored.
		 * @param[in]	allowNull	Determines should null values be allowed. If false the objects with null values will
		 *							instead be instantiated to their default values.
		 */
		static SPtr<ManagedSerializableFieldData> create(const SPtr<ManagedSerializableTypeInfo>& typeInfo, MonoObject* value, 
			bool allowNull);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains type and value of a single field in an object. */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataEntry : public IReflectable
	{
	public:
		static SPtr<ManagedSerializableFieldDataEntry> create(const SPtr<ManagedSerializableFieldKey>& key, const SPtr<ManagedSerializableFieldData>& value);

		SPtr<ManagedSerializableFieldKey> mKey;
		SPtr<ManagedSerializableFieldData> mValue;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataEntryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains boolean field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataBool : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataBool() { }

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		bool value = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataBoolRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains wide character field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataChar : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataChar() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		wchar_t value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataCharRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains signed 8-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */	
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI8 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI8() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		INT8 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI8RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains unsigned 8-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU8 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU8() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		UINT8 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU8RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains signed 16-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI16 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI16() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		INT16 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI16RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains unsigned 16-bit field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU16 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU16() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		UINT16 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU16RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains signed 32-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI32 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI32() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		INT32 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI32RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains unsigned 32-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU32 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU32() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		UINT32 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU32RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains signed 64-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataI64 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI64() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		INT64 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI64RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains unsigned 64-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataU64 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU64() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		UINT64 value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU64RTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains single precision floating point field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataFloat : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataFloat() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		float value = 0.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataFloatRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains double precision floating point field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataDouble : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataDouble() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		double value = 0.0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataDoubleRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains wide character string field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataString : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataString() {}

		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		WString value;
		bool isNull = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataStringRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains resource reference field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataResourceRef : public ManagedSerializableFieldData
	{
	public:
		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		HResource value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataResourceRefRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains game object reference field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataGameObjectRef : public ManagedSerializableFieldData
	{
	public:
		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		HGameObject value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataGameObjectRefRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains complex object field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataObject : public ManagedSerializableFieldData
	{
	public:
		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		/** @copydoc ManagedSerializableFieldData::serialize */
		void serialize() override;

		/** @copydoc ManagedSerializableFieldData::deserialize */
		void deserialize() override;

		SPtr<ManagedSerializableObject> value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains array field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataArray : public ManagedSerializableFieldData
	{
	public:
		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		/** @copydoc ManagedSerializableFieldData::serialize */
		void serialize() override;

		/** @copydoc ManagedSerializableFieldData::deserialize */
		void deserialize() override;

		SPtr<ManagedSerializableArray> value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataArrayRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains list field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataList : public ManagedSerializableFieldData
	{
	public:
		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		/** @copydoc ManagedSerializableFieldData::serialize */
		void serialize() override;

		/** @copydoc ManagedSerializableFieldData::deserialize */
		void deserialize() override;

		SPtr<ManagedSerializableList> value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataListRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains dictionary field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldDataDictionary : public ManagedSerializableFieldData
	{
	public:
		/** @copydoc ManagedSerializableFieldData::getValue */
		void* getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::getValueBoxed */
		MonoObject* getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo) override;

		/** @copydoc ManagedSerializableFieldData::equals */
		bool equals(const SPtr<ManagedSerializableFieldData>& other) override;

		/** @copydoc ManagedSerializableFieldData::getHash */
		size_t getHash() override;

		/** @copydoc ManagedSerializableFieldData::serialize */
		void serialize() override;

		/** @copydoc ManagedSerializableFieldData::deserialize */
		void deserialize() override;

		SPtr<ManagedSerializableDictionary> value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataDictionaryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}