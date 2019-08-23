//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Private/RTTI/BsCharDescRTTI.h"
#include "Text/BsFont.h"
#include "Image/BsTexture.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT FontBitmapRTTI : public RTTIType<FontBitmap, IReflectable, FontBitmapRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(size, 0)
			BS_RTTI_MEMBER_PLAIN(baselineOffset, 1)
			BS_RTTI_MEMBER_PLAIN(lineHeight, 2)
			BS_RTTI_MEMBER_PLAIN(missingGlyph, 3)
			BS_RTTI_MEMBER_PLAIN(spaceWidth, 4)
			BS_RTTI_MEMBER_REFL_ARRAY(texturePages, 5)
			BS_RTTI_MEMBER_PLAIN(characters, 6)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "FontData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_FontBitmap;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<FontBitmap>();
		}
	};

	class BS_CORE_EXPORT FontRTTI : public RTTIType<Font, Resource, FontRTTI>
	{
	private:
		FontBitmap& getBitmap(Font* obj, UINT32 idx)
		{
			if(idx >= obj->mFontDataPerSize.size())
				BS_EXCEPT(InternalErrorException, "Index out of range: " + toString(idx) + ". Valid range: 0 .. " + toString((int)obj->mFontDataPerSize.size()));

			auto iter = obj->mFontDataPerSize.begin();
			for(UINT32 i = 0; i < idx; i++, ++iter)
			{ }

			return *iter->second;
		}

		void setBitmap(Font* obj, UINT32 idx, FontBitmap& value)
		{
			mFontDataPerSize[idx] = bs_shared_ptr_new<FontBitmap>();
			*mFontDataPerSize[idx] = value;
		}

		UINT32 getNumBitmaps(Font* obj)
		{
			return (UINT32)obj->mFontDataPerSize.size();
		}

		void setNumBitmaps(Font* obj, UINT32 size)
		{
			mFontDataPerSize.resize(size);
		}

	public:
		FontRTTI()
		{
			addReflectableArrayField("mBitmaps", 0, &FontRTTI::getBitmap, &FontRTTI::getNumBitmaps, &FontRTTI::setBitmap, &FontRTTI::setNumBitmaps);
		}

		const String& getRTTIName() override
		{
			static String name = "Font";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Font;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Font::_createEmpty();
		}

	protected:
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Font* font = static_cast<Font*>(obj);
			font->initialize(mFontDataPerSize);
		}

		Vector<SPtr<FontBitmap>> mFontDataPerSize;
	};

	/** @} */
	/** @endcond */
}
