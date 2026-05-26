//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <utility>
#include "B3DUtilityPrerequisites.h"
#include "Serialization/B3DSerializedObject.h"
#include "Reflection/B3DRTTIField.h"
#include "Reflection/B3DRTTIType.h"
#include "Utility/B3DBitstream.h"

namespace b3d
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
	struct RTTIOperationContext;

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
	class B3D_EXPORT BinarySerializer
	{
	public:
		BinarySerializer();

		/**
		 * Encodes all serializable fields provided by @p object into a binary format.
		 *
		 * @param	object					Object to encode into binary format.
		 * @param	stream					Stream into which to output the encoded data. The stream must own its memory
		 *									buffer so it may grow as required during encoding, or your must guarantee
		 *									the stream is of adequate size otherwise.
		 * @param	context					Optional object that will be passed along to all serialized objects through
		 *									RTTI operation notify methods. Can be used for controlling serialization,
		 *									maintaining state or sharing information between objects during serialization.
		 * @param	flags					Flags used for controlling serialization.
		 */
		void Encode(IReflectable* object, const TShared<DataStream>& stream, RTTIOperationContext& context, BinarySerializerFlags flags = BinarySerializerFlag::None);

		/** Overload of Encode(IReflectable*, const TShared<DataStream>&, BinarySerializerFlags, RTTIOperationContext&) that uses default constructed context. */
		void Encode(IReflectable* object, const TShared<DataStream>& stream, BinarySerializerFlags flags = BinarySerializerFlag::None);

		/**
		 * Decodes an object from binary data.
		 *
		 * @param	stream  	Stream containing the binary data to decode.
		 * @param	dataLength	Length of the data in bytes. If zero, all the data from the stream will be read.
		 * @param	context		Optional object that will be passed along to all serialized objects through
		 *						their deserialization callbacks. Can be used for controlling deserialization,
		 *						maintaining state or sharing information between objects during deserialization.
		 * @param	progress	Optional callback that will occasionally trigger, reporting the current progress
		 *						of the operation. The reported value is in range [0, 1].
		 * @param	schema		RTTI schema that contains information about types as they were when the data was
		 *						originally serialized. Schema is only used (and required) if BinarySerializerFlag::NoMeta
		 *						is set,	otherwise this information is read directly	from the encoded data.
		 *
		 * @note
		 * Child elements are guaranteed to be fully deserialized before their parents, except for fields marked with WeakRef flag.
		 */
		TShared<IReflectable> Decode(const TShared<DataStream>& stream, u32 dataLength, RTTIOperationContext& context, BinarySerializerFlags flags = BinarySerializerFlag::None, Function<void(float)> progress = nullptr, TShared<RTTISchema> schema = nullptr);

		/**
		 * Overload of Decode(const TShared<DataStream>&, u32, RTTIOperationContext, BinarySerializerFlags, Function<void(float)>, TShared<RTTISchema>) that uses
		 * default constructed context.
		 */
		TShared<IReflectable> Decode(const TShared<DataStream>& stream, u32 dataLength, BinarySerializerFlags flags = BinarySerializerFlag::None, std::function<void(float)> progress = nullptr, TShared<RTTISchema> schema = nullptr);
	private:
		/** Determines how many bytes need to be read before the progress report callback is triggered. */
		static constexpr u32 kReportAfterBytes = 32768;

		/** Determines the size of the temporary write buffer. Should be greater than FLUSH_AFTER_BYTES. */
		static constexpr u32 kWriteBufferSize = 4096;

		/** Determines how often to flush from the local buffer into the output stream, when writing. */
		static constexpr u32 kFlushAfterBytes = (u32)(kWriteBufferSize * 0.75f);

		/**
		 * Minimum amount of bytes to preload into the temporary read buffer per chunk. Also the size of each read-ahead
		 * issued while decoding, so it should be large enough to amortize per-read overhead.
		 */
		static constexpr u32 kReadPreloadChunkBytes = 256 * 1024;

		/** Size the buffered read data is allowed to grow to before older buffered data is discarded. */
		static constexpr u32 kReadMaxBufferBytes = 1024 * 1024;

		u32 mTotalBytesToRead = 0;
		u32 mNextProgressReport = kReportAfterBytes;
		FrameAllocator* mAlloc = nullptr;
		Bitstream mBuffer;

		RTTIOperationContext* mContext = nullptr;
		std::function<void(float)> mReportProgress = nullptr;
	};

	// TODO - Potential improvements:
	//  - I will probably want to extract a generalized Serializer class so we can re-use the code in text or other serializers
	//  - Encode does a chunk-based encode so that we don't need to know the buffer size in advance, and don't have to use
	//    a lot of memory for the buffer. Consider doing something similar for decode.
	//  - Add a simple encode method that doesn't require a callback, instead it calls the callback internally and creates
	//    the buffer internally.

	/** @} */
} // namespace b3d
