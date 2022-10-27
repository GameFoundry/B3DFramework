//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"
#include "Math/BsVector2.h"
#include "Scene/BsSceneActor.h"
#include "Math/BsBounds.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both core and sim thread implementations of Decal. */
	class BS_CORE_EXPORT DecalBase : public SceneActor
	{
	public:
		DecalBase();
		DecalBase(const Vector2& size, float maxDistance);
		virtual ~DecalBase() = default;

		/** Width and height of the decal. */
		void SetSize(const Vector2& size)
		{
			mSize = Vector2::Max(Vector2::kZero, size);
			MarkCoreDirtyInternal();
			UpdateBounds();
		}

		/** @copydoc setSize */
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
			MarkCoreDirtyInternal();
			UpdateBounds();
		}

		/** @copydoc getSize */
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
			MarkCoreDirtyInternal();
		}

		/** @copydoc setLayerMask */
		u32 GetLayerMask() const { return mLayerMask; }

		/**
		 * Determines the layer that controls whether a system is considered visible in a specific camera. Layer must match
		 * camera layer bitfield in order for the camera to render the decal.
		 */
		void SetLayer(u64 layer);

		/** @copydoc setLayer() */
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

		/** @copydoc SceneActor::setTransform */
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

	/** Templated base class for both core and sim thread implementations of Decal. */
	template <bool Core>
	class BS_CORE_EXPORT TDecal : public DecalBase
	{
	public:
		using MaterialType = CoreVariantHandleType<Material, Core>;

		TDecal() = default;

		TDecal(const MaterialType& material, const Vector2& size, float maxDistance)
			: DecalBase(size, maxDistance), mMaterial(material)
		{}

		virtual ~TDecal() = default;

		/** Determines the material to use when rendering the decal. */
		void SetMaterial(const MaterialType& material)
		{
			mMaterial = material;
			MarkCoreDirtyInternal();
		}

		/** @copydoc setMaterial */
		const MaterialType& GetMaterial() const { return mMaterial; }

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template <class P>
		void RttiEnumFields(P p);

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
	class BS_CORE_EXPORT Decal : public IReflectable, public CoreObject, public TDecal<false>
	{
	public:
		/**	Retrieves an implementation of the decal usable only from the core thread. */
		SPtr<ct::Decal> GetCore() const;

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
		Decal(const HMaterial& material, const Vector2& size, float maxDistance);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> CreateCore() const;

		/** @copydoc CoreObject::getCoreDependencies */
		void GetCoreDependencies(Vector<CoreObject*>& dependencies);

		/** @copydoc DecalBase::_markCoreDirty */
		void MarkCoreDirtyInternal(ActorDirtyFlag flags = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

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
		/** Core thread version of a bs::Decal */
		class BS_CORE_EXPORT Decal : public CoreObject, public TDecal<true>
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

			/** @copydoc CoreObject::initialize */
			void Initialize() override;

			/** @copydoc CoreObject::syncToCore */
			void SyncToCore(const CoreSyncData& data) override;

			u32 mRendererId = 0;
		};
	} // namespace ct

	/** @} */
} // namespace bs
