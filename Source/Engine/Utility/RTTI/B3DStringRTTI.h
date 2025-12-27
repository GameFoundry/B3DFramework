//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <>
	struct RTTIPlainType<String>
	{
		enum
		{
			id = TID_String
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				uint32_t size = (uint32_t)(data.size() * sizeof(String::value_type));
				stream.WriteBytes((uint8_t*)data.data(), size);

				return size; });
		}

		static BitLength FromMemory(String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t stringSize = size.Bytes - sizeof(size.Bytes);
			uint8_t* buffer = (uint8_t*)B3DStackAllocate(stringSize + 1);

			stream.ReadBytes(buffer, stringSize);
			buffer[stringSize] = '\0';
			data = String((String::value_type*)buffer);

			B3DStackFree(buffer);
			return size;
		}

		static BitLength GetSize(const String& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = (uint32_t)(data.size() * sizeof(String::value_type));

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<WString>
	{
		enum
		{
			id = TID_WString
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const WString& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				uint32_t size = (uint32_t)(data.size() * sizeof(WString::value_type));
				stream.WriteBytes((uint8_t*)data.data(), size);

				return size; });
		}

		static BitLength FromMemory(WString& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t stringSize = size.Bytes - sizeof(size.Bytes);
			auto buffer = (WString::value_type*)B3DStackAllocate(stringSize + sizeof(WString::value_type));

			stream.ReadBytes((uint8_t*)buffer, stringSize);

			u32 numChars = stringSize / sizeof(WString::value_type);
			buffer[numChars] = L'\0';
			data = WString((WString::value_type*)buffer);

			B3DStackFree(buffer);
			return size;
		}

		static BitLength GetSize(const WString& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = (uint32_t)(data.size() * sizeof(WString::value_type));

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<U32String>
	{
		enum
		{
			id = TID_U32String
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const U32String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				uint32_t size = (uint32_t)(data.size() * sizeof(U32String::value_type));
				stream.WriteBytes((uint8_t*)data.data(), size);

				return size; });
		}

		static BitLength FromMemory(U32String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t stringSize = size.Bytes - sizeof(size.Bytes);
			auto buffer = (U32String::value_type*)B3DStackAllocate(stringSize + sizeof(U32String::value_type));

			stream.ReadBytes((uint8_t*)buffer, stringSize);

			u32 numChars = stringSize / sizeof(U32String::value_type);
			buffer[numChars] = L'\0';
			data = U32String((U32String::value_type*)buffer);

			B3DStackFree(buffer);
			return size;
		}

		static BitLength GetSize(const U32String& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = (uint32_t)(data.size() * sizeof(U32String::value_type));

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
