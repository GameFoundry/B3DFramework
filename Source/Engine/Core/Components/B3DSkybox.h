//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "CoreObject/B3DCoreObject.h"

namespace b3d
{
	namespace render
	{
		class RendererTask;
	}

	/** @addtogroup Rendering-Internal
	 *  @{
	 */

	/** Templated base class for both render and main thread implementations of a skybox. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TSkybox : public CoreVariantType<CoreObject, IsRenderProxy>
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using Super = CoreVariantType<CoreObject, IsRenderProxy>;

		TSkybox() = default;
		virtual ~TSkybox() = default;

		/**
		 * Brightness multiplier that will be applied to skybox values before they're being used. Allows you to make the
		 * skybox more or less bright. Equal to one by default.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Brightness), Property(Setter))
		void SetBrightness(float brightness)
		{
			mBrightness = brightness;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetBrightness */
		B3D_SCRIPT_EXPORT(ExportName(Brightness), Property(Getter))
		float GetBrightness() const { return mBrightness; }

		/**
		 * Determines an environment map to use for sampling skybox radiance. Must be a cube-map texture, and should ideally
		 * contain HDR data.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Getter))
		TextureType GetTexture() const { return mTexture; }

	protected:
		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		TextureType mTexture;
		float mBrightness = 1.0f; /**< Multiplier to apply to evaluated skybox values before using them. */
	};

	/** @} */

	/** @addtogroup Rendering
	 *  @{
	 */

	/** Allows you to specify an environment map to use for sampling radiance of the sky. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Skybox : public Component, public TSkybox<false>
	{
	public:
		Skybox(const HSceneObject& parent);

		/** @copydoc TSkybox::GetTexture */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Setter))
		void SetTexture(const HTexture& texture);

	protected:
		friend class render::Skybox;
		struct SyncPacket;

		/**
		 * Filters the skybox radiance texture, generating filtered radiance (for reflections) and irradiance. Should be
		 * called any time the skybox texture changes.
		 */
		void FilterTexture();

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		SPtr<Texture> mFilteredRadiance;
		SPtr<Texture> mIrradiance;
		SPtr<render::RendererTask> mRendererTask;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void Initialize() override;
		void OnCreated() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnDestroyed() override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SkyboxRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Skybox(); // Serialization only
	};

	/** @} */

	/** @addtogroup Renderer
	 *  @{
	 */

	namespace render
	{
		/** Render thread counterpart of a b3d::Skybox */
		class B3D_EXPORT Skybox : public TSkybox<true>
		{
		public:
			~Skybox();

			/**
			 * Returns a texture containing filtered version of the radiance texture used for reflections. This might not
			 * be available if it hasn't been generated yet.
			 */
			SPtr<Texture> GetFilteredRadiance() const { return mFilteredRadiance; }

			/**
			 * Returns a texture containing sky irradiance. This might not be available if it hasn't been generated yet.
			 */
			SPtr<Texture> GetIrradiance() const { return mIrradiance; }

		protected:
			friend class b3d::Skybox;

			Skybox(const SPtr<SceneInstance>& scene, const SPtr<Texture>& radiance, const SPtr<Texture>& filteredRadiance, const SPtr<Texture>& irradiance);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			SPtr<Texture> mFilteredRadiance;
			SPtr<Texture> mIrradiance;
			bool mActive = true;
			SPtr<SceneInstance> mSceneInstance;
		};
	} // namespace render

	/** @} */
} // namespace b3d
