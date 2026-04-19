//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "B3DUtilityPrerequisites.h"
#include "Serialization/B3DSerializedObject.h"
#include "B3DRTTIIterator.h"

namespace b3d::RTTIObjectWrapper
{
	/** Helper class that wraps either an IReflectable or a SerializedObject object instance, and allows iteration over all of its fields and values. */
	template <bool IsIReflectable>
	class Object {};

	/**
	 * Helper class that provides information about a specific RTTIType of an either IReflectable or a SerializedObject object instance.
	 * This is relevant for types that inherit from other reflectable types.
	 */
	template <bool IsIReflectable>
	class SubObject {};

	/** Helper class that iterates over all base RTTI types in a specific RTTI type. */
	template <bool IsIReflectable>
	struct SubObjectIterator {};

	/**
	 * Helper class that wraps a single RTTI field, that can be read either from a backing IReflectable or SerializedObject object instance.
	 */
	template <bool IsIReflectable>
	struct Field {};

	/**
	 * Represents a value contained by a field (multiple such values can be held by fields referencing a container).
	 * Will contain different data depending on the field type. Data is read either from backing IReflectable or
	 * SerializedObject instance.
	 */
	template <bool IsIReflectable>
	struct Value {};

	/** Helper class that iterates over all fields in a RTTI type. */
	template <bool IsIReflectable>
	struct FieldIterator {};

	/** Helper class that iterates over all values in a RTTI field. */
	template <bool IsIReflectable>
	struct ValueIterator {};

	/** Provides information about a specific RTTIType of an object backed by SerializedObject. */
	template <>
	class SubObject<false>
	{
	public:
		SubObject() = default;
		SubObject(SerializedObject* object, u32 subObjectIndex, FrameAllocator* frameAllocator);

		/** Returns the type ID of the RTTIType. */
		u32 GetTypeId() const;

		/** Returns an iterator that will iterate over all fields in the RTTIType. */
		FieldIterator<false> GetFieldIterator() const;

	private:
		friend class Object<false>;

		SerializedObject* mObject = nullptr;
		u32 mSubObjectIndex = 0;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Provides information about a specific RTTIType of an object backed by IReflectable. */
	template <>
	class SubObject<true>
	{
	public:
		SubObject() = default;
		SubObject(IReflectable* object, RTTIType* rttiType, FrameAllocator* frameAllocator);

		/** Returns the type ID of the RTTIType. */
		u32 GetTypeId() const;

		/** Returns an iterator that will iterate over all fields in the RTTIType. */
		FieldIterator<true> GetFieldIterator() const;

	private:
		friend class Object<true>;

		IReflectable* mObject = nullptr;
		RTTIType* mRTTIType = nullptr;
		RTTIType* mRTTITypeInstance = nullptr;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Wraps a SerializedObject and allows you to retrieve information about its types. */
	template <>
	class Object<false>
	{
	public:
		Object() = default;
		Object(SerializedObject* object, FrameAllocator* frameAllocator);

		/** Returns the type ID of the root RTTIType. */
		u32 GetTypeId() const;

		/** Returns an iterator that will iterate over all the RTTITypes of the object. */
		SubObjectIterator<false> GetSubObjectIterator() const;

		/** Returns the raw pointer to the underlying wrapped object. */
		IReflectable* GetWrappedObject() const { return mObject; }

		/**
		 * Notifies the RTTI object that we're about to begin an operation on the sub-object fields. Should be called
		 * before writing or reading any field values from the sub-object. Must be called once for each sub-object.
		 */
		void NotifyBeginOperation(SubObject<false>& subObject, RTTIOperationType operationType, RTTIOperationContext& context);

		/**
		 * Notifies the RTTI object that we have completed an operation on its fields. Must be called after
		 * NotifyBeginOperation(), after we have finished reading or writing field values from the object.
		 * Should be called just once, after all field operations for all sub-objects end.
		 */
		void NotifyEndOperation(RTTIOperationType operationType, RTTIOperationContext& context);

	private:
		SerializedObject* mObject = nullptr;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Wraps a SerializedObject and allows you to retrieve information about its types. */
	template <>
	class Object<true>
	{
	public:
		Object() = default;
		Object(IReflectable* object, RTTIType* rttiType, FrameAllocator* frameAllocator);

		/** Returns the type ID of the root RTTIType. */
		u32 GetTypeId() const;

		/** Returns an iterator that will iterate over all the RTTITypes of the object. */
		SubObjectIterator<true> GetSubObjectIterator() const;

