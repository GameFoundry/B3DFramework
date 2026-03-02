//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIDropDownContent.h"
#include "GUI/B3DGUITexture.h"
#include "GUI/B3DGUIButton.h"
#include "GUI/B3DGUILabel.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUIToggle.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUICommandEvent.h"
#include "B3DGUILayoutX.h"
#include "B3DGUIVectorPaths.h"
#include "StyleSheet/B3DGUIStyleSheet.h"
#include "Text/B3DStockIcons.h"
#include <climits>

using namespace b3d;

GUIDropDownContent::GUIDropDownContent(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const String& style, const GUISizeConstraints& dimensions)
	: GUIElementContainer(dimensions, style), mDropDownData(dropDownData), mStates(dropDownData.States), mSelectedIdx(UINT_MAX), mRangeStart(0), mRangeEnd(0), mParent(parent), mKeyboardFocus(true), mIsToggle(parent->GetType() == GUIDropDownType::MultiListBox)
{
}

GUIDropDownContent* GUIDropDownContent::Create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const String& style)
{
	return new(B3DAllocate<GUIDropDownContent>()) GUIDropDownContent(parent, dropDownData, GetStyleClass<GUIDropDownContent>(style), GUISizeConstraints::Create());
}

GUIDropDownContent* GUIDropDownContent::Create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const GUIOptions& options, const String& style)
{
	return new(B3DAllocate<GUIDropDownContent>()) GUIDropDownContent(parent, dropDownData, GetStyleClass<GUIDropDownContent>(style), GUISizeConstraints::Create(options));
}

void GUIDropDownContent::SetRange(u32 start, u32 end)
{
	std::function<void(u32, u32)> fnOnHover =
		[&](u32 idx, u32 visIdx)
	{
		SetSelected(visIdx);
		mParent->ElementSelected(idx);
	};

	std::function<void(u32, u32)> fnOnClick =
		[&](u32 idx, u32 visIdx)
	{
		SetSelected(visIdx);

		if(mIsToggle)
			mStates[idx] = !mStates[idx];

		mParent->ElementActivated(idx, mVisibleElements[visIdx].UnderlayButton->GetAbsoluteClippedArea());
	};

	// Remove all elements
	while(GetChildCount() > 0)
	{
		GUIElement* child = GetChild(GetChildCount() - 1);
		child->Destroy();
	}

	mRangeStart = start;
	mRangeEnd = end;

	u32 range = end - start;
	if(mSelectedIdx != UINT_MAX && mSelectedIdx >= range)
		mSelectedIdx = UINT_MAX;

	mVisibleElements.clear();
	u32 currentVisibleIndex = 0;
	for(u32 elementIndex = start; elementIndex < end; elementIndex++)
	{
		mVisibleElements.push_back(VisibleElement());
		VisibleElement& visibleElement = mVisibleElements.back();
		visibleElement.SequentialIndex = elementIndex;

		GUIDropDownDataEntry& element = mDropDownData.Entries[elementIndex];

		if(element.IsSeparator())
		{
			visibleElement.Separator = GUITexture::Create(GUITextureContents(nullptr, TextureScaleMode::StretchToFit), kSeparatorStyleClass);
			RegisterChildElement(visibleElement.Separator);
		}
		else
		{
			visibleElement.Layout = GUILayoutX::Create();
			RegisterChildElement(visibleElement.Layout);

			visibleElement.UnderlayButton = GUIButton::Create(GUIContent(), kUnderlayStyleClass);
			visibleElement.UnderlayButton->SetElementDepth(1);
			RegisterChildElement(visibleElement.UnderlayButton);

			if(element.IsSubMenu())
			{
				GUILabel* const label = GUILabel::Create(GUIContent(GetElementLocalizedName(elementIndex)));
				label->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				GUIButton* const arrow = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeRightLong, 12.0f)), kExpandArrowStyleClass);
				arrow->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				visibleElement.Layout->AddElement(label);
				visibleElement.Layout->AddElement(arrow);

				visibleElement.UnderlayButton->OnHover.Connect([elementIndex, currentVisibleIndex, fnOnClick] { fnOnClick(elementIndex, currentVisibleIndex); });
			}
			else if(mIsToggle)
			{
				GUILabel* const label = GUILabel::Create(GUIContent(GetElementLocalizedName(elementIndex)));
				label->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				GUIToggle* const toggle = GUIToggle::Create(GUIContent(), kToggleStyleClass);
				toggle->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				if(mStates[elementIndex])
					toggle->SetIsToggled(true);

				visibleElement.Button = toggle;

				visibleElement.UnderlayButton->OnHover.Connect([toggle]() {
					toggle->AddStateFlags(GUIElementStateFlag::Hover);
				});

				visibleElement.UnderlayButton->OnOut.Connect([toggle]() {
					toggle->RemoveStateFlags(GUIElementStateFlag::Hover);
				});

				visibleElement.UnderlayButton->OnClick.Connect([toggle]() {
					toggle->SetIsToggled(!toggle->IsToggled());
				});

				visibleElement.Layout->AddElement(visibleElement.Button);
				visibleElement.Layout->AddElement(label);
			}
			else
			{
				visibleElement.Button = GUIButton::Create(GetElementLocalizedName(elementIndex), kButtonStyleClass);
				visibleElement.Button->OnHover.Connect([elementIndex, currentVisibleIndex, fnOnHover] { fnOnHover(elementIndex, currentVisibleIndex); });
				visibleElement.Button->OnClick.Connect([elementIndex, currentVisibleIndex, fnOnClick] { fnOnClick(elementIndex, currentVisibleIndex); });
				visibleElement.Layout->AddElement(visibleElement.Button);
			}

			if(!element.IsSubMenu())
			{
				const String& shortcutTag = element.GetShortcutTag();
				if(!shortcutTag.empty())
				{
					GUILabel* const shortcutLabel = GUILabel::Create(HString(shortcutTag), "RightAlignedLabel");
					visibleElement.Layout->AddElement(shortcutLabel);
				}
			}
		}

		currentVisibleIndex++;
	}

	MarkLayoutAsDirty();
}

