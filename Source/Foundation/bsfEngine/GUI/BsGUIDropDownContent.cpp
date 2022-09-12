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

namespace bs
{
	constexpr const char* GUIDropDownContent::ENTRY_TOGGLE_STYLE_TYPE;
	constexpr const char* GUIDropDownContent::ENTRY_STYLE_TYPE;
	constexpr const char* GUIDropDownContent::ENTRY_EXP_STYLE_TYPE;
	constexpr const char* GUIDropDownContent::SEPARATOR_STYLE_TYPE;

	GUIDropDownContent::GUIDropDownContent(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData,
		const String& style, const GUIDimensions& dimensions)
		: GUIElementContainer(dimensions, style), mDropDownData(dropDownData), mStates(dropDownData.states)
		, MSelectedIdx(UINT_MAX), mRangeStart(0), mRangeEnd(0), mParent(parent), mKeyboardFocus(true)
		, MIsToggle(parent->GetType() == GUIDropDownType::MultiListBox)
	{

	}

	GUIDropDownContent* GUIDropDownContent::create(GUIDropDownMenu::DropDownSubMenu* parent,
		const GUIDropDownData& dropDownData, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIDropDownContent::getGUITypeName();

		return new (bs_alloc<GUIDropDownContent>()) GUIDropDownContent(parent, dropDownData, *curStyle, GUIDimensions::create());
	}

	GUIDropDownContent* GUIDropDownContent::create(GUIDropDownMenu::DropDownSubMenu* parent,
		const GUIDropDownData& dropDownData, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIDropDownContent::getGUITypeName();

		return new (bs_alloc<GUIDropDownContent>()) GUIDropDownContent(parent, dropDownData, *curStyle, GUIDimensions::create(options));
	}

	void GUIDropDownContent::StyleUpdated()
	{
		for (auto& visElem : mVisibleElements)
		{
			GUIDropDownDataEntry& element = mDropDownData.entries[visElem.idx];

			if (element.IsSeparator())
				visElem.separator->SetStyle(getSubStyleName(SEPARATOR_STYLE_TYPE));
			else if (element.IsSubMenu())
				visElem.button->SetStyle(getSubStyleName(ENTRY_EXP_STYLE_TYPE));
			else
			{
				if (mIsToggle)
					visElem.button->SetStyle(getSubStyleName(ENTRY_TOGGLE_STYLE_TYPE));
				else
					visElem.button->SetStyle(getSubStyleName(ENTRY_STYLE_TYPE));
			}
		}
	}

	void GUIDropDownContent::SetRange(UINT32 start, UINT32 end)
	{
		std::function<void(UINT32, UINT32)> onHover =
			[&](UINT32 idx, UINT32 visIdx)
		{
			setSelected(visIdx);
			mParent->ElementSelected(idx);
		};

		std::function<void(UINT32, UINT32)> onClick =
			[&](UINT32 idx, UINT32 visIdx)
		{
			setSelected(visIdx);

			if (mIsToggle)
				mStates[idx] = !mStates[idx];

			mParent->ElementActivated(idx, mVisibleElements[visIdx].button->_getLayoutData().area);
		};

		// Remove all elements
		while (_getNumChildren() > 0)
		{
			GUIElementBase* child = _getChild(_getNumChildren() - 1);
			assert(child->_getType() == GUIElementBase::Type::Element);

			GUIElement::destroy(static_cast<GUIElement*>(child));
		}

		mRangeStart = start;
		mRangeEnd = end;
		
		UINT32 range = end - start;
		if (mSelectedIdx != UINT_MAX && mSelectedIdx >= range)
			mSelectedIdx = UINT_MAX;

		mVisibleElements.Clear();
		UINT32 curVisIdx = 0;
		for (UINT32 i = start; i < end; i++)
		{
			mVisibleElements.push_back(VisibleElement());
			VisibleElement& visElem = mVisibleElements.Back();
			visElem.idx = i;
			GUIDropDownDataEntry& element = mDropDownData.entries[i];

			if (element.IsSeparator())
			{
				visElem.separator = GUITexture::create(TextureScaleMode::StretchToFit, getSubStyleName(SEPARATOR_STYLE_TYPE));
				_registerChildElement(visElem.separator);
			}
			else if (element.IsSubMenu())
			{
				visElem.button = GUIButton::create(getElementLocalizedName(i), getSubStyleName(ENTRY_EXP_STYLE_TYPE));
				visElem.button->onHover.Connect(std::bind(onClick, i, curVisIdx));
				_registerChildElement(visElem.button);
			}
			else
			{
				if (mIsToggle)
				{
					GUIToggle* toggle = GUIToggle::create(getElementLocalizedName(i), getSubStyleName(ENTRY_TOGGLE_STYLE_TYPE));
					if (mStates[i])
						toggle->ToggleOn();

					visElem.button = toggle;					
				}
				else
					visElem.button = GUIButton::create(getElementLocalizedName(i), getSubStyleName(ENTRY_STYLE_TYPE));

				visElem.button->onHover.Connect(std::bind(onHover, i, curVisIdx));
				visElem.button->onClick.Connect(std::bind(onClick, i, curVisIdx));
				_registerChildElement(visElem.button);

				const String& shortcutTag = element.GetShortcutTag();
				if (!shortcutTag.Empty())
				{
					visElem.shortcutLabel = GUILabel::create(HString(shortcutTag), "RightAlignedLabel");
					_registerChildElement(visElem.shortcutLabel);
				}
			}

			curVisIdx++;
		}

		_markLayoutAsDirty();
	}

