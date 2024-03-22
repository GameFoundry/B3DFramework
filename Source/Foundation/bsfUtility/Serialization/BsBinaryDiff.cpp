//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsBinaryDiff.h"
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsBinaryCloner.h"
#include "Serialization/BsIntermediateSerializer.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsDataStream.h"

using namespace bs;

namespace bs
{
	class Bitstream;
	struct SerializationContext;
} // namespace bs

namespace RTTIWrapper
{
	/** Helper class that wraps either an IReflectable or a SerializedObject object instance. */
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
		SubObject(IReflectable* object, RTTITypeBase* rttiType, FrameAllocator* frameAllocator);

		/** Returns the type ID of the RTTIType. */
		u32 GetTypeId() const;

		/** Returns an iterator that will iterate over all fields in the RTTIType. */
		FieldIterator<true> GetFieldIterator() const;

	private:
		IReflectable* mObject = nullptr;
		RTTITypeBase* mRTTIType = nullptr;

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
		Object(IReflectable* object, RTTITypeBase* rttiType, FrameAllocator* frameAllocator);

		/** Returns the type ID of the root RTTIType. */
		u32 GetTypeId() const;

		/** Returns an iterator that will iterate over all the RTTITypes of the object. */
		SubObjectIterator<true> GetSubObjectIterator() const;

		/** Returns the raw pointer to the underlying wrapped object. */
		IReflectable* GetWrappedObject() const { return mObject; }

	private:
		IReflectable* mObject = nullptr;
		RTTITypeBase* mRTTIType = nullptr;

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
		SubObjectIterator(RTTITypeBase* rttiType, IReflectable* object, FrameAllocator* frameAllocator);

		/**
		 * Moves to the next base type and return false if no type was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. MoveNext() must previously be called and return true. */
		SubObject<true> GetValue() const;

	private:
		IReflectable* mObject = nullptr;
		RTTITypeBase* mRTTIType = nullptr;
		RTTITypeBase* mCurrentRTTIType = nullptr;

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
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const;

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
		Field(RTTITypeBase* rttiType, RTTIField* field, IReflectable* object, FrameAllocator* frameAllocator);

		/** Returns the unique identifier of the field within a RTTIType. */
		u32 GetId() const;

		/** Returns an iterator that can iterate over all values in a field. */
		ValueIterator<true> GetValueIterator() const;

		/** Clones the contents of this field and returns them as intermediate serialized data. */
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const;

	private:
		friend struct Field<false>;

		RTTITypeBase* mRTTIType = nullptr;
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
		RTTITypeBase* mRTTIType = nullptr;
		UnorderedMap<u32, SerializedField>::iterator mFieldIterator;
		bool mIsIteratorSet = false;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	/** Iterates over all fields in a RTTIType. */
	template <>
	struct FieldIterator<true>
	{
		FieldIterator(RTTITypeBase* rttiType, IReflectable* value, FrameAllocator* allocator);

		/**
		 * Moves to the next field and return false if no field was available (end was reached).
		 * Initially in before-start position and must be called once to read the first element.
		 */
		bool MoveNext();

		/** Returns the current value as pointer by the iterator. moveNext() must previously be called and return true. */
		Field<true> GetValue() const;

	private:
		IReflectable* mValue = nullptr;
		RTTITypeBase* mRTTIType = nullptr;
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
		Optional<Value<false>> FindMatchingValue(const ValueIterator<false>& otherIterator) const;

		/**
		 * Attempts to find a value in the iterator that matches current value of the provided iterator. The iterators must have been created
		 * from the same field type, otherwise behaviour is undefined.
		 */
		Optional<Value<false>> FindMatchingValue(const ValueIterator<true>& otherIterator) const;

	private:
		friend struct ValueIterator<true>;

		UnorderedMap<u32, SerializedArrayEntry>::iterator mArrayIterator;
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
		ValueIterator(RTTIField* field, RTTITypeBase* rttiType, IReflectable* object, const SPtr<IRTTIIterator>& iterator, FrameAllocator* allocator);
		ValueIterator(RTTIField* field, RTTITypeBase* rttiType, IReflectable* object, u32 elementCount, FrameAllocator* allocator);

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
		Optional<Value<true>> FindMatchingValue(const ValueIterator<true>& otherIterator) const;

		/**
		 * Attempts to find a value in the iterator that matches current value of the provided iterator. The iterators must have been created
		 * from the same field type, otherwise behaviour is undefined.
		 */
		Optional<Value<true>> FindMatchingValue(const ValueIterator<false>& otherIterator) const;

	private:
		friend struct ValueIterator<false>;

		SPtr<IRTTIIterator> mIterator; /**< Iterator in case the field is an iterator field. */
		bool mIsIteratorSet = false; /**< True if the iterator has been advanced to the first element. */

		u32 mElementIndex = 0;
		u32 mElementCount = 0;

		IReflectable* mObject = nullptr;
		RTTITypeBase* mRTTIType = nullptr;
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
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const;

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
		Value(RTTIField* field, u32 tupleElementIndex, const SPtr<IRTTIIterator>& iterator, RTTITypeBase* rttiType, IReflectable* object, FrameAllocator* allocator);
		Value(RTTIField* field, u32 tupleElementIndex, u32 arrayIndex, RTTITypeBase* rttiType, IReflectable* object, FrameAllocator* allocator);

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
		SPtr<ISerialized> Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const;

	private:
		friend struct Value<false>;

		u32 mTupleElementIndex = 0;
		SPtr<IRTTIIterator> mIterator;
		u32 mArrayIndex = ~0u;

		IReflectable* mObject = nullptr;
		RTTITypeBase* mRTTIType = nullptr;
		RTTIField* mField = nullptr;

		FrameAllocator* mFrameAllocator = nullptr;
	};

	Object<false>::Object(SerializedObject* object, FrameAllocator* frameAllocator)
		: mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 Object<false>::GetTypeId() const
	{
		return mObject->GetRootTypeId();
	}

	SubObjectIterator<false> Object<false>::GetSubObjectIterator() const
	{
		return SubObjectIterator<false>(mObject, mFrameAllocator);
	}

