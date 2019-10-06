//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsBinaryDiff.h"
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsBinaryCloner.h"
#include "Serialization/BsIntermediateSerializer.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	class Bitstream;
	struct SerializationContext;

	namespace impl
	{
		/** Helper class that wraps either an IReflectable or a SerializedObject object instance. */
		template <bool REFL>
		class RTTIObjectWrapper
		{ };

		/**
		 * Helper class that provides information about a specific RTTIType of an either IReflectable
		 * or a SerializedObject object instance. This is relevant for types that inherit from other
		 * reflectable types.
		 */
		template <bool REFL>
		class RTTISubObjectWrapper
		{ };

		/** Helper class that iterates over all base RTTI types in a specific RTTI type. */
		template<bool REFL>
		struct RTTISubObjectWrapperIterator
		{ };

		/**
		 * Helper class that wraps a single RTTI field, that can be read either from a backing
		 * IReflectable or SerializedObject object instance.
		 */
		template<bool REFL>
		struct RTTIFieldWrapper
		{ };

		/** Helper class that iterates over all fields in a RTTI type. */
		template<bool REFL>
		struct RTTIFieldWrapperIterator
		{ };

		/** Provides information about a specific RTTIType of an object backed by SerializedObject. */
		template <>
		class RTTISubObjectWrapper<false>
		{
		public:
			RTTISubObjectWrapper() = default;
			RTTISubObjectWrapper(SerializedObject* obj, UINT32 subObjectIdx);

			/** Returns the type ID of the RTTIType. */
			UINT32 getTypeId() const;

			/** Returns an iterator that will iterate over all fields in the RTTIType. */
			RTTIFieldWrapperIterator<false> getFieldIter() const;

		private:
			SerializedObject* mObj = nullptr;
			UINT32 mSubObjectIdx = 0;
		};

		/** Provides information about a specific RTTIType of an object backed by IReflectable. */
		template <>
		class RTTISubObjectWrapper<true>
		{
		public:
			RTTISubObjectWrapper() = default;
			RTTISubObjectWrapper(IReflectable* obj, RTTITypeBase* rttiType);

			/** Returns the type ID of the RTTIType. */
			UINT32 getTypeId() const;

			/** Returns an iterator that will iterate over all fields in the RTTIType. */
			RTTIFieldWrapperIterator<true> getFieldIter() const;

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
			UINT32 getTypeId() const;

			/** Returns an iterator that will iterate over all the RTTITypes of the object. */
			RTTISubObjectWrapperIterator<false> getSubObjectIterator() const;

			/** Returns the raw pointer to the underlying wrapped object. */
			IReflectable* getObjectPtr() const { return mObj; }

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
			UINT32 getTypeId() const;

			/** Returns an iterator that will iterate over all the RTTITypes of the object. */
			RTTISubObjectWrapperIterator<true> getSubObjectIterator() const;

			/** Returns the raw pointer to the underlying wrapped object. */
			IReflectable* getObjectPtr() const { return mObj; }

		private:
			IReflectable* mObj;
			RTTITypeBase* mRTTIType;
		};

		/** Iterates over base RTTITypes of some type. */
		template<>
		struct RTTISubObjectWrapperIterator<false>
		{
			RTTISubObjectWrapperIterator(SerializedObject* obj);

			/**
			 * Moves to the next base type and return false if no type was available (end was reached).
			 * Initially in before-start position and must be called once to read the first element.
			 */
			bool moveNext();

			/**
			 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
			 * return true.
			 */
			RTTISubObjectWrapper<false> value() const;

		private:
			SerializedObject* mObj;
			UINT32 mBaseTypeIdx = (UINT32)-1;
		};

		/** Iterates over base RTTITypes of some type. */
		template<>
		struct RTTISubObjectWrapperIterator<true>
		{
			RTTISubObjectWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj);

			/**
			 * Moves to the next base type and return false if no type was available (end was reached).
			 * Initially in before-start position and must be called once to read the first element.
			 */
			bool moveNext();

			/**
			 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
			 * return true.
			 */
			RTTISubObjectWrapper<true> value() const;

		private:
			IReflectable* mObj;
			RTTITypeBase* mRTTIType;
			RTTITypeBase* mCurRTTIType = nullptr;
		};

		/** Wraps a single RTTIField and allows you to retrieve field information, values and compare it with other fields. */
		template<>
		struct RTTIFieldWrapper<false>
		{
		public:
			RTTIFieldWrapper() = default;
			RTTIFieldWrapper(UINT32 fieldId, SPtr<SerializedInstance> data);

			/** Returns the unique identifier of the field within a RTTIType. */
			UINT32 getId() const;

			/** Returns the size of the array held by the field. Only valid if a field represents an array. */
			UINT32 getArraySize() const;

			/**
			 * Returns a wrapper that holds information about specific element of an array. Only valid if this
			 * field represents an array.
			 */
			RTTIFieldWrapper<false> getArrayElement(int index) const;

			/**
			 * Returns a wrapper that holds the object held by the field. Only valid if the field points to
			 * a reflectable type (pointer or otherwise).
			 */
			RTTIObjectWrapper<false> getObject() const;

			/** Returns a data stream held by the field. Only valid if the field is a data block field. */
			SPtr<DataStream> getDataStream(UINT32& size, UINT32& offset) const;

			/** Returns the size of the plain data in a field, in bytes. Only valid if the field holds a plain type. */
			UINT32 getPlainSize() const;

			/** Compares the data between two plain fields and returns true if they're equal. */
			bool comparePlain(const RTTIFieldWrapper<false>& other) const;

			/** Compares the data between two plain fields and returns true if they're equal. */
			bool comparePlain(const RTTIFieldWrapper<true>& other) const;

			/** Clones the contents of this field and returns them as intermediate serialized data. */
			SPtr<SerializedInstance> clone(SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc) const;
		private:
			friend struct RTTIFieldWrapper<true>;
			
			UINT32 mId = 0;
			SPtr<SerializedInstance> mObj;
		};

		/** Wraps a single RTTIField and allows you to retrieve field information, values and compare it with other fields. */
		template<>
		struct RTTIFieldWrapper<true>
		{
		public:
			RTTIFieldWrapper() = default;
			RTTIFieldWrapper(RTTITypeBase* rttiType, RTTIField* field, bool isArrayElem, UINT32 arrayIdx, IReflectable* data);

			/** Returns the unique identifier of the field within a RTTIType. */
			UINT32 getId() const;

			/** Returns the size of the array held by the field. Only valid if a field represents an array. */
			UINT32 getArraySize() const;

			/**
			 * Returns a wrapper that holds information about specific element of an array. Only valid if this
			 * field represents an array.
			 */
			RTTIFieldWrapper<true> getArrayElement(int index) const;

			/**
			 * Returns a wrapper that holds the object held by the field. Only valid if the field points to
			 * a reflectable type (pointer or otherwise).
			 */
			RTTIObjectWrapper<true> getObject() const;

			/** Returns a data stream held by the field. Only valid if the field is a data block field. */
			SPtr<DataStream> getDataStream(UINT32& size, UINT32& offset) const;

			/** Returns the size of the plain data in a field, in bytes. Only valid if the field holds a plain type. */
			UINT32 getPlainSize() const;

			/**
			 * Writes the data contained in the field into @p buffer. Caller must allocate the buffer and ensure it is of
			 * adequate size. Buffer size in bytes must be provided as @p bufferSize. Only valid if the field holds a plain
			 * type.
			 */
			void getPlainData(UINT8* buffer, UINT32 bufferSize) const;

			/** Compares the data between two plain fields and returns true if they're equal. */
			bool comparePlain(const RTTIFieldWrapper<false>& other) const;

			/** Compares the data between two plain fields and returns true if they're equal. */
			bool comparePlain(const RTTIFieldWrapper<true>& other) const;

			/** Clones the contents of this field and returns them as intermediate serialized data. */
			SPtr<SerializedInstance> clone(SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc) const;
		private:
			friend struct RTTIFieldWrapper<false>;
			
			RTTITypeBase* mRTTIType = nullptr;
			RTTIField* mField = nullptr;
			bool mIsArrayElem = false;
			UINT32 mArrayIdx = 0;
			IReflectable* mObj = nullptr;
		};

		/** Iterates over all fields in a RTTIType. */
		template<>
		struct RTTIFieldWrapperIterator<false>
		{
			RTTIFieldWrapperIterator(SerializedObject* obj, UINT32 subObjectIdx);

			/**
			 * Moves to the next field and return false if no field was available (end was reached).
			 * Initially in before-start position and must be called once to read the first element.
			 */
			bool moveNext();

			/**
			 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
			 * return true.
			 */
			RTTIFieldWrapper<false> value() const;

		private:
			SerializedObject* mObj;
			UINT32 mSubObjectIdx;
			UINT32 mFieldIdx = (UINT32)-1;
		};

		/** Iterates over all fields in a RTTIType. */
		template<>
		struct RTTIFieldWrapperIterator<true>
		{
			RTTIFieldWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj);

			/**
			 * Moves to the next field and return false if no field was available (end was reached).
			 * Initially in before-start position and must be called once to read the first element.
			 */
			bool moveNext();

			/**
			 * Returns the current value as pointer by the iterator. moveNext() must previously be called and
			 * return true.
			 */
			RTTIFieldWrapper<true> value() const;

		private:
			IReflectable* mObj;
			RTTITypeBase* mRTTIType;
			UINT32 mFieldIdx = (UINT32)-1;
		};

		RTTIObjectWrapper<false>::RTTIObjectWrapper(SerializedObject* obj)
			:mObj(obj)
		{ }

		UINT32 RTTIObjectWrapper<false>::getTypeId() const { return mObj->getRootTypeId(); }

		RTTISubObjectWrapperIterator<false> RTTIObjectWrapper<false>::getSubObjectIterator() const
		{
			return RTTISubObjectWrapperIterator<false>(mObj);
		}

		RTTIObjectWrapper<true>::RTTIObjectWrapper(IReflectable* obj, RTTITypeBase* type)
			:mObj(obj), mRTTIType(type)
		{ }

		UINT32 RTTIObjectWrapper<true>::getTypeId() const { return mObj->getTypeId(); }

		RTTISubObjectWrapperIterator<true> RTTIObjectWrapper<true>::getSubObjectIterator() const
		{
			return RTTISubObjectWrapperIterator<true>(mRTTIType, mObj);
		}

		RTTISubObjectWrapper<false>::RTTISubObjectWrapper(SerializedObject* obj, UINT32 subObjectIdx)
			:mObj(obj), mSubObjectIdx(subObjectIdx)
		{ }

		UINT32 RTTISubObjectWrapper<false>::getTypeId() const
		{
			return mObj->subObjects[mSubObjectIdx].typeId;
		}

		RTTIFieldWrapperIterator<false> RTTISubObjectWrapper<false>::getFieldIter() const
		{
			return RTTIFieldWrapperIterator<false>(mObj, mSubObjectIdx);
		}

		RTTISubObjectWrapper<true>::RTTISubObjectWrapper(IReflectable* obj, RTTITypeBase* type)
			: mObj(obj), mRTTIType(type)
		{ }

		UINT32 RTTISubObjectWrapper<true>::getTypeId() const
		{
			return mRTTIType->getRTTIId();
		}

		RTTIFieldWrapperIterator<true> RTTISubObjectWrapper<true>::getFieldIter() const
		{
			return RTTIFieldWrapperIterator<true>(mRTTIType, mObj);
		}

		RTTISubObjectWrapperIterator<false>::RTTISubObjectWrapperIterator(SerializedObject* obj)
			: mObj(obj)
		{ }

		bool RTTISubObjectWrapperIterator<false>::moveNext()
		{
			UINT32 numFields = (UINT32)mObj->subObjects.size();

			if (mBaseTypeIdx == (UINT32)-1)
			{
				if (numFields > 0)
				{
					mBaseTypeIdx = 0;
					return true;
				}

				return false;
			}

			if ((mBaseTypeIdx + 1) < numFields)
			{
				mBaseTypeIdx++;
				return true;
			}

			return false;
		}

		RTTISubObjectWrapper<false> RTTISubObjectWrapperIterator<false>::value() const
		{
			return RTTISubObjectWrapper<false>(mObj, mBaseTypeIdx);
		}

		RTTISubObjectWrapperIterator<true>::RTTISubObjectWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj)
			:mObj(obj), mRTTIType(rttiType)
		{ }

		bool RTTISubObjectWrapperIterator<true>::moveNext()
		{
			if (!mCurRTTIType)
			{
				mCurRTTIType = mRTTIType;
				return true;
			}
			else
			{
				mCurRTTIType = mCurRTTIType->getBaseClass();
				return mCurRTTIType != nullptr;
			}
		}

		RTTISubObjectWrapper<true> RTTISubObjectWrapperIterator<true>::value() const
		{
			return RTTISubObjectWrapper<true>(mObj, mCurRTTIType);
		}

		RTTIFieldWrapper<false>::RTTIFieldWrapper(UINT32 fieldId, SPtr<SerializedInstance> data)
			:mId(fieldId), mObj(std::move(data))
		{ }

		UINT32 RTTIFieldWrapper<false>::getId() const { return mId; }

		UINT32 RTTIFieldWrapper<false>::getArraySize() const
		{
			auto* field = static_cast<SerializedArray*>(mObj.get());
			return field->numElements;
		}

		RTTIFieldWrapper<false> RTTIFieldWrapper<false>::getArrayElement(int index) const
		{
			auto* field = static_cast<SerializedArray*>(mObj.get());
			return RTTIFieldWrapper<false>(mId, field->entries[index].serialized);
		}

		RTTIObjectWrapper<false> RTTIFieldWrapper<false>::getObject() const
		{
			return RTTIObjectWrapper<false>(static_cast<SerializedObject*>(mObj.get()));
		}

		SPtr<DataStream> RTTIFieldWrapper<false>::getDataStream(UINT32& size,  UINT32& offset) const
		{
			auto* field = static_cast<SerializedDataBlock*>(mObj.get());
			size = field->size;
			offset = field->offset;
			
			return field->stream;
		}

		UINT32 RTTIFieldWrapper<false>::getPlainSize() const
		{
			auto* field = static_cast<SerializedField*>(mObj.get());
			return field->size;
		}

		bool RTTIFieldWrapper<false>::comparePlain(const RTTIFieldWrapper<false>& other) const
		{
			auto* curFieldData = static_cast<SerializedField*>(mObj.get());
			auto* otherFieldData = static_cast<SerializedField*>(other.mObj.get());

			bool isModified = curFieldData->size != otherFieldData->size;
			if (!isModified)
				isModified = memcmp(curFieldData->value, otherFieldData->value, otherFieldData->size) != 0;

			return isModified;
		}

		bool RTTIFieldWrapper<false>::comparePlain(const RTTIFieldWrapper<true>& other) const
		{
			auto* curFieldData = static_cast<SerializedField*>(mObj.get());
			
			UINT32 otherTypeSize = other.getPlainSize();

			auto buffer = bs_stack_alloc<UINT8>(otherTypeSize);
			other.getPlainData(buffer, otherTypeSize);
			
			bool isModified = curFieldData->size != otherTypeSize;
			if (!isModified)
				isModified = memcmp(curFieldData->value, buffer, otherTypeSize) != 0;

			return isModified;
		}

		SPtr<SerializedInstance> RTTIFieldWrapper<false>::clone(SerializedObjectEncodeFlags flags,
			SerializationContext* context, FrameAlloc* alloc) const
		{
			return mObj->clone();
		}

		RTTIFieldWrapper<true>::RTTIFieldWrapper(RTTITypeBase* rttiType, RTTIField* field,
			bool isArrayElem, UINT32 arrayIdx, IReflectable* data)
			: mRTTIType(rttiType), mField(field), mIsArrayElem(isArrayElem), mArrayIdx(arrayIdx), mObj(data)
		{ }

		UINT32 RTTIFieldWrapper<true>::getId() const { return mField->schema.id; }

		UINT32 RTTIFieldWrapper<true>::getArraySize() const
		{
			auto* field = static_cast<SerializedArray*>(mObj);
			return field->numElements;
		}

		RTTIFieldWrapper<true> RTTIFieldWrapper<true>::getArrayElement(int index) const
		{
			return RTTIFieldWrapper<true>(mRTTIType, mField, true, index, mObj);
		}

		RTTIObjectWrapper<true> RTTIFieldWrapper<true>::getObject() const
		{
			if (mField->schema.type == SerializableFT_ReflectablePtr)
			{
				SPtr<IReflectable> obj;

				auto* field = static_cast<RTTIReflectablePtrFieldBase*>(mField);
				if (mIsArrayElem)
					obj = field->getArrayValue(mRTTIType, mObj, mArrayIdx);
				else
					obj = field->getValue(mRTTIType, mObj);

				return RTTIObjectWrapper<true>(obj.get(), field->getType());
			}
			else if (mField->schema.type == SerializableFT_Reflectable)
			{
				IReflectable* obj;

				auto* field = static_cast<RTTIReflectableFieldBase*>(mField);
				if (mIsArrayElem)
					obj = &field->getArrayValue(mRTTIType, mObj, mArrayIdx);
				else
					obj = &field->getValue(mRTTIType, mObj);

				return RTTIObjectWrapper<true>(obj, field->getType());
			}

			assert(false && "Invalid field type");
			return RTTIObjectWrapper<true>(nullptr, nullptr);
		}

		SPtr<DataStream> RTTIFieldWrapper<true>::getDataStream(UINT32& size, UINT32& offset) const
		{
			auto* field = static_cast<RTTIManagedDataBlockFieldBase*>(mField);

			SPtr<DataStream> stream = field->getValue(mRTTIType, mObj, size);
			offset = (UINT32)stream->tell();

			return stream;
		}

		UINT32 RTTIFieldWrapper<true>::getPlainSize() const
		{
			auto* field = static_cast<RTTIPlainFieldBase*>(mField);

			UINT32 size;
			if (field->schema.hasDynamicSize)
			{
				if(mIsArrayElem)
					size = field->getArrayElemDynamicSize(mRTTIType, mObj, mArrayIdx, false).bytes;
				else
					size = field->getDynamicSize(mRTTIType, mObj, false).bytes;
			}
			else
				size = field->schema.size.bytes;

			return size;
		}

		bool RTTIFieldWrapper<true>::comparePlain(const RTTIFieldWrapper<true>& other) const
		{
			UINT32 curTypeSize = getPlainSize();

			auto curBuffer = bs_stack_alloc<UINT8>(curTypeSize);
			getPlainData(curBuffer, curTypeSize);

			UINT32 otherTypeSize = other.getPlainSize();

			auto otherBuffer = bs_stack_alloc<UINT8>(otherTypeSize);
			other.getPlainData(otherBuffer, otherTypeSize);

			bool isModified = curTypeSize != otherTypeSize;
			if (!isModified)
				isModified = memcmp(curBuffer, otherBuffer, otherTypeSize) != 0;

			return isModified;
		}

		bool RTTIFieldWrapper<true>::comparePlain(const RTTIFieldWrapper<false>& other) const
		{
			auto* otherFieldData = static_cast<SerializedField*>(other.mObj.get());

			UINT32 curTypeSize = other.getPlainSize();

			auto buffer = bs_stack_alloc<UINT8>(curTypeSize);
			getPlainData(buffer, curTypeSize);

			bool isModified = otherFieldData->size != curTypeSize;
			if (!isModified)
				isModified = memcmp(otherFieldData->value, buffer, curTypeSize) != 0;

			return isModified;
		}

		void RTTIFieldWrapper<true>::getPlainData(UINT8* buffer, UINT32 bufferSize) const
		{
			auto* field = static_cast<RTTIPlainFieldBase*>(mField);
			Bitstream tempStream(buffer, bufferSize);

			if (mIsArrayElem)
				field->arrayElemToStream(mRTTIType, mObj, mArrayIdx, tempStream);
			else
				field->toStream(mRTTIType, mObj, tempStream);
		}

		SPtr<SerializedInstance> RTTIFieldWrapper<true>::clone(SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc) const
		{
			return IntermediateSerializer::_encodeField(mObj, mRTTIType, mField, mArrayIdx, flags, context, alloc);
		}

		RTTIFieldWrapperIterator<false>::RTTIFieldWrapperIterator(SerializedObject* obj, UINT32 subObjectIdx)
			:mObj(obj), mSubObjectIdx(subObjectIdx)
		{ }

		bool RTTIFieldWrapperIterator<false>::moveNext()
		{
			UINT32 numFields = mObj->getRTTI()->getNumFields();

			if (mFieldIdx == (UINT32)-1)
			{
				if (numFields > 0)
				{
					mFieldIdx = 0;
					return true;
				}

				return false;
			}

			if ((mFieldIdx + 1) < numFields)
			{
				mFieldIdx++;
				return true;
			}

			return false;
		}

		RTTIFieldWrapper<false> RTTIFieldWrapperIterator<false>::value() const
		{
			RTTIField* field = mObj->getRTTI()->getField(mFieldIdx);

			return RTTIFieldWrapper<false>(field->schema.id, mObj->subObjects[mSubObjectIdx].entries[mFieldIdx].serialized);
		}

		RTTIFieldWrapperIterator<true>::RTTIFieldWrapperIterator(RTTITypeBase* rttiType, IReflectable* obj)
			:mObj(obj), mRTTIType(rttiType)
		{ }

		bool RTTIFieldWrapperIterator<true>::moveNext()
		{
			UINT32 numFields = mObj->getRTTI()->getNumFields();

			if (mFieldIdx == (UINT32)-1)
			{
				if (numFields > 0)
				{
					mFieldIdx = 0;
					return true;
				}

				return false;
			}

			if ((mFieldIdx + 1) < numFields)
			{
				mFieldIdx++;
				return true;
			}

			return false;
		}

		RTTIFieldWrapper<true> RTTIFieldWrapperIterator<true>::value() const
		{
			RTTIField* field = mRTTIType->getField(mFieldIdx);

			return RTTIFieldWrapper<true>(mRTTIType, field, false, 0, mObj);
		}

		typedef UnorderedMap<IReflectable*, SPtr<SerializedObject>> ObjectMap;

		template<bool REFL_ORG, bool REFL_NEW>
		SPtr<SerializedInstance> generateFieldDiff(RTTITypeBase* rtti, UINT32 fieldType, const RTTIFieldWrapper<REFL_ORG>& orgField,
			const RTTIFieldWrapper<REFL_NEW> newField, ObjectMap& objectMap, bool replicableOnly)
		{
			SerializedObjectEncodeFlags flags = replicableOnly ? SerializedObjectEncodeFlag::ReplicableOnly : SerializedObjectEncodeFlags();
			SerializationContext* context = nullptr;
			FrameAlloc* alloc = &gFrameAlloc();
			
			SPtr<SerializedInstance> modification;
			switch (fieldType)
			{
			case SerializableFT_ReflectablePtr:
			case SerializableFT_Reflectable:
			{
				RTTIObjectWrapper<REFL_ORG> orgObjData = orgField.getObject();
				RTTIObjectWrapper<REFL_NEW> newObjData = newField.getObject();

				auto iterFind = objectMap.find(newObjData.getObjectPtr());
				if (iterFind != objectMap.end())
					modification = iterFind->second;
				else
				{
					RTTITypeBase* childRtti = nullptr;
					if (orgObjData.getTypeId() == newObjData.getTypeId())
						childRtti = IReflectable::_getRTTIfromTypeId(newObjData.getTypeId());

					SPtr<SerializedObject> objectDiff;
					if (childRtti != nullptr)
					{
						IDiff& handler = childRtti->getDiffHandler();
						objectDiff = handler._generateDiff(orgObjData.getObjectPtr(), newObjData.getObjectPtr(), objectMap, replicableOnly);
					}

					if (objectDiff != nullptr)
						objectMap[newObjData.getObjectPtr()] = objectDiff;

					modification = objectDiff;
				}
			}
			break;
			case SerializableFT_Plain:
			{
				if (orgField.comparePlain(newField))
					modification = newField.clone(flags, context, alloc);
			}
			break;
			case SerializableFT_DataBlock:
			{
				UINT32 orgFieldDataSize;
				UINT32 orgFieldOffset;
				SPtr<DataStream> orgFieldStream = orgField.getDataStream(orgFieldDataSize, orgFieldOffset);
					
				UINT32 newFieldDataSize;
				UINT32 newFieldOffset;
				SPtr<DataStream> newFieldStream = newField.getDataStream(newFieldDataSize, newFieldOffset);

				bool isModified = orgFieldDataSize != newFieldDataSize;
				if (!isModified)
				{
					UINT8* orgStreamData = nullptr;
					if (orgFieldStream->isFile())
					{
						orgStreamData = (UINT8*)bs_stack_alloc(orgFieldDataSize);
						orgFieldStream->seek(orgFieldOffset);
						orgFieldStream->read(orgStreamData, orgFieldDataSize);
					}
					else
					{
						SPtr<MemoryDataStream> orgMemStream = std::static_pointer_cast<MemoryDataStream>(orgFieldStream);
						orgStreamData = orgMemStream->cursor();
					}

					UINT8* newStreamData = nullptr;
					if (newFieldStream->isFile())
					{
						newStreamData = (UINT8*)bs_stack_alloc(newFieldDataSize);
						newFieldStream->seek(newFieldOffset);
						newFieldStream->read(newStreamData, newFieldDataSize);
					}
					else
					{
						SPtr<MemoryDataStream> newMemStream = std::static_pointer_cast<MemoryDataStream>(newFieldStream);
						newStreamData = newMemStream->cursor();
					}

					isModified = memcmp(orgStreamData, newStreamData, newFieldDataSize) != 0;

					if (newFieldStream->isFile())
						bs_stack_free(newStreamData);

					if (orgFieldStream->isFile())
						bs_stack_free(orgStreamData);
				}

				if (isModified)
					modification = newField.clone(flags, context, alloc);
			}
			break;
			}

			return modification;
		}

		template<bool REFL_ORG, bool REFL_NEW>
		SPtr<SerializedObject> generateDiff(RTTIObjectWrapper<REFL_ORG> orgObj,
			RTTIObjectWrapper<REFL_NEW> newObj, ObjectMap& objectMap, bool replicableOnly)
		{
			SerializedObjectEncodeFlags flags = replicableOnly ? SerializedObjectEncodeFlag::ReplicableOnly : SerializedObjectEncodeFlags();
			SerializationContext* context = nullptr;
			FrameAlloc* alloc = &gFrameAlloc();
			
			RTTISubObjectWrapperIterator<REFL_NEW> newObjTypeIter = newObj.getSubObjectIterator();

			SPtr<SerializedObject> output;
			while (newObjTypeIter.moveNext())
			{
				RTTISubObjectWrapper<REFL_NEW> newSubObject = newObjTypeIter.value();

				RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(newSubObject.getTypeId());
				if (rtti == nullptr)
					continue;

				RTTISubObjectWrapper<REFL_ORG> orgSubObject;
				RTTISubObjectWrapperIterator<REFL_ORG> orgObjTypeIter = orgObj.getSubObjectIterator();
				while (orgObjTypeIter.moveNext())
				{
					RTTISubObjectWrapper<REFL_ORG> curSubObject = orgObjTypeIter.value();
					if (curSubObject.getTypeId() == newSubObject.getTypeId())
					{
						orgSubObject = curSubObject;
						break;
					}
				}

				RTTIFieldWrapperIterator<REFL_NEW> newObjFieldIter = newSubObject.getFieldIter();

				SerializedSubObject* diffSubObject = nullptr;
				while (newObjFieldIter.moveNext())
				{
					RTTIFieldWrapper<REFL_NEW> newEntry = newObjFieldIter.value();

					RTTIField* genericField = rtti->findField(newEntry.getId());
					if (genericField == nullptr)
						continue;

					if (replicableOnly)
					{
						if (!genericField->schema.info.flags.isSet(RTTIFieldFlag::Replicate))
							continue;
					}

					RTTIFieldWrapperIterator<REFL_ORG> orgObjFieldIter = orgSubObject.getFieldIter();

					RTTIFieldWrapper<REFL_ORG> orgEntry;
					bool hasOrgEntry = false;
					while (orgObjFieldIter.moveNext())
					{
						RTTIFieldWrapper<REFL_ORG> curEntry = orgObjFieldIter.value();

						if (curEntry.getId() == newEntry.getId())
						{
							orgEntry = curEntry;
							hasOrgEntry = true;
							break;
						}
					}

					SPtr<SerializedInstance> modification;
					bool hasModification = false;
					if (genericField->schema.isArray)
					{
						SPtr<SerializedArray> serializedArray;

						if (hasOrgEntry)
						{
							// Check for new or different array entries
							UINT32 newNumArrayEntries = newEntry.getArraySize();
							for (UINT32 i = 0; i < newNumArrayEntries; i++)
							{
								RTTIFieldWrapper<REFL_NEW> newArrayEntry = newEntry.getArrayElement(i);

								SPtr<SerializedInstance> arrayModification;
								bool hasArrayModification;

								UINT32 orgNumArrayEntries = orgEntry.getArraySize();
								if (i < orgNumArrayEntries)
								{
									RTTIFieldWrapper<REFL_ORG> orgArrayEntry = orgEntry.getArrayElement(i);
									arrayModification = generateFieldDiff(rtti, genericField->schema.type,
										orgArrayEntry, newArrayEntry, objectMap, replicableOnly);
									hasArrayModification = arrayModification != nullptr;
								}
								else
								{
									arrayModification = std::static_pointer_cast<SerializedArray>(newArrayEntry.clone(flags, context, alloc));
									hasArrayModification = true;
								}

								if (hasArrayModification)
								{
									if (serializedArray == nullptr)
									{
										serializedArray = bs_shared_ptr_new<SerializedArray>();
										serializedArray->numElements = newEntry.getArraySize();
									}

									SerializedArrayEntry arrayEntry;
									arrayEntry.index = i;
									arrayEntry.serialized = arrayModification;
									serializedArray->entries[i] = arrayEntry;
								}
							}

							if (orgEntry.getArraySize() != newEntry.getArraySize())
							{
								if (serializedArray == nullptr)
								{
									serializedArray = bs_shared_ptr_new<SerializedArray>();
									serializedArray->numElements = newEntry.getArraySize();
								}
							}
						}
						else
						{
							serializedArray = std::static_pointer_cast<SerializedArray>(newEntry.clone(flags, context, alloc));
						}

						modification = serializedArray;
						hasModification = modification != nullptr;
					}
					else
					{
						if (hasOrgEntry)
						{
							modification = generateFieldDiff(rtti, genericField->schema.type, orgEntry, newEntry, objectMap, replicableOnly);
							hasModification = modification != nullptr;
						}
						else
						{
							modification = newEntry.clone(flags, context, alloc);
							hasModification = true;
						}
					}

					if (hasModification)
					{
						if (output == nullptr)
							output = bs_shared_ptr_new<SerializedObject>();

						if (diffSubObject == nullptr)
						{
							output->subObjects.push_back(SerializedSubObject());
							diffSubObject = &output->subObjects.back();
							diffSubObject->typeId = rtti->getRTTIId();
						}

						SerializedEntry modificationEntry;
						modificationEntry.fieldId = genericField->schema.id;
						modificationEntry.serialized = modification;
						diffSubObject->entries[genericField->schema.id] = modificationEntry;
					}
				}
			}

			return output;
		}
	}
	
	SPtr<SerializedObject> IDiff::generateDiff(const SPtr<IReflectable>& orgObj,
		const SPtr<IReflectable>& newObj, bool replicableOnly)
	{
		ObjectMap objectMap;
		return _generateDiff(orgObj.get(), newObj.get(), objectMap, replicableOnly);
	}

	void IDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff,
		SerializationContext* context)
	{
		FrameAlloc& alloc = gFrameAlloc();
		alloc.markFrame();

		FrameVector<DiffCommand> commands;

		DiffObjectMap objectMap;
		applyDiff(object, diff, alloc, objectMap, commands, context);

		IReflectable* destObject = nullptr;
		RTTITypeBase* rttiInstance = nullptr;

		Stack<IReflectable*> objectStack;
		Vector<std::pair<RTTITypeBase*, IReflectable*>> rttiInstances;

		for (auto& command : commands)
		{
			bool isArray = (command.type & Diff_ArrayFlag) != 0;
			DiffCommandType type = (DiffCommandType)(command.type & 0xF);

			switch (type)
			{
			case Diff_ArraySize:
				command.field->setArraySize(rttiInstance, destObject, command.arraySize);
				break;
			case Diff_ObjectStart:
			{
				destObject = command.object.get();
				objectStack.push(destObject);

				FrameStack<RTTITypeBase*> rttiTypes;
				RTTITypeBase* curRtti = destObject->getRTTI();
				while (curRtti != nullptr)
				{
					rttiTypes.push(curRtti);
					curRtti = curRtti->getBaseClass();
				}

				// Call base class first, followed by derived classes
				while(!rttiTypes.empty())
				{
					RTTITypeBase* curRtti = rttiTypes.top();
					RTTITypeBase* rttiInstance = curRtti->_clone(alloc);

					rttiInstances.push_back(std::make_pair(rttiInstance, destObject));
					rttiInstance->onDeserializationStarted(destObject, context);

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

						if(iter->first->getRTTIId() == command.rttiType->getRTTIId())
							rttiInstance = iter->first;
					}

					assert(rttiInstance);
				}
				break;
			case Diff_ObjectEnd:
			{
				while (!rttiInstances.empty())
				{
					if(rttiInstances.back().second != destObject)
						break;

					RTTITypeBase* rttiInstance = rttiInstances.back().first;

					rttiInstance->onDeserializationEnded(destObject, context);
					alloc.destruct(rttiInstance);

					rttiInstances.erase(rttiInstances.end() - 1);
				}

				objectStack.pop();

				if (!objectStack.empty())
					destObject = objectStack.top();
				else
					destObject = nullptr;
			}
				break;
			default:
				break;
			}

			if (isArray)
			{
				switch (type)
				{
				case Diff_ReflectablePtr:
				{
					auto* field = static_cast<RTTIReflectablePtrFieldBase*>(command.field);
					field->setArrayValue(rttiInstance, destObject, command.arrayIdx, command.object);
				}
					break;
				case Diff_Reflectable:
				{
					auto* field = static_cast<RTTIReflectableFieldBase*>(command.field);
					field->setArrayValue(rttiInstance, destObject, command.arrayIdx, *command.object);
				}
					break;
				case Diff_Plain:
				{
					auto* field = static_cast<RTTIPlainFieldBase*>(command.field);

					Bitstream tempStream(command.value, command.size);
					field->arrayElemFromBuffer(rttiInstance, destObject, command.arrayIdx, tempStream);
				}
					break;
				default:
					break;
				}
			}
			else
			{
				switch (type)
				{
				case Diff_ReflectablePtr:
				{
					auto* field = static_cast<RTTIReflectablePtrFieldBase*>(command.field);
					field->setValue(rttiInstance, destObject, command.object);
				}
					break;
				case Diff_Reflectable:
				{
					auto* field = static_cast<RTTIReflectableFieldBase*>(command.field);
					field->setValue(rttiInstance, destObject, *command.object);
				}
					break;
				case Diff_Plain:
				{
					auto* field = static_cast<RTTIPlainFieldBase*>(command.field);

					Bitstream tempStream(command.value, command.size);
					field->fromBuffer(rttiInstance, destObject, tempStream);
				}
					break;
				case Diff_DataBlock:
				{
					auto* field = static_cast<RTTIManagedDataBlockFieldBase*>(command.field);
					field->setValue(rttiInstance, destObject, command.streamValue, command.size);
				}
					break;
				default:
					break;
				}
			}
		}

		alloc.clear();
	}

	void IDiff::applyDiff(RTTITypeBase* rtti, const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff,
		FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context)
	{
		IDiff& diffHandler = rtti->getDiffHandler();
		diffHandler.applyDiff(object, diff, alloc, objectMap, diffCommands, context);
	}

	SPtr<SerializedObject> BinaryDiff::_generateDiff(IReflectable* orgObj,
		IReflectable* newObj, ObjectMap& objectMap, bool replicableOnly)
	{
		if(orgObj->getTypeId() == TID_SerializedObject)
		{
			impl::RTTIObjectWrapper<false> orgObjWrapper(static_cast<SerializedObject*>(orgObj));
			
			if(newObj->getTypeId() == TID_SerializedObject)
			{
				impl::RTTIObjectWrapper<false> newObjWrapper(static_cast<SerializedObject*>(newObj));
				return impl::generateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
			}

			impl::RTTIObjectWrapper<true> newObjWrapper(newObj, newObj->getRTTI());
			return impl::generateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
		}
		else
		{
			impl::RTTIObjectWrapper<true> orgObjWrapper(orgObj, orgObj->getRTTI());
			
			if(newObj->getTypeId() == TID_SerializedObject)
			{
				impl::RTTIObjectWrapper<false> newObjWrapper(static_cast<SerializedObject*>(newObj));
				return impl::generateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
			}

			impl::RTTIObjectWrapper<true> newObjWrapper(newObj, newObj->getRTTI());
			return impl::generateDiff(orgObjWrapper, newObjWrapper, objectMap, replicableOnly);
		}
	}

	void BinaryDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff,
		FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context)
	{
		if (object == nullptr || diff == nullptr || object->getTypeId() != diff->getRootTypeId())
			return;

		// Generate a list of commands per sub-object
		FrameVector<FrameVector<DiffCommand>> commandsPerSubObj;

		Stack<RTTITypeBase*> rttiInstances;
		for (auto& subObject : diff->subObjects)
		{
			RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
			if (rtti == nullptr)
				continue;

			if (!object->isDerivedFrom(rtti))
				continue;

			RTTITypeBase* rttiInstance = rtti->_clone(alloc);
			rttiInstance->onSerializationStarted(object.get(), nullptr);
			rttiInstances.push(rttiInstance);

			FrameVector<DiffCommand> commands;

			DiffCommand subObjStartCommand;
			subObjStartCommand.rttiType = rtti;
			subObjStartCommand.field = nullptr;
			subObjStartCommand.type = Diff_SubObjectStart;

			commands.push_back(subObjStartCommand);

			for (auto& diffEntry : subObject.entries)
			{
				RTTIField* genericField = rtti->findField(diffEntry.first);
				if (genericField == nullptr)
					continue;

				SPtr<SerializedInstance> diffData = diffEntry.second.serialized;

				if (genericField->schema.isArray)
				{
					SPtr<SerializedArray> diffArray = std::static_pointer_cast<SerializedArray>(diffData);

					UINT32 numArrayElements = diffArray->numElements;

					DiffCommand arraySizeCommand;
					arraySizeCommand.field = genericField;
					arraySizeCommand.type = Diff_ArraySize | Diff_ArrayFlag;
					arraySizeCommand.arraySize = numArrayElements;

					commands.push_back(arraySizeCommand);

					switch (genericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
					{
						auto* field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);

						UINT32 orgArraySize = genericField->getArraySize(rttiInstance, object.get());
						for (auto& arrayElem : diffArray->entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

							DiffCommand command;
							command.field = genericField;
							command.type = Diff_ReflectablePtr | Diff_ArrayFlag;
							command.arrayIdx = arrayElem.first;

							if (arrayElemData == nullptr)
							{
								command.object = nullptr;
								commands.push_back(command);
							}
							else
							{
								bool needsNewObject = arrayElem.first >= orgArraySize;

								if (!needsNewObject)
								{
									SPtr<IReflectable> childObj = field->getArrayValue(rttiInstance, object.get(), arrayElem.first);
									if (childObj != nullptr)
									{
										IDiff::applyDiff(childObj->getRTTI(), childObj, arrayElemData, alloc, objectMap,
											commands, context);
										command.object = childObj;
									}
									else
										needsNewObject = true;
								}

								if (needsNewObject)
								{
									RTTITypeBase* childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());
									if (childRtti != nullptr)
									{
										auto findObj = objectMap.find(arrayElemData);
										if (findObj == objectMap.end())
										{
											SPtr<IReflectable> newObject = childRtti->newRTTIObject();
											findObj = objectMap.insert(std::make_pair(arrayElemData, newObject)).first;
										}

										IDiff::applyDiff(childRtti, findObj->second, arrayElemData, alloc, objectMap,
											commands, context);
										command.object = findObj->second;
										commands.push_back(command);
									}
									else
									{
										command.object = nullptr;
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

						UINT32 orgArraySize = genericField->getArraySize(rttiInstance, object.get());

						Vector<SPtr<IReflectable>> newArrayElements(numArrayElements);
						UINT32 minArrayLength = std::min(orgArraySize, numArrayElements);
						for (UINT32 i = 0; i < minArrayLength; i++)
						{
							IReflectable& childObj = field->getArrayValue(rttiInstance, object.get(), i);
							newArrayElements[i] = BinaryCloner::clone(&childObj, true);
						}

						for (auto& arrayElem : diffArray->entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

							if (arrayElem.first < orgArraySize)
							{
								SPtr<IReflectable> childObj = newArrayElements[arrayElem.first];
								IDiff::applyDiff(childObj->getRTTI(), childObj, arrayElemData, alloc, objectMap,
									commands, context);
							}
							else
							{
								RTTITypeBase* childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());
								if (childRtti != nullptr)
								{
									SPtr<IReflectable> newObject = childRtti->newRTTIObject();
									IDiff::applyDiff(childRtti, newObject, arrayElemData, alloc, objectMap, commands,
										context);

									newArrayElements[arrayElem.first] = newObject;
								}
							}
						}

						for (UINT32 i = 0; i < numArrayElements; i++)
						{
							DiffCommand command;
							command.field = genericField;
							command.type = Diff_Reflectable | Diff_ArrayFlag;
							command.arrayIdx = i;
							command.object = newArrayElements[i];

							commands.push_back(command);
						}
					}
						break;
					case SerializableFT_Plain:
					{
						for (auto& arrayElem : diffArray->entries)
						{
							SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(arrayElem.second.serialized);
							if (fieldData != nullptr)
							{
								DiffCommand command;
								command.field = genericField;
								command.type = Diff_Plain | Diff_ArrayFlag;
								command.value = fieldData->value;
								command.size = fieldData->size;
								command.arrayIdx = arrayElem.first;

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
					switch (genericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
					{
						auto* field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(diffData);

						DiffCommand command;
						command.field = genericField;
						command.type = Diff_ReflectablePtr;

						if (fieldObjectData == nullptr)
							command.object = nullptr;
						else
						{
							SPtr<IReflectable> childObj = field->getValue(rttiInstance, object.get());
							if (childObj == nullptr)
							{
								RTTITypeBase* childRtti = IReflectable::_getRTTIfromTypeId(fieldObjectData->getRootTypeId());
								if (childRtti != nullptr)
								{
									auto findObj = objectMap.find(fieldObjectData);
									if (findObj == objectMap.end())
									{
										SPtr<IReflectable> newObject = childRtti->newRTTIObject();
										findObj = objectMap.insert(std::make_pair(fieldObjectData, newObject)).first;
									}

									IDiff::applyDiff(childRtti, findObj->second, fieldObjectData, alloc, objectMap,
										commands, context);
									command.object = findObj->second;
								}
								else
								{
									command.object = nullptr;
								}
							}
							else
							{
								IDiff::applyDiff(childObj->getRTTI(), childObj, fieldObjectData, alloc, objectMap,
									commands, context);
								command.object = childObj;
							}
						}

						commands.push_back(command);
					}
						break;
					case SerializableFT_Reflectable:
					{
						auto* field = static_cast<RTTIReflectableFieldBase*>(genericField);
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(diffData);

						IReflectable& childObj = field->getValue(rttiInstance, object.get());
						SPtr<IReflectable> clonedObj = BinaryCloner::clone(&childObj, true);

						IDiff::applyDiff(clonedObj->getRTTI(), clonedObj, fieldObjectData, alloc, objectMap, commands,
							context);

						DiffCommand command;
						command.field = genericField;
						command.type = Diff_Reflectable;
						command.object = clonedObj;

						commands.push_back(command);
					}
						break;
					case SerializableFT_Plain:
					{
						SPtr<SerializedField> diffFieldData = std::static_pointer_cast<SerializedField>(diffData);

						if (diffFieldData->size > 0)
						{
							DiffCommand command;
							command.field = genericField;
							command.type = Diff_Plain;
							command.value = diffFieldData->value;
							command.size = diffFieldData->size;

							commands.push_back(command);
						}
					}
						break;
					case SerializableFT_DataBlock:
					{
						SPtr<SerializedDataBlock> diffFieldData = std::static_pointer_cast<SerializedDataBlock>(diffData);

						DiffCommand command;
						command.field = genericField;
						command.type = Diff_DataBlock;
						command.streamValue = diffFieldData->stream;
						command.value = nullptr;
						command.size = diffFieldData->size;

						commands.push_back(command);
					}
						break;
					}
				}
			}

			commandsPerSubObj.emplace_back(std::move(commands));
		}

		DiffCommand objStartCommand;
		objStartCommand.field = nullptr;
		objStartCommand.type = Diff_ObjectStart;
		objStartCommand.object = object;

		diffCommands.push_back(objStartCommand);

		// Go in reverse because when deserializing we want to deserialize base first, and then derived types
		for(auto iter = commandsPerSubObj.rbegin(); iter != commandsPerSubObj.rend(); ++iter)
			diffCommands.insert(diffCommands.end(), iter->begin(), iter->end());

		DiffCommand objEndCommand;
		objEndCommand.field = nullptr;
		objEndCommand.type = Diff_ObjectEnd;
		objEndCommand.object = object;

		diffCommands.push_back(objEndCommand);

		while (!rttiInstances.empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.top();
			rttiInstance->onSerializationEnded(object.get(), nullptr);
			alloc.destruct(rttiInstance);

			rttiInstances.pop();
		}
	}
}
