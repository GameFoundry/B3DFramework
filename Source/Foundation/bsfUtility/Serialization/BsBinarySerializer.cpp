//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsBinarySerializer.h"

#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "Reflection/BsIReflectable.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIField.h"
#include "Reflection/BsRTTIPlainField.h"
#include "Reflection/BsRTTIReflectableField.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIManagedDataBlockField.h"
#include "FileSystem/BsDataStream.h"
#include "Utility/BsBufferedBitstream.h"

namespace bs
{
	constexpr UINT32 BinarySerializer::REPORT_AFTER_BYTES;
	constexpr UINT32 BinarySerializer::WRITE_BUFFER_SIZE;
	constexpr UINT32 BinarySerializer::FLUSH_AFTER_BYTES;
	constexpr UINT32 BinarySerializer::PRELOAD_CHUNK_BYTES;

	BinarySerializer::BinarySerializer()
		:mAlloc(&gFrameAlloc())
	{ }

	void BinarySerializer::Encode(IReflectable* object, const SPtr<DataStream>& stream, BinarySerializerFlags flags, SerializationContext* context)
	{
		mObjectsToEncode.Clear();
		mObjectAddrToId.Clear();
		mLastUsedObjectId = 1;
		mContext = context;
		mBuffer.Seek(0);

		mAlloc->MarkFrame();

		BufferedBitstreamWriter BufferedStream(&mBuffer, stream, WRITE_BUFFER_SIZE, FLUSH_AFTER_BYTES);

		Vector<SPtr<IReflectable>> encodedObjects;
		UINT32 objectId = findOrCreatePersistentId(object);
		
		// Encode primary object and its value types
		if (!encodeEntry(object, objectId, bufferedStream, flags))
		{
			BS_LOG(Error, Serialization, "Destination buffer is null or not large enough.");
			return;
		}

		// Encode pointed to objects and their value types
		UnorderedSet<UINT32> serializedObjects;
		while(true)
		{
			auto iter = mObjectsToEncode.begin();
			bool foundObjectToProcess = false;
			for(; iter != mObjectsToEncode.end(); ++iter)
			{
				auto foundExisting = serializedObjects.find(iter->objectId);
				if(foundExisting != serializedObjects.end())
					continue; // Already processed

				SPtr<IReflectable> curObject = iter->object;
				UINT32 curObjectid = iter->objectId;
				serializedObjects.Insert(curObjectid);
				mObjectsToEncode.erase(iter);

				if(!encodeEntry(curObject.Get(), curObjectid, bufferedStream, flags))
				{
					BS_LOG(Error, Serialization, "Destination buffer is null or not large enough.");
					return;
				}

				foundObjectToProcess = true;

				// Ensure we keep a reference to the object so it isn't released.
				// The system assigns unique IDs to IReflectable objects based on pointer
				// addresses but if objects get released then same address could be assigned twice.
				// Note: To get around this I could assign unique IDs to IReflectable objects
				encodedObjects.push_back(curObject);

				break; // Need to start over as mObjectsToSerialize was possibly modified
			}

			if(!foundObjectToProcess) // We're done
				break;
		}

		bufferedStream.Flush(true);

		encodedObjects.Clear();
		mObjectsToEncode.Clear();
		mObjectAddrToId.Clear();

		mAlloc->Clear();
	}

