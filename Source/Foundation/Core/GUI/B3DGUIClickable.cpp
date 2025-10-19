//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIClickable.h"

#include "B3DGUIUtility.h"
#include "Image/B3DSpriteTexture.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUICommandEvent.h"
#include "StyleSheet/B3DGUIStyleSheet.h"
#include "Utility/B3DTime.h"

using namespace b3d;

GUIClickable::GUIClickable(const String& styleName, const GUIContent& content, const GUISizeConstraints& sizeConstraints, GUIElementOptions options)
	: GUIInteractable(styleName, sizeConstraints, options), mContent(content)
{
	mBackgroundSprite.SetAnimationStartTime(GetTime().GetRealTimeInSeconds());
	mContentSprites.SetAnimationStartTime(GetTime().GetRealTimeInSeconds());
}

void GUIClickable::SetContent(const GUIContent& content)
{
	GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());
	mContent = content;

	mContentSprites.SetAnimationStartTime(GetTime().GetRealTimeInSeconds());

	GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());

	if(originalSize != newSize)
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

GUILogicalSize GUIClickable::CalculateUnconstrainedOptimalSize() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return GUILogicalSize::kZero;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	return GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(mContent, styleSheetRules, GetSizeConstraints().MaximumWidth);
}

u32 GUIClickable::GetRenderElementDepthRange() const
{
	return 2;
}

bool GUIClickable::DoOnMouseEvent(const GUIMouseEvent& event)
{
	if(mOptionFlags.IsSet(GUIElementOption::IgnorePointerEvents))
		return false;

	if(event.GetType() == GUIMouseEventType::MouseOver)
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
	else if(event.GetType() == GUIMouseEventType::MouseOut)
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
	else if(event.GetType() == GUIMouseEventType::MouseDown)
	{
		if(!IsDisabled())
		{
			AddStateFlags(GUIElementStateFlag::Active);
			SetStateInternal(IsOnInternal() ? GUIElementState::ActiveOn : GUIElementState::Active);
		}

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(event.GetType() == GUIMouseEventType::MouseUp)
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
	else if(event.GetType() == GUIMouseEventType::MouseDoubleClick)
	{
		if(!IsDisabled())
			OnDoubleClick();

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}

	return false;
}

bool GUIClickable::DoOnCommandEvent(const GUICommandEvent& event)
{
	const bool baseReturnValue = GUIInteractable::DoOnCommandEvent(event);

	GUIElementState state = (GUIElementState)((u32)mActiveState & (u32)GUIElementState::TypeMask);
	if(event.GetType() == GUICommandEventType::FocusGained)
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
	else if(event.GetType() == GUICommandEventType::FocusLost)
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
	mBackgroundSprite.SetAnimationStartTime(GetTime().GetRealTimeInSeconds());
}

void GUIClickable::SetStateInternal(GUIElementState state)
{
	GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());

	if(mActiveState != state)
		mBackgroundSprite.SetAnimationStartTime(GetTime().GetRealTimeInSeconds());

	mActiveState = state;

	GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());

	if(originalSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}
