//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DRenderTexture.h"

namespace b3d
{
	/** @addtogroup NullGpuBackend 
	 *  @{
	 */

	/**
	 * Null implementation of a render texture.
	 *
	 * @note	Main thread only.
	 */
	class NullRenderTexture : public RenderTexture
	{
	public:
		NullRenderTexture(const RenderTextureCreateInformation& createInformation);
		virtual ~NullRenderTexture() = default;
	};

	namespace render
	{
		/**
		 * Null implementation of a render texture.
		 *
		 * @note	Render thread only.
		 */
		class NullRenderTexture : public RenderTexture
		{
		public:
			NullRenderTexture(const RenderTextureCreateInformation& createInformation);
			~NullRenderTexture() override = default;

		protected:
			void Initialize() override {}
		};

	} // namespace render

	/** @} */
} // namespace b3d
