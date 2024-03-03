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
#include "Reflection/BsRTTIIteratorField.h"
#include "Utility/BsBufferedBitstream.h"

using namespace bs;

constexpr u32 BinarySerializer::kReportAfterBytes;
constexpr u32 BinarySerializer::kWriteBufferSize;
constexpr u32 BinarySerializer::kFlushAfterBytes;
constexpr u32 BinarySerializer::kPreloadChunkBytes;

/**
 * Contains used combinations of 00xx xxx0 bits in the field type encoding.
 *
 * Note: This should be refactored so type is stored in the first 2 bits, with iterator/array/dynamic size bits in the rest.
 * For legacy sake we're keeping this convoluted way until we're ready to break serialization.
 */
enum FieldTypeBits : u8
{
	FT_PlainTypeWithFixedSize							= 0b00'000,
	FT_ArrayOfPlainTypesWithFixedSize					= 0b00'001,
	FT_IteratorPlainTypeWithDynamicSize					= 0b00'010,
	//Unused											= 0b00'011,
	//Unused											= 0b00'100,
	//Unused											= 0b00'101,
	//Unused											= 0b00'110,
	//Unused											= 0b00'111,
	//Unused											= 0b01'000,
	//Unused											= 0b01'001,
	FT_IteratorPlainTypeWithFixedSize					= 0b01'010,
	FT_IteratorReflectable								= 0b01'011,
	FT_IteratorReflectablePointer						= 0b01'100,
	FT_IteratorPlainTypeWithFixedSizeWithAnotherType	= 0b01'101,
	FT_IteratorReflectableWithAnotherType				= 0b01'110,
	FT_IteratorReflectablePointerWithAnotherType		= 0b01'111,
	FT_PlainTypeWithDynamicSize							= 0b10'000,
	FT_ArrayOfPlainTypesWithDynamicSize					= 0b10'001,
	FT_DataBlock										= 0b10'010,
	FT_IteratorPlainTypeWithDynamicSizeWithAnotherType	= 0b10'011,
	FT_Reflectable										= 0b10'100,
	FT_ReflectableArray									= 0b10'101,
	//Unused											= 0b10'110
	//Unused											= 0b10'111
	FT_ReflectablePointer								= 0b11'000,
	FT_ReflectablePointerArray							= 0b11'001,
	//Unused											= 0b11'010
	//Unused											= 0b11'011
	//Unused											= 0b11'100
	//Unused											= 0b11'101
	//Unused											= 0b11'110
	//Unused											= 0b11'111
};

