//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "Utility/BsRectOffset.h"
#include "Math/BsMatrix4.h"
#include "Math/BsRect2.h"

namespace bs
{
	class VectorPathRenderableRTTI;

	namespace ct
	{
		class VectorPathRenderable;
	}

	/** @addtogroup VectorGraphics
	 *  @{
	 */

	// TODO - Doc
	struct VectorGraphicsSettings
	{
		Size2 Size = Size2::kZero;
		Vector2 Scale = Vector2::kOne;
		Matrix4 Transform = Matrix4::kIdentity;
		RectOffset Scale9GridBorder;
		bool UseAntialiasing = true;
		bool StencilStrokes = true;
		float DevicePixelRatio = 1.0f;
	};

	enum class VectorGraphicsPaintType
	{
		Solid,
		LinearGradient,
		BoxGradient,
		RadialGradient,
	};

	enum class VectorGraphicsBlendMode
	{
		SourceOver,
		SourceIn,
		SourceOut,
		Atop,
		DestinationOver,
		DestinationIn,
		DestinationOut,
		DestinationAtop,
		Lighter,
		Copy,
		Xor
	};

	enum class VectorGraphicsPathWinding
	{
		Counterclockwise,
		Clockwise
	};

	enum class VectorGraphicsLineCapType
	{
		Butt,
		Round,
		Square
	};

	enum class VectorGraphicsLineJoinStyle
	{
		Miter,
		Round,
		Bevel
	};

	struct VectorGraphicsPaint
	{
	public:
		struct SolidPaint
		{
			Color Color;
		};

		struct LinearGradientPaint
		{
			Color StartColor;
			Color EndColor;
			Vector2 StartPoint;
			Vector2 EndPoint;
		};

		struct BoxGradientPaint
		{
			Color InnerColor;
			Color OuterColor;
			Rect2 Area;
			float CornerRadius;
			float Feather;
		};

		struct RadialGradientPaint
		{
			Color InnerColor;
			Color OuterColor;
			Vector2 Center;
			float InnerRadius;
			float OuterRadius;
		};

		VectorGraphicsPaint()
		{
			Type = VectorGraphicsPaintType::Solid;
			BoxGradient.InnerColor = Color::kBlack;
			BoxGradient.OuterColor = Color::kBlack;
			BoxGradient.Area = Rect2::kEmpty;
			BoxGradient.CornerRadius = 0.0f;
			BoxGradient.Feather = 0.0f;
		}

		VectorGraphicsPaint(const Color& color)
		{
			Type = VectorGraphicsPaintType::Solid;
			Solid.Color = color;
		}

		VectorGraphicsPaintType GetType() const { return Type; }
		const SolidPaint& GetSolidPaint() const;
		const LinearGradientPaint& GetLinearGradientPaint() const;
		const BoxGradientPaint& GetBoxGradientPaint() const;
		const RadialGradientPaint& GetRadialGradientPaint() const;

		static VectorGraphicsPaint CreateSolid(const Color& color);
		static VectorGraphicsPaint CreateLinearGradient(const Color& startColor, const Color& endColor, const Vector2& startPoint, const Vector2& endPoint);
		static VectorGraphicsPaint CreateBoxGradient(const Color& innerColor, const Color& outerColor, const Rect2& area, float cornerRadius, float feather);
		static VectorGraphicsPaint CreateRadialGradient(const Color& innerColor, const Color& outerColor, const Vector2& center, float innerRadius, float outerRadius);

	private:
		friend struct RTTIPlainType<VectorGraphicsPaint>;

		VectorGraphicsPaintType Type = VectorGraphicsPaintType::Solid;
		union
		{
			SolidPaint Solid;
			LinearGradientPaint LinearGradient;
			BoxGradientPaint BoxGradient;
			RadialGradientPaint RadialGradient;
		};
	};

