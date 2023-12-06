//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIListBox.h"

#include "BsGUIHelper.h"
#include "BsGUIVectorPaths.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUIDropDownBoxManager.h"
#include "Image/BsSpriteVectorPath.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

const String& GUIListBox::GetGuiTypeName()
{
	static String name = "ListBox";
	return name;
}

GUIListBox::GUIListBox(const String& styleName, const Vector<HString>& elements, bool isMultiselect, const GUISizeConstraints& dimensions)
	: GUIButtonBase(styleName, GUIContent(HString("")), dimensions), mElements(elements), mIsMultiselect(isMultiselect)
{
	mElementStates.resize(elements.size(), false);
	if(!mIsMultiselect && mElementStates.size() > 0)
		mElementStates[0] = true;

	mArrowSprite = B3DNew<ImageSprite>();
	mArrowPathBuilder = GUIDropDownArrowVectorPathBuilder::Get();
	mArrowPseudoElementIndex = RegisterPseudoElement("arrow");

	UpdateContents();
}

GUIListBox::~GUIListBox()
{
	CloseListBox();

	B3DDelete(mArrowSprite);
}

GUIListBox* GUIListBox::Create(const Vector<HString>& elements, bool isMultiselect, const String& styleName)
{
	return new(B3DAllocate<GUIListBox>()) GUIListBox(GetStyleName<GUIListBox>(styleName), elements, isMultiselect, GUISizeConstraints::Create());
}

GUIListBox* GUIListBox::Create(const Vector<HString>& elements, bool isMultiselect, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIListBox>()) GUIListBox(GetStyleName<GUIListBox>(styleName), elements, isMultiselect, GUISizeConstraints::Create(options));
}

GUIListBox* GUIListBox::Create(const Vector<HString>& elements, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIListBox>()) GUIListBox(GetStyleName<GUIListBox>(styleName), elements, false, GUISizeConstraints::Create(options));
}

void GUIListBox::SetElements(const Vector<HString>& elements)
{
	bool wasOpen = mDropDownBox != nullptr;

	if(wasOpen)
		CloseListBox();

	mElements = elements;

	mElementStates.clear();
	mElementStates.resize(mElements.size(), false);
	if(!mIsMultiselect && mElementStates.size() > 0)
		mElementStates[0] = true;

	UpdateContents();

	if(wasOpen)
		OpenListBox();
}

void GUIListBox::SelectElement(u32 idx)
{
	if(idx >= (u32)mElements.size())
		return;

	if(mElementStates[idx] != true)
		ElementSelected(idx);
}

void GUIListBox::DeselectElement(u32 idx)
{
	if(!mIsMultiselect || idx >= (u32)mElements.size())
		return;

	if(mElementStates[idx] != false)
		ElementSelected(idx);
}

void GUIListBox::SetElementStates(const Vector<bool>& states)
{
	u32 numElements = (u32)mElementStates.size();
	u32 min = std::min(numElements, (u32)states.size());

	bool anythingModified = min != numElements;
	if(!anythingModified)
	{
		for(u32 i = 0; i < numElements; i++)
		{
			if(mElementStates[i] != states[i])
			{
				anythingModified = true;
				break;
			}
		}
	}

	if(!anythingModified)
		return;

	bool wasOpen = mDropDownBox != nullptr;

	if(wasOpen)
		CloseListBox();

	for(u32 i = 0; i < min; i++)
	{
		mElementStates[i] = states[i];

		if(mElementStates[i] && !mIsMultiselect)
		{
			for(u32 j = i + 1; j < numElements; j++)
				mElementStates[j] = false;

			break;
		}
	}

	UpdateContents();

	if(wasOpen)
		OpenListBox();
}

Rect2I GUIListBox::GetCachedContentBoundsInElementSpace() const
{
	const Rect2I& cachedBounds = GetCachedBounds();

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		Size2UI layoutSize(cachedBounds.Width, cachedBounds.Height);

		const u32 arrowAreaWidth = GetArrowCachedContentSize().Width;
		layoutSize.Width = Math::Max(0, (i32)layoutSize.Width - arrowAreaWidth);

		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		return GUIHelper::CalculateContentArea(layoutSize, styleSheetRules);
	}
	else
	{
		const Size2UI layoutSize(cachedBounds.Width, cachedBounds.Height);
		return GUIHelper::CalculateContentArea(layoutSize, *GetStyle());
	}
}