		/** Returns the raw pointer to the underlying wrapped object. */
		IReflectable* GetWrappedObject() const { return mObject; }

		/**
		 * Notifies the RTTI object that we're about to begin an operation on the sub-object fields. Should be called
		 * before writing or reading any field values from the sub-object. Must be called once for each sub-object.
		 */
		void NotifyBeginOperation(SubObject<true>& subObject, RTTIOperationType operationType, RTTIOperationContext& context);

		/**
		 * Notifies the RTTI object that we have completed an operation on its fields. Must be called after
		 * NotifyBeginOperation(), after we have finished reading or writing field values from the object.
		 * Should be called just once, after all field operations for all sub-objects end.
		 */
		void NotifyEndOperation(RTTIOperationType operationType, RTTIOperationContext& context);

	private:
		IReflectable* mObject = nullptr;
		RTTIType* mRTTIType = nullptr;
		FrameVector<RTTIType*> mRTTITypeInstances;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Iterates over base RTTITypes of some type. */
	template <>
	struct SubObjectIterator<false>
	{
		SubObjectIterator(SerializedObject* object, FrameAllocator* frameAllocator);

		/**
		 * Moves to the next base type and return false if no type was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. MoveNext() must previously be called and return true. */
		SubObject<false> GetValue() const;

	private:
		SerializedObject* mObject = nullptr;
		u32 mCurrentSubObjectIndex = ~0u;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Iterates over base RTTITypes of some type. */
	template <>
	struct SubObjectIterator<true>
	{
		SubObjectIterator(RTTIType* rttiType, IReflectable* object, FrameAllocator* frameAllocator);

		/**
		 * Moves to the next base type and return false if no type was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. MoveNext() must previously be called and return true. */
		SubObject<true> GetValue() const;

	private:
		IReflectable* mObject = nullptr;
		RTTIType* mRTTIType = nullptr;
		RTTIType* mCurrentRTTIType = nullptr;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Wraps a single RTTIField and allows you to retrieve field data. */
	template <>
	struct Field<false>
	{
	public:
		Field() = default;
		Field(u32 fieldId, const SPtr<ISerialized>& value, FrameAllocator* frameAllocator);

		/** Returns the unique identifier of the field within a RTTIType. */
		u32 GetId() const;

		/** Returns an iterator that can iterate over all values in a field. */
		ValueIterator<false> GetValueIterator() const;

		/** Clones the contents of this field and returns them as intermediate serialized data. */
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const;

	private:
		friend struct Field<true>;

		u32 mId = 0;
		SPtr<ISerialized> mValue;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Wraps a single RTTIField and allows you to retrieve field data. */
	template <>
	struct Field<true>
	{
	public:
		Field() = default;
		Field(RTTIType* rttiType, RTTIField* field, IReflectable* object, FrameAllocator* frameAllocator);

		/** Returns the unique identifier of the field within a RTTIType. */
		u32 GetId() const;

		/** Returns an iterator that can iterate over all values in a field. */
		ValueIterator<true> GetValueIterator() const;

		/** Clones the contents of this field and returns them as intermediate serialized data. */
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const;

	private:
		friend struct Field<false>;

		RTTIType* mRTTITypeInstance = nullptr;
		RTTIField* mField = nullptr;
		IReflectable* mObject = nullptr;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Iterates over all fields in a RTTIType. */
	template <>
	struct FieldIterator<false>
	{
		FieldIterator(SerializedObject* value, u32 subObjectIndex, FrameAllocator* allocator);

		/**
		 * Moves to the next field and return false if no field was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. MoveNext() must previously be called and return true. */
		Field<false> GetValue() const;

	private:
		SerializedObject* mValue = nullptr;
		u32 mSubObjectIndex = ~0u;
		RTTIType* mRTTIType = nullptr;
		UnorderedMap<u32, SerializedField>::iterator mFieldIterator;
		bool mIsIteratorSet = false;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Iterates over all fields in a RTTIType. */
	template <>
	struct FieldIterator<true>
	{
		FieldIterator(RTTIType* rttiType, RTTIType* rttiTypeInstance, IReflectable* value, FrameAllocator* allocator);

		/**
		 * Moves to the next field and return false if no field was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. moveNext() must previously be called and return true. */
		Field<true> GetValue() const;

	private:
		IReflectable* mValue = nullptr;
		RTTIType* mRTTIType = nullptr;
		RTTIType* mRTTITypeInstance = nullptr;
		u32 mFieldIndex = ~0u;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Iterates over all values in a RTTI field. */
	template <>
	struct ValueIterator<false>
	{
		ValueIterator(const SPtr<ISerialized>& value, FrameAllocator* allocator);