	inline const VectorGraphicsPaint::SolidPaint& VectorGraphicsPaint::GetSolidPaint() const
	{
		B3D_ENSURE(Type == VectorGraphicsPaintType::Solid);
		return Solid;
	}

	inline const VectorGraphicsPaint::LinearGradientPaint& VectorGraphicsPaint::GetLinearGradientPaint() const
	{
		B3D_ENSURE(Type == VectorGraphicsPaintType::LinearGradient);
		return LinearGradient;
	}

	inline const VectorGraphicsPaint::BoxGradientPaint& VectorGraphicsPaint::GetBoxGradientPaint() const
	{
		B3D_ENSURE(Type == VectorGraphicsPaintType::BoxGradient);
		return BoxGradient;
	}

	inline const VectorGraphicsPaint::RadialGradientPaint& VectorGraphicsPaint::GetRadialGradientPaint() const
	{
		B3D_ENSURE(Type == VectorGraphicsPaintType::RadialGradient);
		return RadialGradient;
	}

	inline VectorGraphicsPaint VectorGraphicsPaint::CreateSolid(const Color& color)
	{
		VectorGraphicsPaint paint;
		paint.Type = VectorGraphicsPaintType::Solid;
		paint.Solid.Color = color;

		return paint;
	}

	inline VectorGraphicsPaint VectorGraphicsPaint::CreateLinearGradient(const Color& startColor, const Color& endColor, const Vector2& startPoint, const Vector2& endPoint)
	{
		VectorGraphicsPaint paint;
		paint.Type = VectorGraphicsPaintType::LinearGradient;
		paint.LinearGradient.StartColor = startColor;
		paint.LinearGradient.EndColor = endColor;
		paint.LinearGradient.StartPoint = startPoint;
		paint.LinearGradient.EndPoint = endPoint;

		return paint;
	}

	inline VectorGraphicsPaint VectorGraphicsPaint::CreateBoxGradient(const Color& innerColor, const Color& outerColor, const Rect2& area, float cornerRadius, float feather)
	{
		VectorGraphicsPaint paint;
		paint.Type = VectorGraphicsPaintType::BoxGradient;
		paint.BoxGradient.InnerColor = innerColor;
		paint.BoxGradient.OuterColor = outerColor;
		paint.BoxGradient.Area = area;
		paint.BoxGradient.CornerRadius = cornerRadius;
		paint.BoxGradient.Feather = feather;

		return paint;
	}

	inline VectorGraphicsPaint VectorGraphicsPaint::CreateRadialGradient(const Color& innerColor, const Color& outerColor, const Vector2& center, float innerRadius, float outerRadius)
	{
		VectorGraphicsPaint paint;
		paint.Type = VectorGraphicsPaintType::RadialGradient;
		paint.RadialGradient.InnerColor = innerColor;
		paint.RadialGradient.OuterColor = outerColor;
		paint.RadialGradient.Center = center;
		paint.RadialGradient.InnerRadius = innerRadius;
		paint.RadialGradient.OuterRadius = outerRadius;

		return paint;
	}

	enum class VectorPathCommandType
	{
		Unknown,

		Fill,
		Stroke,

		SetDrawCursor,
		ClosePath,
		SetPathWinding,

		DrawLineTo,
		DrawArcTo,
		DrawCubicBezierTo,
		DrawQuadraticBezierTo,

		DrawRectangle,
		DrawRoundedRectangle,
		DrawEllipse,
		DrawArc,
	};

	struct VectorPathState
	{
		VectorGraphicsPaint StrokePaint = Color::kBlack;
		VectorGraphicsPaint FillPaint = Color::kWhite;
		float MiterLimit = 10.0f;
		float StrokeWidth = 1.0f;
		VectorGraphicsLineCapType LineCapType = VectorGraphicsLineCapType::Butt;
		VectorGraphicsLineJoinStyle LineJoinType = VectorGraphicsLineJoinStyle::Miter;
		bool AntialiasShape = true;
		float Alpha = 1.0f;
		VectorGraphicsBlendMode BlendMode = VectorGraphicsBlendMode::SourceOver;
		Rect2 ScissorArea = Rect2::kEmpty;
	};