Rect2I GUIListBox::GetArrowCachedContentBoundsInElementSpace() const
{
	Rect2I output = Rect2I::kEmpty;

	if(!IsUsingStyleSheets() || mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return output;

	const Rect2I& cachedBounds = GetCachedBounds();
	const Size2UI layoutSize(cachedBounds.Width, cachedBounds.Height);

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const Rect2I& fullContentArea = GUIHelper::CalculateContentArea(layoutSize, styleSheetRules);

	const GUIStyleSheetRuleInformation& arrowRuleInformation = GetPseudoElementStyleSheetRuleInformation(mArrowPseudoElementIndex);
	if(arrowRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& arrowRules = arrowRuleInformation.CurrentStateRuleset->Rules;
		output = GUIHelper::CalculateContentArea(GetArrowCachedContentSize(), arrowRules);

		const i32 arrowAreaOffset = Math::Max(0, (i32)fullContentArea.Width - (i32)(output.Width + arrowRules.Padding.Right + arrowRules.BorderRight.GetVisibleWidth()));
		output.X += fullContentArea.X + arrowAreaOffset;
	}

	return output;
}

Size2UI GUIListBox::GetArrowCachedContentSize() const
{
	const Rect2I& cachedBounds = GetCachedBounds();

	Size2UI output(0, cachedBounds.Height);
	if(!IsUsingStyleSheets() || mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return output;
	
	const GUIStyleSheetRuleInformation& arrowRuleInformation = GetPseudoElementStyleSheetRuleInformation(mArrowPseudoElementIndex);
	if(arrowRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& arrowRules = arrowRuleInformation.CurrentStateRuleset->Rules;
		output.Width = arrowRules.Size.Width +
			arrowRules.Padding.Left + arrowRules.Padding.Right +
			arrowRules.BorderLeft.GetVisibleWidth() + arrowRules.BorderRight.GetVisibleWidth();
	}

	return output;
}


void GUIListBox::UpdateRenderElements()
{
	GUIButtonBase::UpdateRenderElements();

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return;

	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mArrowPseudoElementIndex);
	if(ruleInformation.CurrentStateRuleset == nullptr)
		return;

	const GUIStyleSheetRules& arrowStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	const Rect2I arrowBounds = GetArrowCachedContentBoundsInElementSpace();

	mArrowSpriteInformation.Width = arrowBounds.Width;
	mArrowSpriteInformation.Height = arrowBounds.Height;

	if(mArrowPathBuilder)
	{
		SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
		spriteVectorPathCreateInformation.Size = Size2UI(mArrowSpriteInformation.Width, mArrowSpriteInformation.Height);
		spriteVectorPathCreateInformation.VectorPath = mArrowPathBuilder->BuildPath(spriteVectorPathCreateInformation.Size, arrowStyleSheetRules);

		mArrowSpriteInformation.Image = SpriteVectorPath::Create(spriteVectorPathCreateInformation);
	}
	else
		mArrowSpriteInformation.Image = nullptr;

	mArrowSpriteInformation.Color = GetTint();
	mArrowSpriteInformation.Color.A *= arrowStyleSheetRules.Opacity;

	mArrowSprite->Update(mArrowSpriteInformation, (u64)GetParentWidget());

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;

		T::Append({ T::SpriteInfo(mArrowSprite, 0, (Rect2)arrowBounds) }, mRenderElements);
	}
}

bool GUIListBox::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	bool processed = GUIButtonBase::DoOnMouseEvent(ev);

	if(ev.GetType() == GUIMouseEventType::MouseDown)
	{
		if(!IsDisabled())
		{
			if(mDropDownBox == nullptr)
				OpenListBox();
			else
				CloseListBox();
		}

		processed = true;
	}

	return processed;
}

bool GUIListBox::DoOnCommandEvent(const GUICommandEvent& ev)
{
	const bool processed = GUIButtonBase::DoOnCommandEvent(ev);

	if(ev.GetType() == GUICommandEventType::Confirm)
	{
		if(!IsDisabled())
		{
			if(mDropDownBox == nullptr)
				OpenListBox();
			else
				CloseListBox();
		}

		return true;
	}

	return processed;
}

void GUIListBox::ElementSelected(u32 idx)
{
	if(idx >= (u32)mElements.size())
		return;

	if(mIsMultiselect)
	{
		bool selected = mElementStates[idx];
		mElementStates[idx] = !selected;

		if(!OnSelectionToggled.Empty())
			OnSelectionToggled(idx, !selected);
	}
	else
	{
		for(u32 i = 0; i < (u32)mElementStates.size(); i++)
			mElementStates[i] = false;

		mElementStates[idx] = true;

		if(!OnSelectionToggled.Empty())
			OnSelectionToggled(idx, true);

		CloseListBox();
	}

	UpdateContents();
}

void GUIListBox::OpenListBox()
{
	CloseListBox();

	DROP_DOWN_BOX_DESC desc;

	u32 i = 0;
	for(auto& elem : mElements)
	{
		String identifier = ToString(i);
		desc.DropDownData.Entries.push_back(GUIDropDownDataEntry::Button(identifier, std::bind(&GUIListBox::ElementSelected, this, i)));
		desc.DropDownData.LocalizedNames[identifier] = elem;
		i++;
	}

	GUIWidget* widget = GetParentWidget();

	desc.Camera = widget->GetCamera();
	desc.Skin = widget->GetSkinResource();
	desc.Placement = DropDownAreaPlacement::AroundBoundsHorz(mClippedBounds);
	desc.DropDownData.States = mElementStates;

	GUIDropDownType type;
	if(mIsMultiselect)
		type = GUIDropDownType::MultiListBox;
	else
		type = GUIDropDownType::ListBox;

	mDropDownBox = GUIDropDownBoxManager::Instance().OpenDropDownBox(
		desc, type, std::bind(&GUIListBox::OnListBoxClosed, this));

	SetOnInternal(true);
}

void GUIListBox::CloseListBox()
{
	if(mDropDownBox != nullptr)
	{
		GUIDropDownBoxManager::Instance().CloseDropDownBox();

		SetOnInternal(false);
		mDropDownBox = nullptr;
	}
}

void GUIListBox::UpdateContents()
{
	u32 selectedIdx = 0;
	u32 numSelected = 0;
	for(u32 i = 0; i < (u32)mElementStates.size(); i++)
	{
		if(mElementStates[i])
		{
			selectedIdx = i;
			numSelected++;
		}
	}

	if(mIsMultiselect)
	{
		if(numSelected == 1)
			SetContent(GUIContent(mElements[selectedIdx]));
		else if(numSelected == 0)
			SetContent(GUIContent(HEString("None")));
		else
			SetContent(GUIContent(HEString("Multiple")));
	}
	else
	{
		if(!mElements.empty())
			SetContent(GUIContent(mElements[selectedIdx]));
		else
			SetContent(GUIContent(HEString("None")));
	}
}

void GUIListBox::OnListBoxClosed()
{
	SetOnInternal(false);
	mDropDownBox = nullptr;
}
