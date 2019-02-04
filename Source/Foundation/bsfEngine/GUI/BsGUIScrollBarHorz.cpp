//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollBarHorz.h"
#include "GUI/BsGUIDimensions.h"

namespace bs
{
	GUIScrollBarHorz::GUIScrollBarHorz(bool resizeable, const String& styleName, const GUIDimensions& dimensions)
		:GUIScrollBar(true, resizeable, styleName, dimensions)
	{

	}

	GUIScrollBarHorz* GUIScrollBarHorz::create(const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarHorz>()) GUIScrollBarHorz(false, getStyleName<GUIScrollBarHorz>(false, styleName), 
			GUIDimensions::create());
	}

	GUIScrollBarHorz* GUIScrollBarHorz::create(bool resizeable, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarHorz>()) GUIScrollBarHorz(resizeable, getStyleName<GUIScrollBarHorz>(resizeable, styleName), 
			GUIDimensions::create());
	}

	GUIScrollBarHorz* GUIScrollBarHorz::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarHorz>()) GUIScrollBarHorz(false, getStyleName<GUIScrollBarHorz>(false, styleName), 
			GUIDimensions::create(options));
	}

	GUIScrollBarHorz* GUIScrollBarHorz::create(bool resizeable, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarHorz>()) GUIScrollBarHorz(resizeable, getStyleName<GUIScrollBarHorz>(resizeable, styleName), 
			GUIDimensions::create(options));
	}

	const String& GUIScrollBarHorz::getGUITypeName(bool resizable)
	{
		static String typeName = "ScrollBarHorz";
		static String resizableTypeName = "ResizeableScrollBarHorz";

		if(resizable)
			return resizableTypeName;
		else
			return typeName;
	}
}