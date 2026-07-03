//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DRenderTexture.h"

namespace b3d
{
	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/**
	 * DirectX 12 implementation of a render texture.
	 *
	 * @note	Main thread only.
	 */
	class D3D12RenderTexture : public RenderTexture
	{
	public:
		virtual ~D3D12RenderTexture() = default;

	protected:
		friend class D3D12TextureManager;

		D3D12RenderTexture(const RenderTextureCreateInformation& createInformation)
			: RenderTexture(createInformation)
		{ }
	};

	/** @} */

	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of a render texture. */
		class D3D12RenderTexture : public RenderTexture
		{
		public:
			D3D12RenderTexture(const RenderTextureCreateInformation& createInformation);
			~D3D12RenderTexture() override;

			/** @copydoc RenderTexture::Initialize */
			void Initialize() override;

			/** Returns the D3D12 framebuffer associated with this render texture. */
			D3D12Framebuffer* GetFramebuffer() const { return mFramebuffer; }

		private:
			/** Creates the D3D12 framebuffer (render target and depth-stencil views) for this render texture. */
			void CreateFramebuffer();

			D3D12Framebuffer* mFramebuffer = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
