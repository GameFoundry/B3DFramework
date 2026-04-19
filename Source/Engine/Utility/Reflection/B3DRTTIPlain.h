//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Allocators/B3DMemoryAllocator.h"
#include "Prerequisites/B3DFwdDeclUtil.h" // For TIDs
#include "Prerequisites/B3DTypes.h" // For u32
#include "Utility/B3DBitstream.h"
#include "Utility/B3DBitLength.h"

#include <limits>
#include <type_traits> // For std::is_pod
#include <utility> // For std::pair
#include <vector>

namespace b3d
{
	struct RTTIFieldInfo;

	/** @addtogroup Utility
	 *  @{
	 */

	/** @addtogroup RTTI
	 *  @{
	 */

	/** Various flags you can assign to RTTI fields. */
	enum class RTTIFieldFlag
	{
		/**
		 * This flag is only used on field types of ReflectablePtr type, and it is used
		 * to solve circular references. Circular references cause an issue when deserializing,
		 * as the algorithm doesn't know which object to deserialize first. By making one of
		 * the references weak, you tell the algorithm that it doesn't have to guarantee
		 * the object will be fully deserialized before being assigned to the field.
		 *
		 * In short: If you make a reference weak, when "set" method of that field is called,
		 * it is not guaranteed the value provided is fully initialized, so you should not access any of its
		 * data until deserialization is fully complete. You only need to use this flag if the RTTI system
		 * complains that is has found a circular reference.
		 */
		WeakRef = 1 << 0,
		/**
		 * This flags signals various systems that the flagged field should not be searched when looking for
		 * object references. This normally means the value of this field will no be retrieved during reference
		 * searches but it will likely still be retrieved during other operations (for example serialization).
		 * This is used as an optimization to avoid retrieving values of potentially very expensive fields that
		 * would not contribute to the reference search anyway. Whether or not a field contributes to the reference
		 * search depends on the search and should be handled on a case by case basis.
		 */
		SkipInReferenceSearch = 1 << 1,
		/**
		 * Lets the replication system know that this field should be monitored for changes and replicated across the
		 * network when changes are detected.
		 */
		Replicate = 1 << 2,
		/**
		 * If true, the integer will be encoded as a var-int during networking operations, in order to reduce its
		 * size. Not relevant for non-integers.
		 */
		VarInt = 1 << 3, 
		/**
		 * When set, field will be skipped if performing delta generation through IDeltaHandler interface.
		 * If two objects have different values in this field, it will not be recorded as a delta change.
		 */
		SkipInDeltaCompare = 1 << 4,
		/**
		 * Similar to SkipInDeltaCompare, but extends to concept so a field is also skipped when a complete
		 * copy of the object is being serialized (i.e. not necessarily limited to IDeltaHandler).
		 */
		SkipInDeltaCopy = 1 << 5,
	};

	typedef Flags<RTTIFieldFlag> RTTIFieldFlags;
	B3D_FLAGS_OPERATORS(RTTIFieldFlag)

	/** Provides various optional information regarding a RTTI field. */
	struct B3D_EXPORT RTTIFieldInfo
	{
		RTTIFieldFlags Flags;

		RTTIFieldInfo() = default;

		RTTIFieldInfo(RTTIFieldFlags flags)
			: Flags(flags)
		{}

		static RTTIFieldInfo DEFAULT;
	};

	/** @} */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * Default implementation of RTTIPlainType for a particular type T. Separated from RTTIPlainType primarily so we can specialize this for entire category of
	 * objects (such as enums) using std::enable_if. Default implementation causes a compile error if used, because user must explicitly specialize it.
	 */
	template<class T, typename = void>
	struct TRTTIPlainTypeImplementation
	{
		/**
		 * Serializes the provided object into the provided stream and advances the stream cursor. Returns the number of bytes written. If @p compress is true
		 * the serialization system is allowed to compress the data into bits (e.g. a boolean can be represented via a single bit), otherwise it is guaranteed
		 * the written data will be aligned to byte boundaries. @p fieldInfo can be used for providing additional data, such as wanted method of serialization
		 * and compression.
		 **/
		static BitLength ToMemory(const T& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static_assert(std::is_pod<T>::value, "Provided type isn't plain-old-data. You need to specialize RTTIPlainType template in order to serialize this type. "
												 " (Or call B3D_ALLOW_MEMCPY_SERIALIZATION(type) macro if you are sure the type can be properly serialized using just memcpy.)");

			return stream.WriteBytes(data);
		}

