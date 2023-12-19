//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIToggleGroup.h"
#include "BsGUICommandEvent.h"
#include "BsGUIVectorPaths.h"
#include "Image/BsSpriteVectorPath.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

const String& GUIToggle::GetGuiTypeName()
{
	static String name = "Toggle";
	return name;
}

GUIToggle::GUIToggle(PrivatelyConstruct, const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIClickable(styleName, contents.GeneralContent, dimensions), mToggleGroup(nullptr), mIsToggled(false)
{
	if(contents.ToggleGroup != nullptr)
		contents.ToggleGroup->AddInternal(this);

	mCheckmarkSprite = B3DNew<ImageSprite>();
	mCheckmarkPathBuilder = GUICheckmarkVectorPathBuilder::Get();
	mCheckmarkPseudoElementIndex = RegisterPseudoElement("checkmark");
}

GUIToggle::~GUIToggle()
{
	B3DDelete(mCheckmarkSprite);

	if(mToggleGroup != nullptr)
	{
		mToggleGroup->RemoveInternal(this);
	}
}

SPtr<GUIToggleGroup> GUIToggle::CreateToggleGroup(bool allowAllOff)
{
	SPtr<GUIToggleGroup> toggleGroup = B3DMakeSharedFromExisting<GUIToggleGroup>(new(B3DAllocate<GUIToggleGroup>()) GUIToggleGroup(allowAllOff));
	toggleGroup->Initialize(toggleGroup);

	return toggleGroup;
}

void GUIToggle::SetToggleGroupInternal(SPtr<GUIToggleGroup> toggleGroup)
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
					toggleElem->ToggleOff();
			}
			else
			{
				if(toggleElem->mIsToggled)
					isToggled = true;
			}
		}

		if(!isToggled && !toggleGroup->mAllowAllOff)
			ToggleOn();
	}
}

void GUIToggle::ToggleOnInternal(bool triggerEvent)
{
	if(mIsToggled)
		return;

	mIsToggled = true;

	if(triggerEvent)
	{
		if(!OnToggled.Empty())
			OnToggled(mIsToggled);
	}

	if(mToggleGroup != nullptr)
	{
		for(auto& toggleElem : mToggleGroup->mButtons)
		{
			if(toggleElem != this)
				toggleElem->ToggleOffInternal(triggerEvent);
		}
	}

	SetOnInternal(true);
}

void GUIToggle::ToggleOffInternal(bool triggerEvent)
{
	if(!mIsToggled)
		return;

	bool canBeToggledOff = false;
	if(mToggleGroup != nullptr) // If in group ensure at least one element is toggled on
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
	else
		canBeToggledOff = true;

	if(canBeToggledOff || mToggleGroup->mAllowAllOff)
	{
		mIsToggled = false;

		if(triggerEvent)
		{
			if(!OnToggled.Empty())
				OnToggled(mIsToggled);
		}

		SetOnInternal(false);
	}
}

void GUIToggle::UpdateRenderElements()
{
	Super::UpdateRenderElements();

	// No checkmark when not toggled
	if(!mIsToggled)
		return;

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return;

	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mCheckmarkPseudoElementIndex);
	if(ruleInformation.CurrentStateRuleset == nullptr)
		return;

	HSpriteImage contentImage = mContent.GetImage(mActiveState);
	if(contentImage.IsLoaded(false) || !mContent.Text.GetValue().empty())
		return;

	const GUIStyleSheetRules& checkmarkStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	const Rect2I checkmarkBounds = GetCachedContentBoundsInElementSpace();

	mCheckmarkSpriteInformation.Width = checkmarkBounds.Width;
	mCheckmarkSpriteInformation.Height = checkmarkBounds.Height;

	if(mCheckmarkPathBuilder)
	{
		SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
		spriteVectorPathCreateInformation.Size = Size2UI(mCheckmarkSpriteInformation.Width, mCheckmarkSpriteInformation.Height);
		spriteVectorPathCreateInformation.VectorPath = mCheckmarkPathBuilder->BuildPath(spriteVectorPathCreateInformation.Size, checkmarkStyleSheetRules);

		mCheckmarkSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
	}
	else
		mCheckmarkSpriteInformation.Image = nullptr;

	mCheckmarkSpriteInformation.Color = GetTint();
	mCheckmarkSpriteInformation.Color.A *= checkmarkStyleSheetRules.Opacity;

	mCheckmarkSprite->Update(mCheckmarkSpriteInformation, (u64)GetParentWidget());

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;

		T::Append({ T::SpriteInfo(mCheckmarkSprite, 0, (Rect2)checkmarkBounds) }, mRenderElements);
	}
}

bool GUIToggle::DoOnMouseEvent(const GUIMouseEvent& event)
{
	bool processed = GUIClickable::DoOnMouseEvent(event);

	if(event.GetType() == GUIMouseEventType::MouseUp)
	{
		if(!IsDisabled())
		{
			if(mIsToggled)
				ToggleOffInternal(true);
			else
				ToggleOnInternal(true);
		}

		processed = true;
	}

	return processed;
}

bool GUIToggle::DoOnCommandEvent(const GUICommandEvent& event)
{
	const bool processed = GUIClickable::DoOnCommandEvent(event);

	if(event.GetType() == GUICommandEventType::Confirm)
	{
		if(!IsDisabled())
		{
			if(mIsToggled)
				ToggleOffInternal(true);
			else
				ToggleOnInternal(true);
		}

		return true;
	}

	return processed;
}
