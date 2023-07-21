//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVectorGraphics.h"
#include "Mesh/BsMesh.h"

#include <ThirdParty/nanovg.h>

#include "RenderAPI/BsGpuCommandBuffer.h"
#include "RenderAPI/BsGpuPipelineParameterLayout.h"
#include "Renderer/BsGpuDataParameterBlock.h"
#include "Renderer/BsRendererMaterial.h"
#include "Private/RTTI/BsVectorGraphicsRTTI.h"

using namespace bs;

namespace bs
{
	VectorPath& VectorPath::SetDrawCursor(const Vector2& cursor)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::SetDrawCursor;
		command.SetDrawCursor.Position = cursor;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::SetWinding(VectorGraphicsPathWinding winding)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::SetPathWinding;
		command.SetPathWinding.Winding = winding;

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

	VectorPath& VectorPath::DrawRectangle(const Rect2& area)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawRectangle;
		command.DrawRectangle.Area = area;

		mCommands.push_back(command);
		return *this;
	}

	VectorPath& VectorPath::DrawRoundedRectangle(const Rect2& area, float cornerRadius)
	{
		return DrawRoundedRectangle(area, cornerRadius, cornerRadius, cornerRadius, cornerRadius);
	}

	VectorPath& VectorPath::DrawRoundedRectangle(const Rect2& area, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius)
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

	VectorPath& VectorPath::DrawArc(const Vector2& origin, float radius, Radian startAngle, Radian endAngle, VectorGraphicsPathWinding direction)
	{
		VectorPathCommand command;
		command.Type = VectorPathCommandType::DrawArc;
		command.DrawArc.Origin = origin;
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

	VectorPath& VectorPath::SetScissorRectangle(const Rect2& scissorArea)
	{
		mCurrentState.ScissorArea = scissorArea;
		return *this;
	}

	VectorPath& VectorPath::ClearScissor()
	{
		mCurrentState.ScissorArea = Rect2::kEmpty;
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

} // namespace bs

struct NVGVertex
{
	NVGVertex() = default;
	NVGVertex(Vector2 position, Vector2 uv)
		: Position(position), UV(uv)
	{ }

	Vector2 Position;
	Vector2 UV;
};

struct NVGRenderUniforms
{
	Matrix4 ScissorMatrix;
	Matrix4 PaintMatrix;
	Color InnerColor;
	Color OuterColor;
	Vector2 ScissorExtents;
	Vector2 ScissorScale;
	Vector2 Extent;
	float Radius;
	float Feather;
	float StrokeMultiplier;
	float StrokeThreshold;
};

enum class NVGRenderCommandType
{
	Fill,
	ConvexFill,
	Stroke
};

struct NVGRenderCommand
{
	NVGRenderCommandType Type;
	u32 PathCount;
	VectorGraphicsBlendMode BlendMode;
	NVGRenderUniforms PrimaryPassUniforms;
	Optional<NVGRenderUniforms> SecondaryPassUniforms;
	
};

struct NVGUserContext
{
	VectorGraphicsSettings Settings;

	// Note: All of these should be serializable so I can cache this information without having to re-play all the commands
	Vector<NVGVertex> Vertices;
	Vector<u32> Indices;
	Vector<SubMesh> Submeshes;
	Vector<NVGRenderCommand> RenderCommands;
};

enum class NVGDrawMode
{
	FillShapeStencil,
	FillAA,
	FillDraw,
	StrokeStencil,
	StrokeAA,
	ClearStencil,
	FillSimple,
};

namespace bs::ct
{
	B3D_PARAM_BLOCK_BEGIN(VectorGraphicsRenderUniformDefinition)
		B3D_PARAM_BLOCK_ENTRY(Matrix4, gScissorMatrix)
		B3D_PARAM_BLOCK_ENTRY(Matrix4, gPaintMatrix)
		B3D_PARAM_BLOCK_ENTRY(Color, gInnerColor)
		B3D_PARAM_BLOCK_ENTRY(Color, gOuterColor)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gScissorExtents)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gScissorScale)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gExtent)
		B3D_PARAM_BLOCK_ENTRY(float, gRadius)
		B3D_PARAM_BLOCK_ENTRY(float, gFeather)
		B3D_PARAM_BLOCK_ENTRY(float, gStrokeMultiplier)
		B3D_PARAM_BLOCK_ENTRY(float, gStrokeThreshold)
	B3D_PARAM_BLOCK_END

	VectorGraphicsRenderUniformDefinition gVectorGraphicsRenderUniforms;

	B3D_PARAM_BLOCK_BEGIN(VectorGraphicsViewUniformDefinition)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gViewportOffset)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gInverseViewportHalfSize)
		B3D_PARAM_BLOCK_ENTRY(float, gViewportYFlip)
	B3D_PARAM_BLOCK_END

	VectorGraphicsViewUniformDefinition gVectorGraphicsViewUniforms;

	class VectorGraphicsMaterial : public RendererMaterial<VectorGraphicsMaterial>
	{
		RMAT_DEF("VectorGraphics.bsl");

		/** Helper method used for initializing variations of this material. */
		template <NVGDrawMode DrawMode, VectorGraphicsBlendMode BlendMode, bool Antialiasing>
		static const ShaderVariationParameters& GetVariation()
		{
			static ShaderVariationParameters variation = ShaderVariationParameters(
			{
					ShaderVariationParameter("DRAW_MODE", (u32)DrawMode),
					ShaderVariationParameter("BLEND_MODE", (u32)BlendMode),
					ShaderVariationParameter("EDGE_AA", Antialiasing),
				});

			return variation;
		}

	public:
		VectorGraphicsMaterial() = default;
		void Initialize() override;

		void Execute(const SPtr<Mesh>& mesh, const SubMesh& subMesh);

		static VectorGraphicsMaterial* GetVariation(NVGDrawMode drawMode, VectorGraphicsBlendMode blendMode, bool antialiasing);
	};

	void VectorGraphicsMaterial::Initialize()
	{
		// TODO
	}

	void VectorGraphicsMaterial::Execute(const SPtr<Mesh>& mesh, const SubMesh& subMesh)
	{
		// TODO
	}

	VectorGraphicsMaterial* VectorGraphicsMaterial::GetVariation(NVGDrawMode drawMode, VectorGraphicsBlendMode blendMode, bool antialiasing)
	{
		return Get(ShaderVariationParameters(
			{
				ShaderVariationParameter("DRAW_MODE", (u32)drawMode),
				ShaderVariationParameter("BLEND_MODE", (u32)blendMode),
				ShaderVariationParameter("EDGE_AA", antialiasing),
			}));
	}
}

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

