//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIElementStyle.h"
#include "2D/BsTextSprite.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIHelper.h"

namespace bs
{
	GUILabel::GUILabel(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
		: GUIElement(styleName, dimensions), mContent(content), mImageSprite(nullptr)
	{
		mTextSprite = bs_new<TextSprite>();
	}

	GUILabel::~GUILabel()
	{
		bs_delete(mTextSprite);

		if(mImageSprite != nullptr)
			bs_delete(mImageSprite);
	}

	u32 GUILabel::GetRenderElementDepthRangeInternal() const
	{
		return 2;
	}

	void GUILabel::UpdateRenderElementsInternal()
	{
		const HSpriteTexture& activeTex = GetStyleInternal()->Normal.Texture;
		if(SpriteTexture::CheckIsLoaded(activeTex))
		{
			mImageDesc.Texture = activeTex;

			if(mImageSprite == nullptr)
				mImageSprite = bs_new<ImageSprite>();
		}
		else
		{
			mImageDesc.Texture = nullptr;

			if(mImageSprite != nullptr)
			{
				bs_delete(mImageSprite);
				mImageSprite = nullptr;
			}
		}

		if(mImageSprite != nullptr)
		{
			mImageDesc.Width = mLayoutData.Area.Width;
			mImageDesc.Height = mLayoutData.Area.Height;

			mImageDesc.BorderLeft = GetStyleInternal()->Border.Left;
			mImageDesc.BorderRight = GetStyleInternal()->Border.Right;
			mImageDesc.BorderTop = GetStyleInternal()->Border.Top;
			mImageDesc.BorderBottom = GetStyleInternal()->Border.Bottom;
			mImageDesc.Color = GetTint();

			mImageSprite->Update(mImageDesc, (u64)GetParentWidgetInternal());
		}

		mDesc.Font = GetStyleInternal()->Font;
		mDesc.FontSize = GetStyleInternal()->FontSize;
		mDesc.WordWrap = GetStyleInternal()->WordWrap;
		mDesc.HorzAlign = GetStyleInternal()->TextHorzAlign;
		mDesc.VertAlign = GetStyleInternal()->TextVertAlign;
		mDesc.Width = mLayoutData.Area.Width;
		mDesc.Height = mLayoutData.Area.Height;
		mDesc.Text = mContent.Text;
		mDesc.Color = GetTint() * GetStyleInternal()->Normal.TextColor;
		;

		mTextSprite->Update(mDesc, (u64)GetParentWidgetInternal());

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::Populate({ T::SpriteInfo(mTextSprite), T::SpriteInfo(mImageSprite, 1) }, mRenderElements);
		}

		GUIElement::UpdateRenderElementsInternal();
	}

	Vector2I GUILabel::GetOptimalSizeInternal() const
	{
		return GUIHelper::CalcOptimalContentsSize(mContent, *GetStyleInternal(), GetDimensionsInternal());
	}

	void GUILabel::FillBuffer(
		u8* vertices,
		u32* indices,
		u32 vertexOffset,
		u32 indexOffset,
		const Vector2I& offset,
		u32 maxNumVerts,
		u32 maxNumIndices,
		u32 renderElementIdx) const
	{
		u8* uvs = vertices + sizeof(Vector2);
		u32 vertexStride = sizeof(Vector2) * 2;
		u32 indexStride = sizeof(u32);
		Vector2I layoutOffset = Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y) + offset;

		u32 imageSpriteIdx = mTextSprite->GetNumRenderElements();

		if(renderElementIdx < imageSpriteIdx)
		{
			mTextSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, renderElementIdx, layoutOffset, mLayoutData.GetLocalClipRect());

			return;
		}

		mImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, imageSpriteIdx - renderElementIdx, layoutOffset, mLayoutData.GetLocalClipRect());
	}

	void GUILabel::SetContent(const GUIContent& content)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		mContent = content;
		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

		if(origSize != newSize)
			MarkLayoutAsDirtyInternal();
		else
			MarkContentAsDirtyInternal();
	}

	GUILabel* GUILabel::Create(const HString& text, const String& styleName)
	{
		return Create(GUIContent(text), styleName);
	}

	GUILabel* GUILabel::Create(const HString& text, const GUIOptions& options, const String& styleName)
	{
		return Create(GUIContent(text), options, styleName);
	}

	GUILabel* GUILabel::Create(const GUIContent& content, const String& styleName)
	{
		return new(bs_alloc<GUILabel>()) GUILabel(GetStyleName<GUILabel>(styleName), content, GUIDimensions::Create());
	}

	GUILabel* GUILabel::Create(const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUILabel>()) GUILabel(GetStyleName<GUILabel>(styleName), content, GUIDimensions::Create(options));
	}

	const String& GUILabel::GetGuiTypeName()
	{
		static String typeName = "Label";
		return typeName;
	}
} // namespace bs