	SPtr<IReflectable> BinarySerializer::decode(const SPtr<DataStream>& stream, UINT32 dataLength,
		BinarySerializerFlags flags, SerializationContext* context, std::function<void(float)> progress,
		SPtr<RTTISchema> schema)
	{
		mContext = context;
		mReportProgress = nullptr;
		mTotalBytesToRead = dataLength;
		mBuffer.Seek(0);

		if (dataLength == 0)
		{
			if(mReportProgress)
				mReportProgress(1.0f);

			return nullptr;
		}

		const size_t start = stream->Tell();
		const size_t end = start + dataLength;
		const size_t endBits = end * 8;
		mDecodeObjectMap.Clear();

		bool hasMeta = !flags.IsSet(BinarySerializerFlag::NoMeta);
		bool compress = flags.IsSet(BinarySerializerFlag::Compress);

		// Don't need a schema if we have meta-data
		if (hasMeta)
			schema = nullptr;
		else
		{
			if(!schema)
			{
				BS_LOG(Error, Serialization, "Cannot decode an object without meta-data nor schema.");
				return nullptr;
			}
		}

		BufferedBitstreamReader BufferedStream(&mBuffer, stream, PRELOAD_CHUNK_BYTES, FLUSH_AFTER_BYTES);

		// Note: Ideally we can avoid iterating twice over the stream data
		// We need to find offsets at which all objects start at so we can map object id to offset
		UINT32 rootObjectId = (UINT32)-1;
		SPtr<RTTISchema> curSchema = schema;
		do
		{
			bool isRoot = rootObjectId == (UINT32)-1;
			
			UINT32 objectId = 0;
			UINT32 objectTypeId = 0;
			bool objectIsBaseClass = false;

			UINT32 bitsRead = readObjectMetaData(bufferedStream, flags, objectId, objectTypeId, objectIsBaseClass);
			bufferedStream.Skip(-(int64_t)bitsRead);

			if (objectIsBaseClass)
			{
				BS_EXCEPT(InternalErrorException, "Encountered a base-class object while looking for a new object. " \
					"Base class objects are only supposed to be parts of a larger object.");
			}

			if (curSchema)
				objectTypeId = curSchema->typeId;

			if (isRoot)
			{
				SPtr<IReflectable> object = IReflectable::createInstanceFromTypeId(objectTypeId);
				mDecodeObjectMap.Insert(std::make_pair(objectId, ObjectToDecode(object, bufferedStream.tell(), curSchema)));
			}
			else
			{
				if(hasMeta)
				{
					SPtr<IReflectable> object = IReflectable::createInstanceFromTypeId(objectTypeId);
					mDecodeObjectMap.Insert(std::make_pair(objectId, ObjectToDecode(object, bufferedStream.tell(), curSchema)));
				}
				else
				{
					// If no meta, it's expected the pass over the root object has populated mDecodeObjectMap with object instances
					// as well as references to the schema
					auto iterFind = mDecodeObjectMap.find(objectId);
					assert(iterFind != mDecodeObjectMap.end());

					ObjectToDecode& objectToDecode = iterFind->second;
					objectToDecode.offset = bufferedStream.Tell();
					
					curSchema = objectToDecode.schema;
					objectTypeId = curSchema->typeId;
				}
			}			

			if (isRoot)
				rootObjectId = objectId;

		} while (decodeEntry(bufferedStream, endBits, flags, nullptr, curSchema));

		assert(bufferedStream.Tell() == endBits);

		// Don't set report callback until we actually do the reads
		mReportProgress = std::move(progress);
		bufferedStream.Seek((uint64_t)start * 8);

		// Now actually decode the objects
		ObjectToDecode& rootObjectToDecode = mDecodeObjectMap[rootObjectId];
		SPtr<IReflectable> rootObject = rootObjectToDecode.object;

		rootObjectToDecode.decodeInProgress = true;
		decodeEntry(bufferedStream, endBits, flags, rootObject, schema);
		rootObjectToDecode.decodeInProgress = false;
		rootObjectToDecode.isDecoded = true;

		mDecodeObjectMap.Clear();
		bufferedStream.Seek((uint64_t)endBits);
		stream->Seek(end);

		assert(bufferedStream.Tell() == endBits);

		if (mReportProgress)
			mReportProgress(1.0f);

		return rootObject;
	}

