//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DBinarySerializer.h"

#include "Debug/B3DDebug.h"
#include "Reflection/B3DIReflectable.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIField.h"
#include "FileSystem/B3DDataStream.h"
#include "Reflection/B3DRTTIIteratorField.h"
#include "Utility/B3DBufferedBitstream.h"

using namespace b3d;

constexpr u32 BinarySerializer::kReportAfterBytes;
constexpr u32 BinarySerializer::kWriteBufferSize;
constexpr u32 BinarySerializer::kFlushAfterBytes;
constexpr u32 BinarySerializer::kPreloadChunkBytes;

/** Contains used combinations of 00xx xxx0 bits in the field type encoding. */
enum FieldTypeBits : u8
{
	// Types (First 3 bits)
	FT_FixedSizePlainType								= 0b000'00, // Plain type with non-variable size that's <=255 bytes
	FT_DynamicSizePlainType								= 0b100'00, // Plain type with variable size, or size that's >255 bytes
	FT_DataBlockType									= 0b001'00, // Data block type
	FT_ReflectableType									= 0b101'00, // Reflectable type
	FT_ReflectablePointerType							= 0b110'00, // Reflectable pointer type

	// Masks (Last 2 bits)
	FT_ArrayMask										= 0b000'01, // Mask used for all Array entries. Cannot be combined with FT_DataBlockTypeMask.
	FT_WithAnotherTypeMask								= 0b000'10, // Mask used for all entries with WithAnotherType suffix

	// All supported values from type + mask combinations
	FT_FixedSizePlain							= FT_FixedSizePlainType,																		// 0b000'00
	FT_FixedSizePlainArray						= FT_FixedSizePlainType | FT_ArrayMask,															// 0b000'01
	FT_FixedSizePlainWithAnotherType			= FT_FixedSizePlainType | FT_WithAnotherTypeMask,												// 0b000'10
	FT_FixedSizePlainArrayWithAnotherType		= FT_FixedSizePlainType | FT_ArrayMask | FT_WithAnotherTypeMask,								// 0b000'11
	FT_DataBlock								= FT_DataBlockType,																				// 0b001'00
	FT_DataBlockWithAnotherType					= FT_DataBlockType | FT_WithAnotherTypeMask,													// 0b001'10
	FT_DynamicSizePlain							= FT_DynamicSizePlainType,																		// 0b100'00
	FT_DynamicSizePlainArray					= FT_DynamicSizePlainType | FT_ArrayMask,														// 0b100'01
	FT_DynamicSizePlainWithAnotherType			= FT_DynamicSizePlainType | FT_WithAnotherTypeMask,												// 0b100'10
	FT_DynamicSizePlainArrayWithAnotherType		= FT_DynamicSizePlainType | FT_ArrayMask | FT_WithAnotherTypeMask,								// 0b100'11
	FT_Reflectable								= FT_ReflectableType,																			// 0b101'00
	FT_ReflectableArray							= FT_ReflectableType | FT_ArrayMask,															// 0b101'01
	FT_ReflectableWithAnotherType				= FT_ReflectableType | FT_WithAnotherTypeMask,													// 0b101'10
	FT_ReflectableArrayWithAnotherType			= FT_ReflectableType | FT_ArrayMask | FT_WithAnotherTypeMask,									// 0b101'11
	FT_ReflectablePointer						= FT_ReflectablePointerType,																	// 0b110'00
	FT_ReflectablePointerArray					= FT_ReflectablePointerType | FT_ArrayMask,														// 0b110'01
	FT_ReflectablePointerWithAnotherType		= FT_ReflectablePointerType | FT_WithAnotherTypeMask,											// 0b110'10
	FT_ReflectablePointerArrayWithAnotherTyp	= FT_ReflectablePointerType | FT_ArrayMask | FT_WithAnotherTypeMask,							// 0b110'11
};

