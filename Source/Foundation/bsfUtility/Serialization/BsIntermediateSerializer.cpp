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

		mAlloc->MarkFrame();
		mObjectMap.Clear();

		SPtr<IReflectable> output;
		RTTITypeBase* type = IReflectable::_getRTTIfromTypeId(serializedObject->GetRootTypeId());
		if (type != nullptr)
		{
			output = type->NewRTTIObject();
			auto iterNewObj = mObjectMap.Insert(std::make_pair(serializedObject, ObjectToDecode(output, serializedObject)));

			iterNewObj.first->second.decodeInProgress = true;
			decodeEntry(output, serializedObject);
			iterNewObj.first->second.decodeInProgress = false;
			iterNewObj.first->second.isDecoded = true;
		}

		// Go through the remaining objects (should be only ones with weak refs)
		for (auto iter = mObjectMap.Begin(); iter != mObjectMap.end(); ++iter)
		{
			ObjectToDecode& objToDecode = iter->second;

			if (objToDecode.isDecoded)
				continue;

			objToDecode.decodeInProgress = true;
			decodeEntry(objToDecode.object, objToDecode.serializedObject);
			objToDecode.decodeInProgress = false;
			objToDecode.isDecoded = true;
		}

		mObjectMap.Clear();
		mAlloc->Clear();

		return output;
	}

	SPtr<SerializedObject> IntermediateSerializer::encode(IReflectable* object, SerializedObjectEncodeFlags flags,
		SerializationContext* context)
	{
		mContext = context;

		return EncodeEntry(object, flags, context, mAlloc);
	}

	void IntermediateSerializer::DecodeEntry(const SPtr<IReflectable>& object, const SerializedObject* serializableObject)
	{
		UINT32 numSubObjects = (UINT32)serializableObject->subObjects.Size();
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
			rttiInstance->OnDeserializationStarted(object.Get(), mContext);
			rttiInstances.Push(rttiInstance);

			UINT32 numFields = rtti->GetNumFields();
			for (UINT32 fieldIdx = 0; fieldIdx < numFields; fieldIdx++)
			{
				RTTIField* curGenericField = rtti->GetField(fieldIdx);

				auto iterFindFieldData = subObject.entries.Find(curGenericField->schema.id);
				if (iterFindFieldData == subObject.entries.End())
					continue;

				SPtr<SerializedInstance> entryData = iterFindFieldData->second.serialized;
				if (curGenericField->schema.isArray)
				{
					SPtr<SerializedArray> arrayData = std::static_pointer_cast<SerializedArray>(entryData);

					UINT32 arrayNumElems = (UINT32)arrayData->numElements;
					curGenericField->SetArraySize(rttiInstance, object.Get(), arrayNumElems);

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
								childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->GetRootTypeId());

							if (childRtti != nullptr)
							{
								auto findObj = mObjectMap.Find(arrayElemData.get());
								if (findObj == mObjectMap.End())
								{
									SPtr<IReflectable> newObject = childRtti->NewRTTIObject();
									findObj = mObjectMap.Insert(std::make_pair(arrayElemData.get(),
										ObjectToDecode(newObject, arrayElemData.Get()))).first;
								}

								ObjectToDecode& objToDecode = findObj->second;

								bool needsDecoding = !curField->schema.info.flags.IsSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
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

								curField->SetArrayValue(rttiInstance, object.Get(), arrayElem.first, objToDecode.object);
							}
							else
							{
								curField->SetArrayValue(rttiInstance, object.Get(), arrayElem.first, nullptr);
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
								childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->GetRootTypeId());

							if (childRtti != nullptr)
							{
								SPtr<IReflectable> newObject = childRtti->NewRTTIObject();
								decodeEntry(newObject, arrayElemData.Get());
								curField->SetArrayValue(rttiInstance, object.Get(), arrayElem.first, *newObject);
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
								Bitstream TempStream(fieldData->value, fieldData->size);
								curField->ArrayElemFromBuffer(rttiInstance, object.Get(), arrayElem.first, tempStream);
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
							childRtti = IReflectable::_getRTTIfromTypeId(fieldObjectData->GetRootTypeId());

						if (childRtti != nullptr)
						{
							auto findObj = mObjectMap.Find(fieldObjectData.get());
							if (findObj == mObjectMap.End())
							{
								SPtr<IReflectable> newObject = childRtti->NewRTTIObject();
								findObj = mObjectMap.Insert(std::make_pair(fieldObjectData.get(),
									ObjectToDecode(newObject, fieldObjectData.Get()))).first;
							}

							ObjectToDecode& objToDecode = findObj->second;

							bool needsDecoding = !curField->schema.info.flags.IsSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
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

							curField->SetValue(rttiInstance, object.Get(), objToDecode.object);
						}
						else
						{
							curField->SetValue(rttiInstance, object.Get(), nullptr);
						}
					}
					break;
					case SerializableFT_Reflectable:
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
						RTTITypeBase* childRtti = nullptr;

						if (fieldObjectData != nullptr)
							childRtti = IReflectable::_getRTTIfromTypeId(fieldObjectData->GetRootTypeId());

						if (childRtti != nullptr)
						{
							SPtr<IReflectable> newObject = childRtti->NewRTTIObject();
							decodeEntry(newObject, fieldObjectData.Get());
							curField->SetValue(rttiInstance, object.Get(), *newObject);
						}
						break;
					}
					case SerializableFT_Plain:
					{
						auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

						SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(entryData);
						if (fieldData != nullptr)
						{
							Bitstream TempStream(fieldData->value, fieldData->size);
							curField->FromBuffer(rttiInstance, object.Get(), tempStream);
						}
					}
					break;
					case SerializableFT_DataBlock:
					{
						auto* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

						SPtr<SerializedDataBlock> fieldData = std::static_pointer_cast<SerializedDataBlock>(entryData);
						if (fieldData != nullptr)
						{
							fieldData->stream->Seek(fieldData->offset);
							curField->SetValue(rttiInstance, object.Get(), fieldData->stream, fieldData->size);
						}

						break;
					}
					}
				}
			}
		}

		while (!rttiInstances.Empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.Top();
			rttiInstance->OnDeserializationEnded(object.Get(), mContext);
			mAlloc->Destruct(rttiInstance);

			rttiInstances.Pop();
		}
	}

	SPtr<SerializedObject> IntermediateSerializer::encodeEntry(IReflectable* object, SerializedObjectEncodeFlags flags,
		SerializationContext* context, FrameAlloc* alloc)
	{
		FrameStack<RTTITypeBase*> rttiInstances;
		RTTITypeBase* rtti = object->GetRTTI();

		const auto cleanup = [&]()
		{
			while (!rttiInstances.Empty())
			{
				RTTITypeBase* rttiInstance = rttiInstances.Top();
				rttiInstance->OnSerializationEnded(object, context);
				alloc->Destruct(rttiInstance);

				rttiInstances.Pop();
			}
		};

		bool replicableOnly = flags.IsSet(SerializedObjectEncodeFlag::ReplicableOnly);
		SPtr<SerializedObject> output = bs_shared_ptr_new<SerializedObject>();

		// If an object has base classes, we need to iterate through all of them
		do
		{
			RTTITypeBase* rttiInstance = rtti->_clone(*alloc);
			rttiInstances.Push(rttiInstance);

			rttiInstance->OnSerializationStarted(object, context);

			output->subObjects.push_back(SerializedSubObject());
			SerializedSubObject& subObject = output->subObjects.Back();
			subObject.typeId = rtti->GetRTTIId();

			const UINT32 numFields = rtti->GetNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* curGenericField = rtti->GetField(i);

				if(replicableOnly)
				{
					if(!curGenericField->schema.info.flags.IsSet(RTTIFieldFlag::Replicate))
						continue;
				}

				SPtr<SerializedInstance> serializedEntry = _encodeField(object, rttiInstance, curGenericField, (UINT32)-1,
					flags, context, alloc);

				SerializedEntry entry;
				entry.fieldId = curGenericField->schema.id;
				entry.serialized = serializedEntry;

				subObject.entries.Insert(std::make_pair(curGenericField->schema.id, entry));
			}

			rtti = rtti->GetBaseClass();

		} while (rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

		cleanup();

		return output;
	}

	SPtr<SerializedInstance> IntermediateSerializer::_encodeField(IReflectable* object, RTTITypeBase* rtti, RTTIField* field, UINT32 arrayIdx,
		SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc)
	{
		bool shallow = flags.IsSet(SerializedObjectEncodeFlag::Shallow);

		SPtr<SerializedInstance> output;
		if (field->schema.isArray)
		{
			const UINT32 arrayNumElems = field->GetArraySize(rtti, object);
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
						SPtr<IReflectable> childObject = curField->GetArrayValue(rtti, object, arrIdx);

						if (childObject)
							serializedChildObj = encodeEntry(childObject.Get(), flags, context, alloc);
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
					IReflectable& childObject = curField->GetArrayValue(rtti, object, arrIdx);

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
						typeSize = curField->GetArrayElemDynamicSize(rtti, object, arrIdx, false).bytes;
					else
						typeSize = curField->schema.size.bytes;

					const auto serializedField = bs_shared_ptr_new<SerializedField>();
					serializedField->value = (UINT8*)bs_alloc(typeSize);
					serializedField->ownsMemory = true;
					serializedField->size = typeSize;

					Bitstream TempStream(serializedField->value, typeSize);
					curField->ArrayElemToStream(rtti, object, arrIdx, tempStream);

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
					SPtr<IReflectable> childObject = curField->GetValue(rtti, object);

					if (childObject)
						output = encodeEntry(childObject.Get(), flags, context, alloc);
				}

				break;
			}
			case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);
				IReflectable& childObject = curField->GetValue(rtti, object);

				output = encodeEntry(&childObject, flags, context, alloc);

				break;
			}
			case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				UINT32 typeSize = 0;
				if (curField->schema.hasDynamicSize)
					typeSize = curField->GetDynamicSize(rtti, object, false).bytes;
				else
					typeSize = curField->schema.size.bytes;

				const auto serializedField = bs_shared_ptr_new<SerializedField>();
				serializedField->value = (UINT8*)bs_alloc(typeSize);
				serializedField->ownsMemory = true;
				serializedField->size = typeSize;

				Bitstream TempStream(serializedField->value, typeSize);
				curField->ToStream(rtti, object, tempStream);

				output = serializedField;

				break;
			}
			case SerializableFT_DataBlock:
			{
				auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(field);

				UINT32 dataBlockSize = 0;
				SPtr<DataStream> blockStream = curField->GetValue(rtti, object, dataBlockSize);

				SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>(dataBlockSize);
				blockStream->Read(stream->data(), dataBlockSize);

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
