//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIVector2Field.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutY.h"
#include "BsGUIFloatField.h"
#include "BsGUILabel.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUIVector2Field::ELEMENT_LABEL_WIDTH = 10;

	GUIVector2Field::GUIVector2Field(const PrivatelyConstruct& dummy, const GUIContent& labelContent, 
		UINT32 labelWidth, const String& style, const GUIDimensions& dimensions, bool withLabel)
		:TGUIField(dummy, labelContent, labelWidth, style, dimensions, withLabel),
		mFieldX(nullptr), mFieldY(nullptr)
	{
		mFieldX = GUIFloatField::create(HString(L"X"), ELEMENT_LABEL_WIDTH, getSubStyleName(getFloatFieldStyleType()));
		mFieldY = GUIFloatField::create(HString(L"Y"), ELEMENT_LABEL_WIDTH, getSubStyleName(getFloatFieldStyleType()));

		mFieldX->onValueChanged.connect(std::bind(&GUIVector2Field::valueChanged, this, _1));
		mFieldY->onValueChanged.connect(std::bind(&GUIVector2Field::valueChanged, this, _1));
		mFieldX->onConfirm.connect(std::bind(&GUIVector2Field::inputConfirmed, this));
		mFieldY->onConfirm.connect(std::bind(&GUIVector2Field::inputConfirmed, this));

		mLayout->removeElement(mLabel);

		GUILayout* layout = mLayout->addNewElement<GUILayoutY>();

		layout->addElement(mLabel);
		mLabel->resetDimensions();

		GUILayout* elementLayout = layout->addNewElement<GUILayoutX>();

		elementLayout->addElement(mFieldX);
		elementLayout->addElement(mFieldY);
	}

	Vector2 GUIVector2Field::getValue() const
	{
		Vector2 value;
		value.x = mFieldX->getValue();
		value.y = mFieldY->getValue();

		return value;
	}

	void GUIVector2Field::setValue(const Vector2& value)
	{
		mFieldX->setValue(value.x);
		mFieldY->setValue(value.y);
	}

	bool GUIVector2Field::hasInputFocus() const
	{
		return mFieldX->hasInputFocus() || mFieldY->hasInputFocus();
	}

	void GUIVector2Field::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mFieldX->setTint(color);
		mFieldY->setTint(color);
	}

	void GUIVector2Field::valueChanged(float newValue)
	{
		Vector2 value = getValue();
		onValueChanged(value);
	}

	void GUIVector2Field::inputConfirmed()
	{
		onConfirm();
	}

	void GUIVector2Field::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mFieldX->setStyle(getSubStyleName(getFloatFieldStyleType()));
		mFieldY->setStyle(getSubStyleName(getFloatFieldStyleType()));
	}

	const String& GUIVector2Field::getGUITypeName()
	{
		static String typeName = "GUIVector2Field";
		return typeName;
	}

	const String& GUIVector2Field::getFloatFieldStyleType()
	{
		static String LABEL_STYLE_TYPE = "EditorFloatField";
		return LABEL_STYLE_TYPE;
	}
}