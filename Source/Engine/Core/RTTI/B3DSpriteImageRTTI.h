//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DMathRTTI.h"
#include "Image/B3DSpriteImage.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SpriteImageRTTI : public TRTTIType<SpriteImage, Resource, SpriteImageRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(Animation, mInformation.Animation, 1)
			B3D_RTTI_MEMBER_NAMED(AnimationPlayback, mInformation.AnimationPlayback, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "SpriteImage";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SpriteImage;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return nullptr;
		}
	};

	template <>
	struct RTTIPlainType<SpriteSheetGridAnimation>
	{
		enum
		{
			id = TID_SpriteSheetGridAnimation
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const SpriteSheetGridAnimation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 0;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.RowCount, stream);
				size += B3DRTTIWrite(data.ColumnCount, stream);
				size += B3DRTTIWrite(data.FrameCount, stream);
				size += B3DRTTIWrite(data.FramesPerSecond, stream);

				return size; });
		}

		static BitLength FromMemory(SpriteSheetGridAnimation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version = 0;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				{
					B3DRTTIRead(data.RowCount, stream);
					B3DRTTIRead(data.ColumnCount, stream);
					B3DRTTIRead(data.FrameCount, stream);
					B3DRTTIRead(data.FramesPerSecond, stream);
				}
				break;
			default:
				B3D_LOG(Error, LogRTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const SpriteSheetGridAnimation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.RowCount) + B3DRTTISize(data.ColumnCount) +
				B3DRTTISize(data.FrameCount) + B3DRTTISize(data.FramesPerSecond) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
