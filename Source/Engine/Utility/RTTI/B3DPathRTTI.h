//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStdRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <>
	struct RTTIPlainType<Path>
	{
		enum
		{
			id = TID_Path
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.mDevice, stream);
				size += B3DRTTIWrite(data.mNode, stream);
				size += B3DRTTIWrite(data.mFilename, stream);
				size += B3DRTTIWrite(data.mIsAbsolute, stream);
				size += B3DRTTIWrite(data.mDirectories, stream);

				return size; });
		}

		static BitLength FromMemory(Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			B3DRTTIRead(data.mDevice, stream);
			B3DRTTIRead(data.mNode, stream);
			B3DRTTIRead(data.mFilename, stream);
			B3DRTTIRead(data.mIsAbsolute, stream);
			B3DRTTIRead(data.mDirectories, stream);

			return size;
		}

		static BitLength GetSize(const Path& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.mDevice) + B3DRTTISize(data.mNode) +
				B3DRTTISize(data.mFilename) + B3DRTTISize(data.mIsAbsolute) + B3DRTTISize(data.mDirectories);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
