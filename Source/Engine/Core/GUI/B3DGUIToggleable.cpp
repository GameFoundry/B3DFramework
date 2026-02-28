//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIToggleable.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUIToggleGroup.h"
#include "B3DGUICommandEvent.h"
#include "B3DGUIUtility.h"
#include "B3DGUIVectorPaths.h"
#include "Image/B3DSpriteVectorPath.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

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
		for(auto& toggleElement : mToggleGroup->mButtons)
		{
			if(isToggled)
			{
				if(toggleElement->mIsToggled)
					toggleElement->SetIsToggled(false);
			}
			else
			{
				if(toggleElement->mIsToggled)
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
				for(auto& toggleElement : mToggleGroup->mButtons)
				{
					if(toggleElement != this)
					{
						if(toggleElement->mIsToggled)
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
			for(auto& toggleElement : mToggleGroup->mButtons)
			{
				if(toggleElement != this)
					toggleElement->SetIsToggled(false, triggerEvent);
			}
		}
	}

	SetOnInternal(mIsToggled);
}

GUILogicalSize GUIToggleable::CalculateCheckmarkContentAreaSize(const GUILogicalSize& elementOptimalSize) const
{
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return GUILogicalSize::kZero;

	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mCheckmarkPseudoElementIndex);

	const GUIStyleSheetRules& checkmarkStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	if(checkmarkStyleSheetRules.Visibility == GUIElementVisibility::Hidden)
		return GUILogicalSize::kZero;

	// Determine element's content area, only used for deriving the checkbox area size if one is not provided in the style-sheet
	const GUISizeConstraints& elementSizeConstraints = GetSizeConstraints();
	const GUIStyleSheetRules& elementStyleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

	const GUILogicalSize elementConstrainedOptimalSize = elementSizeConstraints.CalculateConstrainedOptimalSize(elementOptimalSize);
	const GUILogicalArea elementContentArea = GUIUtility::RemovePaddingAndBorder(elementConstrainedOptimalSize, elementStyleSheetRules);

	// Determine the size of the area in which to fit the checkmark. Actual checkmark will be scaled to fit (without cropping/stretching) in this area
	GUILogicalSize checkmarkAreaSize(kZeroTag);
	if(!elementContentArea.IsEmpty())
	{
		// If background image is provided, derive the checkbox area in a way so that aspect ratio matches the image
		if(checkmarkStyleSheetRules.BackgroundImage.IsLoaded(false))
		{
			const Size2I imageSize = checkmarkStyleSheetRules.BackgroundImage->GetDefaultAllocatedImage().GetSize();

			const float aspectRatioX = (float)imageSize.Width / (float)elementContentArea.Width;
			const float aspectRatioY = (float)imageSize.Height / (float)elementContentArea.Height;

			if(aspectRatioY > aspectRatioX)
			{
				checkmarkAreaSize.Width = Math::RoundToI32((float)imageSize.Width / aspectRatioY);
				checkmarkAreaSize.Height = Math::RoundToI32((float)imageSize.Height / aspectRatioY);
			}
			else
			{
				checkmarkAreaSize.Width = Math::RoundToI32((float)imageSize.Width / aspectRatioX);
				checkmarkAreaSize.Height = Math::RoundToI32((float)imageSize.Height / aspectRatioX);
			}
		}
		else if(mCheckmarkPathBuilder)
		{
			checkmarkAreaSize.Width = elementContentArea.Width;
			checkmarkAreaSize.Height = elementContentArea.Height;
		}
		else
		{
			// No checkmark sprite specified, so checkmark area is zero
		}
	}

	return mCheckmarkSizeConstraints.CalculateConstrainedOptimalSize(checkmarkAreaSize);
}

