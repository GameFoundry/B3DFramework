//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNVGVectorGraphics.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "RenderAPI/BsGpuPipelineParameterLayout.h"
#include "Private/RTTI/BsNVGVectorGraphicsRTTI.h"

using namespace bs;

namespace bs::ct
{
	static VectorGraphicsBlendMode NVGCompositeOperationToBlendMode(const NVGcompositeOperationState& compositeOperationState)
	{
		B3D_ASSERT(compositeOperationState.srcRGB == compositeOperationState.srcAlpha);
		B3D_ASSERT(compositeOperationState.dstRGB == compositeOperationState.dstAlpha);

		if(compositeOperationState.srcRGB == NVG_ONE && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
			return VectorGraphicsBlendMode::SourceOver;
		else if(compositeOperationState.srcRGB == NVG_DST_ALPHA && compositeOperationState.dstRGB == NVG_ZERO)
			return VectorGraphicsBlendMode::SourceIn;
		else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_ZERO)
			return VectorGraphicsBlendMode::SourceOut;
		else if(compositeOperationState.srcRGB == NVG_DST_ALPHA && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
			return VectorGraphicsBlendMode::Atop;
		else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_ONE)
			return VectorGraphicsBlendMode::DestinationOver;
		else if(compositeOperationState.srcRGB == NVG_ZERO && compositeOperationState.dstRGB == NVG_SRC_ALPHA)
			return VectorGraphicsBlendMode::DestinationIn;
		else if(compositeOperationState.srcRGB == NVG_ZERO && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
			return VectorGraphicsBlendMode::DestinationOut;
		else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_SRC_ALPHA)
			return VectorGraphicsBlendMode::DestinationAtop;
		else if(compositeOperationState.srcRGB == NVG_ONE && compositeOperationState.dstRGB == NVG_ONE)
			return VectorGraphicsBlendMode::Lighter;
		else if(compositeOperationState.srcRGB == NVG_ONE && compositeOperationState.dstRGB == NVG_ZERO)
			return VectorGraphicsBlendMode::Copy;
		else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
			return VectorGraphicsBlendMode::Xor;

		B3D_ENSURE(false);
		return VectorGraphicsBlendMode::SourceOver;
	}

	static Matrix4 NVGTransformToB3DMatrix(float* transform)
	{
		return Matrix4(Matrix3(
			Vector3(transform[0], transform[1], 0.0f),
			Vector3(transform[2], transform[3], 0.0f),
			Vector3(transform[4], transform[5], 1.0f)));
	}

	static NVGcolor B3DColorToNVGColor(const Color& color)
	{
		return nvgRGBAf(color.R, color.G, color.B, color.A);
	}

	static Vector2 ApplyScale9GridTransform(const Vector2& input, const Size2& shapeSize, const RectOffset& scale9GridBorder, const Vector2& scale)
	{
		const float unscaledLeft = (float)scale9GridBorder.Left;
		const float unscaledRight = shapeSize.Width - (float)scale9GridBorder.Right;
		const float unscaledTop = (float)scale9GridBorder.Top;
		const float unscaledBottom = shapeSize.Height - (float)scale9GridBorder.Bottom;

		const float scaledWidth = shapeSize.Width * scale.X;
		const float scaledHeight = shapeSize.Height * scale.Y;
		const float scaledRight = scaledWidth - (float)scale9GridBorder.Right;
		const float scaledBottom = scaledHeight - (float)scale9GridBorder.Bottom;

		// Top left
		if(input.Y <= unscaledTop && input.X <= unscaledLeft)
		{
			return input;
		}
		// Top right
		else if(input.Y <= unscaledTop && input.X >= unscaledRight)
		{
			return Vector2(scaledRight + (input.X - unscaledRight), input.Y);
		}
		// Top center
		else if(input.Y <= unscaledTop)
		{
			return Vector2(input.X * scale.X, input.Y);
		}
		// Bottom left
		else if(input.Y >= unscaledBottom && input.X <= unscaledLeft)
		{
			return Vector2(input.X, scaledBottom + (input.Y - unscaledBottom));
		}
		// Bottom right
		else if(input.Y >= unscaledBottom && input.X >= unscaledRight)
		{
			return Vector2(scaledRight + (input.X - unscaledRight), scaledBottom + (input.Y - unscaledBottom));
		}
		// Bottom center
		else if(input.Y >= unscaledBottom)
		{
			return Vector2(input.X * scale.X, scaledBottom - (input.Y - unscaledBottom));
		}
		// Middle left
		else if(input.X <= unscaledLeft)
		{
			return Vector2(input.X, input.Y * scale.Y);
		}
		// Middle right
		else if(input.X >= unscaledRight)
		{
			return Vector2(scaledRight - (input.X - unscaledRight), input.Y * scale.Y);
		}

		// Middle center
		return input * scale;
	}

