//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIToggle.h"

using namespace bs;

const String& GUIToggle::GetGuiTypeName()
{
	static String name = "Toggle";
	return name;
}

GUIToggle::GUIToggle(PrivatelyConstruct, const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIToggleable(contents.GeneralContent, styleName, dimensions)
{ }