static void PopulateViewUniformBuffer(const SPtr<ct::GpuBuffer>& uniformBuffer, const Rect2I& viewRegion)
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

static void NVGRenderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths)
{
	if(npaths == 0)
		return;

	NVGUserContext& userContext = *(NVGUserContext*)uptr;
	const bool isConvex = npaths == 1 && paths[0].convex;

	NVGRenderCommand renderCommand;
	renderCommand.Type = isConvex ? NVGRenderCommandType::ConvexFill : NVGRenderCommandType::Fill;
	renderCommand.PathCount = npaths;
	renderCommand.BlendMode = NVGCompositeOperationToBlendMode(compositeOperation);

	u32 fillIndexOffset = (u32)userContext.Indices.size();
	u32 fillTriangleCount = 0;
	for(u32 pathIndex = 0; pathIndex < (u32)npaths; ++pathIndex)
	{
		const NVGpath& path = paths[pathIndex];
		if(path.nfill > 2)
		{
			const u32 vertexCount = (u32)path.nfill;
			const u32 vertexOffset = (u32)userContext.Vertices.size();
			for(u32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				const Vector2 position = Vector2(path.fill[vertexIndex].x, path.fill[vertexIndex].y);
				const Vector2 uv = Vector2(path.fill[vertexIndex].u, path.fill[vertexIndex].v);

				NVGVertex vertex(position, uv);
				userContext.Vertices.push_back(vertex);
			}

			const u32 indexOffset = (u32)userContext.Indices.size();
			const u32 triangleCount = vertexCount - 2;
			for(u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
			{
				userContext.Indices.push_back(vertexOffset);
				userContext.Indices.push_back(vertexOffset + triangleIndex + 1);
				userContext.Indices.push_back(vertexOffset + triangleIndex + 2);
			}

			fillTriangleCount += triangleCount;

			if(isConvex)
				userContext.Submeshes.push_back(SubMesh(indexOffset, triangleCount * 3, DOT_TRIANGLE_LIST));
		}
	}

	u32 strokeIndexOffset = (u32)userContext.Indices.size();
	u32 strokeTriangleCount = 0;
	for(int pathIndex = 0; pathIndex < npaths; ++pathIndex)
	{
		const NVGpath& path = paths[pathIndex];
		if(path.nstroke > 2)
		{
			const u32 vertexCount = (u32)path.nstroke;
			const u32 vertexOffset = (u32)userContext.Vertices.size();
			for(u32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				const Vector2 position = Vector2(path.stroke[vertexIndex].x, path.stroke[vertexIndex].y);
				const Vector2 uv = Vector2(path.stroke[vertexIndex].u, path.stroke[vertexIndex].v);

				const NVGVertex vertex(position, uv);
				userContext.Vertices.push_back(vertex);
			}

			const u32 indexOffset = (u32)userContext.Indices.size();
			const u32 triangleCount = vertexCount - 2;
			for(u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
			{
				if(triangleIndex % 2 == 0)
				{
					userContext.Indices.push_back(vertexOffset + triangleIndex + 0);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 1);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 2);
				}
				else
				{
					userContext.Indices.push_back(vertexOffset + triangleIndex + 1);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 0);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 2);
				}
			}

			strokeTriangleCount += triangleCount;

			if(isConvex)
				userContext.Submeshes.push_back(SubMesh(indexOffset, triangleCount * 3, DOT_TRIANGLE_LIST));
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

		const u32 indexOffset = (u32)userContext.Indices.size();
		const u32 vertexOffset = (u32)userContext.Vertices.size();
		for(u32 vertexIndex = 0; vertexIndex < B3DSize(quadVertexPositions); vertexIndex++)
			userContext.Vertices.push_back(NVGVertex(quadVertexPositions[vertexIndex], Vector2(0.5f, 1.0f)));

		userContext.Indices.push_back(vertexOffset);
		userContext.Indices.push_back(vertexOffset + 1);
		userContext.Indices.push_back(vertexOffset + 2);

		userContext.Indices.push_back(vertexOffset + 2);
		userContext.Indices.push_back(vertexOffset + 1);
		userContext.Indices.push_back(vertexOffset + 3);

		userContext.Submeshes.push_back(SubMesh(fillIndexOffset, fillTriangleCount * 3, DOT_TRIANGLE_LIST));
		userContext.Submeshes.push_back(SubMesh(strokeIndexOffset, strokeTriangleCount * 3, DOT_TRIANGLE_LIST));
		userContext.Submeshes.push_back(SubMesh(indexOffset, 6, DOT_TRIANGLE_LIST));
	}

	renderCommand.PrimaryPassUniforms = CreateNVGRenderUniformParameters(paint, scissor, fringe, fringe, -1.0f);
	userContext.RenderCommands.push_back(renderCommand);
}