/** Decodes the FieldTypeBits enum into individual parts. */
static void DecodeFieldTypeBits(FieldTypeBits bits, RTTIFieldDataType& outType, bool& outIsArray, bool& outHasDynamicSize, bool& outHasAnotherTypeFollowing)
{
	outIsArray = false;
	outHasDynamicSize = true;
	outHasAnotherTypeFollowing = false;

	const FieldTypeBits typeBits = (FieldTypeBits)(bits & 0b111'00);
	switch(typeBits)
	{
	case FT_FixedSizePlainType:
		outHasDynamicSize = false;
		[[fallthrough]];
	case FT_DynamicSizePlainType:
		outType = RTTIFieldDataType::Plain;
		break;
	case FT_DataBlockType:
		outType = RTTIFieldDataType::DataBlock;
		break;
	case FT_ReflectableType:
		outType = RTTIFieldDataType::Reflectable;
		break;
	case FT_ReflectablePointerType:
		outType = RTTIFieldDataType::ReflectablePointer;
		break;
	default:
		B3D_ENSURE(false);
		break;
	}

	outIsArray = (bits & FT_ArrayMask) != 0;
	outHasAnotherTypeFollowing = (bits & FT_WithAnotherTypeMask) != 0;
}

/** Encodes a set of individual flags into the FieldTypeBits enum. */
static FieldTypeBits EncodeFieldTypeBits(RTTIFieldDataType type, bool isArray, bool hasDynamicSize, bool hasAnotherTypeFollowing)
{
	u32 output = 0;

	switch(type)
	{
	case RTTIFieldDataType::Plain:
		if(hasDynamicSize)
			output |= FT_DynamicSizePlainType;
		else
			output |= FT_FixedSizePlainType;
		break;
	case RTTIFieldDataType::DataBlock:
		output |= FT_DataBlockType;
		break;
	case RTTIFieldDataType::Reflectable:
		output |= FT_ReflectableType;
		break;
	case RTTIFieldDataType::ReflectablePointer:
		output |= FT_ReflectablePointerType;
		break;
	}

	if(isArray)
		output |= FT_ArrayMask;

	if(hasAnotherTypeFollowing)
		output |= FT_WithAnotherTypeMask;

	return (FieldTypeBits)output;
}

/** Encoding used for storing either fixed field size, or additional field information. */
union FieldTypeSizeOrExtendedMetaData
{
	u8 FixedSize;

	struct
	{
		u8 Unused : 4;
		u8 BuiltinTypeId : 4;
	};
};

/** Encoding used for storing information about a type in a RTTI field. */
union FieldTypeMetaData
{
	/** Creates the field meta-data from the type schema. */
	static FieldTypeMetaData Create(const RTTIFieldSchema& fieldSchema, const RTTIFieldDataTypeSchema& fieldTypeSchema, bool isLastFieldInType, bool isAnotherFieldTypeFollowing);

	/** Converts the internal data to RTTIFieldTypeSchema. */
	RTTIFieldDataTypeSchema ToFieldTypeSchema(bool& outIsArray, bool& outHasMoreTypesFollowing) const;

	u16 PackedData;
	struct
	{
		u8 IsObjectDescriptor : 1; /**< If 1, meta-data represents an object rather than the field. Rest of the data is invalid in such case. */
		FieldTypeBits FieldType : 5;
		u8 IsLastFieldInType : 1; /**< Field is the last field in the type. Only set for the first provided field type, unused in rest. */
		u8 IsExtended : 1; /**< Fields with this flag contain additional information rather than fixed size, in the AdditionalData field. */
		FieldTypeSizeOrExtendedMetaData FixedSizeOrAdditionalData; /** Contains fixed field size if IsExtended = 0 && HasDynamicSize == 0. Otherwise contains additional information about the field. */
	};
};


FieldTypeMetaData FieldTypeMetaData::Create(const RTTIFieldSchema& fieldSchema, const RTTIFieldDataTypeSchema& fieldTypeSchema, bool isLastFieldInType, bool isAnotherFieldTypeFollowing)
{
	FieldTypeMetaData output;
	output.IsObjectDescriptor = 0;

	B3D_ASSERT(fieldTypeSchema.Type != RTTIFieldDataType::DataBlock || !fieldSchema.IsContainer);
	output.FieldType = EncodeFieldTypeBits(fieldTypeSchema.Type, fieldSchema.IsContainer, fieldTypeSchema.HasDynamicSize, isAnotherFieldTypeFollowing);
	output.IsLastFieldInType = isLastFieldInType;

	const u32 fieldTypeId = fieldTypeSchema.FieldTypeId;
	const bool isBuiltin = false; // Not supported at the moment. This was never properly implemented. fieldTypeId < 16;
	if(isBuiltin)
	{
		output.IsExtended = 1;
		output.FixedSizeOrAdditionalData.Unused = 0;
		output.FixedSizeOrAdditionalData.BuiltinTypeId = fieldTypeId & 0xF;
	}
	else
	{
		output.IsExtended = 0;
		output.FixedSizeOrAdditionalData.FixedSize = fieldTypeSchema.HasDynamicSize ? 0 : fieldTypeSchema.FixedSize.Bytes;
	}

	return output;
}

RTTIFieldDataTypeSchema FieldTypeMetaData::ToFieldTypeSchema(bool& outIsArray, bool& outHasMoreTypesFollowing) const
{
	RTTIFieldDataTypeSchema schema;

	DecodeFieldTypeBits(FieldType, schema.Type, outIsArray, schema.HasDynamicSize, outHasMoreTypesFollowing);

	if(!IsExtended)
		schema.FixedSize = FixedSizeOrAdditionalData.FixedSize;
	else
		schema.FieldTypeId = FixedSizeOrAdditionalData.BuiltinTypeId;

	return schema;
}

struct ObjectMetaData
{
	u32 ObjectMeta;
	u32 TypeId;
};

/** Handles the deserialization portion of BinarySerializer. */
class BinaryDeserializationContext
{
public:
	/** Information about an object that is being deserialized. */
	struct ObjectDeserializationData
	{
		ObjectDeserializationData(SPtr<IReflectable> object = nullptr, uint64_t offset = 0, SPtr<RTTISchema> schema = nullptr)
			: Object(std::move(object)), Offset(offset), Schema(std::move(schema))
		{}

		SPtr<IReflectable> Object; /**< Instance of the object. */
		bool IsDeserialized = false; /**< True if the instance of the object has been populated with deserialized data. */
		bool DeserializationInProgress = false; /**< True while deserialization is happening for the object, used to detect circular references. */
		uint64_t Offset; /**< Location of the object in the stream. */
		SPtr<RTTISchema> Schema; /**< Schema that describes the object. */
	};

	BinaryDeserializationContext(FrameAllocator& allocator, BufferedBitstreamReader& stream, size_t dataEnd, BinarySerializerFlags flags, RTTIOperationContext& rttiContext);

	/**
	 * Deserializes a single IReflectable object. 
	 *
	 * @param	outputObjectSchema		Explicit RTTI schema used for deserializing the data. Required if data was not serialized with inline meta-data.
	 * @param	output					Previously created object to deserialize the data into. If null, the stream will be advanced until the next object, but no deserialization will happen.
	 * @return							True if deserialization succeeded, false otherwise.
	 */
	bool DeserializeReflectableObject(SPtr<RTTISchema> outputObjectSchema, const SPtr<IReflectable>& output);

	/**
	 * Creates an empty reflectable object and its deserialization data based on the provided information.
	 *
	 * @param	reflectableObjectId			ID of the object.
	 * @param	reflectableObjectTypeId		ID of the object's type.
	 * @param	locationInStream			Location of the object in the stream, in bits.
	 * @param	objectSchema				Schema describing the object's type. Can be null if schema has been encoded inline.
	 */
	void CreateReflectableObject(u32 reflectableObjectId, u32 reflectableObjectTypeId, u64 locationInStream, const SPtr<RTTISchema>& objectSchema);

	/** Finds deserialization data for the object with the provided ID. Returns null if it cannot be found. */
	ObjectDeserializationData* GetObjectDeserializationData(u32 reflectableObjectId);

	/**
	 * Decodes object meta-data from the current location in the stream. Decoding accounts for the serializer flags to decode
	 * using the correct format. Returns number of bits read.
	 */
	static u32 ReadObjectMetaData(BufferedBitstreamReader& stream, BinarySerializerFlags flags, u32& objId, u32& objTypeId, bool& isBaseType);

private:
	/** Reads the current location of the stream and returns the object ID of an object referenced by a reflectable pointer field. */
	u32 ReadReferencedReflectableObjectId() const;

	/**
	 * Checks if the instance of the reflectable object with the specified ID has been already created, and if not,
	 * creates the object and its deserialization data based on the provided schema. Provided schema must not be null.
	 */
	void EnsureReflectableObjectExists(u32 reflectableObjectId, const SPtr<RTTISchema>& objectSchema);

	/**
	 * Returns a previously deserialized instance of the reflectable object with the specified ID, or if not already deserialized,
	 * triggers deserialization before returning.
	 *
	 * @param		reflectableObjectId		ID of the object to return/deserialize.
	 * @param		objectSchema			Schema describing the object.
	 * @return								Deserialized object.
	 */
	SPtr<IReflectable> GetOrDeserializeReflectableObject(u32 reflectableObjectId, const SPtr<RTTISchema>& objectSchema);

	/** Returns a previously created instance of the reflectable object with the specified ID. */
	SPtr<IReflectable> GetReflectableObject(u32 reflectableObjectId) const;

	/**
	 * Decode meta field that was encoded using EncodeFieldMetaData(). Note that the decoded schema might not have the full field information. It's guaranteed
	 * to have the following information:
	 * - Field type
	 * - Field id
	 * - Field size (if fixed)
	 * - Field type id (if builtin type)
	 * - Is the field an array or not
	 * - Does the field have fixed or dynamic size
	 *
	 * Rest of the information can be looked up from the field ID and current RTTI type.
	 */
	static RTTIFieldSchema ReadFieldMetaData(BufferedBitstreamReader& stream, bool& terminator);

	/** Decode meta field that was encoded using encodeObjectMetaData(u32, u32, bool). */
	static void DecodeObjectMetaData(ObjectMetaData encodedData, u32& objId, u32& objTypeId, bool& isBaseClass);

	/** Decode meta field that was encoded using encodeObjectMetaData(u32, bool). */
	static void DecodeObjectMetaData(u32 encodedData, u32& objId, bool& isBaseClass);

	/** Skips the builtin type at the current location in the stream. */
	static void SkipBuiltinType(u32 fieldType, BufferedBitstreamReader& stream, bool compressed);

	/** Returns true if the data in the provided byte represents a field terminator as encoded with encodeFieldTerminator(). */
	static bool IsFieldTerminator(u8 data);

	/** Returns true if the provided encoded meta data represents object meta data. */
	static bool IsObjectMetaData(u32 encodedData);

	FrameAllocator& mAllocator;
	RTTIOperationContext& mRTTIContext;
	BufferedBitstreamReader& mStream; /**< Stream from which to read the data. */
	size_t mDataEnd = 0; /**< Byte at which the data ends. Stream will not be advanced past this point. */
	BinarySerializerFlags mFlags = BinarySerializerFlag::None; /**< Flags used to control deserialization. */

	Map<u32, ObjectDeserializationData> mReflectableObjectsToDeserialize;
};

BinaryDeserializationContext::BinaryDeserializationContext(FrameAllocator& allocator, BufferedBitstreamReader& stream, size_t dataEnd, BinarySerializerFlags flags, RTTIOperationContext& rttiContext)
	: mAllocator(allocator), mStream(stream), mDataEnd(dataEnd), mFlags(flags), mRTTIContext(rttiContext)
{ }

bool BinaryDeserializationContext::DeserializeReflectableObject(SPtr<RTTISchema> outputObjectSchema, const SPtr<IReflectable>& output)
{
	const bool hasMeta = !mFlags.IsSet(BinarySerializerFlag::NoMeta);
	const bool compressed = mFlags.IsSet(BinarySerializerFlag::Compress);

	u32 objectId = 0;
	u32 objectTypeId = 0;
	bool objectIsBaseClass = false;

	ReadObjectMetaData(mStream, mFlags, objectId, objectTypeId, objectIsBaseClass);

	if(outputObjectSchema)
		objectTypeId = outputObjectSchema->TypeId;

	if(objectIsBaseClass)
		B3D_LOG(Fatal, LogSerialization, "Encountered a base-class object while looking for a new object. Base class objects are only supposed to be parts of a larger object.");

	RTTIType* rtti = nullptr;
	if(output)
		rtti = output->GetRtti();

	FrameVector<RTTIType*> rttiInstances;

	auto finalizeObject = [&rttiInstances, this](IReflectable* object)
	{
		// Note: It would make sense to finish deserializing derived classes before base classes, but some code
		// depends on the old functionality, so we'll keep it this way
		for(auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
		{
			RTTIType* curRTTI = *iter;

			curRTTI->NotifyOperationEnded(*object, RTTIOperationType::Deserialization, mRTTIContext);
			mAllocator.Destruct(curRTTI);
		}

		rttiInstances.clear();
	};

	RTTIType* curRTTI = rtti;
	while(curRTTI)
	{
		RTTIType* rttiInstance = curRTTI->CloneInternal(mAllocator);
		rttiInstances.push_back(rttiInstance);

		curRTTI = curRTTI->GetBaseClass();
	}

	// Iterate in reverse to notify base classes before derived classes
	for(auto it = rttiInstances.rbegin(); it != rttiInstances.rend(); ++it)
	{
		(*it)->NotifyOperationStarted(*output.get(), RTTIOperationType::Deserialization, mRTTIContext);
	}

	RTTIType* rttiInstance = nullptr;
	u32 rttiInstanceIdx = 0;
	if(!rttiInstances.empty())
		rttiInstance = rttiInstances[0];

	Vector<RTTIFieldSchema>::iterator fieldSchemaIter;
	if(outputObjectSchema)
		fieldSchemaIter = outputObjectSchema->FieldSchemas.begin();

	while(mStream.Tell() < mDataEnd)
	{
		RTTIFieldSchema decodedFieldSchema;
		bool terminator = false;

		u32 baseObjId = 0;
		u32 baseObjTypeId = 0;
		bool objIsBaseClass = false;

		if(hasMeta)
		{
			u8 metaDataHeader = 0;
			mStream.ReadBytes(metaDataHeader);
			mStream.SkipBytes(-(int32_t)sizeof(metaDataHeader));

			if(IsObjectMetaData(metaDataHeader)) // We've reached a new object or a base class of the current one
			{
				u32 readBits = ReadObjectMetaData(mStream, mFlags, baseObjId, baseObjTypeId, objIsBaseClass);

				if(!objIsBaseClass)
				{
					// Found new object, we're done
					mStream.Skip(-(int64_t)readBits);

					finalizeObject(output.get());
					return true;
				}
			}
			else
			{
				if(compressed)
					terminator = IsFieldTerminator(metaDataHeader);

				if(!terminator)
					decodedFieldSchema = ReadFieldMetaData(mStream, terminator);
			}
		}
		else
		{
			if(fieldSchemaIter == outputObjectSchema->FieldSchemas.end())
			{
				// No more fields, move to base type if one exists
				if(outputObjectSchema->BaseTypeSchema)
				{
					outputObjectSchema = outputObjectSchema->BaseTypeSchema;
					fieldSchemaIter = outputObjectSchema->FieldSchemas.begin();

					baseObjTypeId = outputObjectSchema->TypeId;
					objIsBaseClass = true;
				}
				else // Otherwise we're done here
					terminator = true;
			}
			else
			{
				decodedFieldSchema = *fieldSchemaIter;

				++fieldSchemaIter;
			}
		}

		if(objIsBaseClass)
		{
			if(rtti != nullptr)
				rtti = rtti->GetBaseClass();

			// Saved and current base classes don't match, so just skip over all that data
			if(rtti == nullptr || rtti->GetRttiId() != baseObjTypeId)
				rtti = nullptr;

			rttiInstance = nullptr;

			if(rtti)
			{
				rttiInstance = rttiInstances[rttiInstanceIdx + 1];
				rttiInstanceIdx++;
			}

			continue;
		}

		if(terminator)
		{
			// We've processed the last field in this object, so return. Although we return false we don't actually know
			// if there is an object following this one. However it doesn't matter since terminator fields are only used
			// for embedded objects that are all processed within this method so we can compensate.
			finalizeObject(output.get());
			return false;
		}

		RTTIField* field = nullptr;

		if(rtti != nullptr)
			field = rtti->FindField(decodedFieldSchema.Id);

		if(field != nullptr)
		{
			if(field->Schema.IsContainer != decodedFieldSchema.IsContainer)
			{
				B3D_LOG(Error, LogSerialization, "Data type mismatch. One is array, other is a single type.");
				return false;
			}

			if(field->Schema.FieldDataTypes.Size() != decodedFieldSchema.FieldDataTypes.Size())
			{
				B3D_LOG(Error, LogSerialization, "Data type mismatch. Field type count doesn't match ({0} vs {1}). ", field->Schema.FieldDataTypes.Size(), decodedFieldSchema.FieldDataTypes.Size());
				return false;
			}

			const u32 fieldTypeCount = (u32)field->Schema.FieldDataTypes.Size();
			for(u32 fieldTypeIndex = 0; fieldTypeIndex < fieldTypeCount; ++fieldTypeIndex)
			{
				if(!decodedFieldSchema.FieldDataTypes[fieldTypeIndex].HasDynamicSize && field->Schema.FieldDataTypes[fieldTypeIndex].FixedSize != decodedFieldSchema.FieldDataTypes[fieldTypeIndex].FixedSize)
				{
					B3D_LOG(Error, LogSerialization, "Data type mismatch. Type size stored in file and actual type size don't match ({0} vs {1}).", field->Schema.FieldDataTypes[fieldTypeIndex].FixedSize.Bytes, decodedFieldSchema.FieldDataTypes[fieldTypeIndex].FixedSize.Bytes);
					return false;
				}

				if(field->Schema.FieldDataTypes[fieldTypeIndex].Type != decodedFieldSchema.FieldDataTypes[fieldTypeIndex].Type)
				{
					B3D_LOG(Error, LogSerialization, "Data type mismatch. Type stored in file and actual type don't match ({0} vs {1}).", (u32)field->Schema.FieldDataTypes[fieldTypeIndex].Type, (u32)decodedFieldSchema.FieldDataTypes[fieldTypeIndex].Type);
					return false;
				}
			}
		}

		switch(decodedFieldSchema.FieldType)
		{
		case RTTIFieldType::Iterable:
			{
				u32 elementCount = 1;
				if(decodedFieldSchema.IsContainer)
				{
					if(compressed)
						mStream.ReadVarInt(elementCount);
					else
						mStream.ReadBytes(elementCount);
				}

				RTTIIteratorField* iteratorField = nullptr;

				if(field != nullptr)
					iteratorField = static_cast<RTTIIteratorField*>(field);

				SPtr<IRTTIIterator> iterator;

				if(iteratorField != nullptr)
				{
					iterator = iteratorField->GetIterator(rttiInstance, output.get(), mAllocator);

					if(iterator != nullptr)
						iterator->Clear();
				}

				for(u32 elementIndex = 0; elementIndex < elementCount; ++elementIndex)
				{
					void* fieldValue = nullptr;
					if(iterator != nullptr)
						fieldValue = iteratorField->CreateEmptyFieldValue(mAllocator);

					for(u32 typeIndex = 0; typeIndex < (u32)decodedFieldSchema.FieldDataTypes.Size(); ++typeIndex)
					{
						const RTTIFieldDataTypeSchema& decodedFieldTypeSchema = decodedFieldSchema.FieldDataTypes[typeIndex];

						switch(decodedFieldTypeSchema.Type)
						{
						case RTTIFieldDataType::ReflectablePointer:
							{
								const u32 referencedObjectId = ReadReferencedReflectableObjectId();

								// If reading from schema we need to create object here as we don't know its type during the normal pass
								if(outputObjectSchema != nullptr)
									EnsureReflectableObjectExists(referencedObjectId, decodedFieldTypeSchema.FieldTypeSchema);

								if(iteratorField != nullptr)
								{
									SPtr<IReflectable> referencedObject;
									if(iteratorField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
										referencedObject = GetReflectableObject(referencedObjectId);
									else
										referencedObject = GetOrDeserializeReflectableObject(referencedObjectId, decodedFieldTypeSchema.FieldTypeSchema);

									iteratorField->SetReflectablePointer(fieldValue, typeIndex, referencedObject);
								}
								break;
							}
						case RTTIFieldDataType::Reflectable:
							{
								SPtr<IReflectable> referencedObject;
								if(iteratorField != nullptr)
									referencedObject = IReflectable::CreateInstanceFromTypeId(iteratorField->Schema.FieldDataTypes[typeIndex].FieldTypeId);

								DeserializeReflectableObject(decodedFieldTypeSchema.FieldTypeSchema, referencedObject);

								if(iteratorField != nullptr)
								{
									// Note: Would be nice to avoid this copy by value and decode directly into the field
									iteratorField->SetReflectable(fieldValue, typeIndex, *referencedObject);
								}

								break;
							}
						case RTTIFieldDataType::Plain:
							{
								uint64_t typeSizeBits = decodedFieldTypeSchema.FixedSize.GetBits();
								if(decodedFieldTypeSchema.HasDynamicSize)
								{
									if(compressed)
									{
										BitLength typeSize;
										BitLength headerSize = B3DRTTIReadSizeHeader(mStream, true, typeSize);
										mStream.Skip(-(int64_t)headerSize.GetBits());

										typeSizeBits = typeSize.GetBits();
									}
									else
									{
										uint32_t typeSize;
										mStream.ReadBytes(typeSize);
										mStream.SkipBytes(-(int32_t)sizeof(uint32_t));

										typeSizeBits = (uint64_t)typeSize * 8;
									}
								}

								if(iteratorField != nullptr)
								{
									mStream.Preload((uint32_t)Math::DivideAndRoundUp(typeSizeBits, (uint64_t)8));
									iteratorField->ReadPlainTypeTupleFromStream(fieldValue, typeIndex, mStream.GetBitstream(), compressed);

									mStream.Skip(typeSizeBits);
								}
								else
								{
									bool builtin = decodedFieldTypeSchema.FieldTypeId < 16;
									if(compressed && builtin)
										SkipBuiltinType(decodedFieldTypeSchema.FieldTypeId, mStream, compressed);
									else
										mStream.Skip(typeSizeBits);
								}
								break;
							}
						default:
							B3D_LOG(Fatal, LogSerialization, "Error decoding data. Encountered a type I don't know how to decode. Type: {0}, Is array: {1}", (u32)decodedFieldTypeSchema.Type, decodedFieldSchema.IsContainer);
						}
					}

					if(iterator != nullptr)
					{
						iterator->SeekToEnd(); // Ensures value is inserted at the end of the iterable container

						iteratorField->SetIteratorValue(rttiInstance, output.get(), mAllocator, *iterator, fieldValue);
						iteratorField->FreeFieldValue(fieldValue, mAllocator);
					}
				}
			}
			break;
		case RTTIFieldType::DataBlock:
			{
				auto* curField = static_cast<RTTIDataBlockFieldBase*>(field);

				// Data block size
				u32 dataBlockSize = 0;
				if(compressed)
					mStream.ReadVarInt(dataBlockSize);
				else
					mStream.ReadBytes(dataBlockSize);

				mStream.Align();

				// Data block data
				if(curField != nullptr)
				{
					const SPtr<DataStream>& dataStream = mStream.GetDataStream();
					if(dataStream->IsFile()) // Allow streaming
					{
						uint64_t curOffset = mStream.Tell();

						// Data blocks don't support data that isn't byte aligned, but encoder should take care of that
						B3D_ASSERT((curOffset % 8) == 0);
						curOffset /= 8;

						dataStream->Seek(curOffset);
						curField->SetValue(rttiInstance, output.get(), dataStream, dataBlockSize);

						mStream.SkipBytes(dataBlockSize);
					}
					else
					{
						SPtr<MemoryDataStream> dataBlockStream = B3DMakeShared<MemoryDataStream>(dataBlockSize);
						mStream.ReadBytes(dataBlockStream->Data(), dataBlockSize);

						curField->SetValue(rttiInstance, output.get(), dataBlockStream, dataBlockSize);
					}
				}
				else
					mStream.SkipBytes(dataBlockSize);

			}
			break;
		default:
			B3D_ENSURE(false);
			break;
		}

		mStream.ClearBuffered(false);
	}

	finalizeObject(output.get());
	return false;
}

u32 BinaryDeserializationContext::ReadReferencedReflectableObjectId() const
{
	const bool isCompressed = mFlags.IsSet(BinarySerializerFlag::Compress);

	u32 childObjectId = 0;
	if(isCompressed)
		mStream.ReadVarInt(childObjectId);
	else
		mStream.ReadBytes(childObjectId);
	
	return childObjectId;
}

void BinaryDeserializationContext::EnsureReflectableObjectExists(u32 reflectableObjectId, const SPtr<RTTISchema>& objectSchema)
{
	auto foundExisting = mReflectableObjectsToDeserialize.find(reflectableObjectId);

	if(foundExisting != mReflectableObjectsToDeserialize.end())
		return;

	SPtr<IReflectable> object = IReflectable::CreateInstanceFromTypeId(objectSchema->TypeId);
	mReflectableObjectsToDeserialize.insert(std::make_pair(reflectableObjectId, ObjectDeserializationData(object, ~0u, objectSchema)));
}

SPtr<IReflectable> BinaryDeserializationContext::GetOrDeserializeReflectableObject(u32 reflectableObjectId, const SPtr<RTTISchema>& objectSchema)
{
	auto foundExisting = mReflectableObjectsToDeserialize.find(reflectableObjectId);
	if(foundExisting == mReflectableObjectsToDeserialize.end())
	{
		if(reflectableObjectId != 0)
			B3D_LOG(Warning, LogGeneric, "When deserializing, object ID: {0} was found but no such object was contained in the file.", reflectableObjectId);

		return nullptr;
	}

	ObjectDeserializationData& objectToDeserialize = foundExisting->second;
	if(!objectToDeserialize.IsDeserialized)
	{
		if(objectToDeserialize.DeserializationInProgress)
		{
			B3D_LOG(Warning, LogGeneric, "Detected a circular reference when decoding. Referenced "
									 "object's fields will be resolved in an undefined order (i.e. one of the "
									 "objects will not be fully deserialized when assigned to its field). "
									 "Use RTTI_Flag_WeakRef to get rid of this warning and tell the system which of"
									 "the objects is allowed to be deserialized after it is assigned to its field.");
		}
		else
		{
			objectToDeserialize.DeserializationInProgress = true;

			const uint64_t curOffset = mStream.Tell();
			mStream.Seek(objectToDeserialize.Offset);
			DeserializeReflectableObject(objectSchema, objectToDeserialize.Object);
			mStream.Seek(curOffset);

			objectToDeserialize.DeserializationInProgress = false;
			objectToDeserialize.IsDeserialized = true;
		}
	}

	return objectToDeserialize.Object;
}

BinaryDeserializationContext::ObjectDeserializationData* BinaryDeserializationContext::GetObjectDeserializationData(u32 reflectableObjectId)
{
	auto found = mReflectableObjectsToDeserialize.find(reflectableObjectId);
	if(found == mReflectableObjectsToDeserialize.end())
		return nullptr;

	return &found->second;
}

SPtr<IReflectable> BinaryDeserializationContext::GetReflectableObject(u32 reflectableObjectId) const
{
	auto foundExisting = mReflectableObjectsToDeserialize.find(reflectableObjectId);
	if(foundExisting == mReflectableObjectsToDeserialize.end())
	{
		if(reflectableObjectId != 0)
			B3D_LOG(Warning, LogGeneric, "When deserializing, object ID: {0} was found but no such object was contained in the file.", reflectableObjectId);

		return nullptr;
	}
	
	return foundExisting->second.Object;
}

void BinaryDeserializationContext::CreateReflectableObject(u32 reflectableObjectId, u32 reflectableObjectTypeId, u64 locationInStream, const SPtr<RTTISchema>& objectSchema)
{
	const SPtr<IReflectable>& object = IReflectable::CreateInstanceFromTypeId(reflectableObjectTypeId);
	mReflectableObjectsToDeserialize.insert(std::make_pair(reflectableObjectId, ObjectDeserializationData(object, locationInStream, objectSchema)));
}

RTTIFieldSchema BinaryDeserializationContext::ReadFieldMetaData(BufferedBitstreamReader& stream, bool& terminator)
{
	u32 fieldMetaData;
	stream.ReadBytes(fieldMetaData);

	if(IsObjectMetaData(fieldMetaData))
		B3D_LOG(Fatal, LogSerialization, "Meta data represents an object description but is trying to be decoded as a field descriptor.");

	RTTIFieldSchema fieldSchema;
	fieldSchema.Id = (u16)((fieldMetaData >> 16) & 0xFFFF);

	FieldTypeMetaData firstFieldTypeMetaData;
	firstFieldTypeMetaData.PackedData = (u16)(fieldMetaData & 0xFFFF);

	bool hasMoreFieldTypes;
	const RTTIFieldDataTypeSchema firstFieldTypeSchema = firstFieldTypeMetaData.ToFieldTypeSchema(fieldSchema.IsContainer, hasMoreFieldTypes);

	terminator = firstFieldTypeMetaData.IsLastFieldInType;
	fieldSchema.FieldDataTypes.Add(firstFieldTypeSchema);

	fieldSchema.FieldType = firstFieldTypeSchema.Type == RTTIFieldDataType::DataBlock ? RTTIFieldType::DataBlock : RTTIFieldType::Iterable;

	while(hasMoreFieldTypes)
	{
		u16 fieldTypeMetaData;
		stream.ReadBytes(fieldTypeMetaData);

		FieldTypeMetaData additionalFieldTypeMetaData;
		additionalFieldTypeMetaData.PackedData = fieldTypeMetaData;

		bool isArray;
		const RTTIFieldDataTypeSchema additionalFieldTypeSchema = additionalFieldTypeMetaData.ToFieldTypeSchema(isArray, hasMoreFieldTypes);
		fieldSchema.FieldDataTypes.Add(additionalFieldTypeSchema);
	}

	return fieldSchema;
}

void BinaryDeserializationContext::SkipBuiltinType(u32 fieldType, BufferedBitstreamReader& stream, bool compressed)
{
	switch(fieldType)
	{
	case TID_Bool:
		{
			bool data;
			stream.Preload(sizeof(data));
			RTTIPlainType<bool>::FromMemory(data, stream.GetBitstream(), RTTIFieldInfo(), compressed);
			break;
		}
	case TID_Int32:
		{
			int32_t data;
			stream.Preload(sizeof(data));
			RTTIPlainType<int32_t>::FromMemory(data, stream.GetBitstream(), RTTIFieldInfo(), compressed);
			break;
		}
	case TID_UInt32:
		{
			uint32_t data;
			stream.Preload(sizeof(data));
			RTTIPlainType<uint32_t>::FromMemory(data, stream.GetBitstream(), RTTIFieldInfo(), compressed);
			break;
		}
	default:
		B3D_ASSERT(false);
		break;
	}
}

bool BinaryDeserializationContext::IsFieldTerminator(u8 data)
{
	if(IsObjectMetaData(data))
		B3D_LOG(Fatal, LogSerialization, "Meta data represents an object description but is trying to be decoded as a field descriptor.");

	return (data & 0x40) != 0;
}

void BinaryDeserializationContext::DecodeObjectMetaData(ObjectMetaData encodedData, u32& objId, u32& objTypeId, bool& isBaseClass)
{
	if(!IsObjectMetaData(encodedData.ObjectMeta))
		B3D_LOG(Fatal, LogSerialization, "Meta data represents a field description but is trying to be decoded as an object descriptor.");

	DecodeObjectMetaData(encodedData.ObjectMeta, objId, isBaseClass);
	objTypeId = encodedData.TypeId;
}

void BinaryDeserializationContext::DecodeObjectMetaData(u32 encodedData, u32& objId, bool& isBaseClass)
{
	if(!IsObjectMetaData(encodedData))
		B3D_LOG(Fatal, LogSerialization, "Meta data represents a field description but is trying to be decoded as an object descriptor.");

	objId = (encodedData >> 2) & 0x3FFFFFFF;
	isBaseClass = (encodedData & 0x02) != 0;
}

bool BinaryDeserializationContext::IsObjectMetaData(u32 encodedData)
{
	return ((encodedData & 0x01) != 0);
}

u32 BinaryDeserializationContext::ReadObjectMetaData(BufferedBitstreamReader& stream, BinarySerializerFlags flags, uint32_t& objId, uint32_t& objTypeId, bool& isBaseType)
{
	if(!flags.IsSet(BinarySerializerFlag::NoMeta))
	{
		ObjectMetaData objectMetaData;
		objectMetaData.ObjectMeta = 0;
		objectMetaData.TypeId = 0;

		if(stream.ReadBytes(objectMetaData) != sizeof(ObjectMetaData))
			B3D_LOG(Fatal, LogSerialization, "Error decoding data.");

		DecodeObjectMetaData(objectMetaData, objId, objTypeId, isBaseType);
		return sizeof(ObjectMetaData) * 8;
	}
	else
	{
		u32 objectMetaData = 0;

		u32 bitsRead = 0;
		if(flags.IsSet(BinarySerializerFlag::Compress))
			bitsRead = stream.ReadVarInt(objectMetaData);
		else
		{
			if(stream.ReadBytes(objectMetaData) != sizeof(objectMetaData))
				B3D_LOG(Fatal, LogSerialization, "Error decoding data.");

			bitsRead = sizeof(objectMetaData) * 8;
		}

		DecodeObjectMetaData(objectMetaData, objId, isBaseType);
		objTypeId = 0; // Unavailable, needs to be read from a schema

		return bitsRead;
	}
}

/** Handles the serialization portion of BinarySerializer. */
class BinarySerializationContext
{
public:
	/** Information about an object that is being serialized. */
	struct ObjectToSerialize
	{
		ObjectToSerialize(u32 objectId, SPtr<IReflectable> object)
			: ObjectId(objectId), Object(std::move(object))
		{}

		u32 ObjectId;
		SPtr<IReflectable> Object;
	};

	BinarySerializationContext(FrameAllocator& allocator, BufferedBitstreamWriter& stream, BinarySerializerFlags flags, RTTIOperationContext& rttiContext);

	/**
	 * Serializes a single IReflectable object. Any pointers referencing other reflectable types will be registered in mObjectsToSerialize, and
	 * this method should also be called over all objects registered in that array.
	 *
	 * @param	object								Object to serialize.
	 * @param	objectId							Persistent ID of the object in the serialized data. See FindOrCreateReflectableObjectId().
	 * @param	outReferencedObjectsToSerialize		A list of all reflectable object pointers in the provided object, that haven't yet been serialized. You should call this method on them as well, so
	 *												references may be restored.
	 * @return										True if successful, false otherwise.
	 */
	bool SerializeReflectableObject(IReflectable* object, u32 objectId, Vector<ObjectToSerialize>& outReferencedObjectsToSerialize);

	/**	Finds an existing, or creates a unique unique ID for the specified object. See RegisterChildObjectForSerialization. */
	u32 FindOrCreateReflectableObjectId(IReflectable* object);
private:
	/**
	 * Serializes an IReflectable inline as a field value. This is opposed to serializing a reflectable by pointer, which
	 * are stored later in the serialized data, and the field value only stores the object ID.
	 */
	bool SerializeReflectableObjectInline(IReflectable* object, Vector<ObjectToSerialize>& outReferencedObjectsToSerialize);

	/**
	 * Adds the object to the list of objects that still need to be serialized. Assigns the object a unique ID
	 * or returns a previously assigned one, if the object was already registered. This ID will be stored when the object
	 * is serialized, and may be used for referencing the object in the serialized data.
	 */
	u32 RegisterReflectableObjectForSerialization(SPtr<IReflectable> object, Vector<ObjectToSerialize>& outReferencedObjectsToSerialize);

	/** Encodes and writes data required for representing a serialized field, into the provided stream. */
	static void WriteFieldMetaData(const RTTIFieldSchema& fieldSchema, bool isLastFieldInType, BufferedBitstreamWriter& stream);

	/** Encodes data representing a field terminator into 1 byte. */
	static u8 EncodeFieldTerminator();

	/**
	 * Encodes an object identifier, its type and other meta-data into 8 bytes.
	 *
	 * @param[in]	objId	   	Unique ID of the object instance. This can be a maximum of 30 bits, as two bits are reserved.
	 * @param[in]	objTypeId  	Unique ID of the object type.
	 * @param[in]	isBaseClass	True if this object is base class (that is, just a part of a larger object).
	 * @return		Encoded object id, type ID and other meta-data.
	 */
	static ObjectMetaData EncodeObjectMetaData(u32 objId, u32 objTypeId, bool isBaseClass);

	/**
	 * Encodes an object identifier and meta-data into 4 bytes.
	 *
	 * @param[in]	objId	   	Unique ID of the object instance. This can be a maximum of 30 bits, as two bits are reserved.
	 * @param[in]	isBaseClass	true if this object is base class (that is, just a part of a larger object).
	 * @return		Encoded object id and other meta-data.
	 */
	static u32 EncodeObjectMetaData(u32 objId, bool isBaseClass);

	FrameAllocator& mAllocator;
	BufferedBitstreamWriter mStream;
	BinarySerializerFlags mFlags;
	RTTIOperationContext& mRTTIContext;
	UnorderedMap<void*, u32> mReflectableObjectToID;
	u32 mLastUsedObjectId = 1;
};

bool BinarySerializationContext::SerializeReflectableObject(IReflectable* object, u32 objectId, Vector<ObjectToSerialize>& outReferencedObjectsToSerialize)
{
	const bool writeMeta = !mFlags.IsSet(BinarySerializerFlag::NoMeta);
	const bool compress = mFlags.IsSet(BinarySerializerFlag::Compress);

	RTTIType* rtti = object->GetRtti();
	bool isBaseClass = false;

	FrameStack<RTTIType*> rttiInstances;

	const auto cleanup = [&]() // TODO - Use scope guard
	{
		while(!rttiInstances.empty())
		{
			RTTIType* rttiInstance = rttiInstances.top();
			rttiInstance->NotifyOperationEnded(*object, RTTIOperationType::Serialization, mRTTIContext);
			mAllocator.Destruct(rttiInstance);

			rttiInstances.pop();
		}
	};

	// If an object has base classes, we need to iterate through all of them
	do
	{
		RTTIType* rttiInstance = rtti->CloneInternal(mAllocator);
		rttiInstances.push(rttiInstance);

		rttiInstance->NotifyOperationStarted(*object, RTTIOperationType::Serialization, mRTTIContext);

		if(writeMeta)
		{
			// Encode object ID & type
			ObjectMetaData objectMetaData = EncodeObjectMetaData(objectId, rtti->GetRttiId(), isBaseClass);
			mStream.WriteBytes(objectMetaData);
		}
		else
		{
			// Encode object ID
			u32 objectMetaData = EncodeObjectMetaData(objectId, isBaseClass);

			if(compress)
				mStream.WriteVarInt(objectMetaData);
			else
				mStream.WriteBytes(objectMetaData);
		}

		const u32 fieldCount = rtti->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			RTTIField* const field = rtti->GetField(fieldIndex);

			if(writeMeta)
			{
				// Copy field ID & other meta-data like field size and type
				WriteFieldMetaData(field->Schema, false, mStream);
			}

			switch(field->Schema.FieldType)
			{
				case RTTIFieldType::Iterable:
				{
					RTTIIteratorField* const iteratorField = static_cast<RTTIIteratorField*>(field);
					SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object, mAllocator);

					if(iteratorField->Schema.IsContainer)
					{
						u32 elementCount = 0;
						if(iterator != nullptr)
							elementCount = (u32)iterator->GetElementCount();

						// Copy num vector elements
						if(compress)
							mStream.WriteVarInt(elementCount);
						else
							mStream.WriteBytes(elementCount);
					}

					if(iterator != nullptr)
					{
						for(; iterator->IsValid(); iterator->Increment())
						{
							const void* fieldValue = iteratorField->GetIteratorValue(rttiInstance, object, mAllocator, *iterator);
							for(u32 typeIndex = 0; typeIndex < (u32)iteratorField->Schema.FieldDataTypes.Size(); ++typeIndex)
							{
								const RTTIFieldDataTypeSchema& typeSchema = iteratorField->Schema.FieldDataTypes[typeIndex];
								switch(typeSchema.Type)
								{
								case RTTIFieldDataType::ReflectablePointer:
									{
										SPtr<IReflectable> childObject;

										if(!mFlags.IsSet(BinarySerializerFlag::Shallow))
											childObject = iteratorField->GetReflectablePointer(fieldValue, typeIndex);

										const u32 objectId = RegisterReflectableObjectForSerialization(childObject, outReferencedObjectsToSerialize);
										if(compress)
											mStream.WriteVarInt(objectId);
										else
											mStream.WriteBytes(objectId);

										break;
									}
								case RTTIFieldDataType::Reflectable:
									{
										const IReflectable& childObject = iteratorField->GetReflectable(fieldValue, typeIndex);
										if(!SerializeReflectableObjectInline(const_cast<IReflectable*>(&childObject), outReferencedObjectsToSerialize)) // TODO - Get rid of const cast
										{
											cleanup();
											return false;
										}

										break;
									}
								case RTTIFieldDataType::Plain:
									{
										iteratorField->WritePlainTypeTupleToStream(fieldValue, typeIndex, mStream.GetBitstream(), compress);
										break;
									}
								default:
									B3D_LOG(Error, LogSerialization, "Error serializing data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}", (u32)typeSchema.Type, iteratorField->Schema.IsContainer);
								}
							}
						}
					}
				}
				break;
				case RTTIFieldType::DataBlock:
				{
					auto* curField = static_cast<RTTIDataBlockFieldBase*>(field);

					u32 dataBlockSize = 0;
					SPtr<DataStream> blockStream = curField->GetValue(rttiInstance, object, dataBlockSize);

					const u64 originalBlockStreamLocation = blockStream->Tell();

					// Data block size
					if(compress)
						mStream.WriteVarInt(dataBlockSize);
					else
						mStream.WriteBytes(dataBlockSize);

					// Data block data
					auto dataToStore = (u8*)B3DStackAllocate(dataBlockSize);
					blockStream->Read(dataToStore, dataBlockSize);

					mStream.Align();
					mStream.WriteBytes(dataToStore, dataBlockSize);
					B3DStackFree(dataToStore);

					blockStream->Seek(originalBlockStreamLocation);
				}
				break;
			default:
				B3D_ENSURE(false);
				break;
			}

			mStream.Flush(false);
		}

		rtti = rtti->GetBaseClass();
		isBaseClass = true;
	}
	while(rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

	cleanup();

	return true;
}

u8 BinarySerializationContext::EncodeFieldTerminator()
{
	// See the documentation for WriteFieldMetaData() on why we're using this format
	return 0x40;
}

bool BinarySerializationContext::SerializeReflectableObjectInline(IReflectable* object, Vector<ObjectToSerialize>& outReferencedObjectsToSerialize)
{
	if(object != nullptr)
	{
		if(!SerializeReflectableObject(object, 0, outReferencedObjectsToSerialize))
			return false;

		if(!mFlags.IsSet(BinarySerializerFlag::NoMeta))
		{
			// Encode terminator field
			// Complex types require terminator fields because they can be embedded within other complex types and we need
			// to know when their fields end and parent's resume
			if(mFlags.IsSet(BinarySerializerFlag::Compress))
			{
				u8 metaData = EncodeFieldTerminator();
				mStream.WriteBytes(metaData);
			}
			else
				WriteFieldMetaData(RTTIFieldSchema(), true, mStream);
		}
	}

	return true;
}

void BinarySerializationContext::WriteFieldMetaData(const RTTIFieldSchema& fieldSchema, bool isLastFieldInType, BufferedBitstreamWriter& stream)
{
	// If O == 0 - Meta contains field information (Encoded using this method)
	//// Encoding if E = 0: IIII IIII IIII IIII SSSS SSSS ETFF FFFO
	//// Encoding if E = 1: IIII IIII IIII IIII BBBB xxxx ETFF FFFO
	//// I - Id
	//// S - Size
	//// F - FieldTypeBits enum
	//// O - Object descriptor
	//// T - Terminator (last field in an inline object)
	//// E - Extended (size is replaced with additional meta-data)
	//// B - Built-in type ID

	FieldTypeMetaData firstFieldTypeMetaData;

	if(isLastFieldInType)
	{
		B3D_ENSURE(fieldSchema.FieldDataTypes.Empty());
		firstFieldTypeMetaData = FieldTypeMetaData::Create(fieldSchema, RTTIFieldDataTypeSchema(false, 0, RTTIFieldDataType::Plain, 0, nullptr), true, false);
	}
	else
	{
		if(!B3D_ENSURE(!fieldSchema.FieldDataTypes.Empty()))
			return;

		firstFieldTypeMetaData = FieldTypeMetaData::Create(fieldSchema, fieldSchema.FieldDataTypes[0], isLastFieldInType, fieldSchema.FieldDataTypes.Size() > 1);
	}

	// Encodes field ID and meta-data for the first type
	const u32 fieldMetaData = fieldSchema.Id << 16 | (u32)firstFieldTypeMetaData.PackedData;
	stream.WriteBytes(fieldMetaData);

	const u32 fieldTypeCount = (u32)fieldSchema.FieldDataTypes.Size();
	for(u32 fieldTypeIndex = 1; fieldTypeIndex < fieldTypeCount; fieldTypeIndex++)
	{
		const bool isLastFieldType = (fieldTypeIndex + 1) == fieldTypeCount;
		FieldTypeMetaData additionalFieldTypeMetaData = FieldTypeMetaData::Create(fieldSchema, fieldSchema.FieldDataTypes[fieldTypeIndex], false, !isLastFieldType);

		stream.WriteBytes(additionalFieldTypeMetaData.PackedData);
	}
}

ObjectMetaData BinarySerializationContext::EncodeObjectMetaData(u32 objId, u32 objTypeId, bool isBaseClass)
{
	// If O == 1 - Meta contains object instance information (Encoded using EncodeObjectMetaData)
	//// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
	//// S - Size of the object identifier
	//// O - Object descriptor
	//// B - Base class indicator

	if(objId > 1073741823)
		B3D_LOG(Fatal, LogSerialization, "Object ID is larger than we can store (max 30 bits): {0}", objId);

	ObjectMetaData metaData;
	metaData.ObjectMeta = EncodeObjectMetaData(objId, isBaseClass);
	metaData.TypeId = objTypeId;
	return metaData;
}

u32 BinarySerializationContext::EncodeObjectMetaData(u32 objId, bool isBaseClass)
{
	// If O == 1 - Meta contains object instance information (Encoded using encodeObjectMetaData)
	//// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
	//// S - Size of the object identifier
	//// O - Object descriptor
	//// B - Base class indicator

	if(objId > 1073741823)
		B3D_LOG(Fatal, LogSerialization, "Object ID is larger than we can store (max 30 bits): {0}", objId);

	return (objId << 2) | (isBaseClass ? 0x02 : 0) | 0x01;
}

u32 BinarySerializationContext::FindOrCreateReflectableObjectId(IReflectable* object)
{
	void* const objectMemoryAddress = object;

	auto found = mReflectableObjectToID.find(objectMemoryAddress);
	if(found != mReflectableObjectToID.end())
		return found->second;

	u32 objId = mLastUsedObjectId++;
	mReflectableObjectToID.insert(std::make_pair(objectMemoryAddress, objId));

	return objId;
}

u32 BinarySerializationContext::RegisterReflectableObjectForSerialization(SPtr<IReflectable> object, Vector<ObjectToSerialize>& outReferencedObjectsToSerialize)
{
	if(object == nullptr)
		return 0;

	void* const objectMemoryAddress = object.get();

	auto found = mReflectableObjectToID.find(objectMemoryAddress);
	if(found == mReflectableObjectToID.end())
	{
		const u32 objectId = FindOrCreateReflectableObjectId(object.get());

		outReferencedObjectsToSerialize.push_back(ObjectToSerialize(objectId, object));
		mReflectableObjectToID.insert(std::make_pair(objectMemoryAddress, objectId));

		return objectId;
	}

	return found->second;
}


BinarySerializationContext::BinarySerializationContext(FrameAllocator& allocator, BufferedBitstreamWriter& stream, BinarySerializerFlags flags, RTTIOperationContext& rttiContext)
	: mAllocator(allocator), mStream(stream), mFlags(flags), mRTTIContext(rttiContext)
{ }

BinarySerializer::BinarySerializer()
	: mAlloc(&GetFrameAllocator())
{}

void BinarySerializer::Encode(IReflectable* object, const SPtr<DataStream>& stream, RTTIOperationContext& context, BinarySerializerFlags flags)
{
	mContext = &context;
	mBuffer.Seek(0);

	mAlloc->MarkFrame();

	BufferedBitstreamWriter bufferedStream(&mBuffer, stream, kWriteBufferSize, kFlushAfterBytes);
	BinarySerializationContext serializationContext(*mAlloc, bufferedStream, flags, context);

	Vector<SPtr<IReflectable>> encodedObjects;
	u32 objectId = serializationContext.FindOrCreateReflectableObjectId(object);

	Vector<BinarySerializationContext::ObjectToSerialize> referencedObjectsToSerialize;

	// Encode primary object and its value types
	if(!serializationContext.SerializeReflectableObject(object, objectId, referencedObjectsToSerialize))
	{
		B3D_LOG(Error, LogSerialization, "Destination buffer is null or not large enough.");
		return;
	}

	// Encode pointed to objects and their value types
	UnorderedSet<u32> serializedObjects;
	while(true)
	{
		auto it = referencedObjectsToSerialize.begin();
		bool foundObjectToProcess = false;
		for(; it != referencedObjectsToSerialize.end(); ++it)
		{
			auto found = serializedObjects.find(it->ObjectId);
			if(found != serializedObjects.end())
				continue; // Already processed

			SPtr<IReflectable> curObject = it->Object;
			u32 curObjectid = it->ObjectId;
			serializedObjects.insert(curObjectid);
			referencedObjectsToSerialize.erase(it); // TODO - Should update it to returned value from erase()

			if(!serializationContext.SerializeReflectableObject(curObject.get(), curObjectid, referencedObjectsToSerialize))
			{
				B3D_LOG(Error, LogSerialization, "Destination buffer is null or not large enough.");
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

	encodedObjects.clear();

	mAlloc->Clear();
}

void BinarySerializer::Encode(IReflectable* object, const SPtr<DataStream>& stream, BinarySerializerFlags flags)
{
	RTTIOperationContext rttiOperationContext;
	return Encode(object, stream, rttiOperationContext, flags);
}

SPtr<IReflectable> BinarySerializer::Decode(const SPtr<DataStream>& stream, u32 dataLength, RTTIOperationContext& context, BinarySerializerFlags flags, std::function<void(float)> progress, SPtr<RTTISchema> schema)
{
	mContext = &context;
	mReportProgress = nullptr;
	mTotalBytesToRead = dataLength;
	mBuffer.Seek(0);

	if(dataLength == 0)
	{
		if(mReportProgress)
			mReportProgress(1.0f);

		return nullptr;
	}

	const size_t start = stream->Tell();
	const size_t end = start + dataLength;
	const size_t endBits = end * 8;

	bool hasMeta = !flags.IsSet(BinarySerializerFlag::NoMeta);
	bool compress = flags.IsSet(BinarySerializerFlag::Compress);

	// Don't need a schema if we have meta-data
	if(hasMeta)
		schema = nullptr;
	else
	{
		if(!schema)
		{
			B3D_LOG(Error, LogSerialization, "Cannot decode an object without meta-data nor schema.");
			return nullptr;
		}
	}

	BufferedBitstreamReader bufferedStream(&mBuffer, stream, kPreloadChunkBytes, kFlushAfterBytes);
	BinaryDeserializationContext deserializationContext(*mAlloc, bufferedStream, endBits, flags, context);

	auto fnReportProgress = [this, &bufferedStream]()
	{
		const u32 bytesRead = (u32)Math::DivideAndRoundUp(bufferedStream.Tell(), (uint64_t)8);
		if(mReportProgress && (bytesRead >= mNextProgressReport))
		{
			u32 lastReport = (bytesRead / kReportAfterBytes) * kReportAfterBytes;
			mNextProgressReport = lastReport + kReportAfterBytes;

			mReportProgress(bytesRead / (float)mTotalBytesToRead);
		}

		return true;
	};

	// Note: Ideally we can avoid iterating twice over the stream data
	// We need to find offsets at which all objects start at so we can map object id to offset
	u32 rootObjectId = (u32)-1;
	SPtr<RTTISchema> curSchema = schema;
	do
	{
		bool isRoot = rootObjectId == (u32)-1;

		u32 objectId = 0;
		u32 objectTypeId = 0;
		bool objectIsBaseClass = false;

		u32 bitsRead = deserializationContext.ReadObjectMetaData(bufferedStream, flags, objectId, objectTypeId, objectIsBaseClass);
		bufferedStream.Skip(-(int64_t)bitsRead);

		if(objectIsBaseClass)
		{
			B3D_LOG(Fatal, LogSerialization, "Encountered a base-class object while looking for a new object. "
				"Base class objects are only supposed to be parts of a larger object.");
		}

		if(curSchema)
			objectTypeId = curSchema->TypeId;

		if(isRoot)
			deserializationContext.CreateReflectableObject(objectId, objectTypeId, bufferedStream.Tell(), curSchema);
		else
		{
			if(hasMeta)
				deserializationContext.CreateReflectableObject(objectId, objectTypeId, bufferedStream.Tell(), curSchema);
			else
			{
				// If no meta, it's expected the pass over the root object has populated mDecodeObjectMap with object instances as well as references to the schema
				BinaryDeserializationContext::ObjectDeserializationData* const objectToDeserialize = deserializationContext.GetObjectDeserializationData(objectId);
				B3D_ASSERT(objectToDeserialize != nullptr);

				objectToDeserialize->Offset = bufferedStream.Tell();

				curSchema = objectToDeserialize->Schema;
				objectTypeId = curSchema->TypeId;
			}
		}

		if(isRoot)
			rootObjectId = objectId;
	}
	while(deserializationContext.DeserializeReflectableObject(curSchema, nullptr) && fnReportProgress());

	B3D_ASSERT(bufferedStream.Tell() == endBits);

	// Don't set report callback until we actually do the reads
	mReportProgress = std::move(progress);
	bufferedStream.Seek((uint64_t)start * 8);

	// Now actually decode the objects
	BinaryDeserializationContext::ObjectDeserializationData* const rootObjectToDeserialize = deserializationContext.GetObjectDeserializationData(rootObjectId);
	B3D_ASSERT(rootObjectToDeserialize != nullptr);

	SPtr<IReflectable> rootObject = rootObjectToDeserialize->Object;

	rootObjectToDeserialize->DeserializationInProgress = true;
	deserializationContext.DeserializeReflectableObject(schema, rootObject);
	rootObjectToDeserialize->DeserializationInProgress = false;
	rootObjectToDeserialize->IsDeserialized = true;

	bufferedStream.Seek((uint64_t)endBits);
	stream->Seek(end);

	B3D_ASSERT(bufferedStream.Tell() == endBits);

	if(mReportProgress)
		mReportProgress(1.0f);

	return rootObject;
}

SPtr<IReflectable> BinarySerializer::Decode(const SPtr<DataStream>& stream, u32 dataLength, BinarySerializerFlags flags, std::function<void(float)> progress, SPtr<RTTISchema> schema)
{
	RTTIOperationContext rttiOperationContext;
	return Decode(stream, dataLength, rttiOperationContext, flags, std::move(progress), std::move(schema));
}

#undef COPY_TO_BUFFER
