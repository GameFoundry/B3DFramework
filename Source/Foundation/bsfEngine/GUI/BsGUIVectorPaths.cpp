//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGUIVectorPaths.h"
#include "StyleSheet/BsGUIStyleSheet.h"
#include "VectorGraphics/BsVectorGraphics.h"

using namespace bs;

HVectorPath GUIBackgroundVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const
{
	HVectorPath path = VectorPath::Create(Size2((float)size.Width, (float)size.Height));

	const Rect2 fillArea = Rect2(0.0f, 0.0f, (float)size.Width, (float)size.Height);

	path->DrawRoundedRectangle(fillArea, (float)styleSheetRule.BorderTopLeftRadius, (float)styleSheetRule.BorderTopRightRadius, (float)styleSheetRule. BorderBottomLeftRadius, (float)styleSheetRule.BorderTopRightRadius)
		.ClosePath()
		.SetFillPaint(styleSheetRule.BackgroundColor)
		.DrawFill();

	// TODO - Not supporting separate border styles at the moment. See nvgRoundedRectVarying for implementation. Also ideally support elliptical corners
	const bool drawBorder = styleSheetRule.BorderLeft.Width > 0 && styleSheetRule.BorderLeft.Style != GUIBorderElementStyle::None;
	if(drawBorder)
	{
		path->SetStrokePaint(styleSheetRule.BorderLeft.Color)
			.SetStrokeWidth((float)styleSheetRule.BorderLeft.Width)
			.DrawStroke();
	}

	return path;
}

HVectorPath GUICheckmarkVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const
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
		.SetFillPaint(styleSheetRule.Color)
		.DrawFill();

	return path;
}

HVectorPath GUITabBackgroundVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const
{
	HVectorPath path = VectorPath::Create(Size2(135.0f, 27.0f));

	path->SetDrawCursor(Vector2(42.075512f, 56.356697f))
		.DrawCubicBezierTo(Vector2(34.958691f, 56.362197f), Vector2(26.990934f, 83.272579f), Vector2(20.075512f, 83.356697f))
		.DrawCubicBezierTo(Vector2(13.16009f, 83.440797f), Vector2(163.36359f, 83.446497f), Vector2(155.07551f, 83.356697f))
		.DrawCubicBezierTo(Vector2(146.78742f, 83.266907f), Vector2(140.507f, 56.314452f), Vector2(133.07551f, 56.356697f))
		.DrawCubicBezierTo(Vector2(125.64402f, 56.398947f), Vector2(49.192332f, 56.351197f), Vector2(42.075512f, 56.356697f))
		.ClosePath()
		.SetFillPaint(styleSheetRule.BackgroundColor)
		.DrawFill();

	const bool drawBorder = styleSheetRule.BorderLeft.Width > 0 && styleSheetRule.BorderLeft.Style != GUIBorderElementStyle::None;
	if(drawBorder)
	{
		path->SetStrokePaint(styleSheetRule.BorderLeft.Color)
			.SetStrokeWidth((float)styleSheetRule.BorderLeft.Width)
			.DrawStroke();
	}

	return path;
}

HVectorPath GUIDropDownArrowVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const
{
	HVectorPath path = VectorPath::Create(Size2(100.0f, 100.0f));

	path->SetDrawCursor(Vector2(0.0f, 0.0f))
		.DrawLineTo(Vector2(50.0f, 100.0f))
		.DrawLineTo(Vector2(100.0f, 0.0f))
		.ClosePath()
		.SetFillPaint(styleSheetRule.BackgroundColor)
		.DrawFill();

	const bool drawBorder = styleSheetRule.BorderLeft.Width > 0 && styleSheetRule.BorderLeft.Style != GUIBorderElementStyle::None;
	if(drawBorder)
	{
		path->SetStrokePaint(styleSheetRule.BorderLeft.Color)
			.SetStrokeWidth((float)styleSheetRule.BorderLeft.Width)
			.DrawStroke();
	}

	return path;
}

HVectorPath GUIScrollArrowVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const
{
	HVectorPath path = VectorPath::Create(Size2(100.0f, 75.0f));

	path->SetDrawCursor(Vector2(0.0f, 0.0f))
		.DrawLineTo(Vector2(50.724423f, 75.0f))
		.DrawLineTo(Vector2(100.0f, 0.0f))
		.DrawCubicBezierTo(Vector2(69.114599f, 25.917529f), Vector2(36.035557f, 28.608797f), Vector2(0.0f, 0.0f))
		.ClosePath()
		.SetFillPaint(styleSheetRule.Color)
		.DrawFill();

	const bool drawBorder = styleSheetRule.BorderLeft.Width > 0 && styleSheetRule.BorderLeft.Style != GUIBorderElementStyle::None;
	if(drawBorder)
	{
		path->SetStrokePaint(styleSheetRule.BorderLeft.Color)
			.SetStrokeWidth((float)styleSheetRule.BorderLeft.Width)
			.DrawStroke();
	}

	return path;
}

HVectorPath GUIScrollHandleVectorPathBuilder::BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const
{
	constexpr u32 kReferenceRasterSize = 13; // Reference size of the handle in pixels, both width and height
	constexpr float kReferenceCanvasSize = 100.0f; // Reference size of the vector path canvas

	float canvasUnitsPerPixel = kReferenceCanvasSize / (float)kReferenceRasterSize;

	// Adjust the height so it expands
	const u32 shaftRasterHeight = (u32)Math::Max(0, ((i32)size.Height - (i32)kReferenceRasterSize)); // Height not including the caps
	float shaftCanvasHeight = (float)shaftRasterHeight * canvasUnitsPerPixel;

	const Size2 canvasSize(kReferenceCanvasSize, kReferenceCanvasSize + shaftCanvasHeight);

	HVectorPath path = VectorPath::Create(canvasSize);

	path->SetDrawCursor(Vector2(0.0, 50.0f))
		.DrawCubicBezierTo(Vector2(0.0f, 22.32227f), Vector2(22.605928f, 0.0f), Vector2(50.0f, 0.0f))
		.DrawCubicBezierTo(Vector2(77.677738f, 0.0f), Vector2(100.0f, 22.32225f), Vector2(100.0f, 50.0f))
		.DrawLineTo(Vector2(100.0f, 50.0f + shaftCanvasHeight))
		.DrawCubicBezierTo(Vector2(100.0f, 77.78187f), Vector2(77.677738f, 99.78705f), Vector2(50.0f, 100.0f))
		.DrawCubicBezierTo(Vector2(22.60593f, 100.0f), Vector2(0.0f, 77.78187f), Vector2(0.0f, 50.0f))
		.ClosePath()
		.SetFillPaint(styleSheetRule.Color)
		.DrawFill();

	const bool drawBorder = styleSheetRule.BorderLeft.Width > 0 && styleSheetRule.BorderLeft.Style != GUIBorderElementStyle::None;
	if(drawBorder)
	{
		path->SetStrokePaint(styleSheetRule.BorderLeft.Color)
			.SetStrokeWidth((float)styleSheetRule.BorderLeft.Width)
			.DrawStroke();
	}

	return path;
}
