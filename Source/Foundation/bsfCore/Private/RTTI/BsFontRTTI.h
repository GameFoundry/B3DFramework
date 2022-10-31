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
			BS_RTTI_MEMBER_PLAIN(Size, 0)
			BS_RTTI_MEMBER_PLAIN(BaselineOffset, 1)
			BS_RTTI_MEMBER_PLAIN(LineHeight, 2)
			BS_RTTI_MEMBER_PLAIN(MissingGlyph, 3)
			BS_RTTI_MEMBER_PLAIN(SpaceWidth, 4)
			BS_RTTI_MEMBER_REFL_ARRAY(TexturePages, 5)
			BS_RTTI_MEMBER_PLAIN(Characters, 6)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "FontData";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_FontBitmap;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FontBitmap>();
		}
	};

	class BS_CORE_EXPORT FontRTTI : public RTTIType<Font, Resource, FontRTTI>
	{
	private:
		FontBitmap& GetBitmap(Font* obj, u32 idx)
		{
			if(idx >= obj->mFontDataPerSize.size())
				BS_EXCEPT(InternalErrorException, "Index out of range: " + toString(idx) + ". Valid range: 0 .. " + toString((int)obj->mFontDataPerSize.size()));

			auto iter = obj->mFontDataPerSize.begin();
			for(u32 i = 0; i < idx; i++, ++iter)
			{}

			return *iter->second;
		}

		void SetBitmap(Font* obj, u32 idx, FontBitmap& value)
		{
			mFontDataPerSize[idx] = B3DMakeShared<FontBitmap>();
			*mFontDataPerSize[idx] = value;
		}

		u32 GetNumBitmaps(Font* obj)
		{
			return (u32)obj->mFontDataPerSize.size();
		}

		void SetNumBitmaps(Font* obj, u32 size)
		{
			mFontDataPerSize.resize(size);
		}

	public:
		FontRTTI()
		{
			AddReflectableArrayField("mBitmaps", 0, &FontRTTI::GetBitmap, &FontRTTI::GetNumBitmaps, &FontRTTI::SetBitmap, &FontRTTI::SetNumBitmaps);
		}

		const String& GetRttiName() override
		{
			static String name = "Font";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Font;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Font::CreateEmptyInternal();
		}

	protected:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Font* font = static_cast<Font*>(obj);
			font->Initialize(mFontDataPerSize);
		}

		Vector<SPtr<FontBitmap>> mFontDataPerSize;
	};

	/** @} */
	/** @endcond */
} // namespace bs