	static Vector2 ApplyScaleTransform(const Vector2& input, const VectorGraphicsSettings& settings)
	{
		const bool useScale9Grid =
			settings.Scale9GridBorder.Left != 0 ||
			settings.Scale9GridBorder.Right != 0 ||
			settings.Scale9GridBorder.Top != 0 ||
			settings.Scale9GridBorder.Bottom != 0;

		if(useScale9Grid)
			return ApplyScale9GridTransform(input, settings.Size, settings.Scale9GridBorder, settings.Scale);

		return input * settings.Scale;
	}

	static NVGpaint CreateNVGPaint(NVGcontext& context, const VectorGraphicsPaint& paint, const VectorGraphicsSettings& settings)
	{
		switch(paint.GetType())
		{
		default:
			B3D_ENSURE(false);
		case VectorGraphicsPaintType::Solid:
			{
				const NVGcolor& color = B3DColorToNVGColor(paint.GetSolidPaint().Color);
				return nvgLinearGradient(&context, 0.0f, 0.0f, 1.0f, 1.0f, color, color);
			}
		case VectorGraphicsPaintType::LinearGradient:
			{
				const VectorGraphicsPaint::LinearGradientPaint& linearGradientPaint = paint.GetLinearGradientPaint();
				const NVGcolor& startColor = B3DColorToNVGColor(linearGradientPaint.StartColor);
				const NVGcolor& endColor = B3DColorToNVGColor(linearGradientPaint.EndColor);
				const Vector2& startPoint = ApplyScaleTransform(linearGradientPaint.StartPoint, settings);
				const Vector2& endPoint = ApplyScaleTransform(linearGradientPaint.EndPoint, settings);

				return nvgLinearGradient(&context, startPoint.X, startPoint.Y, endPoint.X, endPoint.Y, startColor, endColor);
			}
		case VectorGraphicsPaintType::BoxGradient:
			{
				const VectorGraphicsPaint::BoxGradientPaint& boxGradientPaint = paint.GetBoxGradientPaint();
				const NVGcolor& innerColor = B3DColorToNVGColor(boxGradientPaint.InnerColor);
				const NVGcolor& outerColor = B3DColorToNVGColor(boxGradientPaint.OuterColor);
				const Vector2& topLeft = ApplyScaleTransform(Vector2(boxGradientPaint.Area.X, boxGradientPaint.Area.Y), settings);
				const Vector2& bottomRight = ApplyScaleTransform(Vector2(boxGradientPaint.Area.X + boxGradientPaint.Area.Width, boxGradientPaint.Area.Y + boxGradientPaint.Area.Height), settings);

				const Rect2 area(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y);

				return nvgBoxGradient(&context, area.X, area.Y, area.Width, area.Height, boxGradientPaint.CornerRadius, boxGradientPaint.Feather, innerColor, outerColor);
			}
		case VectorGraphicsPaintType::RadialGradient:
			{
				const VectorGraphicsPaint::RadialGradientPaint& radialGradientPaint = paint.GetRadialGradientPaint();
				const NVGcolor& innerColor = B3DColorToNVGColor(radialGradientPaint.InnerColor);
				const NVGcolor& outerColor = B3DColorToNVGColor(radialGradientPaint.OuterColor);
				const Vector2& center = ApplyScaleTransform(radialGradientPaint.Center, settings);
				const float innerRadius = radialGradientPaint.InnerRadius * settings.Scale.X;
				const float outerRadius = radialGradientPaint.OuterRadius * settings.Scale.X;

				return nvgRadialGradient(&context, center.X, center.Y, innerRadius, outerRadius, innerColor, outerColor);
			}
		}
	}

	static void ApplyNVGState(NVGcontext& context, const VectorPathState& state, const VectorGraphicsSettings& settings)
	{
		nvgFillPaint(&context, CreateNVGPaint(context, state.FillPaint, settings));
		nvgStrokePaint(&context, CreateNVGPaint(context, state.StrokePaint, settings));

		nvgMiterLimit(&context, state.MiterLimit);
		nvgStrokeWidth(&context, state.StrokeWidth);

		nvgGlobalAlpha(&context, state.Alpha);
		nvgShapeAntiAlias(&context, state.AntialiasShape);

		if(state.ScissorArea != Rect2::kEmpty)
			nvgScissor(&context, state.ScissorArea.X, state.ScissorArea.Y, state.ScissorArea.Width, state.ScissorArea.Height);
		else
			nvgResetScissor(&context);

		switch(state.LineCapType)
		{
		case VectorGraphicsLineCapType::Butt: nvgLineCap(&context, NVG_BUTT); break;
		case VectorGraphicsLineCapType::Round: nvgLineCap(&context, NVG_ROUND); break;
		case VectorGraphicsLineCapType::Square: nvgLineCap(&context, NVG_SQUARE); break;
		}

		switch(state.LineJoinType)
		{
		case VectorGraphicsLineJoinStyle::Miter: nvgLineJoin(&context, NVG_MITER); break;
		case VectorGraphicsLineJoinStyle::Round: nvgLineJoin(&context, NVG_ROUND); break;
		case VectorGraphicsLineJoinStyle::Bevel: nvgLineJoin(&context, NVG_BEVEL); break;
		}

		switch(state.BlendMode)
		{
		case VectorGraphicsBlendMode::SourceOver: nvgGlobalCompositeOperation(&context, NVG_SOURCE_OVER); break;
		case VectorGraphicsBlendMode::SourceIn: nvgGlobalCompositeOperation(&context, NVG_SOURCE_IN); break;
		case VectorGraphicsBlendMode::SourceOut: nvgGlobalCompositeOperation(&context, NVG_SOURCE_OUT); break;
		case VectorGraphicsBlendMode::Atop: nvgGlobalCompositeOperation(&context, NVG_ATOP); break;
		case VectorGraphicsBlendMode::DestinationOver: nvgGlobalCompositeOperation(&context, NVG_DESTINATION_OVER); break;
		case VectorGraphicsBlendMode::DestinationIn: nvgGlobalCompositeOperation(&context, NVG_DESTINATION_IN); break;
		case VectorGraphicsBlendMode::DestinationOut: nvgGlobalCompositeOperation(&context, NVG_DESTINATION_OUT); break;
		case VectorGraphicsBlendMode::DestinationAtop: nvgGlobalCompositeOperation(&context, NVG_DESTINATION_ATOP); break;
		case VectorGraphicsBlendMode::Lighter: nvgGlobalCompositeOperation(&context, NVG_LIGHTER); break;
		case VectorGraphicsBlendMode::Copy: nvgGlobalCompositeOperation(&context, NVG_COPY); break;
		case VectorGraphicsBlendMode::Xor: nvgGlobalCompositeOperation(&context, NVG_XOR); break;
		}
	}

