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

using namespace bs;

GUIButtonBase::GUIButtonBase(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions, GUIElementOptions options)
	: GUIElement(styleName, dimensions, options), mContent(content)
{
	mImageSprite = B3DNew<ImageSprite>();
	mTextSprite = B3DNew<TextSprite>();

	mImageDesc.AnimationStartTime = GetTime().GetTime();
	mContentAnimationStartTime = mImageDesc.AnimationStartTime;

	RefreshContentSprite();
}

GUIButtonBase::~GUIButtonBase()
{
	B3DDelete(mTextSprite);
	B3DDelete(mImageSprite);

	if(mContentImageSprite != nullptr)
		B3DDelete(mContentImageSprite);
}

void GUIButtonBase::SetContent(const GUIContent& content)
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
	mContent = content;
	mContentAnimationStartTime = GetTime().GetTime();

	RefreshContentSprite();

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirtyInternal();
	else
		MarkContentAsDirtyInternal();
}

void GUIButtonBase::SetOnInternal(bool on)
{
	if(on)
		SetStateInternal((GUIElementState)((i32)mActiveState | (i32)GUIElementState::OnFlag));
	else
		SetStateInternal((GUIElementState)((i32)mActiveState & ~(i32)GUIElementState::OnFlag));
}

bool GUIButtonBase::IsOnInternal() const
{
	return ((i32)mActiveState & (i32)GUIElementState::OnFlag) != 0;
}

void GUIButtonBase::UpdateRenderElementsInternal()
{
	mImageDesc.Width = mLayoutData.Area.Width;
	mImageDesc.Height = mLayoutData.Area.Height;

	const HSpriteTexture& activeTex = GetActiveTexture();
	if(SpriteTexture::CheckIsLoaded(activeTex))
		mImageDesc.Texture = activeTex;
	else
		mImageDesc.Texture = nullptr;

	mImageDesc.BorderLeft = GetStyleInternal()->Border.Left;
	mImageDesc.BorderRight = GetStyleInternal()->Border.Right;
	mImageDesc.BorderTop = GetStyleInternal()->Border.Top;
	mImageDesc.BorderBottom = GetStyleInternal()->Border.Bottom;
	mImageDesc.Color = GetTint();

	mImageSprite->Update(mImageDesc, (u64)GetParentWidgetInternal());
	mTextSprite->Update(GetTextDesc(), (u64)GetParentWidgetInternal());

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
			contentWidth = Math::RoundToInt(contentWidth * horzRatio);
			contentHeight = Math::RoundToInt(contentHeight * horzRatio);
		}
		else
		{
			contentWidth = Math::RoundToInt(contentWidth * vertRatio);
			contentHeight = Math::RoundToInt(contentHeight * vertRatio);
		}

		IMAGE_SPRITE_DESC contentImgDesc;
		contentImgDesc.Texture = image;
		contentImgDesc.Width = contentWidth;
		contentImgDesc.Height = contentHeight;
		contentImgDesc.Color = GetTint();
		contentImgDesc.AnimationStartTime = mContentAnimationStartTime;

		mContentImageSprite->Update(contentImgDesc, (u64)GetParentWidgetInternal());
	}

	// Populate GUI render elements from the sprites
	{
		using T = impl::GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mImageSprite, 1), T::SpriteInfo(mTextSprite), T::SpriteInfo(mContentImageSprite) }, mRenderElements);
	}

	GUIElement::UpdateRenderElementsInternal();
}

Vector2I GUIButtonBase::GetOptimalSizeInternal() const
{
	u32 imageWidth = 0;
	u32 imageHeight = 0;

	const HSpriteTexture& activeTex = GetActiveTexture();
	if(SpriteTexture::CheckIsLoaded(activeTex))
	{
		imageWidth = activeTex->GetWidth();
		imageHeight = activeTex->GetHeight();
	}

	Vector2I contentSize = GUIHelper::CalcOptimalContentsSize(mContent, *GetStyleInternal(), GetDimensionsInternal(), mActiveState);
	u32 contentWidth = std::max(imageWidth, (u32)contentSize.X);
	u32 contentHeight = std::max(imageHeight, (u32)contentSize.Y);

	return Vector2I(contentWidth, contentHeight);
}

u32 GUIButtonBase::GetRenderElementDepthRangeInternal() const
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

	u32 textSpriteIdx = mImageSprite->GetNumRenderElements();
	u32 contentImgSpriteIdx = textSpriteIdx + mTextSprite->GetNumRenderElements();

	if(renderElementIdx < textSpriteIdx)
	{
		Vector2I imageOffset = Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y) + offset;

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

		if(GetStyleInternal()->ImagePosition == GUIImagePosition::Right)
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

