//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIVector3Field.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutY.h"
#include "BsGUIFloatField.h"
#include "BsGUILabel.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUIVector3Field::ELEMENT_LABEL_WIDTH = 10;

	GUIVector3Field::GUIVector3Field(const PrivatelyConstruct& dummy, const GUIContent& labelContent, 
		UINT32 labelWidth, const String& style, const GUIDimensions& dimensions, bool withLabel)
		:TGUIField(dummy, labelContent, labelWidth, style, dimensions, withLabel),
		mFieldX(nullptr), mFieldY(nullptr), mFieldZ(nullptr)
	{
		mFieldX = GUIFloatField::create(HString(L"X"), ELEMENT_LABEL_WIDTH, getSubStyleName(getFloatFieldStyleType()));
		mFieldY = GUIFloatField::create(HString(L"Y"), ELEMENT_LABEL_WIDTH, getSubStyleName(getFloatFieldStyleType()));
		mFieldZ = GUIFloatField::create(HString(L"Z"), ELEMENT_LABEL_WIDTH, getSubStyleName(getFloatFieldStyleType()));

		mFieldX->onValueChanged.connect(std::bind(&GUIVector3Field::valueChanged, this, _1));
		mFieldY->onValueChanged.connect(std::bind(&GUIVector3Field::valueChanged, this, _1));
		mFieldZ->onValueChanged.connect(std::bind(&GUIVector3Field::valueChanged, this, _1));

		mFieldX->onConfirm.connect(std::bind(&GUIVector3Field::inputConfirmed, this));
		mFieldY->onConfirm.connect(std::bind(&GUIVector3Field::inputConfirmed, this));
		mFieldZ->onConfirm.connect(std::bind(&GUIVector3Field::inputConfirmed, this));

		mLayout->removeElement(mLabel);

		GUILayout* layout = mLayout->addNewElement<GUILayoutY>();
		layout->addElement(mLabel);
		mLabel->resetDimensions();

		GUILayout* elementLayout = layout->addNewElement<GUILayoutX>();

		elementLayout->addElement(mFieldX);
		elementLayout->addElement(mFieldY);
		elementLayout->addElement(mFieldZ);
	}

	Vector3 GUIVector3Field::getValue() const
	{
		Vector3 value;
		value.x = mFieldX->getValue();
		value.y = mFieldY->getValue();
		value.z = mFieldZ->getValue();

		return value;
	}

	void GUIVector3Field::setValue(const Vector3& value)
	{
		mFieldX->setValue(value.x);
		mFieldY->setValue(value.y);
		mFieldZ->setValue(value.z);
	}

	bool GUIVector3Field::hasInputFocus() const
	{
		return mFieldX->hasInputFocus() || mFieldY->hasInputFocus() || mFieldZ->hasInputFocus();
	}

	void GUIVector3Field::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mFieldX->setTint(color);
		mFieldY->setTint(color);
		mFieldZ->setTint(color);
	}

	void GUIVector3Field::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mFieldX->setStyle(getSubStyleName(getFloatFieldStyleType()));
		mFieldY->setStyle(getSubStyleName(getFloatFieldStyleType()));
		mFieldZ->setStyle(getSubStyleName(getFloatFieldStyleType()));
	}

	void GUIVector3Field::valueChanged(float newValue)
	{
		Vector3 value = getValue();
		onValueChanged(value);
	}

	void GUIVector3Field::inputConfirmed()
	{
		onConfirm();
	}

	const String& GUIVector3Field::getGUITypeName()
	{
		static String typeName = "GUIVector3Field";
		return typeName;
	}

	const String& GUIVector3Field::getFloatFieldStyleType()
	{
		static String LABEL_STYLE_TYPE = "EditorFloatField";
		return LABEL_STYLE_TYPE;
	}
}