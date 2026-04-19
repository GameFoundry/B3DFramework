//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGUIVectorPaths.h"
#include "StyleSheet/B3DGUIStyleSheet.h"
#include "VectorGraphics/B3DVectorGraphics.h"

using namespace b3d;

HVectorPath GUIBackgroundVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
{
	HVectorPath path = VectorPath::Create(Size2((float)size.Width, (float)size.Height));

	const bool allBordersEqual =
		styleSheetRule.BorderLeft == styleSheetRule.BorderRight &&
		styleSheetRule.BorderLeft == styleSheetRule.BorderTop &&
		styleSheetRule.BorderLeft == styleSheetRule.BorderBottom;

	const bool drawBorder = ((styleSheetRule.BorderLeft.Width > 0 && styleSheetRule.BorderLeft.Style != GUIBorderElementStyle::None)
		|| (styleSheetRule.BorderRight.Width > 0 && styleSheetRule.BorderRight.Style != GUIBorderElementStyle::None)
		|| (styleSheetRule.BorderTop.Width > 0 && styleSheetRule.BorderTop.Style != GUIBorderElementStyle::None)
		|| (styleSheetRule.BorderBottom.Width > 0 && styleSheetRule.BorderBottom.Style != GUIBorderElementStyle::None));

	const float width = (float)size.Width;
	const float height = (float)size.Height;

	const float minimumExtent = Math::Min(width, height) * 0.5f;

	const float borderTopLeftRadius = Math::Min(minimumExtent, (float)styleSheetRule.BorderTopLeftRadius);
	const float borderTopRightRadius = Math::Min(minimumExtent, (float)styleSheetRule.BorderTopRightRadius);
	const float borderBottomLeftRadius = Math::Min(minimumExtent, (float)styleSheetRule.BorderBottomLeftRadius);
	const float borderBottomRightRadius = Math::Min(minimumExtent, (float)styleSheetRule.BorderBottomRightRadius);

	// If no border, or border with all equal sides, draw border using a stroke
	if(!drawBorder || allBordersEqual)
	{
		// Drawing stroke will extend the width/height by 'strokeWidth', so account for that so our final drawn area matches the requested size. i.e.
		// If user requests a height of 35 pixels, a top & bottom borders of 5 pixels each (strokeWidth = 5), we want the fill rectangle to be 30 pixels high.
		// Note that the total border height in the above example is 10 pixels (5 for top, 5 for bottom), but the other 5 pixels are taken from the fill size, so they won't expand the drawn area.
		const float strokeWidth = drawBorder ? (float)styleSheetRule.BorderLeft.Width : 0.0f;
		const Area2 fillArea = Area2(strokeWidth * 0.5f, strokeWidth * 0.5f, (float)size.Width - strokeWidth, (float)size.Height - strokeWidth);

		path->DrawRoundedRectangle(fillArea, borderTopLeftRadius, borderTopRightRadius, borderBottomLeftRadius, borderBottomRightRadius)
			.ClosePath()
			.SetFillPaint(styleSheetRule.BackgroundColor)
			.DrawFill();

		// Draw simple borders using stroke
		if(drawBorder)
		{
			path->SetStrokePaint(styleSheetRule.BorderLeft.Color)
				.SetStrokeWidth(strokeWidth)
				.DrawStroke();
		}
	}
	// If not all border sides are equal then we can't use stroke for drawing it, need to draw it manually
	else
	{
		const float x = 0.0f;
		const float y = 0.0f;

		const float leftBorderWidth = styleSheetRule.BorderLeft.Style != GUIBorderElementStyle::None ? (float)styleSheetRule.BorderLeft.Width : 0.0f;
		const float rightBorderWidth = styleSheetRule.BorderRight.Style != GUIBorderElementStyle::None ? (float)styleSheetRule.BorderRight.Width : 0.0f;
		const float topBorderWidth = styleSheetRule.BorderTop.Style != GUIBorderElementStyle::None ? (float)styleSheetRule.BorderTop.Width : 0.0f;
		const float bottomBorderWidth = styleSheetRule.BorderBottom.Style != GUIBorderElementStyle::None ? (float)styleSheetRule.BorderBottom.Width : 0.0f;

		// Inner border is the edge of the center rectangle
		const float innerX = x + leftBorderWidth;
		const float innerY = y + topBorderWidth;

		const float innerWidth = Math::Max(0.0f, width - leftBorderWidth - rightBorderWidth);
		const float innerHeight = Math::Max(0.0f, height - topBorderWidth - bottomBorderWidth);

		enum BorderCorner
		{
			BC_TopRight,
			BC_TopLeft,
			BC_BottomLeft,
			BC_BottomRight,
		};

		enum BorderSide
		{
			BS_Top,
			BS_Left,
			BS_Bottom,
			BS_Right,
		};

		constexpr BorderCorner kCornersPerSide[4][2]{
			{ BC_TopRight, BC_TopLeft },
			{ BC_TopLeft, BC_BottomLeft },
			{ BC_BottomLeft, BC_BottomRight },
			{ BC_BottomRight, BC_TopRight },
		};

		float cornerRadii[4];
		cornerRadii[BC_TopRight] = borderTopRightRadius;
		cornerRadii[BC_TopLeft] = borderTopLeftRadius;
		cornerRadii[BC_BottomLeft] = borderBottomLeftRadius;
		cornerRadii[BC_BottomRight] = borderBottomRightRadius;

		GUIStyleSheetBorderElement borderStylePerSide[4];
		borderStylePerSide[BS_Top] = styleSheetRule.BorderTop;
		borderStylePerSide[BS_Left] = styleSheetRule.BorderLeft;
		borderStylePerSide[BS_Bottom] = styleSheetRule.BorderBottom;
		borderStylePerSide[BS_Right] = styleSheetRule.BorderRight;

		// Generates centers we can use for drawing the corner arcs
		auto fnGenerateCornerCenters = [&cornerRadii](float x, float y, float width, float height) {
			const float halfWidth = Math::Abs(width) * 0.5f;
			const float halfHeight = Math::Abs(height) * 0.5f;

			const float right = x + width;
			const float bottom = y + height;

			Vector2 borderCornerOffset[4];
			for(u32 cornerIndex = 0; cornerIndex < 4; ++cornerIndex)
			{
				// TODO - Known issue if the radius is larger than the half height of the inner border, border will not match up with the center rectangle
				borderCornerOffset[cornerIndex] = Vector2(
					Math::Min(cornerRadii[cornerIndex], halfWidth) * Math::Sign(width),
					Math::Min(cornerRadii[cornerIndex], halfHeight) * Math::Sign(height));
			}

			Array<Vector2, 4> cornerCenters;
			cornerCenters[BC_TopRight] = Vector2(right - borderCornerOffset[BC_TopRight].X, y + borderCornerOffset[BC_TopRight].Y);
			cornerCenters[BC_TopLeft] = Vector2(x + borderCornerOffset[BC_TopLeft].X, y + borderCornerOffset[BC_TopLeft].Y);
			cornerCenters[BC_BottomLeft] = Vector2(x + borderCornerOffset[BC_BottomLeft].X, bottom - borderCornerOffset[BC_BottomLeft].Y);
			cornerCenters[BC_BottomRight] = Vector2(right - borderCornerOffset[BC_BottomRight].X, bottom - borderCornerOffset[BC_BottomRight].Y);

			return cornerCenters;
		};

		// Generate centers which form the centers of circles used for the corner arcs
		const Array<Vector2, 4> outerBorderCornerCenters = fnGenerateCornerCenters(x, y, width, height);
		const Array<Vector2, 4> innerBorderCornerCenters = fnGenerateCornerCenters(innerX, innerY, innerWidth, innerHeight);

		// Draw borders separately for each side. Each border is formed by an outer edge and an inner edge, connecting to form the shape we'll fill to draw the border.
		// Inner edge is inset by the border width. Both edges are composed of a 45 degree arc, followed by a straight line, and another 45 degree arc. 
		Degree currentAngle(315.0f);
		const Degree kAngle45(45.0f);
		for(u32 side = 0; side < 4; ++side)
		{
			const bool isVisible = borderStylePerSide[side].Style != GUIBorderElementStyle::None && borderStylePerSide[side].Width > 0;
			if(!isVisible)
				continue;

			const u32 sideCornerA = kCornersPerSide[side][0];
			const u32 sideCornerB = kCornersPerSide[side][1];

			// Outer edge of the border
			path->DrawArc(
				outerBorderCornerCenters[sideCornerA],
				cornerRadii[sideCornerA],
				currentAngle,
				currentAngle - kAngle45, VectorGraphicsPathWinding::Counterclockwise);
			// Line connecting the arcs is done implicitly by the DrawArc call if the new arc's starting coordinate doesn't match previous end coordinate.
			path->DrawArc(
				outerBorderCornerCenters[sideCornerB],
				cornerRadii[sideCornerB],
				currentAngle - kAngle45,
				currentAngle - kAngle45 * 2.0f, VectorGraphicsPathWinding::Counterclockwise);

			// Inner edge of the border (matches the center rectangle)
			path->DrawArc(
				innerBorderCornerCenters[sideCornerB],
				cornerRadii[sideCornerB],
				currentAngle - kAngle45 * 2.0,
				currentAngle - kAngle45, VectorGraphicsPathWinding::Clockwise);
			// Line connecting the arcs is done implicitly by the DrawArc call if the new arc's starting coordinate doesn't match previous end coordinate.
			path->DrawArc(
				innerBorderCornerCenters[sideCornerA],
				cornerRadii[sideCornerA],
				currentAngle - kAngle45,
				currentAngle, VectorGraphicsPathWinding::Clockwise);

			path->ClosePath();
			path->SetFillPaint(borderStylePerSide[side].Color);
			path->DrawFill();

			currentAngle -= kAngle45 * 2.0f;
		}

		// Center rectangle
		path->DrawRoundedRectangle(Area2(innerX, innerY, innerWidth, innerHeight), (float)styleSheetRule.BorderTopLeftRadius, (float)styleSheetRule.BorderTopRightRadius, (float)styleSheetRule.BorderBottomLeftRadius, (float)styleSheetRule.BorderBottomRightRadius);
		path->SetFillPaint(styleSheetRule.BackgroundColor);
		path->DrawFill();
	}

	return path;
}

