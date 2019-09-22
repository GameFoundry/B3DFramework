//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"

namespace bs
{
	struct RTTIFieldInfo;
	
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	/**
	 * RTTIPlainType for std::vector.
	 *
	 * @see		RTTIPlainType
	 */
	template<class T> struct RTTIPlainType<std::vector<T, StdAlloc<T>>>
	{
		enum { id = TID_Vector }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const std::vector<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
					size += rtti_write(item, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(std::vector<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t numElements;
			rtti_read(numElements, stream);

			data.clear();
			for (uint32_t i = 0; i < numElements; i++)
			{
				T element;
				rtti_read(element, stream);

				data.push_back(element);
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const std::vector<T, StdAlloc<T>>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (const auto& item : data)
				dataSize += rtti_size(item);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::list.
	 *
	 * @see		RTTIPlainType
	 */
	template<class T> struct RTTIPlainType<std::list<T, StdAlloc<T>>>
	{
		enum { id = TID_List }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const std::list<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
					size += rtti_write(item, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(std::list<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size = 0;
			rtti_read(size, stream);

			uint32_t numElements;
			rtti_read(numElements, stream);

			for (uint32_t i = 0; i < numElements; i++)
			{
				T element;
				rtti_read(element, stream);

				data.push_back(element);
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const std::list<T, StdAlloc<T>>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (const auto& item : data)
				dataSize += rtti_size(item);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::set.
	 *
	 * @see		RTTIPlainType
	 */
	template<class T> struct RTTIPlainType<std::set<T, std::less<T>, StdAlloc<T>>>
	{
		enum { id = TID_Set }; enum { hasDynamicSize = 1 };

		typedef std::set<T, std::less<T>, StdAlloc<T>> SetType;

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const SetType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
					size += rtti_write(item, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(SetType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t numElements;
			rtti_read(numElements, stream);

			for (uint32_t i = 0; i < numElements; i++)
			{
				T element;
				rtti_read(element, stream);
				data.insert(element);
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const SetType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (const auto& item : data)
				dataSize += rtti_size(item);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::map.
	 *
	 * @see		RTTIPlainType
	 */
	template<class Key, class Value> struct RTTIPlainType<std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>>>
	{
		enum { id = TID_Map }; enum { hasDynamicSize = 1 };

		typedef std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>> MapType;

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
				{
					size += rtti_write(item.first, stream);
					size += rtti_write(item.second, stream);
				}

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t numElements;
			rtti_read(numElements, stream);

			for (uint32_t i = 0; i < numElements; i++)
			{
				Key key;
				rtti_read(key, stream);

				Value value;
				rtti_read(value, stream);

				data[key] = value;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const MapType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (const auto& item : data)
			{
				dataSize += rtti_size(item.first);
				dataSize += rtti_size(item.second);
			}

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::unordered_map.
	 *
	 * @see		RTTIPlainType
	 */
	template<class Key, class Value>
	struct RTTIPlainType<std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, StdAlloc<std::pair<const Key, Value>>>>
	{
		enum { id = TID_UnorderedMap }; enum { hasDynamicSize = 1 };

		typedef std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, StdAlloc<std::pair<const Key, Value>>> MapType;

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
				{
					size += rtti_write(item.first, stream);
					size += rtti_write(item.second, stream);
				}

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t numElements;
			rtti_read(numElements, stream);

			for (uint32_t i = 0; i < numElements; i++)
			{
				Key key;
				rtti_read(key, stream);

				Value value;
				rtti_read(value, stream);

				data[key] = value;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const MapType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (const auto& item : data)
			{
				dataSize += rtti_size(item.first);
				dataSize += rtti_size(item.second);
			}

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::unordered_set.
	 *
	 * @see		RTTIPlainType
	 */
	template<class Key>
	struct RTTIPlainType<std::unordered_set<Key, std::hash<Key>, std::equal_to<Key>, StdAlloc<Key>>>
	{
		enum { id = TID_UnorderedSet }; enum { hasDynamicSize = 1 };

		typedef std::unordered_set<Key, std::hash<Key>, std::equal_to<Key>, StdAlloc<Key>> MapType;

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
					size += rtti_write(item, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t numElements;
			rtti_read(numElements, stream);

			for (uint32_t i = 0; i < numElements; i++)
			{
				Key key;
				rtti_read(key, stream);

				data.insert(key);
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const MapType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (const auto& item : data)
				dataSize += rtti_size(item);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::pair.
	 *
	 * @see		RTTIPlainType
	 */
	template<class A, class B> struct RTTIPlainType<std::pair<A, B>>
	{
		enum { id = TID_Pair }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const std::pair<A, B>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(data.first, stream);
				size += rtti_write(data.second, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(std::pair<A, B>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.first, stream);
			rtti_read(data.second, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const std::pair<A, B>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;
			dataSize += rtti_size(data.first);
			dataSize += rtti_size(data.second);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
