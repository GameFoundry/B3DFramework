//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUICommandEvent.h"

namespace bs
{
	const String& GUIButton::GetGUITypeName()
	{
		static String name = "Button";
		return name;
	}

	GUIButton::GUIButton(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
		:GUIButtonBase(styleName, content, dimensions)
	{ }

	GUIButton* GUIButton::create(const HString& text, const String& styleName)
	{
		return Create(GUIContent(text), styleName);
	}

	GUIButton* GUIButton::create(const HString& text, const GUIOptions& options, const String& styleName)
	{
		return Create(GUIContent(text), options, styleName);
	}

	GUIButton* GUIButton::create(const GUIContent& content, const String& styleName)
	{
		return new (bs_alloc<GUIButton>()) GUIButton(getStyleName<GUIButton>(styleName), content, GUIDimensions::create());
	}

	GUIButton* GUIButton::create(const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIButton>()) GUIButton(getStyleName<GUIButton>(styleName), content, GUIDimensions::create(options));
	}

	bool GUIButton::_commandEvent(const GUICommandEvent& ev)
	{
		const bool processed = GUIButtonBase::_commandEvent(ev);

		if(ev.GetType() == GUICommandEventType::Confirm)
		{
			if(!_isDisabled())
			{
				onClick();
				return true;
			}
		}

		return processed;
	}
}