HVectorPath GUICheckmarkVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
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

HVectorPath GUITabBackgroundVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
{
	HVectorPath path = VectorPath::Create(Size2(135.0f, 27.0f));

	path->SetDrawCursor(Vector2(22.231762f, 4.4818461e-4f))
		.DrawCubicBezierTo(Vector2(15.114941f, 0.00594818f), Vector2(7.1471837f, 26.91633f), Vector2(0.23176174f, 27.000448f))
		.DrawCubicBezierTo(Vector2(-6.6836603f, 27.084548f), Vector2(143.51984f, 27.090248f), Vector2(135.23176f, 27.000448f))
		.DrawCubicBezierTo(Vector2(126.94367f, 26.910658f), Vector2(120.66325f, -0.04179682f), Vector2(113.23176f, 4.4818461e-4f))
		.DrawCubicBezierTo(Vector2(105.80027f, 0.04269818f), Vector2(29.348582f, -0.00505182f), Vector2(22.231762f, 4.4818461e-4f))
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

HVectorPath GUIDropDownArrowVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
{
	constexpr float kCanvasSize = 100.0f;
	constexpr float kArrowSize = kCanvasSize * 0.75f;
	HVectorPath path = VectorPath::Create(Size2(kCanvasSize, kCanvasSize));

	path->DrawRectangle(Area2(0.0f, 0.0f, kCanvasSize, kCanvasSize))
		.ClosePath()
		.SetFillPaint(styleSheetRule.BackgroundColor)
		.DrawFill();

	path->SetDrawCursor(Vector2(kCanvasSize - kArrowSize, kCanvasSize - kArrowSize))
		.DrawLineTo(Vector2(kCanvasSize * 0.5f, kArrowSize))
		.DrawLineTo(Vector2(kArrowSize, kCanvasSize - kArrowSize))
		.ClosePath()
		.SetFillPaint(styleSheetRule.Color)
		.DrawFill();

	return path;
}

HVectorPath GUIScrollArrowVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
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

HVectorPath GUIScrollHandleVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
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

HVectorPath GUIResizableVerticalScrollHandleVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
{
	const Size2 constrainedSize(
		Math::Max(1.0f, (float)size.Width),
		Math::Max(kResizableHandlePadding * 2.0f + kResizableHandleSize * 3.0f, (float)size.Height));

	HVectorPath path = VectorPath::Create(constrainedSize);

	path->DrawRectangle(Area2(0.0f, 0.0f, constrainedSize.Width, kResizableHandleSize))
		.DrawRectangle(Area2(0.0f, kResizableHandleSize + kResizableHandlePadding, constrainedSize.Width, (float)size.Height - kResizableHandleSize * 2.0f - kResizableHandlePadding * 2.0f))
		.DrawRectangle(Area2(0.0f, (float)size.Height - kResizableHandleSize, constrainedSize.Width, kResizableHandleSize))
		.ClosePath()
		.SetFillPaint(styleSheetRule.BackgroundColor)
		.DrawFill();

	return path;
}

HVectorPath GUIResizableHorizontalScrollHandleVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
{
	const Size2 constrainedSize(
		Math::Max(kResizableHandlePadding * 2.0f + kResizableHandleSize * 3.0f, (float)size.Width),
		Math::Max(1.0f, (float)size.Height));

	HVectorPath path = VectorPath::Create(constrainedSize);

	path->DrawRectangle(Area2(0.0f, 0.0f, kResizableHandleSize, constrainedSize.Height))
		.DrawRectangle(Area2(kResizableHandleSize + kResizableHandlePadding, 0.0f, (float)size.Width - kResizableHandleSize * 2.0f - kResizableHandlePadding * 2.0f, constrainedSize.Height))
		.DrawRectangle(Area2((float)size.Width - kResizableHandleSize, 0.0f, kResizableHandleSize, constrainedSize.Height))
		.ClosePath()
		.SetFillPaint(styleSheetRule.BackgroundColor)
		.DrawFill();

	return path;
}

HVectorPath GUISeparatorVectorPathBuilder::BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const
{
	HVectorPath path = VectorPath::Create(Size2((float)size.Width, (float)size.Height));

	if(size.Height == 1)
	{
		path->SetDrawCursor(Vector2(0.0f, 0.0f))
			.DrawLineTo(Vector2((float)size.Width, 0.0f))
			.SetStrokePaint(styleSheetRule.BackgroundColor)
			.SetStrokeWidth(1.0f)
			.DrawFill();
	}
	else if(size.Height > 1)
	{
		path->DrawRectangle(Area2(0.0f, 0.0f, (float)size.Width, (float)size.Height))
			.SetFillPaint(styleSheetRule.BackgroundColor)
			.DrawFill();
	}

	return path;
}
