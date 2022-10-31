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

namespace impl
{
/** Helper class that wraps either an IReflectable or a SerializedObject object instance. */
template <bool REFL>
class RTTIObjectWrapper
{};

/**
 * Helper class that provides information about a specific RTTIType of an either IReflectable
 * or a SerializedObject object instance. This is relevant for types that inherit from other
 * reflectable types.
 */
template <bool REFL>
class RTTISubObjectWrapper
{};

/** Helper class that iterates over all base RTTI types in a specific RTTI type. */
template <bool REFL>
struct RTTISubObjectWrapperIterator
{};

/**
 * Helper class that wraps a single RTTI field, that can be read either from a backing
 * IReflectable or SerializedObject object instance.
 */
template <bool REFL>
struct RTTIFieldWrapper
{};

/** Helper class that iterates over all fields in a RTTI type. */
template <bool REFL>
struct RTTIFieldWrapperIterator
{};

/** Provides information about a specific RTTIType of an object backed by SerializedObject. */
template <>
class RTTISubObjectWrapper<false>
{
public:
	RTTISubObjectWrapper() = default;
	RTTISubObjectWrapper(SerializedObject* obj, u32 subObjectIdx);

	/** Returns the type ID of the RTTIType. */
	u32 GetTypeId() const;

	/** Returns an iterator that will iterate over all fields in the RTTIType. */
	RTTIFieldWrapperIterator<false> GetFieldIter() const;

private:
	SerializedObject* mObj = nullptr;
	u32 mSubObjectIdx = 0;
};

/** Provides information about a specific RTTIType of an object backed by IReflectable. */
template <>
class RTTISubObjectWrapper<true>
{
public:
	RTTISubObjectWrapper() = default;
	RTTISubObjectWrapper(IReflectable* obj, RTTITypeBase* rttiType);

	/** Returns the type ID of the RTTIType. */
	u32 GetTypeId() const;

	/** Returns an iterator that will iterate over all fields in the RTTIType. */
	RTTIFieldWrapperIterator<true> GetFieldIter() const;

private:
	IReflectable* mObj = nullptr;
	RTTITypeBase* mRTTIType = nullptr;
};

/** Wraps a SerializedObject and allows you to retrieve information about its types. */
template <>
class RTTIObjectWrapper<false>
{
public:
	RTTIObjectWrapper(SerializedObject* obj);

	/** Returns the type ID of the root RTTIType. */
	u32 GetTypeId() const;

	/** Returns an iterator that will iterate over all the RTTITypes of the object. */
	RTTISubObjectWrapperIterator<false> GetSubObjectIterator() const;

	/** Returns the raw pointer to the underlying wrapped object. */
	IReflectable* GetObjectPtr() const { return mObj; }

private:
	SerializedObject* mObj;
};

/** Wraps a SerializedObject and allows you to retrieve information about its types. */
template <>
class RTTIObjectWrapper<true>
{
public:
	RTTIObjectWrapper(IReflectable* obj, RTTITypeBase* rttiType);

	/** Returns the type ID of the root RTTIType. */
	u32 GetTypeId() const;

	/** Returns an iterator that will iterate over all the RTTITypes of the object. */
	RTTISubObjectWrapperIterator<true> GetSubObjectIterator() const;

	/** Returns the raw pointer to the underlying wrapped object. */
	IReflectable* GetObjectPtr() const { return mObj; }

private:
	IReflectable* mObj;
	RTTITypeBase* mRTTIType;
};

/** Iterates over base RTTITypes of some type. */
template <>
struct RTTISubObjectWrapperIterator<false>
{
	RTTISubObjectWrapperIterator(SerializedObject* obj);

	/**
	 * Moves to the next base type and return false if no type was available (end was reached).
	 * Initially in before-start position and must be called once to read the first element.
	 */
	bool MoveNext();

	/**
	 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
	 * return true.
	 */
	RTTISubObjectWrapper<false> Value() const;

private:
	SerializedObject* mObj;
	u32 mBaseTypeIdx = (u32)-1;
};

/** Iterates over base RTTITypes of some type. */
template <>
struct RTTISubObjectWrapperIterator<true>
{
	RTTISubObjectWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj);

	/**
	 * Moves to the next base type and return false if no type was available (end was reached).
	 * Initially in before-start position and must be called once to read the first element.
	 */
	bool MoveNext();

	/**
	 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
	 * return true.
	 */
	RTTISubObjectWrapper<true> Value() const;

private:
	IReflectable* mObj;
	RTTITypeBase* mRTTIType;
	RTTITypeBase* mCurRTTIType = nullptr;
};

/** Wraps a single RTTIField and allows you to retrieve field information, values and compare it with other fields. */
template <>
struct RTTIFieldWrapper<false>
{
public:
	RTTIFieldWrapper() = default;
	RTTIFieldWrapper(u32 fieldId, SPtr<SerializedInstance> data);

	/** Returns the unique identifier of the field within a RTTIType. */
	u32 GetId() const;

	/** Returns the size of the array held by the field. Only valid if a field represents an array. */
	u32 GetArraySize() const;

	/**
	 * Returns a wrapper that holds information about specific element of an array. Only valid if this
	 * field represents an array.
	 */
	RTTIFieldWrapper<false> GetArrayElement(int index) const;

	/**
	 * Returns a wrapper that holds the object held by the field. Only valid if the field points to
	 * a reflectable type (pointer or otherwise).
	 */
	RTTIObjectWrapper<false> GetObject() const;

	/** Returns a data stream held by the field. Only valid if the field is a data block field. */
	SPtr<DataStream> GetDataStream(u32& size, u32& offset) const;

	/** Returns the size of the plain data in a field, in bytes. Only valid if the field holds a plain type. */
	u32 GetPlainSize() const;

	/** Compares the data between two plain fields and returns true if they're equal. */
	bool ComparePlain(const RTTIFieldWrapper<false>& other) const;

	/** Compares the data between two plain fields and returns true if they're equal. */
	bool ComparePlain(const RTTIFieldWrapper<true>& other) const;

