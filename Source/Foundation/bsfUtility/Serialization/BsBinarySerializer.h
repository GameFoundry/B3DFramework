//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <utility>
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Serialization/BsSerializedObject.h"
#include "Reflection/BsRTTIField.h"
#include "Utility/BsBitstream.h"

namespace bs
{
	/** @addtogroup Serialization
	 *  @{
	 */

	class IReflectable;
	struct RTTISchema;
	class BufferedBitstreamReader;
	class BufferedBitstreamWriter;
	struct RTTIReflectableFieldBase;
	struct RTTIReflectablePtrFieldBase;
	struct SerializationContext;

	/** Flags used for controlling BinarySerializer encoding and decoding. */
	enum class BinarySerializerFlag
	{
		None = 0,
		/**
		 * Flag to be provided during encoding. Determines how to handle references objects. If set then
		 * references will not be encoded and will be set to null. If not set then references will be encoded
		 * as well as restored upon decoding.
		 */
		Shallow = 1 << 0,
		/**
		 * If set the encoder will use the specialized compression encoding/decoding strategy, suitable
		 * for network transmission. In particular the 'compress' parameter for plain type serialization
		 * will be set to true, allowing those types to be encoded in sub-byte representations that take
		 * up less space (e.g. boolean as 1 bit, integer as var-int, etc.). Data that was encoded using
		 * this strategy must be decoded using this strategy.
		 */
		Compress = 1 << 1,
		/**
		 * If true, no meta-data will be written. This saves on serialization size but it also means
		 * the data can only be decoded if the RTTI types are identical to when the object was encoded
		 * (e.g. no fields were added/removed from the types). Optionally you can also provide a previously
		 * saved RTTI schema from which to read the meta-data from. Data encoded using this flag must also
		 * be decoded with this flag provided.
		 */
		NoMeta = 1 << 2,
	};

	using BinarySerializerFlags = Flags<BinarySerializerFlag>;
	BS_FLAGS_OPERATORS(BinarySerializerFlag)

	/**
	 * Encodes/decodes all the fields of the provided object into/from a binary format. Fields are encoded using their
	 * unique IDs. Encoded data will remain compatible for decoding even if you modify the encoded class, as long as you
	 * assign new unique field IDs to added/modified fields.
	 * 			
	 * Like for any serializable class, fields are defined in RTTIType that each IReflectable class must be able to return.
	 *
	 * Any data the object or its children are pointing to will also be serialized (unless the pointer isn't registered in
	 * RTTIType). Upon decoding the pointer addresses will be set to proper values.
	 */
	class BS_UTILITY_EXPORT BinarySerializer
	{
	public:
		BinarySerializer();

		/**
		 * Encodes all serializable fields provided by @p object into a binary format.
		 * 
		 * @param[in]	object					Object to encode into binary format.
		 * @param[in]	stream					Stream into which to output the encoded data. The stream must own its memory
		 *										buffer so it may grow as required during encoding, or your must guarantee
		 *										the stream is of adequate size otherwise.
		 * @param[in]	flags					Flags used for controlling serialization.
		 * @param[in]	context					Optional object that will be passed along to all serialized objects through
		 *										their serialization callbacks. Can be used for controlling serialization,
		 *										maintaining state or sharing information between objects during
		 *										serialization.
		 */
		void encode(IReflectable* object, const SPtr<DataStream>& stream,
			BinarySerializerFlags flags = BinarySerializerFlag::None, SerializationContext* context = nullptr);

		/**
		 * Decodes an object from binary data.
		 *
		 * @param[in]	stream  	Stream containing the binary data to decode.
		 * @param[in]	dataLength	Length of the data in bytes. If zero, all the data from the stream will be read.
		 * @param[in]	context		Optional object that will be passed along to all serialized objects through
		 *							their deserialization callbacks. Can be used for controlling deserialization,
		 *							maintaining state or sharing information between objects during deserialization.
		 * @param[in]	progress	Optional callback that will occasionally trigger, reporting the current progress
		 *							of the operation. The reported value is in range [0, 1].
		 * @param[in]	schema		RTTI schema that contains information about types as they were when the data was
		 *							originally serialized. Schema is only used (and required) if BinarySerializerFlag::NoMeta
		 *							is set,	otherwise this information is read directly	from the encoded data. 
		 *
		 * @note
		 * Child elements are guaranteed to be fully deserialized before their parents, except for fields marked with WeakRef flag.
		 */
		SPtr<IReflectable> decode(const SPtr<DataStream>& stream, UINT32 dataLength,
			BinarySerializerFlags flags = BinarySerializerFlag::None, SerializationContext* context = nullptr,
			std::function<void(float)> progress = nullptr, SPtr<RTTISchema> schema = nullptr);
	private:
		/** Determines how many bytes need to be read before the progress report callback is triggered. */
		static constexpr UINT32 REPORT_AFTER_BYTES = 32768;

		/** Determines the size of the temporary write buffer. Should be greater than FLUSH_AFTER_BYTES. */
		static constexpr UINT32 WRITE_BUFFER_SIZE = 4096;

		/** Determines how often to flush from the local buffer into the output stream, when writing. */
		static constexpr UINT32 FLUSH_AFTER_BYTES = (UINT32)(WRITE_BUFFER_SIZE * 0.75f);

		/** Determines the minimum amount of bytes to preload into the temporary buffer. */
		static constexpr UINT32 PRELOAD_CHUNK_BYTES = (UINT32)(WRITE_BUFFER_SIZE * 0.25f);