	UINT32 GUIDropDownContent::GetElementHeight(UINT32 idx) const
	{
		if (_getParentWidget() == nullptr)
			return 14; // Arbitrary

		if (mDropDownData.entries[idx].IsSeparator())
			return _getParentWidget()->GetSkin().GetStyle(getSubStyleName(SEPARATOR_STYLE_TYPE))->height;
		else if (mDropDownData.entries[idx].IsSubMenu())
			return _getParentWidget()->GetSkin().GetStyle(getSubStyleName(ENTRY_EXP_STYLE_TYPE))->height;
		else
		{
			if (mIsToggle)
				return _getParentWidget()->GetSkin().GetStyle(getSubStyleName(ENTRY_TOGGLE_STYLE_TYPE))->height;
			else
				return _getParentWidget()->GetSkin().GetStyle(getSubStyleName(ENTRY_STYLE_TYPE))->height;
		}
	}

	HString GUIDropDownContent::GetElementLocalizedName(UINT32 idx) const
	{
		const String& label = mDropDownData.entries[idx].GetLabel();

		auto findLocalizedName = mDropDownData.localizedNames.Find(label);
		if (findLocalizedName != mDropDownData.localizedNames.End())
			return findLocalizedName->second;
		else
			return HString(label);
	}

	void GUIDropDownContent::SetKeyboardFocus(bool focus)
	{
		mKeyboardFocus = focus;
		setFocus(focus);
	}

	bool GUIDropDownContent::_commandEvent(const GUICommandEvent& ev)
	{
		bool baseReturn = GUIElementContainer::_commandEvent(ev);

		if (!mKeyboardFocus)
			return baseReturn;

		switch (ev.GetType())
		{
		case GUICommandEventType::MoveDown:
			if (mSelectedIdx == UINT_MAX)
				selectNext(0);
			else
				selectNext(mVisibleElements[mSelectedIdx].idx + 1);
			return true;
		case GUICommandEventType::MoveUp:
			if (mSelectedIdx == UINT_MAX)
				selectNext(0);
			else
				selectPrevious(mVisibleElements[mSelectedIdx].idx - 1);
			return true;
		case GUICommandEventType::Escape:
		case GUICommandEventType::MoveLeft:
			mParent->Close();
			return true;
		case GUICommandEventType::MoveRight:
		{
			if (mSelectedIdx == UINT_MAX)
				selectNext(0);
			else
			{
				GUIDropDownDataEntry& entry = mDropDownData.entries[mVisibleElements[mSelectedIdx].idx];
				if (entry.IsSubMenu())
					mParent->ElementActivated(mVisibleElements[mSelectedIdx].idx, mVisibleElements[mSelectedIdx].button->_getLayoutData().area);
			}
		}
			return true;
		case GUICommandEventType::Confirm:
			if (mSelectedIdx == UINT_MAX)
				selectNext(0);
			else
			{
				if (mIsToggle)
					mVisibleElements[mSelectedIdx].button->_setOn(!mVisibleElements[mSelectedIdx].button->_isOn());

				mParent->ElementActivated(mVisibleElements[mSelectedIdx].idx, mVisibleElements[mSelectedIdx].button->_getLayoutData().area);
			}
			return true;
		default:
			break;
		}

		return baseReturn;
	}

	bool GUIDropDownContent::_mouseEvent(const GUIMouseEvent& ev)
	{
		if (ev.GetType() == GUIMouseEventType::MouseWheelScroll)
		{
			if (ev.GetWheelScrollAmount() < 0)
				mParent->ScrollDown();
			else
				mParent->ScrollUp();

			return true;
		}

		return false;
	}

