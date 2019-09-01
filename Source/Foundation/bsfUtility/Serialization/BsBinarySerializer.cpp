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

	void BinarySerializer::encode(IReflectable* object, const SPtr<DataStream>& stream, bool shallow, SerializationContext* context)
	{
		mObjectsToEncode.clear();
		mObjectAddrToId.clear();
		mLastUsedObjectId = 1;
		mContext = context;
		mBuffer.seek(0);

		mAlloc->markFrame();

		BufferedBitstreamWriter bufferedStream(&mBuffer, stream, WRITE_BUFFER_SIZE, FLUSH_AFTER_BYTES);

		Vector<SPtr<IReflectable>> encodedObjects;
		UINT32 objectId = findOrCreatePersistentId(object);
		
		// Encode primary object and its value types
		if (!encodeEntry(object, objectId, bufferedStream, shallow))
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
				serializedObjects.insert(curObjectid);
				mObjectsToEncode.erase(iter);

				if(!encodeEntry(curObject.get(), curObjectid, bufferedStream, shallow))
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

		bufferedStream.flush(true);

		encodedObjects.clear();
		mObjectsToEncode.clear();
		mObjectAddrToId.clear();

		mAlloc->clear();
	}

	SPtr<IReflectable> BinarySerializer::decode(const SPtr<DataStream>& stream, UINT32 dataLength,
		SerializationContext* context, std::function<void(float)> progress)
	{
		mContext = context;
		mReportProgress = nullptr;
		mTotalBytesToRead = dataLength;
		mBuffer.seek(0);

		if (dataLength == 0)
		{
			if(mReportProgress)
				mReportProgress(1.0f);

			return nullptr;
		}

		const size_t start = stream->tell();
		const size_t end = start + dataLength;
		const size_t endBits = end * 8;
		mDecodeObjectMap.clear();

		BufferedBitstreamReader bufferedStream(&mBuffer, stream, PRELOAD_CHUNK_BYTES, FLUSH_AFTER_BYTES);

		// Note: Ideally we can avoid iterating twice over the stream data
		// Create empty instances of all ptr objects
		SPtr<IReflectable> rootObject = nullptr;
		do
		{
			ObjectMetaData objectMetaData;
			objectMetaData.objectMeta = 0;
			objectMetaData.typeId = 0;

			if(bufferedStream.readBytes(objectMetaData) != sizeof(ObjectMetaData))
				BS_EXCEPT(InternalErrorException, "Error decoding data.");

			bufferedStream.skipBytes(-(int32_t)sizeof(ObjectMetaData));

			UINT32 objectId = 0;
			UINT32 objectTypeId = 0;
			bool objectIsBaseClass = false;
			decodeObjectMetaData(objectMetaData, objectId, objectTypeId, objectIsBaseClass);

			if (objectIsBaseClass)
			{
				BS_EXCEPT(InternalErrorException, "Encountered a base-class object while looking for a new object. " \
					"Base class objects are only supposed to be parts of a larger object.");
			}

			SPtr<IReflectable> object = IReflectable::createInstanceFromTypeId(objectTypeId);
			mDecodeObjectMap.insert(std::make_pair(objectId, ObjectToDecode(object, bufferedStream.tell())));

			if(rootObject == nullptr)
				rootObject = object;

		} while (decodeEntry(bufferedStream, endBits, nullptr));

		assert(bufferedStream.tell() == endBits);

		// Don't set report callback until we actually do the reads
		mReportProgress = std::move(progress);
		bufferedStream.seek((uint32_t)start * 8);

		// Now go through all of the objects and actually decode them
		for(auto iter = mDecodeObjectMap.begin(); iter != mDecodeObjectMap.end(); ++iter)
		{
			ObjectToDecode& objToDecode = iter->second;

			if(objToDecode.isDecoded)
				continue;

			bufferedStream.seek((uint32_t)objToDecode.offset);

			objToDecode.decodeInProgress = true;
			decodeEntry(bufferedStream, endBits, objToDecode.object);
			objToDecode.decodeInProgress = false;
			objToDecode.isDecoded = true;
		}

		mDecodeObjectMap.clear();
		bufferedStream.seek(endBits);
		stream->seek(end);

		assert(bufferedStream.tell() == endBits);

		if(mReportProgress)
			mReportProgress(1.0f);

		return rootObject;
	}

	bool BinarySerializer::encodeEntry(IReflectable* object, UINT32 objectId, BufferedBitstreamWriter& stream, bool shallow)
	{
		RTTITypeBase* rtti = object->getRTTI();
		bool isBaseClass = false;

		FrameStack<RTTITypeBase*> rttiInstances;

		const auto cleanup = [&]()
		{
			while (!rttiInstances.empty())
			{
				RTTITypeBase* rttiInstance = rttiInstances.top();
				rttiInstance->onSerializationEnded(object, mContext);
				mAlloc->destruct(rttiInstance);

				rttiInstances.pop();
			}
		};

		// If an object has base classes, we need to iterate through all of them
		do
		{
			RTTITypeBase* rttiInstance = rtti->_clone(*mAlloc);
			rttiInstances.push(rttiInstance);

			rttiInstance->onSerializationStarted(object, mContext);

			// Encode object ID & type
			ObjectMetaData objectMetaData = encodeObjectMetaData(objectId, rtti->getRTTIId(), isBaseClass);
			stream.writeBytes(objectMetaData);

			const UINT32 numFields = rtti->getNumFields();
			for(UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* curGenericField = rtti->getField(i);

				// Copy field ID & other meta-data like field size and type
				int metaData = encodeFieldMetaData(curGenericField->mUniqueId, curGenericField->getTypeSize(),
					curGenericField->mIsVectorType, curGenericField->mType, curGenericField->hasDynamicSize(), false);

				stream.writeBytes(metaData);
				static_assert(sizeof(metaData) == META_SIZE, "Size mismatch");

				if(curGenericField->mIsVectorType)
				{
					UINT32 arrayNumElems = curGenericField->getArraySize(rttiInstance, object);

					// Copy num vector elements
					stream.writeBytes(arrayNumElems);
					static_assert(sizeof(arrayNumElems) == NUM_ELEM_FIELD_SIZE, "Size mismatch");

					switch(curGenericField->mType)
					{
					case SerializableFT_ReflectablePtr:
						{
							RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

							for(UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							{
								SPtr<IReflectable> childObject;
								
								if (!shallow)
									childObject = curField->getArrayValue(rttiInstance, object, arrIdx);

								UINT32 objId = registerObjectPtr(childObject);
								stream.writeBytes(objId);
							}

							break;
						}
					case SerializableFT_Reflectable:
						{
							RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

							for(UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							{
								IReflectable& childObject = curField->getArrayValue(rttiInstance, object, arrIdx);

								if(!complexTypeToStream(&childObject, stream, shallow))
								{
									cleanup();
									return false;
								}
							}

							break;
						}
					case SerializableFT_Plain:
						{
							RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

							for(UINT32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
								curField->arrayElemToStream(rttiInstance, object, arrIdx, stream.getBitstream());

							break;
						}
					default:
						BS_LOG(Error, Serialization,
							"Error encoding data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}",
							curGenericField->mType, curGenericField->mIsVectorType);
					}
				}
				else
				{
					switch(curGenericField->mType)
					{
					case SerializableFT_ReflectablePtr:
						{
							RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
							SPtr<IReflectable> childObject;
							
							if (!shallow)
								childObject = curField->getValue(rttiInstance, object);

							UINT32 objId = registerObjectPtr(childObject);
							stream.writeBytes(objId);

							break;
						}
					case SerializableFT_Reflectable:
						{
							RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
							IReflectable& childObject = curField->getValue(rttiInstance, object);

							if(!complexTypeToStream(&childObject, stream, shallow))
							{
								cleanup();
								return false;
							}

							break;
						}
					case SerializableFT_Plain:
						{
							RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);
							curField->toStream(rttiInstance, object, stream.getBitstream());

							break;
						}
					case SerializableFT_DataBlock:
						{
							RTTIManagedDataBlockFieldBase* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

							UINT32 dataBlockSize = 0;
							SPtr<DataStream> blockStream = curField->getValue(rttiInstance, object, dataBlockSize);

							// Data block size
							stream.writeBytes(dataBlockSize);

							// Data block data
							auto dataToStore = (UINT8*)bs_stack_alloc(dataBlockSize);
							blockStream->read(dataToStore, dataBlockSize);

							stream.writeBytes(dataToStore, dataBlockSize);
							bs_stack_free(dataToStore);

							break;
						}
					default:
						BS_LOG(Error, Serialization,
							"Error encoding data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}",
							curGenericField->mType, curGenericField->mIsVectorType);
					}
				}

				stream.flush(false);
			}

			rtti = rtti->getBaseClass();
			isBaseClass = true;

		} while(rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

		cleanup();

		return true;
	}

	bool BinarySerializer::decodeEntry(BufferedBitstreamReader& stream, size_t dataEnd, const SPtr<IReflectable>& output)
	{
		ObjectMetaData objectMetaData;
		objectMetaData.objectMeta = 0;
		objectMetaData.typeId = 0;

		stream.readBytes(objectMetaData);

		UINT32 objectId = 0;
		UINT32 objectTypeId = 0;
		bool objectIsBaseClass = false;
		decodeObjectMetaData(objectMetaData, objectId, objectTypeId, objectIsBaseClass);

		if (objectIsBaseClass)
		{
			BS_EXCEPT(InternalErrorException, "Encountered a base-class object while looking for a new object. " \
				"Base class objects are only supposed to be parts of a larger object.");
		}

		RTTITypeBase* rtti = nullptr;
		if(output)
			rtti = output->getRTTI();

		FrameVector<RTTITypeBase*> rttiInstances;

		auto finalizeObject = [&rttiInstances, this](IReflectable* object)
		{
			// Note: It would make sense to finish deserializing derived classes before base classes, but some code
			// depends on the old functionality, so we'll keep it this way
			for (auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
			{
				RTTITypeBase* curRTTI = *iter;

				curRTTI->onDeserializationEnded(object, mContext);
				mAlloc->destruct(curRTTI);
			}

			rttiInstances.clear();
		};

		RTTITypeBase* curRTTI = rtti;
		while (curRTTI)
		{
			RTTITypeBase* rttiInstance = curRTTI->_clone(*mAlloc);
			rttiInstances.push_back(rttiInstance);

			curRTTI = curRTTI->getBaseClass();
		}

		// Iterate in reverse to notify base classes before derived classes
		for(auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
			(*iter)->onDeserializationStarted(output.get(), mContext);

		RTTITypeBase* rttiInstance = nullptr;
		UINT32 rttiInstanceIdx = 0;
		if(!rttiInstances.empty())
			rttiInstance = rttiInstances[0];

		while (stream.tell() < dataEnd)
		{
			int metaData = -1;
			stream.readBytes(metaData);

			if (isObjectMetaData(metaData)) // We've reached a new object or a base class of the current one
			{
				ObjectMetaData objMetaData;
				objMetaData.objectMeta = 0;
				objMetaData.typeId = 0;

				stream.skipBytes(-META_SIZE);
				stream.readBytes(objMetaData);

				UINT32 objId = 0;
				UINT32 objTypeId = 0;
				bool objIsBaseClass = false;
				decodeObjectMetaData(objMetaData, objId, objTypeId, objIsBaseClass);

				// If it's a base class, get base class RTTI and handle that
				if (objIsBaseClass)
				{
					if (rtti != nullptr)
						rtti = rtti->getBaseClass();

					// Saved and current base classes don't match, so just skip over all that data
					if (rtti == nullptr || rtti->getRTTIId() != objTypeId)
						rtti = nullptr;

					rttiInstance = nullptr;

					if (rtti)
					{
						rttiInstance = rttiInstances[rttiInstanceIdx + 1];
						rttiInstanceIdx++;
					}

					continue;
				}
				else
				{
					// Found new object, we're done
					stream.skipBytes(-(int32_t)sizeof(ObjectMetaData));

					finalizeObject(output.get());
					return true;
				}
			}

			bool isArray;
			SerializableFieldType fieldType;
			UINT16 fieldId;
			UINT8 fieldSize;
			bool hasDynamicSize;
			bool terminator;
			decodeFieldMetaData(metaData, fieldId, fieldSize, isArray, fieldType, hasDynamicSize, terminator);

			if (terminator)
			{
				// We've processed the last field in this object, so return. Although we return false we don't actually know
				// if there is an object following this one. However it doesn't matter since terminator fields are only used
				// for embedded objects that are all processed within this method so we can compensate.
				finalizeObject(output.get());
				return false;
			}

			RTTIField* curGenericField = nullptr;

			if (rtti != nullptr)
				curGenericField = rtti->findField(fieldId);

			if (curGenericField != nullptr)
			{
				if (!hasDynamicSize && curGenericField->getTypeSize() != fieldSize)
				{
					BS_EXCEPT(InternalErrorException,
						"Data type mismatch. Type size stored in file and actual type size don't match. ("
						+ toString(curGenericField->getTypeSize()) + " vs. " + toString(fieldSize) + ")");
				}

				if (curGenericField->mIsVectorType != isArray)
				{
					BS_EXCEPT(InternalErrorException,
						"Data type mismatch. One is array, other is a single type.");
				}

				if (curGenericField->mType != fieldType)
				{
					BS_EXCEPT(InternalErrorException,
						"Data type mismatch. Field types don't match. " + toString(UINT32(curGenericField->mType)) + " vs. " + toString(UINT32(fieldType)));
				}
			}

			int arrayNumElems = 1;
			if (isArray)
			{
				stream.readBytes(arrayNumElems);

				if(curGenericField != nullptr)
					curGenericField->setArraySize(rttiInstance, output.get(), arrayNumElems);

				switch (fieldType)
				{
				case SerializableFT_ReflectablePtr:
				{
					RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

					for (int i = 0; i < arrayNumElems; i++)
					{
						int childObjectId = 0;
						stream.readBytes(childObjectId);

						if (curField != nullptr)
						{
							auto findObj = mDecodeObjectMap.find(childObjectId);

							if(findObj == mDecodeObjectMap.end())
							{
								if(childObjectId != 0)
								{
									BS_LOG(Warning, Generic, "When deserializing, object ID: {0} was found but no such "
										"object was contained in the file.", childObjectId);
								}

								curField->setArrayValue(rttiInstance, output.get(), i, nullptr);
							}
							else
							{
								ObjectToDecode& objToDecode = findObj->second;

								const bool needsDecoding = !curField->getInfo().flags.isSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
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

										const uint32_t curOffset = stream.tell();
										stream.seek((uint32_t)objToDecode.offset);
										decodeEntry(stream, dataEnd, objToDecode.object);
										stream.seek(curOffset);

										objToDecode.decodeInProgress = false;
										objToDecode.isDecoded = true;
									}
								}

								curField->setArrayValue(rttiInstance, output.get(), i, objToDecode.object);
							}
						}
					}

					break;
				}
				case SerializableFT_Reflectable:
				{
					RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

					for (int i = 0; i < arrayNumElems; i++)
					{
						SPtr<IReflectable> childObj;
						if(curField)
							childObj = curField->newObject();

						decodeEntry(stream, dataEnd, childObj);

						if (curField != nullptr)
						{
							// Note: Would be nice to avoid this copy by value and decode directly into the field
							curField->setArrayValue(rttiInstance, output.get(), i, *childObj);
						}
					}
					break;
				}
				case SerializableFT_Plain:
				{
					RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

					for (int i = 0; i < arrayNumElems; i++)
					{
						UINT32 typeSize = fieldSize;
						if (hasDynamicSize)
						{
							stream.readBytes(typeSize);
							stream.skipBytes(-(int32_t)sizeof(uint32_t));
						}

						if (curField != nullptr)
						{
							stream.preload(typeSize);
							curField->arrayElemFromBuffer(rttiInstance, output.get(), i, stream.getBitstream());
							stream.skipBytes(typeSize);
						}
						else
							stream.skipBytes(typeSize);
					}
					break;
				}
				default:
					BS_EXCEPT(InternalErrorException,
						"Error decoding data. Encountered a type I don't know how to decode. Type: " + toString(UINT32(fieldType)) +
						", Is array: " + toString(isArray));
				}
			}
			else
			{
				switch (fieldType)
				{
				case SerializableFT_ReflectablePtr:
				{
					RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

					int childObjectId = 0;
					stream.readBytes(childObjectId);

					if (curField != nullptr)
					{
						auto findObj = mDecodeObjectMap.find(childObjectId);

						if(findObj == mDecodeObjectMap.end())
						{
							if(childObjectId != 0)
							{
								BS_LOG(Warning, Generic, "When deserializing, object ID: {0} was found but no such object "
									"was contained in the file.", childObjectId);
							}

							curField->setValue(rttiInstance, output.get(), nullptr);
						}
						else
						{
							ObjectToDecode& objToDecode = findObj->second;

							const bool needsDecoding = !curField->getInfo().flags.isSet(RTTIFieldFlag::WeakRef) && !objToDecode.isDecoded;
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

									const size_t curOffset = stream.tell();
									stream.seek((uint32_t)objToDecode.offset);
									decodeEntry(stream, dataEnd, objToDecode.object);
									stream.seek((uint32_t)curOffset);

									objToDecode.decodeInProgress = false;
									objToDecode.isDecoded = true;
								}
							}

							curField->setValue(rttiInstance, output.get(), objToDecode.object);
						}
					}

					break;
				}
				case SerializableFT_Reflectable:
				{
					RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

					// Note: Ideally we can skip decoding the entry if the field no longer exists
					SPtr<IReflectable> childObj;
					if (curField)
						childObj = curField->newObject();

					decodeEntry(stream, dataEnd, childObj);

					if (curField != nullptr)
					{
						// Note: Would be nice to avoid this copy by value and decode directly into the field
						curField->setValue(rttiInstance, output.get(), *childObj);
					}

					break;
				}
				case SerializableFT_Plain:
				{
					RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

					UINT32 typeSize = fieldSize;
					if (hasDynamicSize)
					{
						stream.readBytes(typeSize);
						stream.skipBytes(-(int32_t)sizeof(UINT32));
					}

					if (curField != nullptr)
					{
						stream.preload(typeSize);
						curField->fromBuffer(rttiInstance, output.get(), stream.getBitstream());
						stream.skipBytes(typeSize);
					}
					else
						stream.skipBytes(typeSize);

					break;
				}
				case SerializableFT_DataBlock:
				{
					RTTIManagedDataBlockFieldBase* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

					// Data block size
					UINT32 dataBlockSize = 0;
					stream.readBytes(dataBlockSize);

					// Data block data
					if (curField != nullptr)
					{
						const SPtr<DataStream>& dataStream = stream.getDataStream();
						if (dataStream->isFile()) // Allow streaming
						{
							size_t curOffset = stream.tell();

							// Data blocks don't support sub-byte (compressed) data.
							assert((curOffset % 8) == 0);
							curOffset /= 8;
							
							dataStream->seek(curOffset);
							curField->setValue(rttiInstance, output.get(), dataStream, dataBlockSize);

							stream.skipBytes(dataBlockSize);
						}
						else
						{
							SPtr<MemoryDataStream> dataBlockStream = bs_shared_ptr_new<MemoryDataStream>(dataBlockSize);
							stream.readBytes(dataBlockStream->data(), dataBlockSize);

							curField->setValue(rttiInstance, output.get(), dataBlockStream, dataBlockSize);
						}
					}
					else
						stream.skipBytes(dataBlockSize);

					break;
				}
				default:
					BS_EXCEPT(InternalErrorException,
						"Error decoding data. Encountered a type I don't know how to decode. Type: " + toString(UINT32(fieldType)) +
						", Is array: " + toString(isArray));
				}

				stream.clearBuffered(false);
			}

			uint32_t bytesRead = Math::divideAndRoundUp(stream.tell(), 8U);
			if (mReportProgress && (bytesRead >= mNextProgressReport))
			{
				UINT32 lastReport = (bytesRead / REPORT_AFTER_BYTES) * REPORT_AFTER_BYTES;
				mNextProgressReport = lastReport + REPORT_AFTER_BYTES;

				mReportProgress(bytesRead / (float)mTotalBytesToRead);
			}
		}

		finalizeObject(output.get());
		return false;
	}

	bool BinarySerializer::complexTypeToStream(IReflectable* object, BufferedBitstreamWriter& stream, bool shallow)
	{
		if (object != nullptr)
		{
			if (!encodeEntry(object, 0, stream, shallow))
				return false;

			// Encode terminator field
			// Complex types require terminator fields because they can be embedded within other complex types and we need
			// to know when their fields end and parent's resume
			int metaData = encodeFieldMetaData(0, 0, false, SerializableFT_Plain, false, true);
			stream.writeBytes(metaData);
			static_assert(sizeof(metaData) == META_SIZE, "Size mismatch");
		}

		return true;
	}

	UINT32 BinarySerializer::encodeFieldMetaData(UINT16 id, UINT8 size, bool array, SerializableFieldType type,
		bool hasDynamicSize, bool terminator)
	{
		// If O == 0 - Meta contains field information (Encoded using this method)
		//// Encoding: IIII IIII IIII IIII SSSS SSSS xTYP DCAO
		//// I - Id
		//// S - Size
		//// C - Complex
		//// A - Array
		//// D - Data block
		//// P - Complex ptr
		//// O - Object descriptor
		//// Y - Plain field has dynamic size
		//// T - Terminator (last field in an object)

		return (id << 16 | size << 8 |
			(array ? 0x02 : 0) |
			((type == SerializableFT_DataBlock) ? 0x04 : 0) |
			((type == SerializableFT_Reflectable) ? 0x08 : 0) |
			((type == SerializableFT_ReflectablePtr) ? 0x10 : 0) |
			(hasDynamicSize ? 0x20 : 0) |
			(terminator ? 0x40 : 0)); // TODO - Low priority. Technically I could encode this much more tightly, and use var-ints for ID
	}

	void BinarySerializer::decodeFieldMetaData(UINT32 encodedData, UINT16& id, UINT8& size,
		bool& array, SerializableFieldType& type, bool& hasDynamicSize, bool& terminator)
	{
		if(isObjectMetaData(encodedData))
		{
			BS_EXCEPT(InternalErrorException,
				"Meta data represents an object description but is trying to be decoded as a field descriptor.");
		}

		terminator = (encodedData & 0x40) != 0;
		hasDynamicSize = (encodedData & 0x20) != 0;

		if((encodedData & 0x10) != 0)
			type = SerializableFT_ReflectablePtr;
		else if((encodedData & 0x08) != 0)
			type = SerializableFT_Reflectable;
		else if((encodedData & 0x04) != 0)
			type = SerializableFT_DataBlock;
		else
			type = SerializableFT_Plain;

		array = (encodedData & 0x02) != 0;
		size = (UINT8)((encodedData >> 8) & 0xFF);
		id = (UINT16)((encodedData >> 16) & 0xFFFF);
	}

	BinarySerializer::ObjectMetaData BinarySerializer::encodeObjectMetaData(UINT32 objId, UINT32 objTypeId, bool isBaseClass)
	{
		// If O == 1 - Meta contains object instance information (Encoded using encodeObjectMetaData)
		//// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
		//// S - Size of the object identifier
		//// O - Object descriptor
		//// B - Base class indicator

		if(objId > 1073741823)
		{
			BS_EXCEPT(InvalidParametersException, "Object ID is larger than we can store (max 30 bits): " + toString(objId));
		}

		ObjectMetaData metaData;
		metaData.objectMeta = (objId << 2) | (isBaseClass ? 0x02 : 0) | 0x01;
		metaData.typeId = objTypeId;
		return metaData;
	}

	void BinarySerializer::decodeObjectMetaData(ObjectMetaData encodedData, UINT32& objId, UINT32& objTypeId, bool& isBaseClass)
	{
		if(!isObjectMetaData(encodedData.objectMeta))
		{
			BS_EXCEPT(InternalErrorException,
				"Meta data represents a field description but is trying to be decoded as an object descriptor.");
		}

		objId = (encodedData.objectMeta >> 2) & 0x3FFFFFFF;
		isBaseClass = (encodedData.objectMeta & 0x02) != 0;
		objTypeId = encodedData.typeId;
	}

	bool BinarySerializer::isObjectMetaData(UINT32 encodedData)
	{
		return ((encodedData & 0x01) != 0);
	}

	UINT32 BinarySerializer::findOrCreatePersistentId(IReflectable* object)
	{
		void* ptrAddress = (void*)object;

		auto findIter = mObjectAddrToId.find(ptrAddress);
		if(findIter != mObjectAddrToId.end())
			return findIter->second;

		UINT32 objId = mLastUsedObjectId++;
		mObjectAddrToId.insert(std::make_pair(ptrAddress, objId));

		return objId;
	}

	UINT32 BinarySerializer::registerObjectPtr(SPtr<IReflectable> object)
	{
		if(object == nullptr)
			return 0;

		void* ptrAddress = (void*)object.get();

		auto iterFind = mObjectAddrToId.find(ptrAddress);
		if(iterFind == mObjectAddrToId.end())
		{
			UINT32 objId = findOrCreatePersistentId(object.get());

			mObjectsToEncode.push_back(ObjectToEncode(objId, object));
			mObjectAddrToId.insert(std::make_pair(ptrAddress, objId));

			return objId;
		}

		return iterFind->second;
	}
}

#undef COPY_TO_BUFFER
