//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVectorGraphics.h"
#include "B3DNVGVectorGraphics.h"
#include "RTTI/B3DVectorGraphicsRTTI.h"
#include "Resources/B3DResources.h"

using namespace b3d;

namespace b3d
{
	constexpr Size2 VectorPath::kDefaultCanvasSize;

	VectorPath::VectorPath(const Size2& canvasSize)
		: Resource(false, "VectorPath"), mCanvasSize(canvasSize)
	{ }

	VectorPath& VectorPath::SetDrawCursor(const Vector2& cursor)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::SetDrawCursor;
		command.SetDrawCursor.Position = cursor;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::SetSolidity(VectorGraphicsPathSolidity solidity)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::SetPathSolidity;
		command.SetPathSolidity.Solidity = solidity;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::ClosePath()
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::ClosePath;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawLineTo(const Vector2& target)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawLineTo;
		command.DrawLineTo.Target = target;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawArcTo(const Vector2& middlePoint, const Vector2& endPoint, float radius)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawArcTo;
		command.DrawArcTo.MiddlePoint = middlePoint;
		command.DrawArcTo.EndPoint = endPoint;
		command.DrawArcTo.Radius = radius;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawQuadraticBezierTo(const Vector2& controlPoint, const Vector2& endPoint)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawQuadraticBezierTo;
		command.DrawQuadraticBezierTo.ControlPoint = controlPoint;
		command.DrawQuadraticBezierTo.EndPoint = endPoint;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawCubicBezierTo(const Vector2& controlPoint1, const Vector2& controlPoint2, const Vector2& endPoint)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawCubicBezierTo;
		command.DrawCubicBezierTo.ControlPoint1 = controlPoint1;
		command.DrawCubicBezierTo.ControlPoint2 = controlPoint2;
		command.DrawCubicBezierTo.EndPoint = endPoint;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawRectangle(const Area2& area)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawRectangle;
		command.DrawRectangle.Area = area;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawRoundedRectangle(const Area2& area, float cornerRadius)
	{
		return DrawRoundedRectangle(area, cornerRadius, cornerRadius, cornerRadius, cornerRadius);
	}

	VectorPath& VectorPath::DrawRoundedRectangle(const Area2& area, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawRoundedRectangle;
	    command.DrawRoundedRectangle.Area = area;
		command.DrawRoundedRectangle.RadiusTopLeft = topLeftCornerRadius;
		command.DrawRoundedRectangle.RadiusTopRight = topRightCornerRadius;
		command.DrawRoundedRectangle.RadiusBottomLeft = bottomLeftCornerRadius;
		command.DrawRoundedRectangle.RadiusBottomRight = bottomRightCornerRadius;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawCircle(const Vector2& origin, float radius)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawEllipse;
		command.DrawEllipse.Origin = origin;
		command.DrawEllipse.Radius = Vector2(radius, radius);

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawEllipse(const Vector2& origin, const Vector2& radius)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawEllipse;
		command.DrawEllipse.Origin = origin;
		command.DrawEllipse.Radius = radius;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawArc(const Vector2& center, float radius, Radian startAngle, Radian endAngle, VectorGraphicsPathWinding direction)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawArc;
		command.DrawArc.Center = center;
		command.DrawArc.Radius = radius;
		command.DrawArc.StartAngle = startAngle;
		command.DrawArc.EndAngle = endAngle;
		command.DrawArc.Direction = direction;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::SetFillPaint(const VectorGraphicsPaint& paint)
	{
		mCurrentState.FillPaint = paint;
		return *this;
	}

	VectorPath& VectorPath::SetStrokePaint(const VectorGraphicsPaint& paint)
	{
		mCurrentState.StrokePaint = paint;
		return *this;
	}

	VectorPath& VectorPath::SetStrokeWidth(float strokeWidth)
	{
		mCurrentState.StrokeWidth = strokeWidth;
		return *this;
	}

	VectorPath& VectorPath::SetMiterLimit(float miterLimit)
	{
		mCurrentState.MiterLimit = miterLimit;
		return *this;
	}

	VectorPath& VectorPath::SetLineCapType(VectorGraphicsLineCapType lineCap)
	{
		mCurrentState.LineCapType = lineCap;
		return *this;
	}

	VectorPath& VectorPath::SetLineJoinType(VectorGraphicsLineJoinStyle lineJoin)
	{
		mCurrentState.LineJoinType = lineJoin;
		return *this;
	}

	VectorPath& VectorPath::SetAlpha(float alpha)
	{
		mCurrentState.Alpha = alpha;
		return *this;
	}

	VectorPath& VectorPath::SetBlendMode(VectorGraphicsBlendMode blendMode)
	{
		mCurrentState.BlendMode = blendMode;
		return *this;
	}

	VectorPath& VectorPath::SetAntialiasShapes(bool antialiasShapes)
	{
		mCurrentState.AntialiasShape = antialiasShapes;
		return *this;
	}

	VectorPath& VectorPath::SetScissorRectangle(const Area2& scissorArea)
	{
		mCurrentState.ScissorArea = scissorArea;
		return *this;
	}

	VectorPath& VectorPath::ClearScissor()
	{
		mCurrentState.ScissorArea = Area2::kEmpty;
		return *this;
	}

	VectorPath& VectorPath::DrawFill()
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::Fill;
		command.Fill.StateIndex = (u32)mCommandStates.size();

		mCommandStates.push_back(mCurrentState);
		mCommands.push_back(command);

		return *this;
	}

	VectorPath& VectorPath::DrawStroke()
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::Stroke;
		command.Stroke.StateIndex = (u32)mCommandStates.size();

		mCommandStates.push_back(mCurrentState);
		mCommands.push_back(command);

		return *this;
	}

	SPtr<render::VectorPathRenderable> VectorPath::CreateRenderable(const VectorGraphicsSettings& settings) const
	{
		return B3DMakeShared<render::NVGVectorPathRenderable>(*this, settings);
	}

	SPtr<VectorPath> VectorPath::CreateShared(const Size2& canvasSize)
	{
		SPtr<VectorPath> newVectorPath = B3DMakeSharedFromExisting<VectorPath>(new(B3DAllocate<VectorPath>()) VectorPath(canvasSize));
		newVectorPath->SetShared(newVectorPath);
		newVectorPath->Initialize();

		return newVectorPath;
	}

	HVectorPath VectorPath::Create(const Size2& canvasSize)
	{
		const SPtr<VectorPath> newVectorPath = CreateShared(canvasSize);

		return B3DStaticResourceCast<VectorPath>(GetResources().CreateResourceHandle(newVectorPath));
	}

	RTTIType* VectorPath::GetRttiStatic()
	{
		return VectorPathRTTI::Instance();
	}

	RTTIType* VectorPath::GetRtti() const
	{
		return GetRttiStatic();
	}

	namespace render
	{
		RTTIType* VectorPathRenderable::GetRttiStatic()
		{
			return VectorPathRenderableRTTI::Instance();
		}

		RTTIType* VectorPathRenderable::GetRtti() const
		{
			return GetRttiStatic();
		}
	} // namespace render

} // namespace b3d