GUILogicalSize GUIToggleable::CalculateUnconstrainedOptimalSize() const
{
	const GUILogicalSize optimalSize = GUIClickable::CalculateUnconstrainedOptimalSize();

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return optimalSize;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const GUILogicalArea contentArea = GUIUtility::RemovePaddingAndBorder(optimalSize, styleSheetRules);

	const GUILogicalSize checkmarkSize = CalculateCheckmarkContentAreaSize(optimalSize);
	GUILogicalSize contentSizeWithCheckMark(contentArea.Width, contentArea.Height);

	if(checkmarkSize.Width > 0)
	{
		contentSizeWithCheckMark.Width += kCheckmarkContentSpacing + checkmarkSize.Width;
		contentSizeWithCheckMark.Height = Math::Max(contentSizeWithCheckMark.Height, checkmarkSize.Height);
	}

	return GUIUtility::CalculateSizeWithPaddingAndBorder(contentSizeWithCheckMark, styleSheetRules);
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
	const GUIPhysicalSize checkmarkAreaSize = CalculateCheckmarkContentAreaSize(GetAbsoluteBounds().GetSize().To<GUILogicalUnit>()).To<GUIPhysicalUnit>();

	// Use user-provided image, if one is provided
	bool showCheckmarkSprite = false;
	if(checkmarkStyleSheetRules.BackgroundImage.IsLoaded(false))
	{
		const Size2I& imageSize = checkmarkStyleSheetRules.BackgroundImage->GetDefaultAllocatedImage().GetSize();

		mCheckmarkSpriteInformation.Size = imageSize;
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
			spriteVectorPathCreateInformation.DefaultSize = checkmarkAreaSize.To<i32>();
			spriteVectorPathCreateInformation.VectorPath = mCheckmarkPathBuilder->BuildPath(spriteVectorPathCreateInformation.DefaultSize, checkmarkStyleSheetRules);

			mCheckmarkSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
			mCheckmarkSpriteInformation.Color = Color::kWhite;
			mCheckmarkSpriteInformation.Size = checkmarkAreaSize.To<i32>();

			showCheckmarkSprite = true;
		}
	}

	mRenderElements.clear();
	GUISpriteHelper::BuildSpriteRenderElements(*this, mActiveState, mBackgroundSprite);

	const u64 batchId = (u64)GetParentWidget();
	const Color& tint = GetTint();

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

	GUIContentSpriteCreateInformation contentSpriteCreateInformation(Size2UI::kZero, mContent, styleSheetRules, tint, mAbsoluteScale, batchId);

	const GUIPhysicalUnit physicalCheckmarkContentSpacing = GUIUtility::LogicalToPhysical(kCheckmarkContentSpacing, GetAbsoluteScale());

	const GUIPhysicalArea& scaledContentAreaBounds = GetScaledContentBounds();
	GUIPhysicalArea contentArea(
		scaledContentAreaBounds.X + physicalCheckmarkContentSpacing + checkmarkAreaSize.Width,
		scaledContentAreaBounds.Y,
		scaledContentAreaBounds.Width - physicalCheckmarkContentSpacing - checkmarkAreaSize.Width,
		scaledContentAreaBounds.Height);

	contentSpriteCreateInformation.ContentArea = contentArea.To<i32, u32>();

	// Offset to center the checkmark sprite within the checkmark content area
	const Vector2 checkmarkCenterOffset = Vector2(
		((float)checkmarkAreaSize.Width - (float)mCheckmarkSpriteInformation.Size.Width) / 2.0f,
		((float)checkmarkAreaSize.Height - (float)mCheckmarkSpriteInformation.Size.Height) / 2.0f);

	const Area2 checkmarkContentArea = Area2((float)scaledContentAreaBounds.X, (float)scaledContentAreaBounds.Y, (float)checkmarkAreaSize.Width, (float)checkmarkAreaSize.Height);
	const Vector2 checkmarkOffset((float)checkmarkContentArea.X + checkmarkCenterOffset.X, (float)checkmarkContentArea.Y + checkmarkCenterOffset.Y);

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

			T::Append({ T::SpriteInfo(mCheckmarkSprite, 0, checkmarkOffset, checkmarkContentArea) }, mRenderElements);
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

void GUIToggleable::NotifyStyleChanged()
{
	GUIClickable::NotifyStyleChanged();

	// If not drawing a checkmark using style-sheets, fall back to parent implementation
	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return;

	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mCheckmarkPseudoElementIndex);
	if(ruleInformation.CurrentStateRuleset == nullptr)
		return;

	const GUIStyleSheetRules& checkmarkStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	mCheckmarkSizeConstraints.UpdateWithStyleSheetRule(checkmarkStyleSheetRules);
}

