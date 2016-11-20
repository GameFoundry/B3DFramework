//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIFieldBase.h"
#include "BsGUILabel.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutUtility.h"

namespace bs
{
	const UINT32 GUIFieldBase::DEFAULT_LABEL_WIDTH = 100;

	GUIFieldBase::GUIFieldBase(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		:GUIElementContainer(dimensions, style), mLabel(nullptr)
	{
		mLayout = GUILayoutX::create();
		_registerChildElement(mLayout);

		if(withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), getSubStyleName(getLabelStyleType()));
			mLayout->addElement(mLabel);
		}
	}

	void GUIFieldBase::_updateLayoutInternal(const GUILayoutData& data)
	{
		mLayout->_setLayoutData(data);
		mLayout->_updateLayoutInternal(data);
	}

	Vector2I GUIFieldBase::_getOptimalSize() const
	{
		return GUILayoutUtility::calcOptimalSize(mLayout);
	}

	void GUIFieldBase::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));
	}
}