//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Allocators/BsMemoryAllocator.h"
#include "Prerequisites/BsFwdDeclUtil.h" // For TIDs
#include "Prerequisites/BsTypes.h" // For u32
#include "Utility/BsBitstream.h"
#include "Utility/BsBitLength.h"

#include <limits>
#include <type_traits> // For std::is_pod
#include <utility> // For std::pair
#include <vector>

namespace bs
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
		VarInt = 1 << 3
	};

	typedef Flags<RTTIFieldFlag> RTTIFieldFlags;
	BS_FLAGS_OPERATORS(RTTIFieldFlag)

	/** Provides various optional information regarding a RTTI field. */
	struct BS_UTILITY_EXPORT RTTIFieldInfo
	{
		RTTIFieldFlags Flags;

		RTTIFieldInfo() = default;

		RTTIFieldInfo(RTTIFieldFlags flags)
			: Flags(flags)
		{}

		static RTTIFieldInfo DEFAULT;
	};

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
	struct RTTIPlainType
	{
		static_assert(std::is_pod<T>::value, "Provided type isn't plain-old-data. You need to specialize RTTIPlainType template in order to serialize this type. "
											 " (Or call BS_ALLOW_MEMCPY_SERIALIZATION(type) macro if you are sure the type can be properly serialized using just memcpy.)");

		enum
		{
			id = 0 /**< Unique id for the serializable type. */
		};

		enum
		{
			hasDynamicSize = 0 /**< 0 (Object has static size less than 255 bytes, for example int) or 1 (Dynamic size with no size restriction, for example string) */
		};

		/**
		 * Serializes the provided object into the provided stream and advances the stream cursor. Returns the number of bytes written. If @p compress is true
		 * the serialization system is allowed to compress the data into bits (e.g. a boolean can be represented via a single bit), otherwise it is guaranteed
		 * the written data will be aligned to byte boundaries. @p fieldInfo can be used for providing additional data, such as wanted method of serialization
		 * and compression.
		 **/
		static BitLength ToMemory(const T& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return stream.WriteBytes(data);
		}

		/**
		 * Deserializes a previously allocated object from the provided stream and advances the stream cursor. Return the number of bytes read. @p compress
		 * and @p fieldInfo should match the values provided when it was originally encoded using toMemory(). See toMemory() for the description of those parameters.
		 */
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

	template <>
	struct RTTIPlainType<uint32_t>
	{
		enum
		{
			id = TID_UInt32
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const uint32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return stream.WriteBytes(data);
			else
				return stream.WriteVarInt(data);
		}

		static BitLength FromMemory(uint32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return stream.ReadBytes(data);
			else
				return stream.ReadVarInt(data);
		}

		static BitLength GetSize(const uint32_t& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return sizeof(data);
			else
			{
				u8 buffer[5];
				return Bitwise::EncodeVarInt(data, buffer);
			}
		}
	};

	template <>
	struct RTTIPlainType<int32_t>
	{
		enum
		{
			id = TID_Int32
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const int32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return stream.WriteBytes(data);
			else
				return stream.WriteVarInt(data);
		}

		static BitLength FromMemory(int32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return stream.ReadBytes(data);
			else
				return stream.ReadVarInt(data);
		}

		static BitLength GetSize(const int32_t& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if(!compress)
				return sizeof(data);
			else
			{
				u8 buffer[5];
				return Bitwise::EncodeVarInt(data, buffer);
			}
		}
	};

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
			assert(size.Bits == 0);

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

	/** Helper for checking for existance of rttiEnumFields method on a class. */
	template <class T>
	struct has_rttiEnumFields
	{
		struct dummy
		{};

		template <typename C, typename P>
		static auto Test(P* p) -> decltype(std::declval<C>().RttiEnumFields(*p), std::true_type());

		template <typename, typename>
		static std::false_type Test(...);

		typedef decltype(Test<T, dummy>(nullptr)) type;
		static const bool kValue = std::is_same<std::true_type, decltype(Test<T, dummy>(nullptr))>::value;
	};

	/**
	 * Notify the RTTI system that the specified type may be serialized just by using a memcpy.
	 *
	 * @note	Internally this creates a basic RTTIPlainType<T> specialization for the type.
	 *
	 * @see		RTTIPlainType<T>
	 */
#define BS_ALLOW_MEMCPY_SERIALIZATION(type)                                                                           \
	static_assert(std::is_trivially_copyable<type>() == true, #type " is not trivially copyable");                    \
	template <>                                                                                                       \
	struct RTTIPlainType<type>                                                                                        \
	{                                                                                                                 \
		enum                                                                                                          \
		{                                                                                                             \
			id = 0                                                                                                    \
		};                                                                                                            \
		enum                                                                                                          \
		{                                                                                                             \
			hasDynamicSize = 0                                                                                        \
		};                                                                                                            \
		static BitLength ToMemory(const type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress) \
		{                                                                                                             \
			return stream.WriteBytes(data);                                                                           \
		}                                                                                                             \
		static BitLength FromMemory(type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)     \
		{                                                                                                             \
			return stream.ReadBytes(data);                                                                            \
		}                                                                                                             \
		static BitLength GetSize(const type& data, const RTTIFieldInfo& fieldInfo, bool compress)                     \
		{                                                                                                             \
			return sizeof(type);                                                                                      \
		}                                                                                                             \
	};

	/** @} */
	/** @} */
} // namespace bs
