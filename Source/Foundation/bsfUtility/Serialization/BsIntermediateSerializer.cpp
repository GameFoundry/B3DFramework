//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsIntermediateSerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

IntermediateSerializer::IntermediateSerializer()
	: mAlloc(&gFrameAlloc())
{}

SPtr<IReflectable> IntermediateSerializer::Decode(const SerializedObject* serializedObject, SerializationContext* context)
{
	mContext = context;

	mAlloc->MarkFrame();
	mObjectMap.clear();

	SPtr<IReflectable> output;
	RTTITypeBase* type = IReflectable::GetRttifromTypeIdInternal(serializedObject->GetRootTypeId());
	if(type != nullptr)
	{
		output = type->NewRttiObject();
		auto iterNewObj = mObjectMap.insert(std::make_pair(serializedObject, ObjectToDecode(output, serializedObject)));

		iterNewObj.first->second.DecodeInProgress = true;
		DecodeEntry(output, serializedObject);
		iterNewObj.first->second.DecodeInProgress = false;
		iterNewObj.first->second.IsDecoded = true;
	}

	// Go through the remaining objects (should be only ones with weak refs)
	for(auto iter = mObjectMap.begin(); iter != mObjectMap.end(); ++iter)
	{
		ObjectToDecode& objToDecode = iter->second;

		if(objToDecode.IsDecoded)
			continue;

		objToDecode.DecodeInProgress = true;
		DecodeEntry(objToDecode.Object, objToDecode.SerializedObject);
		objToDecode.DecodeInProgress = false;
		objToDecode.IsDecoded = true;
	}

	mObjectMap.clear();
	mAlloc->Clear();

	return output;
}

SPtr<SerializedObject> IntermediateSerializer::Encode(IReflectable* object, SerializedObjectEncodeFlags flags, SerializationContext* context)
{
	mContext = context;

	return EncodeEntry(object, flags, context, mAlloc);
}