	/** Clones the contents of this field and returns them as intermediate serialized data. */
	SPtr<SerializedInstance> Clone(SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc) const;

private:
	friend struct RTTIFieldWrapper<true>;

	u32 mId = 0;
	SPtr<SerializedInstance> mObj;
};

/** Wraps a single RTTIField and allows you to retrieve field information, values and compare it with other fields. */
template <>
struct RTTIFieldWrapper<true>
{
public:
	RTTIFieldWrapper() = default;
	RTTIFieldWrapper(RTTITypeBase* rttiType, RTTIField* field, bool isArrayElem, u32 arrayIdx, IReflectable* data);

	/** Returns the unique identifier of the field within a RTTIType. */
	u32 GetId() const;

	/** Returns the size of the array held by the field. Only valid if a field represents an array. */
	u32 GetArraySize() const;

	/**
	 * Returns a wrapper that holds information about specific element of an array. Only valid if this
	 * field represents an array.
	 */
	RTTIFieldWrapper<true> GetArrayElement(int index) const;

	/**
	 * Returns a wrapper that holds the object held by the field. Only valid if the field points to
	 * a reflectable type (pointer or otherwise).
	 */
	RTTIObjectWrapper<true> GetObject() const;

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
	bool ComparePlain(const RTTIFieldWrapper<false>& other) const;

	/** Compares the data between two plain fields and returns true if they're equal. */
	bool ComparePlain(const RTTIFieldWrapper<true>& other) const;

	/** Clones the contents of this field and returns them as intermediate serialized data. */
	SPtr<SerializedInstance> Clone(SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc) const;

private:
	friend struct RTTIFieldWrapper<false>;

	RTTITypeBase* mRTTIType = nullptr;
	RTTIField* mField = nullptr;
	bool mIsArrayElem = false;
	u32 mArrayIdx = 0;
	IReflectable* mObj = nullptr;
};

/** Iterates over all fields in a RTTIType. */
template <>
struct RTTIFieldWrapperIterator<false>
{
	RTTIFieldWrapperIterator(SerializedObject* obj, u32 subObjectIdx);

	/**
	 * Moves to the next field and return false if no field was available (end was reached).
	 * Initially in before-start position and must be called once to read the first element.
	 */
	bool MoveNext();

	/**
	 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
	 * return true.
	 */
	RTTIFieldWrapper<false> Value() const;

private:
	SerializedObject* mObj;
	u32 mSubObjectIdx;
	RTTITypeBase* mRTTIType;
	UnorderedMap<u32, SerializedEntry>::iterator mFieldIter;
	bool mIterSet = false;
};

/** Iterates over all fields in a RTTIType. */
template <>
struct RTTIFieldWrapperIterator<true>
{
	RTTIFieldWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj);

	/**
	 * Moves to the next field and return false if no field was available (end was reached).
	 * Initially in before-start position and must be called once to read the first element.
	 */
	bool MoveNext();

	/**
	 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
	 * return true.
	 */
	RTTIFieldWrapper<true> Value() const;

private:
	IReflectable* mObj;
	RTTITypeBase* mRTTIType;
	u32 mFieldIdx = (u32)-1;
};

RTTIObjectWrapper<false>::RTTIObjectWrapper(SerializedObject* obj)
	: mObj(obj)
{}

u32 RTTIObjectWrapper<false>::GetTypeId() const
{
	return mObj->GetRootTypeId();
}

RTTISubObjectWrapperIterator<false> RTTIObjectWrapper<false>::GetSubObjectIterator() const
{
	return RTTISubObjectWrapperIterator<false>(mObj);
}

RTTIObjectWrapper<true>::RTTIObjectWrapper(IReflectable* obj, RTTITypeBase* type)
	: mObj(obj), mRTTIType(type)
{}

u32 RTTIObjectWrapper<true>::GetTypeId() const
{
	return mObj->GetTypeId();
}

RTTISubObjectWrapperIterator<true> RTTIObjectWrapper<true>::GetSubObjectIterator() const
{
	return RTTISubObjectWrapperIterator<true>(mRTTIType, mObj);
}

RTTISubObjectWrapper<false>::RTTISubObjectWrapper(SerializedObject* obj, u32 subObjectIdx)
	: mObj(obj), mSubObjectIdx(subObjectIdx)
{}

u32 RTTISubObjectWrapper<false>::GetTypeId() const
{
	return mObj->SubObjects[mSubObjectIdx].TypeId;
}

RTTIFieldWrapperIterator<false> RTTISubObjectWrapper<false>::GetFieldIter() const
{
	return RTTIFieldWrapperIterator<false>(mObj, mSubObjectIdx);
}

RTTISubObjectWrapper<true>::RTTISubObjectWrapper(IReflectable* obj, RTTITypeBase* type)
	: mObj(obj), mRTTIType(type)
{}

u32 RTTISubObjectWrapper<true>::GetTypeId() const
{
	return mRTTIType->GetRttiId();
}

RTTIFieldWrapperIterator<true> RTTISubObjectWrapper<true>::GetFieldIter() const
{
	return RTTIFieldWrapperIterator<true>(mRTTIType, mObj);
}

RTTISubObjectWrapperIterator<false>::RTTISubObjectWrapperIterator(SerializedObject* obj)
	: mObj(obj)
{}

bool RTTISubObjectWrapperIterator<false>::MoveNext()
{
	u32 numFields = (u32)mObj->SubObjects.size();

	if(mBaseTypeIdx == (u32)-1)
	{
		if(numFields > 0)
		{
			mBaseTypeIdx = 0;
			return true;
		}

		return false;
	}

	if((mBaseTypeIdx + 1) < numFields)
	{
		mBaseTypeIdx++;
		return true;
	}

	return false;
}

RTTISubObjectWrapper<false> RTTISubObjectWrapperIterator<false>::Value() const
{
	return RTTISubObjectWrapper<false>(mObj, mBaseTypeIdx);
}

