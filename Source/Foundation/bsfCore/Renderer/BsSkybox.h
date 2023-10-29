//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreObject/BsCoreObject.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	namespace ct
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

	/** Base class for both render and main thread implementations of a skybox. */
	class B3D_CORE_EXPORT SkyboxBase : public SceneActor
	{
	public:
		SkyboxBase() = default;
		virtual ~SkyboxBase() = default;

		/**
		 * Brightness multiplier that will be applied to skybox values before they're being used. Allows you to make the
		 * skybox more or less bright. Equal to one by default.
		 */
		void SetBrightness(float brightness)
		{
			mBrightness = brightness;
			MarkCoreDirtyInternal();
		}

		/** @copydoc SetBrightness */
		float GetBrightness() const { return mBrightness; }

	protected:
		float mBrightness = 1.0f; /**< Multiplier to apply to evaluated skybox values before using them. */
	};

	/** Templated base class for both render and main thread implementations of a skybox. */
	template <bool IsRenderProxy>
	class B3D_CORE_EXPORT TSkybox : public SkyboxBase
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

		virtual ~TSkybox() = default;

		/**
		 * Determines an environment map to use for sampling skybox radiance. Must be a cube-map texture, and should ideally
		 * contain HDR data.
		 */
		TextureType GetTexture() const { return mTexture; }

	protected:
		TextureType mTexture;
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct
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

		/**	Retrieves the render proxy. */
		SPtr<ct::Skybox> GetCore() const;

		/** Creates a new skybox. */
		static SPtr<Skybox> Create();
	
	protected:
		friend class ct::Skybox;
		struct SyncPacket;

		Skybox();

		/**
		 * Filters the skybox radiance texture, generating filtered radiance (for reflections) and irradiance. Should be
		 * called any time the skybox texture changes.
		 */
		void FilterTexture();

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		void MarkCoreDirtyInternal(ActorDirtyFlag flags = ActorDirtyFlag::Everything) override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		SPtr<Texture> mFilteredRadiance;
		SPtr<Texture> mIrradiance;
		SPtr<ct::RendererTask> mRendererTask;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SkyboxRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

		/**	Creates a new skybox instance without initializing it. */
		static SPtr<Skybox> CreateEmpty();
	};

	namespace ct
	{
		/** Render thread counterpart of a bs::Skybox */
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
			friend class bs::Skybox;

			Skybox(const SPtr<Texture>& radiance, const SPtr<Texture>& filteredRadiance, const SPtr<Texture>& irradiance);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			SPtr<Texture> mFilteredRadiance;
			SPtr<Texture> mIrradiance;
		};
	} // namespace ct

	/** @} */
} // namespace bs
