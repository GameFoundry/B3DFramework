//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIToggleable.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIToggleGroup.h"
#include "BsGUICommandEvent.h"
#include "BsGUIHelper.h"
#include "BsGUIVectorPaths.h"
#include "Image/BsSpriteVectorPath.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

GUIToggleable::GUIToggleable(const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIClickable(styleName, contents.GeneralContent, dimensions), mToggleGroup(nullptr), mIsToggled(false)
{
	if(contents.ToggleGroup != nullptr)
		contents.ToggleGroup->AddInternal(this);

	mCheckmarkSprite = B3DNew<ImageSprite>();
	mCheckmarkPathBuilder = GUICheckmarkVectorPathBuilder::Get();
	mCheckmarkPseudoElementIndex = RegisterPseudoElement("checkmark");
}

GUIToggleable::~GUIToggleable()
{
	B3DDelete(mCheckmarkSprite);

	if(mToggleGroup != nullptr)
	{
		mToggleGroup->RemoveInternal(this);
	}
}

void GUIToggleable::SetToggleGroupInternal(SPtr<GUIToggleGroup> toggleGroup)
{
	mToggleGroup = toggleGroup;

	bool isToggled = false;
	if(mToggleGroup != nullptr) // If in group ensure at least one element is toggled on
	{
		for(auto& toggleElem : mToggleGroup->mButtons)
		{
			if(isToggled)
			{
				if(toggleElem->mIsToggled)
					toggleElem->SetIsToggled(false);
			}
			else
			{
				if(toggleElem->mIsToggled)
					isToggled = true;
			}
		}

		if(!isToggled && !toggleGroup->mAllowAllOff)
			SetIsToggled(true);
	}
}

void GUIToggleable::SetIsToggled(bool isToggled, bool triggerEvent)
{
	if(mIsToggled == isToggled)
		return;

	if(!isToggled)
	{
		bool canBeToggledOff = false;
		if(mToggleGroup != nullptr) // If in group ensure at least one element is toggled on
		{

			if(mToggleGroup->mAllowAllOff)
				canBeToggledOff = true;
			else
			{
				for(auto& toggleElem : mToggleGroup->mButtons)
				{
					if(toggleElem != this)
					{
						if(toggleElem->mIsToggled)
						{
							canBeToggledOff = true;
							break;
						}
					}
				}
			}
		}
		else
			canBeToggledOff = true;

		if(!canBeToggledOff)
			return;
	}

	mIsToggled = isToggled;

	if(triggerEvent)
	{
		if(!OnToggled.Empty())
			OnToggled(mIsToggled);
	}

	if(isToggled)
	{
		if(mToggleGroup != nullptr)
		{
			for(auto& toggleElem : mToggleGroup->mButtons)
			{
				if(toggleElem != this)
					toggleElem->SetIsToggled(false, triggerEvent);
			}
		}
	}

	SetOnInternal(mIsToggled);
}

Size2UI GUIToggleable::CalculateCheckmarkSize(const Size2UI& elementOptimalSize) const
{
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return Size2UI::kZero;

	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mCheckmarkPseudoElementIndex);

	const GUIStyleSheetRules& checkmarkStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	if(checkmarkStyleSheetRules.Visibility == GUIElementVisibility::Hidden)
		return Size2UI::kZero;

	const GUISizeConstraints& sizeConstraints = GetSizeConstraints();
	const Vector2I constrainedOptimalSize = sizeConstraints.CalculateConstrainedSize(elementOptimalSize).Optimal;

	// If no content and no fixed size set, then default to some value
	Size2UI actualOptimalSize;
	if(constrainedOptimalSize.X == 0 || constrainedOptimalSize.Y == 0)
		actualOptimalSize = kDefaultCheckmarkSize;
	else
		actualOptimalSize = Size2UI((u32)constrainedOptimalSize.X, (u32)constrainedOptimalSize.Y);

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const Rect2I contentArea = GUIHelper::CalculateContentArea(actualOptimalSize, styleSheetRules);

	if(checkmarkStyleSheetRules.BackgroundImage.IsLoaded(false))
	{
		const Size2UI imageSize = checkmarkStyleSheetRules.BackgroundImage->GetSize();
		const float backgroundImageAspectRatio = (float)imageSize.Width / (float)imageSize.Height;

		// Background image, scaled to fit the content area height, while respecting aspect ratio
		return Size2UI(Math::RoundToU32((float)contentArea.Height * backgroundImageAspectRatio), contentArea.Height);
	}
	else if(mCheckmarkPathBuilder)
	{
		return Size2UI(contentArea.Height, contentArea.Height);
	}

	return Size2UI::kZero;
}

