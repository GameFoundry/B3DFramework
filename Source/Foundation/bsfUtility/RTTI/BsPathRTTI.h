//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	*  @{
	*/

	template<> struct RTTIPlainType<Path>
	{
		enum { id = TID_Path }; enum { hasDynamicSize = 1 };

		static BitLength toMemory(const Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(data.mDevice, stream);
				size += rtti_write(data.mNode, stream);
				size += rtti_write(data.mFilename, stream);
				size += rtti_write(data.mIsAbsolute, stream);
				size += rtti_write(data.mDirectories, stream);

				return size;
			});
		}

		static BitLength fromMemory(Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.mDevice, stream);
			rtti_read(data.mNode, stream);
			rtti_read(data.mFilename, stream);
			rtti_read(data.mIsAbsolute, stream);
			rtti_read(data.mDirectories, stream);

			return size;
		}

		static BitLength getSize(const Path& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(data.mDevice) + rtti_size(data.mNode) +
				rtti_size(data.mFilename) + rtti_size(data.mIsAbsolute) + rtti_size(data.mDirectories);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
