//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsSerializedObject.h"
#include "Private/RTTI/BsSerializedObjectRTTI.h"

namespace bs
{
	namespace detail
	{
		/** Helper class for performing SerializedObject <-> IReflectable encoding & decoding. */
		class BS_UTILITY_EXPORT IntermediateSerializer
		{
		public:
			IntermediateSerializer();

			/**
			 * Encodes an IReflectable object into an intermediate representation.
			 *
			 * @param[in]	object		Object to encode.
			 * @param[in]	shallow		Determines how to handle referenced objects. If true then references will not be encoded
			 *							and will be set to null. If false then references will be encoded as well and restored
			 *							upon decoding.
			 */
			SPtr<SerializedObject> encode(IReflectable* object, bool shallow = false);

			/** Decodes an intermediate representation of a serialized object into the actual object. */
			SPtr<IReflectable> decode(const SerializedObject* serializedObject);

		private:
			struct ObjectToDecode
			{
				ObjectToDecode(const SPtr<IReflectable>& _object, const SerializedObject* serializedObject)
					:object(_object), serializedObject(serializedObject)
				{ }

				SPtr<IReflectable> object;
				const SerializedObject* serializedObject;
				bool isDecoded = false;
				bool decodeInProgress = false; // Used for error reporting circular references
			};

			/**	Decodes a single IReflectable object. */
			void decodeEntry(const SPtr<IReflectable>& object, const SerializedObject* serializableObject);

			/** Encodes a single IReflectable object. */
			SPtr<SerializedObject> encodeEntry(IReflectable* object, bool shallow);

			UnorderedMap<const SerializedObject*, ObjectToDecode> mObjectMap;
			UnorderedMap<String, UINT64> mParams;
			FrameAlloc* mAlloc = nullptr;
		};

		IntermediateSerializer::IntermediateSerializer()
			:mAlloc(&gFrameAlloc())
		{ }

		SPtr<IReflectable> IntermediateSerializer::decode(const SerializedObject* serializedObject)
		{
			mAlloc->markFrame();
			mObjectMap.clear();

			SPtr<IReflectable> output;
			RTTITypeBase* type = IReflectable::_getRTTIfromTypeId(serializedObject->getRootTypeId());
			if (type != nullptr)
			{
				output = type->newRTTIObject();
				auto iterNewObj = mObjectMap.insert(std::make_pair(serializedObject, ObjectToDecode(output, serializedObject)));

				iterNewObj.first->second.decodeInProgress = true;
				decodeEntry(output, serializedObject);
				iterNewObj.first->second.decodeInProgress = false;
				iterNewObj.first->second.isDecoded = true;
			}

			// Go through the remaining objects (should be only ones with weak refs)
			for (auto iter = mObjectMap.begin(); iter != mObjectMap.end(); ++iter)
			{
				ObjectToDecode& objToDecode = iter->second;

				if (objToDecode.isDecoded)
					continue;

				objToDecode.decodeInProgress = true;
				decodeEntry(objToDecode.object, objToDecode.serializedObject);
				objToDecode.decodeInProgress = false;
				objToDecode.isDecoded = true;
			}

			mObjectMap.clear();
			mAlloc->clear();

			return output;
		}

		SPtr<SerializedObject> IntermediateSerializer::encode(IReflectable* object, bool shallow)
		{
			return encodeEntry(object, shallow);
		}

