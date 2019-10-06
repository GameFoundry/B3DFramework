//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsIntermediateSerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	IntermediateSerializer::IntermediateSerializer()
		:mAlloc(&gFrameAlloc())
	{ }

	SPtr<IReflectable> IntermediateSerializer::decode(const SerializedObject* serializedObject,
		SerializationContext* context)
	{
		mContext = context;

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

	SPtr<SerializedObject> IntermediateSerializer::encode(IReflectable* object, SerializedObjectEncodeFlags flags,
		SerializationContext* context)
	{
		mContext = context;

		return encodeEntry(object, flags, context, mAlloc);
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
			rttiInstance->onDeserializationStarted(object.get(), mContext);
			rttiInstances.push(rttiInstance);

			UINT32 numFields = rtti->getNumFields();
			for (UINT32 fieldIdx = 0; fieldIdx < numFields; fieldIdx++)
			{
				RTTIField* curGenericField = rtti->getField(fieldIdx);

				auto iterFindFieldData = subObject.entries.find(curGenericField->schema.id);
				if (iterFindFieldData == subObject.entries.end())
					continue;

				SPtr<SerializedInstance> entryData = iterFindFieldData->second.serialized;
				if (curGenericField->schema.isArray)
				{
					SPtr<SerializedArray> arrayData = std::static_pointer_cast<SerializedArray>(entryData);

					UINT32 arrayNumElems = (UINT32)arrayData->numElements;
					curGenericField->setArraySize(rttiInstance, object.get(), arrayNumElems);

					switch (curGenericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

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

								bool needsDecoding = !curField->schema.info.flags.isSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
								if (needsDecoding)
								{
									if (objToDecode.decodeInProgress)
									{
										BS_LOG(Warning, Generic, "Detected a circular reference when decoding. "
											"Referenced object's fields will be resolved in an undefined order "
											"(i.e. one of the objects will not be fully deserialized when assigned "
											"to its field). Use RTTI_Flag_WeakRef to get rid of this warning and tell "
											"the system which of the objects is allowed to be deserialized after it "
											"is assigned to its field.");
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
						auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

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
						auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

						for (auto& arrayElem : arrayData->entries)
						{
							SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(arrayElem.second.serialized);
							if (fieldData != nullptr)
							{
								Bitstream tempStream(fieldData->value, fieldData->size);
								curField->arrayElemFromBuffer(rttiInstance, object.get(), arrayElem.first, tempStream);
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
					switch (curGenericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

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

							bool needsDecoding = !curField->schema.info.flags.isSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
							if (needsDecoding)
							{
								if (objToDecode.decodeInProgress)
								{
									BS_LOG(Warning, Generic, "Detected a circular reference when decoding. Referenced "
										"object's fields will be resolved in an undefined order (i.e. one of the "
										"objects will not be fully deserialized when assigned to its field). "
										"Use RTTI_Flag_WeakRef to get rid of this warning and tell the system which of"
										"the objects is allowed to be deserialized after it is assigned to its field.");
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
						auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

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
						auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

						SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(entryData);
						if (fieldData != nullptr)
						{
							Bitstream tempStream(fieldData->value, fieldData->size);
							curField->fromBuffer(rttiInstance, object.get(), tempStream);
						}
					}
					break;
					case SerializableFT_DataBlock:
					{
						auto* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

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
			rttiInstance->onDeserializationEnded(object.get(), mContext);
			mAlloc->destruct(rttiInstance);

			rttiInstances.pop();
		}
	}

	SPtr<SerializedObject> IntermediateSerializer::encodeEntry(IReflectable* object, SerializedObjectEncodeFlags flags,
		SerializationContext* context, FrameAlloc* alloc)
	{
		FrameStack<RTTITypeBase*> rttiInstances;
		RTTITypeBase* rtti = object->getRTTI();

		const auto cleanup = [&]()
		{
			while (!rttiInstances.empty())
			{
				RTTITypeBase* rttiInstance = rttiInstances.top();
				rttiInstance->onSerializationEnded(object, context);
				alloc->destruct(rttiInstance);

				rttiInstances.pop();
			}
		};

		bool replicableOnly = flags.isSet(SerializedObjectEncodeFlag::ReplicableOnly);
		SPtr<SerializedObject> output = bs_shared_ptr_new<SerializedObject>();

		// If an object has base classes, we need to iterate through all of them
		do
		{
			RTTITypeBase* rttiInstance = rtti->_clone(*alloc);
			rttiInstances.push(rttiInstance);

			rttiInstance->onSerializationStarted(object, context);

			output->subObjects.push_back(SerializedSubObject());
			SerializedSubObject& subObject = output->subObjects.back();
			subObject.typeId = rtti->getRTTIId();

			const UINT32 numFields = rtti->getNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* curGenericField = rtti->getField(i);

				if(replicableOnly)
				{
					if(!curGenericField->schema.info.flags.isSet(RTTIFieldFlag::Replicate))
						continue;
				}

				SPtr<SerializedInstance> serializedEntry = _encodeField(object, rttiInstance, curGenericField, (UINT32)-1,
					flags, context, alloc);

				SerializedEntry entry;
				entry.fieldId = curGenericField->schema.id;
				entry.serialized = serializedEntry;

				subObject.entries.insert(std::make_pair(curGenericField->schema.id, entry));
			}

			rtti = rtti->getBaseClass();

		} while (rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

		cleanup();

		return output;
	}

	SPtr<SerializedInstance> IntermediateSerializer::_encodeField(IReflectable* object, RTTITypeBase* rtti, RTTIField* field, UINT32 arrayIdx,
		SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc)
	{
		bool shallow = flags.isSet(SerializedObjectEncodeFlag::Shallow);

		SPtr<SerializedInstance> output;
		if (field->schema.isArray)
		{
			const UINT32 arrayNumElems = field->getArraySize(rtti, object);
			bool wholeArray = arrayIdx == (UINT32)-1;

			UINT32 arrayEndIdx;
			if (wholeArray)
			{
				arrayIdx = 0;
				arrayEndIdx = arrayNumElems;
			}
			else
				arrayEndIdx = arrayIdx + 1;

			SPtr<SerializedArray> serializedArray;
			if (wholeArray)
			{
				serializedArray = bs_shared_ptr_new<SerializedArray>();
				serializedArray->numElements = arrayNumElems;

				output = serializedArray;
			}

			switch (field->schema.type)
			{
			case SerializableFT_ReflectablePtr:
			{
				auto curField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				for (UINT32 arrIdx = arrayIdx; arrIdx < arrayEndIdx; arrIdx++)
				{
					SPtr<SerializedObject> serializedChildObj = nullptr;

					if (!shallow)
					{
						SPtr<IReflectable> childObject = curField->getArrayValue(rtti, object, arrIdx);

						if (childObject)
							serializedChildObj = encodeEntry(childObject.get(), flags, context, alloc);
					}

					if (wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.serialized = serializedChildObj;
						arrayEntry.index = arrIdx;

						serializedArray->entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedChildObj;
				}

				break;
			}
			case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);

				for (UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
				{
					IReflectable& childObject = curField->getArrayValue(rtti, object, arrIdx);

					const SPtr<SerializedObject> serializedChildObj = encodeEntry(&childObject, flags, context, alloc);

					if (wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.serialized = serializedChildObj;
						arrayEntry.index = arrIdx;

						serializedArray->entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedChildObj;
				}

				break;
			}
			case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				for (UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
				{
					UINT32 typeSize = 0;
					if (curField->schema.hasDynamicSize)
						typeSize = curField->getArrayElemDynamicSize(rtti, object, arrIdx, false).bytes;
					else
						typeSize = curField->schema.size.bytes;

					const auto serializedField = bs_shared_ptr_new<SerializedField>();
					serializedField->value = (UINT8*)bs_alloc(typeSize);
					serializedField->ownsMemory = true;
					serializedField->size = typeSize;

					Bitstream tempStream(serializedField->value, typeSize);
					curField->arrayElemToStream(rtti, object, arrIdx, tempStream);

					if (wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.serialized = serializedField;
						arrayEntry.index = arrIdx;

						serializedArray->entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedField;
				}

				break;
			}
			default:
				BS_EXCEPT(InternalErrorException,
					"Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(UINT32(field->schema.type)) +
					", Is array: " + toString(field->schema.isArray));
			}
		}
		else
		{
			switch (field->schema.type)
			{
			case SerializableFT_ReflectablePtr:
			{
				auto curField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				if (!shallow)
				{
					SPtr<IReflectable> childObject = curField->getValue(rtti, object);

					if (childObject)
						output = encodeEntry(childObject.get(), flags, context, alloc);
				}

				break;
			}
			case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);
				IReflectable& childObject = curField->getValue(rtti, object);

				output = encodeEntry(&childObject, flags, context, alloc);

				break;
			}
			case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				UINT32 typeSize = 0;
				if (curField->schema.hasDynamicSize)
					typeSize = curField->getDynamicSize(rtti, object, false).bytes;
				else
					typeSize = curField->schema.size.bytes;

				const auto serializedField = bs_shared_ptr_new<SerializedField>();
				serializedField->value = (UINT8*)bs_alloc(typeSize);
				serializedField->ownsMemory = true;
				serializedField->size = typeSize;

				Bitstream tempStream(serializedField->value, typeSize);
				curField->toStream(rtti, object, tempStream);

				output = serializedField;

				break;
			}
			case SerializableFT_DataBlock:
			{
				auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(field);

				UINT32 dataBlockSize = 0;
				SPtr<DataStream> blockStream = curField->getValue(rtti, object, dataBlockSize);

				SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>(dataBlockSize);
				blockStream->read(stream->data(), dataBlockSize);

				SPtr<SerializedDataBlock> serializedDataBlock = bs_shared_ptr_new<SerializedDataBlock>();
				serializedDataBlock->stream = stream;
				serializedDataBlock->offset = 0;

				serializedDataBlock->size = dataBlockSize;
				output = serializedDataBlock;

				break;
			}
			default:
				BS_EXCEPT(InternalErrorException,
					"Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(UINT32(field->schema.type)) +
					", Is array: " + toString(field->schema.isArray));
			}
		}

		return output;
	}
}
