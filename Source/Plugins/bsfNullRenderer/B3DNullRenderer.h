//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullRendererPrerequisites.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsRendererFactory.h"
#include "Renderer/BsIBLUtility.h"

namespace b3d
{
	/** Renderer factory implementation that creates and initializes the null renderer. Used by the RendererManager. */
	class NullRendererFactory : public RendererFactory
	{
	public:
		static constexpr const char* SystemName = "bsfNullRenderer";

		SPtr<render::Renderer> Create() override;
		const String& Name() const override;
	};

	namespace render
	{

		/** @addtogroup NullRenderer
		 *  @{
		 */

		/** Null renderer. */
		class NullRenderer final : public Renderer
		{
		public:
			NullRenderer() = default;

			const StringID& GetName() const override;
			void RenderAll(PerFrameData perFrameData) override;
			void CaptureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) override {}
		};

		/** Render beast implementation of IBLUtility. */
		class NullIBLUtility : public IBLUtility
		{
		public:
			void FilterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const override {}
			void FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const override {}
			void FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output, u32 outputIdx) const override {}
			void ScaleCubemap(const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip) const override {}
		};

		/**	Provides easy access to the null renderer. */
		SPtr<NullRenderer> GetNullRenderer();

		/** @} */
	} // namespace render
} // namespace b3d
