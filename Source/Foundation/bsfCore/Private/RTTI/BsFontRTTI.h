//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStdRTTI.h"
#include "Text/BsFont.h"
#include "Image/BsTexture.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template <>
	struct RTTIPlainType<CharacterInformation>
	{
		enum
		{
			id = TID_CharacterInformation
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const CharacterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.CharId, stream);
				size += B3DRTTIWrite(data.Page, stream);
				size += B3DRTTIWrite(data.UvX, stream);
				size += B3DRTTIWrite(data.UvY, stream);
				size += B3DRTTIWrite(data.UvWidth, stream);
				size += B3DRTTIWrite(data.UvHeight, stream);
				size += B3DRTTIWrite(data.Width, stream);
				size += B3DRTTIWrite(data.Height, stream);
				size += B3DRTTIWrite(data.XOffset, stream);
				size += B3DRTTIWrite(data.YOffset, stream);
				size += B3DRTTIWrite(data.XAdvance, stream);
				size += B3DRTTIWrite(data.YAdvance, stream);
				size += B3DRTTIWrite(data.KerningPairs, stream);

				return size; });
		}

		static BitLength FromMemory(CharacterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);
			B3DRTTIRead(data.CharId, stream);
			B3DRTTIRead(data.Page, stream);
			B3DRTTIRead(data.UvX, stream);
			B3DRTTIRead(data.UvY, stream);
			B3DRTTIRead(data.UvWidth, stream);
			B3DRTTIRead(data.UvHeight, stream);
			B3DRTTIRead(data.Width, stream);
			B3DRTTIRead(data.Height, stream);
			B3DRTTIRead(data.XOffset, stream);
			B3DRTTIRead(data.YOffset, stream);
			B3DRTTIRead(data.XAdvance, stream);
			B3DRTTIRead(data.YAdvance, stream);
			B3DRTTIRead(data.KerningPairs, stream);

			return size;
		}

		static BitLength GetSize(const CharacterInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.CharId) + B3DRTTISize(data.Page) + B3DRTTISize(data.UvX) + B3DRTTISize(data.UvY) + B3DRTTISize(data.UvWidth) + B3DRTTISize(data.UvHeight) + B3DRTTISize(data.Width) + B3DRTTISize(data.Height) + B3DRTTISize(data.XOffset) + B3DRTTISize(data.YOffset) + B3DRTTISize(data.XAdvance) + B3DRTTISize(data.YAdvance) + B3DRTTISize(data.KerningPairs);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	class B3D_CORE_EXPORT FontBitmapPageRTTI : public RTTIType<FontBitmapPage, IReflectable, FontBitmapPageRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(Texture, 0)
			B3D_RTTI_MEMBER_PLAIN(IsDynamic, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "FontBitmapPage";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FontBitmapPage;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FontBitmapPage>();
		}
	};

	class B3D_CORE_EXPORT FontBitmapInformationRTTI : public RTTIType<FontBitmapInformation, IReflectable, FontBitmapInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Size, 0)
			B3D_RTTI_MEMBER_PLAIN(BaselineOffset, 1)
			B3D_RTTI_MEMBER_PLAIN(LineHeight, 2)
			B3D_RTTI_MEMBER_PLAIN(MissingGlyph, 3)
			B3D_RTTI_MEMBER_PLAIN(SpaceWidth, 4)
			B3D_RTTI_MEMBER_PLAIN(Characters, 6)
		B3D_RTTI_END_MEMBERS

		FontBitmapPage& GetPage(FontBitmapInformation* object, u32 index)
		{
			return object->TexturePages[index];
		}

		void SetPage(FontBitmapInformation* object, u32 index, FontBitmapPage& value)
		{
			object->TexturePages[index] = value;
		}

		u32 GetPageCount(FontBitmapInformation* object)
		{
			return mBakedPageCount;
		}

		void SetPageCount(FontBitmapInformation* object, u32 size)
		{
			object->TexturePages.resize(size);
		}

	public:
		FontBitmapInformationRTTI()
		{
			AddReflectableArrayField("TexturePages", 5, &FontBitmapInformationRTTI::GetPage, &FontBitmapInformationRTTI::GetPageCount, &FontBitmapInformationRTTI::SetPage, &FontBitmapInformationRTTI::SetPageCount);
		}

		const String& GetRttiName()
		{
			static String name = "FontBitmapInformation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FontBitmapInformation;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FontBitmapInformation>();
		}

	protected:
		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			FontBitmapInformation* bitmapInformation = static_cast<FontBitmapInformation*>(obj);

			mBakedPageCount = 0;
			for(const auto& entry : bitmapInformation->TexturePages)
			{
				if(entry.IsDynamic)
					break;

				mBakedPageCount++;
			}
		}

		u32 mBakedPageCount = 0;

	};

	class B3D_CORE_EXPORT FontRTTI : public RTTIType<Font, Resource, FontRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(RenderMode, mInformation.RenderMode, 2)
			B3D_RTTI_MEMBER_NAMED(DPI, mInformation.DPI, 3)
		B3D_RTTI_END_MEMBERS

		FontBitmapInformation& GetBitmap(Font* obj, u32 idx)
		{
			if(idx >= obj->mFontBitmaps.size())
				B3D_EXCEPT(InternalErrorException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " + ToString((int)obj->mFontBitmaps.size()));

			auto iter = obj->mFontBitmaps.begin();
			for(u32 i = 0; i < idx; i++, ++iter)
			{}

			return *iter->second;
		}

		void SetBitmap(Font* obj, u32 idx, FontBitmapInformation& value)
		{
			obj->mFontBitmaps[value.Size] = B3DMakeShared<FontBitmapInformation>();
			*obj->mFontBitmaps[value.Size] = value;
		}

		u32 GetBitmapCount(Font* obj)
		{
			return (u32)obj->mFontBitmaps.size();
		}

		void SetBitmapCount(Font* obj, u32 size)
		{
		}

		SPtr<DataStream> GetFontData(Font* obj, u32& outSize)
		{
			outSize = obj->mInformation.FontData != nullptr ? (u32)obj->mInformation.FontData->Size() : 0;
			return obj->mInformation.FontData;
		}

		void SetFontData(Font* obj, const SPtr<DataStream>& value, u32 size)
		{
			if(value == nullptr)
				obj->mInformation.FontData = nullptr;
			else if(value->IsFile())
			{
				obj->mInformation.FontData = B3DMakeShared<MemoryDataStream>(size);
				obj->mInformation.FontData->Seek(size); // Note: Forces the size to be set to this value. TODO: We need a better way to do this.
				value->Read(obj->mInformation.FontData->Data(), size);
			}
			else
				obj->mInformation.FontData = std::static_pointer_cast<MemoryDataStream>(value);
		}

	public:
		FontRTTI()
		{
			AddReflectableArrayField("mBitmaps", 0, &FontRTTI::GetBitmap, &FontRTTI::GetBitmapCount, &FontRTTI::SetBitmap, &FontRTTI::SetBitmapCount);
			AddDataBlockField("mFontData", 1, &FontRTTI::GetFontData, &FontRTTI::SetFontData);
		}

		const String& GetRttiName() override
		{
			static String name = "Font";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Font;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Font::CreateEmpty();
		}

	protected:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Font* font = static_cast<Font*>(obj);
			font->Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
