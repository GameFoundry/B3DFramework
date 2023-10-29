//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreObject/BsCoreObject.h"
#include "Math/BsVector2.h"
#include "Scene/BsSceneActor.h"
#include "Math/BsBounds.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both render and main thread implementations of Decal. */
	class B3D_CORE_EXPORT DecalBase : public SceneActor
	{
	public:
		DecalBase();
		DecalBase(const Vector2& size, float maxDistance);
		virtual ~DecalBase() = default;

		/** Width and height of the decal. */
		void SetSize(const Vector2& size)
		{
			mSize = Vector2::Max(Vector2::kZero, size);
			MarkRenderProxyDataDirtyInternal();
			UpdateBounds();
		}

		/** @copydoc SetSize */
		Vector2 GetSize() const { return mSize; }

		/** Returns width and height of the decal, scaled by decal's transform. */
		Vector2 GetWorldSize() const
		{
			return Vector2(mSize.X * mTransform.GetScale().X, mSize.Y * mTransform.GetScale().Y);
		}

		/** Determines the maximum distance (from its origin) at which the decal is displayed. */
		void SetMaxDistance(float distance)
		{
			mMaxDistance = Math::Max(0.0f, distance);
			MarkRenderProxyDataDirtyInternal();
			UpdateBounds();
		}

		/** @copydoc GetSize */
		float GetMaxDistance() const { return mMaxDistance; }

		/** Maximum distance (from its origin) at which the decal is displayed, scaled by decal's transform. */
		float GetWorldMaxDistance() const { return mMaxDistance * mTransform.GetScale().Z; }

		/**
		 * Bitfield that allows you to mask on which objects will the decal be projected onto. Only objects with the
		 * matching layers will be projected onto. Note that decal layer mask only supports 32-bits and objects with
		 * layers in bits >= 32 will always be projected onto.
		 */
		void SetLayerMask(u32 mask)
		{
			mLayerMask = mask;
			MarkRenderProxyDataDirtyInternal();
		}

		/** @copydoc SetLayerMask */
		u32 GetLayerMask() const { return mLayerMask; }

		/**
		 * Determines the layer that controls whether a system is considered visible in a specific camera. Layer must match
		 * camera layer bitfield in order for the camera to render the decal.
		 */
		void SetLayer(u64 layer);

		/** @copydoc SetLayer() */
		u64 GetLayer() const { return mLayer; }

		/**	Gets world bounds of this object. */
		Bounds GetBounds() const { return mBounds; }

		/**	Returns the transform matrix that is applied to the object when its being rendered. */
		Matrix4 GetMatrix() const { return mTfrmMatrix; }

		/**
		 * Returns the transform matrix that is applied to the object when its being rendered. This transform matrix does
		 * not include scale values.
		 */
		Matrix4 GetMatrixNoScale() const { return mTfrmMatrixNoScale; }

		void SetTransform(const Transform& transform) override;

	protected:
		/** Updates the internal bounds for the decal. Call this whenever a property affecting the bounds changes. */
		void UpdateBounds();

		Vector2 mSize = Vector2::kOne;
		float mMaxDistance = 10.0f;
		u64 mLayer = 1;
		u32 mLayerMask = 0xFFFFFFFF;
		Matrix4 mTfrmMatrix = BsIdentity;
		Matrix4 mTfrmMatrixNoScale = BsIdentity;

		Bounds mBounds;
	};

	/** Templated base class for both render and main thread implementations of Decal. */
	template <bool IsRenderProxy>
	class B3D_CORE_EXPORT TDecal : public DecalBase
	{
	public:
		using MaterialType = CoreVariantHandleType<Material, IsRenderProxy>;

		TDecal() = default;

		TDecal(const MaterialType& material, const Vector2& size, float maxDistance)
			: DecalBase(size, maxDistance), mMaterial(material)
		{}

		virtual ~TDecal() = default;

		/** Determines the material to use when rendering the decal. */
		void SetMaterial(const MaterialType& material)
		{
			mMaterial = material;
			MarkRenderProxyDataDirtyInternal();
		}

		/** @copydoc SetMaterial */
		const MaterialType& GetMaterial() const { return mMaterial; }

	protected:
		MaterialType mMaterial;
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct
	{
		class Decal;
	}

	/**
	 * Specifies a decal that will be projected onto scene geometry. User can set the material to use when rendering
	 * the decal, as well as control decal orientation and size.
	 */
	class B3D_CORE_EXPORT Decal : public IReflectable, public CoreObject, public TDecal<false>
	{
	public:
		/**
		 * Creates a new decal.
		 *
		 * @param[in]	material		Material to use when rendering the decal.
		 * @param[in]	size			Size of the decal in world units.
		 * @param[in]	maxDistance		Maximum distance at which will the decal be visible (from the current decal origin,
		 *								along the negative Z axis).
		 * @returns						New decal object.
		 */
		static SPtr<Decal> Create(const HMaterial& material, const Vector2& size = Vector2::kOne, float maxDistance = 10.0f);

	protected:
		friend ct::Decal;
		struct SyncPacket;

		Decal(const HMaterial& material, const Vector2& size, float maxDistance);

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		void MarkRenderProxyDataDirtyInternal(ActorDirtyFlag flags = ActorDirtyFlag::Everything) override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		/**	Creates the object with without initializing it. Used for serialization. */
		static SPtr<Decal> CreateEmpty();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class DecalRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		Decal() = default; // Serialization only
	};

	namespace ct
	{
		/** Render thread counterpart of a bs::Decal */
		class B3D_CORE_EXPORT Decal : public RenderProxy, public TDecal<true>
		{
		public:
			~Decal();

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

		protected:
			friend class bs::Decal;

			Decal(const SPtr<Material>& material, const Vector2& size, float maxDistance);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			u32 mRendererId = 0;
		};
	} // namespace ct

	/** @} */
} // namespace bs
