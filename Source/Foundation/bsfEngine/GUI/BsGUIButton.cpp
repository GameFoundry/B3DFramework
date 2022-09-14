//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUICommandEvent.h"

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

	bool GUIButton::CommandEventInternal(const GUICommandEvent& ev)
	{
		const bool processed = GUIButtonBase::CommandEventInternal(ev);

		if(ev.getType() == GUICommandEventType::Confirm)
		{
			if(!IsDisabledInternal())
			{
				onClick();
				return true;
			}
		}

		return processed;
	}
}
