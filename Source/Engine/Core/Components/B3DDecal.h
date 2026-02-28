//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "Scene/B3DTransform.h"
#include "CoreObject/B3DCoreObject.h"

namespace b3d
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Common code used both by main and render thread variants of Decal. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TDecal : public CoreVariantType<CoreObject, IsRenderProxy>
	{
	public:
		using MaterialType = CoreVariantHandleType<Material, IsRenderProxy>;
		using SceneInstanceType = CoreVariantType<SceneInstance, IsRenderProxy>;
		using Super = CoreVariantType<CoreObject, IsRenderProxy>;

		TDecal();
		virtual ~TDecal() = default;

		/** Width and height of the decal. */
		B3D_SCRIPT_EXPORT(ExportName(Size), Property(Setter))
		void SetSize(const Vector2& size)
		{
			mSize = Vector2::Max(Vector2::kZero, size);
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}

		/** @copydoc SetSize */
		B3D_SCRIPT_EXPORT(ExportName(Size), Property(Getter))
		Vector2 GetSize() const { return mSize; }

		/** Determines the material to use when rendering the decal. */
		B3D_SCRIPT_EXPORT(ExportName(Material), Property(Setter))
		void SetMaterial(const MaterialType& material)
		{
			mMaterial = material;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetMaterial */
		B3D_SCRIPT_EXPORT(ExportName(Material), Property(Getter))
		const MaterialType& GetMaterial() const { return mMaterial; }

		/** Determines the maximum distance (from its origin) at which the decal is displayed. */
		B3D_SCRIPT_EXPORT(ExportName(MaxDistance), Property(Setter))
		void SetMaxDistance(float distance)
		{
			mMaxDistance = Math::Max(0.0f, distance);
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}

		/** @copydoc SetMaxDistance */
		B3D_SCRIPT_EXPORT(ExportName(MaxDistance), Property(Getter))
		float GetMaxDistance() const { return mMaxDistance; }

		/**
		 * Bitfield that allows you to mask on which objects will the decal be projected onto. Only objects with the
		 * matching layers will be projected onto. Note that decal layer mask only supports 32-bits and objects with
		 * layers in bits >= 32 will always be projected onto.
		 */
		B3D_SCRIPT_EXPORT(ExportName(LayerMask), Property(Setter))
		void SetLayerMask(u32 mask)
		{
			mLayerMask = mask;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetLayerMask */
		B3D_SCRIPT_EXPORT(ExportName(LayerMask), Property(Getter))
		u32 GetLayerMask() const { return mLayerMask; }

		/**
		 * Determines the layer that controls whether a system is considered visible in a specific camera. Layer must match
		 * camera layer bitfield in order for the camera to render the decal.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Setter))
		void SetLayer(u64 layer);

		/** @copydoc SetLayer() */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Getter))
		u64 GetLayer() const { return mLayer; }

		/**	Gets world bounds of this object. */
		Bounds GetBounds() const { return mBounds; }

		/**	Returns the transform matrix that is applied to the object when its being rendered. */
		Matrix4 GetWorldTransformMatrix() const { return mWorldTransformMatrix; }

		/**
		 * Returns the transform matrix that is applied to the object when its being rendered. This transform matrix does
		 * not include scale values.
		 */
		Matrix4 GetWorldTransformMatrixWithoutScale() const { return mWorldTransformMatrixWithoutScale; }

	protected:
		/** Updates the internal bounds for the decal. Call this whenever a property affecting the bounds changes. */
		void UpdateBounds();

		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/** @copydoc CoreObject::MarkDependenciesDirty */
		void MarkCoreObjectDependenciesDirty();

		MaterialType mMaterial;
		Matrix4 mWorldTransformMatrix = kIdentityTag;
		Matrix4 mWorldTransformMatrixWithoutScale = kIdentityTag;

		Vector2 mSize = Vector2::kOne;
		float mMaxDistance = 10.0f;
		u64 mLayer = 1;
		u32 mLayerMask = 0xFFFFFFFF;

		Bounds mBounds = Bounds::kEmpty;
	};

	/** @} */
	/** @addtogroup Components
	 *  @{
	 */

	/** Wraps Decal as a Component. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Decal : public Component, public TDecal<false>
	{
	public:
		Decal(const HSceneObject& parent);

	protected:
		struct FullSyncPacket;
		struct TransformSyncPacket;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void Initialize() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnDestroyed() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class DecalRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Decal(); // Serialization only
	};

	namespace render
	{
		/** Render thread counterpart of a b3d::Decal */
		class B3D_EXPORT Decal : public TDecal<true>
		{
		public:
			~Decal();

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/** Returns the world space transform for the decal. */
			const Transform& GetWorldTransform() const { return mTransform; }

			/** Returns width and height of the decal, scaled by decal's transform. */
			Vector2 GetWorldSize() const
			{
				return Vector2(mSize.X * mTransform.GetScale().X, mSize.Y * mTransform.GetScale().Y);
			}

			/** Maximum distance (from its origin) at which the decal is displayed, scaled by decal's transform. */
			float GetWorldMaxDistance() const { return mMaxDistance * mTransform.GetScale().Z; }

		protected:
			friend class b3d::Decal;

			Decal(const SPtr<SceneInstance>& scene);

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