static void NVGRenderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths)
{
	if(npaths == 0)
		return;

	NVGUserContext& userContext = *(NVGUserContext*)uptr;

	NVGRenderCommand renderCommand;
	renderCommand.Type = NVGRenderCommandType::Stroke;
	renderCommand.PathCount = npaths;
	renderCommand.BlendMode = NVGCompositeOperationToBlendMode(compositeOperation);

	// Note: Duplicated code from NVGRenderFill
	u32 strokeIndexOffset = (u32)userContext.Indices.size();
	u32 strokeTriangleCount = 0;
	for(int pathIndex = 0; pathIndex < npaths; ++pathIndex)
	{
		const NVGpath& path = paths[pathIndex];
		if(path.nstroke > 2)
		{
			const u32 vertexCount = (u32)path.nstroke;
			const u32 vertexOffset = (u32)userContext.Vertices.size();
			for(u32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				const Vector2 position = Vector2(path.stroke[vertexIndex].x, path.stroke[vertexIndex].y);
				const Vector2 uv = Vector2(path.stroke[vertexIndex].u, path.stroke[vertexIndex].v);

				const NVGVertex vertex(position, uv);
				userContext.Vertices.push_back(vertex);
			}

			const u32 triangleCount = vertexCount - 2;
			for(u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
			{
				if(triangleIndex % 2 == 0)
				{
					userContext.Indices.push_back(vertexOffset + triangleIndex + 0);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 1);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 2);
				}
				else
				{
					userContext.Indices.push_back(vertexOffset + triangleIndex + 1);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 0);
					userContext.Indices.push_back(vertexOffset + triangleIndex + 2);
				}
			}

			strokeTriangleCount += triangleCount;
		}
	}

	userContext.Submeshes.push_back(SubMesh(strokeIndexOffset, strokeTriangleCount * 3, DOT_TRIANGLE_LIST));

	renderCommand.PrimaryPassUniforms = CreateNVGRenderUniformParameters(paint, scissor, fringe, strokeWidth, -1.0f);
	if(userContext.Settings.StencilStrokes)
		renderCommand.SecondaryPassUniforms = CreateNVGRenderUniformParameters(paint, scissor, fringe, strokeWidth, 1.0f - 0.5f / 255.0f);

	userContext.RenderCommands.push_back(renderCommand);
}

