//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIClickable.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIHelper.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

GUIClickable::GUIClickable(const String& styleName, const GUIContent& content, const GUISizeConstraints& sizeConstraints, GUIElementOptions options)
	: GUIInteractable(styleName, sizeConstraints, options), mContent(content)
{
	mBackgroundSprite.SetAnimationStartTime(GetTime().GetTime());
	mContentSprites.SetAnimationStartTime(GetTime().GetTime());
}

void GUIClickable::SetContent(const GUIContent& content)
{
	Vector2I origSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
	mContent = content;

	mContentSprites.SetAnimationStartTime(GetTime().GetTime());

	Vector2I newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUIClickable::SetOnInternal(bool on)
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

bool GUIClickable::IsOnInternal() const
{
	return ((i32)mActiveState & (i32)GUIElementState::OnFlag) != 0;
}

void GUIClickable::UpdateRenderElements()
{
	mRenderElements.clear();
	GUISpriteHelper::BuildSpriteRenderElements(*this, mActiveState, mBackgroundSprite);
	GUISpriteHelper::BuildSpriteRenderElements(*this, mActiveState, mContent, mContentSprites);

	GUIInteractable::UpdateRenderElements();
}

Vector2I GUIClickable::CalculateUnconstrainedOptimalSize() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return Vector2I::kZero;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const Size2UI contentSize = GUIHelper::CalculateOptimalContentSizeWithPaddingAndBorder(mContent, styleSheetRules, GetSizeConstraints().MaxWidth);
	
	return Vector2I((i32)contentSize.Width, (i32)contentSize.Height);
}

u32 GUIClickable::GetRenderElementDepthRange() const
{
	return 2;
}

bool GUIClickable::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	if(mOptionFlags.IsSet(GUIElementOption::IgnorePointerEvents))
		return false;

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

bool GUIClickable::DoOnCommandEvent(const GUICommandEvent& ev)
{
	const bool baseReturnValue = GUIInteractable::DoOnCommandEvent(ev);

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

String GUIClickable::GetTooltip() const
{
	return (String)mContent.Tooltip;
}

void GUIClickable::NotifyStyleChanged()
{
	mBackgroundSprite.SetAnimationStartTime(GetTime().GetTime());
}

void GUIClickable::SetStateInternal(GUIElementState state)
{
	Vector2I origSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

	if(mActiveState != state)
		mBackgroundSprite.SetAnimationStartTime(GetTime().GetTime());

	mActiveState = state;

	Vector2I newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}
