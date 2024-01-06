//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIToggleable.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIToggleGroup.h"
#include "BsGUICommandEvent.h"
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

void GUIToggleable::UpdateRenderElements()
{
	Super::UpdateRenderElements();

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return;

	// Draw checkmark if needed
	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mCheckmarkPseudoElementIndex);
	if(ruleInformation.CurrentStateRuleset == nullptr)
		return;

	const GUIStyleSheetRules& checkmarkStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	if(checkmarkStyleSheetRules.Visibility == GUIElementVisibility::Hidden)
		return;

	const Rect2I checkmarkBounds = GetCachedContentBoundsInElementSpace();

	mCheckmarkSpriteInformation.Width = checkmarkBounds.Width;
	mCheckmarkSpriteInformation.Height = checkmarkBounds.Height;

	// Use user-provided image, if one is provided
	if(checkmarkStyleSheetRules.BackgroundImage.IsLoaded(false))
	{
		mCheckmarkSpriteInformation.Image = checkmarkStyleSheetRules.BackgroundImage;
		mCheckmarkSpriteInformation.Color = checkmarkStyleSheetRules.Color;
	}
	// Otherwise, use the default checkmark builder
	else if(mCheckmarkPathBuilder)
	{
		// No checkmark when not toggled
		if(!mIsToggled)
			return;

		SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
		spriteVectorPathCreateInformation.Size = Size2UI(mCheckmarkSpriteInformation.Width, mCheckmarkSpriteInformation.Height);
		spriteVectorPathCreateInformation.VectorPath = mCheckmarkPathBuilder->BuildPath(spriteVectorPathCreateInformation.Size, checkmarkStyleSheetRules);

		mCheckmarkSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
		mCheckmarkSpriteInformation.Color = Color::kWhite;
	}
	else
		return;

	mCheckmarkSpriteInformation.Color *= GetTint();
	mCheckmarkSpriteInformation.Color.A *= checkmarkStyleSheetRules.Opacity;

	mCheckmarkSprite->Update(mCheckmarkSpriteInformation, (u64)GetParentWidget());

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;

		T::Append({ T::SpriteInfo(mCheckmarkSprite, 0, (Rect2)checkmarkBounds) }, mRenderElements);
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