GUILogicalUnit GUIDropDownContent::GetElementHeight(u32 index) const
{
	static constexpr GUILogicalUnit kDefaultHeight = 16; // Height to use when no style available

	GUIWidget* const widget = GetParentWidget();
	if(widget == nullptr)
		return kDefaultHeight;

	const GUIStyleSheetCascade& styleSheetCascade = widget->GetStyleSheetCascade();

	GUIStyleSheetRules rules;
	if(mDropDownData.Entries[index].IsSeparator())
		rules = styleSheetCascade.BuildRules("texture", kSeparatorStyleClass);
	else
	{
		const char* elementClass = mIsToggle ? kToggleStyleClass : kButtonStyleClass;
		rules = styleSheetCascade.BuildRules("button", elementClass);
	}

	return GUILogicalUnit((i32)rules.Size.Height);
}

HString GUIDropDownContent::GetElementLocalizedName(u32 index) const
{
	const String& label = mDropDownData.Entries[index].GetLabel();

	auto found = mDropDownData.LocalizedNames.find(label);
	if(found != mDropDownData.LocalizedNames.end())
		return found->second;
	else
		return HString(label);
}

void GUIDropDownContent::SetKeyboardFocus(bool focus)
{
	mKeyboardFocus = focus;
	SetFocus(focus);
}

bool GUIDropDownContent::DoOnCommandEvent(const GUICommandEvent& ev)
{
	bool baseReturn = GUIElementContainer::DoOnCommandEvent(ev);

	if(!mKeyboardFocus)
		return baseReturn;

	switch(ev.GetType())
	{
	case GUICommandEventType::MoveDown:
		if(mSelectedIdx == UINT_MAX)
			SelectNext(0);
		else
			SelectNext(mVisibleElements[mSelectedIdx].SequentialIndex + 1);
		return true;
	case GUICommandEventType::MoveUp:
		if(mSelectedIdx == UINT_MAX)
			SelectNext(0);
		else
			SelectPrevious(mVisibleElements[mSelectedIdx].SequentialIndex - 1);
		return true;
	case GUICommandEventType::Escape:
	case GUICommandEventType::MoveLeft:
		mParent->Close();
		return true;
	case GUICommandEventType::MoveRight:
		{
			if(mSelectedIdx == UINT_MAX)
				SelectNext(0);
			else
			{
				GUIDropDownDataEntry& entry = mDropDownData.Entries[mVisibleElements[mSelectedIdx].SequentialIndex];
				if(entry.IsSubMenu())
					mParent->ElementActivated(mVisibleElements[mSelectedIdx].SequentialIndex, mVisibleElements[mSelectedIdx].UnderlayButton->GetAbsoluteBounds());
			}
		}
		return true;
	case GUICommandEventType::Confirm:
		if(mSelectedIdx == UINT_MAX)
			SelectNext(0);
		else
		{
			if(mIsToggle)
				mVisibleElements[mSelectedIdx].Button->SetOnInternal(!mVisibleElements[mSelectedIdx].Button->IsOnInternal());

			mParent->ElementActivated(mVisibleElements[mSelectedIdx].SequentialIndex, mVisibleElements[mSelectedIdx].UnderlayButton->GetAbsoluteBounds());
		}
		return true;
	default:
		break;
	}

	return baseReturn;
}

bool GUIDropDownContent::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseWheelScroll)
	{
		if(ev.GetWheelScrollAmount() < 0)
			mParent->ScrollDown();
		else
			mParent->ScrollUp();

		return true;
	}

	return false;
}

