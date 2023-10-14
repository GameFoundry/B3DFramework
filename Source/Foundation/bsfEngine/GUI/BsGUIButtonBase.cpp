//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIButtonBase.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIHelper.h"
#include "StyleSheet/BsGUIStyleSheet.h"
#include "VectorGraphics/BsVectorGraphics.h"

using namespace bs;

GUIButtonBase::GUIButtonBase(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions, GUIElementOptions options)
	: GUIElement(styleName, dimensions, options), mContent(content)
{
	mImageSprite = B3DNew<ImageSprite>();
	mBackgroundSprite = B3DNew<VectorSprite>();
	mTextSprite = B3DNew<TextSprite>();

	mImageDesc.AnimationStartTime = GetTime().GetTime();
	mContentAnimationStartTime = mImageDesc.AnimationStartTime;

	RefreshContentSprite();
}

GUIButtonBase::~GUIButtonBase()
{
	B3DDelete(mTextSprite);
	B3DDelete(mImageSprite);
	B3DDelete(mBackgroundSprite);

	if(mContentImageSprite != nullptr)
		B3DDelete(mContentImageSprite);
}

void GUIButtonBase::SetContent(const GUIContent& content)
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
	mContent = content;
	mContentAnimationStartTime = GetTime().GetTime();

	RefreshContentSprite();

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUIButtonBase::SetOnInternal(bool on)
{
	if(on)
		AddStateFlags(GUIElementStateFlag::Checked);
	else
		RemoveStateFlags(GUIElementStateFlag::Checked);

	if(on)
		SetStateInternal((GUIElementState)((i32)mActiveState | (i32)GUIElementState::OnFlag));
	else
		SetStateInternal((GUIElementState)((i32)mActiveState & ~(i32)GUIElementState::OnFlag));
}

bool GUIButtonBase::IsOnInternal() const
{
	return ((i32)mActiveState & (i32)GUIElementState::OnFlag) != 0;
}

void GUIButtonBase::UpdateRenderElements()
{
	const bool isUsingStyleSheets = GetStyleSheetElement() != nullptr;
	if(isUsingStyleSheets)
	{
		mBackgroundSpriteInformation.Width = mLayoutData.Area.Width;
		mBackgroundSpriteInformation.Height = mLayoutData.Area.Height;

		mBackgroundSpriteInformation.VectorPath = CreateBackgroundVectorPath(Size2UI(mLayoutData.Area.Width, mLayoutData.Area.Height), *mStyleSheetStateStyle);
		mBackgroundSpriteInformation.Color = GetTint();
		mBackgroundSpriteInformation.Color.A *= mStyleSheetStateStyle->Opacity;

		mBackgroundSprite->Update(mBackgroundSpriteInformation, (u64)GetParentWidget());
	}
	else
	{
		mImageDesc.Width = mLayoutData.Area.Width;
		mImageDesc.Height = mLayoutData.Area.Height;

		const HSpriteTexture& activeTex = GetActiveTexture();
		if(SpriteTexture::CheckIsLoaded(activeTex))
			mImageDesc.Image = activeTex;
		else
			mImageDesc.Image = nullptr;

		mImageDesc.BorderLeft = GetStyle()->Border.Left;
		mImageDesc.BorderRight = GetStyle()->Border.Right;
		mImageDesc.BorderTop = GetStyle()->Border.Top;
		mImageDesc.BorderBottom = GetStyle()->Border.Bottom;
		mImageDesc.Color = GetTint();

		mImageSprite->Update(mImageDesc, (u64)GetParentWidget());
	}

	mTextSprite->Update(GetTextDesc(), (u64)GetParentWidget());

	if(mContentImageSprite != nullptr)
	{
		Rect2I contentBounds = GetCachedContentBounds();

		HSpriteTexture image = mContent.GetImage(mActiveState);
		u32 contentWidth = image->GetWidth();
		u32 contentHeight = image->GetHeight();

		u32 contentMaxWidth = std::min((u32)contentBounds.Width, contentWidth);
		u32 contentMaxHeight = std::min((u32)contentBounds.Height, contentHeight);

		float horzRatio = contentMaxWidth / (float)contentWidth;
		float vertRatio = contentMaxHeight / (float)contentHeight;

		if(horzRatio < vertRatio)
		{
			contentWidth = Math::RoundToI32(contentWidth * horzRatio);
			contentHeight = Math::RoundToI32(contentHeight * horzRatio);
		}
		else
		{
			contentWidth = Math::RoundToI32(contentWidth * vertRatio);
			contentHeight = Math::RoundToI32(contentHeight * vertRatio);
		}

		ImageSpriteInformation contentImgDesc;
		contentImgDesc.Image = image;
		contentImgDesc.Width = contentWidth;
		contentImgDesc.Height = contentHeight;
		contentImgDesc.Color = GetTint();
		contentImgDesc.AnimationStartTime = mContentAnimationStartTime;

		mContentImageSprite->Update(contentImgDesc, (u64)GetParentWidget());
	}

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;

		if(isUsingStyleSheets)
			T::Populate({ T::SpriteInfo(mBackgroundSprite, 1), T::SpriteInfo(mTextSprite), T::SpriteInfo(mContentImageSprite) }, mRenderElements);
		else
			T::Populate({ T::SpriteInfo(mImageSprite, 1), T::SpriteInfo(mTextSprite), T::SpriteInfo(mContentImageSprite) }, mRenderElements);
	}

	GUIElement::UpdateRenderElements();
}

