//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIListBox.h"

#include "B3DGUIUtility.h"
#include "B3DGUIVectorPaths.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUIManager.h"
#include "GUI/B3DGUIDropDownBoxManager.h"
#include "Image/B3DSpriteVectorPath.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

const String& GUIListBox::GetGuiTypeName()
{
	static String name = "ListBox";
	return name;
}

GUIListBox::GUIListBox(PrivatelyConstruct, const GUIListBoxContent& content, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIClickable(styleName, GUIContent(HString("")), sizeConstraints), mElements(content.Elements), mIsMultiselect(content.AllowMultiselect)
{
	mElementStates.resize(mElements.size(), false);
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

void GUIListBox::SelectElement(u32 index)
{
	if(index >= (u32)mElements.size())
		return;

	if(mElementStates[index] != true)
		ElementSelected(index);
}

void GUIListBox::DeselectElement(u32 index)
{
	if(!mIsMultiselect || index >= (u32)mElements.size())
		return;

	if(mElementStates[index] != false)
		ElementSelected(index);
}

u32 GUIListBox::GetSelectedElementIndex() const
{
	for(u32 elementIndex = 0; elementIndex < mElementStates.size(); ++elementIndex)
	{
		if(mElementStates[elementIndex])
			return elementIndex;
	}

	return ~0u;
}

void GUIListBox::SetElementStates(const Vector<bool>& states)
{
	u32 elementCount = (u32)mElementStates.size();
	u32 min = std::min(elementCount, (u32)states.size());

	bool anythingModified = min != elementCount;
	if(!anythingModified)
	{
		for(u32 elementIndex = 0; elementIndex < elementCount; elementIndex++)
		{
			if(mElementStates[elementIndex] != states[elementIndex])
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

	for(u32 elementIndex = 0; elementIndex < min; elementIndex++)
	{
		mElementStates[elementIndex] = states[elementIndex];

		if(mElementStates[elementIndex] && !mIsMultiselect)
		{
			for(u32 remainingIndex = elementIndex + 1; remainingIndex < elementCount; remainingIndex++)
				mElementStates[remainingIndex] = false;

			break;
		}
	}

	UpdateContents();

	if(wasOpen)
		OpenListBox();
}

GUILogicalArea GUIListBox::GetContentBounds() const
{
	GUILogicalSize layoutSize = mLayoutData.Size;

	const GUILogicalUnit arrowAreaWidth = GetArrowCachedContentSize().Width;
	layoutSize.Width = Math::Max(layoutSize.Width - arrowAreaWidth, 0);

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		return GUIUtility::RemovePaddingAndBorder(layoutSize, styleSheetRules);
	}

	return GUILogicalArea(GUILogicalPoint(0, 0), mLayoutData.Size);
}

GUIPhysicalArea GUIListBox::GetArrowCachedContentBoundsInElementSpace() const
{
	if(!IsUsingStyleSheets() || mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return GUIPhysicalArea::kEmpty;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	const GUILogicalArea& fullContentArea = GUIUtility::RemovePaddingAndBorder(mLayoutData.Size, styleSheetRules);

	const GUIStyleSheetRuleInformation& arrowRuleInformation = GetPseudoElementStyleSheetRuleInformation(mArrowPseudoElementIndex);
	GUILogicalArea logicalArea = GUILogicalArea::kEmpty;
	if(arrowRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& arrowRules = arrowRuleInformation.CurrentStateRuleset->Rules;
		logicalArea = GUIUtility::RemovePaddingAndBorder(GetArrowCachedContentSize(), arrowRules);

		const GUILogicalUnit arrowAreaOffset = Math::Max(fullContentArea.Width - (logicalArea.Width + arrowRules.Padding.Right + (i32)arrowRules.BorderRight.GetVisibleWidth()), 0);
		logicalArea.X += fullContentArea.X + arrowAreaOffset;
	}

	return GUIUtility::LogicalToPhysical(logicalArea, GetAbsoluteScale());
}

GUILogicalSize GUIListBox::GetArrowCachedContentSize() const
{
	GUILogicalSize output(0, mLayoutData.Size.Height);
	if(!IsUsingStyleSheets() || mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return output;
	
	const GUIStyleSheetRuleInformation& arrowRuleInformation = GetPseudoElementStyleSheetRuleInformation(mArrowPseudoElementIndex);
	if(arrowRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& arrowRules = arrowRuleInformation.CurrentStateRuleset->Rules;
		output.Width = (i32)arrowRules.Size.Width +
			arrowRules.Padding.Left + arrowRules.Padding.Right +
			(i32)arrowRules.BorderLeft.GetVisibleWidth() + (i32)arrowRules.BorderRight.GetVisibleWidth();
	}

	return output;
}


void GUIListBox::UpdateRenderElements()
{
	GUIClickable::UpdateRenderElements();

	const bool isUsingStyleSheets = IsUsingStyleSheets();
	if(!isUsingStyleSheets)
		return;

	const GUIStyleSheetRuleInformation& ruleInformation = GetPseudoElementStyleSheetRuleInformation(mArrowPseudoElementIndex);
	if(ruleInformation.CurrentStateRuleset == nullptr)
		return;

	const GUIStyleSheetRules& arrowStyleSheetRules = ruleInformation.CurrentStateRuleset->Rules;
	const GUIPhysicalArea arrowBounds = GetArrowCachedContentBoundsInElementSpace();

	mArrowSpriteInformation.Size = arrowBounds.GetSize().To<i32>();

	if(mArrowPathBuilder)
	{
		SpriteVectorPathCreateInformation spriteVectorPathCreateInformation;
		spriteVectorPathCreateInformation.DefaultSize = mArrowSpriteInformation.Size;
		spriteVectorPathCreateInformation.VectorPath = mArrowPathBuilder->BuildPath(spriteVectorPathCreateInformation.DefaultSize, arrowStyleSheetRules);

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

		T::Append({ T::SpriteInfo(mArrowSprite, 0, arrowBounds.To<float>()) }, mRenderElements);
	}
}

bool GUIListBox::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	bool processed = GUIClickable::DoOnMouseEvent(ev);

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
	const bool processed = GUIClickable::DoOnCommandEvent(ev);

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

void GUIListBox::ElementSelected(u32 index)
{
	if(index >= (u32)mElements.size())
		return;

	if(mIsMultiselect)
	{
		bool selected = mElementStates[index];
		mElementStates[index] = !selected;

		if(!OnSelectionToggled.Empty())
			OnSelectionToggled(index, !selected);
	}
	else
	{
		for(u32 elementIndex = 0; elementIndex < (u32)mElementStates.size(); elementIndex++)
			mElementStates[elementIndex] = false;

		mElementStates[index] = true;

		if(!OnSelectionToggled.Empty())
			OnSelectionToggled(index, true);

		CloseListBox();
	}

	UpdateContents();
}

void GUIListBox::OpenListBox()
{
	CloseListBox();

	DropDownBoxCreateInformation createInformation;

	u32 elementIndex = 0;
	for(auto& element : mElements)
	{
		String identifier = ToString(elementIndex);
		createInformation.DropDownData.Entries.push_back(GUIDropDownDataEntry::Button(identifier, [this, elementIndex]() { ElementSelected(elementIndex); }));
		createInformation.DropDownData.LocalizedNames[identifier] = element;
		elementIndex++;
	}

	GUIWidget* widget = GetParentWidget();

	createInformation.Camera = widget->GetCamera();
	createInformation.StyleSheetCascade = widget->GetStyleSheetCascadeAsShared();
	createInformation.Placement = TDropDownAreaPlacement<GUIPhysicalUnit>::AroundBoundsHorizontal(GetAbsoluteBounds());
	createInformation.DropDownData.States = mElementStates;

	GUIDropDownType type;
	if(mIsMultiselect)
		type = GUIDropDownType::MultiListBox;
	else
		type = GUIDropDownType::ListBox;

	mDropDownBox = GUIDropDownBoxManager::Instance().OpenDropDownBox(
		createInformation, type, [this]() { OnListBoxClosed(); });

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
	u32 selectedIndex = 0;
	u32 selectedCount = 0;
	for(u32 elementIndex = 0; elementIndex < (u32)mElementStates.size(); elementIndex++)
	{
		if(mElementStates[elementIndex])
		{
			selectedIndex = elementIndex;
			selectedCount++;
		}
	}

	if(mIsMultiselect)
	{
		if(selectedCount == 1)
			SetContent(GUIContent(mElements[selectedIndex]));
		else if(selectedCount == 0)
			SetContent(GUIContent(HEString("None")));
		else
			SetContent(GUIContent(HEString("Multiple")));
	}
	else
	{
		if(!mElements.empty())
			SetContent(GUIContent(mElements[selectedIndex]));
		else
			SetContent(GUIContent(HEString("None")));
	}
}

void GUIListBox::OnListBoxClosed()
{
	SetOnInternal(false);
	mDropDownBox = nullptr;
}
