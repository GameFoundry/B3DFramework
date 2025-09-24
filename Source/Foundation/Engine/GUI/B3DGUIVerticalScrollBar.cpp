//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIVerticalScrollBar.h"
#include "GUI/B3DGUISizeConstraints.h"

using namespace b3d;

GUIVerticalScrollBar::GUIVerticalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIScrollBar(false, false, styleName, dimensions)
{
}

const String& GUIVerticalScrollBar::GetGuiTypeName()
{
	static const String styleClass = "VerticalScrollBar";
	return styleClass;
}

GUIResizableVerticalScrollBar::GUIResizableVerticalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIScrollBar(false, true, styleName, dimensions)
{
}

const String& GUIResizableVerticalScrollBar::GetGuiTypeName()
{
	static const String styleClass = "ResizableVerticalScrollBar";
	return styleClass;
}