bool GUIButtonBase::MouseEventInternal(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseOver)
	{
		if(!IsDisabledInternal())
		{
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
		if(!IsDisabledInternal())
		{
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
		if(!IsDisabledInternal())
			SetStateInternal(IsOnInternal() ? GUIElementState::ActiveOn : GUIElementState::Active);

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(ev.GetType() == GUIMouseEventType::MouseUp)
	{
		if(!IsDisabledInternal())
		{
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
		if(!IsDisabledInternal())
			OnDoubleClick();

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}

	return false;
}

bool GUIButtonBase::CommandEventInternal(const GUICommandEvent& ev)
{
	const bool baseReturnValue = GUIElement::CommandEventInternal(ev);

	GUIElementState state = (GUIElementState)((u32)mActiveState & (u32)GUIElementState::TypeMask);
	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		mHasFocus = true;

		if(!IsDisabledInternal())
		{
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

		if(state == GUIElementState::Focused)
			SetStateInternal(IsOnInternal() ? GUIElementState::NormalOn : GUIElementState::Normal);
		else if(state == GUIElementState::FocusedHover)
			SetStateInternal(IsOnInternal() ? GUIElementState::HoverOn : GUIElementState::Hover);

		return true;
	}

	return baseReturnValue;
}

String GUIButtonBase::GetTooltipInternal() const
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

TEXT_SPRITE_DESC GUIButtonBase::GetTextDesc() const
{
	TEXT_SPRITE_DESC textDesc;
	textDesc.Text = mContent.Text;
	textDesc.Font = GetStyleInternal()->Font;
	textDesc.FontSize = GetStyleInternal()->FontSize;
	textDesc.Color = GetTint() * GetActiveTextColor();

	Rect2I textBounds = GetCachedContentBounds();

	textDesc.Width = textBounds.Width;
	textDesc.Height = textBounds.Height;
	textDesc.HorzAlign = GetStyleInternal()->TextHorzAlign;
	textDesc.VertAlign = GetStyleInternal()->TextVertAlign;

	return textDesc;
}

void GUIButtonBase::StyleUpdated()
{
	mImageDesc.AnimationStartTime = GetTime().GetTime();
}

void GUIButtonBase::SetStateInternal(GUIElementState state)
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

	if(mActiveState != state)
		mImageDesc.AnimationStartTime = GetTime().GetTime();

	mActiveState = state;
	RefreshContentSprite();
	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirtyInternal();
	else
		MarkContentAsDirtyInternal();
}

const HSpriteTexture& GUIButtonBase::GetActiveTexture() const
{
	switch(mActiveState)
	{
	case GUIElementState::Normal:
		return GetStyleInternal()->Normal.Texture;
	case GUIElementState::Hover:
		return GetStyleInternal()->Hover.Texture;
	case GUIElementState::Active:
		return GetStyleInternal()->Active.Texture;
	case GUIElementState::Focused:
		return GetStyleInternal()->Focused.Texture;
	case GUIElementState::FocusedHover:
		return GetStyleInternal()->FocusedHover.Texture;
	case GUIElementState::NormalOn:
		return GetStyleInternal()->NormalOn.Texture;
	case GUIElementState::HoverOn:
		return GetStyleInternal()->HoverOn.Texture;
	case GUIElementState::ActiveOn:
		return GetStyleInternal()->ActiveOn.Texture;
	case GUIElementState::FocusedOn:
		return GetStyleInternal()->FocusedOn.Texture;
	case GUIElementState::FocusedHoverOn:
		return GetStyleInternal()->FocusedHoverOn.Texture;
	default:
		break;
	}

	return GetStyleInternal()->Normal.Texture;
}

Color GUIButtonBase::GetActiveTextColor() const
{
	switch(mActiveState)
	{
	case GUIElementState::Normal:
		return GetStyleInternal()->Normal.TextColor;
	case GUIElementState::Hover:
		return GetStyleInternal()->Hover.TextColor;
	case GUIElementState::Active:
		return GetStyleInternal()->Active.TextColor;
	case GUIElementState::Focused:
		return GetStyleInternal()->Focused.TextColor;
	case GUIElementState::FocusedHover:
		return GetStyleInternal()->FocusedHover.TextColor;
	case GUIElementState::NormalOn:
		return GetStyleInternal()->NormalOn.TextColor;
	case GUIElementState::HoverOn:
		return GetStyleInternal()->HoverOn.TextColor;
	case GUIElementState::ActiveOn:
		return GetStyleInternal()->ActiveOn.TextColor;
	case GUIElementState::FocusedOn:
		return GetStyleInternal()->FocusedOn.TextColor;
	case GUIElementState::FocusedHoverOn:
		return GetStyleInternal()->FocusedHoverOn.TextColor;
	default:
		break;
	}

	return GetStyleInternal()->Normal.TextColor;
}
