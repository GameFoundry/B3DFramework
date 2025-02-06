//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIDropDownContent.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"
#include "BsGUILayoutX.h"
#include "BsGUIVectorPaths.h"
#include "StyleSheet/BsGUIStyleSheet.h"
#include "Text/BsStockIcons.h"
#include <climits>

using namespace std::placeholders;
using namespace bs;

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
	std::function<void(u32, u32)> onHover =
		[&](u32 idx, u32 visIdx)
	{
		SetSelected(visIdx);
		mParent->ElementSelected(idx);
	};

	std::function<void(u32, u32)> onClick =
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
	u32 curVisIdx = 0;
	for(u32 i = start; i < end; i++)
	{
		mVisibleElements.push_back(VisibleElement());
		VisibleElement& visibleElement = mVisibleElements.back();
		visibleElement.SequentialIndex = i;

		GUIDropDownDataEntry& element = mDropDownData.Entries[i];

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
				GUILabel* const label = GUILabel::Create(GUIContent(GetElementLocalizedName(i)));
				label->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				GUIButton* const arrow = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeRightLong, 12.0f)), kExpandArrowStyleClass);
				arrow->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				visibleElement.Layout->AddElement(label);
				visibleElement.Layout->AddElement(arrow);

				visibleElement.UnderlayButton->OnHover.Connect(std::bind(onClick, i, curVisIdx));
			}
			else if(mIsToggle)
			{
				GUILabel* const label = GUILabel::Create(GUIContent(GetElementLocalizedName(i)));
				label->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				GUIToggle* const toggle = GUIToggle::Create(GUIContent(), kToggleStyleClass);
				toggle->SetOptionFlags(GUIElementOption::IgnorePointerEvents);

				if(mStates[i])
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
				visibleElement.Button = GUIButton::Create(GetElementLocalizedName(i), kButtonStyleClass);
				visibleElement.Button->OnHover.Connect(std::bind(onHover, i, curVisIdx));
				visibleElement.Button->OnClick.Connect(std::bind(onClick, i, curVisIdx));
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

		curVisIdx++;
	}

	MarkLayoutAsDirty();
}

u32 GUIDropDownContent::GetElementHeight(u32 idx) const
{
	static constexpr u32 kDefaultHeight = 16; // Height to use when no style available

	GUIWidget* const widget = GetParentWidget();
	if(widget == nullptr)
		return kDefaultHeight;

	const GUIStyleSheetCascade& styleSheetCascade = widget->GetStyleSheetCascade();

	GUIStyleSheetRules rules;
	if(mDropDownData.Entries[idx].IsSeparator())
		rules = styleSheetCascade.BuildRules("texture", kSeparatorStyleClass);
	else
	{
		const char* elementClass = mIsToggle ? kToggleStyleClass : kButtonStyleClass;
		rules = styleSheetCascade.BuildRules("button", elementClass);
	}

	return rules.Size.Height;
}

HString GUIDropDownContent::GetElementLocalizedName(u32 idx) const
{
	const String& label = mDropDownData.Entries[idx].GetLabel();

	auto findLocalizedName = mDropDownData.LocalizedNames.find(label);
	if(findLocalizedName != mDropDownData.LocalizedNames.end())
		return findLocalizedName->second;
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

void GUIDropDownContent::SetSelected(u32 idx)
{
	if(mSelectedIdx != UINT_MAX)
	{
		VisibleElement& previouslySelectedElement = mVisibleElements[mSelectedIdx];

		previouslySelectedElement.UnderlayButton->RemoveStateFlags(GUIElementStateFlag::Hover);
		if(previouslySelectedElement.Button != nullptr)
			previouslySelectedElement.Button->RemoveStateFlags(GUIElementStateFlag::Hover);
	}

	mSelectedIdx = idx;

	VisibleElement& newlySelectedElement = mVisibleElements[mSelectedIdx];

	newlySelectedElement.UnderlayButton->AddStateFlags(GUIElementStateFlag::Hover);
	if(newlySelectedElement.Button != nullptr)
		newlySelectedElement.Button->AddStateFlags(GUIElementStateFlag::Hover);

	mParent->ElementSelected(mVisibleElements[mSelectedIdx].SequentialIndex);
}

void GUIDropDownContent::SelectNext(u32 startIdx)
{
	u32 numElements = (u32)mDropDownData.Entries.size();

	bool gotNextIndex = false;
	u32 nextIdx = startIdx;
	for(u32 i = 0; i < numElements; i++)
	{
		if(nextIdx >= numElements)
			nextIdx = 0; // Wrap around

		GUIDropDownDataEntry& entry = mDropDownData.Entries[nextIdx];
		if(!entry.IsSeparator())
		{
			gotNextIndex = true;
			break;
		}

		nextIdx++;
	}

	if(gotNextIndex)
	{
		while(nextIdx < mRangeStart || nextIdx >= mRangeEnd)
			mParent->ScrollDown();

		u32 visIdx = 0;
		for(auto& visElem : mVisibleElements)
		{
			if(visElem.SequentialIndex == nextIdx)
			{
				SetSelected(visIdx);
				break;
			}

			visIdx++;
		}
	}
}

void GUIDropDownContent::SelectPrevious(u32 startIdx)
{
	u32 numElements = (u32)mDropDownData.Entries.size();

	bool gotNextIndex = false;
	i32 prevIdx = (i32)startIdx;

	for(u32 i = 0; i < numElements; i++)
	{
		if(prevIdx < 0)
			prevIdx = numElements - 1; // Wrap around

		GUIDropDownDataEntry& entry = mDropDownData.Entries[prevIdx];
		if(!entry.IsSeparator())
		{
			gotNextIndex = true;
			break;
		}

		prevIdx--;
	}

	if(gotNextIndex)
	{
		while(prevIdx < (i32)mRangeStart || prevIdx >= (i32)mRangeEnd)
			mParent->ScrollUp();

		u32 visIdx = 0;
		for(auto& visElem : mVisibleElements)
		{
			if(visElem.SequentialIndex == (u32)prevIdx)
			{
				SetSelected(visIdx);
				break;
			}

			visIdx++;
		}
	}
}

Vector2I GUIDropDownContent::CalculateUnconstrainedOptimalSize() const
{
	Vector2I optimalSize(BsZero);
	for(auto& visibleElement : mVisibleElements)
	{
		const GUIDropDownDataEntry& element = mDropDownData.Entries[visibleElement.SequentialIndex];

		optimalSize.Y += (i32)GetElementHeight(visibleElement.SequentialIndex);

		if(element.IsSeparator())
			optimalSize.X = std::max(optimalSize.X, visibleElement.Separator->CalculateUnconstrainedOptimalSize().X);
		else
			optimalSize.X = std::max(optimalSize.X, visibleElement.Layout->CalculateUnconstrainedOptimalSize().X);
	}

	return optimalSize;
}

void GUIDropDownContent::UpdateLayoutForChildren()
{
	GUILayoutData childData = mLayoutData;
	i32 yOffset = 0;

	for(auto& visibleElement : mVisibleElements)
	{
		const GUIDropDownDataEntry& element = mDropDownData.Entries[visibleElement.SequentialIndex];

		childData.RelativePosition = Vector2I(0, yOffset);
		childData.Size.Height = GetElementHeight(visibleElement.SequentialIndex);

		yOffset += (i32)childData.Size.Height;

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