		/**
		 * Deserializes a previously allocated object from the provided stream and advances the stream cursor. Return the number of bytes read. @p compress
		 * and @p fieldInfo should match the values provided when it was originally encoded using toMemory(). See toMemory() for the description of those parameters.
		 */
		static BitLength FromMemory(T& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static_assert(std::is_pod<T>::value, "Provided type isn't plain-old-data. You need to specialize RTTIPlainType template in order to serialize this type. "
												 " (Or call B3D_ALLOW_MEMCPY_SERIALIZATION(type) macro if you are sure the type can be properly serialized using just memcpy.)");

			return stream.ReadBytes(data);
		}

		/** Returns the size of the provided object. (Works for both static and dynamic size types) */
		static BitLength GetSize(const T& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static_assert(std::is_pod<T>::value, "Provided type isn't plain-old-data. You need to specialize RTTIPlainType template in order to serialize this type. "
												 " (Or call B3D_ALLOW_MEMCPY_SERIALIZATION(type) macro if you are sure the type can be properly serialized using just memcpy.)");

			return sizeof(T);
		}
	};

	/** Specializes RTTIPlainType for all enum types. */
	template<class T>
	struct TRTTIPlainTypeImplementation<T, std::enable_if_t<std::is_enum_v<T>>>
	{
		enum
		{
			id = 0 // Should be `id = RTTIPlainType<std::underlying_type_t<T>>::id`, but keeping the old behaviour for now
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const T& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return stream.WriteBytes(data);
		}

		static BitLength FromMemory(T& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return stream.ReadBytes(data);
		}
		/** Returns the size of the provided object. (Works for both static and dynamic size types) */
		static BitLength GetSize(const T& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(T);
		}
	};

	/** @} */

	/** @addtogroup RTTI
	 *  @{
	 */

	/**
	 * Template that you may specialize with a class if you want to provide simple serialization for it.
	 *
	 * Any type that uses the "plain" field in the RTTI system must specialize this class.
	 *
	 * @note
	 * Normally you will want to implement IReflectable interface if you want to provide serialization
	 * as that interface properly handles versioning, nested objects, pointer handling and more.
	 *
	 * @note
	 * This class is useful for types you can easily serialize using a memcpy (built-in types like int/float/etc), or
	 * types you cannot modify so they implement IReflectable interface (like std::string or std::vector).
	 *
	 * @see		RTTITypeBase
	 * @see		RTTIField
	 */
	template <class T>
	struct RTTIPlainType : TRTTIPlainTypeImplementation<T>
	{ };

	/** @} */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/** Checks has the user specialized RTTIPlainType<T> for T. */
	template <typename T, typename = void>
	struct B3DHasRTTIPlainTypeSpecialization : std::false_type {};

	template <typename T>
	struct B3DHasRTTIPlainTypeSpecialization<T, std::void_t<decltype(RTTIPlainType<T>::id)>> : std::true_type {};

	template <>
	struct RTTIPlainType<bool>
	{
		enum
		{
			id = TID_Bool
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const bool& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return stream.WriteBytes(data);
			else
			{
				uint8_t bit = data ? 1 : 0;
				stream.WriteBits(&bit, 1);

				return BitLength(0, 1);
			}
		}

		static BitLength FromMemory(bool& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return stream.ReadBytes(data);
			else
			{
				uint8_t bit = 0;
				stream.ReadBits(&bit, 1);

				data = bit;
				return BitLength(0, 1);
			}
		}

		static BitLength GetSize(const bool& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return sizeof(data);
			else
				return BitLength(0, 1);
		}
	};

	/** @} */

	/** @addtogroup RTTI
	 *  @{
	 */

	/**
	 * Helper method when serializing known data types that have valid
	 * RTTIPlainType specialization.
	 *
	 * Returns the size of the element when serialized.
	 */
	template <class ElemType>
	BitLength B3DRTTISize(const ElemType& data, bool compress = false)
	{
		return RTTIPlainType<ElemType>::GetSize(data, RTTIFieldInfo(), compress);
	}

