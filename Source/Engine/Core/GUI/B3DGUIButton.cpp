//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIButton.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUICommandEvent.h"

using namespace b3d;

const String& GUIButton::GetGuiTypeName()
{
	static String name = "Button";
	return name;
}

GUIButton::GUIButton(PrivatelyConstruct, const GUIContent& content, const String& styleClass, const GUISizeConstraints& dimensions)
	: GUIClickable(styleClass, content, dimensions)
{}

bool GUIButton::DoOnCommandEvent(const GUICommandEvent& ev)
{
	const bool processed = GUIClickable::DoOnCommandEvent(ev);

	if(ev.GetType() == GUICommandEventType::Confirm)
	{
		if(!IsDisabled())
		{
			OnClick();
			return true;
		}
	}

	return processed;
}
