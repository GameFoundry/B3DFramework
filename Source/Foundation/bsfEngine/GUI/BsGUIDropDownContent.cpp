//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIDropDownContent.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"

#include <climits>

using namespace std::placeholders;

using namespace bs;

constexpr const char* GUIDropDownContent::kEntryToggleStyleType;
constexpr const char* GUIDropDownContent::kEntryStyleType;
constexpr const char* GUIDropDownContent::kEntryExpStyleType;
constexpr const char* GUIDropDownContent::kSeparatorStyleType;

GUIDropDownContent::GUIDropDownContent(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const String& style, const GUIDimensions& dimensions)
	: GUIElementContainer(dimensions, style), mDropDownData(dropDownData), mStates(dropDownData.States), mSelectedIdx(UINT_MAX), mRangeStart(0), mRangeEnd(0), mParent(parent), mKeyboardFocus(true), mIsToggle(parent->GetType() == GUIDropDownType::MultiListBox)
{
}

GUIDropDownContent* GUIDropDownContent::Create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const String& style)
{
	const String* curStyle = &style;
	if(*curStyle == StringUtil::kBlank)
		curStyle = &GUIDropDownContent::GetGuiTypeName();

	return new(B3DAllocate<GUIDropDownContent>()) GUIDropDownContent(parent, dropDownData, *curStyle, GUIDimensions::Create());
}

GUIDropDownContent* GUIDropDownContent::Create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const GUIOptions& options, const String& style)
{
	const String* curStyle = &style;
	if(*curStyle == StringUtil::kBlank)
		curStyle = &GUIDropDownContent::GetGuiTypeName();

	return new(B3DAllocate<GUIDropDownContent>()) GUIDropDownContent(parent, dropDownData, *curStyle, GUIDimensions::Create(options));
}

void GUIDropDownContent::NotifyStyleChanged()
{
	for(auto& visElem : mVisibleElements)
	{
		GUIDropDownDataEntry& element = mDropDownData.Entries[visElem.Idx];

		if(element.IsSeparator())
			visElem.Separator->SetStyle(GetSubStyleName(kSeparatorStyleType));
		else if(element.IsSubMenu())
			visElem.Button->SetStyle(GetSubStyleName(kEntryExpStyleType));
		else
		{
			if(mIsToggle)
				visElem.Button->SetStyle(GetSubStyleName(kEntryToggleStyleType));
			else
				visElem.Button->SetStyle(GetSubStyleName(kEntryStyleType));
		}
	}
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

		mParent->ElementActivated(idx, mVisibleElements[visIdx].Button->GetLayoutData().Area);
	};

	// Remove all elements
	while(GetChildCount() > 0)
	{
		GUIElementBase* child = GetChild(GetChildCount() - 1);
		B3D_ASSERT(child->GetType() == GUIElementBase::Type::Element);

		GUIElement::Destroy(static_cast<GUIElement*>(child));
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
		VisibleElement& visElem = mVisibleElements.back();
		visElem.Idx = i;
		GUIDropDownDataEntry& element = mDropDownData.Entries[i];

		if(element.IsSeparator())
		{
			visElem.Separator = GUITexture::Create(TextureScaleMode::StretchToFit, GetSubStyleName(kSeparatorStyleType));
			RegisterChildElement(visElem.Separator);
		}
		else if(element.IsSubMenu())
		{
			visElem.Button = GUIButton::Create(GetElementLocalizedName(i), GetSubStyleName(kEntryExpStyleType));
			visElem.Button->OnHover.Connect(std::bind(onClick, i, curVisIdx));
			RegisterChildElement(visElem.Button);
		}
		else
		{
			if(mIsToggle)
			{
				GUIToggle* toggle = GUIToggle::Create(GetElementLocalizedName(i), GetSubStyleName(kEntryToggleStyleType));
				if(mStates[i])
					toggle->ToggleOn();

				visElem.Button = toggle;
			}
			else
				visElem.Button = GUIButton::Create(GetElementLocalizedName(i), GetSubStyleName(kEntryStyleType));

			visElem.Button->OnHover.Connect(std::bind(onHover, i, curVisIdx));
			visElem.Button->OnClick.Connect(std::bind(onClick, i, curVisIdx));
			RegisterChildElement(visElem.Button);

			const String& shortcutTag = element.GetShortcutTag();
			if(!shortcutTag.empty())
			{
				visElem.ShortcutLabel = GUILabel::Create(HString(shortcutTag), "RightAlignedLabel");
				RegisterChildElement(visElem.ShortcutLabel);
			}
		}

		curVisIdx++;
	}

	MarkLayoutAsDirty();
}

