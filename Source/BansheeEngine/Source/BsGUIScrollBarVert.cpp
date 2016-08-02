//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIScrollBarVert.h"
#include "BsGUIDimensions.h"

namespace BansheeEngine
{
	GUIScrollBarVert::GUIScrollBarVert(bool resizeable, const String& styleName, const GUIDimensions& dimensions)
		:GUIScrollBar(false, resizeable, styleName, dimensions)
	{

	}

	GUIScrollBarVert::~GUIScrollBarVert()
	{

	}

	GUIScrollBarVert* GUIScrollBarVert::create(const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(false, getStyleName<GUIScrollBarVert>(styleName), 
			GUIDimensions::create());
	}

	GUIScrollBarVert* GUIScrollBarVert::create(bool resizeable, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(resizeable, getStyleName<GUIScrollBarVert>(styleName), 
			GUIDimensions::create());
	}

	GUIScrollBarVert* GUIScrollBarVert::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(false, getStyleName<GUIScrollBarVert>(styleName), 
			GUIDimensions::create(options));
	}

	GUIScrollBarVert* GUIScrollBarVert::create(bool resizeable, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIScrollBarVert>()) GUIScrollBarVert(resizeable, getStyleName<GUIScrollBarVert>(styleName), 
			GUIDimensions::create(options));
	}

	const String& GUIScrollBarVert::getGUITypeName()
	{
		static String typeName = "ScrollBarVert";
		return typeName;
	}
}