/** Decodes the FieldTypeBits enum into individual parts. */
static void DecodeFieldTypeBits(FieldTypeBits bits, SerializableFieldType& outType, bool& outIsArray, bool& outIsIterator, bool& hasDynamicSize, bool& hasAnotherTypeFollowing)
{
	outIsArray = false;
	outIsIterator = false;
	hasDynamicSize = false;
	hasAnotherTypeFollowing = false;

	switch(bits)
	{
	case FT_PlainTypeWithDynamicSize:
		hasDynamicSize = true;
		[[fallthrough]];
	case FT_PlainTypeWithFixedSize:
		outType = SerializableFT_Plain;
		break;
	case FT_ArrayOfPlainTypesWithDynamicSize:
		hasDynamicSize = true;
		[[fallthrough]];
	case FT_ArrayOfPlainTypesWithFixedSize:
		outType = SerializableFT_Plain;
		outIsArray = true;
		break;
	case FT_DataBlock:
		outType = SerializableFT_DataBlock;
		break;
	case FT_Reflectable:
		outType = SerializableFT_Reflectable;
		break;
	case FT_ReflectableArray:
		outType = SerializableFT_Reflectable;
		outIsArray = true;
		break;
	case FT_ReflectablePointer:
		outType = SerializableFT_ReflectablePtr;
		break;
	case FT_ReflectablePointerArray:
		outType = SerializableFT_ReflectablePtr;
		outIsArray = true;
		break;
	case FT_IteratorPlainTypeWithFixedSizeWithAnotherType:
		hasAnotherTypeFollowing = true;
		[[fallthrough]];
	case FT_IteratorPlainTypeWithFixedSize:
		outType = SerializableFT_Plain;
		outIsArray = true;
		outIsIterator = true;
		break;
	case FT_IteratorPlainTypeWithDynamicSizeWithAnotherType:
		hasAnotherTypeFollowing = true;
		[[fallthrough]];
	case FT_IteratorPlainTypeWithDynamicSize:
		outType = SerializableFT_Plain;
		outIsArray = true;
		outIsIterator = true;
		hasDynamicSize = true;
		break;
	case FT_IteratorReflectableWithAnotherType:
		hasAnotherTypeFollowing = true;
		[[fallthrough]];
	case FT_IteratorReflectable:
		outType = SerializableFT_Reflectable;
		outIsArray = true;
		outIsIterator = true;
		break;
	case FT_IteratorReflectablePointerWithAnotherType:
		hasAnotherTypeFollowing = true;
		[[fallthrough]];
	case FT_IteratorReflectablePointer:
		outType = SerializableFT_ReflectablePtr;
		outIsArray = true;
		outIsIterator = true;
		break;
	default:
		B3D_ENSURE(false);
		break;
	}
}
/** Encodes a set of individual flags into the FieldTypeBits enum. */
static FieldTypeBits EncodeFieldTypeBits(SerializableFieldType type, bool isArray, bool isIterator, bool hasDynamicSize, bool hasAnotherTypeFollowing)
{
	if(isIterator)
	{
		B3D_ASSERT(isArray); // For now, all iterators are also considered array types

		switch(type)
		{
		case SerializableFT_Plain:
			if(hasDynamicSize)
			{
				if(hasAnotherTypeFollowing)
					return FT_IteratorPlainTypeWithDynamicSizeWithAnotherType;

				return FT_IteratorPlainTypeWithDynamicSize;
			}

			if(hasAnotherTypeFollowing)
				return FT_IteratorPlainTypeWithFixedSizeWithAnotherType;

			return FT_IteratorPlainTypeWithFixedSize;
		case SerializableFT_Reflectable:
			if(hasAnotherTypeFollowing)
				return FT_IteratorReflectableWithAnotherType;

			return FT_IteratorReflectable;
		case SerializableFT_ReflectablePtr:
			if(hasAnotherTypeFollowing)
				return FT_IteratorReflectablePointerWithAnotherType;

			return FT_IteratorReflectablePointer;
		}
	}
	else
	{
		switch(type)
		{
		case SerializableFT_Plain:
			if(hasDynamicSize)
			{
				if(isArray)
					return FT_ArrayOfPlainTypesWithDynamicSize;

				return FT_PlainTypeWithDynamicSize;
			}

			if(isArray)
				return FT_ArrayOfPlainTypesWithFixedSize;

			return FT_PlainTypeWithFixedSize;
		case SerializableFT_DataBlock:
			return FT_DataBlock;
		case SerializableFT_Reflectable:
			if(isArray)
				return FT_ReflectableArray;

			return FT_Reflectable;
		case SerializableFT_ReflectablePtr:
			if(isArray)
				return FT_ReflectablePointerArray;

			return FT_ReflectablePointer;
		}
	}

	B3D_ENSURE(false);
	return FT_PlainTypeWithFixedSize;
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
	static FieldTypeMetaData Create(const RTTIFieldSchema& fieldSchema, const RTTIFieldTypeSchema& fieldTypeSchema, bool isLastFieldInType, bool isAnotherFieldTypeFollowing);

	/** Converts the internal data to RTTIFieldTypeSchema. */
	RTTIFieldTypeSchema ToFieldTypeSchema(bool& outIsArray, bool& outIsIterator, bool& outHasMoreTypesFollowing) const;

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


FieldTypeMetaData FieldTypeMetaData::Create(const RTTIFieldSchema& fieldSchema, const RTTIFieldTypeSchema& fieldTypeSchema, bool isLastFieldInType, bool isAnotherFieldTypeFollowing)
{
	FieldTypeMetaData output;
	output.IsObjectDescriptor = 0;

	B3D_ASSERT(fieldTypeSchema.Type != SerializableFT_DataBlock || !fieldSchema.IsArray);
	output.FieldType = EncodeFieldTypeBits(fieldTypeSchema.Type, fieldSchema.IsArray, fieldSchema.IsIterator, fieldTypeSchema.HasDynamicSize, isAnotherFieldTypeFollowing);
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

RTTIFieldTypeSchema FieldTypeMetaData::ToFieldTypeSchema(bool& outIsArray, bool& outIsIterator, bool& outHasMoreTypesFollowing) const
{
	RTTIFieldTypeSchema schema;

	DecodeFieldTypeBits(FieldType, schema.Type, outIsArray, outIsIterator, schema.HasDynamicSize, outHasMoreTypesFollowing);

	if(!IsExtended)
		schema.FixedSize = FixedSizeOrAdditionalData.FixedSize;
	else
		schema.FieldTypeId = FixedSizeOrAdditionalData.BuiltinTypeId;

	return schema;
}

/** Handles the deserialization portion of BinarySerialized. */
class DeserializationContext
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

	DeserializationContext(FrameAllocator& allocator, BufferedBitstreamReader& stream, size_t dataEnd, BinarySerializerFlags flags, SerializationContext* rttiContext);

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
	struct ObjectMetaData // TODO - Move outside of the class
	{
		u32 ObjectMeta;
		u32 TypeId;
	};

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

	/** Decode meta field that was encoded using encodeFieldMetaData().*/
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
	SerializationContext* mRTTIContext = nullptr;
	BufferedBitstreamReader& mStream; /**< Stream from which to read the data. */
	size_t mDataEnd = 0; /**< Byte at which the data ends. Stream will not be advanced past this point. */
	BinarySerializerFlags mFlags = BinarySerializerFlag::None; /**< Flags used to control deserialization. */

	Map<u32, ObjectDeserializationData> mReflectableObjectsToDeserialize;
};

DeserializationContext::DeserializationContext(FrameAllocator& allocator, BufferedBitstreamReader& stream, size_t dataEnd, BinarySerializerFlags flags, SerializationContext* rttiContext)
	: mAllocator(allocator), mStream(stream), mDataEnd(dataEnd), mFlags(flags), mRTTIContext(rttiContext)
{ }

bool DeserializationContext::DeserializeReflectableObject(SPtr<RTTISchema> outputObjectSchema, const SPtr<IReflectable>& output)
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
	{
		B3D_EXCEPT(InternalErrorException, "Encountered a base-class object while looking for a new object. Base class objects are only supposed to be parts of a larger object.");
	}

	RTTITypeBase* rtti = nullptr;
	if(output)
		rtti = output->GetRtti();

	FrameVector<RTTITypeBase*> rttiInstances;

	auto finalizeObject = [&rttiInstances, this](IReflectable* object)
	{
		// Note: It would make sense to finish deserializing derived classes before base classes, but some code
		// depends on the old functionality, so we'll keep it this way
		for(auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
		{
			RTTITypeBase* curRTTI = *iter;

			curRTTI->OnDeserializationEnded(object, mRTTIContext);
			mAllocator.Destruct(curRTTI);
		}

		rttiInstances.clear();
	};

	RTTITypeBase* curRTTI = rtti;
	while(curRTTI)
	{
		RTTITypeBase* rttiInstance = curRTTI->CloneInternal(mAllocator);
		rttiInstances.push_back(rttiInstance);

		curRTTI = curRTTI->GetBaseClass();
	}

	// Iterate in reverse to notify base classes before derived classes
	for(auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
		(*iter)->OnDeserializationStarted(output.get(), mRTTIContext);

	RTTITypeBase* rttiInstance = nullptr;
	u32 rttiInstanceIdx = 0;
	if(!rttiInstances.empty())
		rttiInstance = rttiInstances[0];

	Vector<RTTIFieldSchema>::iterator fieldSchemaIter;
	if(outputObjectSchema)
		fieldSchemaIter = outputObjectSchema->FieldSchemas.begin();

	while(mStream.Tell() < mDataEnd)
	{
		RTTIFieldSchema fieldSchema;
		SPtr<RTTISchema> fieldTypeSchema;
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
					fieldSchema = ReadFieldMetaData(mStream, terminator);
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
				fieldSchema = *fieldSchemaIter;
				fieldTypeSchema = fieldSchema.FieldTypeSchema;

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

		RTTIField* curGenericField = nullptr;

		if(rtti != nullptr)
			curGenericField = rtti->FindField(fieldSchema.Id);

		if(curGenericField != nullptr)
		{
			if(!fieldSchema.HasDynamicSize && curGenericField->Schema.Size != fieldSchema.Size)
			{
				B3D_EXCEPT(InternalErrorException, "Data type mismatch. Type size stored in file and actual type size don't match. (" + ToString(curGenericField->Schema.Size.Bytes) + " vs. " + ToString(fieldSchema.Size.Bytes) + ")");
			}

			if(curGenericField->Schema.IsArray != fieldSchema.IsArray)
			{
				B3D_EXCEPT(InternalErrorException, "Data type mismatch. One is array, other is a single type.");
			}

			if(curGenericField->Schema.Type != fieldSchema.Type)
			{
				B3D_EXCEPT(InternalErrorException, "Data type mismatch. Field types don't match. " + ToString(u32(curGenericField->Schema.Type)) + " vs. " + ToString(u32(fieldSchema.Type)));
			}
		}

		if(fieldSchema.IsIterator)
		{
			u32 elementCount = 1;
			if(fieldSchema.IsArray)
			{
				if(compressed)
					mStream.ReadVarInt(elementCount);
				else
					mStream.ReadBytes(elementCount);
			}

			RTTIIteratorField* const field = static_cast<RTTIIteratorField*>(curGenericField);
			UPtr<IRTTIIterator> iterator;

			if(field != nullptr)
				iterator = field->GetIterator(rttiInstance, output.get(), mAllocator);

			for(u32 elementIndex = 0; elementIndex < elementCount; ++elementIndex)
			{
				void* fieldValue = nullptr;
				if(field != nullptr && iterator != nullptr)
					fieldValue = field->CreateEmptyFieldValue(mAllocator);

				for(u32 typeIndex = 0; typeIndex < (u32)fieldSchema.FieldTypes.Size(); ++typeIndex)
				{
					const RTTIFieldTypeSchema& fieldSubTypeSchema = fieldSchema.FieldTypes[typeIndex];

					switch(fieldSubTypeSchema.Type)
					{
					case SerializableFT_ReflectablePtr:
						{
							const u32 referencedObjectId = ReadReferencedReflectableObjectId();

							// If reading from schema we need to create object here as we don't know its type during the normal pass
							if(outputObjectSchema != nullptr)
								EnsureReflectableObjectExists(referencedObjectId, fieldTypeSchema);

							if(field != nullptr)
							{
								SPtr<IReflectable> referencedObject;
								if(field->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
									referencedObject = GetReflectableObject(referencedObjectId);
								else
									referencedObject = GetOrDeserializeReflectableObject(referencedObjectId, fieldTypeSchema);

								field->SetReflectablePointer(fieldValue, typeIndex, referencedObject);
							}
							break;
						}
					case SerializableFT_Reflectable:
						{
							SPtr<IReflectable> referencedObject;
							if(field != nullptr)
								referencedObject = IReflectable::CreateInstanceFromTypeId(field->Schema.FieldTypes[typeIndex].FieldTypeId);

							DeserializeReflectableObject(fieldTypeSchema, referencedObject);

							if(field != nullptr)
							{
								// Note: Would be nice to avoid this copy by value and decode directly into the field
								field->SetReflectable(fieldValue, typeIndex, *referencedObject);
							}

							break;
						}
					case SerializableFT_Plain:
						{
							uint64_t typeSizeBits = fieldSchema.Size.GetBits();
							if(fieldSchema.HasDynamicSize)
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

							if(field != nullptr)
							{
								mStream.Preload((uint32_t)Math::DivideAndRoundUp(typeSizeBits, (uint64_t)8));
								field->ReadPlainTypeTupleFromStream(fieldValue, typeIndex, mStream.GetBitstream(), compressed);

								mStream.Skip(typeSizeBits);
							}
							else
							{
								bool builtin = fieldSchema.FieldTypeId < 16;
								if(compressed && builtin)
									SkipBuiltinType(fieldSchema.FieldTypeId, mStream, compressed);
								else
									mStream.Skip(typeSizeBits);
							}
							break;
						}
					default:
						B3D_EXCEPT(InternalErrorException, "Error decoding data. Encountered a type I don't know how to decode. Type: " + ToString(u32(fieldSchema.Type)) + ", Is array: " + ToString(fieldSchema.IsArray));
					}

					if(field != nullptr && iterator != nullptr)
						field->SetIteratorValue(rttiInstance, output.get(), mAllocator, *iterator, fieldValue);
				}
			}
		}
		else if(fieldSchema.IsArray)
		{
			u32 arrayNumElems = 1;
			if(compressed)
				mStream.ReadVarInt(arrayNumElems);
			else
				mStream.ReadBytes(arrayNumElems);

			if(curGenericField != nullptr)
				curGenericField->SetArraySize(rttiInstance, output.get(), arrayNumElems);

			switch(fieldSchema.Type)
			{
			case SerializableFT_ReflectablePtr:
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

					for(u32 i = 0; i < arrayNumElems; i++)
					{
						const u32 referencedObjectId = ReadReferencedReflectableObjectId();

						// If reading from schema we need to create object here as we don't know its type during the normal pass
						if(outputObjectSchema != nullptr)
							EnsureReflectableObjectExists(referencedObjectId, fieldTypeSchema);

						if(curField != nullptr)
						{
							SPtr<IReflectable> referencedObject;
							if(curField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
								referencedObject = GetReflectableObject(referencedObjectId);
							else
								referencedObject = GetOrDeserializeReflectableObject(referencedObjectId, fieldTypeSchema);

							curField->SetArrayValue(rttiInstance, output.get(), i, referencedObject);
						}
					}

					break;
				}
			case SerializableFT_Reflectable:
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

					for(u32 i = 0; i < arrayNumElems; i++)
					{
						SPtr<IReflectable> childObj;
						if(curField)
							childObj = curField->NewObject();

						DeserializeReflectableObject(fieldTypeSchema, childObj);

						if(curField != nullptr)
						{
							// Note: Would be nice to avoid this copy by value and decode directly into the field
							curField->SetArrayValue(rttiInstance, output.get(), i, *childObj);
						}
					}
					break;
				}
			case SerializableFT_Plain:
				{
					auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

					for(u32 i = 0; i < arrayNumElems; i++)
					{
						uint64_t typeSizeBits = fieldSchema.Size.GetBits();
						if(fieldSchema.HasDynamicSize)
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

						if(curField != nullptr)
						{
							mStream.Preload((uint32_t)Math::DivideAndRoundUp(typeSizeBits, (uint64_t)8));
							curField->ArrayElemFromBuffer(rttiInstance, output.get(), i, mStream.GetBitstream(), compressed);

							mStream.Skip(typeSizeBits);
						}
						else
						{
							bool builtin = fieldSchema.FieldTypeId < 16;
							if(compressed && builtin)
								SkipBuiltinType(fieldSchema.FieldTypeId, mStream, compressed);
							else
								mStream.Skip(typeSizeBits);
						}
					}
					break;
				}
			default:
				B3D_EXCEPT(InternalErrorException, "Error decoding data. Encountered a type I don't know how to decode. Type: " + ToString(u32(fieldSchema.Type)) + ", Is array: " + ToString(fieldSchema.IsArray));
			}
		}
		else
		{
			switch(fieldSchema.Type)
			{
			case SerializableFT_ReflectablePtr:
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

					const u32 referencedObjectId = ReadReferencedReflectableObjectId();

					// If reading from schema we need to create object here as we don't know its type during the normal pass
					if(outputObjectSchema != nullptr)
						EnsureReflectableObjectExists(referencedObjectId, fieldTypeSchema);

					if(curField != nullptr)
					{
						SPtr<IReflectable> referencedObject;
						if(curField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
							referencedObject = GetReflectableObject(referencedObjectId);
						else
							referencedObject = GetOrDeserializeReflectableObject(referencedObjectId, fieldTypeSchema);

						curField->SetValue(rttiInstance, output.get(), referencedObject);
					}

					break;
				}
			case SerializableFT_Reflectable:
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

					// Note: Ideally we can skip decoding the entry if the field no longer exists
					SPtr<IReflectable> childObj;
					if(curField)
						childObj = curField->NewObject();

					DeserializeReflectableObject(fieldTypeSchema, childObj);

					if(curField != nullptr)
					{
						// Note: Would be nice to avoid this copy by value and decode directly into the field
						curField->SetValue(rttiInstance, output.get(), *childObj);
					}

					break;
				}
			case SerializableFT_Plain:
				{
					auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

					uint64_t typeSizeBits = fieldSchema.Size.GetBits();
					if(fieldSchema.HasDynamicSize)
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

					if(curField != nullptr)
					{
						mStream.Preload((uint32_t)Math::DivideAndRoundUp(typeSizeBits, (uint64_t)8));
						curField->FromBuffer(rttiInstance, output.get(), mStream.GetBitstream(), compressed);

						mStream.Skip(typeSizeBits);
					}
					else
					{
						bool builtin = fieldSchema.FieldTypeId < 16;
						if(compressed && builtin)
							SkipBuiltinType(fieldSchema.FieldTypeId, mStream, compressed);
						else
							mStream.Skip(typeSizeBits);
					}

					break;
				}
			case SerializableFT_DataBlock:
				{
					auto* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

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

					break;
				}
			default:
				B3D_EXCEPT(InternalErrorException, "Error decoding data. Encountered a type I don't know how to decode. Type: " + ToString(u32(fieldSchema.Type)) + ", Is array: " + ToString(fieldSchema.IsArray));
			}

			mStream.ClearBuffered(false);
		}
	}

	finalizeObject(output.get());
	return false;
}

u32 DeserializationContext::ReadReferencedReflectableObjectId() const
{
	const bool isCompressed = mFlags.IsSet(BinarySerializerFlag::Compress);

	u32 childObjectId = 0;
	if(isCompressed)
		mStream.ReadVarInt(childObjectId);
	else
		mStream.ReadBytes(childObjectId);
	
	return childObjectId;
}

void DeserializationContext::EnsureReflectableObjectExists(u32 reflectableObjectId, const SPtr<RTTISchema>& objectSchema)
{
	auto foundExisting = mReflectableObjectsToDeserialize.find(reflectableObjectId);

	if(foundExisting != mReflectableObjectsToDeserialize.end())
		return;

	SPtr<IReflectable> object = IReflectable::CreateInstanceFromTypeId(objectSchema->TypeId);
	mReflectableObjectsToDeserialize.insert(std::make_pair(reflectableObjectId, ObjectDeserializationData(object, ~0u, objectSchema)));
}

SPtr<IReflectable> DeserializationContext::GetOrDeserializeReflectableObject(u32 reflectableObjectId, const SPtr<RTTISchema>& objectSchema)
{
	auto foundExisting = mReflectableObjectsToDeserialize.find(reflectableObjectId);
	if(foundExisting == mReflectableObjectsToDeserialize.end())
	{
		if(reflectableObjectId != 0)
			B3D_LOG(Warning, Generic, "When deserializing, object ID: {0} was found but no such object was contained in the file.", reflectableObjectId);

		return nullptr;
	}

	ObjectDeserializationData& objectToDeserialize = foundExisting->second;
	if(!objectToDeserialize.IsDeserialized)
	{
		if(objectToDeserialize.DeserializationInProgress)
		{
			B3D_LOG(Warning, Generic, "Detected a circular reference when decoding. Referenced "
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

DeserializationContext::ObjectDeserializationData* DeserializationContext::GetObjectDeserializationData(u32 reflectableObjectId)
{
	auto found = mReflectableObjectsToDeserialize.find(reflectableObjectId);
	if(found == mReflectableObjectsToDeserialize.end())
		return nullptr;

	return &found->second;
}

SPtr<IReflectable> DeserializationContext::GetReflectableObject(u32 reflectableObjectId) const
{
	auto foundExisting = mReflectableObjectsToDeserialize.find(reflectableObjectId);
	if(foundExisting == mReflectableObjectsToDeserialize.end())
	{
		if(reflectableObjectId != 0)
			B3D_LOG(Warning, Generic, "When deserializing, object ID: {0} was found but no such object was contained in the file.", reflectableObjectId);

		return nullptr;
	}
	
	return foundExisting->second.Object;
}

void DeserializationContext::CreateReflectableObject(u32 reflectableObjectId, u32 reflectableObjectTypeId, u64 locationInStream, const SPtr<RTTISchema>& objectSchema)
{
	const SPtr<IReflectable>& object = IReflectable::CreateInstanceFromTypeId(reflectableObjectTypeId);
	mReflectableObjectsToDeserialize.insert(std::make_pair(reflectableObjectId, ObjectDeserializationData(object, locationInStream, objectSchema)));
}

RTTIFieldSchema DeserializationContext::ReadFieldMetaData(BufferedBitstreamReader& stream, bool& terminator)
{
	u32 fieldMetaData;
	stream.ReadBytes(fieldMetaData);

	if(IsObjectMetaData(fieldMetaData))
	{
		B3D_EXCEPT(InternalErrorException, "Meta data represents an object description but is trying to be decoded as a field descriptor.");
	}

	RTTIFieldSchema fieldSchema;
	fieldSchema.Id = (u16)((fieldMetaData >> 16) & 0xFFFF);

	FieldTypeMetaData firstFieldTypeMetaData;
	firstFieldTypeMetaData.PackedData = (u16)(fieldMetaData & 0xFFFF);

	bool hasMoreFieldTypes;
	const RTTIFieldTypeSchema firstFieldTypeSchema = firstFieldTypeMetaData.ToFieldTypeSchema(fieldSchema.IsArray, fieldSchema.IsIterator, hasMoreFieldTypes);

	terminator = firstFieldTypeMetaData.IsLastFieldInType;
	fieldSchema.FieldTypes.Add(firstFieldTypeSchema);

	// For now, duplicate these fields. But ultimately we'll remove them in favor data stored of FieldTypes
	fieldSchema.HasDynamicSize = firstFieldTypeSchema.HasDynamicSize;
	fieldSchema.Type = firstFieldTypeSchema.Type;
	fieldSchema.Size = firstFieldTypeSchema.FixedSize;
	fieldSchema.FieldTypeId = firstFieldTypeSchema.FieldTypeId;

	while(hasMoreFieldTypes)
	{
		u16 fieldTypeMetaData;
		stream.ReadBytes(fieldTypeMetaData);

		FieldTypeMetaData additionalFieldTypeMetaData;
		additionalFieldTypeMetaData.PackedData = fieldTypeMetaData;

		bool isArray, isIterator;
		const RTTIFieldTypeSchema additionalFieldTypeSchema = additionalFieldTypeMetaData.ToFieldTypeSchema(isArray, isIterator, hasMoreFieldTypes);
		fieldSchema.FieldTypes.Add(firstFieldTypeSchema);
	}

	return fieldSchema;
}

void DeserializationContext::SkipBuiltinType(u32 fieldType, BufferedBitstreamReader& stream, bool compressed)
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

bool DeserializationContext::IsFieldTerminator(u8 data)
{
	if(IsObjectMetaData(data))
	{
		B3D_EXCEPT(InternalErrorException, "Meta data represents an object description but is trying to be decoded as a field descriptor.");
	}

	return (data & 0x40) != 0;
}

void DeserializationContext::DecodeObjectMetaData(ObjectMetaData encodedData, u32& objId, u32& objTypeId, bool& isBaseClass)
{
	if(!IsObjectMetaData(encodedData.ObjectMeta))
	{
		B3D_EXCEPT(InternalErrorException, "Meta data represents a field description but is trying to be decoded as an object descriptor.");
	}

	DecodeObjectMetaData(encodedData.ObjectMeta, objId, isBaseClass);
	objTypeId = encodedData.TypeId;
}

void DeserializationContext::DecodeObjectMetaData(u32 encodedData, u32& objId, bool& isBaseClass)
{
	if(!IsObjectMetaData(encodedData))
	{
		B3D_EXCEPT(InternalErrorException, "Meta data represents a field description but is trying to be decoded as an object descriptor.");
	}

	objId = (encodedData >> 2) & 0x3FFFFFFF;
	isBaseClass = (encodedData & 0x02) != 0;
}

bool DeserializationContext::IsObjectMetaData(u32 encodedData)
{
	return ((encodedData & 0x01) != 0);
}

u32 DeserializationContext::ReadObjectMetaData(BufferedBitstreamReader& stream, BinarySerializerFlags flags, uint32_t& objId, uint32_t& objTypeId, bool& isBaseType)
{
	if(!flags.IsSet(BinarySerializerFlag::NoMeta))
	{
		ObjectMetaData objectMetaData;
		objectMetaData.ObjectMeta = 0;
		objectMetaData.TypeId = 0;

		if(stream.ReadBytes(objectMetaData) != sizeof(ObjectMetaData))
			B3D_EXCEPT(InternalErrorException, "Error decoding data.");

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
				B3D_EXCEPT(InternalErrorException, "Error decoding data.");

			bitsRead = sizeof(objectMetaData) * 8;
		}

		DecodeObjectMetaData(objectMetaData, objId, isBaseType);
		objTypeId = 0; // Unavailable, needs to be read from a schema

		return bitsRead;
	}
}

BinarySerializer::BinarySerializer()
	: mAlloc(&GetFrameAllocator())
{}

void BinarySerializer::Encode(IReflectable* object, const SPtr<DataStream>& stream, BinarySerializerFlags flags, SerializationContext* context)
{
	mReflectableObjectsToSerialize.clear();
	mReflectableObjectToID.clear();
	mLastUsedObjectId = 1;
	mContext = context;
	mBuffer.Seek(0);

	mAlloc->MarkFrame();

	BufferedBitstreamWriter bufferedStream(&mBuffer, stream, kWriteBufferSize, kFlushAfterBytes);

	Vector<SPtr<IReflectable>> encodedObjects;
	u32 objectId = FindOrCreateReflectableObjectId(object);

	// Encode primary object and its value types
	if(!SerializeReflectableObject(object, objectId, bufferedStream, flags))
	{
		B3D_LOG(Error, Serialization, "Destination buffer is null or not large enough.");
		return;
	}

	// Encode pointed to objects and their value types
	UnorderedSet<u32> serializedObjects;
	while(true)
	{
		auto iter = mReflectableObjectsToSerialize.begin();
		bool foundObjectToProcess = false;
		for(; iter != mReflectableObjectsToSerialize.end(); ++iter)
		{
			auto foundExisting = serializedObjects.find(iter->ObjectId);
			if(foundExisting != serializedObjects.end())
				continue; // Already processed

			SPtr<IReflectable> curObject = iter->Object;
			u32 curObjectid = iter->ObjectId;
			serializedObjects.insert(curObjectid);
			mReflectableObjectsToSerialize.erase(iter); // TODO - Should update iter to returned value from erase()

			if(!SerializeReflectableObject(curObject.get(), curObjectid, bufferedStream, flags))
			{
				B3D_LOG(Error, Serialization, "Destination buffer is null or not large enough.");
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
	mReflectableObjectsToSerialize.clear();
	mReflectableObjectToID.clear();

	mAlloc->Clear();
}

SPtr<IReflectable> BinarySerializer::Decode(const SPtr<DataStream>& stream, u32 dataLength, BinarySerializerFlags flags, SerializationContext* context, std::function<void(float)> progress, SPtr<RTTISchema> schema)
{
	mContext = context;
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
			B3D_LOG(Error, Serialization, "Cannot decode an object without meta-data nor schema.");
			return nullptr;
		}
	}

	BufferedBitstreamReader bufferedStream(&mBuffer, stream, kPreloadChunkBytes, kFlushAfterBytes);
	DeserializationContext deserializationContext(*mAlloc, bufferedStream, endBits, flags, context);

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
			B3D_EXCEPT(InternalErrorException, "Encountered a base-class object while looking for a new object. "
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
				DeserializationContext::ObjectDeserializationData* const objectToDeserialize = deserializationContext.GetObjectDeserializationData(objectId);
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
	DeserializationContext::ObjectDeserializationData* const rootObjectToDeserialize = deserializationContext.GetObjectDeserializationData(rootObjectId);
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

bool BinarySerializer::SerializeReflectableObject(IReflectable* object, u32 objectId, BufferedBitstreamWriter& stream, BinarySerializerFlags flags)
{
	const bool writeMeta = !flags.IsSet(BinarySerializerFlag::NoMeta);
	const bool compress = flags.IsSet(BinarySerializerFlag::Compress);

	RTTITypeBase* rtti = object->GetRtti();
	bool isBaseClass = false;

	FrameStack<RTTITypeBase*> rttiInstances;

	const auto cleanup = [&]() // TODO - Use scope guard
	{
		while(!rttiInstances.empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.top();
			rttiInstance->OnSerializationEnded(object, mContext);
			mAlloc->Destruct(rttiInstance);

			rttiInstances.pop();
		}
	};

	// If an object has base classes, we need to iterate through all of them
	do
	{
		RTTITypeBase* rttiInstance = rtti->CloneInternal(*mAlloc);
		rttiInstances.push(rttiInstance);

		rttiInstance->OnSerializationStarted(object, mContext);

		if(writeMeta)
		{
			// Encode object ID & type
			ObjectMetaData objectMetaData = EncodeObjectMetaData(objectId, rtti->GetRttiId(), isBaseClass);
			stream.WriteBytes(objectMetaData);
		}
		else
		{
			// Encode object ID
			u32 objectMetaData = EncodeObjectMetaData(objectId, isBaseClass);

			if(compress)
				stream.WriteVarInt(objectMetaData);
			else
				stream.WriteBytes(objectMetaData);
		}

		const u32 numFields = rtti->GetNumFields();
		for(u32 i = 0; i < numFields; i++)
		{
			RTTIField* curGenericField = rtti->GetField(i);

			if(writeMeta)
			{
				// Copy field ID & other meta-data like field size and type
				WriteFieldMetaData(curGenericField->Schema, false, stream);
			}

			if(curGenericField->Schema.IsIterator)
			{
				RTTIIteratorField* const field = static_cast<RTTIIteratorField*>(curGenericField);
				UPtr<IRTTIIterator> iterator = field->GetIterator(rttiInstance, object, *mAlloc);

				if(field->Schema.IsArray)
				{
					u32 elementCount = 0;
					if(iterator != nullptr)
						elementCount = (u32)iterator->GetElementCount();

					// Copy num vector elements
					if(compress)
						stream.WriteVarInt(elementCount);
					else
						stream.WriteBytes(elementCount);
				}

				if(iterator != nullptr)
				{
					for(; iterator->IsValid(); iterator->Increment())
					{
						const void* fieldValue = iterator->GetValue();
						for(u32 typeIndex = 0; typeIndex < (u32)field->Schema.FieldTypes.Size(); ++typeIndex)
						{
							const RTTIFieldTypeSchema& typeSchema = field->Schema.FieldTypes[typeIndex];
							switch(typeSchema.Type)
							{
							case SerializableFT_ReflectablePtr:
								{
									SPtr<IReflectable> childObject;

									if(!flags.IsSet(BinarySerializerFlag::Shallow))
										childObject = field->GetReflectablePointer(fieldValue, typeIndex);

									const u32 objectId = RegisterReflectableObjectForSerialization(childObject);
									if(compress)
										stream.WriteVarInt(objectId);
									else
										stream.WriteBytes(objectId);

									break;
								}
							case SerializableFT_Reflectable:
								{
									const IReflectable& childObject = field->GetReflectable(fieldValue, typeIndex);
									if(!SerializeReflectableObjectInline(const_cast<IReflectable*>(&childObject), stream, flags)) // TODO - Get rid of const cast
									{
										cleanup();
										return false;
									}

									break;
								}
							case SerializableFT_Plain:
								{
									field->WritePlainTypeTupleToStream(fieldValue, typeIndex, stream.GetBitstream(), compress);
									break;
								}
							default:
								B3D_LOG(Error, Serialization, "Error serializing data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}", typeSchema.Type, field->Schema.IsArray);
							}
						}
					}
				}
			}
			else if(curGenericField->Schema.IsArray)
			{
				u32 arrayNumElems = curGenericField->GetArraySize(rttiInstance, object);

				// Copy num vector elements
				if(compress)
					stream.WriteVarInt(arrayNumElems);
				else
					stream.WriteBytes(arrayNumElems);

				switch(curGenericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

						for(u32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
						{
							SPtr<IReflectable> childObject;

							if(!flags.IsSet(BinarySerializerFlag::Shallow))
								childObject = curField->GetArrayValue(rttiInstance, object, arrIdx);

							u32 objId = RegisterReflectableObjectForSerialization(childObject);
							if(compress)
								stream.WriteVarInt(objId);
							else
								stream.WriteBytes(objId);
						}

						break;
					}
				case SerializableFT_Reflectable:
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

						for(u32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
						{
							IReflectable& childObject = curField->GetArrayValue(rttiInstance, object, arrIdx);

							if(!SerializeReflectableObjectInline(&childObject, stream, flags))
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

						for(u32 arrIdx = 0; arrIdx < arrayNumElems; arrIdx++)
							curField->ArrayElemToStream(rttiInstance, object, arrIdx, stream.GetBitstream(), compress);

						break;
					}
				default:
					B3D_LOG(Error, Serialization, "Error encoding data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}", curGenericField->Schema.Type, curGenericField->Schema.IsArray);
				}
			}
			else
			{
				switch(curGenericField->Schema.Type)
				{
				case SerializableFT_ReflectablePtr:
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
						SPtr<IReflectable> childObject;

						if(!flags.IsSet(BinarySerializerFlag::Shallow))
							childObject = curField->GetValue(rttiInstance, object);

						u32 objId = RegisterReflectableObjectForSerialization(childObject);
						if(compress)
							stream.WriteVarInt(objId);
						else
							stream.WriteBytes(objId);

						break;
					}
				case SerializableFT_Reflectable:
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
						IReflectable& childObject = curField->GetValue(rttiInstance, object);

						if(!SerializeReflectableObjectInline(&childObject, stream, flags))
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

						u32 dataBlockSize = 0;
						SPtr<DataStream> blockStream = curField->GetValue(rttiInstance, object, dataBlockSize);

						// Data block size
						if(compress)
							stream.WriteVarInt(dataBlockSize);
						else
							stream.WriteBytes(dataBlockSize);

						// Data block data
						auto dataToStore = (u8*)B3DStackAllocate(dataBlockSize);
						blockStream->Read(dataToStore, dataBlockSize);

						stream.Align();
						stream.WriteBytes(dataToStore, dataBlockSize);
						B3DStackFree(dataToStore);

						break;
					}
				default:
					B3D_LOG(Error, Serialization, "Error encoding data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}", curGenericField->Schema.Type, curGenericField->Schema.IsArray);
				}
			}

			stream.Flush(false);
		}

		rtti = rtti->GetBaseClass();
		isBaseClass = true;
	}
	while(rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

	cleanup();

	return true;
}

u8 BinarySerializer::EncodeFieldTerminator()
{
	// See the documentation for WriteFieldMetaData() on why we're using this format
	return 0x40;
}

bool BinarySerializer::SerializeReflectableObjectInline(IReflectable* object, BufferedBitstreamWriter& stream, BinarySerializerFlags flags)
{
	if(object != nullptr)
	{
		if(!SerializeReflectableObject(object, 0, stream, flags))
			return false;

		if(!flags.IsSet(BinarySerializerFlag::NoMeta))
		{
			// Encode terminator field
			// Complex types require terminator fields because they can be embedded within other complex types and we need
			// to know when their fields end and parent's resume
			if(flags.IsSet(BinarySerializerFlag::Compress))
			{
				u8 metaData = EncodeFieldTerminator();
				stream.WriteBytes(metaData);
			}
			else
				WriteFieldMetaData(RTTIFieldSchema(), true, stream);
		}
	}

	return true;
}

void BinarySerializer::WriteFieldMetaData(const RTTIFieldSchema& fieldSchema, bool isLastFieldInType, BufferedBitstreamWriter& stream)
{
	// If O == 0 - Meta contains field information (Encoded using this method)
	//// Encoding if E = 0: IIII IIII IIII IIII SSSS SSSS ETFF FFFO
	//// Encoding if E = 1: IIII IIII IIII IIII BBBB xxxx ETFF FFFO
	//// I - Id
	//// S - Size
	//// F - Field type enum bits
	//// O - Object descriptor
	//// T - Terminator (last field in an inline object)
	//// E - Extended (size is replaced with additional meta-data)
	//// B - Built-in type ID

	FieldTypeMetaData firstFieldTypeMetaData;
	if(fieldSchema.FieldTypes.Empty()) // Old approach, single field type
	{
		const RTTIFieldTypeSchema fieldTypeSchema(fieldSchema.HasDynamicSize, fieldSchema.Size, fieldSchema.Type, fieldSchema.FieldTypeId, fieldSchema.FieldTypeSchema);
		firstFieldTypeMetaData = FieldTypeMetaData::Create(fieldSchema, fieldTypeSchema, isLastFieldInType, false);
	}
	else
	{
		firstFieldTypeMetaData = FieldTypeMetaData::Create(fieldSchema, fieldSchema.FieldTypes[0], isLastFieldInType, fieldSchema.FieldTypes.Size() > 1);
	}

	// Encodes field ID and meta-data for the first type
	const u32 fieldMetaData = fieldSchema.Id << 16 | (u32)firstFieldTypeMetaData.PackedData;
	stream.WriteBytes(fieldMetaData);

	const u32 fieldTypeCount = (u32)fieldSchema.FieldTypes.Size();
	for(u32 fieldTypeIndex = 1; fieldTypeIndex < fieldTypeCount; fieldTypeIndex++)
	{
		const bool isLastFieldType = (fieldTypeIndex + 1) == fieldTypeCount;
		FieldTypeMetaData additionalFieldTypeMetaData = FieldTypeMetaData::Create(fieldSchema, fieldSchema.FieldTypes[fieldTypeIndex], false, isLastFieldType);

		stream.WriteBytes(additionalFieldTypeMetaData.PackedData);
	}
}

BinarySerializer::ObjectMetaData BinarySerializer::EncodeObjectMetaData(u32 objId, u32 objTypeId, bool isBaseClass)
{
	// If O == 1 - Meta contains object instance information (Encoded using encodeObjectMetaData)
	//// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
	//// S - Size of the object identifier
	//// O - Object descriptor
	//// B - Base class indicator

	if(objId > 1073741823)
	{
		B3D_EXCEPT(InvalidParametersException, "Object ID is larger than we can store (max 30 bits): " + ToString(objId));
	}

	ObjectMetaData metaData;
	metaData.ObjectMeta = EncodeObjectMetaData(objId, isBaseClass);
	metaData.TypeId = objTypeId;
	return metaData;
}

u32 BinarySerializer::EncodeObjectMetaData(u32 objId, bool isBaseClass)
{
	// If O == 1 - Meta contains object instance information (Encoded using encodeObjectMetaData)
	//// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
	//// S - Size of the object identifier
	//// O - Object descriptor
	//// B - Base class indicator

	if(objId > 1073741823)
	{
		B3D_EXCEPT(InvalidParametersException, "Object ID is larger than we can store (max 30 bits): " + ToString(objId));
	}

	return (objId << 2) | (isBaseClass ? 0x02 : 0) | 0x01;
}

u32 BinarySerializer::FindOrCreateReflectableObjectId(IReflectable* object)
{
	void* const objectMemoryAddress = object;

	auto found = mReflectableObjectToID.find(objectMemoryAddress);
	if(found != mReflectableObjectToID.end())
		return found->second;

	u32 objId = mLastUsedObjectId++;
	mReflectableObjectToID.insert(std::make_pair(objectMemoryAddress, objId));

	return objId;
}

u32 BinarySerializer::RegisterReflectableObjectForSerialization(SPtr<IReflectable> object)
{
	if(object == nullptr)
		return 0;

	void* const objectMemoryAddress = object.get();

	auto found = mReflectableObjectToID.find(objectMemoryAddress);
	if(found == mReflectableObjectToID.end())
	{
		const u32 objectId = FindOrCreateReflectableObjectId(object.get());

		mReflectableObjectsToSerialize.push_back(ObjectToSerialize(objectId, object));
		mReflectableObjectToID.insert(std::make_pair(objectMemoryAddress, objectId));

		return objectId;
	}

	return found->second;
}

#undef COPY_TO_BUFFER