		void IntermediateSerializer::decodeEntry(const SPtr<IReflectable>& object, const SerializedObject* serializableObject)
		{
			UINT32 numSubObjects = (UINT32)serializableObject->subObjects.size();
			if (numSubObjects == 0)
				return;

			FrameStack<RTTITypeBase*> rttiInstances;
			for (INT32 subObjectIdx = numSubObjects - 1; subObjectIdx >= 0; subObjectIdx--)
			{
				const SerializedSubObject& subObject = serializableObject->subObjects[subObjectIdx];

				RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
				if (rtti == nullptr)
					continue;

				RTTITypeBase* rttiInstance = rtti->_clone(*mAlloc);
				rttiInstance->onDeserializationStarted(object.get(), mParams);
				rttiInstances.push(rttiInstance);

				UINT32 numFields = rtti->getNumFields();
				for (UINT32 fieldIdx = 0; fieldIdx < numFields; fieldIdx++)
				{
					RTTIField* curGenericField = rtti->getField(fieldIdx);

					auto iterFindFieldData = subObject.entries.find(curGenericField->mUniqueId);
					if (iterFindFieldData == subObject.entries.end())
						continue;

					SPtr<SerializedInstance> entryData = iterFindFieldData->second.serialized;
					if (curGenericField->isArray())
					{
						SPtr<SerializedArray> arrayData = std::static_pointer_cast<SerializedArray>(entryData);

						UINT32 arrayNumElems = (UINT32)arrayData->numElements;
						curGenericField->setArraySize(rttiInstance, object.get(), arrayNumElems);

						switch (curGenericField->mType)
						{
						case SerializableFT_ReflectablePtr:
						{
							RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

							for (auto& arrayElem : arrayData->entries)
							{
								SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);
								RTTITypeBase* childRtti = nullptr;

								if (arrayElemData != nullptr)
									childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());

								if (childRtti != nullptr)
								{
									auto findObj = mObjectMap.find(arrayElemData.get());
									if (findObj == mObjectMap.end())
									{
										SPtr<IReflectable> newObject = childRtti->newRTTIObject();
										findObj = mObjectMap.insert(std::make_pair(arrayElemData.get(), 
											ObjectToDecode(newObject, arrayElemData.get()))).first;
									}

									ObjectToDecode& objToDecode = findObj->second;

									bool needsDecoding = (curField->getFlags() & RTTI_Flag_WeakRef) == 0 && !objToDecode.isDecoded;
									if (needsDecoding)
									{
										if (objToDecode.decodeInProgress)
										{
											LOGWRN("Detected a circular reference when decoding. Referenced object fields " \
												"will be resolved in an undefined order (i.e. one of the objects will not " \
												"be fully deserialized when assigned to its field). Use RTTI_Flag_WeakRef to " \
												"get rid of this warning and tell the system which of the objects is allowed " \
												"to be deserialized after it is assigned to its field.");
										}
										else
										{
											objToDecode.decodeInProgress = true;
											decodeEntry(objToDecode.object, objToDecode.serializedObject);
											objToDecode.decodeInProgress = false;
											objToDecode.isDecoded = true;
										}
									}

									curField->setArrayValue(rttiInstance, object.get(), arrayElem.first, objToDecode.object);
								}
								else
								{
									curField->setArrayValue(rttiInstance, object.get(), arrayElem.first, nullptr);
								}
							}
						}
						break;
						case SerializableFT_Reflectable:
						{
							RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

							for (auto& arrayElem : arrayData->entries)
							{
								SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);
								RTTITypeBase* childRtti = nullptr;

								if (arrayElemData != nullptr)
									childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());

								if (childRtti != nullptr)
								{
									SPtr<IReflectable> newObject = childRtti->newRTTIObject();
									decodeEntry(newObject, arrayElemData.get());
									curField->setArrayValue(rttiInstance, object.get(), arrayElem.first, *newObject);
								}
							}
							break;
						}
						case SerializableFT_Plain:
						{
							RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

							for (auto& arrayElem : arrayData->entries)
							{
								SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(arrayElem.second.serialized);
								if (fieldData != nullptr)
								{
									curField->arrayElemFromBuffer(rttiInstance, object.get(), arrayElem.first, fieldData->value);
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
						switch (curGenericField->mType)
						{
						case SerializableFT_ReflectablePtr:
						{
							RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

							SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
							RTTITypeBase* childRtti = nullptr;

							if (fieldObjectData != nullptr)
								childRtti = IReflectable::_getRTTIfromTypeId(fieldObjectData->getRootTypeId());

							if (childRtti != nullptr)
							{
								auto findObj = mObjectMap.find(fieldObjectData.get());
								if (findObj == mObjectMap.end())
								{
									SPtr<IReflectable> newObject = childRtti->newRTTIObject();
									findObj = mObjectMap.insert(std::make_pair(fieldObjectData.get(), 
										ObjectToDecode(newObject, fieldObjectData.get()))).first;
								}

								ObjectToDecode& objToDecode = findObj->second;

								bool needsDecoding = (curField->getFlags() & RTTI_Flag_WeakRef) == 0 && !objToDecode.isDecoded;
								if (needsDecoding)
								{
									if (objToDecode.decodeInProgress)
									{
										LOGWRN("Detected a circular reference when decoding. Referenced object's fields " \
											"will be resolved in an undefined order (i.e. one of the objects will not " \
											"be fully deserialized when assigned to its field). Use RTTI_Flag_WeakRef to " \
											"get rid of this warning and tell the system which of the objects is allowed " \
											"to be deserialized after it is assigned to its field.");
									}
									else
									{
										objToDecode.decodeInProgress = true;
										decodeEntry(objToDecode.object, objToDecode.serializedObject);
										objToDecode.decodeInProgress = false;
										objToDecode.isDecoded = true;
									}
								}

								curField->setValue(rttiInstance, object.get(), objToDecode.object);
							}
							else
							{
								curField->setValue(rttiInstance, object.get(), nullptr);
							}
						}
						break;
						case SerializableFT_Reflectable:
						{
							RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

							SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
							RTTITypeBase* childRtti = nullptr;

							if (fieldObjectData != nullptr)
								childRtti = IReflectable::_getRTTIfromTypeId(fieldObjectData->getRootTypeId());

							if (childRtti != nullptr)
							{
								SPtr<IReflectable> newObject = childRtti->newRTTIObject();
								decodeEntry(newObject, fieldObjectData.get());
								curField->setValue(rttiInstance, object.get(), *newObject);
							}
							break;
						}
						case SerializableFT_Plain:
						{
							RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

							SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(entryData);
							if (fieldData != nullptr)
							{
								curField->fromBuffer(rttiInstance, object.get(), fieldData->value);
							}
						}
						break;
						case SerializableFT_DataBlock:
						{
							RTTIManagedDataBlockFieldBase* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

							SPtr<SerializedDataBlock> fieldData = std::static_pointer_cast<SerializedDataBlock>(entryData);
							if (fieldData != nullptr)
							{
								fieldData->stream->seek(fieldData->offset);
								curField->setValue(rttiInstance, object.get(), fieldData->stream, fieldData->size);
							}

							break;
						}
						}
					}
				}
			}

			while (!rttiInstances.empty())
			{
				RTTITypeBase* rttiInstance = rttiInstances.top();
				rttiInstance->onDeserializationEnded(object.get(), mParams);
				mAlloc->destruct(rttiInstance);

				rttiInstances.pop();
			}
		}

