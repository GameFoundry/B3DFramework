//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIColorField.h"
#include "BsGUILayout.h"
#include "BsGUILabel.h"
#include "BsGUIColor.h"

using namespace std::placeholders;

namespace bs
{
	GUIColorField::GUIColorField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		: TGUIField(dummy, labelContent, labelWidth, style, dimensions, withLabel)
		, mLabelWidth(100), mLabel(nullptr), mColor(nullptr)
	{
		mColor = GUIColor::create(getSubStyleName(getColorInputStyleType()));
		mColor->onClicked.connect(std::bind(&GUIColorField::clicked, this));

		mLayout->addElement(mColor);
	}


	GUIColorField::~GUIColorField()
	{

	}

	void GUIColorField::setValue(const Color& color)
	{
		mValue = color;
		mColor->setColor(color);
	}

	void GUIColorField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mColor->setTint(color);
	}

	Vector2I GUIColorField::_getOptimalSize() const
	{
		Vector2I optimalsize = mColor->_getOptimalSize();

		if(mLabel != nullptr)
		{
			optimalsize.x += mLabel->_getOptimalSize().x;
			optimalsize.y = std::max(optimalsize.y, mLabel->_getOptimalSize().y);
		}

		return optimalsize;
	}

	void GUIColorField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mColor->setStyle(getSubStyleName(getColorInputStyleType()));
	}

	void GUIColorField::clicked()
	{
		onClicked();
	}

	const String& GUIColorField::getGUITypeName()
	{
		static String typeName = "GUIColorField";
		return typeName;
	}

	const String& GUIColorField::getColorInputStyleType()
	{
		static String STYLE_TYPE = "EditorFieldColor";
		return STYLE_TYPE;
	}
}