	bool BinarySerializer::EncodeEntry(IReflectable* object, UINT32 objectId, BufferedBitstreamWriter& stream, BinarySerializerFlags flags)
	{
		const bool writeMeta = !flags.IsSet(BinarySerializerFlag::NoMeta);
		const bool compress = flags.IsSet(BinarySerializerFlag::Compress);

		RTTITypeBase* rtti = object->GetRTTI();
		bool isBaseClass = false;

		FrameStack<RTTITypeBase*> rttiInstances;

		const auto cleanup = [&]()
		{
			while (!rttiInstances.empty())
			{
				RTTITypeBase* rttiInstance = rttiInstances.Top();
				rttiInstance->OnSerializationEnded(object, mContext);
				mAlloc->Destruct(rttiInstance);

				rttiInstances.pop();
			}
		};

		// If an object has base classes, we need to iterate through all of them
		do
		{
			RTTITypeBase* rttiInstance = rtti->_clone(*mAlloc);
			rttiInstances.Push(rttiInstance);

			rttiInstance->OnSerializationStarted(object, mContext);

			if (writeMeta)
			{
				// Encode object ID & type
				ObjectMetaData objectMetaData = encodeObjectMetaData(objectId, rtti->GetRTTIId(), isBaseClass);
				stream.WriteBytes(objectMetaData);
			}
			else
			{
				// Encode object ID
				UINT32 objectMetaData = encodeObjectMetaData(objectId, isBaseClass);

				if (compress)
					stream.WriteVarInt(objectMetaData);
				else
					stream.WriteBytes(objectMetaData);
			}

			const UINT32 numFields = rtti->GetNumFields();
			for(UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* curGenericField = rtti->GetField(i);

				if (writeMeta)
				{
					// Copy field ID & other meta-data like field size and type
					int metaData = encodeFieldMetaData(curGenericField->schema, false);

					stream.WriteBytes(metaData);
				}

				if(curGenericField->schema.isArray)
				{
					UINT32 arrayNumElems = curGenericField->GetArraySize(rttiInstance, object);

					// Copy num vector elements
					if (compress)
						stream.WriteVarInt(arrayNumElems);
					else
						stream.WriteBytes(arrayNumElems);

					switch(curGenericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
						{
							auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

							for(UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							{
								SPtr<IReflectable> childObject;
								
								if (!flags.IsSet(BinarySerializerFlag::Shallow))
									childObject = curField->GetArrayValue(rttiInstance, object, arrIdx);

								UINT32 objId = registerObjectPtr(childObject);
								if (compress)
									stream.WriteVarInt(objId);
								else
									stream.WriteBytes(objId);
							}

							break;
						}
					case SerializableFT_Reflectable:
						{
							auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

							for(UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							{
								IReflectable& childObject = curField->GetArrayValue(rttiInstance, object, arrIdx);

								if(!complexTypeToStream(&childObject, stream, flags))
								{
									cleanup();
									return false;
								}
							}

							break;
						}
					case SerializableFT_Plain:
						{
							auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

							for(UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
								curField->ArrayElemToStream(rttiInstance, object, arrIdx, stream.GetBitstream(), compress);

							break;
						}
					default:
						BS_LOG(Error, Serialization,
							"Error encoding data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}",
							curGenericField->schema.type, curGenericField->schema.isArray);
					}
				}
				else
				{
					switch(curGenericField->schema.type)
					{
					case SerializableFT_ReflectablePtr:
						{
							auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
							SPtr<IReflectable> childObject;
							
							if (!flags.IsSet(BinarySerializerFlag::Shallow))
								childObject = curField->GetValue(rttiInstance, object);

							UINT32 objId = registerObjectPtr(childObject);
							if (compress)
								stream.WriteVarInt(objId);
							else
								stream.WriteBytes(objId);

							break;
						}
					case SerializableFT_Reflectable:
						{
							auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
							IReflectable& childObject = curField->GetValue(rttiInstance, object);

							if(!complexTypeToStream(&childObject, stream, flags))
							{
								cleanup();
								return false;
							}

							break;
						}
					case SerializableFT_Plain:
						{
							auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);
							curField->ToStream(rttiInstance, object, stream.GetBitstream(), compress);

							break;
						}
					case SerializableFT_DataBlock:
						{
							auto* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

							UINT32 dataBlockSize = 0;
							SPtr<DataStream> blockStream = curField->GetValue(rttiInstance, object, dataBlockSize);

							// Data block size
							if (compress)
								stream.WriteVarInt(dataBlockSize);
							else
								stream.WriteBytes(dataBlockSize);

							// Data block data
							auto dataToStore = (UINT8*)bs_stack_alloc(dataBlockSize);
							blockStream->Read(dataToStore, dataBlockSize);

							stream.Align();
							stream.WriteBytes(dataToStore, dataBlockSize);
							bs_stack_free(dataToStore);

							break;
						}
					default:
						BS_LOG(Error, Serialization,
							"Error encoding data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}",
							curGenericField->schema.type, curGenericField->schema.isArray);
					}
				}

				stream.Flush(false);
			}

			rtti = rtti->GetBaseClass();
			isBaseClass = true;

		} while(rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

		cleanup();

		return true;
	}

	bool BinarySerializer::decodeEntry(BufferedBitstreamReader& stream, size_t dataEnd, BinarySerializerFlags flags,
		const SPtr<IReflectable>& output, SPtr<RTTISchema> schema)
	{
		const bool hasMeta = !flags.IsSet(BinarySerializerFlag::NoMeta);
		const bool compressed = flags.IsSet(BinarySerializerFlag::Compress);

		UINT32 objectId = 0;
		UINT32 objectTypeId = 0;
		bool objectIsBaseClass = false;

		readObjectMetaData(stream, flags, objectId, objectTypeId, objectIsBaseClass);

		if(schema)
			objectTypeId = schema->typeId;

		if (objectIsBaseClass)
		{
			BS_EXCEPT(InternalErrorException, "Encountered a base-class object while looking for a new object. " \
				"Base class objects are only supposed to be parts of a larger object.");
		}

		RTTITypeBase* rtti = nullptr;
		if(output)
			rtti = output->GetRTTI();

		FrameVector<RTTITypeBase*> rttiInstances;

		auto finalizeObject = [&rttiInstances, this](IReflectable* object)
		{
			// Note: It would make sense to finish deserializing derived classes before base classes, but some code
			// depends on the old functionality, so we'll keep it this way
			for (auto iter = rttiInstances.Rbegin(); iter != rttiInstances.rend(); ++iter)
			{
				RTTITypeBase* curRTTI = *iter;

				curRTTI->OnDeserializationEnded(object, mContext);
				mAlloc->Destruct(curRTTI);
			}

			rttiInstances.Clear();
		};

		RTTITypeBase* curRTTI = rtti;
		while (curRTTI)
		{
			RTTITypeBase* rttiInstance = curRTTI->_clone(*mAlloc);
			rttiInstances.push_back(rttiInstance);

			curRTTI = curRTTI->GetBaseClass();
		}

		// Iterate in reverse to notify base classes before derived classes
		for(auto iter = rttiInstances.Rbegin(); iter != rttiInstances.rend(); ++iter)
			(*iter)->OnDeserializationStarted(output.Get(), mContext);

		RTTITypeBase* rttiInstance = nullptr;
		UINT32 rttiInstanceIdx = 0;
		if(!rttiInstances.empty())
			rttiInstance = rttiInstances[0];

		Vector<RTTIFieldSchema>::iterator fieldSchemaIter;
		if (schema)
			fieldSchemaIter = schema->fieldSchemas.begin();
		
		while (stream.Tell() < dataEnd)
		{
			RTTIFieldSchema fieldSchema;
			SPtr<RTTISchema> fieldTypeSchema;
			bool terminator = false;

			UINT32 baseObjId = 0;
			UINT32 baseObjTypeId = 0;
			bool objIsBaseClass = false;

			if(hasMeta)
			{
				UINT8 metaDataHeader = 0;
				stream.ReadBytes(metaDataHeader);
				stream.SkipBytes(-(int32_t)sizeof(metaDataHeader));

				if (isObjectMetaData(metaDataHeader)) // We've reached a new object or a base class of the current one
				{
					UINT32 readBits = readObjectMetaData(stream, flags, baseObjId, baseObjTypeId, objIsBaseClass);

					if (!objIsBaseClass)
					{
						// Found new object, we're done
						stream.Skip(-(int64_t)readBits);

						finalizeObject(output.Get());
						return true;
					}
				}
				else
				{
					if (compressed)
						terminator = isFieldTerminator(metaDataHeader);

					if (!terminator)
					{
						UINT32 fieldMetaData;
						stream.ReadBytes(fieldMetaData);

						fieldSchema = decodeFieldMetaData(fieldMetaData, terminator);
					}
				}
			}
			else
			{
				if(fieldSchemaIter == schema->fieldSchemas.end())
				{
					// No more fields, move to base type if one exists
					if (schema->baseTypeSchema)
					{
						schema = schema->baseTypeSchema;
						fieldSchemaIter = schema->fieldSchemas.begin();
						
						baseObjTypeId = schema->typeId;
						objIsBaseClass = true;
					}
					else // Otherwise we're done here
						terminator = true;
				}
				else
				{
					fieldSchema = *fieldSchemaIter;
					fieldTypeSchema = fieldSchema.fieldTypeSchema;

					++fieldSchemaIter;
				}
			}

			if (objIsBaseClass)
			{
				if (rtti != nullptr)
					rtti = rtti->GetBaseClass();

				// Saved and current base classes don't match, so just skip over all that data
				if (rtti == nullptr || rtti->GetRTTIId() != baseObjTypeId)
					rtti = nullptr;

				rttiInstance = nullptr;

				if (rtti)
				{
					rttiInstance = rttiInstances[rttiInstanceIdx + 1];
					rttiInstanceIdx++;
				}

				continue;
			}

			if (terminator)
			{
				// We've processed the last field in this object, so return. Although we return false we don't actually know
				// if there is an object following this one. However it doesn't matter since terminator fields are only used
				// for embedded objects that are all processed within this method so we can compensate.
				finalizeObject(output.Get());
				return false;
			}

			RTTIField* curGenericField = nullptr;

			if (rtti != nullptr)
				curGenericField = rtti->FindField(fieldSchema.id);

			if (curGenericField != nullptr)
			{
				if (!fieldSchema.hasDynamicSize && curGenericField->schema.size != fieldSchema.size)
				{
					BS_EXCEPT(InternalErrorException,
							  "Data type mismatch. Type size stored in file and actual type size don't match. ("
							  + ToString(curGenericField->schema.size.bytes) + " vs. " +
							  ToString(fieldSchema.size.bytes) + ")");
				}

				if (curGenericField->schema.isArray != fieldSchema.isArray)
				{
					BS_EXCEPT(InternalErrorException,
						"Data type mismatch. One is array, other is a single type.");
				}

				if (curGenericField->schema.type != fieldSchema.type)
				{
					BS_EXCEPT(InternalErrorException,
							  "Data type mismatch. Field types don't match. " +
									  ToString(UINT32(curGenericField->schema.type)) + " vs. " +
							  ToString(UINT32(fieldSchema.type)));
				}
			}

			UINT32 arrayNumElems = 1;
			if (fieldSchema.isArray)
			{
				if (compressed)
					stream.ReadVarInt(arrayNumElems);
				else
					stream.ReadBytes(arrayNumElems);

				if(curGenericField != nullptr)
					curGenericField->SetArraySize(rttiInstance, output.Get(), arrayNumElems);

				switch (fieldSchema.type)
				{
				case SerializableFT_ReflectablePtr:
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

					for (UINT32 i = 0; i < arrayNumElems; i++)
					{
						UINT32 childObjectId = 0;
						if (compressed)
							stream.ReadVarInt(childObjectId);
						else
							stream.ReadBytes(childObjectId);

						auto findObj = mDecodeObjectMap.find(childObjectId);

						// If reading from schema we need to create object here as we don't know its type during the normal pass
						if (schema)
						{
							if (findObj == mDecodeObjectMap.end())
							{
								SPtr<IReflectable> childObj = IReflectable::createInstanceFromTypeId(fieldTypeSchema->typeId);
								mDecodeObjectMap.Insert(std::make_pair(objectId, ObjectToDecode(childObj, (UINT32)-1, fieldTypeSchema)));
							}
						}
						
						if (curField != nullptr)
						{
							if(findObj == mDecodeObjectMap.end())
							{
								if(childObjectId != 0)
								{
									BS_LOG(Warning, Generic, "When deserializing, object ID: {0} was found but no such "
										"object was contained in the file.", childObjectId);
								}

								curField->SetArrayValue(rttiInstance, output.Get(), i, nullptr);
							}
							else
							{
								ObjectToDecode& objToDecode = findObj->second;
								
								const bool needsDecoding = !curField->schema.info.flags.IsSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
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

										const uint64_t curOffset = stream.Tell();
										stream.Seek(objToDecode.offset);
										decodeEntry(stream, dataEnd, flags, objToDecode.object, fieldTypeSchema);
										stream.Seek(curOffset);

										objToDecode.decodeInProgress = false;
										objToDecode.isDecoded = true;
									}
								}

								curField->SetArrayValue(rttiInstance, output.Get(), i, objToDecode.object);
							}
						}
					}

					break;
				}
				case SerializableFT_Reflectable:
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

					for (UINT32 i = 0; i < arrayNumElems; i++)
					{
						SPtr<IReflectable> childObj;
						if(curField)
							childObj = curField->NewObject();

						decodeEntry(stream, dataEnd, flags, childObj, fieldTypeSchema);

						if (curField != nullptr)
						{
							// Note: Would be nice to avoid this copy by value and decode directly into the field
							curField->SetArrayValue(rttiInstance, output.Get(), i, *childObj);
						}
					}
					break;
				}
				case SerializableFT_Plain:
				{
					auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

					for (UINT32 i = 0; i < arrayNumElems; i++)
					{
						uint64_t typeSizeBits = fieldSchema.size.GetBits();
						if (fieldSchema.hasDynamicSize)
						{
							if(compressed)
							{
								BitLength typeSize;
								BitLength headerSize = rtti_read_size_header(stream, true, typeSize);
								stream.Skip(-(int64_t)headerSize.getBits());

								typeSizeBits = typeSize.GetBits();
							}
							else
							{
								uint32_t typeSize;
								stream.ReadBytes(typeSize);
								stream.SkipBytes(-(int32_t)sizeof(uint32_t));

								typeSizeBits = (uint64_t)typeSize * 8;
							}
						}

						if (curField != nullptr)
						{
							stream.Preload((uint32_t)Math::DivideAndRoundUp(typeSizeBits, (uint64_t)8));
							curField->ArrayElemFromBuffer(rttiInstance, output.Get(), i, stream.getBitstream(), compressed);

							stream.Skip(typeSizeBits);
						}
						else
						{
							bool builtin = fieldSchema.fieldTypeId < 16;
							if (compressed && builtin)
								skipBuiltinType(fieldSchema.fieldTypeId, stream, compressed);
							else
								stream.Skip(typeSizeBits);
						}
					}
					break;
				}
				default:
					BS_EXCEPT(InternalErrorException,
							  "Error decoding data. Encountered a type I don't know how to decode. Type: " +
									  ToString(UINT32(fieldSchema.type)) +
							  ", Is array: " + ToString(fieldSchema.isArray));
				}
			}
			else
			{
				switch (fieldSchema.type)
				{
				case SerializableFT_ReflectablePtr:
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

					UINT32 childObjectId = 0;
					if (compressed)
						stream.ReadVarInt(childObjectId);
					else
						stream.ReadBytes(childObjectId);

					auto findObj = mDecodeObjectMap.find(childObjectId);

					// If reading from schema we need to create object here as we don't know its type during the normal pass
					if (schema)
					{
						if (findObj == mDecodeObjectMap.end())
						{
							SPtr<IReflectable> childObj = IReflectable::createInstanceFromTypeId(fieldTypeSchema->typeId);
							mDecodeObjectMap.Insert(std::make_pair(objectId, ObjectToDecode(childObj, (UINT32)-1, fieldTypeSchema)));
						}
					}
						
					if (curField != nullptr)
					{
						if(findObj == mDecodeObjectMap.end())
						{
							if(childObjectId != 0)
							{
								BS_LOG(Warning, Generic, "When deserializing, object ID: {0} was found but no such object "
									"was contained in the file.", childObjectId);
							}

							curField->SetValue(rttiInstance, output.Get(), nullptr);
						}
						else
						{
							ObjectToDecode& objToDecode = findObj->second;

							const bool needsDecoding = !curField->schema.info.flags.IsSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
							if (needsDecoding)
							{
								if (objToDecode.decodeInProgress)
								{
									BS_LOG(Warning, Generic, "Detected a circular reference when decoding. Referenced "
										"object's fields will be resolved in an undefined order (i.e. one of the objects "
										"will not be fully deserialized when assigned to its field). Use "
										"RTTI_Flag_WeakRef to get rid of this warning and tell the system which of the "
										"objects is allowed to be deserialized after it is assigned to its field.");
								}
								else
								{
									objToDecode.decodeInProgress = true;

									const uint64_t curOffset = stream.Tell();
									stream.Seek(objToDecode.offset);
									decodeEntry(stream, dataEnd, flags, objToDecode.object, fieldTypeSchema);
									stream.Seek(curOffset);

									objToDecode.decodeInProgress = false;
									objToDecode.isDecoded = true;
								}
							}

							curField->SetValue(rttiInstance, output.Get(), objToDecode.object);
						}
					}

					break;
				}
				case SerializableFT_Reflectable:
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

					// Note: Ideally we can skip decoding the entry if the field no longer exists
					SPtr<IReflectable> childObj;
					if (curField)
						childObj = curField->NewObject();

					decodeEntry(stream, dataEnd, flags, childObj, fieldTypeSchema);

					if (curField != nullptr)
					{
						// Note: Would be nice to avoid this copy by value and decode directly into the field
						curField->SetValue(rttiInstance, output.Get(), *childObj);
					}

					break;
				}
				case SerializableFT_Plain:
				{
					auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

					uint64_t typeSizeBits = fieldSchema.size.GetBits();
					if (fieldSchema.hasDynamicSize)
					{
						if (compressed)
						{
							BitLength typeSize;
							BitLength headerSize = rtti_read_size_header(stream, true, typeSize);
							stream.Skip(-(int64_t)headerSize.getBits());

							typeSizeBits = typeSize.GetBits();
						}
						else
						{
							uint32_t typeSize;
							stream.ReadBytes(typeSize);
							stream.SkipBytes(-(int32_t)sizeof(uint32_t));

							typeSizeBits = (uint64_t)typeSize * 8;
						}
					}

					if (curField != nullptr)
					{
						stream.Preload((uint32_t)Math::DivideAndRoundUp(typeSizeBits, (uint64_t)8));
						curField->FromBuffer(rttiInstance, output.Get(), stream.getBitstream(), compressed);

						stream.Skip(typeSizeBits);
					}
					else
					{
						bool builtin = fieldSchema.fieldTypeId < 16;
						if (compressed && builtin)
							skipBuiltinType(fieldSchema.fieldTypeId, stream, compressed);
						else
							stream.Skip(typeSizeBits);
					}

					break;
				}
				case SerializableFT_DataBlock:
				{
					auto* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

					// Data block size
					UINT32 dataBlockSize = 0;
					if (compressed)
						stream.ReadVarInt(dataBlockSize);
					else
						stream.ReadBytes(dataBlockSize);

					stream.Align();

					// Data block data
					if (curField != nullptr)
					{
						const SPtr<DataStream>& dataStream = stream.GetDataStream();
						if (dataStream->IsFile()) // Allow streaming
						{
							uint64_t curOffset = stream.Tell();

							// Data blocks don't support data that isn't byte aligned, but encoder should take care of that
							assert((curOffset % 8) == 0);
							curOffset /= 8;
							
							dataStream->Seek(curOffset);
							curField->SetValue(rttiInstance, output.Get(), dataStream, dataBlockSize);

							stream.SkipBytes(dataBlockSize);
						}
						else
						{
							SPtr<MemoryDataStream> dataBlockStream = bs_shared_ptr_new<MemoryDataStream>(dataBlockSize);
							stream.ReadBytes(dataBlockStream->Data(), dataBlockSize);

							curField->SetValue(rttiInstance, output.Get(), dataBlockStream, dataBlockSize);
						}
					}
					else
						stream.SkipBytes(dataBlockSize);

					break;
				}
				default:
					BS_EXCEPT(InternalErrorException,
							  "Error decoding data. Encountered a type I don't know how to decode. Type: " +
									  ToString(UINT32(fieldSchema.type)) +
							  ", Is array: " + ToString(fieldSchema.isArray));
				}

