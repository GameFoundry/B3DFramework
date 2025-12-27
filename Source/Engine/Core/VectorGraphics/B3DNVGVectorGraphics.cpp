//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNVGVectorGraphics.h"
#include "Mesh/B3DMesh.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "RTTI/B3DNVGVectorGraphicsRTTI.h"

using namespace b3d;

namespace b3d::render
{
	/** Converts a NanoVG composite operation into a B3D enum representing the operation. */
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

	/** Converts a NanoVG matrix into a B3D matrix. */
	static Matrix4 NVGTransformToB3DMatrix(float* transform)
	{
		return Matrix4(Matrix3(
			Vector3(transform[0], transform[2], transform[4]),
			Vector3(transform[1], transform[3], transform[5]),
			Vector3(0.0f, 0.0f, 1.0f)));
	}

	/** Converts a B3D color into a NanoVG color. */
	static NVGcolor B3DColorToNVGColor(const Color& color)
	{
		return nvgRGBAf(color.R, color.G, color.B, color.A);
	}

	/** Creates a NanoVG paint from a B3D paint. Gradients will have scaling applied as provided by vector graphics settings. */
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
				const Vector2& startPoint = linearGradientPaint.StartPoint;
				const Vector2& endPoint = linearGradientPaint.EndPoint;

