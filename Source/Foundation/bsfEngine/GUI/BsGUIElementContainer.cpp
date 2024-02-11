//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIElementContainer.h"
#include "GUI/BsGUIManager.h"

using namespace bs;

GUIElementContainer::GUIElementContainer(const GUISizeConstraints& dimensions, const char* style, GUIElementOptions options)
	: GUIInteractable(style, dimensions, options)
{
	mOptionFlags.Set(GUIElementOption::ClickThrough);
}

GUIElementContainer::GUIElementContainer(const GUISizeConstraints& dimensions, const String& style, GUIElementOptions options)
	: GUIInteractable(style, dimensions, options)
{
	mOptionFlags.Set(GUIElementOption::ClickThrough);
}

Vector2I GUIElementContainer::CalculateUnconstrainedOptimalSize() const
{
	return Vector2I();
}

void GUIElementContainer::SetFocus(bool enabled, bool clear)
{
	if(mFocusElement)
		mFocusElement->SetFocus(enabled, clear);
	else
		GUIInteractable::SetFocus(enabled, clear);
}

bool GUIElementContainer::DoOnCommandEvent(const GUICommandEvent& ev)
{
	// Make sure to pass through focus events to elements below
	if(ev.GetType() == GUICommandEventType::FocusGained)
		return false;
	else if(ev.GetType() == GUICommandEventType::FocusLost)
		return false;

	return GUIInteractable::DoOnCommandEvent(ev);
}