RTTISubObjectWrapperIterator<true>::RTTISubObjectWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj)
	: mObj(obj), mRTTIType(rttiType)
{}

bool RTTISubObjectWrapperIterator<true>::MoveNext()
{
	if(!mCurRTTIType)
	{
		mCurRTTIType = mRTTIType;
		return true;
	}
	else
	{
		mCurRTTIType = mCurRTTIType->GetBaseClass();
		return mCurRTTIType != nullptr;
	}
}

RTTISubObjectWrapper<true> RTTISubObjectWrapperIterator<true>::Value() const
{
	return RTTISubObjectWrapper<true>(mObj, mCurRTTIType);
}

RTTIFieldWrapper<false>::RTTIFieldWrapper(u32 fieldId, SPtr<SerializedInstance> data)
	: mId(fieldId), mObj(std::move(data))
{}

u32 RTTIFieldWrapper<false>::GetId() const
{
	return mId;
}

u32 RTTIFieldWrapper<false>::GetArraySize() const
{
	auto* field = static_cast<SerializedArray*>(mObj.get());
	return field->NumElements;
}

RTTIFieldWrapper<false> RTTIFieldWrapper<false>::GetArrayElement(int index) const
{
	auto* field = static_cast<SerializedArray*>(mObj.get());
	return RTTIFieldWrapper<false>(mId, field->Entries[index].Serialized);
}

RTTIObjectWrapper<false> RTTIFieldWrapper<false>::GetObject() const
{
	return RTTIObjectWrapper<false>(static_cast<SerializedObject*>(mObj.get()));
}

SPtr<DataStream> RTTIFieldWrapper<false>::GetDataStream(u32& size, u32& offset) const
{
	auto* field = static_cast<SerializedDataBlock*>(mObj.get());
	size = field->Size;
	offset = field->Offset;

	return field->Stream;
}

u32 RTTIFieldWrapper<false>::GetPlainSize() const
{
	auto* field = static_cast<SerializedField*>(mObj.get());
	return field->Size;
}

bool RTTIFieldWrapper<false>::ComparePlain(const RTTIFieldWrapper<false>& other) const
{
	auto* curFieldData = static_cast<SerializedField*>(mObj.get());
	auto* otherFieldData = static_cast<SerializedField*>(other.mObj.get());

	bool isModified = curFieldData->Size != otherFieldData->Size;
	if(!isModified)
		isModified = memcmp(curFieldData->Value, otherFieldData->Value, otherFieldData->Size) != 0;

	return isModified;
}

bool RTTIFieldWrapper<false>::ComparePlain(const RTTIFieldWrapper<true>& other) const
{
	auto* curFieldData = static_cast<SerializedField*>(mObj.get());

	u32 otherTypeSize = other.GetPlainSize();

	auto buffer = bs_stack_alloc<u8>(otherTypeSize);
	other.GetPlainData(buffer, otherTypeSize);

	bool isModified = curFieldData->Size != otherTypeSize;
	if(!isModified)
		isModified = memcmp(curFieldData->Value, buffer, otherTypeSize) != 0;

	return isModified;
}

SPtr<SerializedInstance> RTTIFieldWrapper<false>::Clone(SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc) const
{
	return mObj->Clone();
}

RTTIFieldWrapper<true>::RTTIFieldWrapper(RTTITypeBase* rttiType, RTTIField* field, bool isArrayElem, u32 arrayIdx, IReflectable* data)
	: mRTTIType(rttiType), mField(field), mIsArrayElem(isArrayElem), mArrayIdx(arrayIdx), mObj(data)
{}

u32 RTTIFieldWrapper<true>::GetId() const
{
	return mField->Schema.Id;
}

u32 RTTIFieldWrapper<true>::GetArraySize() const
{
	auto* field = static_cast<SerializedArray*>(mObj);
	return field->NumElements;
}

RTTIFieldWrapper<true> RTTIFieldWrapper<true>::GetArrayElement(int index) const
{
	return RTTIFieldWrapper<true>(mRTTIType, mField, true, index, mObj);
}

RTTIObjectWrapper<true> RTTIFieldWrapper<true>::GetObject() const
{
	if(mField->Schema.Type == SerializableFT_ReflectablePtr)
	{
		SPtr<IReflectable> obj;

		auto* field = static_cast<RTTIReflectablePtrFieldBase*>(mField);
		if(mIsArrayElem)
			obj = field->GetArrayValue(mRTTIType, mObj, mArrayIdx);
		else
			obj = field->GetValue(mRTTIType, mObj);

		return RTTIObjectWrapper<true>(obj.get(), field->GetType());
	}
	else if(mField->Schema.Type == SerializableFT_Reflectable)
	{
		IReflectable* obj;

		auto* field = static_cast<RTTIReflectableFieldBase*>(mField);
		if(mIsArrayElem)
			obj = &field->GetArrayValue(mRTTIType, mObj, mArrayIdx);
		else
			obj = &field->GetValue(mRTTIType, mObj);

		return RTTIObjectWrapper<true>(obj, field->GetType());
	}

	assert(false && "Invalid field type");
	return RTTIObjectWrapper<true>(nullptr, nullptr);
}

SPtr<DataStream> RTTIFieldWrapper<true>::GetDataStream(u32& size, u32& offset) const
{
	auto* field = static_cast<RTTIManagedDataBlockFieldBase*>(mField);

	SPtr<DataStream> stream = field->GetValue(mRTTIType, mObj, size);
	offset = (u32)stream->Tell();

	return stream;
}

u32 RTTIFieldWrapper<true>::GetPlainSize() const
{
	auto* field = static_cast<RTTIPlainFieldBase*>(mField);

	u32 size;
	if(field->Schema.HasDynamicSize)
	{
		if(mIsArrayElem)
			size = field->GetArrayElemDynamicSize(mRTTIType, mObj, mArrayIdx, false).Bytes;
		else
			size = field->GetDynamicSize(mRTTIType, mObj, false).Bytes;
	}
	else
		size = field->Schema.Size.Bytes;

	return size;
}

