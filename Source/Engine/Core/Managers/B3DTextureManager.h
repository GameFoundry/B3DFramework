//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Image-Internal
	 *  @{
	 */

	/**
	 * Defines interface for creation of textures. Render systems provide their own implementations.
	 *
	 * @note	Main thread only.
	 */
	class B3D_EXPORT TextureManager : public Module<TextureManager>
	{
	public:
		virtual ~TextureManager() = default;

		/**
		 * Creates a new RenderTexture and automatically generates a single color surface and (optionally) a depth/stencil
		 * surface.
		 *
		 * @param	colorDesc			Description of the color surface to create.
		 * @param	createDepth			Determines will a depth/stencil buffer of the same size as the color buffer be
		 *									created for the render texture.
		 * @param	depthStencilFormat	Format of the depth/stencil buffer if enabled.
		 */
		virtual TShared<RenderTexture> CreateRenderTexture(const TextureCreateInformation& colorDesc, bool createDepth = true, PixelFormat depthStencilFormat = PF_D32);

		/**
		 * Creates a RenderTexture using the description struct.
		 *
		 * @param	desc	Description of the render texture to create.
		 */
		virtual TShared<RenderTexture> CreateRenderTexture(const RenderTextureCreateInformation& desc);

		/**
		 * Gets the format which will be natively used for a requested format given the constraints of the current device.
		 *
		 * @note	Thread safe.
		 */
		virtual PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma) = 0;

	protected:
		/**
		 * Creates an empty and uninitialized render texture of a specific type. This is to be implemented by render
		 * systems with their own implementations.
		 */
		virtual TShared<RenderTexture> CreateRenderTextureImpl(const RenderTextureCreateInformation& desc) = 0;
	};

	namespace render
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/**
		 * Defines interface for creation of textures. Render systems provide their own implementations.
		 *
		 * @note	Render thread only.
		 */
		class B3D_EXPORT TextureManager : public Module<TextureManager>
		{
		public:
			TextureManager(GpuDevice& gpuDevice)
				:mGpuDevice(gpuDevice)
			{ }
			virtual ~TextureManager() = default;

			void OnStartUp() override;
			void OnShutDown() override;

			/**
			 * @copydoc b3d::TextureManager::CreateRenderTexture(const RenderTextureCreateInformation&)
			 */
			TShared<RenderTexture> CreateRenderTexture(const RenderTextureCreateInformation& desc);

		protected:
			friend class b3d::RenderTexture;

			/** @copydoc CreateRenderTexture */
			virtual TShared<RenderTexture> CreateRenderTextureInternal(const RenderTextureCreateInformation& desc) = 0;

			GpuDevice& mGpuDevice;
		};

		/** @} */
	} // namespace render

	/** @} */
} // namespace b3d
