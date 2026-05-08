//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "FileSystem/B3DDataStream.h"
#include "Serialization/B3DIntermediateSerializer.h"

namespace b3d::RTTIObjectWrapper
{
	inline Object<false>::Object(SerializedObject* object, FrameAllocator* frameAllocator)
		: mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 Object<false>::GetTypeId() const
	{
		return mObject->GetRootTypeId();
	}

	inline SubObjectIterator<false> Object<false>::GetSubObjectIterator() const
	{
		return SubObjectIterator<false>(mObject, mFrameAllocator);
	}

	inline void Object<false>::NotifyBeginOperation(SubObject<false>& subObject, RTTIOperationType operationType, RTTIOperationContext& context)
	{
		// Do nothing
	}

	inline void Object<false>::NotifyEndOperation(RTTIOperationType operationType, RTTIOperationContext& context)
	{
		// Do nothing
	}

	inline Object<true>::Object(IReflectable* object, RTTIType* type, FrameAllocator* frameAllocator)
		: mObject(object), mRTTIType(type), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(type != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 Object<true>::GetTypeId() const
	{
		return mObject->GetTypeId();
	}

	inline void Object<true>::NotifyBeginOperation(SubObject<true>& subObject, RTTIOperationType operationType, RTTIOperationContext& context)
	{
		B3D_ASSERT(subObject.mRTTIType != nullptr);
		B3D_ASSERT(subObject.mRTTITypeInstance == nullptr);

		RTTIType* rttiTypeInstance = subObject.mRTTIType->CloneInternal(*mFrameAllocator);
		rttiTypeInstance->NotifyOperationStarted(*mObject, operationType, context);

		mRTTITypeInstances.push_back(rttiTypeInstance);
		subObject.mRTTITypeInstance = rttiTypeInstance;
	}

	inline void Object<true>::NotifyEndOperation(RTTIOperationType operationType, RTTIOperationContext& context)
	{
		// Note: It would make sense to finish deserializing derived classes before base classes, but some code
		// depends on the old functionality, so we'll keep it this way
		for(auto it = mRTTITypeInstances.rbegin(); it != mRTTITypeInstances.rend(); ++it)
		{
			RTTIType* const currentRTTIInstance = *it;

			currentRTTIInstance->NotifyOperationEnded(*mObject, operationType, context);
			mFrameAllocator->Destruct(currentRTTIInstance);
		}

		mRTTITypeInstances.clear();
	}

	inline SubObjectIterator<true> Object<true>::GetSubObjectIterator() const
	{
		return SubObjectIterator<true>(mRTTIType, mObject, mFrameAllocator);
	}

	inline SubObject<false>::SubObject(SerializedObject* object, u32 subObjectIndex, FrameAllocator* frameAllocator)
		: mObject(object), mSubObjectIndex(subObjectIndex), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 SubObject<false>::GetTypeId() const
	{
		return mObject->SubObjects[mSubObjectIndex].TypeId;
	}

	inline FieldIterator<false> SubObject<false>::GetFieldIterator() const
	{
		return FieldIterator<false>(mObject, mSubObjectIndex, mFrameAllocator);
	}

	inline SubObject<true>::SubObject(IReflectable* object, RTTIType* type, FrameAllocator* frameAllocator)
		: mObject(object), mRTTIType(type), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(type != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 SubObject<true>::GetTypeId() const
	{
		return mRTTIType->GetRttiId();
	}

	inline FieldIterator<true> SubObject<true>::GetFieldIterator() const
	{
		B3D_ASSERT(mRTTITypeInstance != nullptr);
		return FieldIterator<true>(mRTTIType, mRTTITypeInstance, mObject, mFrameAllocator);
	}

	inline SubObjectIterator<false>::SubObjectIterator(SerializedObject* object, FrameAllocator* frameAllocator)
		: mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline bool SubObjectIterator<false>::MoveNext()
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

	inline SubObject<false> SubObjectIterator<false>::GetValue() const
	{
		return SubObject<false>(mObject, mCurrentSubObjectIndex, mFrameAllocator);
	}

	inline SubObjectIterator<true>::SubObjectIterator(RTTIType* rttiType, IReflectable* object, FrameAllocator* frameAllocator)
		: mObject(object), mRTTIType(rttiType), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline bool SubObjectIterator<true>::MoveNext()
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

	inline SubObject<true> SubObjectIterator<true>::GetValue() const
	{
		return SubObject<true>(mObject, mCurrentRTTIType, mFrameAllocator);
	}

	inline Field<false>::Field(u32 fieldId, const TShared<ISerialized>& value, FrameAllocator* frameAllocator)
		: mId(fieldId), mValue(value), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 Field<false>::GetId() const
	{
		return mId;
	}

	inline ValueIterator<false> Field<false>::GetValueIterator() const
	{
		return ValueIterator<false>(mValue, mFrameAllocator);
	}

	inline TShared<ISerialized> Field<false>::Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const
	{
		return mValue != nullptr ? mValue->Clone() : nullptr;
	}

	inline Field<true>::Field(RTTIType* rttiTypeInstance, RTTIField* field, IReflectable* object, FrameAllocator* frameAllocator)
		: mRTTITypeInstance(rttiTypeInstance), mField(field), mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(rttiTypeInstance != nullptr);
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 Field<true>::GetId() const
	{
		return mField->Schema.Id;
	}

	inline ValueIterator<true> Field<true>::GetValueIterator() const
	{
		switch(mField->Schema.FieldType)
		{
		case RTTIFieldType::Iterable:
		{
			auto* const field = static_cast<RTTIIteratorField*>(mField);
			const TShared<IRTTIIterator> iterator = field->GetIterator(mRTTITypeInstance, mObject, *mFrameAllocator);

			return ValueIterator<true>(mField, mRTTITypeInstance, mObject, iterator, mFrameAllocator);
		}
		case RTTIFieldType::DataBlock:
			return ValueIterator<true>(mField, mRTTITypeInstance, mObject, ~0u, mFrameAllocator);
		default:
			return ValueIterator<true>(nullptr, nullptr, nullptr, ~0u, mFrameAllocator);
		}
	}

	inline TShared<ISerialized> Field<true>::Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const
	{
		IntermediateSerializer intermediateSerializer(mFrameAllocator, context);

		switch(mField->Schema.FieldType)
		{
		case RTTIFieldType::Iterable:
		{
			auto* const field = static_cast<RTTIIteratorField*>(mField);
			return intermediateSerializer.SerializeIterableField(*mObject, *mRTTITypeInstance, *field, flags);
		}
		case RTTIFieldType::DataBlock:
		{
			return intermediateSerializer.SerializeDataBlockField(mObject, mRTTITypeInstance, mField, flags);
		}
		default:
			return nullptr;
		}
	}

	inline ValueIterator<false>::ValueIterator(const TShared<ISerialized>& value, FrameAllocator* frameAllocator)
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

	inline bool ValueIterator<false>::MoveNext()
	{
		if(mArrayContainerValue != nullptr)
		{
			if(!mIsIteratorSet)
			{
				mArrayIndex = 0;
				mIsIteratorSet = true;
			}
			else
				++mArrayIndex;

			return mArrayIndex < mArrayContainerValue->Entries.Size();
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

	inline Value<false> ValueIterator<false>::GetValue() const
	{
		TShared<ISerialized> value;
		if(mArrayContainerValue != nullptr)
			value = mArrayContainerValue->Entries[mArrayIndex];
		else if(mMapContainerValue != nullptr)
			value = mMapIterator->second;
		else
			value = mValue;

		return Value<false>(~0u, value, mFrameAllocator);
	}

	inline u32 ValueIterator<false>::GetElementCount() const
	{
		if(mArrayContainerValue != nullptr)
			return (u32)mArrayContainerValue->Entries.Size();
		else if(mMapContainerValue != nullptr)
			return (u32)mMapContainerValue->Entries.size();

		return 1;
	}

	inline TOptional<Value<false>> ValueIterator<false>::FindMatchingValue(const ValueIterator<false>& otherIterator) const
	{
		if(!B3D_ENSURE(otherIterator.mIsIteratorSet))
			return {};

		if(mArrayContainerValue != nullptr)
		{
			if(!B3D_ENSURE(otherIterator.mArrayContainerValue != nullptr))
				return {};

			if(otherIterator.mArrayIndex < mArrayContainerValue->Entries.Size())
				return Value<false>(~0u, mArrayContainerValue->Entries[otherIterator.mArrayIndex], mFrameAllocator);
		}
		else if(mMapContainerValue != nullptr)
		{
			if(!B3D_ENSURE(otherIterator.mMapContainerValue != nullptr))
				return {};

			auto found = mMapContainerValue->Entries.find(otherIterator.mMapIterator->first);
			if(found != mMapContainerValue->Entries.end())
				return Value<false>(~0u, found->second, mFrameAllocator);
		}
		else // Just a regular field
			return Value<false>(~0u, mValue, mFrameAllocator);

		return {};
	}

	inline TOptional<Value<false>> ValueIterator<false>::FindMatchingValue(const ValueIterator<true>& otherIterator) const
	{
		B3D_ASSERT(false); // Not supported at the moment. To support, we'd need to convert the otherIterator value to ISerialized object that we can lookup.
		return {};
	}

	inline ValueIterator<true>::ValueIterator(RTTIField* field, RTTIType* rttiTypeInstance, IReflectable* object, const TShared<IRTTIIterator>& iterator, FrameAllocator* frameAllocator)
		: mIterator(iterator), mObject(object), mRTTITypeInstance(rttiTypeInstance), mField(field), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(rttiTypeInstance != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(iterator != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline ValueIterator<true>::ValueIterator(RTTIField* field, RTTIType* rttiTypeInstance, IReflectable* object, u32 elementCount, FrameAllocator* frameAllocator)
		: mElementCount(elementCount), mObject(object), mRTTITypeInstance(rttiTypeInstance), mField(field), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(rttiTypeInstance != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline bool ValueIterator<true>::MoveNext()
	{
		if(!mIsIteratorSet)
		{
			mIsIteratorSet = true;
			mElementIndex = 0;
		}
		else
		{
			mElementIndex++;

			if(mIterator != nullptr)
				mIterator->Increment();
		}

		if(mIterator != nullptr)
			return mIterator->IsValid();
		else
		{
			if(mElementCount == ~0u)
				return mElementIndex == 0;
			else
				return mElementIndex < mElementCount;
		}
	}

	inline Value<true> ValueIterator<true>::GetValue() const
	{
		if(mIterator != nullptr)
			return Value<true>(mField, ~0u, mIterator, mRTTITypeInstance, mObject, mFrameAllocator);

		if(mElementCount != ~0u)
			return Value<true>(mField, ~0u, mElementIndex, mRTTITypeInstance, mObject, mFrameAllocator);

		return Value<true>(mField, ~0u, ~0u, mRTTITypeInstance, mObject, mFrameAllocator);
	}

	inline u32 ValueIterator<true>::GetElementCount() const
	{
		if(mIterator != nullptr)
			return (u32)mIterator->GetElementCount();

		if(mElementCount != ~0u)
			return mElementCount;

		return 1;
	}

	inline TOptional<Value<true>> ValueIterator<true>::FindMatchingValue(const ValueIterator<true>& otherIterator) const
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
				TShared<IRTTIIterator> iteratorCopy = mIterator->Clone(*mFrameAllocator);

				const void* fieldValue = field.GetIteratorValue(mRTTITypeInstance, mObject, *mFrameAllocator, *mIterator);

				if(!iteratorCopy->SeekToKey(fieldValue))
					return {};
				
				return Value<true>(mField, ~0u, iteratorCopy, mRTTITypeInstance, mObject, mFrameAllocator);
			}
			else if(field.IteratorSupportsSeekToIndex())
			{
				TShared<IRTTIIterator> iteratorCopy = mIterator->Clone(*mFrameAllocator);
				if(!iteratorCopy->SeekToIndex(otherIterator.mElementIndex))
					return {};
				
				return Value<true>(mField, ~0u, iteratorCopy, mRTTITypeInstance, mObject, mFrameAllocator);
			}
			else // Just a regular field
				return Value<true>(mField, ~0u, mIterator, mRTTITypeInstance, mObject, mFrameAllocator);
		}
		else
		{
			if(mElementCount == ~0u) // Just a regular field
				return Value<true>(mField, ~0u, ~0u, mRTTITypeInstance, mObject, mFrameAllocator);

			if(otherIterator.mElementIndex >= mElementCount)
				return {};

			return Value<true>(mField, ~0u, otherIterator.mElementIndex, mRTTITypeInstance, mObject, mFrameAllocator);
		}

		return {};
	}

	inline TOptional<Value<true>> ValueIterator<true>::FindMatchingValue(const ValueIterator<false>& otherIterator) const
	{
		B3D_ASSERT(false); // Not supported at the moment. To support, we'd need to convert the otherIterator from a ISerialized object into fieldValue pointer we can lookup
		return {};
	}

	inline Value<false>::Value(u32 tupleElementIndex, const TShared<ISerialized>& value, FrameAllocator* frameAllocator)
		: mTupleElementIndex(tupleElementIndex), mValue(value), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 Value<false>::GetTupleElementCount() const
	{
		if(auto tuple = B3DRTTICast<SerializedTuple>(mValue))
			return (u32)tuple->Values.Size();
		
		return 1;
	}

	inline Value<false> Value<false>::GetTupleElement(u32 tupleElementIndex) const
	{
		if(!B3D_ENSURE(mTupleElementIndex == ~0u))
			return Value<false>(tupleElementIndex, nullptr, mFrameAllocator);

		TShared<ISerialized> value;
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
			else
				value = mValue;
		}

		return Value<false>(tupleElementIndex, value, mFrameAllocator);
	}

	inline Object<false> Value<false>::GetObject() const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return Object<false>(nullptr, mFrameAllocator);

		return Object<false>(static_cast<SerializedObject*>(mValue.get()), mFrameAllocator);
	}

	inline TShared<DataStream> Value<false>::GetDataStream(u32& size, u32& offset) const
	{
		auto* field = static_cast<SerializedDataBlock*>(mValue.get());
		size = field->Size;
		offset = field->Offset;

		return field->Stream;
	}

	inline u32 Value<false>::GetPlainSize() const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return 0;

		auto* field = static_cast<SerializedPlainData*>(mValue.get());
		return field->Size;
	}

	inline bool Value<false>::ComparePlain(const Value<false>& other) const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return false;

		auto* curFieldData = static_cast<SerializedPlainData*>(mValue.get());
		auto* otherFieldData = static_cast<SerializedPlainData*>(other.mValue.get());

		bool isModified = curFieldData->Size != otherFieldData->Size;
		if(!isModified)
			isModified = memcmp(curFieldData->Value, otherFieldData->Value, otherFieldData->Size) != 0;

		return isModified;
	}