static void PlaybackNVGRenderCommands(const NVGUserContext& userContext, ct::GpuCommandBuffer& commandBuffer)
{
	const VectorGraphicsSettings& settings = userContext.Settings;

	// TODO - Mesh and uniform buffers should be cached

	// Create mesh
	const u32 vertexCount = (u32)userContext.Vertices.size();
	const u32 indexCount = (u32)userContext.Indices.size();

	SmallVector<VertexElement, 2> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT2, VES_POSITION));
	vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));

	const SPtr<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(vertexElements);
	B3D_ASSERT(vertexDescription->GetVertexStride() == sizeof(NVGVertex));

	const SPtr<MeshData> meshData = MeshData::Create(vertexCount, indexCount, vertexDescription);

	NVGVertex* const vertices = (NVGVertex*)meshData->GetStreamData(0);
	memcpy(vertices, userContext.Vertices.data(), vertexCount * sizeof(NVGVertex));

	u32* const indices = meshData->GetIndices32();
	memcpy(indices, userContext.Indices.data(), indexCount * sizeof(u32));

	MeshCreateInformation creationInformation;
	creationInformation.VertexCount = meshData->GetVertexCount();
	creationInformation.IndexCount = meshData->GetIndexCount();
	creationInformation.VertexDescription = meshData->GetVertexDescription();
	creationInformation.SubMeshes = userContext.Submeshes;

	if(vertexCount == 0 || indexCount == 0)
		return;

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return;

	GpuBufferInformation indexBufferCreateInformation;
	indexBufferCreateInformation.Type = GpuBufferType::Index;
	indexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU;
	indexBufferCreateInformation.Index.Type = IT_32BIT;
	indexBufferCreateInformation.Index.Count = indexCount;

	const SPtr<ct::GpuBuffer> indexBuffer = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

	GpuBufferCreateInformation vertexBufferCreateInformation;
	vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
	vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU;
	vertexBufferCreateInformation.Vertex.ElementSize = vertexDescription->GetVertexStride();
	vertexBufferCreateInformation.Vertex.Count = vertexCount;

	const SPtr<ct::GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

	u32 uniformBlockCount = 0;
	for(const auto& command : userContext.RenderCommands)
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
			uniformBlockCount += settings.StencilStrokes ? 2 : 1;
			break;
		}
	}

	// Create uniform buffers
	const SPtr<ct::GpuBuffer> renderUniformBuffer = ct::gVectorGraphicsRenderUniforms.CreateBuffer(uniformBlockCount);
	B3D_ASSERT(ct::gVectorGraphicsRenderUniforms.GetSize() == sizeof(NVGRenderUniforms)); // TODO - I need a way to assign parameter block entries into a particular uniform block, so I don't just do a memcpy (it might not work everywhere)

	NVGRenderUniforms simplePassUniforms;
	B3DZeroOut(simplePassUniforms);
	simplePassUniforms.StrokeThreshold = -1.0f;

	u32 uniformBlockStride = ct::gVectorGraphicsRenderUniforms.GetSize();
	if(const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice())
		uniformBlockStride = Math::CeilToMultiple(uniformBlockStride, gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);

	u8* uniformBufferData = (u8*)renderUniformBuffer->Lock(GBL_WRITE_ONLY_DISCARD);

	for(const auto& command : userContext.RenderCommands)
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
			if(settings.StencilStrokes && B3D_ENSURE(command.SecondaryPassUniforms.has_value()))
			{
				memcpy(uniformBufferData, &command.SecondaryPassUniforms.value(), sizeof(command.SecondaryPassUniforms.value()));
				uniformBufferData += uniformBlockStride;
			}

			memcpy(uniformBufferData, &command.PrimaryPassUniforms, sizeof(command.PrimaryPassUniforms));
			uniformBufferData += uniformBlockStride;

			break;
		}
	}

	renderUniformBuffer->Unlock();

	const SPtr<ct::GpuBuffer> viewUniformBuffer = ct::gVectorGraphicsViewUniforms.CreateBuffer();
	PopulateViewUniformBuffer(viewUniformBuffer, Rect2I(0, 0, settings.Size.Width, settings.Size.Height));

	SPtr<ct::GpuBuffer> vertexBuffers[] = { vertexBuffer };
	commandBuffer.SetVertexBuffers(0, vertexBuffers, 1);
	commandBuffer.SetIndexBuffer(indexBuffer); // TODO - We shouldn't need one at all actually
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	// Note: The parameter layout for all variations must match
	const SPtr<ct::GpuParameters> gpuParameters = ct::VectorGraphicsMaterial::Get()->GetParams();
	gpuParameters->SetUniformBuffer("RenderUniforms", renderUniformBuffer);
	gpuParameters->SetUniformBuffer("ViewUniforms", viewUniformBuffer);

	const u32 renderUniformBufferDynamicIndex = gpuParameters->GetPipelineParameterInformation()->GetDynamicOffsetIndex(GPT_FRAGMENT_PROGRAM, "RenderUniforms");
	B3D_ENSURE(renderUniformBufferDynamicIndex != ~0u);

	commandBuffer.SetGpuParameters(gpuParameters);

	// Execute draw commands
	u32 uniformBlockIndex = 0;
	u32 submeshIndex = 0;
	for(const auto& command : userContext.RenderCommands)
	{

		switch(command.Type)
		{
		case NVGRenderCommandType::Fill:
			{
				const SubMesh& fillShapeStencilSubmesh = userContext.Submeshes[submeshIndex++];
				commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
				uniformBlockIndex++;

				ct::VectorGraphicsMaterial* const fillShapeStencilMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillShapeStencil, command.BlendMode, settings.UseAntialiasing);
				if(B3D_ENSURE(fillShapeStencilMaterial))
				{
					commandBuffer.SetGpuGraphicsPipelineState(fillShapeStencilMaterial->GetGraphicsPipeline());
					commandBuffer.DrawIndexed(fillShapeStencilSubmesh.IndexOffset, fillShapeStencilSubmesh.IndexCount, 0, vertexCount, 1);
				}

				const SubMesh& strokeSubmesh = userContext.Submeshes[submeshIndex++];
				commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
				uniformBlockIndex++;

				if(settings.UseAntialiasing)
				{
					ct::VectorGraphicsMaterial* const fillAAMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillAA, command.BlendMode, settings.UseAntialiasing);
					if(B3D_ENSURE(fillAAMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(fillAAMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
					}
				}

				const SubMesh& quadSubmesh = userContext.Submeshes[submeshIndex++];
				ct::VectorGraphicsMaterial* const fillDrawMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillDraw, command.BlendMode, settings.UseAntialiasing);
				if(B3D_ENSURE(fillDrawMaterial))
				{
					commandBuffer.SetGpuGraphicsPipelineState(fillDrawMaterial->GetGraphicsPipeline());
					commandBuffer.DrawIndexed(quadSubmesh.IndexOffset, quadSubmesh.IndexCount, 0, vertexCount, 1);
				}
			}
			break;
		case NVGRenderCommandType::ConvexFill:
		{
			ct::VectorGraphicsMaterial* const simpleFillMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillSimple, command.BlendMode, settings.UseAntialiasing);
			if(B3D_ENSURE(simpleFillMaterial))
			{
				commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
				uniformBlockIndex++;

				commandBuffer.SetGpuGraphicsPipelineState(simpleFillMaterial->GetGraphicsPipeline());
				for(u32 pathIndex = 0; pathIndex < command.PathCount; pathIndex++)
				{
					// TODO - No need for multiple draw calls here, I can just intertwine these in a single buffer
					const SubMesh& fillSubmesh = userContext.Submeshes[submeshIndex++];
					commandBuffer.DrawIndexed(fillSubmesh.IndexOffset, fillSubmesh.IndexCount, 0, vertexCount, 1);

					const SubMesh& strokeSubmesh = userContext.Submeshes[submeshIndex++];
					commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
				}
			}
		}
			break;
		case NVGRenderCommandType::Stroke:
			{
				const SubMesh& strokeSubmesh = userContext.Submeshes[submeshIndex++];

				commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
				uniformBlockIndex++;

				if(settings.StencilStrokes)
				{
					ct::VectorGraphicsMaterial* const strokeStencilMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::StrokeStencil, command.BlendMode, settings.UseAntialiasing);
					if(B3D_ENSURE(strokeStencilMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(strokeStencilMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
					}

					commandBuffer.SetDynamicBufferOffset(renderUniformBufferDynamicIndex, uniformBlockIndex * uniformBlockStride);
					uniformBlockIndex++;

					ct::VectorGraphicsMaterial* const strokeAAMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::StrokeAA, command.BlendMode, settings.UseAntialiasing);
					if(B3D_ENSURE(strokeAAMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(strokeAAMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
					}

					ct::VectorGraphicsMaterial* const clearStencilMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::ClearStencil, command.BlendMode, settings.UseAntialiasing);
					if(B3D_ENSURE(clearStencilMaterial))
					{
						commandBuffer.SetGpuGraphicsPipelineState(clearStencilMaterial->GetGraphicsPipeline());
						commandBuffer.DrawIndexed(strokeSubmesh.IndexOffset, strokeSubmesh.IndexCount, 0, vertexCount, 1);
					}
				}
				else
				{
					ct::VectorGraphicsMaterial* const simpleFillMaterial = ct::VectorGraphicsMaterial::GetVariation(NVGDrawMode::FillSimple, command.BlendMode, settings.UseAntialiasing);
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

static void NVGDrawPath(NVGcontext* nvgContext, const VectorPath& path, const VectorGraphicsSettings& settings)
{
	// TODO - Draw path commands here

	// TODO - Make sure to add support for scaling (regular and scale 9 grid)
}

void VectorGraphics::Render(const VectorPath& path, const VectorGraphicsSettings& settings)
{
	NVGUserContext userContext;
	userContext.Settings = settings;

	NVGparams nvgParameters;
	B3DZeroOut(nvgParameters);

	nvgParameters.userPtr = &userContext;
	nvgParameters.renderCreate = [](void* uptr)
	{ return 1; };
	nvgParameters.renderViewport = [](void* uptr, float width, float height, float devicePixelRatio) {};
	nvgParameters.renderCancel = [](void* uptr) {};
	nvgParameters.renderFlush = [](void* uptr) {};
	nvgParameters.renderFill = NVGRenderFill;
	nvgParameters.renderStroke = NVGRenderStroke;
	nvgParameters.renderDelete = [](void* uptr) {};
	nvgParameters.edgeAntiAlias = settings.UseAntialiasing;

	NVGcontext* const nvgContext = nvgCreateInternal(&nvgParameters);

	nvgBeginFrame(nvgContext, (float)settings.Size.Width, (float)settings.Size.Height, settings.DevicePixelRatio);

	const Matrix4& transform = settings.Transform;
	const Vector3 translation = transform.GetTranslation();
	nvgTranslate(nvgContext, translation.X, translation.Y);
	nvgTransform(nvgContext, transform[0][0], transform[1][0], transform[0][1], transform[1][1], transform[0][2], transform[1][2]);

	NVGDrawPath(nvgContext, path, settings);

	nvgEndFrame(nvgContext);
	nvgDeleteInternal(nvgContext);

	// TODO - This should be split off to be done in a separate step. And everything in userContext should be serializable and cacheable. Additionally the gpu buffers created from playback should be runtime cacheable as well.
	//PlaybackNVGRenderCommands(userContext);
}
