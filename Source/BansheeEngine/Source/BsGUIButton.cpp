//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIButton.h"
#include "BsGUIDimensions.h"

namespace bs
{
	const String& GUIButton::getGUITypeName()
	{
		static String name = "Button";
		return name;
	}

	GUIButton::GUIButton(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
		:GUIButtonBase(styleName, content, dimensions)
	{ }

	GUIButton* GUIButton::create(const HString& text, const String& styleName)
	{
		return create(GUIContent(text), styleName);
	}

	GUIButton* GUIButton::create(const HString& text, const GUIOptions& options, const String& styleName)
	{
		return create(GUIContent(text), options, styleName);
	}

	GUIButton* GUIButton::create(const GUIContent& content, const String& styleName)
	{
		return new (bs_alloc<GUIButton>()) GUIButton(getStyleName<GUIButton>(styleName), content, GUIDimensions::create());
	}

	GUIButton* GUIButton::create(const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIButton>()) GUIButton(getStyleName<GUIButton>(styleName), content, GUIDimensions::create(options));
	}
}