	static void ApplyPathCommands(NVGcontext& context, const VectorPath& path, const VectorGraphicsSettings& settings)
	{
		nvgBeginPath(&context);

		bool pathClosed = false;
		const auto fnEnsurePathOpen = [&context, &pathClosed]()
		{
			if(pathClosed)
			{
				nvgBeginPath(&context);
				pathClosed = false;
			}
		};

		const Vector<VectorPathCommand>& commands = path.GetCommands();
		const Vector<VectorPathState>& commandStates = path.GetCommandStates();

		for(const auto& command : commands)
		{
			switch(command.Type)
			{
			case VectorPathCommandType::Fill:
				{
					const VectorPathState& state = commandStates[command.Fill.StateIndex];
					ApplyNVGState(context, state, settings);

					nvgFill(&context);
					break;
				}

			case VectorPathCommandType::Stroke:
				{
					const VectorPathState& state = commandStates[command.Stroke.StateIndex];
					ApplyNVGState(context, state, settings);

					nvgStroke(&context);
					break;
				}
			case VectorPathCommandType::SetDrawCursor:
				{
					fnEnsurePathOpen();

					const Vector2& position = ApplyScaleTransform(command.SetDrawCursor.Position, settings);
					nvgMoveTo(&context, position.X, position.Y);
					break;
				}

			case VectorPathCommandType::ClosePath:
				{
					nvgClosePath(&context);
					pathClosed = true;
					break;
				}

			case VectorPathCommandType::SetPathWinding:
				{
					switch(command.SetPathWinding.Winding)
					{
					case VectorGraphicsPathWinding::Clockwise:
						nvgPathWinding(&context, NVG_CW);
						break;
					case VectorGraphicsPathWinding::Counterclockwise:
						nvgPathWinding(&context, NVG_CCW);
						break;
					}
					break;
				}

			case VectorPathCommandType::DrawLineTo:
				{
					fnEnsurePathOpen();

					const Vector2& position = ApplyScaleTransform(command.DrawLineTo.Target, settings);
					nvgLineTo(&context, position.X, position.Y);
					break;
				}
			case VectorPathCommandType::DrawArcTo:
				{
					fnEnsurePathOpen();

					const Vector2& middlePoint = ApplyScaleTransform(Vector2(command.DrawArcTo.MiddlePoint), settings);
					const Vector2& endPoint = ApplyScaleTransform(Vector2(command.DrawArcTo.EndPoint), settings);
					const float radius = command.DrawArcTo.Radius * settings.Scale.X;

					nvgArcTo(&context, middlePoint.X, middlePoint.Y, endPoint.X, endPoint.Y, radius);
					break;
				}
			case VectorPathCommandType::DrawCubicBezierTo:
				{
					fnEnsurePathOpen();

					const Vector2 controlPoint1 = ApplyScaleTransform(command.DrawCubicBezierTo.ControlPoint1, settings);
					const Vector2 controlPoint2 = ApplyScaleTransform(command.DrawCubicBezierTo.ControlPoint2, settings);
					const Vector2 endPoint = ApplyScaleTransform(command.DrawCubicBezierTo.EndPoint, settings);

					nvgBezierTo(&context, controlPoint1.X, controlPoint1.Y, controlPoint2.X, controlPoint2.Y, endPoint.X, endPoint.Y);
					break;
				}
			case VectorPathCommandType::DrawQuadraticBezierTo:
				{
					fnEnsurePathOpen();

					const Vector2 controlPoint = ApplyScaleTransform(command.DrawQuadraticBezierTo.ControlPoint, settings);
					const Vector2 endPoint = ApplyScaleTransform(command.DrawQuadraticBezierTo.EndPoint, settings);

					nvgQuadTo(&context, controlPoint.X, controlPoint.Y, endPoint.X, endPoint.Y);
					break;
				}

			case VectorPathCommandType::DrawRectangle:
				{
					fnEnsurePathOpen();

					const Rect2& area = command.DrawRectangle.Area;

					const Vector2& topLeft = ApplyScaleTransform(Vector2(area.X, area.Y), settings);
					const Vector2& bottomRight = ApplyScaleTransform(Vector2(area.X + area.Width, area.Y + area.Height), settings);

					nvgRect(&context, topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y);
					break;
				}

			case VectorPathCommandType::DrawRoundedRectangle:
				{
					fnEnsurePathOpen();

					const VectorPathCommand::DrawRoundedRectangleCommand& drawRoundedRectangle = command.DrawRoundedRectangle;
					const Rect2& area = drawRoundedRectangle.Area;

					const Vector2& topLeft = ApplyScaleTransform(Vector2(area.X, area.Y), settings);
					const Vector2& bottomRight = ApplyScaleTransform(Vector2(area.X + area.Width, area.Y + area.Height), settings);

					nvgRoundedRectVarying(&context, topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y,
						drawRoundedRectangle.RadiusTopLeft, drawRoundedRectangle.RadiusTopRight,
						drawRoundedRectangle.RadiusBottomRight, drawRoundedRectangle.RadiusBottomLeft);
					break;
				}

			case VectorPathCommandType::DrawEllipse:
				{
					fnEnsurePathOpen();

					const VectorPathCommand::DrawEllipseCommand& drawEllipse = command.DrawEllipse;
					const Vector2& origin = ApplyScaleTransform(drawEllipse.Origin, settings);
					const Vector2& radius = drawEllipse.Radius * settings.Scale;

					nvgEllipse(&context, origin.X, origin.Y, radius.X, radius.Y);
					break;
				}

			case VectorPathCommandType::DrawArc:
				{
					fnEnsurePathOpen();

					const VectorPathCommand::DrawArcCommand& drawArc = command.DrawArc;
					const Vector2 origin = ApplyScaleTransform(drawArc.Origin, settings);
					const float radius = drawArc.Radius * settings.Scale.X;
					const NVGwinding winding = drawArc.Direction == VectorGraphicsPathWinding::Clockwise ? NVG_CW : NVG_CCW;

					nvgArc(&context, origin.X, origin.Y, radius, drawArc.StartAngle.GetValueInRadians(), drawArc.EndAngle.GetValueInRadians(), winding);
					break;
				}
			}
		}
	}