	struct VectorPathCommand
	{
		VectorPathCommand()
		{
			
		}

		struct FillCommand
		{
			u32 StateIndex;
		};

		struct StrokeCommand
		{
			u32 StateIndex;
		};

		struct SetDrawCursorCommand
		{
			Vector2 Position;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				cursor = Position;
				return Rect2::kEmpty;
			}
		};

		struct SetPathWindingCommand
		{
			VectorGraphicsPathWinding Winding = VectorGraphicsPathWinding::Counterclockwise;
		};

		struct DrawLineToCommand
		{
			Vector2 Target;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				const Vector2 minimum = Vector2::Min(cursor, Target);
				const Vector2 maximum = Vector2::Max(cursor, Target);
				const Vector2 size = maximum - minimum;

				cursor = Target;
				return Rect2(minimum.X, minimum.Y, size.X, size.Y);
			}
		};

		struct DrawArcToCommand
		{
			Vector2 MiddlePoint;
			Vector2 EndPoint;
			float Radius;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				const Vector2 minimum = Vector2::Min(cursor, Vector2::Min(MiddlePoint, EndPoint));
				const Vector2 maximum = Vector2::Max(cursor, Vector2::Max(MiddlePoint, EndPoint));
				const Vector2 size = maximum - minimum;

				cursor = EndPoint;
				return Rect2(minimum.X, minimum.Y, size.X, size.Y);

			}
		};

		struct DrawCubicBezierToCommand
		{
			Vector2 ControlPoint1;
			Vector2 ControlPoint2;
			Vector2 EndPoint;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				// TODO - Calculate minimum bezier curve bounds with a more algebraic approach
				const Vector2 minimum = Vector2::Min(cursor, Vector2::Min(ControlPoint1, Vector2::Min(ControlPoint2, EndPoint)));
				const Vector2 maximum = Vector2::Max(cursor, Vector2::Max(ControlPoint1, Vector2::Max(ControlPoint2, EndPoint)));
				const Vector2 size = maximum - minimum;

				cursor = EndPoint;
				return Rect2(minimum.X, minimum.Y, size.X, size.Y);

			}
		};

		struct DrawQuadraticBezierToCommand
		{
			Vector2 ControlPoint;
			Vector2 EndPoint;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				// TODO - Calculate minimum bezier curve bounds with a more algebraic approach
				// See https://github.com/Pomax/bezierjs/blob/master/src/bezier.js
				const Vector2 minimum = Vector2::Min(cursor, Vector2::Min(ControlPoint, EndPoint));
				const Vector2 maximum = Vector2::Max(cursor, Vector2::Max(ControlPoint, EndPoint));
				const Vector2 size = maximum - minimum;

				cursor = EndPoint;
				return Rect2(minimum.X, minimum.Y, size.X, size.Y);

			}
		};

		struct DrawRectangleCommand
		{
			Rect2 Area;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				return Area;
			}
		};

		struct DrawRoundedRectangleCommand
		{
			Rect2 Area;
			float RadiusTopLeft;
			float RadiusTopRight;
			float RadiusBottomLeft;
			float RadiusBottomRight;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				return Area;
			}
		};

		struct DrawEllipseCommand
		{
			Vector2 Origin;
			Vector2 Radius;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				return Rect2(Origin.X - Radius.X, Origin.Y - Radius.Y, 2.0f * Radius.X, 2.0f * Radius.Y);
			}
		};

		struct DrawArcCommand
		{
			Vector2 Origin;
			float Radius;
			Radian StartAngle;
			Radian EndAngle;
			VectorGraphicsPathWinding Direction;

			Rect2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				// TODO - Calculate minimum arc bounds with a more algebraic approach
				return Rect2(Origin.X, Origin.Y, Radius, Radius);
			}
		};

		VectorPathCommandType Type = VectorPathCommandType::Unknown;
		union
		{
			FillCommand Fill;
			StrokeCommand Stroke;

			SetDrawCursorCommand SetDrawCursor;
			SetPathWindingCommand SetPathWinding;

			DrawLineToCommand DrawLineTo;
			DrawArcToCommand DrawArcTo;
			DrawQuadraticBezierToCommand DrawQuadraticBezierTo;
			DrawCubicBezierToCommand DrawCubicBezierTo;

			DrawRectangleCommand DrawRectangle;
			DrawRoundedRectangleCommand DrawRoundedRectangle;
			DrawArcCommand DrawArc;
			DrawEllipseCommand DrawEllipse;
		};
	};

	// TODO
	class VectorPath : public IReflectable
	{
	public:
		VectorPath() = default;

		VectorPath& SetDrawCursor(const Vector2& cursor);
		VectorPath& SetWinding(VectorGraphicsPathWinding winding);
		VectorPath& ClosePath();

		VectorPath& DrawLineTo(const Vector2& target);
		VectorPath& DrawArcTo(const Vector2& middlePoint, const Vector2& endPoint, float radius);
		VectorPath& DrawQuadraticBezierTo(const Vector2& controlPoint, const Vector2& endPoint);
		VectorPath& DrawCubicBezierTo(const Vector2& controlPoint1, const Vector2& controlPoint2, const Vector2& endPoint);

		VectorPath& DrawRectangle(const Rect2& area);
		VectorPath& DrawRoundedRectangle(const Rect2& area, float cornerRadius);
		VectorPath& DrawRoundedRectangle(const Rect2& area, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius);
		VectorPath& DrawCircle(const Vector2& origin, float radius);
		VectorPath& DrawEllipse(const Vector2& origin, const Vector2& radius);
		VectorPath& DrawArc(const Vector2& origin, float radius, Radian startAngle, Radian endAngle, VectorGraphicsPathWinding direction);

		VectorPath& SetFillPaint(const VectorGraphicsPaint& paint);
		VectorPath& SetStrokePaint(const VectorGraphicsPaint& paint);
		VectorPath& SetStrokeWidth(float strokeWidth);
		VectorPath& SetMiterLimit(float miterLimit);
		VectorPath& SetLineCapType(VectorGraphicsLineCapType lineCap);
		VectorPath& SetLineJoinType(VectorGraphicsLineJoinStyle lineJoin);
		VectorPath& SetAlpha(float alpha);
		VectorPath& SetBlendMode(VectorGraphicsBlendMode blendMode);
		VectorPath& SetAntialiasShapes(bool antialiasShapes);

		VectorPath& SetScissorRectangle(const Rect2& scissorArea);
		VectorPath& ClearScissor();

		VectorPath& DrawFill();
		VectorPath& DrawStroke();

		const Vector<VectorPathCommand>& GetCommands() const { return mCommands; }
		const Vector<VectorPathState>& GetCommandStates() const { return mCommandStates; }

		SPtr<ct::VectorPathRenderable> CreateRenderable(VectorGraphicsSettings& settings);

	private:
		VectorPathState mCurrentState;
		Vector<VectorPathCommand> mCommands;
		Vector<VectorPathState> mCommandStates;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class VectorPathRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	namespace ct
	{
		class VectorPathRenderable : public IReflectable
		{
		public:
			VectorPathRenderable(const VectorPath& vectorPath, const VectorGraphicsSettings& settings): mSettings(settings) { }
			~VectorPathRenderable() override = default;

			virtual void Render(GpuCommandBuffer& commandBuffer) = 0;
		protected:
			VectorGraphicsSettings mSettings;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			VectorPathRenderable() = default; // Deserialization only

			friend class bs::VectorPathRenderableRTTI;
			static RTTITypeBase* GetRttiStatic();
			RTTITypeBase* GetRtti() const override;
		};
	} // namespace ct

	/** @} */

} // namespace bs