u32 GUIDropDownContent::GetElementHeight(u32 idx) const
{
	if(GetParentWidget() == nullptr)
		return 14; // Arbitrary

	if(mDropDownData.Entries[idx].IsSeparator())
		return GetParentWidget()->GetSkin().GetStyle(GetSubStyleName(kSeparatorStyleType))->Height;
	else if(mDropDownData.Entries[idx].IsSubMenu())
		return GetParentWidget()->GetSkin().GetStyle(GetSubStyleName(kEntryExpStyleType))->Height;
	else
	{
		if(mIsToggle)
			return GetParentWidget()->GetSkin().GetStyle(GetSubStyleName(kEntryToggleStyleType))->Height;
		else
			return GetParentWidget()->GetSkin().GetStyle(GetSubStyleName(kEntryStyleType))->Height;
	}
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
			SelectNext(mVisibleElements[mSelectedIdx].Idx + 1);
		return true;
	case GUICommandEventType::MoveUp:
		if(mSelectedIdx == UINT_MAX)
			SelectNext(0);
		else
			SelectPrevious(mVisibleElements[mSelectedIdx].Idx - 1);
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
				GUIDropDownDataEntry& entry = mDropDownData.Entries[mVisibleElements[mSelectedIdx].Idx];
				if(entry.IsSubMenu())
					mParent->ElementActivated(mVisibleElements[mSelectedIdx].Idx, mVisibleElements[mSelectedIdx].Button->GetLayoutData().Area);
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

			mParent->ElementActivated(mVisibleElements[mSelectedIdx].Idx, mVisibleElements[mSelectedIdx].Button->GetLayoutData().Area);
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
		if(mVisibleElements[mSelectedIdx].Button->IsOnInternal())
			mVisibleElements[mSelectedIdx].Button->SetStateInternal(GUIElementState::NormalOn);
		else
			mVisibleElements[mSelectedIdx].Button->SetStateInternal(GUIElementState::Normal);
	}

	mSelectedIdx = idx;
	if(mVisibleElements[mSelectedIdx].Button->IsOnInternal())
		mVisibleElements[mSelectedIdx].Button->SetStateInternal(GUIElementState::HoverOn);
	else
		mVisibleElements[mSelectedIdx].Button->SetStateInternal(GUIElementState::Hover);

	mParent->ElementSelected(mVisibleElements[mSelectedIdx].Idx);
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
			if(visElem.Idx == nextIdx)
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
			if(visElem.Idx == (u32)prevIdx)
			{
				SetSelected(visIdx);
				break;
			}

			visIdx++;
		}
	}
}

Vector2I GUIDropDownContent::GetOptimalSize() const
{
	Vector2I optimalSize;
	for(auto& visElem : mVisibleElements)
	{
		const GUIDropDownDataEntry& element = mDropDownData.Entries[visElem.Idx];

		optimalSize.Y += (i32)GetElementHeight(visElem.Idx);

		if(element.IsSeparator())
			optimalSize.X = std::max(optimalSize.X, visElem.Separator->GetOptimalSize().X);
		else
			optimalSize.X = std::max(optimalSize.X, visElem.Button->GetOptimalSize().X);
	}

	return optimalSize;
}

void GUIDropDownContent::UpdateLayoutRecursive(const GUILayoutData& data)
{
	GUILayoutData childData = data;
	i32 yOffset = data.Area.Y;

	for(auto& visElem : mVisibleElements)
	{
		const GUIDropDownDataEntry& element = mDropDownData.Entries[visElem.Idx];

		GUIElement* guiMainElement = nullptr;
		if(element.IsSeparator())
			guiMainElement = visElem.Separator;
		else
			guiMainElement = visElem.Button;

		childData.Area.Y = yOffset;
		childData.Area.Height = GetElementHeight(visElem.Idx);

		yOffset += childData.Area.Height;

		guiMainElement->SetLayoutData(childData);

		// Shortcut label
		GUILabel* shortcutLabel = visElem.ShortcutLabel;
		if(shortcutLabel != nullptr)
			shortcutLabel->SetLayoutData(childData);
	}
}

const String& GUIDropDownContent::GetGuiTypeName()
{
	static String typeName = "GUIDropDownContent";
	return typeName;
}