	static NVGRenderUniforms CreateNVGRenderUniformParameters(NVGpaint* paint, NVGscissor* scissor, float fringe, float width, float strokeThreshold)
	{
		auto fnConvertAndPremultiplyColor = [](NVGcolor& color) { return Color(color.r * color.a, color.g * color.a, color.b * color.a, color.a); };

		NVGRenderUniforms uniformParameters;
		uniformParameters.InnerColor = fnConvertAndPremultiplyColor(paint->innerColor);
		uniformParameters.OuterColor = fnConvertAndPremultiplyColor(paint->outerColor);

		if(scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f)
		{
			uniformParameters.ScissorMatrix = Matrix4::kZero;
			uniformParameters.ScissorExtents = Vector2::kOne;
			uniformParameters.ScissorScale = Vector2::kOne;
		}
		else
		{
			float inverseScissorTransform[6];
			nvgTransformInverse(inverseScissorTransform, scissor->xform);

			uniformParameters.ScissorMatrix = NVGTransformToB3DMatrix(inverseScissorTransform);
			uniformParameters.ScissorExtents = Vector2(scissor->extent[0], scissor->extent[1]);
			uniformParameters.ScissorScale.X = Math::SquareRoot(scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]) / fringe;
			uniformParameters.ScissorScale.Y = Math::SquareRoot(scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]) / fringe;
		}

		uniformParameters.Extent = Vector2(paint->extent[0], paint->extent[1]);
		uniformParameters.StrokeMultiplier = (width * 0.5f + fringe * 0.5f) / fringe;
		uniformParameters.StrokeThreshold = strokeThreshold;

		uniformParameters.Radius = paint->radius;
		uniformParameters.Feather = paint->feather;

		float inversePaintTransform[6];
		nvgTransformInverse(inversePaintTransform, paint->xform);
		uniformParameters.PaintMatrix = NVGTransformToB3DMatrix(inversePaintTransform);

		return uniformParameters;
	}

	static void PopulateNVGViewUniformBuffer(const SPtr<ct::GpuBuffer>& uniformBuffer, const Rect2I& viewRegion)
	{
		ct::gVectorGraphicsViewUniforms.gViewportOffset.Set(uniformBuffer, Vector2(-(float)viewRegion.X, -(float)viewRegion.Y));
		ct::gVectorGraphicsViewUniforms.gInverseViewportHalfSize.Set(uniformBuffer, Vector2(1.0f / ((float)viewRegion.Width * 0.5f), 1.0f / ((float)viewRegion.Height * 0.5f)));

		bool viewportYFlip = true;
		const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
		{
			const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
			viewportYFlip = gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down;
		}

		ct::gVectorGraphicsViewUniforms.gViewportYFlip.Set(uniformBuffer, viewportYFlip ? -1.0f : 1.0f);
	}

	VectorGraphicsRenderUniformDefinition gVectorGraphicsRenderUniforms;
	VectorGraphicsViewUniformDefinition gVectorGraphicsViewUniforms;

	VectorGraphicsMaterial* VectorGraphicsMaterial::GetVariation(NVGDrawMode drawMode, VectorGraphicsBlendMode blendMode, bool antialiasing)
	{
		return Get(ShaderVariationParameters(
			{
				ShaderVariationParameter("DRAW_MODE", (u32)drawMode),
				ShaderVariationParameter("BLEND_MODE", (u32)blendMode),
				ShaderVariationParameter("EDGE_AA", antialiasing),
			}));
	}

	NVGVectorPathRenderable::NVGVectorPathRenderable(const VectorPath& vectorPath, const VectorGraphicsSettings& settings)
		:VectorPathRenderable(vectorPath, settings), mRawRenderData(PlaybackPathCommands(vectorPath, settings))
	{ }

	void NVGVectorPathRenderable::NVGRenderFillCallback(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths)
	{
		if(npaths == 0)
			return;

		NVGRenderContext& userContext = *(NVGRenderContext*)uptr;
		NVGPathRenderData& outputRenderData = userContext.OutputRenderData;
		const bool isConvex = npaths == 1 && paths[0].convex;

		NVGRenderCommand renderCommand;
		renderCommand.Type = isConvex ? NVGRenderCommandType::ConvexFill : NVGRenderCommandType::Fill;
		renderCommand.PathCount = npaths;
		renderCommand.BlendMode = NVGCompositeOperationToBlendMode(compositeOperation);

		u32 fillIndexOffset = (u32)outputRenderData.Indices.size();
		u32 fillTriangleCount = 0;
		for(u32 pathIndex = 0; pathIndex < (u32)npaths; ++pathIndex)
		{
			const NVGpath& path = paths[pathIndex];
			if(path.nfill > 2)
			{
				const u32 vertexCount = (u32)path.nfill;
				const u32 vertexOffset = (u32)outputRenderData.Vertices.size();
				for(u32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					const Vector2 position = Vector2(path.fill[vertexIndex].x, path.fill[vertexIndex].y);
					const Vector2 uv = Vector2(path.fill[vertexIndex].u, path.fill[vertexIndex].v);

					NVGVertex vertex(position, uv);
					outputRenderData.Vertices.push_back(vertex);
				}

				const u32 indexOffset = (u32)outputRenderData.Indices.size();
				const u32 triangleCount = vertexCount - 2;
				for(u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
				{
					outputRenderData.Indices.push_back(vertexOffset);
					outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 2);
					outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 1);
				}

				fillTriangleCount += triangleCount;

				if(isConvex)
					outputRenderData.Submeshes.push_back(SubMesh(indexOffset, triangleCount * 3, DOT_TRIANGLE_LIST));
			}
		}

		u32 strokeIndexOffset = (u32)outputRenderData.Indices.size();
		u32 strokeTriangleCount = 0;
		for(int pathIndex = 0; pathIndex < npaths; ++pathIndex)
		{
			const NVGpath& path = paths[pathIndex];
			if(path.nstroke > 2)
			{
				const u32 vertexCount = (u32)path.nstroke;
				const u32 vertexOffset = (u32)outputRenderData.Vertices.size();
				for(u32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					const Vector2 position = Vector2(path.stroke[vertexIndex].x, path.stroke[vertexIndex].y);
					const Vector2 uv = Vector2(path.stroke[vertexIndex].u, path.stroke[vertexIndex].v);

					const NVGVertex vertex(position, uv);
					outputRenderData.Vertices.push_back(vertex);
				}

				const u32 indexOffset = (u32)outputRenderData.Indices.size();
				const u32 triangleCount = vertexCount - 2;
				for(u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
				{
					if(triangleIndex % 2 == 0)
					{
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 0);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 2);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 1);
					}
					else
					{
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 1);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 2);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 0);
					}
				}

				strokeTriangleCount += triangleCount;

				if(isConvex)
					outputRenderData.Submeshes.push_back(SubMesh(indexOffset, triangleCount * 3, DOT_TRIANGLE_LIST));
			}
		}

		if(!isConvex)
		{
			const Vector2 quadVertexPositions[] = {
				Vector2(bounds[2], bounds[3]),
				Vector2(bounds[2], bounds[1]),
				Vector2(bounds[0], bounds[3]),
				Vector2(bounds[0], bounds[1])
			};

			const u32 indexOffset = (u32)outputRenderData.Indices.size();
			const u32 vertexOffset = (u32)outputRenderData.Vertices.size();
			for(u32 vertexIndex = 0; vertexIndex < B3DSize(quadVertexPositions); vertexIndex++)
				outputRenderData.Vertices.push_back(NVGVertex(quadVertexPositions[vertexIndex], Vector2(0.5f, 1.0f)));

			outputRenderData.Indices.push_back(vertexOffset);
			outputRenderData.Indices.push_back(vertexOffset + 2);
			outputRenderData.Indices.push_back(vertexOffset + 1);

			outputRenderData.Indices.push_back(vertexOffset + 2);
			outputRenderData.Indices.push_back(vertexOffset + 3);
			outputRenderData.Indices.push_back(vertexOffset + 1);

			outputRenderData.Submeshes.push_back(SubMesh(fillIndexOffset, fillTriangleCount * 3, DOT_TRIANGLE_LIST));
			outputRenderData.Submeshes.push_back(SubMesh(strokeIndexOffset, strokeTriangleCount * 3, DOT_TRIANGLE_LIST));
			outputRenderData.Submeshes.push_back(SubMesh(indexOffset, 6, DOT_TRIANGLE_LIST));
		}

		renderCommand.PrimaryPassUniforms = CreateNVGRenderUniformParameters(paint, scissor, fringe, fringe, -1.0f);
		outputRenderData.RenderCommands.push_back(renderCommand);
	}

	void NVGVectorPathRenderable::NVGRenderStrokeCallback(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths)
	{
		if(npaths == 0)
			return;

		NVGRenderContext& userContext = *(NVGRenderContext*)uptr;
		NVGPathRenderData& outputRenderData = userContext.OutputRenderData;

		NVGRenderCommand renderCommand;
		renderCommand.Type = NVGRenderCommandType::Stroke;
		renderCommand.PathCount = npaths;
		renderCommand.BlendMode = NVGCompositeOperationToBlendMode(compositeOperation);

		// Note: Duplicated code from NVGRenderFill
		u32 strokeIndexOffset = (u32)outputRenderData.Indices.size();
		u32 strokeTriangleCount = 0;
		for(int pathIndex = 0; pathIndex < npaths; ++pathIndex)
		{
			const NVGpath& path = paths[pathIndex];
			if(path.nstroke > 2)
			{
				const u32 vertexCount = (u32)path.nstroke;
				const u32 vertexOffset = (u32)outputRenderData.Vertices.size();
				for(u32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					const Vector2 position = Vector2(path.stroke[vertexIndex].x, path.stroke[vertexIndex].y);
					const Vector2 uv = Vector2(path.stroke[vertexIndex].u, path.stroke[vertexIndex].v);

					const NVGVertex vertex(position, uv);
					outputRenderData.Vertices.push_back(vertex);
				}

				const u32 triangleCount = vertexCount - 2;
				for(u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
				{
					if(triangleIndex % 2 == 0)
					{
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 0);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 2);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 1);
					}
					else
					{
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 1);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 2);
						outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 0);
					}
				}

				strokeTriangleCount += triangleCount;
			}
		}

		outputRenderData.Submeshes.push_back(SubMesh(strokeIndexOffset, strokeTriangleCount * 3, DOT_TRIANGLE_LIST));

		renderCommand.PrimaryPassUniforms = CreateNVGRenderUniformParameters(paint, scissor, fringe, strokeWidth, -1.0f);
		if(userContext.Settings.StencilStrokes)
			renderCommand.SecondaryPassUniforms = CreateNVGRenderUniformParameters(paint, scissor, fringe, strokeWidth, 1.0f - 0.5f / 255.0f);

		outputRenderData.RenderCommands.push_back(renderCommand);
	}

	NVGPathRenderData NVGVectorPathRenderable::PlaybackPathCommands(const VectorPath& vectorPath, const VectorGraphicsSettings& settings)
	{
		NVGRenderContext userContext;
		userContext.Settings = settings;

		NVGparams nvgParameters;
		B3DZeroOut(nvgParameters);

		nvgParameters.userPtr = &userContext;
		nvgParameters.renderCreate = [](void* uptr)
		{ return 1; };
		nvgParameters.renderViewport = [](void* uptr, float width, float height, float devicePixelRatio) {};
		nvgParameters.renderCancel = [](void* uptr) {};
		nvgParameters.renderFlush = [](void* uptr) {};
		nvgParameters.renderFill = NVGRenderFillCallback;
		nvgParameters.renderStroke = NVGRenderStrokeCallback;
		nvgParameters.renderDelete = [](void* uptr) {};
		nvgParameters.edgeAntiAlias = settings.UseAntialiasing;

		NVGcontext* const nvgContext = nvgCreateInternal(&nvgParameters);

		nvgBeginFrame(nvgContext, (float)settings.Size.Width, (float)settings.Size.Height, settings.DevicePixelRatio);

		const Matrix4& transform = settings.Transform;
		const Vector3 translation = transform.GetTranslation();
		nvgTranslate(nvgContext, translation.X, translation.Y);
		nvgTransform(nvgContext, transform[0][0], transform[1][0], transform[0][1], transform[1][1], transform[0][2], transform[1][2]);

		ApplyPathCommands(*nvgContext, vectorPath, settings);

		nvgEndFrame(nvgContext);
		nvgDeleteInternal(nvgContext);

		return std::move(userContext.OutputRenderData);
	}

	NVGVectorPathRenderable::RenderGpuBuffers NVGVectorPathRenderable::CookRenderBuffers()
	{
		RenderGpuBuffers renderBuffers;

		// Create vertex and index buffers
		const u32 vertexCount = (u32)mRawRenderData.Vertices.size();
		const u32 indexCount = (u32)mRawRenderData.Indices.size();

		if(vertexCount == 0 || indexCount == 0)
			return renderBuffers;

		const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		if(!gpuDevice)
			return renderBuffers;

		SmallVector<VertexElement, 2> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_POSITION));
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));

		renderBuffers.VertexDescription = B3DMakeShared<VertexDescription>(vertexElements);
		B3D_ASSERT(renderBuffers.VertexDescription->GetVertexStride() == sizeof(NVGVertex));

		GpuBufferInformation indexBufferCreateInformation;
		indexBufferCreateInformation.Type = GpuBufferType::Index;
		indexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU;
		indexBufferCreateInformation.Index.Type = IT_32BIT;
		indexBufferCreateInformation.Index.Count = indexCount;

		renderBuffers.IndexBuffer = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

		const u32 indexBufferSize = indexCount * sizeof(u32);
		renderBuffers.IndexBuffer->WriteData(0, indexBufferSize, mRawRenderData.Indices.data());

		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU;
		vertexBufferCreateInformation.Vertex.ElementSize = renderBuffers.VertexDescription->GetVertexStride();
		vertexBufferCreateInformation.Vertex.Count = vertexCount;

		renderBuffers.VertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

		const u32 vertexBufferSize = renderBuffers.VertexDescription->GetVertexStride() * vertexCount;
		renderBuffers.VertexBuffer->WriteData(0, vertexBufferSize, mRawRenderData.Vertices.data());

		u32 uniformBlockCount = 0;
		for(const auto& command : mRawRenderData.RenderCommands)
		{
			switch(command.Type)
			{
			case NVGRenderCommandType::Fill:
				uniformBlockCount += 2;
				break;
			case NVGRenderCommandType::ConvexFill:
				uniformBlockCount++;
				break;
			case NVGRenderCommandType::Stroke:
				uniformBlockCount += mSettings.StencilStrokes ? 2 : 1;
				break;
			}
		}

		// Create uniform buffers
		renderBuffers.RenderUniformBuffer = gVectorGraphicsRenderUniforms.CreateBuffer(uniformBlockCount);
		B3D_ASSERT(ct::gVectorGraphicsRenderUniforms.GetSize() == sizeof(NVGRenderUniforms)); // TODO - I need a way to assign parameter block entries into a particular uniform block, so I don't just do a memcpy (it might not work everywhere)

		NVGRenderUniforms simplePassUniforms;
		B3DZeroOut(simplePassUniforms);
		simplePassUniforms.StrokeThreshold = -1.0f;

		const u32 uniformBlockStride = Math::CeilToMultiple(gVectorGraphicsRenderUniforms.GetSize(), gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);

		u8* uniformBufferData = (u8*)renderBuffers.RenderUniformBuffer->Lock(GBL_WRITE_ONLY_DISCARD);

		for(const auto& command : mRawRenderData.RenderCommands)
		{
			switch(command.Type)
			{
			case NVGRenderCommandType::Fill:
				memcpy(uniformBufferData, &simplePassUniforms, sizeof(simplePassUniforms));
				uniformBufferData += uniformBlockStride;

				// Fallthrough
			case NVGRenderCommandType::ConvexFill:
				memcpy(uniformBufferData, &command.PrimaryPassUniforms, sizeof(command.PrimaryPassUniforms));
				uniformBufferData += uniformBlockStride;
				break;
			case NVGRenderCommandType::Stroke:
				if(mSettings.StencilStrokes && B3D_ENSURE(command.SecondaryPassUniforms.has_value()))
				{
					memcpy(uniformBufferData, &command.SecondaryPassUniforms.value(), sizeof(command.SecondaryPassUniforms.value()));
					uniformBufferData += uniformBlockStride;
				}

				memcpy(uniformBufferData, &command.PrimaryPassUniforms, sizeof(command.PrimaryPassUniforms));
				uniformBufferData += uniformBlockStride;

				break;
			}
		}

		renderBuffers.RenderUniformBuffer->Unlock();

		renderBuffers.ViewUniformBuffer = ct::gVectorGraphicsViewUniforms.CreateBuffer();
		PopulateNVGViewUniformBuffer(renderBuffers.ViewUniformBuffer, Rect2I(0, 0, Math::RoundToU32(mSettings.Size.Width), Math::RoundToU32(mSettings.Size.Height)));

		return renderBuffers;
	}

	void NVGVectorPathRenderable::Render(GpuCommandBuffer& commandBuffer)
	{
		if(!mRenderBuffersCooked)
		{
			mRenderBuffers = CookRenderBuffers();
			mRenderBuffersCooked = true;
		}

		SPtr<GpuBuffer> vertexBuffers[] = { mRenderBuffers.VertexBuffer };
		commandBuffer.SetVertexDescription(mRenderBuffers.VertexDescription);
		commandBuffer.SetVertexBuffers(0, vertexBuffers, 1);
		commandBuffer.SetIndexBuffer(mRenderBuffers.IndexBuffer); // TODO - We shouldn't need one at all actually
		commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

		// Note: The parameter layout for all variations must match
		const SPtr<ct::GpuParameters> gpuParameters = ct::VectorGraphicsMaterial::Get()->GetParams();
		gpuParameters->SetUniformBuffer("RenderUniforms", mRenderBuffers.RenderUniformBuffer);
		gpuParameters->SetUniformBuffer("ViewUniforms", mRenderBuffers.ViewUniformBuffer);

		const u32 renderUniformBufferDynamicIndex = gpuParameters->GetPipelineParameterInformation()->GetDynamicOffsetIndex(GPT_FRAGMENT_PROGRAM, "RenderUniforms");
		B3D_ENSURE(renderUniformBufferDynamicIndex != ~0u);

		commandBuffer.SetGpuParameters(gpuParameters);

		const u32 vertexCount = (u32)mRawRenderData.Vertices.size();

		u32 uniformBlockStride = gVectorGraphicsRenderUniforms.GetSize();
		if(const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice())
			uniformBlockStride = Math::CeilToMultiple(uniformBlockStride, gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);

		// Execute draw commands
		u32 uniformBlockIndex = 0;
		u32 submeshIndex = 0;
		for(const auto& command : mRawRenderData.RenderCommands)
		{
			switch(command.Type)
			{
			case NVGRenderCommandType::Fill:
				{
					const SubMesh& fillShapeStencilSubmesh = mRawRenderData.Submeshes[submeshIndex++];
					commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					ct::VectorGraphicsMaterial* const fillShapeStencilMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillShapeStencil, command.BlendMode, mSettings.UseAntialiasing);
					if(B3D_ENSURE(fillShapeStencilMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(fillShapeStencilMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(fillShapeStencilSubmesh.IndexOffset, fillShapeStencilSubmesh.IndexCount, 0, vertexCount, 1);
					}

					const SubMesh& strokeSubmesh = mRawRenderData.Submeshes[submeshIndex++];
					commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					if(mSettings.UseAntialiasing)
					{
						ct::VectorGraphicsMaterial* const fillAAMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillAA, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(fillAAMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(fillAAMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}
					}

					const SubMesh& quadSubmesh = mRawRenderData.Submeshes[submeshIndex++];
					ct::VectorGraphicsMaterial* const fillDrawMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillDraw, command.BlendMode, mSettings.UseAntialiasing);
					if(B3D_ENSURE(fillDrawMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(fillDrawMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(quadSubmesh.IndexOffset, quadSubmesh.IndexCount, 0, vertexCount, 1);
					}
				}
				break;
			case NVGRenderCommandType::ConvexFill:
			{
				ct::VectorGraphicsMaterial* const simpleFillMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillSimple, command.BlendMode, mSettings.UseAntialiasing);
				if(B3D_ENSURE(simpleFillMaterial))
				{
					commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					commandBuffer.SetGpuGraphicsPipelineState(simpleFillMaterial->GetGraphicsPipeline());
					for(u32 pathIndex = 0; pathIndex < command.PathCount; pathIndex++)
					{
						// TODO - No need for multiple draw calls here, I can just intertwine these in a single buffer
						const SubMesh& fillSubmesh = mRawRenderData.Submeshes[submeshIndex++];
						commandBuffer.DrawIndexed(fillSubmesh.IndexOffset, fillSubmesh.IndexCount, 0, vertexCount, 1);

						const SubMesh& strokeSubmesh = mRawRenderData.Submeshes[submeshIndex++];
						commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
					}
				}
			}
				break;
			case NVGRenderCommandType::Stroke:
				{
					const SubMesh& strokeSubmesh = mRawRenderData.Submeshes[submeshIndex++];

					commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					if(mSettings.StencilStrokes)
					{
						ct::VectorGraphicsMaterial* const strokeStencilMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::StrokeStencil, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(strokeStencilMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(strokeStencilMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}

						commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
						uniformBlockIndex++;

						ct::VectorGraphicsMaterial* const strokeAAMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::StrokeAA, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(strokeAAMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(strokeAAMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}

						ct::VectorGraphicsMaterial* const clearStencilMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::ClearStencil, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(clearStencilMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(clearStencilMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}
					}
					else
					{
						ct::VectorGraphicsMaterial* const simpleFillMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillSimple, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(simpleFillMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(simpleFillMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}
					}
				}
				break;
			}
		}
	}

	RTTITypeBase* NVGVectorPathRenderable::GetRttiStatic()
	{
		return NVGVectorPathRenderableRTTI::Instance();
	}

	RTTITypeBase* NVGVectorPathRenderable::GetRtti() const
	{
		return GetRttiStatic();
	}
}

