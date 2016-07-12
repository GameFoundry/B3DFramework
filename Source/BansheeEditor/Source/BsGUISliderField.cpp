//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUISliderField.h"
#include "BsGUILayout.h"
#include "BsGUIInputBox.h"
#include "BsGUISpace.h"
#include "BsGUISlider.h"
#include "BsGUILabel.h"
#include "BsCmdInputFieldValueChange.h"
#include <regex>

using namespace std::placeholders;

namespace BansheeEngine
{
	GUISliderField::GUISliderField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		:TGUIField(dummy, labelContent, labelWidth, style, dimensions, withLabel), mInputBox(nullptr), mSlider(nullptr), mHasInputFocus(false)
	{
		mSlider = GUISliderHorz::create(GUIOptions(GUIOption::flexibleWidth()), getSubStyleName(getSliderStyleType()));
		mSlider->onChanged.connect(std::bind(&GUISliderField::sliderChanged, this, _1));

		mInputBox = GUIInputBox::create(false, GUIOptions(GUIOption::fixedWidth(75)), getSubStyleName(getInputStyleType()));
		mInputBox->setFilter(&GUISliderField::floatFilter);

		mInputBox->onValueChanged.connect(std::bind((void(GUISliderField::*)(const WString&))&GUISliderField::inputBoxValueChanging, this, _1));
		mInputBox->onConfirm.connect(std::bind((void(GUISliderField::*)(void))&GUISliderField::inputBoxValueChanged, this));
		mInputBox->onFocusChanged.connect(std::bind(&GUISliderField::inputBoxFocusChanged, this, _1));

		mLayout->addElement(mSlider);
		mLayout->addNewElement<GUIFixedSpace>(5);
		mLayout->addElement(mInputBox);

		setValue(0);
		mInputBox->setText(L"0");
	}

	GUISliderField::~GUISliderField()
	{

	}

	float GUISliderField::getValue() const
	{
		return mSlider->getValue();
	}

	float GUISliderField::getStep() const
	{
		return mSlider->getStep();
	}

	float GUISliderField::setValue(float value)
	{
		float origValue = mSlider->getValue();
		if (origValue != value)
			mSlider->setValue(value);

		float clampedValue = mSlider->getValue();

		// Only update with new value if it actually changed, otherwise
		// problems can occur when user types in "0." and the field
		// updates back to "0" effectively making "." unusable
		float curValue = parseFloat(mInputBox->getText());
		if (clampedValue != curValue)
			mInputBox->setText(toWString(clampedValue));
		return clampedValue;
	}

	void GUISliderField::setRange(float min, float max)
	{
		mSlider->setRange(min, max);
	}

	void GUISliderField::setStep(float step)
	{
		mSlider->setStep(step / (mSlider->getRangeMaximum() - mSlider->getRangeMinimum()));
	}

	void GUISliderField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mInputBox->setTint(color);
	}

	void GUISliderField::_setValue(float value, bool triggerEvent)
	{
		float clamped = setValue(value);

		if (triggerEvent)
			onValueChanged(clamped);
	}

	const String& GUISliderField::getGUITypeName()
	{
		static String typeName = "GUISliderField";
		return typeName;
	}

	const String& GUISliderField::getInputStyleType()
	{
		static String LABEL_STYLE_TYPE = "EditorFieldInput";
		return LABEL_STYLE_TYPE;
	}

	const String& GUISliderField::getSliderStyleType()
	{
		static String SLIDER_STYLE_TYPE = "EditorSliderInput";
		return SLIDER_STYLE_TYPE;
	}

	void GUISliderField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mSlider->setStyle(getSubStyleName(getSliderStyleType()));
		mInputBox->setStyle(getSubStyleName(getInputStyleType()));
	}

	void GUISliderField::inputBoxValueChanging(const WString& newValue) {
		inputBoxValueChanged(false);
	}

	void GUISliderField::inputBoxValueChanged(bool confirmed = true)
	{
		float newFloatValue = parseFloat(mInputBox->getText());
		if (mSlider->getValue() != newFloatValue) {
			if (confirmed) {
				CmdInputFieldValueChange<GUISliderField, float>::execute(this, newFloatValue);
			}
			else
			{
				mSlider->setValue(newFloatValue);
				onValueChanged(mSlider->getValue());
			}
		}
		else if (mInputBox->getText() == L"" && confirmed) //Avoid leaving label blank
		{
			mInputBox->setText(L"0");
		}
	}

	void GUISliderField::inputBoxFocusChanged(bool focus)
	{
		if (focus)
		{
			UndoRedo::instance().pushGroup("InputBox");
			mHasInputFocus = true;
		}
		else
		{
			UndoRedo::instance().popGroup("InputBox");
			inputBoxValueChanged();
			mHasInputFocus = false;
		}
	}

	void GUISliderField::sliderChanged(float newValue)
	{
		//float old = mSlider->getValue();
		_setValue(newValue, true);
	}

	bool GUISliderField::floatFilter(const WString& str)
	{
		//-?(\\d+(\\.\\d*)?)?
		//[-+]?[0-9]*\.?[0-9]*
		bool result = std::regex_match(str, std::wregex(L"-?(\\d*(\\.\\d*)?)?"));
		return result;
	}
}