				stream.ClearBuffered(false);
			}

			UINT32 bytesRead = (UINT32)Math::DivideAndRoundUp(stream.Tell(), (uint64_t)8);
			if (mReportProgress && (bytesRead >= mNextProgressReport))
			{
				UINT32 lastReport = (bytesRead / REPORT_AFTER_BYTES) * REPORT_AFTER_BYTES;
				mNextProgressReport = lastReport + REPORT_AFTER_BYTES;

				mReportProgress(bytesRead / (float)mTotalBytesToRead);
			}
		}

		finalizeObject(output.Get());
		return false;
	}

	bool BinarySerializer::ComplexTypeToStream(IReflectable* object, BufferedBitstreamWriter& stream, BinarySerializerFlags flags)
	{
		if (object != nullptr)
		{
			if (!encodeEntry(object, 0, stream, flags))
				return false;

			if (!flags.IsSet(BinarySerializerFlag::NoMeta))
			{
				// Encode terminator field
				// Complex types require terminator fields because they can be embedded within other complex types and we need
				// to know when their fields end and parent's resume
				if(flags.IsSet(BinarySerializerFlag::Compress))
				{
					UINT8 metaData = encodeFieldTerminator();
					stream.WriteBytes(metaData);
				}
				else
				{
					int metaData = encodeFieldMetaData(RTTIFieldSchema(), true);
					stream.WriteBytes(metaData);
				}
			}
		}

		return true;
	}

	UINT32 BinarySerializer::EncodeFieldMetaData(const RTTIFieldSchema& schema, bool terminator)
	{
		// If O == 0 - Meta contains field information (Encoded using this method)
		//// Encoding if E = 0: IIII IIII IIII IIII SSSS SSSS ETYP DCAO
		//// Encoding if E = 1: IIII IIII IIII IIII BBBB xxxx ETYP DCAO
		//// I - Id
		//// S - Size
		//// C - Complex
		//// A - Array
		//// D - Data block
		//// P - Complex ptr
		//// O - Object descriptor
		//// Y - Plain field has dynamic size
		//// T - Terminator (last field in an object)
		//// E - Extended (size is replaced with additional meta-Data)
		//// B - Built-in type ID

		bool isBuiltin = schema.GetTypeId() < 16;
		UINT32 sizeBytes = schema.hasDynamicSize ? 0 : schema.size.bytes;

		if(!isBuiltin)
			return (schema.id << 16 | sizeBytes << 8 |
				(schema.isArray ? 0x02 : 0) |
				((schema.type == SerializableFT_DataBlock) ? 0x04 : 0) |
				((schema.type == SerializableFT_Reflectable) ? 0x08 : 0) |
				((schema.type == SerializableFT_ReflectablePtr) ? 0x10 : 0) |
				(schema.hasDynamicSize ? 0x20 : 0) |
				(terminator ? 0x40 : 0));
		else
			return (schema.id << 16 | (schema.GetTypeId() & 0xF) << 12 |
				(schema.isArray ? 0x02 : 0) |
				((schema.type == SerializableFT_DataBlock) ? 0x04 : 0) |
				((schema.type == SerializableFT_Reflectable) ? 0x08 : 0) |
				((schema.type == SerializableFT_ReflectablePtr) ? 0x10 : 0) |
				(schema.hasDynamicSize ? 0x20 : 0) |
				(terminator ? 0x40 : 0)) |
				0x80;

	}

	RTTIFieldSchema BinarySerializer::DecodeFieldMetaData(UINT32 encodedData, bool& terminator)
	{
		if(isObjectMetaData(encodedData))
		{
			BS_EXCEPT(InternalErrorException,
				"Meta data represents an object description but is trying to be decoded as a field descriptor.");
		}

		terminator = (encodedData & 0x40) != 0;

		RTTIFieldSchema schema;
		schema.hasDynamicSize = (encodedData & 0x20) != 0;

		if((encodedData & 0x10) != 0)
			schema.type = SerializableFT_ReflectablePtr;
		else if((encodedData & 0x08) != 0)
			schema.type = SerializableFT_Reflectable;
		else if((encodedData & 0x04) != 0)
			schema.type = SerializableFT_DataBlock;
		else
			schema.type = SerializableFT_Plain;

		schema.isArray = (encodedData & 0x02) != 0;
		schema.id = (UINT16)((encodedData >> 16) & 0xFFFF);

		bool extended = (encodedData & 0x80) != 0;
		if (!extended)
			schema.size = (UINT8)((encodedData >> 8) & 0xFF);
		else
			schema.fieldTypeId = (UINT8)((encodedData >> 12) & 0xF);

		return schema;
	}

	UINT8 BinarySerializer::EncodeFieldTerminator()
	{
		// See the documentation for encodeFieldMetaData() on why we're using this format
		return 0x40;
	}

	void BinarySerializer::SkipBuiltinType(UINT32 fieldType, BufferedBitstreamReader& stream, bool compressed)
	{
		switch(fieldType)
		{
		case TID_Bool:
		{
			bool data;
			stream.Preload(sizeof(data));
			RTTIPlainType<bool>::fromMemory(data, stream.GetBitstream(), RTTIFieldInfo(), compressed);
			break;
		}
		case TID_Int32:
		{
			int32_t data;
			stream.Preload(sizeof(data));
			RTTIPlainType<int32_t>::fromMemory(data, stream.GetBitstream(), RTTIFieldInfo(), compressed);
			break;
		}
		case TID_UInt32:
		{
			uint32_t data;
			stream.Preload(sizeof(data));
			RTTIPlainType<uint32_t>::fromMemory(data, stream.GetBitstream(), RTTIFieldInfo(), compressed);
			break;
		}
		default:
			assert(false);
			break;
		}
	}

	bool BinarySerializer::IsFieldTerminator(UINT8 data)
	{
		if(isObjectMetaData(data))
		{
			BS_EXCEPT(InternalErrorException,
				"Meta data represents an object description but is trying to be decoded as a field descriptor.");
		}

		return (data & 0x40) != 0;
	}

	BinarySerializer::ObjectMetaData BinarySerializer::EncodeObjectMetaData(UINT32 objId, UINT32 objTypeId, bool isBaseClass)
	{
		// If O == 1 - Meta contains object instance information (Encoded using encodeObjectMetaData)
		//// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
		//// S - Size of the object identifier
		//// O - Object descriptor
		//// B - Base class indicator

		if(objId > 1073741823)
		{
			BS_EXCEPT(InvalidParametersException, "Object ID is larger than we can store (max 30 bits): " +
					ToString(objId));
		}

		ObjectMetaData metaData;
		metaData.objectMeta = encodeObjectMetaData(objId, isBaseClass);
		metaData.typeId = objTypeId;
		return metaData;
	}

	void BinarySerializer::DecodeObjectMetaData(ObjectMetaData encodedData, UINT32& objId, UINT32& objTypeId, bool& isBaseClass)
	{
		if(!isObjectMetaData(encodedData.objectMeta))
		{
			BS_EXCEPT(InternalErrorException,
				"Meta data represents a field description but is trying to be decoded as an object descriptor.");
		}

		decodeObjectMetaData(encodedData.objectMeta, objId, isBaseClass);
		objTypeId = encodedData.typeId;
	}

	UINT32 BinarySerializer::EncodeObjectMetaData(UINT32 objId, bool isBaseClass)
	{
		// If O == 1 - Meta contains object instance information (Encoded using encodeObjectMetaData)
		//// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
		//// S - Size of the object identifier
		//// O - Object descriptor
		//// B - Base class indicator

		if(objId > 1073741823)
		{
			BS_EXCEPT(InvalidParametersException, "Object ID is larger than we can store (max 30 bits): " +
					ToString(objId));
		}

		return (objId << 2) | (isBaseClass ? 0x02 : 0) | 0x01;
	}

	void BinarySerializer::DecodeObjectMetaData(UINT32 encodedData, UINT32& objId, bool& isBaseClass)
	{
		if(!isObjectMetaData(encodedData))
		{
			BS_EXCEPT(InternalErrorException,
				"Meta data represents a field description but is trying to be decoded as an object descriptor.");
		}

		objId = (encodedData >> 2) & 0x3FFFFFFF;
		isBaseClass = (encodedData & 0x02) != 0;
	}

	bool BinarySerializer::IsObjectMetaData(UINT32 encodedData)
	{
		return ((encodedData & 0x01) != 0);
	}

	UINT32 BinarySerializer::ReadObjectMetaData(BufferedBitstreamReader& stream, BinarySerializerFlags flags, uint32_t& objId, uint32_t& objTypeId, bool& isBaseType)
	{
		if (!flags.IsSet(BinarySerializerFlag::NoMeta))
		{
			ObjectMetaData objectMetaData;
			objectMetaData.objectMeta = 0;
			objectMetaData.typeId = 0;

			if (stream.ReadBytes(objectMetaData) != sizeof(ObjectMetaData))
				BS_EXCEPT(InternalErrorException, "Error decoding data.");

			decodeObjectMetaData(objectMetaData, objId, objTypeId, isBaseType);
			return Sizeof(ObjectMetaData) * 8;
		}
		else
		{
			UINT32 objectMetaData = 0;

			UINT32 bitsRead = 0;
			if (flags.IsSet(BinarySerializerFlag::Compress))
				bitsRead = stream.ReadVarInt(objectMetaData);
			else
			{
				if (stream.ReadBytes(objectMetaData) != sizeof(objectMetaData))
					BS_EXCEPT(InternalErrorException, "Error decoding data.");

				bitsRead = sizeof(objectMetaData) * 8;
			}

			decodeObjectMetaData(objectMetaData, objId, isBaseType);
			objTypeId = 0; // Unavailable, needs to be read from a schema

			return bitsRead;
		}
	}

	UINT32 BinarySerializer::FindOrCreatePersistentId(IReflectable* object)
	{
		void* ptrAddress = (void*)object;

		auto findIter = mObjectAddrToId.find(ptrAddress);
		if(findIter != mObjectAddrToId.end())
			return findIter->second;

		UINT32 objId = mLastUsedObjectId++;
		mObjectAddrToId.Insert(std::make_pair(ptrAddress, objId));

		return objId;
	}

	UINT32 BinarySerializer::RegisterObjectPtr(SPtr<IReflectable> object)
	{
		if(object == nullptr)
			return 0;

		void* ptrAddress = (void*)object.Get();

		auto iterFind = mObjectAddrToId.find(ptrAddress);
		if(iterFind == mObjectAddrToId.end())
		{
			UINT32 objId = findOrCreatePersistentId(object.Get());

			mObjectsToEncode.push_back(ObjectToEncode(objId, object));
			mObjectAddrToId.Insert(std::make_pair(ptrAddress, objId));

			return objId;
		}

		return iterFind->second;
	}
}

#undef COPY_TO_BUFFER
