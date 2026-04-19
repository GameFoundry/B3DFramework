//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullRendererPrerequisites.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererScene.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DRendererFactory.h"
#include "Renderer/B3DIBLUtility.h"

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

		/** Null implementation of RendererScene. */
		class NullRendererScene final : public RendererScene
		{
		public:
			void RegisterCamera(Camera* camera) override {}
			void UpdateCamera(Camera* camera, u32 updateFlag) override {}
			void UnregisterCamera(Camera* camera) override {}
			void RegisterLightProbeVolume(LightProbeVolume* volume) override {}
			void UpdateLightProbeVolume(LightProbeVolume* volume) override {}
			void UnregisterLightProbeVolume(LightProbeVolume* volume) override {}
			void RegisterSkybox(Skybox* skybox) override {}
			void UnregisterSkybox(Skybox* skybox) override {}
		};

		/** Null renderer. */
		class NullRenderer final : public Renderer
		{
		public:
			NullRenderer() = default;

			const StringID& GetName() const override;
			void Initialize(const SPtr<GpuDevice>& gpuDevice) override;
			void Destroy() override;
			void RenderAll(PerFrameData perFrameData) override;
			void CaptureSceneCubeMap(RendererScene& scene, GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) override {}
			SPtr<RendererScene> CreateScene() override;
		};

		/** Null implementation of IBLUtility. */
		class NullIBLUtility : public IBLUtility
		{
		public:
			void FilterCubemapForSpecular(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const override {}
			void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const override {}
			void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& output, u32 outputIdx) const override {}
			void ScaleCubemap(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip) const override {}
		};

		/**	Provides easy access to the null renderer. */
		SPtr<NullRenderer> GetNullRenderer();

		/** @} */
	} // namespace render
} // namespace b3d
