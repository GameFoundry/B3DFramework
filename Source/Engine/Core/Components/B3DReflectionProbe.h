//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "Scene/B3DTransform.h"
#include "CoreObject/B3DCoreObject.h"
#include "Scene/B3DSceneObject.h"

namespace b3d
{
	namespace render
	{
		class RendererTask;
	}

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Probe type that determines the shape of the probe and how is it interpreted by the renderer. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ReflectionProbeType
	{
		/**
		 * Reflection probe cubemap is generated, and box extents are used for calculating influence ranges and box
		 * geometry.
		 */
		Box,
		/**
		 * Reflection probe cubemap is generated, but sphere is used for calculating the influence radius and
		 * proxy geometry.
		 */
		Sphere
	};

	/** Templated base class for both render and main thread implementations of a reflection probe. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TReflectionProbe : public CoreVariantType<CoreObject, IsRenderProxy>
	{
	public:
		using TextureType = CoreVariantType<Texture, IsRenderProxy>;
		using SceneInstanceType = CoreVariantType<SceneInstance, IsRenderProxy>;
		using Super = CoreVariantType<CoreObject, IsRenderProxy>;

		TReflectionProbe() = default;
		TReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents);
		virtual ~TReflectionProbe() = default;

		/**	Determines the type of the probe. */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Setter))
		void SetType(ReflectionProbeType type)
		{
			mType = type;
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}

		/** @copydoc SetType */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Getter))
		ReflectionProbeType GetType() const { return mType; }

		/** Determines the radius of a sphere reflection probe. */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius)
		{
			mRadius = radius;
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}
		
		/** Determines the extents of a box reflection probe. Determines range of influence. */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Setter))
		void SetExtents(const Vector3& extents)
		{
			mExtents = extents;
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}

		/**	Returns world space bounds that completely encompass the probe's area of influence. */
		Sphere GetBounds() const { return mBounds; }

		/**
		 * Determines a distance that will be used for fading out the box reflection probe with distance. By default it
		 * is equal to one, and can never be less than one. Only relevant for box probes.
		 */
		void SetTransitionDistance(float distance) { mTransitionDistance = std::max(1.0f, distance); }

		/** @copydoc SetTransitionDistance */
		float GetTransitionDistance() const { return mTransitionDistance; }

		/** Returns a pre-filtered texture that is generated either from the provided custom texture, or from scene capture. */
		SPtr<TextureType> GetFilteredTexture() const { return mFilteredTexture; }

	protected:
		/** Updates the internal bounds for the probe. Call this whenever a property affecting the bounds changes. */
		void UpdateBounds();

		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		ReflectionProbeType mType = ReflectionProbeType::Box; /**< Type of probe that determines how are the rest of the parameters interpreted. */
		float mRadius = 1.0f; /**< Radius used for sphere reflection probes. */
		Vector3 mExtents = { 1.0f, 1.0f, 1.0f }; /**< Extents used by box reflection probe. */
		float mTransitionDistance = 0.1f; /**< Extra distance to used for fading out box probes. */

		Sphere mBounds = { Vector3::kZero, 1.0f }; /**< Sphere that bounds the probe area of influence. */

		SPtr<TextureType> mFilteredTexture;
	};

	/** @} */
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * Specifies a location at which a pre-computed texture containing scene radiance will be generated. This texture will
	 * then be used by the renderer to provide specular reflections.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ReflectionProbe : public Component, public TReflectionProbe<false>
	{
	public:
		ReflectionProbe(const HSceneObject& parent);

		/**
		 * Allows you assign a custom texture to use as a reflection map. This will disable automatic generation of
		 * reflections. To re-enable auto-generation call this with a null parameter.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CustomTexture), Property(Setter))
		void SetCustomTexture(const HTexture& texture);

		/** @copydoc SetCustomTexture */
		B3D_SCRIPT_EXPORT(ExportName(CustomTexture), Property(Getter))
		HTexture GetCustomTexture() const { return mCustomTexture; }

		/** @copydoc SetRadius */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))
		float GetRadius() const;

		/** @copydoc SetExtents */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Getter))
		Vector3 GetExtents() const { return mExtents * SceneObject()->GetTransform().GetScale(); }

		/**
		 * Captures the scene at the current location and generates a filtered reflection cubemap. No action is taken
		 * if a custom texture is set.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Capture))
		void Capture();

	protected:
		/**
		 * Captures the scene color at current probe location and generates a filtered map. If a custom texture is set then
		 * it will be filtered, instead of capturing scene color.
		 */
		void CaptureAndFilter();

		/**
		 * Filters the custom texture, making it usable for rendering. Called automatically when custom texture changes. If
		 * no custom texture is set, no action is taken.
		 */
		void Filter();

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		HTexture mCustomTexture;
		SPtr<render::RendererTask> mRendererTask;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;
		struct SyncPacket;
		friend class render::ReflectionProbe;

		void Initialize() override;
		void OnCreated() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnDestroyed() override;
		void Update() override {}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ReflectionProbeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		ReflectionProbe(); // Serialization only
	};

	namespace render
	{
		/** Render thread counterpart of a b3d::ReflectionProbe */
		class B3D_EXPORT ReflectionProbe : public TReflectionProbe<true>
		{
		public:
			~ReflectionProbe();

			/** @copydoc b3d::ReflectionProbe::GetRadius */
			float GetRadius() const;

			/** @copydoc b3d::ReflectionProbe::GetExtents */
			Vector3 GetExtents() const { return mExtents * mTransform.GetScale(); }

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/** Returns the world space transform for the probe. */
			const Transform& GetWorldTransform() const { return mTransform; }

		protected:
			friend class b3d::ReflectionProbe;

			ReflectionProbe(const SPtr<SceneInstance>& scene, ReflectionProbeType type, float radius, const Vector3& extents, const SPtr<Texture>& filteredTexture);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			u32 mRendererId = 0;
			Transform mTransform;
			bool mActive = true;
			SPtr<SceneInstance> mSceneInstance;
		};
	} // namespace render

	/** @} */
} // namespace b3d
