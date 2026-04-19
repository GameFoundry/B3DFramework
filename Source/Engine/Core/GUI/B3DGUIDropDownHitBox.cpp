//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIDropDownHitBox.h"
#include "GUI/B3DGUICommandEvent.h"
#include "GUI/B3DGUIMouseEvent.h"

using namespace b3d;

const String& GUIDropDownHitBox::GetGuiTypeName()
{
	static String name = "DropDownHitBox";
	return name;
}

GUIDropDownHitBox* GUIDropDownHitBox::Create(bool captureMouseOver, bool captureMousePresses)
{
	return new(B3DAllocate<GUIDropDownHitBox>())
		GUIDropDownHitBox(captureMouseOver, captureMousePresses, GUISizeConstraints::Create());
}

GUIDropDownHitBox* GUIDropDownHitBox::Create(bool captureMouseOver, bool captureMousePresses, const GUIOptions& options)
{
	return new(B3DAllocate<GUIDropDownHitBox>())
		GUIDropDownHitBox(captureMouseOver, captureMousePresses, GUISizeConstraints::Create(options));
}

GUIDropDownHitBox::GUIDropDownHitBox(bool captureMouseOver, bool captureMousePresses, const GUISizeConstraints& dimensions)
	: GUIElementContainer(dimensions), mCaptureMouseOver(captureMouseOver), mCaptureMousePresses(captureMousePresses)
{
	mOptionFlags.Set(GUIElementOption::ClickThrough);
}

void GUIDropDownHitBox::SetBounds(const GUIPhysicalArea& bounds)
{
	mBounds.clear();
	mBounds.push_back(bounds);
}

void GUIDropDownHitBox::SetBounds(const Vector<GUIPhysicalArea>& bounds)
{
	mBounds = bounds;
}

bool GUIDropDownHitBox::DoOnCommandEvent(const GUICommandEvent& ev)
{
	bool processed = GUIElementContainer::DoOnCommandEvent(ev);

	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		if(!OnFocusGained.Empty())
			OnFocusGained();

		return false;
	}
	else if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		if(!OnFocusLost.Empty())
			OnFocusLost();

		return false;
	}

	return processed;
}

bool GUIDropDownHitBox::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	bool processed = GUIElementContainer::DoOnMouseEvent(ev);

	if(mCaptureMouseOver)
	{
		if(ev.GetType() == GUIMouseEventType::MouseOver)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseOut)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseMove)
		{
			return true;
		}
	}

	if(mCaptureMousePresses)
	{
		if(ev.GetType() == GUIMouseEventType::MouseUp)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseDown)
		{
			return true;
		}
		else if(ev.GetType() == GUIMouseEventType::MouseDoubleClick)
		{
			return true;
		}
	}

	return processed;
}

bool GUIDropDownHitBox::IsInInteractionBounds(const GUIPhysicalPoint& position) const
{
	for(auto& bound : mBounds)
	{
		if(bound.Contains(position))
			return true;
	}

	return false;
}
