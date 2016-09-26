//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsFont.h"
#include "BsFontManager.h"
#include "BsTexture.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT FontBitmapRTTI : public RTTIType<FontBitmap, IReflectable, FontBitmapRTTI>
	{
	private:
		UINT32& getSize(FontBitmap* obj) { return obj->size; }
		void setSize(FontBitmap* obj, UINT32& size) { obj->size = size; }

		FONT_DESC& getFontDesc(FontBitmap* obj) { return obj->fontDesc; }
		void setFontDesc(FontBitmap* obj, FONT_DESC& val) { obj->fontDesc = val; }

		HTexture& getTexture(FontBitmap* obj, UINT32 idx) { return obj->texturePages.at(idx); }
		void setTexture(FontBitmap* obj, UINT32 idx, HTexture& value) { obj->texturePages[idx] = value; }

		UINT32 getTextureArraySize(FontBitmap* obj) { return (UINT32)obj->texturePages.size(); }
		void setTextureArraySize(FontBitmap* obj, UINT32 size) { obj->texturePages.resize(size); }

	public:
		FontBitmapRTTI()
		{
			addPlainField("size", 0, &FontBitmapRTTI::getSize, &FontBitmapRTTI::setSize);
			addPlainField("fontDesc", 1, &FontBitmapRTTI::getFontDesc, &FontBitmapRTTI::setFontDesc);
			addReflectableArrayField("texturePages", 2, &FontBitmapRTTI::getTexture, &FontBitmapRTTI::getTextureArraySize, &FontBitmapRTTI::setTexture, &FontBitmapRTTI::setTextureArraySize);
		}

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
		struct FontInitData
		{
			Vector<SPtr<FontBitmap>> fontDataPerSize;
		};

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
			FontInitData* initData = any_cast<FontInitData*>(obj->mRTTIData);

			initData->fontDataPerSize[idx] = bs_shared_ptr_new<FontBitmap>();
			*initData->fontDataPerSize[idx] = value;
		}

		UINT32 getNumBitmaps(Font* obj)
		{
			return (UINT32)obj->mFontDataPerSize.size();
		}

		void setNumBitmaps(Font* obj, UINT32 size)
		{
			FontInitData* initData = any_cast<FontInitData*>(obj->mRTTIData);

			initData->fontDataPerSize.resize(size);
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
			return FontManager::instance()._createEmpty();
		}

	protected:
		void onDeserializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			FontInitData* initData = bs_new<FontInitData>();

			Font* font = static_cast<Font*>(obj);
			font->mRTTIData = initData;
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Font* font = static_cast<Font*>(obj);
			FontInitData* initData = any_cast<FontInitData*>(font->mRTTIData);

			font->initialize(initData->fontDataPerSize);

			bs_delete(initData);
		}
	};

	/** @} */
	/** @endcond */
}