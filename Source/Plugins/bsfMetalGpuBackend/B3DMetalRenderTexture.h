//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DRenderTexture.h"

namespace b3d
{
	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Metal implementation of a render texture.
	 *
	 * @note	Main thread only.
	 */
	class MetalRenderTexture : public RenderTexture
	{
	public:
		MetalRenderTexture(const RenderTextureCreateInformation& createInformation);
		virtual ~MetalRenderTexture() = default;
	};

	namespace render
	{
		class MetalFramebuffer;

		/**
		 * Metal implementation of a render texture.
		 *
		 * @note	Render thread only.
		 */
		class MetalRenderTexture : public RenderTexture
		{
		public:
			MetalRenderTexture(const RenderTextureCreateInformation& createInformation);
			~MetalRenderTexture() override;

			void Initialize() override;

			/** Returns the framebuffer holding this render texture's attachments. Null until Initialize() is called. */
			MetalFramebuffer* GetFramebuffer() const { return mFramebuffer.get(); }

		private:
			TUnique<MetalFramebuffer> mFramebuffer;
		};

	} // namespace render

	/** @} */
} // namespace b3d