	/**
	 * Helper method when serializing known data types that have valid RTTIPlainType specialization. The data will be
	 * written to the provided stream and its write cursor advanced.
	 */
	template <class ElemType>
	BitLength B3DRTTIWrite(const ElemType& data, Bitstream& stream, bool compress = false)
	{
		return RTTIPlainType<ElemType>::ToMemory(data, stream, RTTIFieldInfo(), compress);
	}

	/**
	 * Helper method when serializing known data types that have valid RTTIPlainType specialization. The data will be
	 * read from the provided stream and its read cursor advanced.
	 */
	template <class ElemType>
	BitLength B3DRTTIRead(ElemType& data, Bitstream& stream, bool compress = false)
	{
		return RTTIPlainType<ElemType>::FromMemory(data, stream, RTTIFieldInfo(), compress);
	}

	/**
	 * Writes a set of data to the stream through the user provided function @p t and then
	 * writes the size of that data as a header. The size header is written as a 32-bit integer
	 * right before the data written by @p t. The size value will include the size of the data
	 * from @p t as well as the size of the header itself (32-bits). The size of the data is
	 * determined by the return value from @p t. Returns the size written.
	 */
	template <class T, class P>
	BitLength B3DRTTIWriteWithSizeHeader(Bitstream& stream, const T& data, bool compress, P p)
	{
		if(compress)
		{
			BitLength size = B3DRTTISize(data);
			uint64_t headerSize = stream.WriteVarInt(size.Bytes);
			headerSize += stream.WriteBits(&size.Bits, 3);

			size += BitLength::FromBits(headerSize);
			p();

			return size;
		}
		else
		{
			uint64_t sizePos = stream.Tell();

			BitLength size = 0;
			stream.WriteBytes(size.Bytes);

			size = p() + sizeof(uint32_t);
			B3D_ASSERT(size.Bits == 0);

			stream.Seek(sizePos);
			stream.WriteBytes(size.Bytes);
			stream.SkipBytes(size.Bytes - sizeof(uint32_t));

			return size;
		}
	}

	/**
	 * Reads the size header that was encoded with B3DRTTIWriteWithSizeHeader. @p will contain
	 * the size value read from the stream, while the return value represents the number of bits
	 * read from the header itself (e.g. 4 bytes for uncompressed size).
	 */
	template <class T = Bitstream>
	BitLength B3DRTTIReadSizeHeader(T& stream, bool compress, BitLength& size)
	{
		if(compress)
		{
			uint64_t headerSizeBits = stream.ReadVarInt(size.Bytes);
			size.Bits = 0;
			headerSizeBits += stream.ReadBits(&size.Bits, 3);

			BitLength headerSize = BitLength::FromBits(headerSizeBits);
			size += headerSize;

			return headerSize;
		}
		else
		{
			uint32_t sizeBytes = stream.ReadBytes(size.Bytes);
			size.Bits = 0;

			return sizeBytes;
		}
	}

	/** Increments the provided size with the required size of the header. */
	inline void B3DRTTIAddHeaderSize(BitLength& size, bool compress)
	{
		if(compress)
		{
			uint8_t bytes[5];
			uint32_t numBytes = Bitwise::EncodeVarInt(size.Bytes, bytes);

			size += BitLength(numBytes, 3);
		}
		else
			size += sizeof(size.Bytes);
	}

	/** @} */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/** Helper that calls B3DRTTIWrite() using operator(), and accumulates written size. */
	struct RTTIWriteProcessor
	{
		RTTIWriteProcessor(Bitstream& stream) : mStream(stream) {}
		template <class T> void operator()(T&& value) { mSize += B3DRTTIWrite(value, mStream); }
		BitLength GetSize() const { return mSize; }

	private:
		Bitstream& mStream;
		BitLength mSize;
	};

	/** Helper that calls B3DRTTIRead() using operator(). */
	struct RTTIReadProcessor
	{
		RTTIReadProcessor(Bitstream& stream) : mStream(stream) {} 
		template <class T> void operator()(T&& value) { B3DRTTIRead(value, mStream); }

	private:
		Bitstream& mStream;
	};

	/** Helper that calls B3DRTTISize() using operator(). */
	struct RTTISizeProcessor
	{
		RTTISizeProcessor() = default;
		template <class T> void operator()(T&& value) { mSize += B3DRTTISize(value); }
		BitLength GetSize() const { return mSize; }