				return nvgLinearGradient(&context, startPoint.X, startPoint.Y, endPoint.X, endPoint.Y, startColor, endColor);
			}
		case VectorGraphicsPaintType::BoxGradient:
			{
				const VectorGraphicsPaint::BoxGradientPaint& boxGradientPaint = paint.GetBoxGradientPaint();
				const NVGcolor& innerColor = B3DColorToNVGColor(boxGradientPaint.InnerColor);
				const NVGcolor& outerColor = B3DColorToNVGColor(boxGradientPaint.OuterColor);
				const Vector2& topLeft = Vector2(boxGradientPaint.Area.X, boxGradientPaint.Area.Y);
				const Vector2& bottomRight = Vector2(boxGradientPaint.Area.X + boxGradientPaint.Area.Width, boxGradientPaint.Area.Y + boxGradientPaint.Area.Height);

				const Area2 area(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y);

				return nvgBoxGradient(&context, area.X, area.Y, area.Width, area.Height, boxGradientPaint.CornerRadius, boxGradientPaint.Feather, innerColor, outerColor);
			}
		case VectorGraphicsPaintType::RadialGradient:
			{
				const VectorGraphicsPaint::RadialGradientPaint& radialGradientPaint = paint.GetRadialGradientPaint();
				const NVGcolor& innerColor = B3DColorToNVGColor(radialGradientPaint.InnerColor);
				const NVGcolor& outerColor = B3DColorToNVGColor(radialGradientPaint.OuterColor);
				const Vector2& center = radialGradientPaint.Center;
				const float innerRadius = radialGradientPaint.InnerRadius;
				const float outerRadius = radialGradientPaint.OuterRadius;

				return nvgRadialGradient(&context, center.X, center.Y, innerRadius, outerRadius, innerColor, outerColor);
			}
		}
	}

	/** Sets the current NanoVG state from the provided B3D vector path state object. All following fill or stroke operations will use this state. */
	static void ApplyNVGState(NVGcontext& context, const VectorPathState& state, const VectorGraphicsSettings& settings)
	{
		nvgFillPaint(&context, CreateNVGPaint(context, state.FillPaint, settings));
		nvgStrokePaint(&context, CreateNVGPaint(context, state.StrokePaint, settings));

		nvgMiterLimit(&context, state.MiterLimit);
		nvgStrokeWidth(&context, state.StrokeWidth);

		nvgGlobalAlpha(&context, state.Alpha);
		nvgShapeAntiAlias(&context, state.AntialiasShape);

		if(state.ScissorArea != Area2::kEmpty)
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

	/** Executes the NanoVG path commands in the provided path. Command output will be recorded to the provided context object. */
	static void ApplyPathCommands(NVGcontext& context, const b3d::VectorPath& path, const VectorGraphicsSettings& settings)
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

					const Vector2& position = command.SetDrawCursor.Position;
					nvgMoveTo(&context, position.X, position.Y);
					break;
				}

			case VectorPathCommandType::ClosePath:
				{
					nvgClosePath(&context);
					pathClosed = true;
					break;
				}

			case VectorPathCommandType::SetPathSolidity:
				{
					switch(command.SetPathSolidity.Solidity)
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

					const Vector2& position = command.DrawLineTo.Target;
					nvgLineTo(&context, position.X, position.Y);
					break;
				}
			case VectorPathCommandType::DrawArcTo:
				{
					fnEnsurePathOpen();

					const Vector2& middlePoint = Vector2(command.DrawArcTo.MiddlePoint);
					const Vector2& endPoint = Vector2(command.DrawArcTo.EndPoint);
					const float radius = command.DrawArcTo.Radius;

					nvgArcTo(&context, middlePoint.X, middlePoint.Y, endPoint.X, endPoint.Y, radius);
					break;
				}
			case VectorPathCommandType::DrawCubicBezierTo:
				{
					fnEnsurePathOpen();

					const Vector2 controlPoint1 = command.DrawCubicBezierTo.ControlPoint1;
					const Vector2 controlPoint2 = command.DrawCubicBezierTo.ControlPoint2;
					const Vector2 endPoint = command.DrawCubicBezierTo.EndPoint;

					nvgBezierTo(&context, controlPoint1.X, controlPoint1.Y, controlPoint2.X, controlPoint2.Y, endPoint.X, endPoint.Y);
					break;
				}
			case VectorPathCommandType::DrawQuadraticBezierTo:
				{
					fnEnsurePathOpen();

					const Vector2 controlPoint = command.DrawQuadraticBezierTo.ControlPoint;
					const Vector2 endPoint = command.DrawQuadraticBezierTo.EndPoint;

					nvgQuadTo(&context, controlPoint.X, controlPoint.Y, endPoint.X, endPoint.Y);
					break;
				}

			case VectorPathCommandType::DrawRectangle:
				{
					fnEnsurePathOpen();

					const Area2& area = command.DrawRectangle.Area;

					const Vector2& topLeft = Vector2(area.X, area.Y);
					const Vector2& bottomRight = Vector2(area.X + area.Width, area.Y + area.Height);

					nvgRect(&context, topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y);
					break;
				}

			case VectorPathCommandType::DrawRoundedRectangle:
				{
					fnEnsurePathOpen();

					const VectorPathCommand::DrawRoundedRectangleCommand& drawRoundedRectangle = command.DrawRoundedRectangle;
					const Area2& area = drawRoundedRectangle.Area;

					const Vector2& topLeft = Vector2(area.X, area.Y);
					const Vector2& bottomRight = Vector2(area.X + area.Width, area.Y + area.Height);

					nvgRoundedRectVarying(&context, topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y,
						drawRoundedRectangle.RadiusTopLeft, drawRoundedRectangle.RadiusTopRight,
						drawRoundedRectangle.RadiusBottomRight, drawRoundedRectangle.RadiusBottomLeft);
					break;
				}

			case VectorPathCommandType::DrawEllipse:
				{
					fnEnsurePathOpen();

					const VectorPathCommand::DrawEllipseCommand& drawEllipse = command.DrawEllipse;
					const Vector2& origin = drawEllipse.Origin;
					const Vector2& radius = drawEllipse.Radius;

					nvgEllipse(&context, origin.X, origin.Y, radius.X, radius.Y);
					break;
				}

			case VectorPathCommandType::DrawArc:
				{
					fnEnsurePathOpen();

					const VectorPathCommand::DrawArcCommand& drawArc = command.DrawArc;
					const Vector2 origin = drawArc.Center;
					const float radius = drawArc.Radius;
					const NVGwinding winding = drawArc.Direction == VectorGraphicsPathWinding::Clockwise ? NVG_CW : NVG_CCW;

					nvgArc(&context, origin.X, origin.Y, radius, drawArc.StartAngle.GetValueInRadians(), drawArc.EndAngle.GetValueInRadians(), winding);
					break;
				}
			}
		}
	}

	/*** Populates the uniform buffer parameters required for rendering a path element. */
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

	/** Populates the per-view uniform buffer that is shared by all path elements of a single VectorPath object. */
	static void PopulateNVGViewUniformBuffer(const SPtr<render::GpuBuffer>& uniformBuffer, const Area2I& viewRegion)
	{
		GpuBufferMappedScope uniforms = uniformBuffer->Map(GpuMapOption::Write);

		render::gVectorGraphicsViewUniforms.gViewportOffset.Set(uniforms, Vector2(-(float)viewRegion.X, -(float)viewRegion.Y));
		render::gVectorGraphicsViewUniforms.gInverseViewportHalfSize.Set(uniforms, Vector2(1.0f / ((float)viewRegion.Width * 0.5f), 1.0f / ((float)viewRegion.Height * 0.5f)));

		bool viewportYFlip = true;
		const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
		{
			const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
			viewportYFlip = gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down;
		}

		render::gVectorGraphicsViewUniforms.gViewportYFlip.Set(uniforms, viewportYFlip ? -1.0f : 1.0f);
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

	NVGVectorPathRenderable::NVGVectorPathRenderable(const b3d::VectorPath& vectorPath, const VectorGraphicsSettings& settings)
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
		renderCommand.BlendMode = NVGCompositeOperationToBlendMode(compositeOperation);

		auto fnAddFillVertices = [&outputRenderData](const NVGpath& path)
		{
			if(path.nfill <= 2)
				return 0u;

			const u32 vertexCount = (u32)path.nfill;
			const u32 vertexOffset = (u32)outputRenderData.Vertices.size();
			for(u32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				const Vector2 position = Vector2(path.fill[vertexIndex].x, path.fill[vertexIndex].y);
				const Vector2 uv = Vector2(path.fill[vertexIndex].u, path.fill[vertexIndex].v);

				NVGVertex vertex(position, uv);
				outputRenderData.Vertices.push_back(vertex);
			}

			const u32 triangleCount = vertexCount - 2;
			for(u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
			{
				outputRenderData.Indices.push_back(vertexOffset);
				outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 2);
				outputRenderData.Indices.push_back(vertexOffset + triangleIndex + 1);
			}

			return triangleCount;
		};

		auto fnAddStrokeVertices = [&outputRenderData](const NVGpath& path)
		{
			if(path.nstroke <= 2)
				return 0u;

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

			return triangleCount;
		};

		if(isConvex)
		{
			const u32 indexOffset = (u32)outputRenderData.Indices.size();
			u32 triangleCount = 0;
			for(u32 pathIndex = 0; pathIndex < (u32)npaths; ++pathIndex)
			{
				const NVGpath& path = paths[pathIndex];
				triangleCount += fnAddFillVertices(path);
				triangleCount += fnAddStrokeVertices(path);
			}

			outputRenderData.Submeshes.push_back(SubMesh(indexOffset, triangleCount * 3, DOT_TRIANGLE_LIST));
		}
		else
		{
			const u32 fillIndexOffset = (u32)outputRenderData.Indices.size();
			u32 fillTriangleCount = 0;
			for(u32 pathIndex = 0; pathIndex < (u32)npaths; ++pathIndex)
			{
				const NVGpath& path = paths[pathIndex];
				fillTriangleCount += fnAddFillVertices(path);
			}

			outputRenderData.Submeshes.push_back(SubMesh(fillIndexOffset, fillTriangleCount * 3, DOT_TRIANGLE_LIST));
			
			const u32 strokeIndexOffset = (u32)outputRenderData.Indices.size();
			u32 strokeTriangleCount = 0;
			for(u32 pathIndex = 0; pathIndex < (u32)npaths; ++pathIndex)
			{
				const NVGpath& path = paths[pathIndex];
				strokeTriangleCount += fnAddStrokeVertices(path);
			}

			outputRenderData.Submeshes.push_back(SubMesh(strokeIndexOffset, strokeTriangleCount * 3, DOT_TRIANGLE_LIST));

			const Vector2 quadVertexPositions[] = {
				Vector2(bounds[2], bounds[3]),
				Vector2(bounds[2], bounds[1]),
				Vector2(bounds[0], bounds[3]),
				Vector2(bounds[0], bounds[1])
			};

			const u32 quadIndexOffset = (u32)outputRenderData.Indices.size();
			const u32 quadVertexOffset = (u32)outputRenderData.Vertices.size();
			for(u32 vertexIndex = 0; vertexIndex < B3DSize(quadVertexPositions); vertexIndex++)
				outputRenderData.Vertices.push_back(NVGVertex(quadVertexPositions[vertexIndex], Vector2(0.5f, 1.0f)));

			outputRenderData.Indices.push_back(quadVertexOffset);
			outputRenderData.Indices.push_back(quadVertexOffset + 2);
			outputRenderData.Indices.push_back(quadVertexOffset + 1);

			outputRenderData.Indices.push_back(quadVertexOffset + 2);
			outputRenderData.Indices.push_back(quadVertexOffset + 3);
			outputRenderData.Indices.push_back(quadVertexOffset + 1);

			outputRenderData.Submeshes.push_back(SubMesh(quadIndexOffset, 6, DOT_TRIANGLE_LIST));
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

	NVGPathRenderData NVGVectorPathRenderable::PlaybackPathCommands(const b3d::VectorPath& vectorPath, const VectorGraphicsSettings& settings)
	{
		NVGRenderContext userContext;
		userContext.Settings = settings;

		// Scale canvas to output area if requested
		if(settings.ScalingMode != VectorGraphicsRasterizationScaling::None)
		{
			const Size2 canvasSize = vectorPath.GetCanvasSize();
			const Size2 rasterSize = settings.Size;

			const Size2 ratio = rasterSize / canvasSize;

			Size2 scale(1.0f, 1.0f);
			Size2 offset(0.0f, 0.0f);

			switch(settings.ScalingMode)
			{
			case VectorGraphicsRasterizationScaling::StretchToFit:
				{
					scale = ratio;
					break;
				}
			case VectorGraphicsRasterizationScaling::ScaleToFit:
				{
					const float uniformScale = Math::Min(ratio.Width, ratio.Height);
					scale = Size2(uniformScale, uniformScale);

					const Size2 aspectRestrictedScreenSize = Size2(canvasSize.Width * scale.Width, canvasSize.Height * scale.Height);
					offset = (rasterSize - aspectRestrictedScreenSize) * 0.5f;

					break;
				}
			case VectorGraphicsRasterizationScaling::CropToFit:
				{
					const float uniformScale = Math::Max(ratio.Width, ratio.Height);
					scale = Size2(uniformScale, uniformScale);

					const Size2 aspectRestrictedScreenSize = Size2(canvasSize.Width * scale.Width, canvasSize.Height * scale.Height);
					offset = (rasterSize - aspectRestrictedScreenSize) * 0.5f;

					break;
				}
			default:
			case VectorGraphicsRasterizationScaling::None:
				break;
			}

			const Matrix4 scaleTransform =
				Matrix4::Translation(Vector3(canvasSize.Width * 0.5f * scale.Width, canvasSize.Height * 0.5f * scale.Height, 0.0f)) *
				Matrix4::TRS(Vector3(offset.Width, offset.Height, 0.0f), Quaternion::kIdentity, Vector3(scale.Width, scale.Height, 1.0f)) *
				Matrix4::Translation(Vector3(-canvasSize.Width * 0.5f, -canvasSize.Height * 0.5f, 0.0f));

			userContext.Settings.Transform = settings.Transform * scaleTransform;
		}

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

		const Matrix4& transform = userContext.Settings.Transform;
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

		const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if(!gpuDevice)
			return renderBuffers;

		TInlineArray<VertexElement, 2> vertexElements;
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
		GpuBufferUtility::Write(renderBuffers.IndexBuffer, 0, indexBufferSize, mRawRenderData.Indices.data());

		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU;
		vertexBufferCreateInformation.Vertex.ElementSize = renderBuffers.VertexDescription->GetVertexStride();
		vertexBufferCreateInformation.Vertex.Count = vertexCount;

		renderBuffers.VertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

		const u32 vertexBufferSize = renderBuffers.VertexDescription->GetVertexStride() * vertexCount;
		GpuBufferUtility::Write(renderBuffers.VertexBuffer, 0, vertexBufferSize, mRawRenderData.Vertices.data());

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
		B3D_ASSERT(render::gVectorGraphicsRenderUniforms.GetSize() == sizeof(NVGRenderUniforms)); // TODO - I need a way to assign parameter block entries into a particular uniform block, so I don't just do a memcpy (it might not work everywhere)

		NVGRenderUniforms simplePassUniforms;
		B3DZeroOut(simplePassUniforms);
		simplePassUniforms.StrokeThreshold = -1.0f;

		const u32 uniformBlockStride = Math::CeilToMultiple(gVectorGraphicsRenderUniforms.GetSize(), gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);

		GpuBufferMappedScope mapping = renderBuffers.RenderUniformBuffer->Map(GpuMapOption::Write);
		u8* uniformBufferData = (u8*)mapping.GetMappedMemory();

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

		renderBuffers.ViewUniformBuffer = render::gVectorGraphicsViewUniforms.CreateBuffer();
		PopulateNVGViewUniformBuffer(renderBuffers.ViewUniformBuffer, Area2I(0, 0, Math::RoundToU32(mSettings.Size.Width), Math::RoundToU32(mSettings.Size.Height)));

		return renderBuffers;
	}

	SPtr<GpuParameterSet> NVGVectorPathRenderable::Prepare()
	{
		// Cook render buffers if not already done
		if(!mRenderBuffersCooked)
		{
			mRenderBuffers = CookRenderBuffers();
			mRenderBuffersCooked = true;
		}

		// Create or reuse GPU parameters
		if(!mRenderBuffers.GpuParameterSet)
		{
			// Create new GPU parameters object
			mRenderBuffers.GpuParameterSet = VectorGraphicsMaterial::Get()->CreateGpuParameterSet();

			// Set uniform buffers
			mRenderBuffers.GpuParameterSet->SetUniformBuffer("RenderUniforms", mRenderBuffers.RenderUniformBuffer);
			mRenderBuffers.GpuParameterSet->SetUniformBuffer("ViewUniforms", mRenderBuffers.ViewUniformBuffer);
		}

		return mRenderBuffers.GpuParameterSet;
	}

	void NVGVectorPathRenderable::Render(GpuCommandBuffer& commandBuffer)
	{
		// Ensure Prepare() was called first
		B3D_ENSURE(mRenderBuffers.GpuParameterSet != nullptr);

		commandBuffer.BeginLabel("VectorPathRenderable::Render");

		SPtr<GpuBuffer> vertexBuffers[] = { mRenderBuffers.VertexBuffer };
		commandBuffer.SetVertexDescription(mRenderBuffers.VertexDescription);
		commandBuffer.SetVertexBuffers(0, vertexBuffers, 1);
		commandBuffer.SetIndexBuffer(mRenderBuffers.IndexBuffer); // TODO - We shouldn't need one at all actually
		commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

		// Use stored GPU parameters from mRenderBuffers
		const SPtr<GpuParameterSet>& gpuParameterSet = mRenderBuffers.GpuParameterSet;

		const u32 renderUniformBufferDynamicIndex = gpuParameterSet->GetLayout()->GetDynamicOffsetIndex("RenderUniforms");
		B3D_ENSURE(renderUniformBufferDynamicIndex != ~0u);

		commandBuffer.SetGpuParameterSet(gpuParameterSet);

		const u32 setIndex = gpuParameterSet->GetSet();
		const u32 vertexCount = (u32)mRawRenderData.Vertices.size();

		u32 uniformBlockStride = gVectorGraphicsRenderUniforms.GetSize();
		if(const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice())
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
					commandBuffer.SetDynamicBufferOffset(setIndex, renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					render::VectorGraphicsMaterial* const fillShapeStencilMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillShapeStencil, command.BlendMode, mSettings.UseAntialiasing);
					if(B3D_ENSURE(fillShapeStencilMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(fillShapeStencilMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(fillShapeStencilSubmesh.IndexOffset, fillShapeStencilSubmesh.IndexCount, 0, vertexCount, 1);
					}

					const SubMesh& strokeSubmesh = mRawRenderData.Submeshes[submeshIndex++];
					commandBuffer.SetDynamicBufferOffset(setIndex, renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					if(mSettings.UseAntialiasing)
					{
						render::VectorGraphicsMaterial* const fillAAMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillAA, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(fillAAMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(fillAAMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}
					}

					const SubMesh& quadSubmesh = mRawRenderData.Submeshes[submeshIndex++];
					render::VectorGraphicsMaterial* const fillDrawMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillDraw, command.BlendMode, mSettings.UseAntialiasing);
					if(B3D_ENSURE(fillDrawMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(fillDrawMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(quadSubmesh.IndexOffset, quadSubmesh.IndexCount, 0, vertexCount, 1);
					}
				}
				break;
			case NVGRenderCommandType::ConvexFill:
			{
				const SubMesh& fillAndStrokeSubmesh = mRawRenderData.Submeshes[submeshIndex++];
				render::VectorGraphicsMaterial* const simpleFillMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillSimple, command.BlendMode, mSettings.UseAntialiasing);
				if(B3D_ENSURE(simpleFillMaterial))
				{
					commandBuffer.SetDynamicBufferOffset(setIndex, renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					commandBuffer.SetGpuGraphicsPipelineState(simpleFillMaterial->GetGraphicsPipeline());
					commandBuffer.DrawIndexed(fillAndStrokeSubmesh.IndexOffset, fillAndStrokeSubmesh.IndexCount, 0, vertexCount, 1);
				}
			}
				break;
			case NVGRenderCommandType::Stroke:
				{
					const SubMesh& strokeSubmesh = mRawRenderData.Submeshes[submeshIndex++];

					commandBuffer.SetDynamicBufferOffset(setIndex, renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					if(mSettings.StencilStrokes)
					{
						render::VectorGraphicsMaterial* const strokeStencilMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::StrokeStencil, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(strokeStencilMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(strokeStencilMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}

						commandBuffer.SetDynamicBufferOffset(setIndex, renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
						uniformBlockIndex++;

						render::VectorGraphicsMaterial* const strokeAAMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::StrokeAA, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(strokeAAMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(strokeAAMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}

						render::VectorGraphicsMaterial* const clearStencilMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::ClearStencil, command.BlendMode, mSettings.UseAntialiasing);
						if(B3D_ENSURE(clearStencilMaterial))
						{
							commandBuffer.SetGpuGraphicsPipelineState(clearStencilMaterial->GetGraphicsPipeline());
							commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
						}
					}
					else
					{
						render::VectorGraphicsMaterial* const simpleFillMaterial = render::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillSimple, command.BlendMode, mSettings.UseAntialiasing);
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

		commandBuffer.EndLabel();
	}

	RTTIType* NVGVectorPathRenderable::GetRttiStatic()
	{
		return NVGVectorPathRenderableRTTI::Instance();
	}

	RTTIType* NVGVectorPathRenderable::GetRtti() const
	{
		return GetRttiStatic();
	}
}