bool RTTIFieldWrapper<true>::ComparePlain(const RTTIFieldWrapper<true>& other) const
{
	u32 curTypeSize = GetPlainSize();

	auto curBuffer = bs_stack_alloc<u8>(curTypeSize);
	GetPlainData(curBuffer, curTypeSize);

	u32 otherTypeSize = other.GetPlainSize();

	auto otherBuffer = bs_stack_alloc<u8>(otherTypeSize);
	other.GetPlainData(otherBuffer, otherTypeSize);

	bool isModified = curTypeSize != otherTypeSize;
	if(!isModified)
		isModified = memcmp(curBuffer, otherBuffer, otherTypeSize) != 0;

	return isModified;
}

bool RTTIFieldWrapper<true>::ComparePlain(const RTTIFieldWrapper<false>& other) const
{
	auto* otherFieldData = static_cast<SerializedField*>(other.mObj.get());

	u32 curTypeSize = other.GetPlainSize();

	auto buffer = bs_stack_alloc<u8>(curTypeSize);
	GetPlainData(buffer, curTypeSize);

	bool isModified = otherFieldData->Size != curTypeSize;
	if(!isModified)
		isModified = memcmp(otherFieldData->Value, buffer, curTypeSize) != 0;

	return isModified;
}

void RTTIFieldWrapper<true>::GetPlainData(u8* buffer, u32 bufferSize) const
{
	auto* field = static_cast<RTTIPlainFieldBase*>(mField);
	Bitstream tempStream(buffer, bufferSize);

	if(mIsArrayElem)
		field->ArrayElemToStream(mRTTIType, mObj, mArrayIdx, tempStream);
	else
		field->ToStream(mRTTIType, mObj, tempStream);
}

SPtr<SerializedInstance> RTTIFieldWrapper<true>::Clone(SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc) const
{
	return IntermediateSerializer::EncodeFieldInternal(mObj, mRTTIType, mField, mArrayIdx, flags, context, alloc);
}

RTTIFieldWrapperIterator<false>::RTTIFieldWrapperIterator(SerializedObject* obj, u32 subObjectIdx)
	: mObj(obj), mSubObjectIdx(subObjectIdx), mRTTIType(IReflectable::GetRttifromTypeIdInternal(obj->SubObjects[subObjectIdx].TypeId))
{}

bool RTTIFieldWrapperIterator<false>::MoveNext()
{
	u32 numFields = mRTTIType->GetNumFields();

	if(!mIterSet)
	{
		mFieldIter = mObj->SubObjects[mSubObjectIdx].Entries.begin();
		mIterSet = true;
	}
	else
		++mFieldIter;

	return mFieldIter != mObj->SubObjects[mSubObjectIdx].Entries.end();
}

RTTIFieldWrapper<false> RTTIFieldWrapperIterator<false>::Value() const
{
	return RTTIFieldWrapper<false>(mFieldIter->first, mFieldIter->second.Serialized);
}

RTTIFieldWrapperIterator<true>::RTTIFieldWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj)
	: mObj(obj), mRTTIType(rttiType)
{}

bool RTTIFieldWrapperIterator<true>::MoveNext()
{
	u32 numFields = mRTTIType->GetNumFields();

	if(mFieldIdx == (u32)-1)
	{
		if(numFields > 0)
		{
			mFieldIdx = 0;
			return true;
		}

		return false;
	}

	if((mFieldIdx + 1) < numFields)
	{
		mFieldIdx++;
		return true;
	}

	return false;
}

RTTIFieldWrapper<true> RTTIFieldWrapperIterator<true>::Value() const
{
	RTTIField* field = mRTTIType->GetField(mFieldIdx);

	return RTTIFieldWrapper<true>(mRTTIType, field, false, 0, mObj);
}

typedef UnorderedMap<IReflectable*, SPtr<SerializedObject>> ObjectMap;

template <bool REFL_ORG, bool REFL_NEW>
SPtr<SerializedInstance> GenerateFieldDiff(RTTITypeBase* rtti, u32 fieldType, const RTTIFieldWrapper<REFL_ORG>& orgField, const RTTIFieldWrapper<REFL_NEW> newField, ObjectMap& objectMap, bool replicableOnly)
{
	SerializedObjectEncodeFlags flags = replicableOnly ? SerializedObjectEncodeFlag::ReplicableOnly : SerializedObjectEncodeFlags();
	SerializationContext* context = nullptr;
	FrameAlloc* alloc = &GetFrameAllocator();

	SPtr<SerializedInstance> modification;
	switch(fieldType)
	{
	case SerializableFT_ReflectablePtr:
	case SerializableFT_Reflectable:
		{
			RTTIObjectWrapper<REFL_ORG> orgObjData = orgField.GetObject();
			RTTIObjectWrapper<REFL_NEW> newObjData = newField.GetObject();

			auto iterFind = objectMap.find(newObjData.GetObjectPtr());
			if(iterFind != objectMap.end())
				modification = iterFind->second;
			else
			{
				RTTITypeBase* childRtti = nullptr;
				if(orgObjData.GetTypeId() == newObjData.GetTypeId())
					childRtti = IReflectable::GetRttifromTypeIdInternal(newObjData.GetTypeId());

				SPtr<SerializedObject> objectDiff;
				if(childRtti != nullptr)
				{
					IDiff& handler = childRtti->GetDiffHandler();
					objectDiff = handler.GenerateDiffInternal(orgObjData.GetObjectPtr(), newObjData.GetObjectPtr(), objectMap, replicableOnly);
				}

				if(objectDiff != nullptr)
					objectMap[newObjData.GetObjectPtr()] = objectDiff;

				modification = objectDiff;
			}
		}
		break;
	case SerializableFT_Plain:
		{
			if(orgField.ComparePlain(newField))
				modification = newField.Clone(flags, context, alloc);
		}
		break;
	case SerializableFT_DataBlock:
		{
			u32 orgFieldDataSize;
			u32 orgFieldOffset;
			SPtr<DataStream> orgFieldStream = orgField.GetDataStream(orgFieldDataSize, orgFieldOffset);

			u32 newFieldDataSize;
			u32 newFieldOffset;
			SPtr<DataStream> newFieldStream = newField.GetDataStream(newFieldDataSize, newFieldOffset);

			bool isModified = orgFieldDataSize != newFieldDataSize;
			if(!isModified)
			{
				u8* orgStreamData = nullptr;
				if(orgFieldStream->IsFile())
				{
					orgStreamData = (u8*)bs_stack_alloc(orgFieldDataSize);
					orgFieldStream->Seek(orgFieldOffset);
					orgFieldStream->Read(orgStreamData, orgFieldDataSize);
				}
				else
				{
					SPtr<MemoryDataStream> orgMemStream = std::static_pointer_cast<MemoryDataStream>(orgFieldStream);
					orgStreamData = orgMemStream->Cursor();
				}

				u8* newStreamData = nullptr;
				if(newFieldStream->IsFile())
				{
					newStreamData = (u8*)bs_stack_alloc(newFieldDataSize);
					newFieldStream->Seek(newFieldOffset);
					newFieldStream->Read(newStreamData, newFieldDataSize);
				}
				else
				{
					SPtr<MemoryDataStream> newMemStream = std::static_pointer_cast<MemoryDataStream>(newFieldStream);
					newStreamData = newMemStream->Cursor();
				}

				isModified = memcmp(orgStreamData, newStreamData, newFieldDataSize) != 0;

				if(newFieldStream->IsFile())
					bs_stack_free(newStreamData);

				if(orgFieldStream->IsFile())
					bs_stack_free(orgStreamData);
			}

			if(isModified)
				modification = newField.Clone(flags, context, alloc);
		}
		break;
	}

	return modification;
}

