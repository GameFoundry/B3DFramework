//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"
#include "Math/BsRect2.h"
#include "Math/BsVector2I.h"
#include "Math/BsRect2I.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsParamBlocks.h"

namespace bs { namespace ct
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/** Shader that copies a source texture into a render target, and optionally resolves it. */
	class BlitMat : public RendererMaterial<BlitMat>
	{
		RMAT_DEF("Blit.bsl");

		/** Helper method used for initializing variations of this material. */
		template<UINT32 msaa, bool color>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			SmallVector<ShaderVariation::Param, 4>({
				ShaderVariation::Param("MSAA_COUNT", msaa),
				ShaderVariation::Param("COLOR", color),
			}));

			return variation;
		}
	public:
		BlitMat();

		/** Executes the material on the currently bound render target, copying from @p source. */
		void execute(const SPtr<Texture>& source, const Rect2& area, bool flipUV);

		/** 
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param	msaaCount		Number of MSAA samples in the input texture. If larger than 1 the texture will be resolved
		 *							before written to the destination.
		 * @param	isColor			If true the input is assumed to be a 4-component color texture. If false it is assumed
		 *							the input is a 1-component depth texture. This controls how is the texture resolve and is
		 *							only relevant if @p msaaCount > 1. Color texture MSAA samples will be averaged, while for
		 *							depth textures the minimum of all samples will be used.
		 */
		static BlitMat* getVariation(UINT32 msaaCount, bool isColor);
	private:
		GpuParamTexture mSource;
	};

	BS_PARAM_BLOCK_BEGIN(ClearParamDef)
		BS_PARAM_BLOCK_ENTRY(INT32, gClearValue)
	BS_PARAM_BLOCK_END

	extern ClearParamDef gClearParamDef;

	/** Shader that clears the currently bound render target to an integer value. */
	class ClearMat : public RendererMaterial<ClearMat>
	{
		RMAT_DEF("Clear.bsl");

	public:
		ClearMat();

		/** Executes the material on the currently bound render target, clearing to to @p value. */
		void execute(UINT32 value);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
	};

	/**
	 * Contains various utility methods that make various common operations in the renderer easier.
	 * 			
	 * @note	Core thread only.
	 */
	class BS_EXPORT RendererUtility : public Module<RendererUtility>
	{
	public:
		RendererUtility();
		~RendererUtility() = default;

		/**
		 * Activates the specified material pass for rendering. Any further draw calls will be executed using this pass.
		 *
		 * @param[in]	material		Material containing the pass.
		 * @param[in]	passIdx			Index of the pass in the material.
		 * @param[in]	techniqueIdx	Index of the technique the pass belongs to, if the material has multiple techniques.
		 *
		 * @note	Core thread.
		 */
		void setPass(const SPtr<Material>& material, UINT32 passIdx = 0, UINT32 techniqueIdx = 0);

		/**
		 * Activates the specified material pass for compute. Any further dispatch calls will be executed using this pass.
		 *
		 * @param[in]	material		Material containing the pass.
		 * @param[in]	passIdx			Index of the pass in the material.
		 *
		 * @note	Core thread.
		 */
		void setComputePass(const SPtr<Material>& material, UINT32 passIdx = 0);

		/**
		 * Sets parameters (textures, samplers, buffers) for the currently active pass.
		 *
		 * @param[in]	params		Object containing the parameters.
		 * @param[in]	passIdx		Pass for which to set the parameters.
		 *					
		 * @note	Core thread.
		 */
		void setPassParams(const SPtr<GpuParamsSet>& params, UINT32 passIdx = 0);

		/**
		 * Draws the specified mesh.
		 *
		 * @param[in]	mesh			Mesh to draw.
		 * @param[in]	numInstances	Number of times to draw the mesh using instanced rendering.
		 *
		 * @note	Core thread.
		 */
		void draw(const SPtr<MeshBase>& mesh, UINT32 numInstances = 1);

		/**
		 * Draws the specified mesh.
		 *
		 * @param[in]	mesh			Mesh to draw.
		 * @param[in]	subMesh			Portion of the mesh to draw.
		 * @param[in]	numInstances	Number of times to draw the mesh using instanced rendering.
		 *
		 * @note	Core thread.
		 */
		void draw(const SPtr<MeshBase>& mesh, const SubMesh& subMesh, UINT32 numInstances = 1);

		/**
		 * Draws the specified mesh with an additional vertex buffer containing morph shape vertices.
		 *
		 * @param[in]	mesh					Mesh to draw.
		 * @param[in]	subMesh					Portion of the mesh to draw.
		 * @param[in]	morphVertices			Buffer containing the morph shape vertices. Will be bound to stream 1. 
		 *										Expected to contain the same number of vertices as the source mesh.
		 * @param[in]	morphVertexDeclaration	Vertex declaration describing vertices of the provided mesh and the vertices
		 *										provided in the morph vertex buffer.
		 *
		 * @note	Core thread.
		 */
		void drawMorph(const SPtr<MeshBase>& mesh, const SubMesh& subMesh, const SPtr<VertexBuffer>& morphVertices, 
			const SPtr<VertexDeclaration>& morphVertexDeclaration);

		/**
		 * Blits contents of the provided texture into the currently bound render target. If the provided texture contains
		 * multiple samples, they will be resolved.
		 *
		 * @param[in]	texture	Source texture to blit.
		 * @param[in]	area	Area of the source texture to blit in pixels. If width or height is zero it is assumed
		 *						the entire texture should be blitted.
		 * @param[in]	flipUV	If true, vertical UV coordinate will be flipped upside down.
		 * @param[in]	isDepth	If true, the input texture is assumed to be a depth texture (instead of a color one).
		 *						Multisampled depth textures will be resolved by taking the minimum value of all samples,
		 *						unlike color textures which wil be averaged.
		 */
		void blit(const SPtr<Texture>& texture, const Rect2I& area = Rect2I::EMPTY, bool flipUV = false, 
			bool isDepth = false);

		/**
		 * Draws a quad over the entire viewport in normalized device coordinates.
		 * 			
		 * @param[in]	uv				UV coordinates to assign to the corners of the quad.
		 * @param[in]	textureSize		Size of the texture the UV coordinates are specified for. If the UV coordinates are 
		 *								already in normalized (0, 1) range then keep this value as is. If the UV coordinates 
		 *								are in texels then set this value to the texture size so they can be normalized 
		 *								internally.
		 * @param[in]	numInstances	How many instances of the quad to draw (using instanced rendering). Useful when
		 *								drawing to 3D textures.
		 * @param[in]	flipUV			If true, vertical UV coordinate will be flipped upside down.
		 * 			
		 * @note	Core thread.
		 */
		void drawScreenQuad(const Rect2& uv, const Vector2I& textureSize = Vector2I(1, 1), 
			UINT32 numInstances = 1, bool flipUV = false);

		/**
		 * Draws a quad over the entire viewport in normalized device coordinates.
		 * 			
		 * @param[in]	numInstances	How many instances of the quad to draw (using instanced rendering). Useful when
		 *								drawing to 3D textures.
		 * 			
		 * @note	Core thread.
		 */
		void drawScreenQuad(UINT32 numInstances = 1)
		{
			Rect2 uv(0.0f, 0.0f, 1.0f, 1.0f);
			Vector2I textureSize(1, 1);

			drawScreenQuad(uv, textureSize, numInstances);
		}

		/** 
		 * Clears the currently bound render target to the provided integer value. This is similar to 
		 * RenderAPI::clearRenderTarget(), except it supports integer clears.
		 */
		void clear(UINT32 value);

		/** Returns a unit sphere stencil mesh. */
		SPtr<Mesh> getSphereStencil() const { return mUnitSphereStencilMesh; }

		/** Returns a unit axis aligned box stencil mesh. */
		SPtr<Mesh> getBoxStencil() const { return mUnitBoxStencilMesh; }

		/** 
		 * Returns a stencil mesh used for a spot light. Actual vertex positions need to be computed in shader as this
		 * method will return uninitialized vertex positions. 
		 */
		SPtr<Mesh> getSpotLightStencil() const { return mSpotLightStencilMesh; }

		/** Returns a mesh that can be used for rendering a skybox. */
		SPtr<Mesh> getSkyBoxMesh() const { return mSkyBoxMesh; }

	private:
		SPtr<Mesh> mFullScreenQuadMesh;
		SPtr<Mesh> mUnitSphereStencilMesh;
		SPtr<Mesh> mUnitBoxStencilMesh;
		SPtr<Mesh> mSpotLightStencilMesh;
		SPtr<Mesh> mSkyBoxMesh;
	};

	/** Provides easy access to RendererUtility. */
	BS_EXPORT RendererUtility& gRendererUtility();

	/** @} */
}}
