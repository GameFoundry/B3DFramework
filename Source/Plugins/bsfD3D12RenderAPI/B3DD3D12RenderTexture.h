//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DRenderTexture.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
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

		protected:
			/** @copydoc RenderTexture::CreateFramebuffer */
			void CreateFramebuffer() override;

		public:
			/** Returns the D3D12 framebuffer associated with this render texture. */
			D3D12Framebuffer* GetFramebuffer() const { return mFramebuffer.get(); }

		private:
			UniquePtr<D3D12Framebuffer> mFramebuffer;
		};

		/** @} */
	} // namespace render
} // namespace b3d