	Object<true>::Object(IReflectable* object, RTTITypeBase* type, FrameAllocator* frameAllocator)
		: mObject(object), mRTTIType(type), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(type != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 Object<true>::GetTypeId() const
	{
		return mObject->GetTypeId();
	}

	SubObjectIterator<true> Object<true>::GetSubObjectIterator() const
	{
		return SubObjectIterator<true>(mRTTIType, mObject, mFrameAllocator);
	}

	SubObject<false>::SubObject(SerializedObject* object, u32 subObjectIndex, FrameAllocator* frameAllocator)
		: mObject(object), mSubObjectIndex(subObjectIndex), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 SubObject<false>::GetTypeId() const
	{
		return mObject->SubObjects[mSubObjectIndex].TypeId;
	}

	FieldIterator<false> SubObject<false>::GetFieldIterator() const
	{
		return FieldIterator<false>(mObject, mSubObjectIndex, mFrameAllocator);
	}

	SubObject<true>::SubObject(IReflectable* object, RTTITypeBase* type, FrameAllocator* frameAllocator)
		: mObject(object), mRTTIType(type), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(type != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 SubObject<true>::GetTypeId() const
	{
		return mRTTIType->GetRttiId();
	}

	FieldIterator<true> SubObject<true>::GetFieldIterator() const
	{
		return FieldIterator<true>(mRTTIType, mObject, mFrameAllocator);
	}

	SubObjectIterator<false>::SubObjectIterator(SerializedObject* object, FrameAllocator* frameAllocator)
		: mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	bool SubObjectIterator<false>::MoveNext()
	{
		u32 subObjectCount = (u32)mObject->SubObjects.size();

		if(mCurrentSubObjectIndex == ~0u)
		{
			if(subObjectCount > 0)
			{
				mCurrentSubObjectIndex = 0;
				return true;
			}

			return false;
		}

		if((mCurrentSubObjectIndex + 1) < subObjectCount)
		{
			mCurrentSubObjectIndex++;
			return true;
		}

		return false;
	}

	SubObject<false> SubObjectIterator<false>::GetValue() const
	{
		return SubObject<false>(mObject, mCurrentSubObjectIndex, mFrameAllocator);
	}

	SubObjectIterator<true>::SubObjectIterator(RTTITypeBase* rttiType, IReflectable* object, FrameAllocator* frameAllocator)
		: mObject(object), mRTTIType(rttiType), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	bool SubObjectIterator<true>::MoveNext()
	{
		if(!mCurrentRTTIType)
		{
			mCurrentRTTIType = mRTTIType;
			return true;
		}
		else
		{
			mCurrentRTTIType = mCurrentRTTIType->GetBaseClass();
			return mCurrentRTTIType != nullptr;
		}
	}

	SubObject<true> SubObjectIterator<true>::GetValue() const
	{
		return SubObject<true>(mObject, mCurrentRTTIType, mFrameAllocator);
	}

	Field<false>::Field(u32 fieldId, const SPtr<ISerialized>& value, FrameAllocator* frameAllocator)
		: mId(fieldId), mValue(value), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 Field<false>::GetId() const
	{
		return mId;
	}

	ValueIterator<false> Field<false>::GetValueIterator() const
	{
		return ValueIterator<false>(mValue, mFrameAllocator);
	}

	SPtr<ISerialized> Field<false>::Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const
	{
		return mValue != nullptr ? mValue->Clone() : nullptr;
	}

	Field<true>::Field(RTTITypeBase* rttiType, RTTIField* field, IReflectable* object, FrameAllocator* frameAllocator)
		: mRTTIType(rttiType), mField(field), mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 Field<true>::GetId() const
	{
		return mField->Schema.Id;
	}

	ValueIterator<true> Field<true>::GetValueIterator() const
	{
		if(mField->Schema.IsIterator)
		{
			auto* const field = static_cast<RTTIIteratorField*>(mField);
			const SPtr<IRTTIIterator> iterator = field->GetIterator(mRTTIType, mObject, *mFrameAllocator);

			return ValueIterator<true>(mField, mRTTIType, mObject, iterator, mFrameAllocator);
		}
		else
		{
			if(mField->Schema.IsArray)
			{
				const u32 arraySize = mField->GetArraySize(mRTTIType, mObject);
				return ValueIterator<true>(mField, mRTTIType, mObject, arraySize, mFrameAllocator);
			}
			else
				return ValueIterator<true>(mField, mRTTIType, mObject, ~0u, mFrameAllocator);
		}
	}

	SPtr<ISerialized> Field<true>::Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const
	{
		if(mField->Schema.IsIterator)
		{
			auto* const field = static_cast<RTTIIteratorField*>(mField);
			return IntermediateSerializer::SerializeField(*mObject, *mRTTIType, *field, flags, context, *mFrameAllocator);
		}

		return IntermediateSerializer::SerializeField(mObject, mRTTIType, mField, ~0u, flags, context, mFrameAllocator);
	}

	ValueIterator<false>::ValueIterator(const SPtr<ISerialized>& value, FrameAllocator* frameAllocator)
		:mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(frameAllocator != nullptr);

		if(const auto& array = B3DRTTICast<SerializedArray>(value))
			mArrayContainerValue = array;
		else if(const auto& map = B3DRTTICast<SerializedMap>(value))
			mMapContainerValue = map;
		else
			mValue = value;
	}

	bool ValueIterator<false>::MoveNext()
	{
		if(mArrayContainerValue != nullptr)
		{
			if(!mIsIteratorSet)
			{
				mArrayIterator = mArrayContainerValue->Entries.begin();
				mIsIteratorSet = true;
			}
			else
				++mArrayIterator;

			return mArrayIterator != mArrayContainerValue->Entries.end();
		}
		else if(mMapContainerValue != nullptr)
		{
			if(!mIsIteratorSet)
			{
				mMapIterator = mMapContainerValue->Entries.begin();
				mIsIteratorSet = true;
			}
			else
				++mMapIterator;

			return mMapIterator != mMapContainerValue->Entries.end();
		}
		else
		{
			if(!mIsIteratorSet)
			{
				mIsIteratorSet = true;
				return true;
			}
		}

		return false;
	}

	Value<false> ValueIterator<false>::GetValue() const
	{
		SPtr<ISerialized> value;
		if(mArrayContainerValue != nullptr)
			value = mArrayIterator->second.Value;
		else if(mMapContainerValue != nullptr)
			value = mMapIterator->second;
		else
			value = mValue;

		return Value<false>(~0u, value, mFrameAllocator);
	}

	u32 ValueIterator<false>::GetElementCount() const
	{
		if(mArrayContainerValue != nullptr)
			return mArrayContainerValue->ElementCount;
		else if(mMapContainerValue != nullptr)
			return (u32)mMapContainerValue->Entries.size();

		return 1;
	}

	Optional<Value<false>> ValueIterator<false>::FindMatchingValue(const ValueIterator<false>& otherIterator) const
	{
		if(!B3D_ENSURE(otherIterator.mIsIteratorSet))
			return {};

		if(mArrayContainerValue != nullptr)
		{
			if(!B3D_ENSURE(otherIterator.mArrayContainerValue != nullptr))
				return {};

			auto found = mArrayContainerValue->Entries.find(otherIterator.mArrayIterator->first);
			if(found != mArrayContainerValue->Entries.end())
				return Value<false>(~0u, found->second.Value, mFrameAllocator);

		}
		else if(mMapContainerValue != nullptr)
		{
			if(!B3D_ENSURE(otherIterator.mMapContainerValue != nullptr))
				return {};

			auto found = mMapContainerValue->Entries.find(otherIterator.mMapIterator->first);
			if(found != mMapContainerValue->Entries.end())
				return Value<false>(~0u, found->second, mFrameAllocator);
		}

		// Not a container iterator
		return {};
	}

	Optional<Value<false>> ValueIterator<false>::FindMatchingValue(const ValueIterator<true>& otherIterator) const
	{
		B3D_ASSERT(false); // Not supported at the moment. To support, we'd need to convert the otherIterator value to ISerialized object that we can lookup.
		return {};
	}

	ValueIterator<true>::ValueIterator(RTTIField* field, RTTITypeBase* rttiType, IReflectable* object, const SPtr<IRTTIIterator>& iterator, FrameAllocator* frameAllocator)
		: mIterator(iterator), mObject(object), mRTTIType(rttiType), mField(field), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(iterator != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	ValueIterator<true>::ValueIterator(RTTIField* field, RTTITypeBase* rttiType, IReflectable* object, u32 elementCount, FrameAllocator* frameAllocator)
		: mElementCount(elementCount), mObject(object), mRTTIType(rttiType), mField(field), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	bool ValueIterator<true>::MoveNext()
	{
		if(!mIsIteratorSet)
		{
			mIsIteratorSet = true;
			mElementIndex = 0;

			return true;
		}
		else
		{
			mElementIndex++;

			if(mIterator != nullptr)
			{
				mIterator->Increment();
				return mIterator->IsValid();
			}
			else
			{
				if(mElementCount == ~0u)
					return mElementIndex == 0;
				else
					return mElementIndex < mElementCount;
			}
		}
	}

	Value<true> ValueIterator<true>::GetValue() const
	{
		if(mIterator != nullptr)
			return Value<true>(mField, ~0u, mIterator, mRTTIType, mObject, mFrameAllocator);

		if(mElementCount != ~0u)
			return Value<true>(mField, ~0u, mElementIndex, mRTTIType, mObject, mFrameAllocator);

		return Value<true>(mField, ~0u, ~0u, mRTTIType, mObject, mFrameAllocator);
	}

	u32 ValueIterator<true>::GetElementCount() const
	{
		if(mIterator != nullptr)
			return (u32)mIterator->GetElementCount();

		if(mElementCount != ~0u)
			return mElementCount;

		return 1;
	}

	Optional<Value<true>> ValueIterator<true>::FindMatchingValue(const ValueIterator<true>& otherIterator) const
	{
		B3D_ASSERT(mField == otherIterator.mField);

		if(mIterator != nullptr)
		{
			if(!B3D_ENSURE(otherIterator.mIterator != nullptr))
				return {};

			if(!B3D_ENSURE(otherIterator.mIsIteratorSet))
				return {};

			const auto& field = *static_cast<RTTIIteratorField*>(mField);
			if(field.IteratorSupportsSeekToKey())
			{
				SPtr<IRTTIIterator> iteratorCopy = mIterator->Clone(*mFrameAllocator);

				const void* fieldValue = field.GetIteratorValue(mRTTIType, mObject, *mFrameAllocator, *mIterator);

				if(!iteratorCopy->SeekToKey(fieldValue))
					return {};
				
				return Value<true>(mField, ~0u, iteratorCopy, mRTTIType, mObject, mFrameAllocator);
			}
			else if(field.IteratorSupportsSeekToIndex())
			{
				SPtr<IRTTIIterator> iteratorCopy = mIterator->Clone(*mFrameAllocator);
				if(!iteratorCopy->SeekToIndex(otherIterator.mElementIndex))
					return {};
				
				return Value<true>(mField, ~0u, iteratorCopy, mRTTIType, mObject, mFrameAllocator);
			}
		}
		else // DEPRECATED
		{
			if(mElementCount == ~0u)
				return {};

			if(otherIterator.mElementIndex >= mElementCount)
				return {};

			return Value<true>(mField, ~0u, otherIterator.mElementIndex, mRTTIType, mObject, mFrameAllocator);
		}

		return {};
	}

	Optional<Value<true>> ValueIterator<true>::FindMatchingValue(const ValueIterator<false>& otherIterator) const
	{
		B3D_ASSERT(false); // Not supported at the moment. To support, we'd need to convert the otherIterator from a ISerialized object into fieldValue pointer we can lookup
		return {};
	}

	Value<false>::Value(u32 tupleElementIndex, const SPtr<ISerialized>& value, FrameAllocator* frameAllocator)
		: mTupleElementIndex(tupleElementIndex), mValue(value), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 Value<false>::GetTupleElementCount() const
	{
		if(auto tuple = B3DRTTICast<SerializedTuple>(mValue))
			return (u32)tuple->Values.Size();
		
		return 1;
	}

	Value<false> Value<false>::GetTupleElement(u32 tupleElementIndex) const
	{
		SPtr<ISerialized> value;
		if(auto tuple = B3DRTTICast<SerializedTuple>(mValue))
		{
			if(B3D_ENSURE(tuple->Values.Size() > tupleElementIndex))
				value = tuple->Values[tupleElementIndex];
			else
				value = nullptr;
		}
		else
		{
			if(!B3D_ENSURE(tupleElementIndex == 0))
				value = nullptr;
		}

		return Value<false>(tupleElementIndex, value, mFrameAllocator);
	}

	Object<false> Value<false>::GetObject() const
	{
		return Object<false>(static_cast<SerializedObject*>(mValue.get()), mFrameAllocator);
	}

	SPtr<DataStream> Value<false>::GetDataStream(u32& size, u32& offset) const
	{
		auto* field = static_cast<SerializedDataBlock*>(mValue.get());
		size = field->Size;
		offset = field->Offset;

		return field->Stream;
	}

	u32 Value<false>::GetPlainSize() const
	{
		auto* field = static_cast<SerializedPlainData*>(mValue.get());
		return field->Size;
	}

	bool Value<false>::ComparePlain(const Value<false>& other) const
	{
		auto* curFieldData = static_cast<SerializedPlainData*>(mValue.get());
		auto* otherFieldData = static_cast<SerializedPlainData*>(other.mValue.get());

		bool isModified = curFieldData->Size != otherFieldData->Size;
		if(!isModified)
			isModified = memcmp(curFieldData->Value, otherFieldData->Value, otherFieldData->Size) != 0;

		return isModified;
	}

	bool Value<false>::ComparePlain(const Value<true>& other) const
	{
		auto* curFieldData = static_cast<SerializedPlainData*>(mValue.get());

		u32 otherTypeSize = other.GetPlainSize();

		auto buffer = B3DStackAllocate<u8>(otherTypeSize);
		other.GetPlainData(buffer, otherTypeSize);

		bool isModified = curFieldData->Size != otherTypeSize;
		if(!isModified)
			isModified = memcmp(curFieldData->Value, buffer, otherTypeSize) != 0;

		return isModified;
	}

	SPtr<ISerialized> Value<false>::Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const
	{
		return mValue->Clone();
	}

	Value<true>::Value(RTTIField* field, u32 tupleElementIndex, const SPtr<IRTTIIterator>& iterator, RTTITypeBase* rttiType, IReflectable* object, FrameAllocator* frameAllocator)
		: mField(field), mTupleElementIndex(tupleElementIndex), mIterator(iterator), mRTTIType(rttiType), mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(iterator != nullptr);
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	Value<true>::Value(RTTIField* field, u32 tupleElementIndex, u32 arrayIndex, RTTITypeBase* rttiType, IReflectable* object, FrameAllocator* frameAllocator)
		: mField(field), mTupleElementIndex(tupleElementIndex), mArrayIndex(arrayIndex), mRTTIType(rttiType), mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	u32 Value<true>::GetTupleElementCount() const
	{
		if(mTupleElementIndex != ~0u)
			return 1;

		return (u32)mField->Schema.FieldTypes.Size();
	}

	Value<true> Value<true>::GetTupleElement(u32 tupleElementIndex) const
	{
		if(!B3D_ENSURE(mTupleElementIndex == ~0u))
			return Value<true>(mField, ~0u, nullptr, mRTTIType, mObject, mFrameAllocator);

		return Value<true>(mField, tupleElementIndex, mIterator, mRTTIType, mObject, mFrameAllocator);
	}

	Object<true> Value<true>::GetObject() const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return Object<true>(nullptr, nullptr, mFrameAllocator);

		if(mIterator != nullptr)
		{
			auto* field = static_cast<RTTIIteratorField*>(mField);
			const void* fieldValue = field->GetIteratorValue(mRTTIType, mObject, *mFrameAllocator, *mIterator);

			if(mField->Schema.Type == SerializableFT_ReflectablePtr)
			{
				SPtr<IReflectable> object = field->GetReflectablePointer(fieldValue, mTupleElementIndex);

				const u32 typeId = field->Schema.FieldTypes[mTupleElementIndex].FieldTypeId;
				return Object<true>(object.get(), IReflectable::GetRTTITypeFromTypeId(typeId), mFrameAllocator);
			}
			else if(mField->Schema.Type == SerializableFT_Reflectable)
			{
				const IReflectable& object = field->GetReflectable(fieldValue, mTupleElementIndex);

				const u32 typeId = field->Schema.FieldTypes[mTupleElementIndex].FieldTypeId;
				return Object<true>(const_cast<IReflectable*>(&object), IReflectable::GetRTTITypeFromTypeId(typeId), mFrameAllocator);
			}
		}
		else
		{
			const bool isArrayElement = mArrayIndex != ~0u;
			if(mField->Schema.Type == SerializableFT_ReflectablePtr)
			{
				SPtr<IReflectable> object;

				auto* field = static_cast<RTTIReflectablePtrFieldBase*>(mField);
				if(isArrayElement)
					object = field->GetArrayValue(mRTTIType, mObject, mArrayIndex);
				else
					object = field->GetValue(mRTTIType, mObject);

				return Object<true>(object.get(), field->GetType(), mFrameAllocator);
			}
			else if(mField->Schema.Type == SerializableFT_Reflectable)
			{
				IReflectable* object;

				auto* field = static_cast<RTTIReflectableFieldBase*>(mField);
				if(isArrayElement)
					object = &field->GetArrayValue(mRTTIType, mObject, mArrayIndex);
				else
					object = &field->GetValue(mRTTIType, mObject);

				return Object<true>(object, field->GetType(), mFrameAllocator);
			}
		}

		B3D_ASSERT(false && "Invalid field type");
		return Object<true>(nullptr, nullptr, mFrameAllocator);
	}

	SPtr<DataStream> Value<true>::GetDataStream(u32& size, u32& offset) const
	{
		auto* field = static_cast<RTTIManagedDataBlockFieldBase*>(mField);

		SPtr<DataStream> stream = field->GetValue(mRTTIType, mObject, size);
		offset = (u32)stream->Tell();

		return stream;
	}

	u32 Value<true>::GetPlainSize() const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return 0;

		if(mIterator != nullptr)
		{
			auto* field = static_cast<RTTIIteratorField*>(mField);
			const void* fieldValue = field->GetIteratorValue(mRTTIType, mObject, *mFrameAllocator, *mIterator);

			return field->GetPlainTypeSize(fieldValue, mTupleElementIndex, false).Bytes;
		}
		else
		{
			auto* field = static_cast<RTTIPlainFieldBase*>(mField);

			u32 size;
			if(field->Schema.HasDynamicSize)
			{
				const bool isArrayElement = mArrayIndex != ~0u;
				if(isArrayElement)
					size = field->GetArrayElemDynamicSize(mRTTIType, mObject, (int)mArrayIndex, false).Bytes;
				else
					size = field->GetDynamicSize(mRTTIType, mObject, false).Bytes;
			}
			else
				size = field->Schema.Size.Bytes;

			return size;
		}
	}

