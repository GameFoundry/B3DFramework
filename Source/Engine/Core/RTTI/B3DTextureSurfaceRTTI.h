//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template <>
	struct RTTIPlainType<TextureSurface>
	{
		enum { id = TID_TextureSurface };
		enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::ToMemory */
		static BitLength ToMemory(const TextureSurface& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 1; // In case the data structure changes

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
					BitLength size = 0;
					size += B3DRTTIWrite(kVersion, stream);
					size += B3DRTTIWrite(data.MipLevel, stream);
					size += B3DRTTIWrite(data.MipLevelCount, stream);
					size += B3DRTTIWrite(data.Face, stream);
					size += B3DRTTIWrite(data.FaceCount, stream);
					size += B3DRTTIWrite(data.IsBoundAs2DArray, stream);

					return size; });
		}

		static BitLength FromMemory(TextureSurface& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			const bool hasVersionField = size > (sizeof(u32) * 4);

			uint32_t version;
			if(!hasVersionField)
			{
				version = 0;
			}
			else
			{
				B3DRTTIRead(version, stream);
			}

			B3DRTTIRead(data.MipLevel, stream);
			B3DRTTIRead(data.MipLevelCount, stream);
			B3DRTTIRead(data.Face, stream);
			B3DRTTIRead(data.FaceCount, stream);

			switch(version)
			{
			case 0:
				break;
			case 1:
				B3DRTTIRead(data.IsBoundAs2DArray, stream);
				break;
			default:
				B3D_LOG(Error, LogRTTI, "Unable to deserialize TextureSurface. (Unknown version.)");
				break;
			}

			return size;
		}

		static BitLength GetSize(const TextureSurface& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += B3DRTTISize(data.MipLevel);
			dataSize += B3DRTTISize(data.MipLevelCount);
			dataSize += B3DRTTISize(data.Face);
			dataSize += B3DRTTISize(data.FaceCount);
			dataSize += B3DRTTISize(data.IsBoundAs2DArray);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
