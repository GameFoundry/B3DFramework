//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Contains data that can be used for identifying a field in an object when cross referenced with the object type.
	 *
	 * @note
	 * Essentially a light-weight identifier for the field so that we don't need to store entire field type for each field
	 * when serializing. Instead field types are stored separately and we just use this object for lookup.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldKey : public IReflectable
	{
	public:
		ManagedSerializableFieldKey() = default;
		ManagedSerializableFieldKey(u16 typeId, u16 fieldId);

		/**
		 * Creates a new field key.
		 *
		 * @param[in]	typeId	Unique ID of the parent type the field belongs to. See ManagedSerializableTypeInfoObject.
		 * @param[in]	fieldId	Unique ID of the field within its parent class. See ManagedSerializableObjectInfo.
		 */
		static SPtr<ManagedSerializableFieldKey> Create(u16 typeId, u16 fieldId);

		u16 MTypeId = 0;
		u16 MFieldId = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ScriptSerializableFieldDataKeyRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains value of a single field in a managed object. This class can contain any data type and should be overridden
	 * for specific types.
	 *
	 * Stored values can be serialized and stored for later use, and deserialized back to managed objects when needed. You
	 * must call serialize() before performing RTTI serialization. After field data has been serialized you should not call
	 * any methods on it before calling deserialize().
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldData : public IReflectable
	{
	public:
		virtual ~ManagedSerializableFieldData() = default;

		/**
		 * Creates a new data wrapper for some field data.
		 *
		 * @param[in]	typeInfo	Type of the data we're storing.
		 * @param[in]	value		Managed boxed value to store in the field. Value will be copied into the internal buffer
		 *							and stored.
		 */
		static SPtr<ManagedSerializableFieldData> Create(const SPtr<ManagedTypeInfo>& typeInfo, MonoObject* value);

		/**
		 * Creates a new data wrapper containing default instance of the provided type.
		 *
		 * @param[in]	typeInfo	Type of the data we're storing.
		 */
		static SPtr<ManagedSerializableFieldData> CreateDefault(const SPtr<ManagedTypeInfo>& typeInfo);

		/**
		 * Returns the internal value.
		 *
		 * @param[in]	typeInfo	Type of the data we're looking to retrieve. This isn't required for actually retrieving
		 *							the data but is used as an extra check to ensure the field contains the data type we're
		 *							looking for.
		 * @return					Pointer to the internal serialized data. Caller must ensure the pointer is cast to the
		 *							proper type.
		 */
		virtual void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) = 0;

		/**
		 * Boxes the internal value and returns it.
		 *
		 * @param[in]	typeInfo	Type of the data we're looking to retrieve. This isn't required for actually retrieving
		 *							the data but is used as an extra check to ensure the field contains the data type we're
		 *							looking for.
		 * @return					Boxed representation of the internal value.
		 */
		virtual MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) = 0;

		/**
		 * Checks if the internal value stored in this object matches the value stored in another. Does shallow comparison
		 * for complex objects.
		 */
		virtual bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) = 0;

		/**	Returns a hash value for the internally stored value. */
		virtual size_t GetHash() = 0;

		/**
		 * Serializes the internal value so that it may be stored and deserialized later.
		 *
		 * @note
		 * This is generally only relevant for complex objects, as primitive types have their values copied and serialized
		 * automatically whenever field data is created.
		 */
		virtual void Serialize() {}

		/**	Deserializes the internal value so that the managed instance can be retrieved. */
		virtual void Deserialize() {}

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
		static SPtr<ManagedSerializableFieldData> Create(const SPtr<ManagedTypeInfo>& typeInfo, MonoObject* value, bool allowNull);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**	Contains type and value of a single field in an object. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataEntry : public IReflectable
	{
	public:
		static SPtr<ManagedSerializableFieldDataEntry> Create(const SPtr<ManagedSerializableFieldKey>& key, const SPtr<ManagedSerializableFieldData>& value);

		SPtr<ManagedSerializableFieldKey> MKey;
		SPtr<ManagedSerializableFieldData> MValue;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataEntryRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains boolean field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataBool : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataBool() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		bool Value = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataBoolRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains wide character field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataChar : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataChar() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		wchar_t Value = 0;
		u32 Value32 = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataCharRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains signed 8-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI8 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI8() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		i8 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI8RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains unsigned 8-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU8 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU8() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		u8 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU8RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains signed 16-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI16 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI16() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		i16 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI16RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains unsigned 16-bit field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU16 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU16() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		u16 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU16RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains signed 32-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI32 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI32() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		i32 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI32RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains unsigned 32-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU32 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU32() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		u32 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU32RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains signed 64-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataI64 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataI64() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		i64 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataI64RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Contains unsigned 64-bit integer field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataU64 : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataU64() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		u64 Value = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataU64RTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains single precision floating point field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataFloat : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataFloat() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		float Value = 0.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataFloatRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains double precision floating point field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataDouble : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataDouble() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		double Value = 0.0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataDoubleRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains wide character string field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataString : public ManagedSerializableFieldData
	{
	public:
		ManagedSerializableFieldDataString() = default;

		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		WString Value;
		U32String Value32;
		bool IsNull = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataStringRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains resource reference field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataResourceRef : public ManagedSerializableFieldData
	{
	public:
		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		HResource Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataResourceRefRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains game object reference field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataGameObjectRef : public ManagedSerializableFieldData
	{
	public:
		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		HGameObject Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataGameObjectRefRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains field data for a native object implementing IReflectable.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataReflectableRef : public ManagedSerializableFieldData
	{
	public:
		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;

		SPtr<IReflectable> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldReflectableRefRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains complex object field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataObject : public ManagedSerializableFieldData
	{
	public:
		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;
		void Serialize() override;
		void Deserialize() override;

		SPtr<ManagedSerializableObject> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains array field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataArray : public ManagedSerializableFieldData
	{
	public:
		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;
		void Serialize() override;
		void Deserialize() override;

		SPtr<ManagedSerializableArray> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataArrayRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains list field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataList : public ManagedSerializableFieldData
	{
	public:
		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;
		void Serialize() override;
		void Deserialize() override;

		SPtr<ManagedSerializableList> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataListRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains dictionary field data.
	 *
	 * @copydoc	ManagedSerializableFieldData
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldDataDictionary : public ManagedSerializableFieldData
	{
	public:
		void* GetValue(const SPtr<ManagedTypeInfo>& typeInfo) override;
		MonoObject* GetValueBoxed(const SPtr<ManagedTypeInfo>& typeInfo) override;
		bool Equals(const SPtr<ManagedSerializableFieldData>& other, RTTIOperationContext* context = nullptr) override;
		size_t GetHash() override;
		void Serialize() override;
		void Deserialize() override;

		SPtr<ManagedSerializableDictionary> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ManagedSerializableFieldDataDictionaryRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
