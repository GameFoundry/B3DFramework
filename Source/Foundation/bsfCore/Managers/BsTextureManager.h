//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Defines interface for creation of textures. Render systems provide their own implementations.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT TextureManager : public Module<TextureManager>
	{
	public:
		virtual ~TextureManager() = default;

		/**
		 * Creates a new RenderTexture and automatically generates a single color surface and (optionally) a depth/stencil
		 * surface.
		 *
		 * @param[in]	colorDesc			Description of the color surface to create.
		 * @param[in]	createDepth			Determines will a depth/stencil buffer of the same size as the color buffer be
		 *									created for the render texture.
		 * @param[in]	depthStencilFormat	Format of the depth/stencil buffer if enabled.
		 */
		virtual SPtr<RenderTexture> CreateRenderTexture(const TextureCreateInformation& colorDesc, bool createDepth = true, PixelFormat depthStencilFormat = PF_D32);

		/**
		 * Creates a RenderTexture using the description struct.
		 *
		 * @param[in]	desc	Description of the render texture to create.
		 */
		virtual SPtr<RenderTexture> CreateRenderTexture(const RENDER_TEXTURE_DESC& desc);

		/**
		 * Gets the format which will be natively used for a requested format given the constraints of the current device.
		 *
		 * @note	Thread safe.
		 */
		virtual PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma) = 0;

	protected:
		/**
		 * Creates an empty and uninitialized render texture of a specific type. This is to be implemented by render
		 * systems with their own implementations.
		 */
		virtual SPtr<RenderTexture> CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) = 0;
	};

	namespace ct
	{
		/**
		 * Defines interface for creation of textures. Render systems provide their own implementations.
		 *
		 * @note	Core thread only.
		 */
		class B3D_CORE_EXPORT TextureManager : public Module<TextureManager>
		{
		public:
			TextureManager(GpuDevice& gpuDevice)
				:mGpuDevice(gpuDevice)
			{ }
			virtual ~TextureManager() = default;

			void OnStartUp() override;
			void OnShutDown() override;

			/**
			 * @copydoc bs::TextureManager::CreateRenderTexture(const RENDER_TEXTURE_DESC&)
			 */
			SPtr<RenderTexture> CreateRenderTexture(const RENDER_TEXTURE_DESC& desc);

		protected:
			friend class bs::RenderTexture;

			/** @copydoc CreateRenderTexture */
			virtual SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc) = 0;

			GpuDevice& mGpuDevice;
		};
	} // namespace ct

	/** @} */
} // namespace bs
