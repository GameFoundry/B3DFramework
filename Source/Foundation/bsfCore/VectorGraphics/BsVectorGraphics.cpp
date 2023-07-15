//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVectorGraphics.h"
#include "Mesh/BsMesh.h"

#include <ThirdParty/nanovg.h>

#include "RenderAPI/BsGpuCommandBuffer.h"
#include "Renderer/BsGpuDataParameterBlock.h"
#include "Renderer/BsRendererMaterial.h"

using namespace bs;

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

struct NVGViewUniforms
{
	Vector2 ViewportOffset;
	Vector2 InverseViewportHalfSize;
	float ViewportYFlip;
};

enum class NVGRenderCommandType
{
	Fill,
	ConvexFill,
	Stroke
};

enum class NVGBlendMode
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

struct NVGRenderCommand
{
	NVGRenderCommandType Type;
	u32 PathCount;
	NVGBlendMode BlendMode;
	NVGRenderUniforms PrimaryPassUniforms;
	Optional<NVGRenderUniforms> SecondaryPassUniforms;
	
};

struct NVGUserContext
{
	VectorShapeSettings Settings;

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
	FillConvex,
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
		template <NVGDrawMode DrawMode, NVGBlendMode BlendMode, bool Antialiasing>
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

		static VectorGraphicsMaterial* GetVariation(NVGDrawMode drawMode, NVGBlendMode blendMode, bool antialiasing);
	};

	void VectorGraphicsMaterial::Initialize()
	{
		// TODO
	}

	void VectorGraphicsMaterial::Execute(const SPtr<Mesh>& mesh, const SubMesh& subMesh)
	{
		// TODO
	}

	VectorGraphicsMaterial* VectorGraphicsMaterial::GetVariation(NVGDrawMode drawMode, NVGBlendMode blendMode, bool antialiasing)
	{
		return Get(ShaderVariationParameters(
			{
				ShaderVariationParameter("DRAW_MODE", (u32)drawMode),
				ShaderVariationParameter("BLEND_MODE", (u32)blendMode),
				ShaderVariationParameter("EDGE_AA", antialiasing),
			}));
	}
}