		struct ObjectMetaData
		{
			UINT32 objectMeta;
			UINT32 typeId;
		};

		struct ObjectToEncode
		{
			ObjectToEncode(UINT32 objectId, SPtr<IReflectable> object)
				:objectId(objectId), object(std::move(object))
			{ }

			UINT32 objectId;
			SPtr<IReflectable> object;
		};

		struct ObjectToDecode
		{
			ObjectToDecode(SPtr<IReflectable> object = nullptr, uint64_t offset = 0, SPtr<RTTISchema> schema = nullptr)
				:object(std::move(object)), offset(offset), schema(std::move(schema))
			{ }

			SPtr<IReflectable> object;
			bool isDecoded = false;
			bool decodeInProgress = false; // Used for error reporting circular references
			uint64_t offset;
			SPtr<RTTISchema> schema;
		};

		/** Encodes a single IReflectable object. */
		bool encodeEntry(IReflectable* object, UINT32 objectId, BufferedBitstreamWriter& stream, BinarySerializerFlags flags);

		/**	Decodes a single IReflectable object. */
		bool decodeEntry(BufferedBitstreamReader& stream, size_t dataLength, BinarySerializerFlags flags, const SPtr<IReflectable>& output,
			SPtr<RTTISchema> schema);

		/**	Helper method for encoding a complex object and writing its data to a stream. */
		bool complexTypeToStream(IReflectable* object, BufferedBitstreamWriter& stream, BinarySerializerFlags flags);

		/**	Finds an existing, or creates a unique unique identifier for the specified object. */
		UINT32 findOrCreatePersistentId(IReflectable* object);

		/**
		 * Finds or creates an id for the provided object and returns it. And it adds the object to a list of objects that
		 * need to be encoded, if it's not already there.
		 */
		UINT32 registerObjectPtr(SPtr<IReflectable> object);

		/**
		 * Decodes object meta-data from the current location in the stream. Decoding accounts for the serializer flags to decode
		 * using the correct format. Returns number of bits read.
		 */
		static UINT32 readObjectMetaData(BufferedBitstreamReader& stream, BinarySerializerFlags flags, UINT32& objId, UINT32& objTypeId, bool& isBaseType);
		
		/** Encodes data required for representing a serialized field, into 4 bytes. */
		static UINT32 encodeFieldMetaData(const RTTIFieldSchema& fieldSchema, bool terminator);

		/** Decode meta field that was encoded using encodeFieldMetaData().*/
		static RTTIFieldSchema decodeFieldMetaData(UINT32 encodedData, bool& terminator);

		/** Encodes data representing a field terminator into 1 byte. */
		static UINT8 encodeFieldTerminator();

		/** Skips the builtin type at the current location in the stream. */
		static void skipBuiltinType(UINT32 fieldType, BufferedBitstreamReader& stream, bool compressed);
		
		/** Returns true if the data in the provided byte represents a field terminator as encoded with encodeFieldTerminator(). */
		static bool isFieldTerminator(UINT8 data);

		/**
		 * Encodes an object identifier, its type and other meta-data into 8 bytes.
		 *
		 * @param[in]	objId	   	Unique ID of the object instance. This can be a maximum of 30 bits, as two bits are reserved.
		 * @param[in]	objTypeId  	Unique ID of the object type.
		 * @param[in]	isBaseClass	True if this object is base class (that is, just a part of a larger object).
		 * @return		Encoded object id, type ID and other meta-data.
		 */
		static ObjectMetaData encodeObjectMetaData(UINT32 objId, UINT32 objTypeId, bool isBaseClass);

		/** Decode meta field that was encoded using encodeObjectMetaData(UINT32, UINT32, bool). */
		static void decodeObjectMetaData(ObjectMetaData encodedData, UINT32& objId, UINT32& objTypeId, bool& isBaseClass);

		/** Returns true if the provided encoded meta data represents object meta data. */
		static bool isObjectMetaData(UINT32 encodedData);

		/**
		 * Encodes an object identifier and meta-data into 4 bytes. 
		 *
		 * @param[in]	objId	   	Unique ID of the object instance. This can be a maximum of 30 bits, as two bits are reserved.
		 * @param[in]	isBaseClass	true if this object is base class (that is, just a part of a larger object).
		 * @return		Encoded object id and other meta-data.
		 */
		static UINT32 encodeObjectMetaData(UINT32 objId,  bool isBaseClass);

		/** Decode meta field that was encoded using encodeObjectMetaData(UINT32, bool). */
		static void decodeObjectMetaData(UINT32 encodedData, UINT32& objId, bool& isBaseClass);

		Map<UINT32, ObjectToDecode> mDecodeObjectMap;
		Vector<ObjectToEncode> mObjectsToEncode;
		UnorderedMap<void*, UINT32> mObjectAddrToId;
		UINT32 mLastUsedObjectId = 1;
		UINT32 mTotalBytesToRead = 0;
		UINT32 mNextProgressReport = REPORT_AFTER_BYTES;
		FrameAlloc* mAlloc = nullptr;
		Bitstream mBuffer;

		SerializationContext* mContext = nullptr;
		std::function<void(float)> mReportProgress = nullptr;
	};

	// TODO - Potential improvements:
	//  - I will probably want to extract a generalized Serializer class so we can re-use the code in text or other serializers
	//  - Encode does a chunk-based encode so that we don't need to know the buffer size in advance, and don't have to use
	//    a lot of memory for the buffer. Consider doing something similar for decode.
	//  - Add a simple encode method that doesn't require a callback, instead it calls the callback internally and creates
	//    the buffer internally.

	/** @} */
}
