//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIToggle.h"

using namespace b3d;

const String& GUIToggle::GetGuiTypeName()
{
	static String name = "Toggle";
	return name;
}

GUIToggle::GUIToggle(PrivatelyConstruct, const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIToggleable(contents, styleName, sizeConstraints)
{ }
