//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIWindowFrame.h"
#include "BsGUISkin.h"
#include "BsGUIDimensions.h"

namespace BansheeEngine
{
	const String& GUIWindowFrame::getGUITypeName()
	{
		static String name = "WindowFrame";
		return name;
	}

	GUIWindowFrame::GUIWindowFrame(const String& styleName, const GUIDimensions& dimensions)
		:GUITexture(styleName, HSpriteTexture(), TextureScaleMode::StretchToFit, true, dimensions)
	{

	}

	GUIWindowFrame::~GUIWindowFrame()
	{ }

	GUIWindowFrame* GUIWindowFrame::create(const String& styleName)
	{
		return new (bs_alloc<GUIWindowFrame>()) GUIWindowFrame(getStyleName<GUIWindowFrame>(styleName), GUIDimensions::create());
	}

	GUIWindowFrame* GUIWindowFrame::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIWindowFrame>()) GUIWindowFrame(getStyleName<GUIWindowFrame>(styleName), GUIDimensions::create(options));
	}

	void GUIWindowFrame::setFocused(bool focused)
	{
		Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		if(focused)
			mActiveTexture = _getStyle()->focused.texture;
		else
			mActiveTexture = _getStyle()->normal.texture;

		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}
}