//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollBarHorz.h"
#include "GUI/BsGUIDimensions.h"

using namespace bs;

GUIScrollBarHorz::GUIScrollBarHorz(bool resizeable, const String& styleName, const GUIDimensions& dimensions)
	: GUIScrollBar(true, resizeable, styleName, dimensions)
{
}

GUIScrollBarHorz* GUIScrollBarHorz::Create(const String& styleName)
{
	return new(B3DAllocate<GUIScrollBarHorz>()) GUIScrollBarHorz(false, GetStyleName<GUIScrollBarHorz>(false, styleName), GUIDimensions::Create());
}

GUIScrollBarHorz* GUIScrollBarHorz::Create(bool resizeable, const String& styleName)
{
	return new(B3DAllocate<GUIScrollBarHorz>()) GUIScrollBarHorz(resizeable, GetStyleName<GUIScrollBarHorz>(resizeable, styleName), GUIDimensions::Create());
}

GUIScrollBarHorz* GUIScrollBarHorz::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIScrollBarHorz>()) GUIScrollBarHorz(false, GetStyleName<GUIScrollBarHorz>(false, styleName), GUIDimensions::Create(options));
}

GUIScrollBarHorz* GUIScrollBarHorz::Create(bool resizeable, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIScrollBarHorz>()) GUIScrollBarHorz(resizeable, GetStyleName<GUIScrollBarHorz>(resizeable, styleName), GUIDimensions::Create(options));
}

const String& GUIScrollBarHorz::GetGuiTypeName(bool resizable)
{
	static String typeName = "ScrollBarHorz";
	static String resizableTypeName = "ResizeableScrollBarHorz";

	if(resizable)
		return resizableTypeName;
	else
		return typeName;
}