template <bool REFL_ORG, bool REFL_NEW>
SPtr<SerializedObject> GenerateDiff(RTTIObjectWrapper<REFL_ORG> orgObj, RTTIObjectWrapper<REFL_NEW> newObj, ObjectMap& objectMap, bool replicableOnly)
{
	SerializedObjectEncodeFlags flags = replicableOnly ? SerializedObjectEncodeFlag::ReplicableOnly : SerializedObjectEncodeFlags();
	SerializationContext* context = nullptr;
	FrameAlloc* alloc = &GetFrameAllocator();

	RTTISubObjectWrapperIterator<REFL_NEW> newObjTypeIter = newObj.GetSubObjectIterator();

	SPtr<SerializedObject> output;
	while(newObjTypeIter.MoveNext())
	{
		RTTISubObjectWrapper<REFL_NEW> newSubObject = newObjTypeIter.Value();

		RTTITypeBase* rtti = IReflectable::GetRttifromTypeIdInternal(newSubObject.GetTypeId());
		if(rtti == nullptr)
			continue;

		RTTISubObjectWrapper<REFL_ORG> orgSubObject;
		RTTISubObjectWrapperIterator<REFL_ORG> orgObjTypeIter = orgObj.GetSubObjectIterator();
		while(orgObjTypeIter.MoveNext())
		{
			RTTISubObjectWrapper<REFL_ORG> curSubObject = orgObjTypeIter.Value();
			if(curSubObject.GetTypeId() == newSubObject.GetTypeId())
			{
				orgSubObject = curSubObject;
				break;
			}
		}

		RTTIFieldWrapperIterator<REFL_NEW> newObjFieldIter = newSubObject.GetFieldIter();

		SerializedSubObject* diffSubObject = nullptr;
		while(newObjFieldIter.MoveNext())
		{
			RTTIFieldWrapper<REFL_NEW> newEntry = newObjFieldIter.Value();

			RTTIField* genericField = rtti->FindField(newEntry.GetId());
			if(genericField == nullptr)
				continue;

			if(replicableOnly)
			{
				if(!genericField->Schema.Info.Flags.IsSet(RTTIFieldFlag::Replicate))
					continue;
			}

			RTTIFieldWrapperIterator<REFL_ORG> orgObjFieldIter = orgSubObject.GetFieldIter();

			RTTIFieldWrapper<REFL_ORG> orgEntry;
			bool hasOrgEntry = false;
			while(orgObjFieldIter.MoveNext())
			{
				RTTIFieldWrapper<REFL_ORG> curEntry = orgObjFieldIter.Value();

				if(curEntry.GetId() == newEntry.GetId())
				{
					orgEntry = curEntry;
					hasOrgEntry = true;

					break;
				}
			}

			SPtr<SerializedInstance> modification;
			bool hasModification = false;
			if(genericField->Schema.IsArray)
			{
				SPtr<SerializedArray> serializedArray;

				if(hasOrgEntry)
				{
					// Check for new or different array entries
					u32 newNumArrayEntries = newEntry.GetArraySize();
					for(u32 i = 0; i < newNumArrayEntries; i++)
					{
						RTTIFieldWrapper<REFL_NEW> newArrayEntry = newEntry.GetArrayElement(i);

						SPtr<SerializedInstance> arrayModification;
						bool hasArrayModification;

						u32 orgNumArrayEntries = orgEntry.GetArraySize();
						if(i < orgNumArrayEntries)
						{
							RTTIFieldWrapper<REFL_ORG> orgArrayEntry = orgEntry.GetArrayElement(i);
							arrayModification = GenerateFieldDiff(rtti, genericField->Schema.Type, orgArrayEntry, newArrayEntry, objectMap, replicableOnly);
							hasArrayModification = arrayModification != nullptr;
						}
						else
						{
							arrayModification = std::static_pointer_cast<SerializedArray>(newArrayEntry.Clone(flags, context, alloc));
							hasArrayModification = true;
						}

						if(hasArrayModification)
						{
							if(serializedArray == nullptr)
							{
								serializedArray = bs_shared_ptr_new<SerializedArray>();
								serializedArray->NumElements = newEntry.GetArraySize();
							}

							SerializedArrayEntry arrayEntry;
							arrayEntry.Index = i;
							arrayEntry.Serialized = arrayModification;
							serializedArray->Entries[i] = arrayEntry;
						}
					}

					if(orgEntry.GetArraySize() != newEntry.GetArraySize())
					{
						if(serializedArray == nullptr)
						{
							serializedArray = bs_shared_ptr_new<SerializedArray>();
							serializedArray->NumElements = newEntry.GetArraySize();
						}
					}
				}
				else
				{
					serializedArray = std::static_pointer_cast<SerializedArray>(newEntry.Clone(flags, context, alloc));
				}

				modification = serializedArray;
				hasModification = modification != nullptr;
			}
			else
			{
				bool newEntryNull = false;
				if(genericField->Schema.Type == SerializableFT_ReflectablePtr)
					newEntryNull = newEntry.GetObject().GetObjectPtr() == nullptr;

				if(hasOrgEntry)
				{
					bool orgEntryNull = false;
					if(genericField->Schema.Type == SerializableFT_ReflectablePtr)
						orgEntryNull = orgEntry.GetObject().GetObjectPtr() == nullptr;

					if(!orgEntryNull)
					{
						if(!newEntryNull)
						{
							modification = GenerateFieldDiff(rtti, genericField->Schema.Type, orgEntry, newEntry, objectMap, replicableOnly);
							hasModification = modification != nullptr;
						}
						else
						{
							modification = nullptr;
							hasModification = true;
						}
					}
					else
					{
						if(!newEntryNull)
						{
							modification = newEntry.Clone(flags, context, alloc);
							hasModification = true;
						}
					}
				}
				else
				{
					if(!newEntryNull)
						modification = newEntry.Clone(flags, context, alloc);
					else
						modification = nullptr;

					hasModification = true;
				}
			}

			if(hasModification)
			{
				if(output == nullptr)
					output = bs_shared_ptr_new<SerializedObject>();

				if(diffSubObject == nullptr)
				{
					output->SubObjects.push_back(SerializedSubObject());
					diffSubObject = &output->SubObjects.back();
					diffSubObject->TypeId = rtti->GetRttiId();
				}

				SerializedEntry modificationEntry;
				modificationEntry.FieldId = genericField->Schema.Id;
				modificationEntry.Serialized = modification;
				diffSubObject->Entries[genericField->Schema.Id] = modificationEntry;
			}
		}
	}

	return output;
}
} // namespace impl