void IntermediateSerializer::DecodeEntry(const SPtr<IReflectable>& object, const SerializedObject* serializableObject)
{
	u32 numSubObjects = (u32)serializableObject->SubObjects.size();
	if(numSubObjects == 0)
		return;

	FrameStack<RTTITypeBase*> rttiInstances;
	for(i32 subObjectIdx = numSubObjects - 1; subObjectIdx >= 0; subObjectIdx--)
	{
		const SerializedSubObject& subObject = serializableObject->SubObjects[subObjectIdx];

		RTTITypeBase* rtti = IReflectable::GetRttifromTypeIdInternal(subObject.TypeId);
		if(rtti == nullptr)
			continue;

		RTTITypeBase* rttiInstance = rtti->CloneInternal(*mAlloc);
		rttiInstance->OnDeserializationStarted(object.get(), mContext);
		rttiInstances.push(rttiInstance);

		u32 numFields = rtti->GetNumFields();
		for(u32 fieldIdx = 0; fieldIdx < numFields; fieldIdx++)
		{
			RTTIField* curGenericField = rtti->GetField(fieldIdx);

			auto iterFindFieldData = subObject.Entries.find(curGenericField->Schema.Id);
			if(iterFindFieldData == subObject.Entries.end())
				continue;

			SPtr<SerializedInstance> entryData = iterFindFieldData->second.Serialized;
			if(curGenericField->Schema.IsArray)
			{
				SPtr<SerializedArray> arrayData = std::static_pointer_cast<SerializedArray>(entryData);

				u32 arrayNumElems = (u32)arrayData->NumElements;
				curGenericField->SetArraySize(rttiInstance, object.get(), arrayNumElems);

				switch(curGenericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						for(auto& arrayElem : arrayData->Entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.Serialized);
							RTTITypeBase* childRtti = nullptr;

							if(arrayElemData != nullptr)
								childRtti = IReflectable::GetRttifromTypeIdInternal(arrayElemData->GetRootTypeId());

							if(childRtti != nullptr)
							{
								auto findObj = mObjectMap.find(arrayElemData.get());
								if(findObj == mObjectMap.end())
								{
									SPtr<IReflectable> newObject = childRtti->NewRttiObject();
									findObj = mObjectMap.insert(std::make_pair(arrayElemData.get(), ObjectToDecode(newObject, arrayElemData.get()))).first;
								}

								ObjectToDecode& objToDecode = findObj->second;

								bool needsDecoding = !curField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef) && !objToDecode.IsDecoded;
								if(needsDecoding)
								{
									if(objToDecode.DecodeInProgress)
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
										objToDecode.DecodeInProgress = true;
										DecodeEntry(objToDecode.Object, objToDecode.SerializedObject);
										objToDecode.DecodeInProgress = false;
										objToDecode.IsDecoded = true;
									}
								}

								curField->SetArrayValue(rttiInstance, object.get(), arrayElem.first, objToDecode.Object);
							}
							else
							{
								curField->SetArrayValue(rttiInstance, object.get(), arrayElem.first, nullptr);
							}
						}
					}
					break;
				case SerializableFT_Reflectable:
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

						for(auto& arrayElem : arrayData->Entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.Serialized);
							RTTITypeBase* childRtti = nullptr;

							if(arrayElemData != nullptr)
								childRtti = IReflectable::GetRttifromTypeIdInternal(arrayElemData->GetRootTypeId());

							if(childRtti != nullptr)
							{
								SPtr<IReflectable> newObject = childRtti->NewRttiObject();
								DecodeEntry(newObject, arrayElemData.get());
								curField->SetArrayValue(rttiInstance, object.get(), arrayElem.first, *newObject);
							}
						}
						break;
					}
				case SerializableFT_Plain:
					{
						auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

						for(auto& arrayElem : arrayData->Entries)
						{
							SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(arrayElem.second.Serialized);
							if(fieldData != nullptr)
							{
								Bitstream tempStream(fieldData->Value, fieldData->Size);
								curField->ArrayElemFromBuffer(rttiInstance, object.get(), arrayElem.first, tempStream);
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
				switch(curGenericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
						RTTITypeBase* childRtti = nullptr;

						if(fieldObjectData != nullptr)
							childRtti = IReflectable::GetRttifromTypeIdInternal(fieldObjectData->GetRootTypeId());

						if(childRtti != nullptr)
						{
							auto findObj = mObjectMap.find(fieldObjectData.get());
							if(findObj == mObjectMap.end())
							{
								SPtr<IReflectable> newObject = childRtti->NewRttiObject();
								findObj = mObjectMap.insert(std::make_pair(fieldObjectData.get(), ObjectToDecode(newObject, fieldObjectData.get()))).first;
							}

							ObjectToDecode& objToDecode = findObj->second;

							bool needsDecoding = !curField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef) && !objToDecode.IsDecoded;
							if(needsDecoding)
							{
								if(objToDecode.DecodeInProgress)
								{
									BS_LOG(Warning, Generic, "Detected a circular reference when decoding. Referenced "
															 "object's fields will be resolved in an undefined order (i.e. one of the "
															 "objects will not be fully deserialized when assigned to its field). "
															 "Use RTTI_Flag_WeakRef to get rid of this warning and tell the system which of"
															 "the objects is allowed to be deserialized after it is assigned to its field.");
								}
								else
								{
									objToDecode.DecodeInProgress = true;
									DecodeEntry(objToDecode.Object, objToDecode.SerializedObject);
									objToDecode.DecodeInProgress = false;
									objToDecode.IsDecoded = true;
								}
							}

							curField->SetValue(rttiInstance, object.get(), objToDecode.Object);
						}
						else
						{
							curField->SetValue(rttiInstance, object.get(), nullptr);
						}
					}
					break;
				case SerializableFT_Reflectable:
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(entryData);
						RTTITypeBase* childRtti = nullptr;

						if(fieldObjectData != nullptr)
							childRtti = IReflectable::GetRttifromTypeIdInternal(fieldObjectData->GetRootTypeId());

						if(childRtti != nullptr)
						{
							SPtr<IReflectable> newObject = childRtti->NewRttiObject();
							DecodeEntry(newObject, fieldObjectData.get());
							curField->SetValue(rttiInstance, object.get(), *newObject);
						}
						break;
					}
				case SerializableFT_Plain:
					{
						auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

						SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(entryData);
						if(fieldData != nullptr)
						{
							Bitstream tempStream(fieldData->Value, fieldData->Size);
							curField->FromBuffer(rttiInstance, object.get(), tempStream);
						}
					}
					break;
				case SerializableFT_DataBlock:
					{
						auto* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

						SPtr<SerializedDataBlock> fieldData = std::static_pointer_cast<SerializedDataBlock>(entryData);
						if(fieldData != nullptr)
						{
							fieldData->Stream->Seek(fieldData->Offset);
							curField->SetValue(rttiInstance, object.get(), fieldData->Stream, fieldData->Size);
						}

						break;
					}
				}
			}
		}
	}

	while(!rttiInstances.empty())
	{
		RTTITypeBase* rttiInstance = rttiInstances.top();
		rttiInstance->OnDeserializationEnded(object.get(), mContext);
		mAlloc->Destruct(rttiInstance);

		rttiInstances.pop();
	}
}