		/**
		 * Moves to the next value and return false if no value was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. MoveNext() must previously be called and return true. */
		Value<false> GetValue() const;

		/** Returns the number of elements to be iterated over. */
		u32 GetElementCount() const;

		/**
		 * Attempts to find a value in the iterator that matches current value of the provided iterator. The iterators must have been created
		 * from the same field type, otherwise behaviour is undefined.
		 */
		TOptional<Value<false>> FindMatchingValue(const ValueIterator<false>& otherIterator) const;

		/**
		 * Attempts to find a value in the iterator that matches current value of the provided iterator. The iterators must have been created
		 * from the same field type, otherwise behaviour is undefined.
		 */
		TOptional<Value<false>> FindMatchingValue(const ValueIterator<true>& otherIterator) const;

	private:
		friend struct ValueIterator<true>;

		u64 mArrayIndex = 0;
		UnorderedMap<SPtr<ISerialized>, SPtr<ISerialized>>::iterator mMapIterator;
		bool mIsIteratorSet = false;
		SPtr<SerializedArray> mArrayContainerValue;
		SPtr<SerializedMap> mMapContainerValue;
		SPtr<ISerialized> mValue;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Iterates over all values in a RTTI field. */
	template <>
	struct ValueIterator<true>
	{
		ValueIterator(RTTIField* field, RTTIType* rttiType, IReflectable* object, const SPtr<IRTTIIterator>& iterator, FrameAllocator* allocator);
		ValueIterator(RTTIField* field, RTTIType* rttiType, IReflectable* object, u32 elementCount, FrameAllocator* allocator);

		/**
		 * Moves to the next value and return false if no value was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. MoveNext() must previously be called and return true. */
		Value<true> GetValue() const;

		/** Returns the number of elements to be iterated over. */
		u32 GetElementCount() const;

		/**
		 * Attempts to find a value in the iterator that matches current value of the provided iterator. The iterators must have been created
		 * from the same field type, otherwise behaviour is undefined.
		 */
		TOptional<Value<true>> FindMatchingValue(const ValueIterator<true>& otherIterator) const;

		/**
		 * Attempts to find a value in the iterator that matches current value of the provided iterator. The iterators must have been created
		 * from the same field type, otherwise behaviour is undefined.
		 */
		TOptional<Value<true>> FindMatchingValue(const ValueIterator<false>& otherIterator) const;

	private:
		friend struct ValueIterator<false>;

		SPtr<IRTTIIterator> mIterator; /**< Iterator in case the field is an iterator field. */
		bool mIsIteratorSet = false; /**< True if the iterator has been advanced to the first element. */

		u32 mElementIndex = 0;
		u32 mElementCount = 0;

		IReflectable* mObject = nullptr;
		RTTIType* mRTTITypeInstance = nullptr;
		RTTIField* mField = nullptr;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/**
	 * Represents a value contained by a field (multiple such values can be held by fields referencing a container).
	 * Will contain different data depending on the field type. 
	 */
	template <>
	struct Value<false>
	{
	public:
		Value() = default;
		Value(u32 tupleElementIndex, const SPtr<ISerialized>& value, FrameAllocator* allocator);

		/** If the value represents a tuple (e.g. std::pair<K, V>), represents the index within the tuple. */
		u32 GetTupleElementIndex() const { return mTupleElementIndex; }

		/** Returns number of elements contained in a tuple. Returns 1 if value doesn't represent a tuple type. */
		u32 GetTupleElementCount() const;

		/** Returns an element within the tuple at the specified index. Only valid if the field points to a tuple. */
		Value<false> GetTupleElement(u32 tupleElementIndex) const;

		/**
		 * Returns a wrapper that holds the object held by the field. Only valid if the field points to
		 * a reflectable type (pointer or otherwise).
		 */
		Object<false> GetObject() const;

		/** Returns a data stream held by the field. Only valid if the field is a data block field. */
		SPtr<DataStream> GetDataStream(u32& size, u32& offset) const;

		/** Returns the size of the plain data in a field, in bytes. Only valid if the field holds a plain type. */
		u32 GetPlainSize() const;

		/** Compares the data between two plain fields and returns true if they're equal. */
		bool ComparePlain(const Value<false>& other) const;

		/** Compares the data between two plain fields and returns true if they're equal. */
		bool ComparePlain(const Value<true>& other) const;