static NVGBlendMode NVGCompositeOperationToBlendMode(const NVGcompositeOperationState& compositeOperationState)
{
	B3D_ASSERT(compositeOperationState.srcRGB == compositeOperationState.srcAlpha);
	B3D_ASSERT(compositeOperationState.dstRGB == compositeOperationState.dstAlpha);

	if(compositeOperationState.srcRGB == NVG_ONE && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
		return NVGBlendMode::SourceOver;
	else if(compositeOperationState.srcRGB == NVG_DST_ALPHA && compositeOperationState.dstRGB == NVG_ZERO)
		return NVGBlendMode::SourceIn;
	else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_ZERO)
		return NVGBlendMode::SourceOut;
	else if(compositeOperationState.srcRGB == NVG_DST_ALPHA && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
		return NVGBlendMode::Atop;
	else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_ONE)
		return NVGBlendMode::DestinationOver;
	else if(compositeOperationState.srcRGB == NVG_ZERO && compositeOperationState.dstRGB == NVG_SRC_ALPHA)
		return NVGBlendMode::DestinationIn;
	else if(compositeOperationState.srcRGB == NVG_ZERO && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
		return NVGBlendMode::DestinationOut;
	else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_SRC_ALPHA)
		return NVGBlendMode::DestinationAtop;
	else if(compositeOperationState.srcRGB == NVG_ONE && compositeOperationState.dstRGB == NVG_ONE)
		return NVGBlendMode::Lighter;
	else if(compositeOperationState.srcRGB == NVG_ONE && compositeOperationState.dstRGB == NVG_ZERO)
		return NVGBlendMode::Copy;
	else if(compositeOperationState.srcRGB == NVG_ONE_MINUS_DST_ALPHA && compositeOperationState.dstRGB == NVG_ONE_MINUS_SRC_ALPHA)
		return NVGBlendMode::Xor;

	B3D_ENSURE(false);
	return NVGBlendMode::SourceOver;
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

static NVGViewUniforms CreateNVGViewUniformParameters(const Rect2I& viewRegion)
{
	NVGViewUniforms uniformParameters;

	uniformParameters.ViewportOffset = Vector2(-(float)viewRegion.X, -(float)viewRegion.Y);
	uniformParameters.InverseViewportHalfSize = Vector2(1.0f / ((float)viewRegion.Width * 0.5f), 1.0f / ((float)viewRegion.Height * 0.5f));

	bool viewportYFlip = true;
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(gpuDevice != nullptr)
	{
		const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
		viewportYFlip = gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down;
	}

	uniformParameters.ViewportYFlip = viewportYFlip ? -1.0f : 1.0f;

	return uniformParameters;
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

static void PlaybackNVGRenderCommands(const NVGUserContext& userContext)
{
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

	const SPtr<ct::Mesh> mesh = ct::Mesh::Create(meshData, creationInformation);

	u32 uniformBlockCount = 0;
	for(const auto& command : userContext.RenderCommands)
	{
		switch(command.Type)
		{
		case NVGRenderCommandType::Fill:
			uniformBlockCount++;
			break;
		case NVGRenderCommandType::ConvexFill:
			uniformBlockCount += 2;
			break;
		case NVGRenderCommandType::Stroke:
			uniformBlockCount += userContext.Settings.StencilStrokes ? 2 : 1;
			break;
		}
	}

	// Create uniform buffer
	const SPtr<ct::GpuBuffer> uniformBuffer = ct::gVectorGraphicsRenderUniforms.CreateBuffer(uniformBlockCount);
	B3D_ASSERT(ct::gVectorGraphicsRenderUniforms.GetSize() == sizeof(NVGRenderUniforms)); // TODO - I need a way to assign parameter block entries into a particular uniform block, so I don't just do a memcpy (it might not work everywhere)

	NVGRenderUniforms simplePassUniforms;
	B3DZeroOut(simplePassUniforms);
	simplePassUniforms.StrokeThreshold = -1.0f;

	u32 uniformBlockStride = ct::gVectorGraphicsRenderUniforms.GetSize();
	if(const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice())
		uniformBlockStride = Math::CeilToMultiple(uniformBlockStride, gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);

	u8* uniformBufferData = (u8*)uniformBuffer->Lock(GBL_WRITE_ONLY_DISCARD);

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
			memcpy(uniformBufferData, &command.PrimaryPassUniforms, sizeof(command.PrimaryPassUniforms));
			uniformBufferData += uniformBlockStride;

			if(userContext.Settings.StencilStrokes && B3D_ENSURE(command.SecondaryPassUniforms.has_value()))
			{
				memcpy(uniformBufferData, &command.SecondaryPassUniforms.value(), sizeof(command.SecondaryPassUniforms.value()));
				uniformBufferData += uniformBlockStride;
			}

			break;
		}
	}

	uniformBuffer->Unlock();

	//// Execute draw commands
	//u32 uniformBlockIndex = 0;
	//u32 submeshIndex = 0;
	//for(const auto& command : userContext.RenderCommands)
	//{
	//	switch(command.Type)
	//	{
	//	case NVGRenderCommandType::Fill:
	//		commandBuffer.SetGpuParameters()


	//		memcpy(uniformBufferData, &simplePassUniforms, sizeof(simplePassUniforms));
	//		uniformBufferData += uniformBlockStride;

	//		// Fallthrough
	//	case NVGRenderCommandType::ConvexFill:
	//		memcpy(uniformBufferData, &command.PrimaryPassUniforms, sizeof(command.PrimaryPassUniforms));
	//		uniformBufferData += uniformBlockStride;
	//		break;
	//	case NVGRenderCommandType::Stroke:
	//		memcpy(uniformBufferData, &command.PrimaryPassUniforms, sizeof(command.PrimaryPassUniforms));
	//		uniformBufferData += uniformBlockStride;

	//		if(userContext.Settings.StencilStrokes && B3D_ENSURE(command.SecondaryPassUniforms.has_value()))
	//		{
	//			memcpy(uniformBufferData, &command.SecondaryPassUniforms.value(), sizeof(command.SecondaryPassUniforms.value()));
	//			uniformBufferData += uniformBlockStride;
	//		}

	//		break;
	//	}
	//}
}

static void NVGDrawPath(NVGcontext* nvgContext, const VectorPath& path, const VectorShapeSettings& settings)
{
	// TODO - Draw path commands here

	// TODO - Make sure to add support for scaling (regular and scale 9 grid)
}

void VectorGraphics::Render(const VectorPath& path, const VectorShapeSettings& settings)
{
	NVGUserContext userContext;
	userContext.Settings = settings;

	NVGparams nvgParameters;
	B3DZeroOut(nvgParameters);

	nvgParameters.userPtr = &userContext;
	nvgParameters.renderCreate = [](void* uptr) { return 1; };
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
	PlaybackNVGRenderCommands(userContext);
}