SPtr<SerializedObject> IDiff::GenerateDiff(const SPtr<IReflectable>& orgObj, const SPtr<IReflectable>& newObj, bool replicableOnly)
{
	ObjectMap objectMap;
	return GenerateDiffInternal(orgObj.get(), newObj.get(), objectMap, replicableOnly);
}

void IDiff::ApplyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, SerializationContext* context)
{
	FrameAlloc& alloc = GetFrameAllocator();
	alloc.MarkFrame();

	FrameVector<DiffCommand> commands;

	DiffObjectMap objectMap;
	ApplyDiff(object, diff, alloc, objectMap, commands, context);

	IReflectable* destObject = nullptr;
	RTTITypeBase* rttiInstance = nullptr;

	Stack<IReflectable*> objectStack;
	Vector<std::pair<RTTITypeBase*, IReflectable*>> rttiInstances;

	for(auto& command : commands)
	{
		bool isArray = (command.Type & Diff_ArrayFlag) != 0;
		DiffCommandType type = (DiffCommandType)(command.Type & 0xF);

		switch(type)
		{
		case Diff_ArraySize:
			command.Field->SetArraySize(rttiInstance, destObject, command.ArraySize);
			break;
		case Diff_ObjectStart:
			{
				destObject = command.Object.get();
				objectStack.push(destObject);

				FrameStack<RTTITypeBase*> rttiTypes;
				RTTITypeBase* curRtti = destObject->GetRtti();
				while(curRtti != nullptr)
				{
					rttiTypes.push(curRtti);
					curRtti = curRtti->GetBaseClass();
				}

				// Call base class first, followed by derived classes
				while(!rttiTypes.empty())
				{
					RTTITypeBase* curRtti = rttiTypes.top();
					RTTITypeBase* rttiInstance = curRtti->CloneInternal(alloc);

					rttiInstances.push_back(std::make_pair(rttiInstance, destObject));
					rttiInstance->OnDeserializationStarted(destObject, context);

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
					if(iter->second != destObject)
						break;

					if(iter->first->GetRttiId() == command.RttiType->GetRttiId())
						rttiInstance = iter->first;
				}

				assert(rttiInstance);
			}
			break;
		case Diff_ObjectEnd:
			{
				while(!rttiInstances.empty())
				{
					if(rttiInstances.back().second != destObject)
						break;

					RTTITypeBase* rttiInstance = rttiInstances.back().first;

					rttiInstance->OnDeserializationEnded(destObject, context);
					alloc.Destruct(rttiInstance);

					rttiInstances.erase(rttiInstances.end() - 1);
				}

				objectStack.pop();

				if(!objectStack.empty())
					destObject = objectStack.top();
				else
					destObject = nullptr;
			}
			break;
		default:
			break;
		}

		if(isArray)
		{
			switch(type)
			{
			case Diff_ReflectablePtr:
				{
					auto* field = static_cast<RTTIReflectablePtrFieldBase*>(command.Field);
					field->SetArrayValue(rttiInstance, destObject, command.ArrayIdx, command.Object);
				}
				break;
			case Diff_Reflectable:
				{
					auto* field = static_cast<RTTIReflectableFieldBase*>(command.Field);
					field->SetArrayValue(rttiInstance, destObject, command.ArrayIdx, *command.Object);
				}
				break;
			case Diff_Plain:
				{
					auto* field = static_cast<RTTIPlainFieldBase*>(command.Field);

					Bitstream tempStream(command.Value, command.Size);
					field->ArrayElemFromBuffer(rttiInstance, destObject, command.ArrayIdx, tempStream);
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
					field->SetValue(rttiInstance, destObject, command.Object);
				}
				break;
			case Diff_Reflectable:
				{
					auto* field = static_cast<RTTIReflectableFieldBase*>(command.Field);
					field->SetValue(rttiInstance, destObject, *command.Object);
				}
				break;
			case Diff_Plain:
				{
					auto* field = static_cast<RTTIPlainFieldBase*>(command.Field);

					Bitstream tempStream(command.Value, command.Size);
					field->FromBuffer(rttiInstance, destObject, tempStream);
				}
				break;
			case Diff_DataBlock:
				{
					auto* field = static_cast<RTTIManagedDataBlockFieldBase*>(command.Field);
					field->SetValue(rttiInstance, destObject, command.StreamValue, command.Size);
				}
				break;
			default:
				break;
			}
		}
	}

	alloc.Clear();
}