		/** Clones the contents of this value and returns them as intermediate serialized data. */
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const;

	private:
		friend struct Value<true>;

		u32 mTupleElementIndex = 0;
		SPtr<ISerialized> mValue;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/**
	 * Represents a value contained by a field (multiple such values can be held by fields referencing a container).
	 * Will contain different data depending on the field type. 
	 */
	template <>
	struct Value<true>
	{
	public:
		Value() = default;
		Value(RTTIField* field, u32 tupleElementIndex, const SPtr<IRTTIIterator>& iterator, RTTIType* rttiTypeInstance, IReflectable* object, FrameAllocator* allocator);
		Value(RTTIField* field, u32 tupleElementIndex, u32 arrayIndex, RTTIType* rttiTypeInstance, IReflectable* object, FrameAllocator* allocator);

		/** If the value represents a tuple (e.g. std::pair<K, V>), represents the index within the tuple. */
		u32 GetTupleElementIndex() const { return mTupleElementIndex; }

		/** Returns number of elements contained in a tuple. Returns 1 if value doesn't represent a tuple type. */
		u32 GetTupleElementCount() const;

		/** Returns an element within the tuple at the specified index. Only valid if the field points to a tuple. */
		Value<true> GetTupleElement(u32 tupleIndex) const;

		/**
		 * Returns a wrapper that holds the object held by the field. Only valid if the field points to
		 * a reflectable type (pointer or otherwise).
		 */
		Object<true> GetObject() const;

		/** Returns a data stream held by the field. Only valid if the field is a data block field. */
		SPtr<DataStream> GetDataStream(u32& size, u32& offset) const;

		/** Returns the size of the plain data in a field, in bytes. Only valid if the field holds a plain type. */
		u32 GetPlainSize() const;

		/**
		 * Writes the data contained in the field into @p buffer. Caller must allocate the buffer and ensure it is of
		 * adequate size. Buffer size in bytes must be provided as @p bufferSize. Only valid if the field holds a plain
		 * type.
		 */
		void GetPlainData(u8* buffer, u32 bufferSize) const;

		/** Compares the data between two plain fields and returns true if they're equal. */
		bool ComparePlain(const Value<false>& other) const;

		/** Compares the data between two plain fields and returns true if they're equal. */
		bool ComparePlain(const Value<true>& other) const;

		/** Clones the contents of this value and returns them as intermediate serialized data. */
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const;

	private:
		friend struct Value<false>;

		u32 mTupleElementIndex = 0;
		SPtr<IRTTIIterator> mIterator;
		u32 mArrayIndex = ~0u;

		IReflectable* mObject = nullptr;
		RTTIType* mRTTITypeInstance = nullptr;
		RTTIField* mField = nullptr;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/**
	 * Iterates over all fields in the provided object and triggers @p fnPredicate. This includes fields from all base classes.
	 *
	 * Predicate signature must be void(const RTTIField& rttiField, Field<IsIReflectable>& field).
	 */
	template<bool IsIReflectable, typename Predicate>
	void IterateFields(Object<IsIReflectable> object, RTTIOperationType operationType, Predicate&& fnPredicate);

	/**
	 * Iterates over all field values in the provided object and triggers @p fnPredicate. This will trigger
	 * once for each non-container field, and once for each entry in container (e.g. array, map) fields.
	 *
	 * Predicate signature must be void(const RTTIFieldSchema& fieldSchema, Value<IsIReflectable>& value).
	 */
	template<bool IsIReflectable, typename Predicate, typename FieldFilterPredicate>
	void IterateFieldValues(Object<IsIReflectable> object, RTTIOperationType operationType, Predicate&& fnPredicate, FieldFilterPredicate&& fnFieldFilterPredicate = nullptr);

	/**
	 * Iterates over all field value tuple entries in the provided object and triggers @p fnPredicate. This is similar
	 * to IterateFieldValues(), but in case the value is tuple (e.g. std::pair<K,  V>), this will trigger once for each
	 * tuple element. If not a tuple, behaviour is identical to IterateFieldValues().
	 *
	 * Predicate signature must be void(const RTTIFieldTypeSchema& fieldTypeSchema, Value<IsIReflectable>& value).
	 */
	template<bool IsIReflectable, typename Predicate, typename FieldFilterPredicate>
	void IterateFieldTupleValues(Object<IsIReflectable> object, RTTIOperationType operationType, Predicate&& fnPredicate, FieldFilterPredicate&& fnFieldFilterPredicate = nullptr);
}

#include "B3DRTTIObjectWrapper.inl"
