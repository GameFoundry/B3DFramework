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
	B3D_FLAGS_OPERATORS(BinarySerializerFlag)

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
	class B3D_UTILITY_EXPORT BinarySerializer
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
		void Encode(IReflectable* object, const SPtr<DataStream>& stream, BinarySerializerFlags flags = BinarySerializerFlag::None, SerializationContext* context = nullptr);

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
		SPtr<IReflectable> Decode(const SPtr<DataStream>& stream, u32 dataLength, BinarySerializerFlags flags = BinarySerializerFlag::None, SerializationContext* context = nullptr, std::function<void(float)> progress = nullptr, SPtr<RTTISchema> schema = nullptr);

	private:
		/** Determines how many bytes need to be read before the progress report callback is triggered. */
		static constexpr u32 kReportAfterBytes = 32768;

		/** Determines the size of the temporary write buffer. Should be greater than FLUSH_AFTER_BYTES. */
		static constexpr u32 kWriteBufferSize = 4096;

		/** Determines how often to flush from the local buffer into the output stream, when writing. */
		static constexpr u32 kFlushAfterBytes = (u32)(kWriteBufferSize * 0.75f);

		/** Determines the minimum amount of bytes to preload into the temporary buffer. */
		static constexpr u32 kPreloadChunkBytes = (u32)(kWriteBufferSize * 0.25f);

		struct ObjectMetaData
		{
			u32 ObjectMeta;
			u32 TypeId;
		};

		struct ObjectToSerialize
		{
			ObjectToSerialize(u32 objectId, SPtr<IReflectable> object)
				: ObjectId(objectId), Object(std::move(object))
			{}

			u32 ObjectId;
			SPtr<IReflectable> Object;
		};

		/**
		 * Serializes a single IReflectable object. Any pointers referencing other reflectable types will be registered in mObjectsToSerialize, and
		 * this method should also be called over all objects registered in that array.
		 *
		 * @param	object		Object to serialize.
		 * @param	objectId	Persistent ID of the object in the serialized data. See FindOrCreateReflectableObjectId().
		 * @param	stream		Stream into which to serialize the object.
		 * @param	flags		Flags used to control serialization.
		 * @return				True if successful, false otherwise.
		 */
		bool SerializeReflectableObject(IReflectable* object, u32 objectId, BufferedBitstreamWriter& stream, BinarySerializerFlags flags);

		/**
		 * Serializes an IReflectable inline as a field value. This is opposed to serializing a reflectable by pointer, which
		 * are stored later in the serialized data, and the field value only stores the object ID.
		 */
		bool SerializeReflectableObjectInline(IReflectable* object, BufferedBitstreamWriter& stream, BinarySerializerFlags flags);

		/**	Finds an existing, or creates a unique unique ID for the specified object. See RegisterChildObjectForSerialization. */
		u32 FindOrCreateReflectableObjectId(IReflectable* object);

		/**
		 * Adds the object to the list of objects that need to be serialized in a later stage. Assigns the object a unique ID
		 * or returns a previously assigned one, if the object was already registered. This ID will be stored when the object
		 * is serialized, and may be used for referencing the object in the serialized data.
		 */
		u32 RegisterReflectableObjectForSerialization(SPtr<IReflectable> object);

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

		Vector<ObjectToSerialize> mReflectableObjectsToSerialize;
		UnorderedMap<void*, u32> mReflectableObjectToID;
		u32 mLastUsedObjectId = 1;
		u32 mTotalBytesToRead = 0;
		u32 mNextProgressReport = kReportAfterBytes;
		FrameAllocator* mAlloc = nullptr;
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
} // namespace bs
