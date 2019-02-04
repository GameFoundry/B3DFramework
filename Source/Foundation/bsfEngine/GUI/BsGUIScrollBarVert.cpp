//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollBarVert.h"
#include "GUI/BsGUIDimensions.h"

namespace bs
{
	GUIScrollBarVert::GUIScrollBarVert(bool resizeable, const String& styleName, const GUIDimensions& dimensions)
		:GUIScrollBar(false, resizeable, styleName, dimensions)
	{

	}

	GUIScrollBarVert* GUIScrollBarVert::create(const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(false, getStyleName<GUIScrollBarVert>(false, styleName), 
			GUIDimensions::create());
	}

	GUIScrollBarVert* GUIScrollBarVert::create(bool resizeable, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(resizeable, getStyleName<GUIScrollBarVert>(resizeable, styleName), 
			GUIDimensions::create());
	}

	GUIScrollBarVert* GUIScrollBarVert::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(false, getStyleName<GUIScrollBarVert>(false, styleName), 
			GUIDimensions::create(options));
	}

	GUIScrollBarVert* GUIScrollBarVert::create(bool resizeable, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(resizeable, getStyleName<GUIScrollBarVert>(resizeable, styleName), 
			GUIDimensions::create(options));
	}

	const String& GUIScrollBarVert::getGUITypeName(bool resizable)
	{
		static String typeName = "ScrollBarVert";
		static String resizableTypeName = "ResizeableScrollBarVert";

		if (resizable)
			return resizableTypeName;
		else
			return typeName;
	}
}