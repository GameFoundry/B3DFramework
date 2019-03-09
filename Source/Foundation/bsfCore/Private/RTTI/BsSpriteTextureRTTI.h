//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
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
		const String& getRTTIName() override
		{
			static String name = "SpriteTexture";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SpriteTexture;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return SpriteTexture::createEmpty();
		}

	private:
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			SpriteTexture* texture = static_cast<SpriteTexture*>(obj);
			texture->initialize();
		}
	};

	template<> struct RTTIPlainType<SpriteSheetGridAnimation>
	{	
		enum { id = TID_SpriteSheetGridAnimation }; enum { hasDynamicSize = 1 };

		static void toMemory(const SpriteSheetGridAnimation& data, char* memory)
		{ 
			static constexpr UINT32 VERSION = 0;

			const UINT32 size = getDynamicSize(data);

			memory = rttiWriteElem(size, memory);
			memory = rttiWriteElem(VERSION, memory);
			memory = rttiWriteElem(data.numRows, memory);
			memory = rttiWriteElem(data.numColumns, memory);
			memory = rttiWriteElem(data.count, memory);
			memory = rttiWriteElem(data.fps, memory);
		}

		static UINT32 fromMemory(SpriteSheetGridAnimation& data, char* memory)
		{ 
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT32 version = 0;
			memory = rttiReadElem(version, memory);

			switch(version)
			{
			case 0:
			{
				memory = rttiReadElem(data.numRows, memory);
				memory = rttiReadElem(data.numColumns, memory);
				memory = rttiReadElem(data.count, memory);
				memory = rttiReadElem(data.fps, memory);
			}
				break;
			default:
				LOGERR("Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static UINT32 getDynamicSize(const SpriteSheetGridAnimation& data)
		{ 
			UINT32 size = sizeof(UINT32) * 2 + rttiGetElemSize(data.numRows) + rttiGetElemSize(data.numColumns) +
				rttiGetElemSize(data.count) + rttiGetElemSize(data.fps);
			return size;
		}	
	};

	/** @} */
	/** @endcond */
}
