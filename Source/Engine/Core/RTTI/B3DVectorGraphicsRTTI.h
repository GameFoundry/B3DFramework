//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DColorRTTI.h"
#include "RTTI/B3DRectOffsetRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DResourceRTTI.h"
#include "VectorGraphics/B3DVectorGraphics.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template <>
	struct RTTIPlainType<VectorGraphicsPaint> : RTTIPlainTypeHelper<VectorGraphicsPaint, TID_VectorGraphicsPaint, 0>
	{
		template<class Processor>
		static void RTTIEnumerateFields(VectorGraphicsPaint& object, Processor& processor, u8 version)
		{
			processor(object.Type);
			
			switch(object.Type)
			{
			case VectorGraphicsPaintType::Solid:
				processor(object.Solid.Color);
				break;
			case VectorGraphicsPaintType::LinearGradient:
				processor(object.LinearGradient.StartColor);
				processor(object.LinearGradient.EndColor);
				processor(object.LinearGradient.StartPoint);
				processor(object.LinearGradient.EndPoint);
				break;
			case VectorGraphicsPaintType::BoxGradient:
				processor(object.BoxGradient.InnerColor);
				processor(object.BoxGradient.OuterColor);
				processor(object.BoxGradient.Area);
				processor(object.BoxGradient.CornerRadius);
				processor(object.BoxGradient.Feather);
				break;
			case VectorGraphicsPaintType::RadialGradient:
				processor(object.RadialGradient.InnerColor);
				processor(object.RadialGradient.OuterColor);
				processor(object.RadialGradient.Center);
				processor(object.RadialGradient.InnerRadius);
				processor(object.RadialGradient.OuterRadius);
				break;
			}
		}
	};

	template <>
	struct RTTIPlainType<VectorPathCommand> : RTTIPlainTypeHelper<VectorPathCommand, TID_VectorPathCommand, 0>
	{
		template<class Processor>
		static void RTTIEnumerateFields(VectorPathCommand& object, Processor& processor, u8 version)
		{
			processor(object.Type);
			
			switch(object.Type)
			{
			case VectorPathCommandType::Fill:
				processor(object.Fill.StateIndex);
				break;
			case VectorPathCommandType::Stroke:
				processor(object.Stroke.StateIndex);
				break;
			case VectorPathCommandType::SetDrawCursor:
				processor(object.SetDrawCursor.Position);
				break;
			case VectorPathCommandType::SetPathSolidity:
				processor(object.SetPathSolidity.Solidity);
				break;
			case VectorPathCommandType::DrawLineTo:
				processor(object.DrawLineTo.Target);
				break;
			case VectorPathCommandType::DrawArcTo:
				processor(object.DrawArcTo.MiddlePoint);
				processor(object.DrawArcTo.EndPoint);
				processor(object.DrawArcTo.Radius);
				break;
			case VectorPathCommandType::DrawCubicBezierTo:
				processor(object.DrawCubicBezierTo.ControlPoint1);
				processor(object.DrawCubicBezierTo.ControlPoint1);
				processor(object.DrawCubicBezierTo.EndPoint);
				break;
			case VectorPathCommandType::DrawQuadraticBezierTo:
				processor(object.DrawQuadraticBezierTo.ControlPoint);
				processor(object.DrawQuadraticBezierTo.EndPoint);
				break;
			case VectorPathCommandType::DrawRectangle:
				processor(object.DrawRectangle.Area);
				break;
			case VectorPathCommandType::DrawRoundedRectangle:
				processor(object.DrawRoundedRectangle.Area);
				processor(object.DrawRoundedRectangle.RadiusTopLeft);
				processor(object.DrawRoundedRectangle.RadiusTopRight);
				processor(object.DrawRoundedRectangle.RadiusBottomLeft);
				processor(object.DrawRoundedRectangle.RadiusBottomRight);
				break;
			case VectorPathCommandType::DrawEllipse:
				processor(object.DrawEllipse.Origin);
				processor(object.DrawEllipse.Radius);
				break;
			case VectorPathCommandType::DrawArc:
				processor(object.DrawArc.Center);
				processor(object.DrawArc.Radius);
				processor(object.DrawArc.StartAngle);
				processor(object.DrawArc.EndAngle);
				processor(object.DrawArc.Direction);
				break;
			}
		}
	};

	template <>
	struct RTTIPlainType<VectorPathState> : RTTIPlainTypeHelper<VectorPathState, TID_VectorPathState, 0>
	{
		template<class Processor>
		static void RTTIEnumerateFields(VectorPathState& object, Processor& processor, u8 version)
		{
			processor(object.StrokePaint);
			processor(object.FillPaint);
			processor(object.MiterLimit);
			processor(object.StrokeWidth);
			processor(object.LineCapType);
			processor(object.LineJoinType);
			processor(object.AntialiasShape);
			processor(object.Alpha);
			processor(object.BlendMode);
			processor(object.ScissorArea);
		}
	};

	template<>
	struct RTTIPlainType<VectorGraphicsSettings> : RTTIPlainTypeHelper<VectorGraphicsSettings, TID_NVGRenderCommand, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(VectorGraphicsSettings& object, Processor& processor, u8 version)
		{
			processor(object.Size);
			processor(object.ScalingMode);
			processor(object.Transform);
			processor(object.UseAntialiasing);
			processor(object.StencilStrokes);
			processor(object.DevicePixelRatio);
		}
	};

	class B3D_EXPORT VectorPathRTTI : public TRTTIType<VectorPath, Resource, VectorPathRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mCurrentState, 0)
			B3D_RTTI_MEMBER(mCommands, 1)
			B3D_RTTI_MEMBER(mCommandStates, 2)
			B3D_RTTI_MEMBER(mCanvasSize, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "VectorPath";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_VectorPath;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<VectorPath>();
		}
	};

	class B3D_EXPORT VectorPathRenderableRTTI : public TRTTIType<render::VectorPathRenderable, IReflectable, VectorPathRenderableRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mSettings, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "VectorPathRenderable";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_VectorPathRenderable;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_ENSURE_LOG(false, "Attempting to construct an abstract type from RTTI.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