Vector2I GUIButtonBase::GetOptimalSize() const
{
	u32 imageWidth = 0;
	u32 imageHeight = 0;

	const bool isUsingStyleSheets = GetStyleSheetElement() != nullptr;
	if(isUsingStyleSheets)
	{
		const Size2UI contentSize = GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(mContent, *mStyleSheetStateStyle, GetDimensions().MaxWidth);
		
		const u32 contentWidth = std::max(imageWidth, contentSize.Width);
		const u32 contentHeight = std::max(imageHeight, contentSize.Height);

		return Vector2I(contentWidth, contentHeight);
	}
	else
	{
		const HSpriteTexture& activeTex = GetActiveTexture();
		if(SpriteTexture::CheckIsLoaded(activeTex))
		{
			imageWidth = activeTex->GetWidth();
			imageHeight = activeTex->GetHeight();
		}

		Vector2I contentSize = GUIHelper::CalculateOptimalContentSize(mContent, *GetStyle(), GetDimensions(), mActiveState);
		u32 contentWidth = std::max(imageWidth, (u32)contentSize.X);
		u32 contentHeight = std::max(imageHeight, (u32)contentSize.Y);

		return Vector2I(contentWidth, contentHeight);
	}
}

u32 GUIButtonBase::GetRenderElementDepthRange() const
{
	return 2;
}