	void GUIDropDownContent::SetSelected(UINT32 idx)
	{
		if (mSelectedIdx != UINT_MAX)
		{
			if (mVisibleElements[mSelectedIdx].button->_isOn())
				mVisibleElements[mSelectedIdx].button->_setState(GUIElementState::NormalOn);
			else
				mVisibleElements[mSelectedIdx].button->_setState(GUIElementState::Normal);
		}

		mSelectedIdx = idx;
		if (mVisibleElements[mSelectedIdx].button->_isOn())
			mVisibleElements[mSelectedIdx].button->_setState(GUIElementState::HoverOn);
		else
			mVisibleElements[mSelectedIdx].button->_setState(GUIElementState::Hover);

		mParent->ElementSelected(mVisibleElements[mSelectedIdx].idx);
	}

	void GUIDropDownContent::SelectNext(UINT32 startIdx)
	{
		UINT32 numElements = (UINT32)mDropDownData.entries.Size();

		bool gotNextIndex = false;
		UINT32 nextIdx = startIdx;
		for (UINT32 i = 0; i < numElements; i++)
		{
			if (nextIdx >= numElements)
				nextIdx = 0; // Wrap around

			GUIDropDownDataEntry& entry = mDropDownData.entries[nextIdx];
			if (!entry.IsSeparator())
			{
				gotNextIndex = true;
				break;
			}

			nextIdx++;
		}

		if (gotNextIndex)
		{
			while (nextIdx < mRangeStart || nextIdx >= mRangeEnd)
				mParent->ScrollDown();

			UINT32 visIdx = 0;
			for (auto& visElem : mVisibleElements)
			{
				if (visElem.idx == nextIdx)
				{
					setSelected(visIdx);
					break;
				}

				visIdx++;
			}
		}
	}

	void GUIDropDownContent::SelectPrevious(UINT32 startIdx)
	{
		UINT32 numElements = (UINT32)mDropDownData.entries.Size();

		bool gotNextIndex = false;
		INT32 prevIdx = (INT32)startIdx;

		for (UINT32 i = 0; i < numElements; i++)
		{
			if (prevIdx < 0)
				prevIdx = numElements - 1; // Wrap around

			GUIDropDownDataEntry& entry = mDropDownData.entries[prevIdx];
			if (!entry.IsSeparator())
			{
				gotNextIndex = true;
				break;
			}

			prevIdx--;
		}

		if (gotNextIndex)
		{
			while (prevIdx < (INT32)mRangeStart || prevIdx >= (INT32)mRangeEnd)
				mParent->ScrollUp();

			UINT32 visIdx = 0;
			for (auto& visElem : mVisibleElements)
			{
				if (visElem.idx == (UINT32)prevIdx)
				{
					setSelected(visIdx);
					break;
				}

				visIdx++;
			}
		}
	}

	Vector2I GUIDropDownContent::_getOptimalSize() const
	{
		Vector2I optimalSize;
		for (auto& visElem : mVisibleElements)
		{
			const GUIDropDownDataEntry& element = mDropDownData.entries[visElem.idx];

			optimalSize.y += (INT32)getElementHeight(visElem.idx);

			if (element.IsSeparator())
				optimalSize.x = std::max(optimalSize.x, visElem.separator->_getOptimalSize().x);
			else
				optimalSize.x = std::max(optimalSize.x, visElem.button->_getOptimalSize().x);
		}

		return optimalSize;
	}

	void GUIDropDownContent::_updateLayoutInternal(const GUILayoutData& data)
	{
		GUILayoutData childData = data;
		INT32 yOffset = data.area.y;

		for (auto& visElem : mVisibleElements)
		{
			const GUIDropDownDataEntry& element = mDropDownData.entries[visElem.idx];

			GUIElement* guiMainElement = nullptr;
			if (element.IsSeparator())
				guiMainElement = visElem.separator;
			else
				guiMainElement = visElem.button;

			childData.area.y = yOffset;
			childData.area.height = getElementHeight(visElem.idx);

			yOffset += childData.area.height;

			guiMainElement->_setLayoutData(childData);

			// Shortcut label
			GUILabel* shortcutLabel = visElem.shortcutLabel;
			if (shortcutLabel != nullptr)
				shortcutLabel->_setLayoutData(childData);
		}
	}

	const String& GUIDropDownContent::GetGUITypeName()
	{
		static String typeName = "GUIDropDownContent";
		return typeName;
	}
}
