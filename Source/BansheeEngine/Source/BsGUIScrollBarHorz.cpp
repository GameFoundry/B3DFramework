//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIScrollBarHorz.h"
#include "BsGUIDimensions.h"

namespace bs
{
	GUIScrollBarHorz::GUIScrollBarHorz(bool resizeable, const String& styleName, const GUIDimensions& dimensions)
		:GUIScrollBar(true, resizeable, styleName, dimensions)
	{

	}

	GUIScrollBarHorz::~GUIScrollBarHorz()
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