void IDiff::ApplyDiff(RTTITypeBase* rtti, const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context)
{
	IDiff& diffHandler = rtti->GetDiffHandler();
	diffHandler.ApplyDiff(object, diff, alloc, objectMap, diffCommands, context);
}

SPtr<SerializedObject> BinaryDiff::GenerateDiffInternal(IReflectable* orgObj, IReflectable* newObj, ObjectMap& objectMap, bool replicableOnly)
{
	if(orgObj->GetTypeId() == TID_SerializedObject)
	{
		::impl::RTTIObjectWrapper<false> orgObjWrapper(static_cast<SerializedObject*>(orgObj));

		if(newObj->GetTypeId() == TID_SerializedObject)
		{
			::impl::RTTIObjectWrapper<false> newObjWrapper(static_cast<SerializedObject*>(newObj));
			return ::impl::GenerateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
		}

		::impl::RTTIObjectWrapper<true> newObjWrapper(newObj, newObj->GetRtti());
		return ::impl::GenerateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
	}
	else
	{
		::impl::RTTIObjectWrapper<true> orgObjWrapper(orgObj, orgObj->GetRtti());

		if(newObj->GetTypeId() == TID_SerializedObject)
		{
			::impl::RTTIObjectWrapper<false> newObjWrapper(static_cast<SerializedObject*>(newObj));
			return ::impl::GenerateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
		}

		::impl::RTTIObjectWrapper<true> newObjWrapper(newObj, newObj->GetRtti());
		return ::impl::GenerateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
	}
}