void GUIButtonBase::FillBuffer(
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

	const bool isUsingStyleSheets = GetStyleSheetElement() != nullptr;
	u32 textSpriteIdx = isUsingStyleSheets ? mBackgroundSprite->GetRenderElementCount() : mImageSprite->GetRenderElementCount();
	u32 contentImgSpriteIdx = textSpriteIdx + mTextSprite->GetRenderElementCount();

	if(renderElementIdx < textSpriteIdx)
	{
		Vector2I imageOffset = Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y) + offset;

		if(isUsingStyleSheets)
			mBackgroundSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, renderElementIdx, imageOffset, mLayoutData.GetLocalClipRect());
		else
			mImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, renderElementIdx, imageOffset, mLayoutData.GetLocalClipRect());

		return;
	}

	Rect2I contentBounds = GetCachedContentBounds();
	Rect2I contentClipRect = GetCachedContentClipRect();
	Rect2I textBounds = mTextSprite->GetBounds(Vector2I(), Rect2I());

	Vector2I textOffset;
	Rect2I textClipRect;

	Vector2I contentOffset;
	Rect2I imageClipRect;
	if(mContentImageSprite != nullptr)
	{
		Rect2I imageBounds = mContentImageSprite->GetBounds(Vector2I(), Rect2I());
		i32 imageXOffset = 0;
		i32 textImageSpacing = 0;

		if(textBounds.Width == 0)
		{
			u32 freeWidth = (u32)std::max(0, (i32)contentBounds.Width - (i32)textBounds.Width - (i32)imageBounds.Width);
			imageXOffset = (i32)(freeWidth / 2);
		}
		else
			textImageSpacing = GUIContent::kImageTextSpacing;

		if(GetStyle()->ImagePosition == GUIImagePosition::Right)
		{
			i32 imageReservedWidth = std::max(0, (i32)contentBounds.Width - (i32)textBounds.Width);

			textOffset = Vector2I(contentBounds.X, contentBounds.Y);
			textClipRect = contentClipRect;
			textClipRect.Width = std::min(contentBounds.Width - imageReservedWidth, textClipRect.Width);

			contentOffset = Vector2I(contentBounds.X + textBounds.Width + imageXOffset + textImageSpacing, contentBounds.Y) + offset;
			imageClipRect = contentClipRect;
			imageClipRect.X -= textBounds.Width + imageXOffset;
		}
		else
		{
			i32 imageReservedWidth = imageBounds.Width + imageXOffset;

			contentOffset = Vector2I(contentBounds.X + imageXOffset, contentBounds.Y) + offset;
			imageClipRect = contentClipRect;
			imageClipRect.X -= imageXOffset;
			imageClipRect.Width = std::min(imageReservedWidth, (i32)imageClipRect.Width);

			textOffset = Vector2I(contentBounds.X + imageReservedWidth + textImageSpacing, contentBounds.Y);
			textClipRect = contentClipRect;
			textClipRect.X -= imageReservedWidth;
		}

		i32 imageYOffset = (contentBounds.Height - imageBounds.Height) / 2;
		imageClipRect.Y -= imageYOffset;
		contentOffset.Y += imageYOffset;
	}
	else
	{
		textOffset = Vector2I(contentBounds.X, contentBounds.Y) + offset;
		textClipRect = contentClipRect;
	}

	if(renderElementIdx >= contentImgSpriteIdx)
	{
		mContentImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, contentImgSpriteIdx - renderElementIdx, contentOffset, imageClipRect);
	}
	else
	{
		mTextSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, textSpriteIdx - renderElementIdx, textOffset, textClipRect);
	}
}

