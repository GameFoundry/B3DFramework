//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Allocators/BsMemoryAllocator.h"
#include "Prerequisites/BsFwdDeclUtil.h"      // For TIDs
#include "Prerequisites/BsTypes.h"            // For UINT32
#include "Utility/BsBitstream.h"
#include "Utility/BsBitLength.h"

#include <limits>
#include <type_traits>          // For std::is_pod
#include <utility>              // For std::pair
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
		RTTIFieldFlags flags;

		RTTIFieldInfo() = default;

		RTTIFieldInfo(RTTIFieldFlags flags)
			:flags(flags)
		{ }

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
	template<class T>
	struct RTTIPlainType
	{
		static_assert(std::is_pod<T>::value,
			"Provided type isn't plain-old-data. You need to specialize RTTIPlainType template in order to serialize this type. "\
			" (Or call BS_ALLOW_MEMCPY_SERIALIZATION(type) macro if you are sure the type can be properly serialized using just memcpy.)");

		enum { id = 0 /**< Unique id for the serializable type. */ };
		enum { hasDynamicSize = 0 /**< 0 (Object has static size less than 255 bytes, for example int) or 1 (Dynamic size with no size restriction, for example string) */ };

		/**
		 * Serializes the provided object into the provided stream and advances the stream cursor. Returns the number of bytes written. If @p compress is true
		 * the serialization system is allowed to compress the data into bits (e.g. a boolean can be represented via a single bit), otherwise it is guaranteed
		 * the written data will be aligned to byte boundaries. @p fieldInfo can be used for providing additional data, such as wanted method of serialization
		 * and compression.
		 **/
		static BitLength toMemory(const T& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return stream.writeBytes(data);
		}

		/**
		 * Deserializes a previously allocated object from the provided stream and advances the stream cursor. Return the number of bytes read. @p compress
		 * and @p fieldInfo should match the values provided when it was originally encoded using toMemory(). See toMemory() for the description of those parameters.
		 */
		static BitLength fromMemory(T& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return stream.readBytes(data);
		}

		/** Returns the size of the provided object. (Works for both static and dynamic size types) */
		static BitLength getSize(const T& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(T);
		}
	};

	template<>
	struct RTTIPlainType<bool>
	{
		enum { id = TID_Bool };
		enum { hasDynamicSize = 0 };

		static BitLength toMemory(const bool& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return stream.writeBytes(data);
			else
			{
				uint8_t bit = data ? 1 : 0;
				stream.writeBits(&bit, 1);

				return BitLength(0, 1);
			}
		}

		static BitLength fromMemory(bool& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return stream.readBytes(data);
			else
			{
				uint8_t bit = 0;
				stream.readBits(&bit, 1);

				data = bit;
				return BitLength(0, 1);
			}
		}

		static BitLength getSize(const bool& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return sizeof(data);
			else
				return BitLength(0, 1);
		}
	};

	template<>
	struct RTTIPlainType<uint32_t>
	{
		enum { id = TID_UInt32 };
		enum { hasDynamicSize = 0 };

		static BitLength toMemory(const uint32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return stream.writeBytes(data);
			else
				return stream.writeVarInt(data);
		}

		static BitLength fromMemory(uint32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return stream.readBytes(data);
			else
				return stream.readVarInt(data);
		}

		static BitLength getSize(const uint32_t& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return sizeof(data);
			else
			{
				UINT8 buffer[5];
				return Bitwise::encodeVarInt(data, buffer);
			}
		}
	};

	template<>
	struct RTTIPlainType<int32_t>
	{
		enum { id = TID_Int32 };
		enum { hasDynamicSize = 0 };

		static BitLength toMemory(const int32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return stream.writeBytes(data);
			else
				return stream.writeVarInt(data);
		}

		static BitLength fromMemory(int32_t& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return stream.readBytes(data);
			else
				return stream.readVarInt(data);
		}

		static BitLength getSize(const int32_t& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			if (!compress)
				return sizeof(data);
			else
			{
				UINT8 buffer[5];
				return Bitwise::encodeVarInt(data, buffer);
			}
		}
	};

	/**
	 * Helper method when serializing known data types that have valid
	 * RTTIPlainType specialization.
	 *
	 * Returns the size of the element when serialized.
	 */
	template<class ElemType>
	BitLength rtti_size(const ElemType& data, bool compress = false)
	{
		return RTTIPlainType<ElemType>::getSize(data, RTTIFieldInfo(), compress);
	}

	/**
	 * Helper method when serializing known data types that have valid RTTIPlainType specialization. The data will be
	 * written to the provided stream and its write cursor advanced.
	 */
	template<class ElemType>
	BitLength rtti_write(const ElemType& data, Bitstream& stream, bool compress = false)
	{
		return RTTIPlainType<ElemType>::toMemory(data, stream, RTTIFieldInfo(), compress);
	}

	/**
	 * Helper method when serializing known data types that have valid RTTIPlainType specialization. The data will be
	 * read from the provided stream and its read cursor advanced.
	 */
	template<class ElemType>
	BitLength rtti_read(ElemType& data, Bitstream& stream, bool compress = false)
	{
		return RTTIPlainType<ElemType>::fromMemory(data, stream, RTTIFieldInfo(), compress);
	}

	/**
	 * Writes a set of data to the stream through the user provided function @p t and then
	 * writes the size of that data as a header. The size header is written as a 32-bit integer
	 * right before the data written by @p t. The size value will include the size of the data
	 * from @p t as well as the size of the header itself (32-bits). The size of the data is
	 * determined by the return value from @p t. Returns the size written.
	 */
	template<class T, class P>
	BitLength rtti_write_with_size_header(Bitstream& stream, const T& data, bool compress, P p)
	{
		if(compress)
		{
			BitLength size = rtti_size(data);
			uint64_t headerSize = stream.writeVarInt(size.bytes);
			headerSize += stream.writeBits(&size.bits, 3);

			size += BitLength::fromBits(headerSize);
			p();
			
			return size;
		}
		else
		{
			uint64_t sizePos = stream.tell();

			BitLength size = 0;
			stream.writeBytes(size.bytes);
			
			size = p() + sizeof(uint32_t);
			assert(size.bits == 0);

			stream.seek(sizePos);
			stream.writeBytes(size.bytes);
			stream.skipBytes(size.bytes - sizeof(uint32_t));

			return size;
		}
	}

	/**
	 * Reads the size header that was encoded with rtti_write_with_size_header. @p will contain
	 * the size value read from the stream, while the return value represents the number of bits
	 * read from the header itself (e.g. 4 bytes for uncompressed size).
	 */
	template<class T = Bitstream>
	BitLength rtti_read_size_header(T& stream, bool compress, BitLength& size)
	{
		if(compress)
		{
			uint64_t headerSizeBits = stream.readVarInt(size.bytes);
			size.bits = 0;
			headerSizeBits += stream.readBits(&size.bits, 3);

			BitLength headerSize = BitLength::fromBits(headerSizeBits);
			size += headerSize;

			return headerSize;
		}
		else
		{
			uint32_t sizeBytes = stream.readBytes(size.bytes);
			size.bits = 0;
			
			return sizeBytes;
		}
	}

	/** Increments the provided size with the required size of the header. */
	inline void rtti_add_header_size(BitLength& size, bool compress)
	{
		if (compress)
		{
			uint8_t bytes[5];
			uint32_t numBytes = Bitwise::encodeVarInt(size.bytes, bytes);

			size += BitLength(numBytes, 3);
		}
		else
			size += sizeof(size.bytes);
	}

	/** Helper for checking for existance of rttiEnumFields method on a class. */
	template <class T>
	struct has_rttiEnumFields
	{
		struct dummy {};

		template <typename C, typename P>
		static auto test(P* p) -> decltype(std::declval<C>().rttiEnumFields(*p), std::true_type());

		template <typename, typename>
		static std::false_type test(...);

		typedef decltype(test<T, dummy>(nullptr)) type;
		static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
	};

	/**
	 * Notify the RTTI system that the specified type may be serialized just by using a memcpy.
	 *
	 * @note	Internally this creates a basic RTTIPlainType<T> specialization for the type.
	 *
	 * @see		RTTIPlainType<T>
	 */
#define BS_ALLOW_MEMCPY_SERIALIZATION(type)																				\
	static_assert (std::is_trivially_copyable<type>()==true,															\
						#type " is not trivially copyable");															\
	template<> struct RTTIPlainType<type>																				\
	{	enum { id=0 }; enum { hasDynamicSize = 0 };																		\
		static BitLength toMemory(const type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)	\
		{ return stream.writeBytes(data); }																				\
		static BitLength fromMemory(type& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)		\
		{ return stream.readBytes(data); }																				\
		static BitLength getSize(const type& data, const RTTIFieldInfo& fieldInfo, bool compress)						\
		{ return sizeof(type); }																						\
	};

	/** @} */
	/** @} */
}