Vector2I GUIToggleable::CalculateUnconstrainedOptimalSize() const
{
	const Vector2I optimalSize = GUIClickable::CalculateUnconstrainedOptimalSize();

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return optimalSize;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const Rect2I contentArea = GUIHelper::CalculateContentArea(Size2UI(optimalSize.X, optimalSize.Y), styleSheetRules);

	const Size2UI checkmarkSize = CalculateCheckmarkSize(Size2UI((u32)optimalSize.X, (u32)optimalSize.Y));
	Size2UI contentSizeWithCheckMark(contentArea.Width, contentArea.Height);

	if(checkmarkSize.Width > 0)
	{
		contentSizeWithCheckMark.Width += kCheckmarkContentSpacing + checkmarkSize.Width;
		contentSizeWithCheckMark.Height = Math::Max(contentSizeWithCheckMark.Height, checkmarkSize.Height);
	}

	const Size2UI optimalSizeWithCheckmark = GUIHelper::CalculateSizeWithPaddingAndBorder(contentSizeWithCheckMark, styleSheetRules);
	return Vector2I((i32)optimalSizeWithCheckmark.Width, (i32)optimalSizeWithCheckmark.Height);
}

void GUIToggleable::UpdateRenderElements()
{
	// If not drawing a checkmark using style-sheets, fall back to parent implementation
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
	{
		GUIClickable::UpdateRenderElements();
		return;
	}

	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mCheckmarkPseudoElementIndex);
	if(ruleInformation.CurrentStateRuleset == nullptr)
	{
		GUIClickable::UpdateRenderElements();
		return;
	}

	const GUIStyleSheetRules& checkmarkStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	if(checkmarkStyleSheetRules.Visibility == GUIElementVisibility::Hidden)
	{
		GUIClickable::UpdateRenderElements();
		return;
	}

	// Otherwise, create the checkmark sprite and offset the parent's contents to make room
	const Size2UI checkmarkSize = CalculateCheckmarkSize(GetCachedBounds().GetSize());

	mCheckmarkSpriteInformation.Width = checkmarkSize.Width;
	mCheckmarkSpriteInformation.Height = checkmarkSize.Height;

	// Use user-provided image, if one is provided
	bool showCheckmarkSprite = false;
	if(checkmarkStyleSheetRules.BackgroundImage.IsLoaded(false))
	{
		mCheckmarkSpriteInformation.Image = checkmarkStyleSheetRules.BackgroundImage;
		mCheckmarkSpriteInformation.Color = checkmarkStyleSheetRules.Color;

		showCheckmarkSprite = true;
	}
	// Otherwise, use the default checkmark builder
	else if(mCheckmarkPathBuilder)
	{
		// No checkmark when not toggled
		if(mIsToggled)
		{
			SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
			spriteVectorPathCreateInformation.Size = Size2UI(mCheckmarkSpriteInformation.Width, mCheckmarkSpriteInformation.Height);
			spriteVectorPathCreateInformation.VectorPath = mCheckmarkPathBuilder->BuildPath(spriteVectorPathCreateInformation.Size, checkmarkStyleSheetRules);

			mCheckmarkSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
			mCheckmarkSpriteInformation.Color = Color::kWhite;

			showCheckmarkSprite = true;
		}
	}

	mRenderElements.clear();
	GUISpriteHelper::BuildSpriteRenderElements(*this, mActiveState, mBackgroundSprite);

	const u64 batchId = (u64)GetParentWidget();
	const Color& tint = GetTint();

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

	GUIContentSpriteCreateInformation contentSpriteCreateInformation(Size2UI::kZero, mContent, styleSheetRules, tint, batchId);

	const Rect2I& contentAreaBounds = GetCachedContentBoundsInElementSpace();
	contentSpriteCreateInformation.ContentArea = Rect2I(contentAreaBounds.X + (i32)kCheckmarkContentSpacing + (i32)checkmarkSize.Width, contentAreaBounds.Y,
		contentAreaBounds.Width - (i32)kCheckmarkContentSpacing - checkmarkSize.Width, contentAreaBounds.Height);

	const Rect2I checkmarkContentArea = Rect2I(contentAreaBounds.X, contentAreaBounds.Y, checkmarkSize.Width, checkmarkSize.Height);

	mContentSprites.BuildRenderElements(contentSpriteCreateInformation, mRenderElements);

	// Note: Purposefully skipping the parent's implementation, as we add its sprites above
	GUIInteractable::UpdateRenderElements();

	if(showCheckmarkSprite)
	{
		mCheckmarkSpriteInformation.Color *= GetTint();
		mCheckmarkSpriteInformation.Color.A *= checkmarkStyleSheetRules.Opacity;
		mCheckmarkSprite->Update(mCheckmarkSpriteInformation, (u64)GetParentWidget());

		// Populate GUI render elements from the sprites
		{
			using T = GUIRenderElementHelper;

			T::Append({ T::SpriteInfo(mCheckmarkSprite, 0, (Rect2)checkmarkContentArea) }, mRenderElements);
		}
	}
}

bool GUIToggleable::DoOnMouseEvent(const GUIMouseEvent& event)
{
	bool processed = GUIClickable::DoOnMouseEvent(event);

	if(event.GetType() == GUIMouseEventType::MouseUp)
	{
		if(!IsDisabled())
			SetIsToggled(!mIsToggled, true);

		processed = true;
	}

	return processed;
}

bool GUIToggleable::DoOnCommandEvent(const GUICommandEvent& event)
{
	const bool processed = GUIClickable::DoOnCommandEvent(event);

	if(event.GetType() == GUICommandEventType::Confirm)
	{
		if(!IsDisabled())
			SetIsToggled(!mIsToggled, true);

		return true;
	}

	return processed;
}