void GUIDropDownContent::SetSelected(u32 index)
{
	if(mSelectedIdx != UINT_MAX)
	{
		VisibleElement& previouslySelectedElement = mVisibleElements[mSelectedIdx];

		previouslySelectedElement.UnderlayButton->RemoveStateFlags(GUIElementStateFlag::Hover);
		if(previouslySelectedElement.Button != nullptr)
			previouslySelectedElement.Button->RemoveStateFlags(GUIElementStateFlag::Hover);
	}

	mSelectedIdx = index;

	VisibleElement& newlySelectedElement = mVisibleElements[mSelectedIdx];

	newlySelectedElement.UnderlayButton->AddStateFlags(GUIElementStateFlag::Hover);
	if(newlySelectedElement.Button != nullptr)
		newlySelectedElement.Button->AddStateFlags(GUIElementStateFlag::Hover);

	mParent->ElementSelected(mVisibleElements[mSelectedIdx].SequentialIndex);
}

void GUIDropDownContent::SelectNext(u32 startIndex)
{
	u32 elementCount = (u32)mDropDownData.Entries.size();

	bool gotNextIndex = false;
	u32 nextIndex = startIndex;
	for(u32 elementIndex = 0; elementIndex < elementCount; elementIndex++)
	{
		if(nextIndex >= elementCount)
			nextIndex = 0; // Wrap around

		GUIDropDownDataEntry& entry = mDropDownData.Entries[nextIndex];
		if(!entry.IsSeparator())
		{
			gotNextIndex = true;
			break;
		}

		nextIndex++;
	}

	if(gotNextIndex)
	{
		while(nextIndex < mRangeStart || nextIndex >= mRangeEnd)
			mParent->ScrollDown();

		u32 visibleIndex = 0;
		for(auto& visibleElement : mVisibleElements)
		{
			if(visibleElement.SequentialIndex == nextIndex)
			{
				SetSelected(visibleIndex);
				break;
			}

			visibleIndex++;
		}
	}
}

void GUIDropDownContent::SelectPrevious(u32 startIndex)
{
	u32 elementCount = (u32)mDropDownData.Entries.size();

	bool gotNextIndex = false;
	i32 previousIndex = (i32)startIndex;

	for(u32 elementIndex = 0; elementIndex < elementCount; elementIndex++)
	{
		if(previousIndex < 0)
			previousIndex = elementCount - 1; // Wrap around

		GUIDropDownDataEntry& entry = mDropDownData.Entries[previousIndex];
		if(!entry.IsSeparator())
		{
			gotNextIndex = true;
			break;
		}

		previousIndex--;
	}

	if(gotNextIndex)
	{
		while(previousIndex < (i32)mRangeStart || previousIndex >= (i32)mRangeEnd)
			mParent->ScrollUp();

		u32 visibleIndex = 0;
		for(auto& visibleElement : mVisibleElements)
		{
			if(visibleElement.SequentialIndex == (u32)previousIndex)
			{
				SetSelected(visibleIndex);
				break;
			}

			visibleIndex++;
		}
	}
}

GUILogicalSize GUIDropDownContent::CalculateUnconstrainedOptimalSize() const
{
	GUILogicalSize optimalSize(kZeroTag);
	for(auto& visibleElement : mVisibleElements)
	{
		const GUIDropDownDataEntry& element = mDropDownData.Entries[visibleElement.SequentialIndex];

		optimalSize.Height += GetElementHeight(visibleElement.SequentialIndex);

		if(element.IsSeparator())
			optimalSize.Width = Math::Max(optimalSize.Width, visibleElement.Separator->CalculateUnconstrainedOptimalSize().Width);
		else
			optimalSize.Width = Math::Max(optimalSize.Width, visibleElement.Layout->CalculateUnconstrainedOptimalSize().Width);
	}

	return optimalSize;
}

void GUIDropDownContent::UpdateLayoutForChildren()
{
	GUILayoutData childData = mLayoutData;
	GUILogicalUnit yOffset = 0;

	for(auto& visibleElement : mVisibleElements)
	{
		const GUIDropDownDataEntry& element = mDropDownData.Entries[visibleElement.SequentialIndex];

		childData.RelativePosition = GUILogicalPoint(0, yOffset);
		childData.Size.Height = GetElementHeight(visibleElement.SequentialIndex);

		yOffset += childData.Size.Height;

		if(element.IsSeparator())
		{
			visibleElement.Separator->SetLayoutData(childData);
			visibleElement.Separator->UpdateLayoutForChildren();
		}
		else
		{
			visibleElement.Layout->SetLayoutData(childData);
			visibleElement.Layout->UpdateLayoutForChildren();
		}

		if(visibleElement.UnderlayButton)
		{
			visibleElement.UnderlayButton->SetLayoutData(childData);
			visibleElement.UnderlayButton->UpdateLayoutForChildren();
		}
	}
}

const String& GUIDropDownContent::GetGuiTypeName()
{
	static String typeName = "GUIDropDownContent";
	return typeName;
}
