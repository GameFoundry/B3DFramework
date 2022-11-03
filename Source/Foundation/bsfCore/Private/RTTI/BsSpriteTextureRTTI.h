//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Image/BsSpriteTexture.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT SpriteTextureRTTI : public RTTIType<SpriteTexture, Resource, SpriteTextureRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mAtlasTexture, 0)
			B3D_RTTI_MEMBER_PLAIN(mUVOffset, 1)
			B3D_RTTI_MEMBER_PLAIN(mUVScale, 2)
			B3D_RTTI_MEMBER_PLAIN(mAnimation, 3)
			B3D_RTTI_MEMBER_PLAIN(mPlayback, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "SpriteTexture";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_SpriteTexture;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SpriteTexture::CreateEmpty();
		}

	private:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			SpriteTexture* texture = static_cast<SpriteTexture*>(obj);
			texture->Initialize();
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
				size += B3DRTTIWrite(data.NumRows, stream);
				size += B3DRTTIWrite(data.NumColumns, stream);
				size += B3DRTTIWrite(data.Count, stream);
				size += B3DRTTIWrite(data.Fps, stream);

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
					B3DRTTIRead(data.NumRows, stream);
					B3DRTTIRead(data.NumColumns, stream);
					B3DRTTIRead(data.Count, stream);
					B3DRTTIRead(data.Fps, stream);
				}
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const SpriteSheetGridAnimation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.NumRows) + B3DRTTISize(data.NumColumns) +
				B3DRTTISize(data.Count) + B3DRTTISize(data.Fps) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
