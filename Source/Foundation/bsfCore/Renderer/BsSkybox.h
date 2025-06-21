//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreObject/BsCoreObject.h"
#include "Scene/BsSceneActor.h"

namespace b3d
{
	namespace render
	{
		class RendererTask;
	}

	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Signals which portion of a Skybox is dirty. */
	enum class SkyboxDirtyFlag
	{
		// First few bits reserved by ActorDiryFlag
		Texture = 1 << 4
	};

	/** Templated base class for both render and main thread implementations of a skybox. */
	template <bool IsRenderProxy>
	class B3D_CORE_EXPORT TSkybox : public CoreVariantType<SceneActor, IsRenderProxy>
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using SceneInstanceType = CoreVariantType<SceneInstance, IsRenderProxy>;
		using Super = CoreVariantType<SceneActor, IsRenderProxy>;

		TSkybox() = default;
		TSkybox(const SPtr<SceneInstanceType>& scene);
		virtual ~TSkybox() = default;

		/**
		 * Brightness multiplier that will be applied to skybox values before they're being used. Allows you to make the
		 * skybox more or less bright. Equal to one by default.
		 */
		void SetBrightness(float brightness)
		{
			mBrightness = brightness;
			SceneActor::MarkSceneActorRenderProxyDataDirty();
		}

		/** @copydoc SetBrightness */
		float GetBrightness() const { return mBrightness; }

		/**
		 * Determines an environment map to use for sampling skybox radiance. Must be a cube-map texture, and should ideally
		 * contain HDR data.
		 */
		TextureType GetTexture() const { return mTexture; }

	protected:
		TextureType mTexture;
		float mBrightness = 1.0f; /**< Multiplier to apply to evaluated skybox values before using them. */
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace render
	{
		class Skybox;
	}

	/** Allows you to specify an environment map to use for sampling radiance of the sky. */
	class B3D_CORE_EXPORT Skybox : public IReflectable, public CoreObject, public TSkybox<false>
	{
	public:
		~Skybox();

		/** @copydoc TSkybox::GetTexture */
		void SetTexture(const HTexture& texture);

		/** Creates a new skybox. */
		static SPtr<Skybox> Create(const SPtr<SceneInstance>& scene);
	
	protected:
		friend class render::Skybox;
		struct SyncPacket;

		Skybox(const SPtr<SceneInstance>& scene);
		Skybox();

		/**
		 * Filters the skybox radiance texture, generating filtered radiance (for reflections) and irradiance. Should be
		 * called any time the skybox texture changes.
		 */
		void FilterTexture();

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		void MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flags = ActorDirtyFlag::Everything) override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		SPtr<Texture> mFilteredRadiance;
		SPtr<Texture> mIrradiance;
		SPtr<render::RendererTask> mRendererTask;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SkyboxRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

		/**	Creates a new skybox instance without initializing it. */
		static SPtr<Skybox> CreateEmpty();
	};

	namespace render
	{
		/** Render thread counterpart of a b3d::Skybox */
		class B3D_CORE_EXPORT Skybox : public RenderProxy, public TSkybox<true>
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
		};
	} // namespace render

	/** @} */
} // namespace b3d