SPtr<SerializedObject> IntermediateSerializer::EncodeEntry(IReflectable* object, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc)
{
	FrameStack<RTTITypeBase*> rttiInstances;
	RTTITypeBase* rtti = object->GetRtti();

	const auto cleanup = [&]()
	{
		while(!rttiInstances.empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.top();
			rttiInstance->OnSerializationEnded(object, context);
			alloc->Destruct(rttiInstance);

			rttiInstances.pop();
		}
	};

	bool replicableOnly = flags.IsSet(SerializedObjectEncodeFlag::ReplicableOnly);
	SPtr<SerializedObject> output = bs_shared_ptr_new<SerializedObject>();

	// If an object has base classes, we need to iterate through all of them
	do
	{
		RTTITypeBase* rttiInstance = rtti->CloneInternal(*alloc);
		rttiInstances.push(rttiInstance);

		rttiInstance->OnSerializationStarted(object, context);

		output->SubObjects.push_back(SerializedSubObject());
		SerializedSubObject& subObject = output->SubObjects.back();
		subObject.TypeId = rtti->GetRttiId();

		const u32 numFields = rtti->GetNumFields();
		for(u32 i = 0; i < numFields; i++)
		{
			RTTIField* curGenericField = rtti->GetField(i);

			if(replicableOnly)
			{
				if(!curGenericField->Schema.Info.Flags.IsSet(RTTIFieldFlag::Replicate))
					continue;
			}

			SPtr<SerializedInstance> serializedEntry = EncodeFieldInternal(object, rttiInstance, curGenericField, (u32)-1, flags, context, alloc);

			SerializedEntry entry;
			entry.FieldId = curGenericField->Schema.Id;
			entry.Serialized = serializedEntry;

			subObject.Entries.insert(std::make_pair(curGenericField->Schema.Id, entry));
		}

		rtti = rtti->GetBaseClass();
	}
	while(rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

	cleanup();

	return output;
}

SPtr<SerializedInstance> IntermediateSerializer::EncodeFieldInternal(IReflectable* object, RTTITypeBase* rtti, RTTIField* field, u32 arrayIdx, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc)
{
	bool shallow = flags.IsSet(SerializedObjectEncodeFlag::Shallow);

	SPtr<SerializedInstance> output;
	if(field->Schema.IsArray)
	{
		const u32 arrayNumElems = field->GetArraySize(rtti, object);
		bool wholeArray = arrayIdx == (u32)-1;

		u32 arrayEndIdx;
		if(wholeArray)
		{
			arrayIdx = 0;
			arrayEndIdx = arrayNumElems;
		}
		else
			arrayEndIdx = arrayIdx + 1;

		SPtr<SerializedArray> serializedArray;
		if(wholeArray)
		{
			serializedArray = bs_shared_ptr_new<SerializedArray>();
			serializedArray->NumElements = arrayNumElems;

			output = serializedArray;
		}

		switch(field->Schema.Type)
		{
		case SerializableFT_ReflectablePtr:
			{
				auto curField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				for(u32 arrIdx = arrayIdx; arrIdx < arrayEndIdx; arrIdx++)
				{
					SPtr<SerializedObject> serializedChildObj = nullptr;

					if(!shallow)
					{
						SPtr<IReflectable> childObject = curField->GetArrayValue(rtti, object, arrIdx);

						if(childObject)
							serializedChildObj = EncodeEntry(childObject.get(), flags, context, alloc);
					}

					if(wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.Serialized = serializedChildObj;
						arrayEntry.Index = arrIdx;

						serializedArray->Entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedChildObj;
				}

				break;
			}
		case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);

				for(u32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
				{
					IReflectable& childObject = curField->GetArrayValue(rtti, object, arrIdx);

					const SPtr<SerializedObject> serializedChildObj = EncodeEntry(&childObject, flags, context, alloc);

					if(wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.Serialized = serializedChildObj;
						arrayEntry.Index = arrIdx;

						serializedArray->Entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedChildObj;
				}

				break;
			}
		case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				for(u32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
				{
					u32 typeSize = 0;
					if(curField->Schema.HasDynamicSize)
						typeSize = curField->GetArrayElemDynamicSize(rtti, object, arrIdx, false).Bytes;
					else
						typeSize = curField->Schema.Size.Bytes;

					const auto serializedField = bs_shared_ptr_new<SerializedField>();
					serializedField->Value = (u8*)bs_alloc(typeSize);
					serializedField->OwnsMemory = true;
					serializedField->Size = typeSize;

					Bitstream tempStream(serializedField->Value, typeSize);
					curField->ArrayElemToStream(rtti, object, arrIdx, tempStream);

					if(wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.Serialized = serializedField;
						arrayEntry.Index = arrIdx;

						serializedArray->Entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedField;
				}

				break;
			}
		default:
			BS_EXCEPT(InternalErrorException, "Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(u32(field->Schema.Type)) + ", Is array: " + toString(field->Schema.IsArray));
		}
	}
	else
	{
		switch(field->Schema.Type)
		{
		case SerializableFT_ReflectablePtr:
			{
				auto curField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				if(!shallow)
				{
					SPtr<IReflectable> childObject = curField->GetValue(rtti, object);

					if(childObject)
						output = EncodeEntry(childObject.get(), flags, context, alloc);
				}

				break;
			}
		case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);
				IReflectable& childObject = curField->GetValue(rtti, object);

				output = EncodeEntry(&childObject, flags, context, alloc);

				break;
			}
		case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				u32 typeSize = 0;
				if(curField->Schema.HasDynamicSize)
					typeSize = curField->GetDynamicSize(rtti, object, false).Bytes;
				else
					typeSize = curField->Schema.Size.Bytes;

				const auto serializedField = bs_shared_ptr_new<SerializedField>();
				serializedField->Value = (u8*)bs_alloc(typeSize);
				serializedField->OwnsMemory = true;
				serializedField->Size = typeSize;

				Bitstream tempStream(serializedField->Value, typeSize);
				curField->ToStream(rtti, object, tempStream);

				output = serializedField;

				break;
			}
		case SerializableFT_DataBlock:
			{
				auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(field);

				u32 dataBlockSize = 0;
				SPtr<DataStream> blockStream = curField->GetValue(rtti, object, dataBlockSize);

				SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>(dataBlockSize);
				blockStream->Read(stream->Data(), dataBlockSize);

				SPtr<SerializedDataBlock> serializedDataBlock = bs_shared_ptr_new<SerializedDataBlock>();
				serializedDataBlock->Stream = stream;
				serializedDataBlock->Offset = 0;

				serializedDataBlock->Size = dataBlockSize;
				output = serializedDataBlock;

				break;
			}
		default:
			BS_EXCEPT(InternalErrorException, "Error encoding data. Encountered a type I don't know how to encode. Type: " + toString(u32(field->Schema.Type)) + ", Is array: " + toString(field->Schema.IsArray));
		}
	}

	return output;
}
