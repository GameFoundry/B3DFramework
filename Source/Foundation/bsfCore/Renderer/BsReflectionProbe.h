//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreObject/BsCoreObject.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Math/BsSphere.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	/** @addtogroup Renderer-Internal
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

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both core and sim thread implementations of a reflection probe. */
	class B3D_CORE_EXPORT ReflectionProbeBase : public SceneActor
	{
	public:
		ReflectionProbeBase() = default;
		ReflectionProbeBase(ReflectionProbeType type, float radius, const Vector3& extents);
		virtual ~ReflectionProbeBase() = default;

		/**	Returns the type of the probe. */
		ReflectionProbeType GetType() const { return mType; }

		/**	Changes the type of the probe. */
		void SetType(ReflectionProbeType type)
		{
			mType = type;
			MarkCoreDirtyInternal();
			UpdateBounds();
		}

		/** Returns the radius of a sphere reflection probe. Determines range of influence. */
		float GetRadius() const;

		/** Sets the radius of a sphere reflection probe. */
		void SetRadius(float radius)
		{
			mRadius = radius;
			MarkCoreDirtyInternal();
			UpdateBounds();
		}

		/** Returns the extents of a box reflection probe. */
		Vector3 GetExtents() const { return mExtents * mTransform.GetScale(); }

		/** Sets the extents of a box reflection probe. Determines range of influence. */
		void SetExtents(const Vector3& extents)
		{
			mExtents = extents;
			MarkCoreDirtyInternal();
			UpdateBounds();
		}

		/**	Returns world space bounds that completely encompass the probe's area of influence. */
		Sphere GetBounds() const { return mBounds; }

		/**
		 * Sets a distance that will be used for fading out the box reflection probe with distance. By default it
		 * is equal to one, and can never be less than one. Only relevant for box probes.
		 */
		void SetTransitionDistance(float distance) { mTransitionDistance = std::max(1.0f, distance); }

		/** Retrieves transition distance set by setTransitionDistance(). */
		float GetTransitionDistance() const { return mTransitionDistance; }

	protected:
		/** Updates the internal bounds for the probe. Call this whenever a property affecting the bounds changes. */
		void UpdateBounds();

		ReflectionProbeType mType = ReflectionProbeType::Box; /**< Type of probe that determines how are the rest of the parameters interpreted. */
		float mRadius = 1.0f; /**< Radius used for sphere reflection probes. */
		Vector3 mExtents = { 1.0f, 1.0f, 1.0f }; /**< Extents used by box reflection probe. */
		float mTransitionDistance = 0.1f; /**< Extra distance to used for fading out box probes. */

		Sphere mBounds = { Vector3::kZero, 1.0f }; /**< Sphere that bounds the probe area of influence. */
	};

	/** Templated base class for both core and sim thread implementations of a reflection probe. */
	template <bool Core>
	class B3D_CORE_EXPORT TReflectionProbe : public ReflectionProbeBase
	{
	public:
		using TextureType = CoreVariantType<Texture, Core>;

		TReflectionProbe() = default;

		TReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
			: ReflectionProbeBase(type, radius, extents)
		{}

		virtual ~TReflectionProbe() = default;

		/**
		 * Returns a pre-filtered texture that is generated either from the provided custom texture, or from scene capture.
		 */
		SPtr<TextureType> GetFilteredTexture() const { return mFilteredTexture; }

	protected:
		SPtr<TextureType> mFilteredTexture;
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct
	{
		class RendererTask;
		class ReflectionProbe;
	} // namespace ct

	/**
	 * Specifies a location at which a pre-computed texture containing scene radiance will be generated. This texture will
	 * then be used by the renderer to provide specular reflections.
	 */
	class B3D_CORE_EXPORT ReflectionProbe : public IReflectable, public CoreObject, public TReflectionProbe<false>
	{
	public:
		~ReflectionProbe();

		/**
		 * Allows you assign a custom texture to use as a reflection map. This will disable automatic generation of
		 * reflections. To re-enable auto-generation call this with a null parameter.
		 */
		void SetCustomTexture(const HTexture& texture)
		{
			mCustomTexture = texture;
			Filter();
		}

		/** Gets the custom texture assigned through setCustomTexture(). */
		HTexture GetCustomTexture() const { return mCustomTexture; }

		/**
		 * Captures the scene at the current location and generates a filtered reflection cubemap. No action is taken
		 * if a custom texture is set.
		 */
		void Capture();

		/**
		 * Filters the custom texture, making it usable for rendering. Called automatically when custom texture changes. If
		 * no custom texture is set, no action is taken.
		 */
		void Filter();

		/**	Retrieves an implementation of the reflection probe usable only from the core thread. */
		SPtr<ct::ReflectionProbe> GetCore() const;

		/**
		 * Creates a new sphere reflection probe.
		 *
		 * @param[in]	radius	Radius in which the reflection probe will be rendered within.
		 * @returns				New reflection probe.
		 */
		static SPtr<ReflectionProbe> CreateSphere(float radius);

		/**
		 * Creates a new box reflection probe.
		 *
		 * @param[in]	extents	Extents of the box in which the reflection probe will be rendered within.
		 * @returns				New reflection probe.
		 */
		static SPtr<ReflectionProbe> CreateBox(const Vector3& extents);

	protected:
		friend class ct::ReflectionProbe;
		struct SyncPacket;

		ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents);

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		void MarkCoreDirtyInternal(ActorDirtyFlag flags = ActorDirtyFlag::Everything) override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		/**
		 * Captures the scene color at current probe location and generates a filtered map. If a custom texture is set then
		 * it will be filtered, instead of capturing scene color.
		 */
		void CaptureAndFilter();

		/**	Creates a probe with without initializing it. Used for serialization. */
		static SPtr<ReflectionProbe> CreateEmpty();

		HTexture mCustomTexture;
		SPtr<ct::RendererTask> mRendererTask;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ReflectionProbeRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		ReflectionProbe() = default; // Serialization only
	};

	namespace ct
	{
		/** Core thread usable version of a bs::ReflectionProbe */
		class B3D_CORE_EXPORT ReflectionProbe : public RenderProxy, public TReflectionProbe<true>
		{
		public:
			~ReflectionProbe();

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

		protected:
			friend class bs::ReflectionProbe;

			ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents, const SPtr<Texture>& filteredTexture);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			u32 mRendererId;
		};
	} // namespace ct

	/** @} */
} // namespace bs
