//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"
#include "BsRect2.h"
#include "BsVector2I.h"
#include "BsRect2I.h"
#include "BsRendererMaterial.h"

namespace bs { namespace ct
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	class ResolveMat;
	class BlitMat;

	/**
	 * Contains various utility methods that make various common operations in the renderer easier.
	 * 			
	 * @note	Core thread only.
	 */
	class BS_EXPORT RendererUtility : public Module<RendererUtility>
	{
	public:
		RendererUtility();
		~RendererUtility();

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
		 * @param[in]	flipUV If true, vertical UV coordinate will be flipped upside down.
		 */
		void blit(const SPtr<Texture>& texture, const Rect2I& area = Rect2I::EMPTY, bool flipUV = false);

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

		/** Returns a stencil mesh used for a point light (a unit sphere). */
		SPtr<Mesh> getPointLightStencil() const { return mPointLightStencilMesh; }

		/** Returns a stencil mesh used for spot light. Actual vertex positions need to be computed in shader. */
		SPtr<Mesh> getSpotLightStencil() const { return mSpotLightStencilMesh; }

		/** Returns a mesh that can be used for rendering a skybox. */
		SPtr<Mesh> getSkyBoxMesh() const { return mSkyBoxMesh; }

	private:
		SPtr<Mesh> mFullScreenQuadMesh;
		SPtr<Mesh> mPointLightStencilMesh;
		SPtr<Mesh> mSpotLightStencilMesh;
		SPtr<Mesh> mSkyBoxMesh;
		SPtr<ResolveMat> mResolveMat;
		SPtr<BlitMat> mBlitMat;
	};

	/** Provides easy access to RendererUtility. */
	BS_EXPORT RendererUtility& gRendererUtility();

	/** Shader that resolves a MSAA surface into a non-MSAA render target. */
	class ResolveMat : public RendererMaterial<ResolveMat>
	{
		RMAT_DEF("Resolve.bsl");

	public:
		ResolveMat();

		/** Updates the parameter buffers used by the material. */
		void setParameters(const SPtr<Texture>& source);
	private:
		MaterialParamInt mNumSamples;
		MaterialParamTexture mSource;
	};

	/** Shader that copies a source texture into a render target. */
	class BlitMat : public RendererMaterial<BlitMat>
	{
		RMAT_DEF("Blit.bsl");

	public:
		BlitMat();

		/** Updates the parameter buffers used by the material. */
		void setParameters(const SPtr<Texture>& source);
	private:
		MaterialParamTexture mSource;
	};

	/** @} */
}}