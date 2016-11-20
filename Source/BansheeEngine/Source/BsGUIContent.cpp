//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIContent.h"

namespace bs
{
	const UINT32 GUIContent::IMAGE_TEXT_SPACING = 3;

	GUIContentImages::GUIContentImages(const HSpriteTexture& image)
		:normal(image), hover(image), active(image), focused(image), 
		normalOn(image), hoverOn(image), activeOn(image), focusedOn(image)
	{ }

	GUIContent::GUIContent()
		:mText(L"")
	{ }

	GUIContent::GUIContent(const HString& text)
		:mText(text)
	{ }

	GUIContent::GUIContent(const HString& text, const HString& tooltip)
		:mText(text), mTooltipText(tooltip)
	{ }

	GUIContent::GUIContent(const GUIContentImages& image)
		:mImages(image)
	{ }

	GUIContent::GUIContent(const HSpriteTexture& image, const HString& tooltip)
		:mImages(image), mTooltipText(tooltip)
	{ }

	GUIContent::GUIContent(const HString& text, const GUIContentImages& image)
		:mText(text), mImages(image)
	{ }

	GUIContent::GUIContent(const HString& text, const GUIContentImages& image, const HString& tooltip)
		:mText(text), mImages(image), mTooltipText(tooltip)
	{ }

	const HSpriteTexture& GUIContent::getImage(GUIElementState state) const
	{
		switch (state)
		{
		case GUIElementState::Normal:
			return mImages.normal;
		case GUIElementState::Hover:
			return mImages.hover;
		case GUIElementState::Active:
			return mImages.active;
		case GUIElementState::Focused:
			return mImages.focused;
		case GUIElementState::NormalOn:
			return mImages.normalOn;
		case GUIElementState::HoverOn:
			return mImages.hoverOn;
		case GUIElementState::ActiveOn:
			return mImages.activeOn;
		case GUIElementState::FocusedOn:
			return mImages.focusedOn;
		default:
			return mImages.normal;
		}
	}
}