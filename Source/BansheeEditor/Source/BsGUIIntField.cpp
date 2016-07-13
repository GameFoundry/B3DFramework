//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIIntField.h"
#include "BsGUILayout.h"
#include "BsGUILabel.h"
#include "BsGUIInputBox.h"
#include "BsGUIWidget.h"
#include "BsGUIMouseEvent.h"
#include "BsCursor.h"
#include "BsUndoRedo.h"
#include "BsViewport.h"
#include "BsCmdInputFieldValueChange.h"
#include <regex>

using namespace std::placeholders;

namespace BansheeEngine
{
	const INT32 GUIIntField::DRAG_SPEED = 5;

	GUIIntField::GUIIntField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		: TGUIField(dummy, labelContent, labelWidth, style, dimensions, withLabel), mInputBox(nullptr), mValue(0)
		, mLastDragPos(0), mMinValue(std::numeric_limits<INT32>::lowest())
		, mMaxValue(std::numeric_limits<INT32>::max()), mIsDragging(false), mIsDragCursorSet(false), mHasInputFocus(false)
	{
		mInputBox = GUIInputBox::create(false, GUIOptions(GUIOption::flexibleWidth()), getSubStyleName(getInputStyleType()));
		mInputBox->setFilter(&GUIIntField::intFilter);

		mInputBox->onValueChanged.connect(std::bind((void(GUIIntField::*)(const WString&))&GUIIntField::valueChanged, this, _1));
		mInputBox->onFocusChanged.connect(std::bind(&GUIIntField::focusChanged, this, _1));
		mInputBox->onConfirm.connect(std::bind(&GUIIntField::inputConfirmed, this));

		mLayout->addElement(mInputBox);

		setValue(0);
		mInputBox->setText(L"0");
	}

	GUIIntField::~GUIIntField()
	{

	}

	bool GUIIntField::_hasCustomCursor(const Vector2I position, CursorType& type) const
	{
		if (!_isDisabled())
		{
			Rect2I draggableArea;

			if (mLabel != nullptr)
				draggableArea = mLabel->_getLayoutData().area;

			if (draggableArea.contains(position))
			{
				type = CursorType::ArrowLeftRight;
				return true;
			}
		}

		return false;
	}

	bool GUIIntField::_mouseEvent(const GUIMouseEvent& event)
	{
		GUIElementContainer::_mouseEvent(event);

		Rect2I draggableArea;

		if(mLabel != nullptr)
			draggableArea = mLabel->_getLayoutData().area;

		if(event.getType() == GUIMouseEventType::MouseDragStart)
		{
			if (!_isDisabled())
			{
				if (draggableArea.contains(event.getDragStartPosition()))
				{
					mLastDragPos = event.getPosition().x;
					mIsDragging = true;
				}
			}

			return true;
		}
		else if(event.getType() == GUIMouseEventType::MouseDrag)
		{
			if (!_isDisabled())
			{
				if (mIsDragging)
				{
					INT32 xDiff = event.getPosition().x - mLastDragPos;

					INT32 jumpAmount = 0;
					if (event.getPosition().x < 0)
					{
						Vector2I cursorScreenPos = Cursor::instance().getScreenPosition();
						cursorScreenPos.x += _getParentWidget()->getTarget()->getWidth();
						jumpAmount = _getParentWidget()->getTarget()->getWidth();

						Cursor::instance().setScreenPosition(cursorScreenPos);
					}
					else if (event.getPosition().x >= _getParentWidget()->getTarget()->getWidth())
					{
						Vector2I cursorScreenPos = Cursor::instance().getScreenPosition();
						cursorScreenPos.x -= _getParentWidget()->getTarget()->getWidth();
						jumpAmount = -_getParentWidget()->getTarget()->getWidth();

						Cursor::instance().setScreenPosition(cursorScreenPos);
					}

					INT32 oldValue = getValue();
					INT32 newValue = oldValue;

					if (xDiff >= DRAG_SPEED)
					{
						while (xDiff >= DRAG_SPEED)
						{
							newValue++;
							xDiff -= DRAG_SPEED;
						}
					}
					else if (xDiff <= -DRAG_SPEED)
					{
						while (xDiff <= -DRAG_SPEED)
						{
							newValue--;
							xDiff += DRAG_SPEED;
						}
					}

					mLastDragPos += (newValue - oldValue) * DRAG_SPEED + jumpAmount;

					if (oldValue != newValue)
					{
						setValue(newValue);
						valueChanged(newValue);
					}
				}
			}

			return true;
		}
		else if(event.getType() == GUIMouseEventType::MouseDragEnd)
		{
			if (!_isDisabled())
				mIsDragging = false;

			return true;
		}

		return false;
	}

	void GUIIntField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mInputBox->setStyle(getSubStyleName(getInputStyleType()));
	}

	void GUIIntField::setValue(INT32 value)
	{
		mValue = Math::clamp(value, mMinValue, mMaxValue);

		// Only update with new value if it actually changed, otherwise
		// problems can occur when user types in "0." and the field
		// updates back to "0" effectively making "." unusable
		float curValue = parseFloat(mInputBox->getText());
		if (mValue != curValue)
			mInputBox->setText(toWString(mValue));
	}

	void GUIIntField::setRange(INT32 min, INT32 max)
	{
		mMinValue = min;
		mMaxValue = max;
	}

	void GUIIntField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mInputBox->setTint(color);
	}

	void GUIIntField::_setValue(INT32 value, bool triggerEvent)
	{
		setValue(value);

		if (triggerEvent)
			onValueChanged(mValue);
	}

	const String& GUIIntField::getGUITypeName()
	{
		static String typeName = "GUIIntField";
		return typeName;
	}

	const String& GUIIntField::getInputStyleType()
	{
		static String LABEL_STYLE_TYPE = "EditorFieldInput";
		return LABEL_STYLE_TYPE;
	}

	void GUIIntField::valueChanged(const WString& newValue)
	{
		valueChanged(parseINT32(newValue));
	}

	void GUIIntField::valueChanged(INT32 newValue)
	{
		CmdInputFieldValueChange<GUIIntField, INT32>::execute(this, newValue);
	}

	void GUIIntField::focusChanged(bool focus)
	{
		if (focus)
		{
			UndoRedo::instance().pushGroup("InputBox");
			mHasInputFocus = true;
		}
		else
		{
			UndoRedo::instance().popGroup("InputBox");
			mHasInputFocus = false;
		}
	}

	void GUIIntField::inputConfirmed()
	{
		onConfirm();
	}

	bool GUIIntField::intFilter(const WString& str)
	{
		return std::regex_match(str, std::wregex(L"-?(\\d+)?"));
	}
}