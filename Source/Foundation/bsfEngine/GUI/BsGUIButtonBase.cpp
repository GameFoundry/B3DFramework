//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIButtonBase.h"

#include "BsGUIVectorPaths.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIHelper.h"
#include "StyleSheet/BsGUIStyleSheet.h"
#include "VectorGraphics/BsVectorGraphics.h"

using namespace bs;

GUIButtonBase::GUIButtonBase(const String& styleName, const GUIContent& content, const GUISizeConstraints& dimensions, GUIElementOptions options)
	: GUIElement(styleName, dimensions, options), mContent(content)
{
	mImageSprite = B3DNew<ImageSprite>();
	mBackgroundSprite = B3DNew<VectorSprite>();
	mTextSprite = B3DNew<TextSprite>();

	mImageDesc.AnimationStartTime = GetTime().GetTime();
	mContentAnimationStartTime = mImageDesc.AnimationStartTime;

	SetBackgroundPathBuilder(GUIBackgroundVectorPathBuilder::Get());
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
	Vector2I origSize = mSizeConstraints.CalculateConstrainedSize(GetOptimalSize()).Optimal;
	mContent = content;
	mContentAnimationStartTime = GetTime().GetTime();

	RefreshContentSprite();

	Vector2I newSize = mSizeConstraints.CalculateConstrainedSize(GetOptimalSize()).Optimal;

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
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		mBackgroundSpriteInformation.Width = mLayoutData.Area.Width;
		mBackgroundSpriteInformation.Height = mLayoutData.Area.Height;

		if(mBackgroundPathBuilder)
			mBackgroundSpriteInformation.VectorPath = mBackgroundPathBuilder->BuildPath(Size2UI(mLayoutData.Area.Width, mLayoutData.Area.Height), styleSheetRules);
		else
			mBackgroundSpriteInformation.VectorPath = nullptr;

		mBackgroundSpriteInformation.Color = GetTint();
		mBackgroundSpriteInformation.Color.A *= styleSheetRules.Opacity;

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

	// Calculate content bounds
	const Rect2 backgroundImageBounds(
		0.0f, 0.0f,
		(float)mLayoutData.Area.Width, (float)mLayoutData.Area.Height);

	Rect2 textBounds;
	Rect2 contentImageBounds;

	const Vector2I contentOffset = GetContentOffsetInElementSpace();
	Rect2I contentBounds = GetCachedContentBounds();
	Rect2I textSpriteBounds = mTextSprite->GetBounds(Vector2I(), Rect2I());

	if(mContentImageSprite != nullptr)
	{
		Rect2I contentImageSpriteBounds = mContentImageSprite->GetBounds(Vector2I(), Rect2I());
		i32 imageXOffset = 0;
		i32 textImageSpacing = 0;

		if(textSpriteBounds.Width == 0)
		{
			const u32 freeWidth = (u32)std::max(0, (i32)contentBounds.Width - (i32)textSpriteBounds.Width - (i32)contentImageSpriteBounds.Width);
			imageXOffset = (i32)(freeWidth / 2);
		}
		else
			textImageSpacing = GUIContent::kImageTextSpacing;

		if(GetStyle()->ImagePosition == GUIImagePosition::Right)
		{
			const i32 imageReservedWidth = Math::Max(0, (i32)contentBounds.Width - (i32)textSpriteBounds.Width);

			textBounds.X = (float)contentOffset.X;
			textBounds.Width = (float)Math::Max(0, (i32)contentBounds.Width - imageReservedWidth);

			contentImageBounds.X = (float)(contentOffset.X + textSpriteBounds.Width + imageXOffset + textImageSpacing);
			contentImageBounds.Width = (float)Math::Max(0, imageReservedWidth - imageXOffset);
		}
		else
		{
			const i32 imageReservedWidth = (i32)contentImageSpriteBounds.Width + imageXOffset;

			contentImageBounds.X = (float)(contentOffset.X + imageXOffset);
			contentImageBounds.Width = (float)Math::Min(imageReservedWidth - imageXOffset, (i32)contentBounds.Width);

			textBounds.X = (float)(contentOffset.X + imageReservedWidth + textImageSpacing);
			textBounds.Width = (float)Math::Max(0, (i32)contentBounds.Width - imageReservedWidth);
		}

		textBounds.Y = (float)contentOffset.Y;
		textBounds.Height = (float)contentBounds.Height;

		const float imageYOffset = Math::Max(0, (float)contentBounds.Height - (float)contentImageSpriteBounds.Height) / 2.0f;
		contentImageBounds.Y = (float)contentOffset.Y + (float)imageYOffset;
		contentImageBounds.Height = (float)contentBounds.Height - imageYOffset;
	}
	else
	{
		textBounds.X = (float)contentOffset.X;
		textBounds.Y = (float)contentOffset.Y;
		textBounds.Width = (float)contentBounds.Width;
		textBounds.Height = (float)contentBounds.Height;
	}

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;

		if(isUsingStyleSheets)
		{
			T::Populate({
				T::SpriteInfo(mBackgroundSprite, 1, backgroundImageBounds),
				T::SpriteInfo(mTextSprite, 0, textBounds),
				T::SpriteInfo(mContentImageSprite, 0, contentImageBounds) },
				mRenderElements);
		}
		else
		{
			T::Populate({
				T::SpriteInfo(mImageSprite, 1, backgroundImageBounds),
				T::SpriteInfo(mTextSprite, 0, textBounds),
				T::SpriteInfo(mContentImageSprite, 0, contentImageBounds) },
				mRenderElements);
		}
	}

	GUIElement::UpdateRenderElements();
}

Vector2I GUIButtonBase::GetOptimalSize() const
{
	u32 imageWidth = 0;
	u32 imageHeight = 0;

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		const Size2UI contentSize = GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(mContent, styleSheetRules, GetSizeConstraints().MaxWidth);
		
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

		Vector2I contentSize = GUIHelper::CalculateOptimalContentSize(mContent, *GetStyle(), GetSizeConstraints(), mActiveState);
		u32 contentWidth = std::max(imageWidth, (u32)contentSize.X);
		u32 contentHeight = std::max(imageHeight, (u32)contentSize.Y);

		return Vector2I(contentWidth, contentHeight);
	}
}

u32 GUIButtonBase::GetRenderElementDepthRange() const
{
	return 2;
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
			AddStateFlags(GUIElementStateFlag::Focus);

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
		RemoveStateFlags(GUIElementStateFlag::Focus);

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
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(isUsingStyleSheets)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		TextSpriteInformation textSpriteInformation;
		textSpriteInformation.Text = mContent.Text;
		textSpriteInformation.Font = styleSheetRules.Font;
		textSpriteInformation.FontSize = styleSheetRules.FontSize;
		textSpriteInformation.Color = GetTint() * styleSheetRules.Color;
		textSpriteInformation.Color.A *= styleSheetRules.Opacity;

		Rect2I textBounds = GetCachedContentBounds();

		textSpriteInformation.Width = textBounds.Width;
		textSpriteInformation.Height = textBounds.Height;
		textSpriteInformation.HorzAlign = styleSheetRules.HorizontalTextAlignment;
		textSpriteInformation.VertAlign = styleSheetRules.VerticalTextAlignment;

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
	Vector2I origSize = mSizeConstraints.CalculateConstrainedSize(GetOptimalSize()).Optimal;

	if(mActiveState != state)
		mImageDesc.AnimationStartTime = GetTime().GetTime();

	mActiveState = state;
	RefreshContentSprite();
	Vector2I newSize = mSizeConstraints.CalculateConstrainedSize(GetOptimalSize()).Optimal;

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
