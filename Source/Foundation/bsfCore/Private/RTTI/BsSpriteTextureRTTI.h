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

	class BS_CORE_EXPORT SpriteTextureRTTI : public RTTIType<SpriteTexture, Resource, SpriteTextureRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mAtlasTexture, 0)
			BS_RTTI_MEMBER_PLAIN(mUVOffset, 1)
			BS_RTTI_MEMBER_PLAIN(mUVScale, 2)
			BS_RTTI_MEMBER_PLAIN(mAnimation, 3)
			BS_RTTI_MEMBER_PLAIN(mPlayback, 4)
		BS_END_RTTI_MEMBERS

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

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.NumRows, stream);
				size += rtti_write(data.NumColumns, stream);
				size += rtti_write(data.Count, stream);
				size += rtti_write(data.Fps, stream);

				return size; });
		}

		static BitLength FromMemory(SpriteSheetGridAnimation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version = 0;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				{
					rtti_read(data.NumRows, stream);
					rtti_read(data.NumColumns, stream);
					rtti_read(data.Count, stream);
					rtti_read(data.Fps, stream);
				}
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const SpriteSheetGridAnimation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(data.NumRows) + rtti_size(data.NumColumns) +
				rtti_size(data.Count) + rtti_size(data.Fps) + sizeof(uint32_t);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
