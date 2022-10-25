//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullRendererPrerequisites.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsRendererFactory.h"
#include "Renderer/BsIBLUtility.h"

namespace bs
{
	/** Renderer factory implementation that creates and initializes the null renderer. Used by the RendererManager. */
	class NullRendererFactory : public RendererFactory
	{
	public:
		static constexpr const char* SystemName = "bsfNullRenderer";

		/** @copydoc RendererFactory::create */
		SPtr<ct::Renderer> Create();

		/** @copydoc RendererFactory::name */
		const String& Name() const;
	};

	namespace ct
	{

		/** @addtogroup NullRenderer
		 *  @{
		 */

		/** Null renderer. */
		class NullRenderer final : public Renderer
		{
		public:
			NullRenderer() = default;

			/** @copydoc Renderer::getName */
			const StringID& GetName() const;

			/** @copydoc Renderer::renderAll */
			void RenderAll(PerFrameData perFrameData);

			/** @copydoc Renderer::captureSceneCubeMap */
			void CaptureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) {}
		};

		/** Render beast implementation of IBLUtility. */
		class NullIBLUtility : public IBLUtility
		{
		public:
			/** @copydoc IBLUtility::filterCubemapForSpecular */
			void FilterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const {}

			/** @copydoc IBLUtility::filterCubemapForIrradiance(const SPtr<Texture>&, const SPtr<Texture>&) const */
			void FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const override {}

			/** @copydoc IBLUtility::filterCubemapForIrradiance(const SPtr<Texture>&, const SPtr<Texture>&, u32) const */
			void FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output, u32 outputIdx) const {}

			/** @copydoc IBLUtility::scaleCubemap */
			void ScaleCubemap(const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip) const {}
		};

		/**	Provides easy access to the null renderer. */
		SPtr<NullRenderer> gNullRenderer();

		/** @} */
	} // namespace ct
} // namespace bs