bool GUIButtonBase::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseOver)
	{
		if(!IsDisabled())
		{
			AddStateFlags(GUIElementStateFlag::Hover);

			if(mHasFocus)
				SetStateInternal(IsOnInternal() ? GUIElementState::FocusedHoverOn : GUIElementState::FocusedHover);
			else
				SetStateInternal(IsOnInternal() ? GUIElementState::HoverOn : GUIElementState::Hover);

			OnHover();
		}

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(ev.GetType() == GUIMouseEventType::MouseOut)
	{
		if(!IsDisabled())
		{
			RemoveStateFlags(GUIElementStateFlag::Hover | GUIElementStateFlag::Active);

			if(mHasFocus)
				SetStateInternal(IsOnInternal() ? GUIElementState::FocusedOn : GUIElementState::Focused);
			else
				SetStateInternal(IsOnInternal() ? GUIElementState::NormalOn : GUIElementState::Normal);

			OnOut();
		}

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDown)
	{
		if(!IsDisabled())
		{
			AddStateFlags(GUIElementStateFlag::Active);
			SetStateInternal(IsOnInternal() ? GUIElementState::ActiveOn : GUIElementState::Active);
		}

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(ev.GetType() == GUIMouseEventType::MouseUp)
	{
		if(!IsDisabled())
		{
			RemoveStateFlags(GUIElementStateFlag::Active);

			if(mHasFocus)
				SetStateInternal(IsOnInternal() ? GUIElementState::FocusedHoverOn : GUIElementState::FocusedHover);
			else
				SetStateInternal(IsOnInternal() ? GUIElementState::HoverOn : GUIElementState::Hover);

			OnClick();
		}

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDoubleClick)
	{
		if(!IsDisabled())
			OnDoubleClick();

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}

	return false;
}

bool GUIButtonBase::DoOnCommandEvent(const GUICommandEvent& ev)
{
	const bool baseReturnValue = GUIElement::DoOnCommandEvent(ev);

	GUIElementState state = (GUIElementState)((u32)mActiveState & (u32)GUIElementState::TypeMask);
	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		mHasFocus = true;

		if(!IsDisabled())
		{
			AddStateFlags(GUIElementStateFlag::Focused);

			if(state == GUIElementState::Normal)
				SetStateInternal(IsOnInternal() ? GUIElementState::FocusedOn : GUIElementState::Focused);
			else if(state == GUIElementState::Hover)
				SetStateInternal(IsOnInternal() ? GUIElementState::FocusedHoverOn : GUIElementState::FocusedHover);
		}

		return true;
	}
	else if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		mHasFocus = false;
		RemoveStateFlags(GUIElementStateFlag::Focused);

		if(state == GUIElementState::Focused)
			SetStateInternal(IsOnInternal() ? GUIElementState::NormalOn : GUIElementState::Normal);
		else if(state == GUIElementState::FocusedHover)
			SetStateInternal(IsOnInternal() ? GUIElementState::HoverOn : GUIElementState::Hover);

		return true;
	}

	return baseReturnValue;
}

String GUIButtonBase::GetTooltip() const
{
	return mContent.Tooltip;
}

void GUIButtonBase::RefreshContentSprite()
{
	HSpriteTexture contentTex = mContent.GetImage(mActiveState);
	if(SpriteTexture::CheckIsLoaded(contentTex))
	{
		if(mContentImageSprite == nullptr)
			mContentImageSprite = B3DNew<ImageSprite>();
	}
	else
	{
		if(mContentImageSprite != nullptr)
		{
			B3DDelete(mContentImageSprite);
			mContentImageSprite = nullptr;
		}
	}
}

TextSpriteInformation GUIButtonBase::GetTextDesc() const
{
	const bool isUsingStyleSheets = GetStyleSheetElement() != nullptr;
	if(isUsingStyleSheets)
	{
		TextSpriteInformation textSpriteInformation;
		textSpriteInformation.Text = mContent.Text;
		textSpriteInformation.Font = mStyleSheetStateStyle->Font;
		textSpriteInformation.FontSize = mStyleSheetStateStyle->FontSize;
		textSpriteInformation.Color = GetTint() * mStyleSheetStateStyle->Color;
		textSpriteInformation.Color.A *= mStyleSheetStateStyle->Opacity;

		Rect2I textBounds = GetCachedContentBounds();

		textSpriteInformation.Width = textBounds.Width;
		textSpriteInformation.Height = textBounds.Height;
		textSpriteInformation.HorzAlign = mStyleSheetStateStyle->HorizontalTextAlignment;
		textSpriteInformation.VertAlign = mStyleSheetStateStyle->VerticalTextAlignment;

		return textSpriteInformation;
	}
	else
	{
		TextSpriteInformation textDesc;
		textDesc.Text = mContent.Text;
		textDesc.Font = GetStyle()->Font;
		textDesc.FontSize = GetStyle()->FontSize;
		textDesc.Color = GetTint() * GetActiveTextColor();

		Rect2I textBounds = GetCachedContentBounds();

		textDesc.Width = textBounds.Width;
		textDesc.Height = textBounds.Height;
		textDesc.HorzAlign = GetStyle()->TextHorzAlign;
		textDesc.VertAlign = GetStyle()->TextVertAlign;

		return textDesc;
	}
}