	void Value<true>::GetPlainData(u8* buffer, u32 bufferSize) const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return;

		if(mIterator != nullptr)
		{
			auto* field = static_cast<RTTIIteratorField*>(mField);
			const void* fieldValue = field->GetIteratorValue(mRTTIType, mObject, *mFrameAllocator, *mIterator);
			Bitstream tempStream(buffer, bufferSize);

			field->WritePlainTypeTupleToStream(fieldValue, mTupleElementIndex, tempStream, false);
		}
		else
		{
			auto* field = static_cast<RTTIPlainFieldBase*>(mField);
			Bitstream tempStream(buffer, bufferSize);

			const bool isArrayElement = mArrayIndex != ~0u;
			if(isArrayElement)
				field->ArrayElemToStream(mRTTIType, mObject, (int)mArrayIndex, tempStream);
			else
				field->ToStream(mRTTIType, mObject, tempStream);
		}
	}

	bool Value<true>::ComparePlain(const Value<true>& other) const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return false;

		u32 curTypeSize = GetPlainSize();

		auto curBuffer = B3DStackAllocate<u8>(curTypeSize);
		GetPlainData(curBuffer, curTypeSize);

		u32 otherTypeSize = other.GetPlainSize();

		auto otherBuffer = B3DStackAllocate<u8>(otherTypeSize);
		other.GetPlainData(otherBuffer, otherTypeSize);

		bool isModified = curTypeSize != otherTypeSize;
		if(!isModified)
			isModified = memcmp(curBuffer, otherBuffer, otherTypeSize) != 0;

		return isModified;
	}

	bool Value<true>::ComparePlain(const Value<false>& other) const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return false;

		auto* otherFieldData = static_cast<SerializedPlainData*>(other.mValue.get());

		u32 curTypeSize = other.GetPlainSize();

		auto buffer = B3DStackAllocate<u8>(curTypeSize);
		GetPlainData(buffer, curTypeSize);

		bool isModified = otherFieldData->Size != curTypeSize;
		if(!isModified)
			isModified = memcmp(otherFieldData->Value, buffer, curTypeSize) != 0;

		return isModified;
	}

	SPtr<ISerialized> Value<true>::Clone(SerializedObjectEncodeFlags flags, SerializationContext* context) const
	{
		auto* field = static_cast<RTTIIteratorField*>(mField);

		if(mIterator != nullptr)
		{
			if(mTupleElementIndex != ~0u)
				return IntermediateSerializer::SerializeTupleElement(*mObject, *mRTTIType, *field, *mIterator, mTupleElementIndex, flags, context, *mFrameAllocator);

			return IntermediateSerializer::SerializeElement(*mObject, *mRTTIType, *field, *mIterator, flags, context, *mFrameAllocator);
		}

		return IntermediateSerializer::SerializeField(mObject, mRTTIType, mField, mArrayIndex, flags, context, mFrameAllocator);
	}

	FieldIterator<false>::FieldIterator(SerializedObject* value, u32 subObjectIndex, FrameAllocator* frameAllocator)
		: mValue(value), mSubObjectIndex(subObjectIndex), mRTTIType(IReflectable::GetRTTITypeFromTypeId(value->SubObjects[subObjectIndex].TypeId)), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(value != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	bool FieldIterator<false>::MoveNext()
	{
		if(!mIsIteratorSet)
		{
			mFieldIterator = mValue->SubObjects[mSubObjectIndex].FieldEntries.begin();
			mIsIteratorSet = true;
		}
		else
			++mFieldIterator;

		return mFieldIterator != mValue->SubObjects[mSubObjectIndex].FieldEntries.end();
	}

	Field<false> FieldIterator<false>::GetValue() const
	{
		return Field<false>(mFieldIterator->first, mFieldIterator->second.Value, mFrameAllocator);
	}

	FieldIterator<true>::FieldIterator(RTTITypeBase* rttiType, IReflectable* value, FrameAllocator* frameAllocator)
		: mValue(value), mRTTIType(rttiType), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(value != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	bool FieldIterator<true>::MoveNext()
	{
		u32 numFields = mRTTIType->GetFieldCount();

		if(mFieldIndex == (u32)-1)
		{
			if(numFields > 0)
			{
				mFieldIndex = 0;
				return true;
			}

			return false;
		}

		if((mFieldIndex + 1) < numFields)
		{
			mFieldIndex++;
			return true;
		}

		return false;
	}

	Field<true> FieldIterator<true>::GetValue() const
	{
		RTTIField* field = mRTTIType->GetField(mFieldIndex);

		return Field<true>(mRTTIType, field, mValue, mFrameAllocator);
	}
} // namespace RTTIWrapper

using namespace RTTIWrapper;

typedef UnorderedMap<IReflectable*, SPtr<SerializedObject>> ObjectMap;

template <bool IsLHSIReflectable, bool IsRHSIReflectable>
SPtr<ISerialized> GenerateValueDelta(const RTTIFieldSchema& fieldSchema, const Value<IsLHSIReflectable>& lhs, const Value<IsRHSIReflectable> rhs, ObjectMap& objectMap, bool replicableOnly)
{
	SerializedObjectEncodeFlags flags = replicableOnly ? SerializedObjectEncodeFlag::ReplicableOnly : SerializedObjectEncodeFlags();
	SerializationContext* context = nullptr;

	SPtr<SerializedTuple> serializedTuple;
	SPtr<ISerialized> modification;
	bool hasModification = false;

	const bool isTuple = fieldSchema.FieldTypes.Size() > 1;
	for(u32 tupleElementIndex = 0; tupleElementIndex < (u32)fieldSchema.FieldTypes.Size(); ++tupleElementIndex)
	{
		const RTTIFieldTypeSchema& fieldTypeSchema = fieldSchema.FieldTypes[tupleElementIndex];
		const Value<IsLHSIReflectable>& lhsTupleElement = isTuple ? lhs.GetTupleElement(tupleElementIndex) : lhs;
		const Value<IsRHSIReflectable>& rhsTupleElement = isTuple ? rhs.GetTupleElement(tupleElementIndex) : rhs;

		SPtr<ISerialized> tupleElementModification;

		switch(fieldTypeSchema.Type)
		{
		case SerializableFT_ReflectablePtr:
		case SerializableFT_Reflectable:
			{
				Object<IsLHSIReflectable> lhsObject = lhsTupleElement.GetObject();
				Object<IsRHSIReflectable> rhsObject = rhsTupleElement.GetObject();

				auto found = objectMap.find(rhsObject.GetWrappedObject());
				if(found != objectMap.end())
					tupleElementModification = found->second;
				else
				{
					RTTITypeBase* rttiType = nullptr;
					if(lhsObject.GetTypeId() == rhsObject.GetTypeId())
						rttiType = IReflectable::GetRTTITypeFromTypeId(rhsObject.GetTypeId());

					SPtr<SerializedObject> objectDelta;
					if(rttiType != nullptr)
					{
						IDiff& handler = rttiType->GetDiffHandler();
						objectDelta = handler.GenerateDeltaRecursive(lhsObject.GetWrappedObject(), rhsObject.GetWrappedObject(), objectMap, replicableOnly);
					}

					if(objectDelta != nullptr)
						objectMap[rhsObject.GetWrappedObject()] = objectDelta;

					tupleElementModification = objectDelta;
				}
			}
			break;
		case SerializableFT_Plain:
			{
				if(lhsTupleElement.ComparePlain(rhsTupleElement))
					tupleElementModification = rhsTupleElement.Clone(flags, context);
			}
			break;
		case SerializableFT_DataBlock:
			{
				u32 lhsFieldDataSize;
				u32 lhsFieldDataOffset;
				SPtr<DataStream> lhsFieldStream = lhs.GetDataStream(lhsFieldDataSize, lhsFieldDataOffset);

				u32 rhsFieldDataSize;
				u32 rhsFieldDataOffset;
				SPtr<DataStream> rhsFieldStream = rhs.GetDataStream(rhsFieldDataSize, rhsFieldDataOffset);

				bool isModified = lhsFieldDataSize != rhsFieldDataSize;
				if(!isModified)
				{
					u8* lhsStreamData = nullptr;
					if(lhsFieldStream->IsFile())
					{
						lhsStreamData = (u8*)B3DStackAllocate(lhsFieldDataSize);
						lhsFieldStream->Seek(lhsFieldDataOffset);
						lhsFieldStream->Read(lhsStreamData, lhsFieldDataSize);
					}
					else
					{
						SPtr<MemoryDataStream> lhsMemoryStream = std::static_pointer_cast<MemoryDataStream>(lhsFieldStream);
						lhsStreamData = lhsMemoryStream->Cursor();
					}

					u8* rhsStreamData = nullptr;
					if(rhsFieldStream->IsFile())
					{
						rhsStreamData = (u8*)B3DStackAllocate(rhsFieldDataSize);
						rhsFieldStream->Seek(rhsFieldDataOffset);
						rhsFieldStream->Read(rhsStreamData, rhsFieldDataSize);
					}
					else
					{
						SPtr<MemoryDataStream> rhsMemoryStream = std::static_pointer_cast<MemoryDataStream>(rhsFieldStream);
						rhsStreamData = rhsMemoryStream->Cursor();
					}

					isModified = memcmp(lhsStreamData, rhsStreamData, rhsFieldDataSize) != 0;

					if(rhsFieldStream->IsFile())
						B3DStackFree(rhsStreamData);

					if(lhsFieldStream->IsFile())
						B3DStackFree(lhsStreamData);
				}

				if(isModified)
					tupleElementModification = rhs.Clone(flags, context);
			}
			break;
		}

		if(isTuple)
		{
			if(serializedTuple == nullptr)
			{
				serializedTuple = B3DMakeShared<SerializedTuple>();
				modification = serializedTuple;
			}

			serializedTuple->Values.Add(tupleElementModification);
		}
		else
		{
			modification = tupleElementModification;
		}

		hasModification = hasModification || tupleElementModification != nullptr;
	}

	return hasModification ? modification : nullptr;
}

template <bool IsLHSIReflectable, bool IsRHSIReflectable>
SPtr<SerializedObject> GenerateObjectDelta(Object<IsLHSIReflectable> lhs, Object<IsRHSIReflectable> rhs, ObjectMap& inOutObjectMap, bool replicableOnly)
{
	SerializedObjectEncodeFlags flags = replicableOnly ? SerializedObjectEncodeFlag::ReplicableOnly : SerializedObjectEncodeFlags();
	SerializationContext* context = nullptr;

	SubObjectIterator<IsRHSIReflectable> rhsSubObjectIterator = rhs.GetSubObjectIterator();

	SPtr<SerializedObject> output;
	while(rhsSubObjectIterator.MoveNext())
	{
		SubObject<IsRHSIReflectable> rhsSubObject = rhsSubObjectIterator.GetValue();

		RTTITypeBase* rtti = IReflectable::GetRTTITypeFromTypeId(rhsSubObject.GetTypeId());
		if(rtti == nullptr)
			continue;

		SubObject<IsLHSIReflectable> lhsSubObject;
		SubObjectIterator<IsLHSIReflectable> lhsSubObjectIterator = lhs.GetSubObjectIterator();
		while(lhsSubObjectIterator.MoveNext())
		{
			SubObject<IsLHSIReflectable> lhsSubObjectCandidate = lhsSubObjectIterator.GetValue();
			if(lhsSubObjectCandidate.GetTypeId() == rhsSubObject.GetTypeId())
			{
				lhsSubObject = lhsSubObjectCandidate;
				break;
			}
		}

		FieldIterator<IsRHSIReflectable> rhsFieldIterator = rhsSubObject.GetFieldIterator();

		SerializedSubObject* subObjectDelta = nullptr;
		while(rhsFieldIterator.MoveNext())
		{
			Field<IsRHSIReflectable> rhsField = rhsFieldIterator.GetValue();

			RTTIField* const field = rtti->FindField(rhsField.GetId());
			if(field == nullptr)
				continue;

			if(replicableOnly)
			{
				if(!field->Schema.Info.Flags.IsSet(RTTIFieldFlag::Replicate))
					continue;
			}

			FieldIterator<IsLHSIReflectable> lhsFieldIterator = lhsSubObject.GetFieldIterator();

			Field<IsLHSIReflectable> lhsField;
			bool foundMatchingField = false;
			while(lhsFieldIterator.MoveNext())
			{
				Field<IsLHSIReflectable> lhsFieldCandidate = lhsFieldIterator.GetValue();
				if(lhsFieldCandidate.GetId() == rhsField.GetId())
				{
					lhsField = lhsFieldCandidate;
					foundMatchingField = true;

					break;
				}
			}

			SPtr<SerializedArray> serializedArray;
			SPtr<SerializedMap> serializedMap;
			SPtr<ISerialized> modification;
			bool hasModification = false;

			if(foundMatchingField)
			{
				ValueIterator<IsRHSIReflectable> rhsValueIterator = rhsField.GetValueIterator();
				for(u32 elementIndex = 0; rhsValueIterator.MoveNext(); ++elementIndex)
				{
					SPtr<ISerialized> valueModification;
					bool hasValueModification = false;

					Value<IsRHSIReflectable> rhsValue = rhsValueIterator.GetValue();

					bool isRHSEntryNull = false;
					if(field->Schema.Type == SerializableFT_ReflectablePtr)
						isRHSEntryNull = rhsValue.GetObject().GetWrappedObject() == nullptr;

					ValueIterator<IsLHSIReflectable> lhsValueIterator = lhsField.GetValueIterator();
					Optional<Value<IsLHSIReflectable>> maybeLHSValue = lhsValueIterator.FindMatchingValue(rhsValueIterator);

					if(maybeLHSValue.has_value())
					{
						Value<IsLHSIReflectable>& lhsValue = *maybeLHSValue;

						bool isLHSEntryNull = false;
						if(field->Schema.Type == SerializableFT_ReflectablePtr)
							isLHSEntryNull = lhsValue.GetObject().GetWrappedObject() == nullptr;

						if(!isLHSEntryNull)
						{
							if(!isRHSEntryNull)
							{
								valueModification = GenerateValueDelta(field->Schema, lhsValue, rhsValue, inOutObjectMap, replicableOnly);
								hasValueModification = valueModification != nullptr;
							}
							else
							{
								valueModification = nullptr;
								hasValueModification = true;
							}
						}
						else
						{
							if(!isRHSEntryNull)
							{
								valueModification = rhsValue.Clone(flags, context);
								hasValueModification = true;
							}
						}
					}
					else
					{
						if(!isRHSEntryNull)
							valueModification = rhsValue.Clone(flags, context);
						else
							valueModification = nullptr;

						hasValueModification = true;
					}

					// If container, the modification above is just a single entry
					if(hasValueModification)
					{
						const bool isMap = field->Schema.IsIterator && field->Schema.IsArray && static_cast<RTTIIteratorField*>(field)->IteratorSupportsSeekToKey();
						const bool isArray = field->Schema.IsArray;

						if(isMap)
						{
							if(serializedMap == nullptr)
								serializedMap = B3DMakeShared<SerializedMap>();

							SPtr<ISerialized> entryKey;
							if(const auto& tuple = B3DRTTICast<SerializedTuple>(valueModification))
							{
								if(B3D_ENSURE(!tuple->Values.Empty()))
									entryKey = tuple->Values[0];
							}
							else
							{
								entryKey = valueModification;
							}

							serializedMap->Entries[entryKey] = valueModification;
							modification = serializedMap;
							
						}
						else if(isArray)
						{
							if(serializedArray == nullptr)
							{
								serializedArray = B3DMakeShared<SerializedArray>();
								serializedArray->ElementCount = rhsValueIterator.GetElementCount();
							}

							SerializedArrayEntry arrayEntry;
							arrayEntry.Index = elementIndex;
							arrayEntry.Value = valueModification;

							serializedArray->Entries[elementIndex] = arrayEntry;
							modification = serializedArray;
						}
						else
						{
							modification = valueModification;
						}

						hasModification = true;
					}
				} 

				// Note: I'm ignoring the case if a field is prevent in LHS, but removed in RHS. I can't imagine a case where this might be relevant.
			}
			else
			{
				modification = rhsField.Clone(flags, context);
				hasModification = true;
			}

			if(hasModification)
			{
				if(output == nullptr)
					output = B3DMakeShared<SerializedObject>();

				if(subObjectDelta == nullptr)
				{
					output->SubObjects.push_back(SerializedSubObject());
					subObjectDelta = &output->SubObjects.back();
					subObjectDelta->TypeId = rtti->GetRttiId();
				}

				SerializedField modificationEntry;
				modificationEntry.FieldId = field->Schema.Id;
				modificationEntry.Value = modification;
				subObjectDelta->FieldEntries[field->Schema.Id] = modificationEntry;
			}
		}
	}

	return output;
}

SPtr<SerializedObject> IDiff::GenerateDiff(const SPtr<IReflectable>& orgObj, const SPtr<IReflectable>& newObj, bool replicableOnly)
{
	ObjectMap objectMap;
	return GenerateDeltaRecursive(orgObj.get(), newObj.get(), objectMap, replicableOnly);
}

void IDiff::ApplyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, SerializationContext* context)
{
	FrameAllocator& allocator = GetFrameAllocator();
	allocator.MarkFrame();

	FrameVector<DeltaCommand> commands;

	DeltaObjectMap objectMap;
	GenerateDeltaApplyCommands(object, diff, allocator, objectMap, commands, context);

	IReflectable* destinationObject = nullptr;
	RTTITypeBase* rttiInstance = nullptr;

	Stack<IReflectable*> objectStack;
	Vector<std::pair<RTTITypeBase*, IReflectable*>> rttiInstances;

	SPtr<IRTTIIterator> currentIterator;
	void* currentIteratorFieldValue = nullptr;

	for(auto& command : commands)
	{
		const bool isArray = (command.Type & Diff_ArrayFlag) != 0;
		const bool isMap = (command.Type & Diff_MapFlag) != 0;
		DiffCommandType type = (DiffCommandType)(command.Type & 0xF);

		switch(type)
		{
		case Diff_ArraySize:
			command.Field->SetArraySize(rttiInstance, destinationObject, command.ArraySize);
			break;
		case Diff_ObjectStart:
			{
				destinationObject = command.Object.get();
				objectStack.push(destinationObject);

				FrameStack<RTTITypeBase*> rttiTypes;
				RTTITypeBase* curRtti = destinationObject->GetRtti();
				while(curRtti != nullptr)
				{
					rttiTypes.push(curRtti);
					curRtti = curRtti->GetBaseClass();
				}

				// Call base class first, followed by derived classes
				while(!rttiTypes.empty())
				{
					RTTITypeBase* curRtti = rttiTypes.top();
					RTTITypeBase* rttiInstance = curRtti->CloneInternal(allocator);

					rttiInstances.push_back(std::make_pair(rttiInstance, destinationObject));
					rttiInstance->OnDeserializationStarted(destinationObject, context);

					rttiTypes.pop();
				}
			}
			break;
		case Diff_SubObjectStart:
			{
				// Find the instance
				rttiInstance = nullptr;
				for(auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
				{
					if(iter->second != destinationObject)
						break;

					if(iter->first->GetRttiId() == command.RttiType->GetRttiId())
						rttiInstance = iter->first;
				}

				B3D_ASSERT(rttiInstance);
			}
			break;
		case Diff_ObjectEnd:
			{
				while(!rttiInstances.empty())
				{
					if(rttiInstances.back().second != destinationObject)
						break;

					RTTITypeBase* rttiInstance = rttiInstances.back().first;

					rttiInstance->OnDeserializationEnded(destinationObject, context);
					allocator.Destruct(rttiInstance);

					rttiInstances.erase(rttiInstances.end() - 1);
				}

				objectStack.pop();

				if(!objectStack.empty())
					destinationObject = objectStack.top();
				else
					destinationObject = nullptr;
			}
			break;
		case Diff_IterableEntryStart:
			{
				B3D_ASSERT(command.Field != nullptr);
				B3D_ASSERT(command.Field->Schema.IsIterator);
				B3D_ASSERT(currentIterator == nullptr);
				B3D_ASSERT(currentIteratorFieldValue == nullptr);
				B3D_ASSERT(rttiInstance != nullptr);
				B3D_ASSERT(destinationObject != nullptr);

				auto& field = *static_cast<RTTIIteratorField*>(command.Field);
				currentIterator = field.GetIterator(rttiInstance, destinationObject, allocator);

				if(isMap)
				{
					B3D_ASSERT(command.MapKey != nullptr);

					if(currentIterator->SeekToKey(command.MapKey)) // Modifying existing value
						currentIteratorFieldValue = field.GetIteratorValueCopy(rttiInstance, destinationObject, allocator, *currentIterator);
					else // Inserting a new value
						currentIteratorFieldValue = field.CreateEmptyFieldValue(allocator);

					field.FreeFieldValue(command.MapKey, allocator);
					command.MapKey = nullptr;
				}
				else if(isArray)
				{
					B3D_ASSERT(command.ArrayIndex != ~0u);

					// If there are any missing elements in the array, insert empty ones first
					if((command.ArrayIndex + 1) < currentIterator->GetElementCount())
					{
						while((command.ArrayIndex + 1) < currentIterator->GetElementCount())
						{
							currentIterator->SeekToEnd(); // Ensures value is inserted at the end of the iterable container

							void* fieldValue = field.CreateEmptyFieldValue(allocator);
							field.SetIteratorValue(rttiInstance, destinationObject, allocator, *currentIterator, fieldValue);
							field.FreeFieldValue(fieldValue, allocator);
						}
					}

					if(currentIterator->SeekToIndex(command.ArrayIndex))
						currentIteratorFieldValue = field.GetIteratorValueCopy(rttiInstance, destinationObject, allocator, *currentIterator);
					else // Inserting a new value
					{
						currentIterator->SeekToEnd();
						currentIteratorFieldValue = field.CreateEmptyFieldValue(allocator);
					}
				}
				else
				{
					currentIteratorFieldValue = field.GetIteratorValueCopy(rttiInstance, destinationObject, allocator, *currentIterator);
				}
			}
			break;
		case Diff_IterableEntryEnd:
			{
				B3D_ASSERT(command.Field != nullptr);
				B3D_ASSERT(command.Field->Schema.IsIterator);
				B3D_ASSERT(currentIterator != nullptr);
				B3D_ASSERT(currentIteratorFieldValue != nullptr);
				B3D_ASSERT(rttiInstance != nullptr);
				B3D_ASSERT(destinationObject != nullptr);

				auto& field = *static_cast<RTTIIteratorField*>(command.Field);
				field.SetIteratorValue(rttiInstance, destinationObject, allocator, *currentIterator, currentIteratorFieldValue);
				field.FreeFieldValue(currentIteratorFieldValue, allocator);
				currentIteratorFieldValue = nullptr;
			}
			break;
		default:
			break;
		}

		if(command.Field != nullptr && command.Field->Schema.IsIterator)
		{
			auto& field = *static_cast<RTTIIteratorField*>(command.Field);

			switch(type)
			{
			case Diff_ReflectablePtr:
				{
					B3D_ASSERT(currentIterator != nullptr);
					B3D_ASSERT(currentIteratorFieldValue != nullptr);

					field.SetReflectablePointer(currentIteratorFieldValue, command.TupleElementIndex, command.Object);
				}
				break;
			case Diff_Reflectable:
				{
					B3D_ASSERT(currentIterator != nullptr);
					B3D_ASSERT(currentIteratorFieldValue != nullptr);
					B3D_ASSERT(command.Object != nullptr);

					field.SetReflectable(currentIteratorFieldValue, command.TupleElementIndex, *command.Object);
				}
				break;
			case Diff_Plain:
				{
					B3D_ASSERT(currentIterator != nullptr);
					B3D_ASSERT(currentIteratorFieldValue != nullptr);

					Bitstream tempStream(command.Value, command.Size);
					field.ReadPlainTypeTupleFromStream(currentIteratorFieldValue, command.TupleElementIndex, tempStream, false);
				}
				break;
			default:
				break;
			}
		}
		else // DEPRECATED - All except DataBlock case
		{
			if(isArray)
			{
				switch(type)
				{
				case Diff_ReflectablePtr:
					{
						auto* field = static_cast<RTTIReflectablePtrFieldBase*>(command.Field);
						field->SetArrayValue(rttiInstance, destinationObject, command.ArrayIndex, command.Object);
					}
					break;
				case Diff_Reflectable:
					{
						auto* field = static_cast<RTTIReflectableFieldBase*>(command.Field);
						field->SetArrayValue(rttiInstance, destinationObject, command.ArrayIndex, *command.Object);
					}
					break;
				case Diff_Plain:
					{
						auto* field = static_cast<RTTIPlainFieldBase*>(command.Field);

						Bitstream tempStream(command.Value, command.Size);
						field->ArrayElemFromBuffer(rttiInstance, destinationObject, command.ArrayIndex, tempStream);
					}
					break;
				default:
					break;
				}
			}
			else
			{
				switch(type)
				{
				case Diff_ReflectablePtr:
					{
						auto* field = static_cast<RTTIReflectablePtrFieldBase*>(command.Field);
						field->SetValue(rttiInstance, destinationObject, command.Object);
					}
					break;
				case Diff_Reflectable:
					{
						auto* field = static_cast<RTTIReflectableFieldBase*>(command.Field);
						field->SetValue(rttiInstance, destinationObject, *command.Object);
					}
					break;
				case Diff_Plain:
					{
						auto* field = static_cast<RTTIPlainFieldBase*>(command.Field);

						Bitstream tempStream(command.Value, command.Size);
						field->FromBuffer(rttiInstance, destinationObject, tempStream);
					}
					break;
				case Diff_DataBlock:
					{
						auto* field = static_cast<RTTIManagedDataBlockFieldBase*>(command.Field);
						field->SetValue(rttiInstance, destinationObject, command.StreamValue, command.Size);
					}
					break;
				default:
					break;
				}
			}
		}
	}

	allocator.Clear();
}

void IDiff::ApplyDiff(RTTITypeBase* rtti, const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, FrameAllocator& alloc, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& diffCommands, SerializationContext* context)
{
	IDiff& diffHandler = rtti->GetDiffHandler();
	diffHandler.GenerateDeltaApplyCommands(object, diff, alloc, objectMap, diffCommands, context);
}

SPtr<SerializedObject> BinaryDiff::GenerateDeltaRecursive(IReflectable* lhs, IReflectable* rhs, ObjectMap& objectMap, bool replicableOnly)
{
	FrameAllocator& frameAllocator = GetFrameAllocator();

	if(lhs->GetTypeId() == TID_SerializedObject)
	{
		Object<false> lhsWrapper(static_cast<SerializedObject*>(lhs), &frameAllocator);

		if(rhs->GetTypeId() == TID_SerializedObject)
		{
			Object<false> rhsWrapper(static_cast<SerializedObject*>(rhs), &frameAllocator);
			return ::GenerateObjectDelta(lhsWrapper, rhsWrapper, objectMap, replicableOnly);
		}

		Object<true> rhsWrapper(rhs, rhs->GetRtti(), &frameAllocator);
		return ::GenerateObjectDelta(lhsWrapper, rhsWrapper, objectMap, replicableOnly);
	}
	else
	{
		Object<true> lhsWrapper(lhs, lhs->GetRtti(), &frameAllocator);

		if(rhs->GetTypeId() == TID_SerializedObject)
		{
			Object<false> rhsWrapper(static_cast<SerializedObject*>(rhs), &frameAllocator);
			return ::GenerateObjectDelta(lhsWrapper, rhsWrapper, objectMap, replicableOnly);
		}

		Object<true> rhsWrapper(rhs, rhs->GetRtti(), &frameAllocator);
		return ::GenerateObjectDelta(lhsWrapper, rhsWrapper, objectMap, replicableOnly);
	}
}

void BinaryDiff::GenerateDeltaApplyCommands(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& diffCommands, SerializationContext* context)
{
	if(object == nullptr || diff == nullptr || object->GetTypeId() != diff->GetRootTypeId())
		return;

	// Generate a list of commands per sub-object
	FrameVector<FrameVector<DeltaCommand>> commandsPerSubObj;

	Stack<RTTITypeBase*> rttiInstances;
	for(auto& subObject : diff->SubObjects)
	{
		RTTITypeBase* rtti = IReflectable::GetRTTITypeFromTypeId(subObject.TypeId);
		if(rtti == nullptr)
			continue;

		if(!object->IsDerivedFrom(rtti))
			continue;

		RTTITypeBase* rttiInstance = rtti->CloneInternal(allocator);
		rttiInstance->OnSerializationStarted(object.get(), nullptr);
		rttiInstances.push(rttiInstance);

		FrameVector<DeltaCommand> commands;

		DeltaCommand subObjStartCommand;
		subObjStartCommand.RttiType = rtti;
		subObjStartCommand.Field = nullptr;
		subObjStartCommand.Type = Diff_SubObjectStart;

		commands.push_back(subObjStartCommand);

		for(auto& diffEntry : subObject.FieldEntries)
		{
			RTTIField* genericField = rtti->FindField(diffEntry.first);
			if(genericField == nullptr)
				continue;

			SPtr<ISerialized> fieldDelta = diffEntry.second.Value;

			if(genericField->Schema.IsIterator)
			{
				auto* field = static_cast<RTTIIteratorField*>(genericField);

				if(const auto& serializedArrayDelta = B3DRTTICast<SerializedArray>(fieldDelta))
				{
					const u32 arrayDeltaElementCount = serializedArrayDelta->ElementCount;

					DeltaCommand arraySizeCommand;
					arraySizeCommand.Field = genericField;
					arraySizeCommand.Type = Diff_ArraySize | Diff_ArrayFlag;
					arraySizeCommand.ArraySize = arrayDeltaElementCount;

					commands.push_back(arraySizeCommand);

					for(auto& arrayDeltaElement : serializedArrayDelta->Entries)
					{
						GenerateDeltaCommandForEntry(rttiInstance, object, *field, arrayDeltaElement.second.Value, arrayDeltaElement.second.Index, nullptr, objectMap, commands, context, allocator);
					}
				}
				else if(const auto& serializedMapDelta = B3DRTTICast<SerializedMap>(fieldDelta))
				{
					for(auto& mapDeltaElement : serializedMapDelta->Entries)
					{
						void* deserializedMapKey = field->CreateEmptyFieldValue(allocator);

						IntermediateSerializer intermediateSerializer(&allocator);
						intermediateSerializer.DeserializeTupleElement(*field, deserializedMapKey, 0, mapDeltaElement.first);

						GenerateDeltaCommandForEntry(rttiInstance, object, *field, mapDeltaElement.second, ~0u, deserializedMapKey, objectMap, commands, context, allocator);
					}
				}
				else
				{
					GenerateDeltaCommandForEntry(rttiInstance, object, *field, fieldDelta, ~0u, nullptr, objectMap, commands, context, allocator);
				}
			}
			else if(genericField->Schema.IsArray) // DEPRECATED
			{
				SPtr<SerializedArray> arrayDelta = std::static_pointer_cast<SerializedArray>(fieldDelta);

				const u32 arrayDeltaElementCount = arrayDelta->ElementCount;

				DeltaCommand arraySizeCommand;
				arraySizeCommand.Field = genericField;
				arraySizeCommand.Type = Diff_ArraySize | Diff_ArrayFlag;
				arraySizeCommand.ArraySize = arrayDeltaElementCount;

				commands.push_back(arraySizeCommand);

				for(auto& arrayDeltaElement : arrayDelta->Entries)
					GenerateDeltaCommandForEntry(rttiInstance, object, *genericField, arrayDeltaElement.second.Value, arrayDeltaElement.second.Index, objectMap, commands, context, allocator);
			}
			else
			{
				GenerateDeltaCommandForEntry(rttiInstance, object, *genericField, fieldDelta, ~0u, objectMap, commands, context, allocator);
			}
		}

		commandsPerSubObj.emplace_back(std::move(commands));
	}

	DeltaCommand objStartCommand;
	objStartCommand.Field = nullptr;
	objStartCommand.Type = Diff_ObjectStart;
	objStartCommand.Object = object;

	diffCommands.push_back(objStartCommand);

	// Go in reverse because when deserializing we want to deserialize base first, and then derived types
	for(auto iter = commandsPerSubObj.rbegin(); iter != commandsPerSubObj.rend(); ++iter)
		diffCommands.insert(diffCommands.end(), iter->begin(), iter->end());

	DeltaCommand objEndCommand;
	objEndCommand.Field = nullptr;
	objEndCommand.Type = Diff_ObjectEnd;
	objEndCommand.Object = object;

	diffCommands.push_back(objEndCommand);

	while(!rttiInstances.empty())
	{
		RTTITypeBase* rttiInstance = rttiInstances.top();
		rttiInstance->OnSerializationEnded(object.get(), nullptr);
		allocator.Destruct(rttiInstance);

		rttiInstances.pop();
	}
}

void BinaryDiff::GenerateDeltaCommandForEntry(RTTITypeBase* rttiInstance, const SPtr<IReflectable>& object, RTTIIteratorField& field, const SPtr<ISerialized>& entryDelta, u32 arrayIndex, void* mapKey, DeltaObjectMap& inOutObjectMap, FrameVector<DeltaCommand>& outCommands, SerializationContext* context, FrameAllocator& allocator)
{
	const SPtr<SerializedTuple> serializedTuple = B3DRTTICast<SerializedTuple>(entryDelta);
	if(!B3D_ENSURE(field.Schema.FieldTypes.Size() == 1 || serializedTuple != nullptr))
		return;

	SPtr<IRTTIIterator> iterator = field.GetIterator(rttiInstance, object.get(), allocator);
	if(iterator == nullptr)
		return;

	DeltaCommand iteratorStartCommand;
	iteratorStartCommand.Field = &field;
	iteratorStartCommand.Type = Diff_IterableEntryStart;

	const void* fieldValue = nullptr;
	if(arrayIndex != ~0u)
	{
		iteratorStartCommand.Type = Diff_ArrayFlag;
		iteratorStartCommand.ArrayIndex = arrayIndex;

		if(!B3D_ENSURE(field.IteratorSupportsSeekToIndex()))
			return;

		if(iterator->SeekToIndex(arrayIndex))
			fieldValue = field.GetIteratorValue(rttiInstance, object.get(), allocator, *iterator);
	}
	else if(mapKey != nullptr)
	{
		iteratorStartCommand.Type = Diff_MapFlag;
		iteratorStartCommand.MapKey = mapKey;

		if(!B3D_ENSURE(field.IteratorSupportsSeekToKey()))
			return;

		if(iterator->SeekToKey(mapKey))
			fieldValue = field.GetIteratorValue(rttiInstance, object.get(), allocator, *iterator);
	}
	else
	{
		fieldValue = field.GetIteratorValue(rttiInstance, object.get(), allocator, *iterator);
	}

	outCommands.push_back(iteratorStartCommand);

	for(u32 tupleElementIndex = 0; tupleElementIndex < field.Schema.FieldTypes.Size(); ++tupleElementIndex)
	{
		SPtr<ISerialized> serializedEntry = serializedTuple != nullptr ? serializedTuple->Values[tupleElementIndex] : entryDelta;

		DeltaCommand command;
		command.Field = &field;
		command.TupleElementIndex = tupleElementIndex;
		command.Type = 0;

		// Retrieves the deserialized object from the object map, or if it doesn't exist creates and new instance of the object.
		auto fnGetOrDeserializeObject = [&inOutObjectMap](const SPtr<SerializedObject>& serializedObject) -> SPtr<IReflectable>
		{
			if(serializedObject == nullptr)
				return nullptr;

			RTTITypeBase* childRtti = IReflectable::GetRTTITypeFromTypeId(serializedObject->GetRootTypeId());
			if(childRtti != nullptr)
			{
				auto foundObject = inOutObjectMap.find(serializedObject);
				if(foundObject == inOutObjectMap.end())
				{
					SPtr<IReflectable> newObject = childRtti->NewRttiObject();
					foundObject = inOutObjectMap.insert(std::make_pair(serializedObject, newObject)).first;
				}

				return foundObject->second;
			}

			return nullptr;
		};

		switch(field.Schema.Type)
		{
		case SerializableFT_ReflectablePtr:
			{
				SPtr<SerializedObject> serializedObjectDelta = std::static_pointer_cast<SerializedObject>(serializedEntry);

				command.Type |= Diff_ReflectablePtr;

				if(serializedObjectDelta == nullptr)
					command.Object = nullptr;
				else
				{
					SPtr<IReflectable> childObject = fieldValue != nullptr ? field.GetReflectablePointer(fieldValue, tupleElementIndex) : nullptr;
					if(childObject == nullptr)
						command.Object = fnGetOrDeserializeObject(serializedObjectDelta);
					else
						command.Object = childObject;

					if(childObject != nullptr)
						IDiff::ApplyDiff(childObject->GetRtti(), childObject, serializedObjectDelta, allocator, inOutObjectMap, outCommands, context);
				}

				outCommands.push_back(command);
			}
			break;
		case SerializableFT_Reflectable:
			{
				SPtr<SerializedObject> serializedObjectDelta = std::static_pointer_cast<SerializedObject>(serializedEntry);

				SPtr<IReflectable> clonedObject;
				if(fieldValue != nullptr)
				{
					const IReflectable& childObject = field.GetReflectable(fieldValue, tupleElementIndex);
					clonedObject = BinaryCloner::Clone(const_cast<IReflectable*>(&childObject), true);
				}
				else
					clonedObject = fnGetOrDeserializeObject(serializedObjectDelta);

				if(B3D_ENSURE(clonedObject != nullptr))
					IDiff::ApplyDiff(clonedObject->GetRtti(), clonedObject, serializedObjectDelta, allocator, inOutObjectMap, outCommands, context);

				command.Type |= Diff_Reflectable;
				command.Object = clonedObject;

				outCommands.push_back(command);
			}
			break;
		case SerializableFT_Plain:
			{
				SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(serializedEntry);

				if(serializedPlainData->Size > 0)
				{
					command.Type |= Diff_Plain;
					command.Value = serializedPlainData->Value;
					command.Size = serializedPlainData->Size;

					outCommands.push_back(command);
				}
			}
			break;
		default:
			B3D_ENSURE(false);
			return;
		}
	}

	DeltaCommand iteratorEndCommand;
	iteratorEndCommand.Field = &field;
	iteratorEndCommand.Type = Diff_IterableEntryEnd;

	if(arrayIndex != ~0u)
	{
		iteratorEndCommand.Type = Diff_ArrayFlag;
		iteratorEndCommand.ArrayIndex = arrayIndex;
	}
	else if(mapKey != nullptr)
	{
		iteratorEndCommand.Type = Diff_MapFlag;
		iteratorEndCommand.MapKey = mapKey;
	}

	outCommands.push_back(iteratorEndCommand);
}

void BinaryDiff::GenerateDeltaCommandForEntry(RTTITypeBase* rttiInstance, const SPtr<IReflectable>& object, RTTIField& field, const SPtr<ISerialized>& entryDelta, u32 arrayIndex, DeltaObjectMap& inOutObjectMap, FrameVector<DeltaCommand>& outCommands, SerializationContext* context, FrameAllocator& allocator)
{
	const SPtr<SerializedTuple> serializedTuple = B3DRTTICast<SerializedTuple>(entryDelta);
	if(!B3D_ENSURE(field.Schema.FieldTypes.Size() == 1 || serializedTuple != nullptr))
		return;

	const bool isArrayElement = arrayIndex != ~0u;
	const u32 arraySize = isArrayElement ? field.GetArraySize(rttiInstance, object.get()) : 1;

	for(u32 tupleElementIndex = 0; tupleElementIndex < field.Schema.FieldTypes.Size(); ++tupleElementIndex)
	{
		SPtr<ISerialized> serializedEntry = serializedTuple != nullptr ? serializedTuple->Values[tupleElementIndex] : entryDelta;

		switch(field.Schema.Type)
		{
		case SerializableFT_ReflectablePtr:
			{
				auto& reflectableField = static_cast<RTTIReflectablePtrFieldBase&>(field);
				SPtr<SerializedObject> serializedObjectDelta = std::static_pointer_cast<SerializedObject>(serializedEntry);

				DeltaCommand command;
				command.Field = &field;
				command.Type = Diff_ReflectablePtr;

				if(isArrayElement)
				{
					command.Type |= Diff_ArrayFlag;
					command.ArrayIndex = arrayIndex;
				}

				if(serializedObjectDelta == nullptr)
					command.Object = nullptr;
				else
				{
					SPtr<IReflectable> childObject;

					if(isArrayElement)
					{
						if(arrayIndex < arraySize)
							childObject = reflectableField.GetArrayValue(rttiInstance, object.get(), arrayIndex);
					}
					else
						childObject = reflectableField.GetValue(rttiInstance, object.get());

					if(childObject == nullptr)
					{
						RTTITypeBase* childRtti = IReflectable::GetRTTITypeFromTypeId(serializedObjectDelta->GetRootTypeId());
						if(childRtti != nullptr)
						{
							auto findObj = inOutObjectMap.find(serializedObjectDelta);
							if(findObj == inOutObjectMap.end())
							{
								SPtr<IReflectable> newObject = childRtti->NewRttiObject();
								findObj = inOutObjectMap.insert(std::make_pair(serializedObjectDelta, newObject)).first;
							}

							IDiff::ApplyDiff(childRtti, findObj->second, serializedObjectDelta, allocator, inOutObjectMap, outCommands, context);
							command.Object = findObj->second;
						}
						else
						{
							command.Object = nullptr;
						}
					}
					else
					{
						IDiff::ApplyDiff(childObject->GetRtti(), childObject, serializedObjectDelta, allocator, inOutObjectMap, outCommands, context);
						command.Object = childObject;
					}
				}

				outCommands.push_back(command);
			}
			break;
		case SerializableFT_Reflectable:
			{
				auto& reflectableField = static_cast<RTTIReflectableFieldBase&>(field);
				SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(serializedEntry);

				SPtr<IReflectable> clonedObject;
				if(isArrayElement)
				{
					if(arrayIndex < arraySize)
					{
						IReflectable& childObject = reflectableField.GetArrayValue(rttiInstance, object.get(), arrayIndex);
						clonedObject = BinaryCloner::Clone(&childObject, true);
					}
					else
					{
						RTTITypeBase* childRtti = IReflectable::GetRTTITypeFromTypeId(fieldObjectData->GetRootTypeId());
						if(childRtti != nullptr)
							clonedObject = childRtti->NewRttiObject();
					}
				}
				else
				{
					IReflectable& childObject = reflectableField.GetValue(rttiInstance, object.get());
					clonedObject = BinaryCloner::Clone(&childObject, true);
				}

				IDiff::ApplyDiff(clonedObject->GetRtti(), clonedObject, fieldObjectData, allocator, inOutObjectMap, outCommands, context);

				DeltaCommand command;
				command.Field = &field;
				command.Type = Diff_Reflectable;
				command.Object = clonedObject;

				if(isArrayElement)
				{
					command.Type |= Diff_ArrayFlag;
					command.ArrayIndex = arrayIndex;
				}


				outCommands.push_back(command);
			}
			break;
		case SerializableFT_Plain:
			{
				SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(serializedEntry);

				if(serializedPlainData->Size > 0)
				{
					DeltaCommand command;
					command.Field = &field;
					command.Type = Diff_Plain;
					command.Value = serializedPlainData->Value;
					command.Size = serializedPlainData->Size;

					if(isArrayElement)
					{
						command.Type |= Diff_ArrayFlag;
						command.ArrayIndex = arrayIndex;
					}

					outCommands.push_back(command);
				}
			}
			break;
		case SerializableFT_DataBlock:
			{
				SPtr<SerializedDataBlock> serializedDataBlock = std::static_pointer_cast<SerializedDataBlock>(serializedEntry);

				DeltaCommand command;
				command.Field = &field;
				command.Type = Diff_DataBlock;
				command.StreamValue = serializedDataBlock->Stream;
				command.Value = nullptr;
				command.Size = serializedDataBlock->Size;

				outCommands.push_back(command);
			}
			break;
		}
	}
}


