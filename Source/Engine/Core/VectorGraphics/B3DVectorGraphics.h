//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "Utility/B3DRectOffset.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DArea2.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	class VectorPathRenderableRTTI;

	namespace render
	{
		class VectorPathRenderable;
	}

	/** @addtogroup VectorGraphics
	 *  @{
	 */

	/** Determines how to scale path canvas when rasterizing for a particular size. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(VectorGraphics)) VectorGraphicsRasterizationScaling
	{
		StretchToFit, /**< Canvas will stretch non-uniformly in both dimensions in order to cover the raster area fully. */
		ScaleToFit, /**< Canvas will scale uniformly until one dimension is aligned with the raster area. Remaining dimension might have empty space, and canvas will be placed in the center of the raster dimension. */
		CropToFit, /**< Canvas will scale uniformly until both dimensions are larger or aligned with the raster area. Remaining dimension might be cropped. */
		None, /**< Do not perform any scaling. Canvas size is ignored and path coordinates are mapped 1:1 to raster coordinates. */
	};

	/** Settings that control how is a VectorPath drawn. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(VectorGraphics)) VectorGraphicsSettings : public IScriptExportable
	{
		Size2 Size = Size2::kZero; /**< Size of the area in which the path is being rasterized to, in pixels. Canvas size will be mapped to this size according to @p ScalingMode. */
		VectorGraphicsRasterizationScaling ScalingMode = VectorGraphicsRasterizationScaling::ScaleToFit; /**< Determines how is canvas size mapped to @p Size. */
		Matrix4 Transform = Matrix4::kIdentity; /**< Optional transform to apply to the path. */
		bool UseAntialiasing = true; /**< If true, path will be rasterized using higher quality rendering. */
		bool StencilStrokes = true; /**< If true, strokes will be rasterized using higher quality rendering. */
		float DevicePixelRatio = 1.0f; /**< Adjusts vector rendering to better match high DPI rendering. */

		bool operator==(const VectorGraphicsSettings& rhs) const
		{
			return Size == rhs.Size && ScalingMode == rhs.ScalingMode && Transform == rhs.Transform && UseAntialiasing == rhs.UseAntialiasing && StencilStrokes == rhs.StencilStrokes && DevicePixelRatio == rhs.DevicePixelRatio;
		}
	};

	/** Type of paints supported by VectorPath. */
	enum class VectorGraphicsPaintType
	{
		Solid, /**< Single color. */
		LinearGradient, /**< Blend between two colors in a line (one color for each point of the line). */
		BoxGradient, /**< One color in box center, and another for box edges. */
		RadialGradient, /**< One color in circle center, and another near circle radius. */
	};

	/** Determines how are vector paths internally blended with each other. */
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

	/** Determines in what order are the path indices sent to the renderer. */
	enum class VectorGraphicsPathWinding
	{
		Counterclockwise,
		Clockwise
	};

	/** Determines if the path we are drawing represents a solid path (default) or a hole in an existing path. */
	enum class VectorGraphicsPathSolidity
	{
		Solid,
		Hole
	};

	/** Determines how is the start/end of a line drawn. */
	enum class VectorGraphicsLineCapType
	{
		Butt, /**< Flat end cap. */
		Round, /**< Round end cap. */
		Square /**< Square end cap. */
	};

	/** Determines how are sharp corners drawn. */
	enum class VectorGraphicsLineJoinStyle
	{
		Miter, /**< Sharp corner respecting the set miter limit. */
		Round, /**< Rounded corner. */
		Bevel /**< Beveled corner. */
	};

	/** Represents a solid color or a gradient to paint vector path fill or stroke. */
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
			Area2 Area;
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
			BoxGradient.Area = Area2::kEmpty;
			BoxGradient.CornerRadius = 0.0f;
			BoxGradient.Feather = 0.0f;
		}

		VectorGraphicsPaint(const Color& color)
		{
			Type = VectorGraphicsPaintType::Solid;
			Solid.Color = color;
		}

		/** Returns the type of paint stored by this object. */
		VectorGraphicsPaintType GetType() const { return Type; }

		/** Returns information about a solid paint. Caller must ensure the paint type is Solid before calling. */
		const SolidPaint& GetSolidPaint() const;

		/** Returns information about a linear gradient paint. Caller must ensure the paint type is LinearGradient before calling. */
		const LinearGradientPaint& GetLinearGradientPaint() const;

		/** Returns information about a box gradient paint. Caller must ensure the paint type is BoxGradient before calling. */
		const BoxGradientPaint& GetBoxGradientPaint() const;

		/** Returns information about a radial gradient paint. Caller must ensure the paint type is RadialGradient before calling. */
		const RadialGradientPaint& GetRadialGradientPaint() const;

		/** Creates a paint with a solid color covering the entire painted area. */
		static VectorGraphicsPaint CreateSolid(const Color& color);

		/** Creates a paint with one color at one edge, and another at the other edge, interpolated between on a line. */
		static VectorGraphicsPaint CreateLinearGradient(const Color& startColor, const Color& endColor, const Vector2& startPoint, const Vector2& endPoint);

		/** Creates a paint with on color at the center of a box, and another color at the edges/corners of the box. */
		static VectorGraphicsPaint CreateBoxGradient(const Color& innerColor, const Color& outerColor, const Area2& area, float cornerRadius, float feather);

		/** Creates a paint with on color at the center of a circle, and another color at the radius of the circle. */
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

	inline VectorGraphicsPaint VectorGraphicsPaint::CreateBoxGradient(const Color& innerColor, const Color& outerColor, const Area2& area, float cornerRadius, float feather)
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

	/** Types of commands as recorded by the vector path. See VectorPath for more information on each command. */
	enum class VectorPathCommandType
	{
		Unknown,

		Fill,
		Stroke,

		SetDrawCursor,
		ClosePath,
		SetPathSolidity,

		DrawLineTo,
		DrawArcTo,
		DrawCubicBezierTo,
		DrawQuadraticBezierTo,

		DrawRectangle,
		DrawRoundedRectangle,
		DrawEllipse,
		DrawArc,
	};

	/** Represents all the common state of VectorPath, required for executing fill & stroke commands. */
	struct VectorPathState
	{
		VectorGraphicsPaint StrokePaint = Color::kBlack; /**< Paint to use when drawing the stroke. */
		VectorGraphicsPaint FillPaint = Color::kWhite; /**< Paint to use when drawing fill. */
		float MiterLimit = 10.0f; /**< Controls when is a sharp corner beveled. */
		float StrokeWidth = 1.0f; /**< Controls the width of the drawn stroke. */
		VectorGraphicsLineCapType LineCapType = VectorGraphicsLineCapType::Butt; /**< Determines how is the end of the line drawn. */
		VectorGraphicsLineJoinStyle LineJoinType = VectorGraphicsLineJoinStyle::Miter; /**< Determines when is a sharp corner beveled. */
		bool AntialiasShape = true; /**< If true, drawn shape will use antialiasing. */
		float Alpha = 1.0f; /**< If less than zero, path will be drawn semi-transparently. */
		VectorGraphicsBlendMode BlendMode = VectorGraphicsBlendMode::SourceOver; /**< Determines how are paths blended with each other. */
		Area2 ScissorArea = Area2::kEmpty; /**< Allows you to clip path drawing to a particular rectangle. */
	};

	/** Represents a single command recorded by a VectorPath. */
	struct VectorPathCommand
	{
		VectorPathCommand()
		{
			
		}

		/** Draws the fill using the state at the provided index. */
		struct FillCommand
		{
			u32 StateIndex;
		};

		/** Draws the stroke using the state at the provided index. */
		struct StrokeCommand
		{
			u32 StateIndex;
		};

		/** Moves the cursor to a particular location without drawing anything .*/
		struct SetDrawCursorCommand
		{
			Vector2 Position;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				cursor = Position;
				return Area2::kEmpty;
			}
		};

		/**
		 * Changes path solidity, which determines if we're drawing new solid shapes or holes in existing shapes. This will affect the path immediately
		 * recorded before this command.
		 */
		struct SetPathSolidityCommand
		{
			VectorGraphicsPathSolidity Solidity = VectorGraphicsPathSolidity::Solid;
		};

		/** Draws a line from the current cursor location to the provided cursor location. Moves the new cursor location to the line end point. */
		struct DrawLineToCommand
		{
			Vector2 Target;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				const Vector2 minimum = Vector2::Min(cursor, Target);
				const Vector2 maximum = Vector2::Max(cursor, Target);
				const Vector2 size = maximum - minimum;

				cursor = Target;
				return Area2(minimum.X, minimum.Y, size.X, size.Y);
			}
		};

		/** Draws an arc from the current cursor location. Moves the cursor to the arc end point. */
		struct DrawArcToCommand
		{
			Vector2 MiddlePoint;
			Vector2 EndPoint;
			float Radius;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				const Vector2 minimum = Vector2::Min(cursor, Vector2::Min(MiddlePoint, EndPoint));
				const Vector2 maximum = Vector2::Max(cursor, Vector2::Max(MiddlePoint, EndPoint));
				const Vector2 size = maximum - minimum;

				cursor = EndPoint;
				return Area2(minimum.X, minimum.Y, size.X, size.Y);

			}
		};

		/** Draws a cubic bezier curve from the current cursor location. Moves the cursor to the curve end point. */
		struct DrawCubicBezierToCommand
		{
			Vector2 ControlPoint1;
			Vector2 ControlPoint2;
			Vector2 EndPoint;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				// TODO - Calculate minimum bezier curve bounds with a more algebraic approach
				const Vector2 minimum = Vector2::Min(cursor, Vector2::Min(ControlPoint1, Vector2::Min(ControlPoint2, EndPoint)));
				const Vector2 maximum = Vector2::Max(cursor, Vector2::Max(ControlPoint1, Vector2::Max(ControlPoint2, EndPoint)));
				const Vector2 size = maximum - minimum;

				cursor = EndPoint;
				return Area2(minimum.X, minimum.Y, size.X, size.Y);

			}
		};

		/** Draws a quadratic bezier curve from the current cursor location. Moves the cursor to the curve end point. */
		struct DrawQuadraticBezierToCommand
		{
			Vector2 ControlPoint;
			Vector2 EndPoint;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				// TODO - Calculate minimum bezier curve bounds with a more algebraic approach
				// See https://github.com/Pomax/bezierjs/blob/master/src/bezier.js
				const Vector2 minimum = Vector2::Min(cursor, Vector2::Min(ControlPoint, EndPoint));
				const Vector2 maximum = Vector2::Max(cursor, Vector2::Max(ControlPoint, EndPoint));
				const Vector2 size = maximum - minimum;

				cursor = EndPoint;
				return Area2(minimum.X, minimum.Y, size.X, size.Y);

			}
		};

		/** Draws a rectangle. */
		struct DrawRectangleCommand
		{
			Area2 Area;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				return Area;
			}
		};

		/** Draws a rectangle with rounded corners. */
		struct DrawRoundedRectangleCommand
		{
			Area2 Area;
			float RadiusTopLeft;
			float RadiusTopRight;
			float RadiusBottomLeft;
			float RadiusBottomRight;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				return Area;
			}
		};

		/** Draws an ellipse. */
		struct DrawEllipseCommand
		{
			Vector2 Origin;
			Vector2 Radius;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				return Area2(Origin.X - Radius.X, Origin.Y - Radius.Y, 2.0f * Radius.X, 2.0f * Radius.Y);
			}
		};

		/** Draws an arc. */
		struct DrawArcCommand
		{
			Vector2 Center;
			float Radius;
			Radian StartAngle;
			Radian EndAngle;
			VectorGraphicsPathWinding Direction;

			Area2 GetBoundsAndUpdateCursor(Vector2& cursor) const
			{
				// TODO - Calculate minimum arc bounds with a more algebraic approach
				return Area2(Center.X, Center.Y, Radius, Radius);
			}
		};

		VectorPathCommandType Type = VectorPathCommandType::Unknown;
		union
		{
			FillCommand Fill;
			StrokeCommand Stroke;

			SetDrawCursorCommand SetDrawCursor;
			SetPathSolidityCommand SetPathSolidity;

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

	/** Represents a vector path containing curves and geometric shapes that can be rasterized to any dimension. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(VectorGraphics)) VectorPath : public Resource
	{
	public:
		static constexpr Size2 kDefaultCanvasSize = Size2(512.0f, 512.0f);

		VectorPath(const Size2& canvasSize = kDefaultCanvasSize);

		 /** Determines the size of the coordinate system in which to draw the path. This will be used for scaling/offset when rasterizing the path and for bounds testing. */
		void SetCanvasSize(const Size2& canvasSize) { mCanvasSize = canvasSize; }

		/** @copydoc SetCanvasSize() */
		const Size2& GetCanvasSize() const { return mCanvasSize; }

		/** Changes the current location of the draw cursor. Any command using a draw cursor will use this value as the starting point. */
		VectorPath& SetDrawCursor(const Vector2& cursor);

		/**
		 * Sets the solidity of the drawn path. By default paths will draw solid shapes, but changing the solidity to Hole allows you to create
		 * holes in previously drawn paths. Solidity should be set /after/ recording a path, and will be applied to the last recorded path.
		 */
		VectorPath& SetSolidity(VectorGraphicsPathSolidity solidity);

		/** Closes a path by connecting the first and last path points. This should be called before starting a new path. */
		VectorPath& ClosePath();

		/** Draws a line from the current cursor position to the target position. Advances the cursor to the target position. */
		VectorPath& DrawLineTo(const Vector2& target);

		/**
		 * Draws an arc using three points on the arc (last cursor position, and two provided points), and a radius of the
		 * circle that the arc would be a part of.. Advances the cursor to the arc end point.
		 */
		VectorPath& DrawArcTo(const Vector2& middlePoint, const Vector2& endPoint, float radius);

		/**
		 * Draws a quadratic bezier curve using the current cursor position as the starting point. Advanced the cursor to the
		 * curve end point.
		 */
		VectorPath& DrawQuadraticBezierTo(const Vector2& controlPoint, const Vector2& endPoint);

		/**
		 * Draws a cubic bezier curve using the current cursor position as the starting point. Advanced the cursor to the
		 * curve end point.
		 */
		VectorPath& DrawCubicBezierTo(const Vector2& controlPoint1, const Vector2& controlPoint2, const Vector2& endPoint);

		/** Draws a rectangle. */
		VectorPath& DrawRectangle(const Area2& area);

		/** Draws a rectangle with rounded corners, all corners having the same radius. */
		VectorPath& DrawRoundedRectangle(const Area2& area, float cornerRadius);

		/** Draws a rectangle with rounded corners, with explicit radius for each corner. */
		VectorPath& DrawRoundedRectangle(const Area2& area, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius);

		/** Draws a circle. */
		VectorPath& DrawCircle(const Vector2& origin, float radius);

		/** Draws an ellipse. */
		VectorPath& DrawEllipse(const Vector2& origin, const Vector2& radius);

		/**
		 * Draws an arc. Note if the start of the arc doesn't correspond to the last drawn cursor position, and line will be drawn between the
		 * last drawn point (if any) and the arc start.
		 *
		 * @param	center		Center of the circle that the arc is a part of.
		 * @param	radius		Radius of the circle that the arc is a part of.
		 * @param	startAngle	Angle at which to start drawing the arc.
		 * @param	endAngle	Angle at which to start drawing the arc.
		 * @param	direction	Direction of the arc. Doesn't affect coordinate system of @p startAngle or @p endAngle, but the direction in which
		 *						the arc is filled. e.g. start angle 0, end angle 90 and direction 'counter-clickwise' will result in a 270 degree arc.
		 *
		 * @note	Note the underlying NVG backend is using non-conventional angle coordinates, where 90 degrees in on negative Y, and 270 on positive Y
		 *			(angles map to clockwise XY coordinates on the graph).
		 */
		VectorPath& DrawArc(const Vector2& center, float radius, Radian startAngle, Radian endAngle, VectorGraphicsPathWinding direction = VectorGraphicsPathWinding::Clockwise);

		/** Sets the paint to use when calling DrawFill(). */
		VectorPath& SetFillPaint(const VectorGraphicsPaint& paint);

		/** Sets the paint to use when calling DrawStroke(). */
		VectorPath& SetStrokePaint(const VectorGraphicsPaint& paint);

		/** Sets the width of the stroke to draw. */
		VectorPath& SetStrokeWidth(float strokeWidth);

		/** Sets the limit that controls when is sharp stroke corner beveled. */
		VectorPath& SetMiterLimit(float miterLimit);

		/** Sets the style that controls how is the line end rendered. */
		VectorPath& SetLineCapType(VectorGraphicsLineCapType lineCap);

		/** Sets the style that controls how are line bends rendered. */
		VectorPath& SetLineJoinType(VectorGraphicsLineJoinStyle lineJoin);

		/** Sets the transparency of the drawn shape. */
		VectorPath& SetAlpha(float alpha);

		/** Sets the blend mode that determines how a shape blends with existing shapes in the path. */
		VectorPath& SetBlendMode(VectorGraphicsBlendMode blendMode);

		/** Enables or disables antialiasing for the shape. */
		VectorPath& SetAntialiasShapes(bool antialiasShapes);

		/** Sets a rectangle to clip shape rendering to. */
		VectorPath& SetScissorRectangle(const Area2& scissorArea);

		/** Clears a previously set scissor rectangle. */
		VectorPath& ClearScissor();

		/** Draws the fill for the recorded shape. */
		VectorPath& DrawFill();

		/** Draws the stroke for the recorded shape. */
		VectorPath& DrawStroke();

		/** Returns all the currently recorded commands. */
		const Vector<VectorPathCommand>& GetCommands() const { return mCommands; }

		/** Returns all the states associated with the recorded commands. */
		const Vector<VectorPathState>& GetCommandStates() const { return mCommandStates; }

		/** Creates a renderable object that can be used for rasterizing the vector path into pixels. */
		SPtr<render::VectorPathRenderable> CreateRenderable(const VectorGraphicsSettings& settings) const;

		/**
		 * Creates a new empty vector path.
		 *
		 * @param	canvasSize		Determines the size of the coordinate system in which to draw the path. This will be used for scaling/offset
		 *							when rasterizing the path and for bounds testing. 
		 * @return					Newly created path.
		 */
		static SPtr<VectorPath> CreateShared(const Size2& canvasSize = kDefaultCanvasSize);

		/** @copydoc CreateShared() */
		static HVectorPath Create(const Size2& canvasSize = kDefaultCanvasSize);

	private:
		Size2 mCanvasSize;
		VectorPathState mCurrentState;
		Vector<VectorPathCommand> mCommands;
		Vector<VectorPathState> mCommandStates;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class VectorPathRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	namespace render
	{
		/** Allows a vector path to be rasterized into pixels. */
		class B3D_EXPORT VectorPathRenderable : public IReflectable
		{
		public:
			VectorPathRenderable(const b3d::VectorPath& vectorPath, const VectorGraphicsSettings& settings)
				: mSettings(settings)
			{ }
			~VectorPathRenderable() override = default;

			/** Returns the settings object used for creating this renderable. */
			const VectorGraphicsSettings& GetSettings() const { return mSettings; }

			/** Prepares GPU parameters for rendering. Must be called before Render(). */
			virtual SPtr<GpuParameterSet> Prepare() = 0;

			/**
			 * Records command required for rasterizing the path into pixels. Before calling this the user is required to have bound a render target containing
			 * a color texture and a stencil buffer. Prepare() must be called before this method.
			 */
			virtual void Render(GpuCommandBuffer& commandBuffer) = 0;

		protected:
			VectorGraphicsSettings mSettings;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			VectorPathRenderable() = default; // Deserialization only

			friend class b3d::VectorPathRenderableRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};
	} // namespace render

	/** @} */

} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for ValidParamKey. */
template <>
struct hash<b3d::VectorGraphicsSettings>
{
	size_t operator()(const b3d::VectorGraphicsSettings& value) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, value.Size);
		b3d::B3DCombineHash(hash, value.ScalingMode);
		b3d::B3DCombineHash(hash, value.Transform);
		b3d::B3DCombineHash(hash, value.UseAntialiasing);
		b3d::B3DCombineHash(hash, value.StencilStrokes);
		b3d::B3DCombineHash(hash, value.DevicePixelRatio);

		return hash;
	}
};
} // namespace std

/** @endcond */
