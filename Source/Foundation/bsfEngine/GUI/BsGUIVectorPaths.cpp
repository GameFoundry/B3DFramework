//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGUIVectorPaths.h"
#include "StyleSheet/BsGUIStyleSheet.h"
#include "VectorGraphics/BsVectorGraphics.h"

using namespace bs;

HVectorPath GUIBackgroundVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetStateRule& style) const
{
	HVectorPath path = VectorPath::Create(Size2((float)size.Width, (float)size.Height));

	const Rect2 fillArea = Rect2(0.0f, 0.0f, (float)size.Width, (float)size.Height);

	path->DrawRoundedRectangle(fillArea, (float)style.BorderTopLeftRadius, (float)style.BorderTopRightRadius, (float)style. BorderBottomLeftRadius, (float)style.BorderTopRightRadius)
		.ClosePath()
		.SetFillPaint(style.BackgroundColor)
		.DrawFill();

	// TODO - Not supporting separate border styles at the moment. See nvgRoundedRectVarying for implementation. Also ideally support elliptical corners
	const bool drawBorder = style.BorderLeft.Width > 0 && style.BorderLeft.Style != GUIBorderElementStyle::None;
	if(drawBorder)
	{
		path->SetStrokePaint(style.BorderLeft.Color)
			.SetStrokeWidth((float)style.BorderLeft.Width)
			.DrawStroke();
	}

	return path;
}

HVectorPath GUICheckmarkVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetStateRule& style) const
{
	HVectorPath path = VectorPath::Create(Size2(512.0f, 512.0f));

	// TODO: Each GUI element will create its own path, while in most cases only one of these will be needed for the entire UI
	path->SetDrawCursor(Vector2(17.47f, 250.9f))
		.DrawCubicBezierTo(Vector2(88.82f, 328.1f), Vector2(158.0f, 397.6f), Vector2(224.5f, 485.5f))
		.DrawCubicBezierTo(Vector2(296.8f, 341.7f), Vector2(370.8f, 197.4f), Vector2(492.9f, 41.13f))
		.DrawLineTo(Vector2(460.0f, 26.06f))
		.DrawCubicBezierTo(Vector2(356.9f, 135.4f), Vector2(276.8f, 238.9f), Vector2(207.2f, 361.9f))
		.DrawCubicBezierTo(Vector2(158.8f, 318.3f), Vector2(80.58f, 256.6f), Vector2(32.82f, 224.9f))
		.ClosePath()
		.SetFillPaint(style.Color)
		.DrawFill();

	return path;
	
}
