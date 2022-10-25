//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUICommandEvent.h"

namespace bs
{
	const String& GUIButton::GetGuiTypeName()
	{
		static String name = "Button";
		return name;
	}

	GUIButton::GUIButton(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
		: GUIButtonBase(styleName, content, dimensions)
	{}

	GUIButton* GUIButton::Create(const HString& text, const String& styleName)
	{
		return Create(GUIContent(text), styleName);
	}

	GUIButton* GUIButton::Create(const HString& text, const GUIOptions& options, const String& styleName)
	{
		return Create(GUIContent(text), options, styleName);
	}

	GUIButton* GUIButton::Create(const GUIContent& content, const String& styleName)
	{
		return new(bs_alloc<GUIButton>()) GUIButton(GetStyleName<GUIButton>(styleName), content, GUIDimensions::Create());
	}

	GUIButton* GUIButton::Create(const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUIButton>()) GUIButton(GetStyleName<GUIButton>(styleName), content, GUIDimensions::Create(options));
	}

	bool GUIButton::CommandEventInternal(const GUICommandEvent& ev)
	{
		const bool processed = GUIButtonBase::CommandEventInternal(ev);

		if(ev.GetType() == GUICommandEventType::Confirm)
		{
			if(!IsDisabledInternal())
			{
				OnClick();
				return true;
			}
		}

		return processed;
	}
} // namespace bs