void GUIButtonBase::NotifyStyleChanged()
{
	mImageDesc.AnimationStartTime = GetTime().GetTime();
}

void GUIButtonBase::SetStateInternal(GUIElementState state)
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

	if(mActiveState != state)
		mImageDesc.AnimationStartTime = GetTime().GetTime();

	mActiveState = state;
	RefreshContentSprite();
	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

const HSpriteTexture& GUIButtonBase::GetActiveTexture() const
{
	switch(mActiveState)
	{
	case GUIElementState::Normal:
		return GetStyle()->Normal.Texture;
	case GUIElementState::Hover:
		return GetStyle()->Hover.Texture;
	case GUIElementState::Active:
		return GetStyle()->Active.Texture;
	case GUIElementState::Focused:
		return GetStyle()->Focused.Texture;
	case GUIElementState::FocusedHover:
		return GetStyle()->FocusedHover.Texture;
	case GUIElementState::NormalOn:
		return GetStyle()->NormalOn.Texture;
	case GUIElementState::HoverOn:
		return GetStyle()->HoverOn.Texture;
	case GUIElementState::ActiveOn:
		return GetStyle()->ActiveOn.Texture;
	case GUIElementState::FocusedOn:
		return GetStyle()->FocusedOn.Texture;
	case GUIElementState::FocusedHoverOn:
		return GetStyle()->FocusedHoverOn.Texture;
	default:
		break;
	}

	return GetStyle()->Normal.Texture;
}

Color GUIButtonBase::GetActiveTextColor() const
{
	switch(mActiveState)
	{
	case GUIElementState::Normal:
		return GetStyle()->Normal.TextColor;
	case GUIElementState::Hover:
		return GetStyle()->Hover.TextColor;
	case GUIElementState::Active:
		return GetStyle()->Active.TextColor;
	case GUIElementState::Focused:
		return GetStyle()->Focused.TextColor;
	case GUIElementState::FocusedHover:
		return GetStyle()->FocusedHover.TextColor;
	case GUIElementState::NormalOn:
		return GetStyle()->NormalOn.TextColor;
	case GUIElementState::HoverOn:
		return GetStyle()->HoverOn.TextColor;
	case GUIElementState::ActiveOn:
		return GetStyle()->ActiveOn.TextColor;
	case GUIElementState::FocusedOn:
		return GetStyle()->FocusedOn.TextColor;
	case GUIElementState::FocusedHoverOn:
		return GetStyle()->FocusedHoverOn.TextColor;
	default:
		break;
	}

	return GetStyle()->Normal.TextColor;
}

HVectorPath GUIButtonBase::CreateBackgroundVectorPath(const Size2UI& size, const GUIStyleSheetStateRule& style)
{
	HVectorPath path = VectorPath::Create();

	const Rect2 fillArea = Rect2(0.0f, 0.0f, (float)size.Width, (float)size.Height);

	path->DrawRoundedRectangle(fillArea, (float)style.BorderTopLeftRadius, (float)style.BorderTopRightRadius, (float)style. BorderBottomLeftRadius, (float)style.BorderTopRightRadius)
		.ClosePath()
		.SetFillPaint(style.BackgroundColor)
		.DrawFill();

	// TODO - Not supporting separate border styles at the moment. See nvgRoundedRectVarying for implementation. Also ideally support elliptical corners
	const bool drawBorder = style.BorderLeft.Width > 0 && style.BorderLeft.Style != GUIBorderElementStyle::None;
	if(drawBorder)
	{
		path->SetStrokePaint(style.BorderLeft.Color)
			.SetStrokeWidth((float)style.BorderLeft.Width)
			.DrawStroke();
	}

	return path;
}