	private:
		BitLength mSize;
	};

	/** @} */

	/** @addtogroup RTTI
	 *  @{
	 */

	/**
	 * Helper that allows you to construct a RTTIPlainType<T> specialization more easily. Your specialization needs to inherit this type, and
	 * implement a RTTIEnumerateFields(T& data, Processor processor, u8 version) method. The method should then call
	 * `processor(data.field);` for every field to be serialized. If your type supports versioning, you should check
	 * the version field and serialize/deserialize the fields accordingly.
	 */
	template <class SerializedObjectType, u32 SerializedObjectTypeId, u8 Version = 255, u32 HasDynamicSize = 1>
	struct RTTIPlainTypeHelper
	{
		enum { id = SerializedObjectTypeId };
		enum { hasDynamicSize = HasDynamicSize };

		using EnumerateFieldsType = RTTIPlainType<SerializedObjectType>;

		static constexpr u8 kNoVersioning = 255;

		static BitLength ToMemory(const SerializedObjectType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				RTTIWriteProcessor writeProcessor(stream);
				if constexpr(Version != kNoVersioning)
				{
					size += B3DRTTIWrite(Version, stream);
					EnumerateFieldsType::RTTIEnumerateFields(const_cast<SerializedObjectType&>(data), writeProcessor, Version);
				}
				else
					EnumerateFieldsType::RTTIEnumerateFields(const_cast<SerializedObjectType&>(data), writeProcessor);

				size += writeProcessor.GetSize();
				return size; });
		}

		static BitLength FromMemory(SerializedObjectType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			RTTIReadProcessor readProcessor(stream);
			if constexpr(Version != kNoVersioning)
			{
				u8 version;
				B3DRTTIRead(version, stream);

				B3D_ASSERT(version <= Version);
				EnumerateFieldsType::RTTIEnumerateFields(data, readProcessor, version);
			}
			else
				EnumerateFieldsType::RTTIEnumerateFields(data, readProcessor);

			return size;
		}

		static BitLength GetSize(const SerializedObjectType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = 0;

			if constexpr(Version != kNoVersioning)
				dataSize += sizeof(u8);

			RTTISizeProcessor sizeProcessor;
			if constexpr(Version != kNoVersioning)
				EnumerateFieldsType::RTTIEnumerateFields(const_cast<SerializedObjectType&>(data), sizeProcessor, Version);
			else
				EnumerateFieldsType::RTTIEnumerateFields(const_cast<SerializedObjectType&>(data), sizeProcessor);

			dataSize += sizeProcessor.GetSize();

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * Notify the RTTI system that the specified type may be serialized just by using a memcpy.
	 *
	 * @note	Internally this creates a basic RTTIPlainType<T> specialization for the type.
	 *
	 * @see		RTTIPlainType<T>
	 */
#define B3D_ALLOW_MEMCPY_SERIALIZATION(Type, TypeId)																  \
	static_assert(std::is_trivially_copyable<Type>() == true, #Type " is not trivially copyable");                    \
	static_assert(sizeof(Type) <= 256, #Type " doesn't fit. Use memcpy variant with size header.");					  \
	template <>                                                                                                       \
	struct RTTIPlainType<Type>                                                                                        \
	{                                                                                                                 \
		enum                                                                                                          \
		{                                                                                                             \
			id = TypeId                                                                                               \
		};                                                                                                            \
		enum                                                                                                          \
		{                                                                                                             \
			hasDynamicSize = 0                                                                                        \
		};                                                                                                            \
		static BitLength ToMemory(const Type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress) \
		{                                                                                                             \
			return stream.WriteBytes(data);                                                                           \
		}                                                                                                             \
		static BitLength FromMemory(Type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)     \
		{                                                                                                             \
			return stream.ReadBytes(data);                                                                            \
		}                                                                                                             \
		static BitLength GetSize(const Type& data, const RTTIFieldInfo& fieldInfo, bool compress)                     \
		{                                                                                                             \
			return sizeof(Type);                                                                                      \
		}                                                                                                             \
	};

#define B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_SIZE_HEADER(Type, Id)                                                     \
	static_assert(std::is_trivially_copyable<Type>() == true, #Type " is not trivially copyable");                    \
	template <>                                                                                                       \
	struct RTTIPlainType<Type>                                                                                        \
	{                                                                                                                 \
		enum                                                                                                          \
		{                                                                                                             \
			id = Id                                                                                                   \
		};                                                                                                            \
		enum                                                                                                          \
		{                                                                                                             \
			hasDynamicSize = 1                                                                                        \
		};                                                                                                            \
		static constexpr u8 kVersion = 0;                                                                             \
                                                                                                                      \
		static BitLength ToMemory(const Type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress) \
		{                                                                                                             \
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]() {							  \
					BitLength size = stream.WriteBytes(kVersion);													  \
					size += stream.WriteBytes(data);																  \
																													  \
					return size; });																				  \
		}                                                                                                             \
                                                                                                                      \
		static BitLength FromMemory(Type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)     \
		{                                                                                                             \
			BitLength size;                                                                                           \
			B3DRTTIReadSizeHeader(stream, compress, size);                                                            \
                                                                                                                      \
			u8 version;                                                                                               \
			stream.ReadBytes(version);                                                                                \
			if(!B3D_ENSURE(version == 0))                                                                             \
				return 0;                                                                                             \
                                                                                                                      \
			stream.ReadBytes(data);                                                                                   \
                                                                                                                      \
			return size;                                                                                              \
		}                                                                                                             \
                                                                                                                      \
		static BitLength GetSize(const Type& data, const RTTIFieldInfo& fieldInfo, bool compress)                     \
		{                                                                                                             \
			BitLength dataSize = sizeof(kVersion) + sizeof(data);                                                     \
			B3DRTTIAddHeaderSize(dataSize, compress);                                                                 \
                                                                                                                      \
			return dataSize;                                                                                          \
		}                                                                                                             \
	};

#define B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_VAR_INT(Type, Id)                                                              \
	template <>                                                                                                       \
	struct RTTIPlainType<Type>                                                                                        \
	{                                                                                                                 \
		enum                                                                                                          \
		{                                                                                                             \
			id = Id                                                                                                   \
		};                                                                                                            \
                                                                                                                      \
		enum                                                                                                          \
		{                                                                                                             \
			hasDynamicSize = 0                                                                                        \
		};                                                                                                            \
                                                                                                                      \
		static BitLength ToMemory(const Type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress) \
		{                                                                                                             \
			if(!compress)                                                                                             \
				return stream.WriteBytes(data);                                                                       \
			else                                                                                                      \
				return stream.WriteVarInt(data);                                                                      \
		}                                                                                                             \
                                                                                                                      \
		static BitLength FromMemory(Type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)     \
		{                                                                                                             \
			if(!compress)                                                                                             \
				return stream.ReadBytes(data);                                                                        \
			else                                                                                                      \
				return stream.ReadVarInt(data);                                                                       \
		}                                                                                                             \
                                                                                                                      \
		static BitLength GetSize(const Type& data, const RTTIFieldInfo& fieldInfo, bool compress)                     \
		{                                                                                                             \
			if(!compress)                                                                                             \
				return sizeof(data);                                                                                  \
			else                                                                                                      \
			{                                                                                                         \
				u8 buffer[sizeof(Type) + (sizeof(Type) / 4)];                                                         \
				return Bitwise::EncodeVarInt(data, buffer);                                                           \
			}                                                                                                         \
		}                                                                                                             \
	};

	B3D_ALLOW_MEMCPY_SERIALIZATION(float, TID_Float)
	B3D_ALLOW_MEMCPY_SERIALIZATION(double, TID_Double)
	B3D_ALLOW_MEMCPY_SERIALIZATION(u8, TID_UInt8)
	B3D_ALLOW_MEMCPY_SERIALIZATION(i8, TID_Int8)
	B3D_ALLOW_MEMCPY_SERIALIZATION(u16, TID_UInt16)
	B3D_ALLOW_MEMCPY_SERIALIZATION(i16, TID_Int16)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_VAR_INT(u32, TID_UInt32)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_VAR_INT(i32, TID_Int32)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_VAR_INT(u64, TID_UInt64)
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_VAR_INT(i64, TID_Int64)

	/** @} */
	/** @} */
} // namespace b3d
