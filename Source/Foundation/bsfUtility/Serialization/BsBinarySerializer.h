//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

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
	class BufferedBitstreamReader;
	class BufferedBitstreamWriter;
	struct RTTIReflectableFieldBase;
	struct RTTIReflectablePtrFieldBase;
	struct SerializationContext;

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
		 * @param[in]	shallow					Determines how to handle referenced objects. If true then references will
		 *										not be encoded and will be set to null. If false then references will be
		 *										encoded as well and restored upon decoding.
		 * @param[in]	context					Optional object that will be passed along to all serialized objects through
		 *										their serialization callbacks. Can be used for controlling serialization,
		 *										maintaining state or sharing information between objects during
		 *										serialization.
		 */
		void encode(IReflectable* object, const SPtr<DataStream>& stream, bool shallow = false, SerializationContext* context = nullptr);

		/**
		 * Decodes an object from binary data.
		 *
		 * @param[in]	stream  	Stream containing the binary data to decode.
		 * @param[in]	dataLength	Length of the data in bytes. If zero, all the data from the stream will be read.
		 * @param[in]	context		Optional object that will be passed along to all serialized objects through
		 *							their deserialization callbacks. Can be used for controlling deserialization,
		 *							maintaining state or sharing information between objects during deserialization.
		 * @param[in]	progress	Optional callback that will occassionally trigger, reporting the current progress
		 *							of the operation. The reported value is in range [0, 1].
		 *
		 * @note
		 * Child elements are guaranteed to be fully deserialized before their parents, except for fields marked with WeakRef flag.
		 */
		SPtr<IReflectable> decode(const SPtr<DataStream>& stream, UINT32 dataLength, SerializationContext* context = nullptr,
			std::function<void(float)> progress = nullptr);
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
			ObjectToEncode(UINT32 _objectId, const SPtr<IReflectable>& _object)
				:objectId(_objectId), object(_object)
			{ }

			UINT32 objectId;
			SPtr<IReflectable> object;
		};

		struct ObjectToDecode
		{
			ObjectToDecode(const SPtr<IReflectable>& _object, size_t offset = 0)
				:object(_object), offset(offset)
			{ }

			SPtr<IReflectable> object;
			bool isDecoded = false;
			bool decodeInProgress = false; // Used for error reporting circular references
			size_t offset;
		};

		/** Encodes a single IReflectable object. */
		bool encodeEntry(IReflectable* object, UINT32 objectId, BufferedBitstreamWriter& stream, bool shallow);

		/**	Decodes a single IReflectable object. */
		bool decodeEntry(BufferedBitstreamReader& stream, size_t dataLength, const SPtr<IReflectable>& output);

		/**	Helper method for encoding a complex object and writing its data to a stream. */
		bool complexTypeToStream(IReflectable* object, BufferedBitstreamWriter& stream, bool shallow);

		/**	Finds an existing, or creates a unique unique identifier for the specified object. */
		UINT32 findOrCreatePersistentId(IReflectable* object);

		/**
		 * Finds or creates an id for the provided object and returns it. And it adds the object to a list of objects that
		 * need to be encoded, if it's not already there.
		 */
		UINT32 registerObjectPtr(SPtr<IReflectable> object);

		/** Encodes data required for representing a serialized field, into 4 bytes. */
		static UINT32 encodeFieldMetaData(UINT16 id, UINT8 size, bool array,
			SerializableFieldType type, bool hasDynamicSize, bool terminator);

		/** Decode meta field that was encoded using encodeFieldMetaData().*/
		static void decodeFieldMetaData(UINT32 encodedData, UINT16& id, UINT8& size, bool& array,
			SerializableFieldType& type, bool& hasDynamicSize, bool& terminator);

		/**
		 * Encodes data required for representing an object identifier, into 8 bytes.
		 *
		 * @param[in]	objId	   	Unique ID of the object instance.
		 * @param[in]	objTypeId  	Unique ID of the object type.
		 * @param[in]	isBaseClass	true if this object is base class (that is, just a part of a larger object).
		 *
		 * @note		Id can be a maximum of 30 bits, as two bits are reserved.
		 */
		static ObjectMetaData encodeObjectMetaData(UINT32 objId, UINT32 objTypeId, bool isBaseClass);

		/** Decode meta field that was encoded using encodeObjectMetaData. */
		static void decodeObjectMetaData(ObjectMetaData encodedData, UINT32& objId, UINT32& objTypeId, bool& isBaseClass);

		/** Returns true if the provided encoded meta data represents object meta data. */
		static bool isObjectMetaData(UINT32 encodedData);

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

		static constexpr int META_SIZE = 4; // Meta field size
		static constexpr int NUM_ELEM_FIELD_SIZE = 4; // Size of the field storing number of array elements
		static constexpr int COMPLEX_TYPE_FIELD_SIZE = 4; // Size of the field storing the size of a child complex type
		static constexpr int DATA_BLOCK_TYPE_FIELD_SIZE = 4;
	};

	// TODO - Potential improvements:
	//  - I will probably want to extract a generalized Serializer class so we can re-use the code in text or other serializers
	//  - Encode does a chunk-based encode so that we don't need to know the buffer size in advance, and don't have to use
	//    a lot of memory for the buffer. Consider doing something similar for decode.
	//  - Add a simple encode method that doesn't require a callback, instead it calls the callback internally and creates
	//    the buffer internally.

	/** @} */
}