	inline bool Value<false>::ComparePlain(const Value<true>& other) const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return false;

		auto* curFieldData = static_cast<SerializedPlainData*>(mValue.get());

		u32 otherTypeSize = other.GetPlainSize();

		auto buffer = B3DStackAllocate<u8>(otherTypeSize);
		other.GetPlainData(buffer, otherTypeSize);

		bool isModified = curFieldData->Size != otherTypeSize;
		if(!isModified)
			isModified = memcmp(curFieldData->Value, buffer, otherTypeSize) != 0;

		return isModified;
	}

	inline TShared<ISerialized> Value<false>::Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const
	{
		if(mValue != nullptr)
			return mValue->Clone();

		return nullptr;
	}

	inline Value<true>::Value(RTTIField* field, u32 tupleElementIndex, const TShared<IRTTIIterator>& iterator, RTTIType* rttiTypeInstance, IReflectable* object, FrameAllocator* frameAllocator)
		: mField(field), mTupleElementIndex(tupleElementIndex), mIterator(iterator), mRTTITypeInstance(rttiTypeInstance), mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(iterator != nullptr);
		B3D_ASSERT(rttiTypeInstance != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline Value<true>::Value(RTTIField* field, u32 tupleElementIndex, u32 arrayIndex, RTTIType* rttiTypeInstance, IReflectable* object, FrameAllocator* frameAllocator)
		: mField(field), mTupleElementIndex(tupleElementIndex), mArrayIndex(arrayIndex), mRTTITypeInstance(rttiTypeInstance), mObject(object), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(field != nullptr);
		B3D_ASSERT(rttiTypeInstance != nullptr);
		B3D_ASSERT(object != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline u32 Value<true>::GetTupleElementCount() const
	{
		if(mTupleElementIndex != ~0u)
			return 1;

		return (u32)mField->Schema.FieldDataTypes.Size();
	}

	inline Value<true> Value<true>::GetTupleElement(u32 tupleElementIndex) const
	{
		if(!B3D_ENSURE(mTupleElementIndex == ~0u))
			return Value<true>(mField, ~0u, nullptr, mRTTITypeInstance, mObject, mFrameAllocator);

		if(mIterator != nullptr)
			return Value<true>(mField, tupleElementIndex, mIterator, mRTTITypeInstance, mObject, mFrameAllocator);
		else
			return Value<true>(mField, tupleElementIndex, mArrayIndex, mRTTITypeInstance, mObject, mFrameAllocator);
	}

	inline Object<true> Value<true>::GetObject() const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return Object<true>(nullptr, nullptr, mFrameAllocator);

		if(!B3D_ENSURE(mTupleElementIndex < mField->Schema.FieldDataTypes.Size()))
			return Object<true>(nullptr, nullptr, mFrameAllocator);

		const RTTIFieldDataTypeSchema& fieldTypeSchema = mField->Schema.FieldDataTypes[mTupleElementIndex];
		if(!B3D_ENSURE(mIterator != nullptr))
			return Object<true>(nullptr, nullptr, mFrameAllocator);

		auto* field = static_cast<RTTIIteratorField*>(mField);
		const void* fieldValue = field->GetIteratorValue(mRTTITypeInstance, mObject, *mFrameAllocator, *mIterator);

		if(fieldTypeSchema.Type == RTTIFieldDataType::ReflectablePointer)
		{
			TShared<IReflectable> object = field->GetReflectablePointer(fieldValue, mTupleElementIndex);

			const u32 typeId = fieldTypeSchema.FieldTypeId;
			return Object<true>(object.get(), IReflectable::GetRTTITypeFromTypeId(typeId), mFrameAllocator);
		}
		else if(fieldTypeSchema.Type == RTTIFieldDataType::Reflectable)
		{
			const IReflectable& object = field->GetReflectable(fieldValue, mTupleElementIndex);

			const u32 typeId = fieldTypeSchema.FieldTypeId;
			return Object<true>(const_cast<IReflectable*>(&object), IReflectable::GetRTTITypeFromTypeId(typeId), mFrameAllocator);
		}

		B3D_ASSERT(false && "Invalid field type");
		return Object<true>(nullptr, nullptr, mFrameAllocator);
	}

	inline TShared<DataStream> Value<true>::GetDataStream(u32& size, u32& offset) const
	{
		auto* field = static_cast<RTTIDataBlockFieldBase*>(mField);

		TShared<DataStream> stream = field->GetValue(mRTTITypeInstance, mObject, size);
		offset = (u32)stream->Tell();

		return stream;
	}

	inline u32 Value<true>::GetPlainSize() const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return 0;

		if(!B3D_ENSURE(mIterator != nullptr))
			return 0;

		auto* field = static_cast<RTTIIteratorField*>(mField);
		const void* fieldValue = field->GetIteratorValue(mRTTITypeInstance, mObject, *mFrameAllocator, *mIterator);

		return field->GetPlainTypeSize(fieldValue, mTupleElementIndex, false).Bytes;
	}

	inline void Value<true>::GetPlainData(u8* buffer, u32 bufferSize) const
	{
		if(!B3D_ENSURE(mTupleElementIndex != ~0u))
			return;

		if(!B3D_ENSURE(mIterator != nullptr))
			return;

		auto* field = static_cast<RTTIIteratorField*>(mField);
		const void* fieldValue = field->GetIteratorValue(mRTTITypeInstance, mObject, *mFrameAllocator, *mIterator);
		Bitstream tempStream(buffer, bufferSize);

		field->WritePlainTypeTupleToStream(fieldValue, mTupleElementIndex, tempStream, false);
	}

	inline bool Value<true>::ComparePlain(const Value<true>& other) const
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

	inline bool Value<true>::ComparePlain(const Value<false>& other) const
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

	inline TShared<ISerialized> Value<true>::Clone(SerializedObjectEncodeFlags flags, RTTIOperationContext& context) const
	{
		auto* field = static_cast<RTTIIteratorField*>(mField);

		IntermediateSerializer intermediateSerializer(mFrameAllocator, context);

		if(mIterator != nullptr)
		{
			if(mTupleElementIndex != ~0u)
				return intermediateSerializer.SerializeTupleElement(*mObject, *mRTTITypeInstance, *field, *mIterator, mTupleElementIndex, flags);

			return intermediateSerializer.SerializeElement(*mObject, *mRTTITypeInstance, *field, *mIterator, flags);
		}

		return intermediateSerializer.SerializeDataBlockField(mObject, mRTTITypeInstance, mField, flags);
	}

	inline FieldIterator<false>::FieldIterator(SerializedObject* value, u32 subObjectIndex, FrameAllocator* frameAllocator)
		: mValue(value), mSubObjectIndex(subObjectIndex), mRTTIType(IReflectable::GetRTTITypeFromTypeId(value->SubObjects[subObjectIndex].TypeId)), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(value != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline bool FieldIterator<false>::MoveNext()
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

	inline Field<false> FieldIterator<false>::GetValue() const
	{
		return Field<false>(mFieldIterator->first, mFieldIterator->second.Value, mFrameAllocator);
	}

	inline FieldIterator<true>::FieldIterator(RTTIType* rttiType, RTTIType* rttiTypeInstance, IReflectable* value, FrameAllocator* frameAllocator)
		: mValue(value), mRTTIType(rttiType), mRTTITypeInstance(rttiTypeInstance), mFrameAllocator(frameAllocator)
	{
		B3D_ASSERT(rttiType != nullptr);
		B3D_ASSERT(value != nullptr);
		B3D_ASSERT(frameAllocator != nullptr);
	}

	inline bool FieldIterator<true>::MoveNext()
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

	inline Field<true> FieldIterator<true>::GetValue() const
	{
		RTTIField* field = mRTTIType->GetField(mFieldIndex);

		return Field<true>(mRTTITypeInstance, field, mValue, mFrameAllocator);
	}

	template<bool IsIReflectable, typename Predicate>
	void IterateFields(Object<IsIReflectable> object, RTTIOperationType operationType, Predicate&& fnPredicate)
	{
		if(object.GetWrappedObject() == nullptr)
			return;

		RTTIOperationContext rttiOperationContext;
		SubObjectIterator<IsIReflectable> subObjectIterator = object.GetSubObjectIterator();

		while(subObjectIterator.MoveNext())
		{
			SubObject<IsIReflectable> subObject = subObjectIterator.GetValue();

			RTTIType* rtti = IReflectable::GetRTTITypeFromTypeId(subObject.GetTypeId());
			if(!B3D_ENSURE(rtti != nullptr))
				continue;

			object.NotifyBeginOperation(subObject, operationType, rttiOperationContext);

			FieldIterator<IsIReflectable> fieldIterator = subObject.GetFieldIterator();
			while(fieldIterator.MoveNext())
			{
				Field<IsIReflectable> field = fieldIterator.GetValue();

				RTTIField* const rttiField = rtti->FindField(field.GetId());
				if(!B3D_ENSURE(rttiField != nullptr))
					continue;

				if(!fnPredicate(rttiField->Schema, field))
					break;
			}
		}

		object.NotifyEndOperation(operationType, rttiOperationContext);
	}

	template<bool IsIReflectable, typename Predicate, typename FieldFilterPredicate>
	void IterateFieldValues(Object<IsIReflectable> object, RTTIOperationType operationType, Predicate&& fnPredicate, FieldFilterPredicate&& fnFieldFilterPredicate)
	{
		IterateFields(object, operationType, [&fnPredicate, &fnFieldFilterPredicate](const RTTIFieldSchema& rttiFieldSchema, const Field<IsIReflectable>& field) {
			if(fnFieldFilterPredicate != nullptr && !fnFieldFilterPredicate(rttiFieldSchema))
				return false;

			ValueIterator<IsIReflectable> valueIterator = field.GetValueIterator();
			for(u32 elementIndex = 0; valueIterator.MoveNext(); ++elementIndex)
			{
				Value<IsIReflectable> value = valueIterator.GetValue();
				fnPredicate(rttiFieldSchema, value);
			}

			return true;
		});
	}

	template<bool IsIReflectable, typename Predicate, typename FieldFilterPredicate>
	void IterateFieldTupleValues(Object<IsIReflectable> object, RTTIOperationType operationType, Predicate&& fnPredicate, FieldFilterPredicate&& fnFieldFilterPredicate)
	{
		IterateFieldValues(object, operationType, [&fnPredicate](const RTTIFieldSchema& rttiFieldSchema, const Value<IsIReflectable>& value) {
			for(u32 tupleElementIndex = 0; tupleElementIndex < (u32)rttiFieldSchema.FieldDataTypes.Size(); ++tupleElementIndex)
			{
				const RTTIFieldDataTypeSchema& fieldTypeSchema = rttiFieldSchema.FieldDataTypes[tupleElementIndex];
				Value<IsIReflectable> tupleElement = value.GetTupleElement(tupleElementIndex);

				fnPredicate(fieldTypeSchema, tupleElement);
			}
		}, std::forward<FieldFilterPredicate>(fnFieldFilterPredicate));
	}
} // namespace RTTIWrapper
