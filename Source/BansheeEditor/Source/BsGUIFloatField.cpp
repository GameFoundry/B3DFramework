//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIFloatField.h"
#include "BsGUILayout.h"
#include "BsGUILabel.h"
#include "BsGUIInputBox.h"
#include "BsGUIWidget.h"
#include "BsGUIMouseEvent.h"
#include "BsViewport.h"
#include "BsCursor.h"
#include "BsCmdInputFieldValueChange.h"
#include <regex>

using namespace std::placeholders;

namespace BansheeEngine
{
	const float GUIFloatField::DRAG_SPEED = 0.05f;

	GUIFloatField::GUIFloatField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth, 
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		: TGUIField(dummy, labelContent, labelWidth, style, dimensions, withLabel), mInputBox(nullptr), mValue(0.0f)
		, mLastDragPos(0), mMinValue(std::numeric_limits<float>::lowest()), mMaxValue(std::numeric_limits<float>::max())
		, mIsDragging(false), mHasInputFocus(false), mStep(0.0f)
	{
		mInputBox = GUIInputBox::create(false, GUIOptions(GUIOption::flexibleWidth()), getSubStyleName(getInputStyleType()));
		mInputBox->setFilter(&GUIFloatField::floatFilter);

		mInputBox->onValueChanged.connect(std::bind((void(GUIFloatField::*)(const WString&))&GUIFloatField::valueChanging, this, _1));
		mInputBox->onFocusChanged.connect(std::bind(&GUIFloatField::focusChanged, this, _1));
		mInputBox->onConfirm.connect(std::bind(&GUIFloatField::inputConfirmed, this));

		mLayout->addElement(mInputBox);

		setValue(0);
		mInputBox->setText(L"0");
	}

	GUIFloatField::~GUIFloatField()
	{

	}

	bool GUIFloatField::_hasCustomCursor(const Vector2I position, CursorType& type) const
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

	bool GUIFloatField::_mouseEvent(const GUIMouseEvent& event)
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

					float oldValue = getValue();
					float newValue = oldValue + xDiff * DRAG_SPEED;

					mLastDragPos = event.getPosition().x + jumpAmount;

					if (oldValue != newValue)
						valueChanged(newValue);
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

	float GUIFloatField::getValue() const
	{
		return applyRangeAndStep(mValue);
	}

	float GUIFloatField::setValue(float value)
	{
		if (mValue == value)
			return value;

		mValue = value;
		
		value = applyRangeAndStep(value);
		setText(value);

		return value;
	}

	void GUIFloatField::setRange(float min, float max)
	{
		mMinValue = min;
		mMaxValue = max;
	}

	void GUIFloatField::setStep(float step) 
	{
		mStep = step;
	}

	void GUIFloatField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mInputBox->setTint(color);
	}

	void GUIFloatField::_setValue(float value, bool triggerEvent)
	{
		mValue = value;
		setText(value);

		if(triggerEvent)
			onValueChanged(mValue);
	}

	const String& GUIFloatField::getGUITypeName()
	{
		static String typeName = "GUIFloatField";
		return typeName;
	}

	const String& GUIFloatField::getInputStyleType()
	{
		static String LABEL_STYLE_TYPE = "EditorFieldInput";
		return LABEL_STYLE_TYPE;
	}

	void GUIFloatField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mInputBox->setStyle(getSubStyleName(getInputStyleType()));
	}

	void GUIFloatField::valueChanging(const WString& newValue)
	{
		valueChanged(parseFloat(newValue));
	}

	void GUIFloatField::valueChanged(float newValue)
	{
		CmdInputFieldValueChange<GUIFloatField, float>::execute(this, newValue);
	}

	void GUIFloatField::focusChanged(bool focus)
	{
		if (focus)
		{
			GlobalUndoRedo::instance().pushGroup("InputBox");

			mHasInputFocus = true;
			onFocusChanged(true);
		}
		else
		{
			GlobalUndoRedo::instance().popGroup("InputBox");

			setText(applyRangeAndStep(mValue));

			mHasInputFocus = false;
			onFocusChanged(false);
		}
	}

	void GUIFloatField::inputConfirmed()
	{
		setText(applyRangeAndStep(mValue));

		onConfirm();
	}

	void GUIFloatField::setText(float value)
	{
		// Only update with new value if it actually changed, otherwise
		// problems can occur when user types in "0." and the field
		// updates back to "0" effectively making "." unusable
		float curValue = parseFloat(mInputBox->getText());
		if (value != curValue)
			mInputBox->setText(toWString(value));
	}

	float GUIFloatField::applyRangeAndStep(float value) const
	{
		if (mStep != 0.0f)
			value = value - fmod(value, mStep);

		return Math::clamp(value, mMinValue, mMaxValue);
	}

	bool GUIFloatField::floatFilter(const WString& str)
	{
		return std::regex_match(str, std::wregex(L"-?(\\d*(\\.\\d*)?)?"));
	}
}