void BinaryDiff::ApplyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context)
{
	if(object == nullptr || diff == nullptr || object->GetTypeId() != diff->GetRootTypeId())
		return;

	// Generate a list of commands per sub-object
	FrameVector<FrameVector<DiffCommand>> commandsPerSubObj;

	Stack<RTTITypeBase*> rttiInstances;
	for(auto& subObject : diff->SubObjects)
	{
		RTTITypeBase* rtti = IReflectable::GetRttifromTypeIdInternal(subObject.TypeId);
		if(rtti == nullptr)
			continue;

		if(!object->IsDerivedFrom(rtti))
			continue;

		RTTITypeBase* rttiInstance = rtti->CloneInternal(alloc);
		rttiInstance->OnSerializationStarted(object.get(), nullptr);
		rttiInstances.push(rttiInstance);

		FrameVector<DiffCommand> commands;

		DiffCommand subObjStartCommand;
		subObjStartCommand.RttiType = rtti;
		subObjStartCommand.Field = nullptr;
		subObjStartCommand.Type = Diff_SubObjectStart;

		commands.push_back(subObjStartCommand);

		for(auto& diffEntry : subObject.Entries)
		{
			RTTIField* genericField = rtti->FindField(diffEntry.first);
			if(genericField == nullptr)
				continue;

			SPtr<SerializedInstance> diffData = diffEntry.second.Serialized;

			if(genericField->Schema.IsArray)
			{
				SPtr<SerializedArray> diffArray = std::static_pointer_cast<SerializedArray>(diffData);

				u32 numArrayElements = diffArray->NumElements;

				DiffCommand arraySizeCommand;
				arraySizeCommand.Field = genericField;
				arraySizeCommand.Type = Diff_ArraySize | Diff_ArrayFlag;
				arraySizeCommand.ArraySize = numArrayElements;

				commands.push_back(arraySizeCommand);

				switch(genericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto* field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);

						u32 orgArraySize = genericField->GetArraySize(rttiInstance, object.get());
						for(auto& arrayElem : diffArray->Entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.Serialized);

							DiffCommand command;
							command.Field = genericField;
							command.Type = Diff_ReflectablePtr | Diff_ArrayFlag;
							command.ArrayIdx = arrayElem.first;

							if(arrayElemData == nullptr)
							{
								command.Object = nullptr;
								commands.push_back(command);
							}
							else
							{
								bool needsNewObject = arrayElem.first >= orgArraySize;

								if(!needsNewObject)
								{
									SPtr<IReflectable> childObj = field->GetArrayValue(rttiInstance, object.get(), arrayElem.first);
									if(childObj != nullptr)
									{
										IDiff::ApplyDiff(childObj->GetRtti(), childObj, arrayElemData, alloc, objectMap, commands, context);
										command.Object = childObj;
									}
									else
										needsNewObject = true;
								}

								if(needsNewObject)
								{
									RTTITypeBase* childRtti = IReflectable::GetRttifromTypeIdInternal(arrayElemData->GetRootTypeId());
									if(childRtti != nullptr)
									{
										auto findObj = objectMap.find(arrayElemData);
										if(findObj == objectMap.end())
										{
											SPtr<IReflectable> newObject = childRtti->NewRttiObject();
											findObj = objectMap.insert(std::make_pair(arrayElemData, newObject)).first;
										}

										IDiff::ApplyDiff(childRtti, findObj->second, arrayElemData, alloc, objectMap, commands, context);
										command.Object = findObj->second;
										commands.push_back(command);
									}
									else
									{
										command.Object = nullptr;
										commands.push_back(command);
									}
								}
							}
						}
					}
					break;
				case SerializableFT_Reflectable:
					{
						auto* field = static_cast<RTTIReflectableFieldBase*>(genericField);

						u32 orgArraySize = genericField->GetArraySize(rttiInstance, object.get());

						Vector<SPtr<IReflectable>> newArrayElements(numArrayElements);
						u32 minArrayLength = std::min(orgArraySize, numArrayElements);
						for(u32 i = 0; i < minArrayLength; i++)
						{
							IReflectable& childObj = field->GetArrayValue(rttiInstance, object.get(), i);
							newArrayElements[i] = BinaryCloner::Clone(&childObj, true);
						}

						for(auto& arrayElem : diffArray->Entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.Serialized);

							if(arrayElem.first < orgArraySize)
							{
								SPtr<IReflectable> childObj = newArrayElements[arrayElem.first];
								IDiff::ApplyDiff(childObj->GetRtti(), childObj, arrayElemData, alloc, objectMap, commands, context);
							}
							else
							{
								RTTITypeBase* childRtti = IReflectable::GetRttifromTypeIdInternal(arrayElemData->GetRootTypeId());
								if(childRtti != nullptr)
								{
									SPtr<IReflectable> newObject = childRtti->NewRttiObject();
									IDiff::ApplyDiff(childRtti, newObject, arrayElemData, alloc, objectMap, commands, context);

									newArrayElements[arrayElem.first] = newObject;
								}
							}
						}

						for(u32 i = 0; i < numArrayElements; i++)
						{
							DiffCommand command;
							command.Field = genericField;
							command.Type = Diff_Reflectable | Diff_ArrayFlag;
							command.ArrayIdx = i;
							command.Object = newArrayElements[i];

							commands.push_back(command);
						}
					}
					break;
				case SerializableFT_Plain:
					{
						for(auto& arrayElem : diffArray->Entries)
						{
							SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(arrayElem.second.Serialized);
							if(fieldData != nullptr)
							{
								DiffCommand command;
								command.Field = genericField;
								command.Type = Diff_Plain | Diff_ArrayFlag;
								command.Value = fieldData->Value;
								command.Size = fieldData->Size;
								command.ArrayIdx = arrayElem.first;

								commands.push_back(command);
							}
						}
					}
					break;
				default:
					break;
				}
			}
			else
			{
				switch(genericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto* field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(diffData);

						DiffCommand command;
						command.Field = genericField;
						command.Type = Diff_ReflectablePtr;

						if(fieldObjectData == nullptr)
							command.Object = nullptr;
						else
						{
							SPtr<IReflectable> childObj = field->GetValue(rttiInstance, object.get());
							if(childObj == nullptr)
							{
								RTTITypeBase* childRtti = IReflectable::GetRttifromTypeIdInternal(fieldObjectData->GetRootTypeId());
								if(childRtti != nullptr)
								{
									auto findObj = objectMap.find(fieldObjectData);
									if(findObj == objectMap.end())
									{
										SPtr<IReflectable> newObject = childRtti->NewRttiObject();
										findObj = objectMap.insert(std::make_pair(fieldObjectData, newObject)).first;
									}

									IDiff::ApplyDiff(childRtti, findObj->second, fieldObjectData, alloc, objectMap, commands, context);
									command.Object = findObj->second;
								}
								else
								{
									command.Object = nullptr;
								}
							}
							else
							{
								IDiff::ApplyDiff(childObj->GetRtti(), childObj, fieldObjectData, alloc, objectMap, commands, context);
								command.Object = childObj;
							}
						}

						commands.push_back(command);
					}
					break;
				case SerializableFT_Reflectable:
					{
						auto* field = static_cast<RTTIReflectableFieldBase*>(genericField);
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(diffData);

						IReflectable& childObj = field->GetValue(rttiInstance, object.get());
						SPtr<IReflectable> clonedObj = BinaryCloner::Clone(&childObj, true);

						IDiff::ApplyDiff(clonedObj->GetRtti(), clonedObj, fieldObjectData, alloc, objectMap, commands, context);

						DiffCommand command;
						command.Field = genericField;
						command.Type = Diff_Reflectable;
						command.Object = clonedObj;

						commands.push_back(command);
					}
					break;
				case SerializableFT_Plain:
					{
						SPtr<SerializedField> diffFieldData = std::static_pointer_cast<SerializedField>(diffData);

						if(diffFieldData->Size > 0)
						{
							DiffCommand command;
							command.Field = genericField;
							command.Type = Diff_Plain;
							command.Value = diffFieldData->Value;
							command.Size = diffFieldData->Size;

							commands.push_back(command);
						}
					}
					break;
				case SerializableFT_DataBlock:
					{
						SPtr<SerializedDataBlock> diffFieldData = std::static_pointer_cast<SerializedDataBlock>(diffData);

						DiffCommand command;
						command.Field = genericField;
						command.Type = Diff_DataBlock;
						command.StreamValue = diffFieldData->Stream;
						command.Value = nullptr;
						command.Size = diffFieldData->Size;

						commands.push_back(command);
					}
					break;
				}
			}
		}

		commandsPerSubObj.emplace_back(std::move(commands));
	}

	DiffCommand objStartCommand;
	objStartCommand.Field = nullptr;
	objStartCommand.Type = Diff_ObjectStart;
	objStartCommand.Object = object;

	diffCommands.push_back(objStartCommand);

	// Go in reverse because when deserializing we want to deserialize base first, and then derived types
	for(auto iter = commandsPerSubObj.rbegin(); iter != commandsPerSubObj.rend(); ++iter)
		diffCommands.insert(diffCommands.end(), iter->begin(), iter->end());

	DiffCommand objEndCommand;
	objEndCommand.Field = nullptr;
	objEndCommand.Type = Diff_ObjectEnd;
	objEndCommand.Object = object;

	diffCommands.push_back(objEndCommand);

	while(!rttiInstances.empty())
	{
		RTTITypeBase* rttiInstance = rttiInstances.top();
		rttiInstance->OnSerializationEnded(object.get(), nullptr);
		alloc.Destruct(rttiInstance);

		rttiInstances.pop();
	}
}
