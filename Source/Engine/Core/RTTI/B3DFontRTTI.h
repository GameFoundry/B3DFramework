//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStdRTTI.h"
#include "Text/B3DFont.h"
#include "Image/B3DTexture.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
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
				size += B3DRTTIWrite(data.PointSize, stream);

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
			B3DRTTIRead(data.PointSize, stream);

			return size;
		}

		static BitLength GetSize(const CharacterInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.CharId) + B3DRTTISize(data.Page) + B3DRTTISize(data.UvX) + B3DRTTISize(data.UvY) + B3DRTTISize(data.UvWidth) + B3DRTTISize(data.UvHeight) + B3DRTTISize(data.Width) + B3DRTTISize(data.Height) + B3DRTTISize(data.XOffset) + B3DRTTISize(data.YOffset) + B3DRTTISize(data.XAdvance) + B3DRTTISize(data.YAdvance) + B3DRTTISize(data.KerningPairs) + B3DRTTISize(data.PointSize);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	class B3D_EXPORT FontBitmapPageRTTI : public TRTTIType<FontBitmapPage, IReflectable, FontBitmapPageRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Texture, 0)
			B3D_RTTI_MEMBER(Type, 1)
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

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FontBitmapPage>();
		}
	};

	class B3D_EXPORT FontBitmapInformationRTTI : public TRTTIType<FontBitmapInformation, IReflectable, FontBitmapInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Size, 0)
			B3D_RTTI_MEMBER(BaselineOffset, 1)
			B3D_RTTI_MEMBER(LineHeight, 2)
			B3D_RTTI_MEMBER(MissingGlyph, 3)
			B3D_RTTI_MEMBER(SpaceWidth, 4)
			B3D_RTTI_MEMBER(Characters, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "FontBitmapInformation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FontBitmapInformation;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FontBitmapInformation>();
		}

	protected:
	};

	class B3D_EXPORT FontRTTI : public TRTTIType<Font, Resource, FontRTTI>
	{
	private:
		Vector<FontBitmapPage> mBakedPages;
		UnorderedMap<float, TShared<FontBitmapInformation>> mCharactersByPointSize;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(RenderMode, mInformation.RenderMode, 2)
			B3D_RTTI_MEMBER_NAMED(DPI, mInformation.DPI, 3)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mCharactersByPointSize, 4)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mBakedPages, 6)
		B3D_RTTI_END_MEMBERS

		TShared<DataStream> GetFontData(Font* obj, u32& outSize)
		{
			outSize = obj->mInformation.FontData != nullptr ? (u32)obj->mInformation.FontData->Size() : 0;
			return obj->mInformation.FontData;
		}

		void SetFontData(Font* obj, const TShared<DataStream>& value, u32 size)
		{
			if(value == nullptr)
			{
				obj->mInformation.FontData = nullptr;
				return;
			}

			if(!value->IsFile())
			{
				obj->mInformation.FontData = std::static_pointer_cast<MemoryDataStream>(value);
				return;
			}

			TShared<MemoryDataStream> fontData = B3DMakeShared<MemoryDataStream>(size);
			fontData->Seek(size); // Forces Size() to be set (the capacity constructor leaves it at 0).

			TAsyncOp<TShared<MemoryDataStream>> readOp = value->ReadAsync((u64)value->Tell(), size, DataRange(fontData->Data(), size));
			readOp.BlockUntilComplete();

			obj->mInformation.FontData = fontData;
		}

	public:
		FontRTTI()
		{
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

		TShared<IReflectable> NewRttiObject() override
		{
			return Font::CreateEmpty();
		}

	protected:
		void OnOperationStarted(Font& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				// Only serialize non-runtime characters and pages
				Vector<u32> pageIndexRemapping(object.mFontPages.size());
				for(u32 pageIndex = 0; pageIndex < (u32)object.mFontPages.size(); ++pageIndex)
				{
					FontBitmapPage& page = object.mFontPages[pageIndex];
					if(page.Type == FontBitmapPageType::Runtime)
					{
						pageIndexRemapping[pageIndex] = ~0u;
						continue;
					}

					pageIndexRemapping[pageIndex] = (u32)mBakedPages.size();

					FontBitmapPage pageCopy = page;
					pageCopy.Type = FontBitmapPageType::Loaded;

					mBakedPages.push_back(pageCopy);
				}

				for(const auto& bitmapPair : object.mCharactersByPointSize)
				{
					const TShared<FontBitmapInformation>& bitmapInformation = bitmapPair.second;
					if(!B3D_ENSURE(bitmapInformation != nullptr))
						continue;

					TShared<FontBitmapInformation> bitmapInformationCopy = B3DMakeShared<FontBitmapInformation>();
					*bitmapInformationCopy = *bitmapInformation;
					bitmapInformationCopy->Characters.clear();

					for(auto& characterPair : bitmapInformation->Characters)
					{
						FontBitmapPage& page = object.mFontPages[characterPair.second.Page];
						if(page.Type == FontBitmapPageType::Runtime)
							continue;

						CharacterInformation characterInformation = characterPair.second;
						characterInformation.Page = pageIndexRemapping[characterInformation.Page];

						bitmapInformationCopy->Characters.insert(std::make_pair(characterPair.first, characterInformation));
					}
				}
			}
		}

		void OnOperationEnded(Font& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
			{
				const u32 bakedPageCount = (u32)mBakedPages.size();

				object.mFontPages.reserve(bakedPageCount);
				for(u32 pageIndex = 0; pageIndex < bakedPageCount; ++pageIndex)
					object.mFontPages.push_back(mBakedPages[pageIndex]);

				for(const auto& entry : mCharactersByPointSize)
					object.mCharactersByPointSize.insert(entry);

				object.Initialize();
			}
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
