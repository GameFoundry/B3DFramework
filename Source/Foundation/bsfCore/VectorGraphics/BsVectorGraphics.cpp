//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVectorGraphics.h"
#include "Mesh/BsMesh.h"

#include <ThirdParty/nanovg.h>

#include "Renderer/BsGpuDataParameterBlock.h"
#include "Renderer/BsRendererMaterial.h"

using namespace bs;

struct NVGVertex
{
	NVGVertex(Vector2 position, Vector2 uv)
		: Position(position), UV(uv)
	{ }

	Vector2 Position;
	Vector2 UV;
};

struct NVGRenderCommand
{
	
};

struct NVGUserContext
{
	VectorShapeSettings Settings;

	Vector<NVGVertex> Vertices;
	Vector<u32> Indices;
	Vector<SubMesh> Submeshes;
	Vector<NVGRenderCommand> RenderCommands;
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
	DesinationAtop,
	Lighter,
	Copy,
	Xor
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
	B3D_PARAM_BLOCK_BEGIN(VectorGraphicsUniformDefinition)
		B3D_PARAM_BLOCK_ENTRY(Matrix4, gScissorMatrix)
		B3D_PARAM_BLOCK_ENTRY(Matrix4, gPaintMatrix)
		B3D_PARAM_BLOCK_ENTRY(Vector4, gInnerColor)
		B3D_PARAM_BLOCK_ENTRY(Vector4, gOuterColor)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gScissorExtents)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gScissorScale)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gExtent)
		B3D_PARAM_BLOCK_ENTRY(float, gRadius)
		B3D_PARAM_BLOCK_ENTRY(float, gFeather)
		B3D_PARAM_BLOCK_ENTRY(float, gStrokeMultiplier)
		B3D_PARAM_BLOCK_ENTRY(float, gStrokeThreshold)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gViewportOffset)
		B3D_PARAM_BLOCK_ENTRY(Vector2, gInverseViewportHalfSize)
		B3D_PARAM_BLOCK_ENTRY(float, gViewportYFlip)
	B3D_PARAM_BLOCK_END

	VectorGraphicsUniformDefinition gVectorGraphicsUniforms;

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


static void NVGRenderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths)
{
	// TODO
}

static void NVGRenderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths)
{
	// TODO
}

static void NVGDrawPath(NVGcontext* nvgContext, const VectorPath& path, const VectorShapeSettings& settings)
{
	// TODO - Draw path commands here

	// TODO - Make sure to add support for scaling (regular and scale 9 grid)
}

// TODO:
//  - Fix up the Render method below so it compiles (add indices, vertex to NVGUserContext)
//  - Add required shaders and their renderer materials
//  - Add code for creating relevant vertex buffers

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

	SPtr<ct::Mesh> mesh;
	if(vertexCount != 0 && indexCount != 0)
	{
		mesh = ct::Mesh::Create(meshData, creationInformation);
	}

	// TODO - Render the mesh
}
