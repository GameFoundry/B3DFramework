//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIHorizontalScrollBar.h"
#include "GUI/B3DGUISizeConstraints.h"

using namespace b3d;

GUIHorizontalScrollBar::GUIHorizontalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIScrollBar(true, false, styleName, sizeConstraints)
{ }

const String& GUIHorizontalScrollBar::GetGuiTypeName()
{
	static const String styleClass = "HorizontalScrollBar";
	return styleClass;
}

GUIResizableHorizontalScrollBar::GUIResizableHorizontalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIScrollBar(true, false, styleName, sizeConstraints)
{ }

const String& GUIResizableHorizontalScrollBar::GetGuiTypeName()
{
	static const String styleClass = "GUIResizableHorizontalScrollBar";
	return styleClass;
}