		SPtr<SerializedObject> IntermediateSerializer::encodeEntry(IReflectable* object, bool shallow)
		{
			FrameStack<RTTITypeBase*> rttiInstances;
			RTTITypeBase* rtti = object->getRTTI();

			const auto cleanup = [&]()
			{
				while (!rttiInstances.empty())
				{
					RTTITypeBase* rttiInstance = rttiInstances.top();
					rttiInstance->onSerializationEnded(object, mParams);
					mAlloc->destruct(rttiInstance);

					rttiInstances.pop();
				}
			};

			SPtr<SerializedObject> output = bs_shared_ptr_new<SerializedObject>();

			// If an object has base classes, we need to iterate through all of them
			do
			{
				RTTITypeBase* rttiInstance = rtti->_clone(*mAlloc);
				rttiInstances.push(rttiInstance);

				rttiInstance->onSerializationStarted(object, mParams);

				output->subObjects.push_back(SerializedSubObject());
				SerializedSubObject& subObject = output->subObjects.back();
				subObject.typeId = rtti->getRTTIId();

				const UINT32 numFields = rtti->getNumFields();
				for (UINT32 i = 0; i < numFields; i++)
				{
					SPtr<SerializedInstance> serializedEntry;

					RTTIField* curGenericField = rtti->getField(i);
					if (curGenericField->mIsVectorType)
					{
						const UINT32 arrayNumElems = curGenericField->getArraySize(rttiInstance, object);

						const SPtr<SerializedArray> serializedArray = bs_shared_ptr_new<SerializedArray>();
						serializedArray->numElements = arrayNumElems;

						serializedEntry = serializedArray;

						switch (curGenericField->mType)
						{
						case SerializableFT_ReflectablePtr:
						{
							auto curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

							for (UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							{
								SPtr<SerializedObject> serializedChildObj = nullptr;

								if (!shallow)
								{
									SPtr<IReflectable> childObject = curField->getArrayValue(rttiInstance, object, arrIdx);

									if (childObject)
										serializedChildObj = encodeEntry(childObject.get(), shallow);
								}

								SerializedArrayEntry arrayEntry;
								arrayEntry.serialized = serializedChildObj;
								arrayEntry.index = arrIdx;

								serializedArray->entries[arrIdx] = arrayEntry;
							}

							break;
						}
						case SerializableFT_Reflectable:
						{
							auto curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

							for (UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							{
								IReflectable& childObject = curField->getArrayValue(rttiInstance, object, arrIdx);

								const SPtr<SerializedObject> serializedChildObj = encodeEntry(&childObject, shallow);

								SerializedArrayEntry arrayEntry;
								arrayEntry.serialized = serializedChildObj;
								arrayEntry.index = arrIdx;

								serializedArray->entries[arrIdx] = arrayEntry;
							}

							break;
						}
						case SerializableFT_Plain:
						{
							auto curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

							for (UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							{
								UINT32 typeSize = 0;
								if (curField->hasDynamicSize())
									typeSize = curField->getArrayElemDynamicSize(rttiInstance, object, arrIdx);
								else
									typeSize = curField->getTypeSize();

								const auto serializedField = bs_shared_ptr_new<SerializedField>();
								serializedField->value = (UINT8*)bs_alloc(typeSize);
								serializedField->ownsMemory = true;
								serializedField->size = typeSize;

								curField->arrayElemToBuffer(rttiInstance, object, arrIdx, serializedField->value);

								SerializedArrayEntry arrayEntry;
								arrayEntry.serialized = serializedField;
								arrayEntry.index = arrIdx;

								serializedArray->entries[arrIdx] = arrayEntry;
							}

							break;
						}
						default:
							BS_EXCEPT(InternalErrorException,
								"Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(UINT32(curGenericField->mType)) +
								", Is array: " + toString(curGenericField->mIsVectorType));
						}
					}
					else
					{
						switch (curGenericField->mType)
						{
						case SerializableFT_ReflectablePtr:
						{
							auto curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

							if (!shallow)
							{
								SPtr<IReflectable> childObject = curField->getValue(rttiInstance, object);

								if (childObject)
									serializedEntry = encodeEntry(childObject.get(), shallow);
							}

							break;
						}
						case SerializableFT_Reflectable:
						{
							auto curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
							IReflectable& childObject = curField->getValue(rttiInstance, object);

							serializedEntry = encodeEntry(&childObject, shallow);

							break;
						}
						case SerializableFT_Plain:
						{
							auto curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

							UINT32 typeSize = 0;
							if (curField->hasDynamicSize())
								typeSize = curField->getDynamicSize(rttiInstance, object);
							else
								typeSize = curField->getTypeSize();

							const auto serializedField = bs_shared_ptr_new<SerializedField>();
							serializedField->value = (UINT8*)bs_alloc(typeSize);
							serializedField->ownsMemory = true;
							serializedField->size = typeSize;

							curField->toBuffer(rttiInstance, object, serializedField->value);

							serializedEntry = serializedField;

							break;
						}
						case SerializableFT_DataBlock:
						{
							auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

							UINT32 dataBlockSize = 0;
							SPtr<DataStream> blockStream = curField->getValue(rttiInstance, object, dataBlockSize);

							auto dataBlockBuffer = (UINT8*)bs_alloc(dataBlockSize);
							blockStream->read(dataBlockBuffer, dataBlockSize);

							SPtr<DataStream> stream = bs_shared_ptr_new<MemoryDataStream>(dataBlockBuffer, dataBlockSize);

							SPtr<SerializedDataBlock> serializedDataBlock = bs_shared_ptr_new<SerializedDataBlock>();
							serializedDataBlock->stream = stream;
							serializedDataBlock->offset = 0;

							serializedDataBlock->size = dataBlockSize;
							serializedEntry = serializedDataBlock;

							break;
						}
						default:
							BS_EXCEPT(InternalErrorException,
								"Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(UINT32(curGenericField->mType)) +
								", Is array: " + toString(curGenericField->mIsVectorType));
						}
					}

					SerializedEntry entry;
					entry.fieldId = curGenericField->mUniqueId;
					entry.serialized = serializedEntry;

					subObject.entries.insert(std::make_pair(curGenericField->mUniqueId, entry));
				}

				rtti = rtti->getBaseClass();

			} while (rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

			cleanup();

			return output;
		}
	}

	SPtr<SerializedObject> SerializedObject::create(IReflectable& obj, bool shallow)
	{
		detail::IntermediateSerializer is;
		return is.encode(&obj, shallow);
		
	}

	SPtr<IReflectable> SerializedObject::decode() const
	{
		detail::IntermediateSerializer is;
		return is.decode(this);
	}

	SPtr<SerializedInstance> SerializedObject::clone(bool cloneData)
	{
		SPtr<SerializedObject> copy = bs_shared_ptr_new<SerializedObject>();
		copy->subObjects = Vector<SerializedSubObject>(subObjects.size());

		UINT32 i = 0;
		for (auto& subObject : subObjects)
		{
			copy->subObjects[i].typeId = subObject.typeId;

			for (auto& entryPair : subObject.entries)
			{
				SerializedEntry entry = entryPair.second;

				if (entry.serialized != nullptr)
					entry.serialized = entry.serialized->clone(cloneData);

				copy->subObjects[i].entries[entryPair.first] = entry;
			}

			i++;
		}

		return copy;
	}

	SPtr<SerializedInstance> SerializedField::clone(bool cloneData)
	{
		SPtr<SerializedField> copy = bs_shared_ptr_new<SerializedField>();
		copy->size = size;

		if (cloneData)
		{
			copy->value = (UINT8*)bs_alloc(size);
			memcpy(copy->value, value, size);
			copy->ownsMemory = true;
		}
		else
		{
			copy->value = value;
			copy->ownsMemory = false;
		}

		return copy;
	}

	SPtr<SerializedInstance> SerializedDataBlock::clone(bool cloneData)
	{
		SPtr<SerializedDataBlock> copy = bs_shared_ptr_new<SerializedDataBlock>();
		copy->size = size;

		if (cloneData)
		{
			if(stream->isFile())
				LOGWRN("Cloning a file stream. Streaming is disabled and stream data will be loaded into memory.");

			UINT8* data = (UINT8*)bs_alloc(size);
			stream->read(data, size);

			copy->stream = bs_shared_ptr_new<MemoryDataStream>(data, size);
			copy->offset = 0;
		}
		else
		{
			copy->stream = stream;
			copy->offset = offset;
		}

		return copy;
	}

	SPtr<SerializedInstance> SerializedArray::clone(bool cloneData)
	{
		SPtr<SerializedArray> copy = bs_shared_ptr_new<SerializedArray>();
		copy->numElements = numElements;

		for (auto& entryPair : entries)
		{
			SerializedArrayEntry entry = entryPair.second;
			entry.serialized = entry.serialized->clone(cloneData);

			copy->entries[entryPair.first] = entry;
		}

		return copy;
	}

	RTTITypeBase* SerializedInstance::getRTTIStatic()
	{
		return SerializedInstanceRTTI::instance();
	}

	RTTITypeBase* SerializedInstance::getRTTI() const
	{
		return SerializedInstance::getRTTIStatic();
	}

	RTTITypeBase* SerializedDataBlock::getRTTIStatic()
	{
		return SerializedDataBlockRTTI::instance();
	}

	RTTITypeBase* SerializedDataBlock::getRTTI() const
	{
		return SerializedDataBlock::getRTTIStatic();
	}

	RTTITypeBase* SerializedField::getRTTIStatic()
	{
		return SerializedFieldRTTI::instance();
	}

	RTTITypeBase* SerializedField::getRTTI() const
	{
		return SerializedField::getRTTIStatic();
	}

	UINT32 SerializedObject::getRootTypeId() const
	{
		if(subObjects.size() > 0)
			return subObjects[0].typeId;

		return 0;
	}

	RTTITypeBase* SerializedObject::getRTTIStatic()
	{
		return SerializedObjectRTTI::instance();
	}

	RTTITypeBase* SerializedObject::getRTTI() const
	{
		return SerializedObject::getRTTIStatic();
	}

	RTTITypeBase* SerializedArray::getRTTIStatic()
	{
		return SerializedArrayRTTI::instance();
	}

	RTTITypeBase* SerializedArray::getRTTI() const
	{
		return SerializedArray::getRTTIStatic();
	}

	RTTITypeBase* SerializedSubObject::getRTTIStatic()
	{
		return SerializedSubObjectRTTI::instance();
	}

	RTTITypeBase* SerializedSubObject::getRTTI() const
	{
		return SerializedSubObject::getRTTIStatic();
	}

	RTTITypeBase* SerializedEntry::getRTTIStatic()
	{
		return SerializedEntryRTTI::instance();
	}

	RTTITypeBase* SerializedEntry::getRTTI() const
	{
		return SerializedEntry::getRTTIStatic();
	}

	RTTITypeBase* SerializedArrayEntry::getRTTIStatic()
	{
		return SerializedArrayEntryRTTI::instance();
	}

	RTTITypeBase* SerializedArrayEntry::getRTTI() const
	{
		return SerializedArrayEntry::getRTTIStatic();
	}
}