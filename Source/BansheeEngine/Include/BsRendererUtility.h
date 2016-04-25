//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"
#include "BsRect2.h"
#include "BsVector2I.h"
#include "BsRect2I.h"
#include "BsRendererMaterial.h"

namespace BansheeEngine
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
		 * @param[in]	material	Material containing the pass.
		 * @param[in]	passIdx		Index of the pass in the material.
		 *
		 * @note	Core thread.
		 */
		void setPass(const SPtr<MaterialCore>& material, UINT32 passIdx = 0);

		/**
		 * Activates the specified material pass for compute. Any further dispatch calls will be executed using this pass.
		 *
		 * @param[in]	material	Material containing the pass.
		 * @param[in]	passIdx		Index of the pass in the material.
		 *
		 * @note	Core thread.
		 */
		void setComputePass(const SPtr<MaterialCore>& material, UINT32 passIdx = 0);

		/**
		 * Sets parameters (textures, samplers, buffers) for the currently active pass.
		 *
		 * @param[in]	material	Material whose pass' parameters to bind.
		 * @param[in]	passIdx		Index of the pass in the material.
		 *					
		 * @note	Core thread.
		 */
		static void setPassParams(const SPtr<MaterialCore>& material, UINT32 passIdx = 0);

		/**
		 * Draws the specified mesh.
		 *
		 * @note	Core thread.
		 */
		void draw(const SPtr<MeshCoreBase>& mesh, const SubMesh& subMesh);

		/**
		 * Blits contents of the provided texture into the currently bound render target. If the provided texture contains
		 * multiple samples, they will be resolved.
		 *
		 * @param[in]	texture	Source texture to blit.
		 * @param[in]	area	Area of the source texture to blit in pixels. If width or height is zero it is assumed
		 *						the entire texture should be blitted.
		 */
		void blit(const SPtr<TextureCore>& texture, const Rect2I& area = Rect2I::EMPTY);

		/**
		 * Draws a quad over the entire viewport in normalized device coordinates.
		 * 			
		 * @param[in]	uv			UV coordinates to assign to the corners of the quad.
		 * @param[in]	textureSize	Size of the texture the UV coordinates are specified for. If the UV coordinates are 
		 *							already in normalized (0, 1) range then keep this value as is. If the UV coordinates 
		 *							are in texels then set this value to the texture size so they can be normalized 
		 *							internally.
		 * 			
		 * @note	Core thread.
		 */
		void drawScreenQuad(const Rect2& uv = Rect2(0.0f, 0.0f, 1.0f, 1.0f), const Vector2I& textureSize = Vector2I(1, 1));

		/** Returns a stencil mesh used for a point light (a unit sphere). */
		SPtr<MeshCore> getPointLightStencil() const { return mPointLightStencilMesh; }

		/** Returns a stencil mesh used for spot light. Actual vertex positions need to be computed in shader. */
		SPtr<MeshCore> getSpotLightStencil() const { return mSpotLightStencilMesh; }

	private:
		SPtr<MeshCore> mFullScreenQuadMesh;
		SPtr<MeshCore> mPointLightStencilMesh;
		SPtr<MeshCore> mSpotLightStencilMesh;
		SPtr<ResolveMat> mResolveMat;
		SPtr<BlitMat> mBlitMat;
	};

	/**
	 * Provides easy access to RendererUtility.
	 * 			
	 * @note	Core thread only.
	 */
	BS_EXPORT RendererUtility& gRendererUtility();

	/** Shader that resolves a MSAA surface into a non-MSAA render target. */
	class ResolveMat : public RendererMaterial<ResolveMat>
	{
		RMAT_DEF("Resolve.bsl");

	public:
		ResolveMat();

		/** Updates the parameter buffers used by the material. */
		void setParameters(const SPtr<TextureCore>& source);
	private:
		MaterialParamIntCore mNumSamples;
		MaterialParamTextureCore mSource;
	};

	/** Shader that copies a source texture into a render target. */
	class BlitMat : public RendererMaterial<BlitMat>
	{
		RMAT_DEF("Blit.bsl");

	public:
		BlitMat();

		/** Updates the parameter buffers used by the material. */
		void setParameters(const SPtr<TextureCore>& source);
	private:
		MaterialParamTextureCore mSource;
	};

	/** @} */
}