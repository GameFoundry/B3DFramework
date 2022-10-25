//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollBarVert.h"
#include "GUI/BsGUIDimensions.h"

namespace bs
{
	GUIScrollBarVert::GUIScrollBarVert(bool resizeable, const String& styleName, const GUIDimensions& dimensions)
		: GUIScrollBar(false, resizeable, styleName, dimensions)
	{
	}

	GUIScrollBarVert* GUIScrollBarVert::Create(const String& styleName)
	{
		return new(bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(false, GetStyleName<GUIScrollBarVert>(false, styleName), GUIDimensions::Create());
	}

	GUIScrollBarVert* GUIScrollBarVert::Create(bool resizeable, const String& styleName)
	{
		return new(bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(resizeable, GetStyleName<GUIScrollBarVert>(resizeable, styleName), GUIDimensions::Create());
	}

	GUIScrollBarVert* GUIScrollBarVert::Create(const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(false, GetStyleName<GUIScrollBarVert>(false, styleName), GUIDimensions::Create(options));
	}

	GUIScrollBarVert* GUIScrollBarVert::Create(bool resizeable, const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(resizeable, GetStyleName<GUIScrollBarVert>(resizeable, styleName), GUIDimensions::Create(options));
	}

	const String& GUIScrollBarVert::GetGuiTypeName(bool resizable)
	{
		static String typeName = "ScrollBarVert";
		static String resizableTypeName = "ResizeableScrollBarVert";

		if(resizable)
			return resizableTypeName;
		else
			return typeName;
	}
} // namespace bs
