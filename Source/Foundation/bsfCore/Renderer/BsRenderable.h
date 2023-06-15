//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"
#include "Resources/BsIResourceListener.h"
#include "Math/BsBounds.h"
#include "Math/BsAABox.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	struct EvaluatedAnimationData;

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Type of animation that can be applied to a renderable object. */
	enum class RenderableAnimType
	{
		None,
		Skinned,
		Morph,
		SkinnedMorph,
		Count // Keep at end
	};

	/**
	 * Renderable represents any visible object in the scene. It has a mesh, bounds and a set of materials. Renderer will
	 * render any Renderable objects visible by a camera.
	 */
	template <bool Core>
	class B3D_CORE_EXPORT TRenderable : public SceneActor
	{
		using MeshType = CoreVariantHandleType<Mesh, Core>;
		using MaterialType = CoreVariantHandleType<Material, Core>;

	public:
		TRenderable();
		virtual ~TRenderable() = default;

		void SetTransform(const Transform& transform) override;

		/**
		 * Determines the mesh to render. All sub-meshes of the mesh will be rendered, and you may set individual materials
		 * for each sub-mesh.
		 */
		void SetMesh(const MeshType& mesh);

		/**
		 * Sets a material that will be used for rendering a sub-mesh with the specified index. If a sub-mesh doesn't have
		 * a specific material set then the primary material will be used.
		 */
		void SetMaterial(u32 idx, const MaterialType& material);

		/**
		 * Sets the primary material to use for rendering. Any sub-mesh that doesn't have an explicit material set will use
		 * this material.
		 *
		 * @note	This is equivalent to calling setMaterial(0, material).
		 */
		void SetMaterial(const MaterialType& material);

		/** @copydoc SetMaterials() */
		const Vector<MaterialType>& GetMaterials() { return mMaterials; }

		/**
		 * Determines all materials used for rendering this renderable. Each of the materials is used for rendering a single
		 * sub-mesh. If number of materials is larger than number of sub-meshes, they will be ignored. If lower, the
		 * remaining materials will be removed.
		 */
		void SetMaterials(const Vector<MaterialType>& materials);

		/**
		 * Determines the layer bitfield that controls whether a renderable is considered visible in a specific camera.
		 * Renderable layer must match camera layer in order for the camera to render the component.
		 */
		void SetLayer(u64 layer);

		/**
		 * Sets bounds that will be used when determining if object is visible. Only relevant if setUseOverrideBounds() is
		 * set to true.
		 *
		 * @param[in]	bounds	Bounds in local space.
		 */
		void SetOverrideBounds(const AABox& bounds);

		/**
		 * Enables or disables override bounds. When enabled the bounds provided to setOverrideBounds() will be used for
		 * determining object visibility, otherwise the bounds from the object's mesh will be used. Disabled by default.
		 */
		void SetUseOverrideBounds(bool enable);

		/**
		 * If enabled this renderable will write per-pixel velocity information when rendered. This is required for effects
		 * such as temporal anti-aliasing and motion blur, but comes with a minor performance overhead. If you are not using
		 * those effects you can disable this for a performance gain.
		 */
		void SetWriteVelocity(bool enable);

		/** @copydoc SetWriteVelocity */
		bool GetWriteVelocity() const { return mWriteVelocity; }

		/** Factor to be applied to the cull distance set in the camera's render settings.  */
		void SetCullDistanceFactor(float factor);

		/** @copydoc SetCullDistanceFactor() */
		float GetCullDistanceFactor() const { return mCullDistanceFactor; }

		/** @copydoc SetLayer() */
		u64 GetLayer() const { return mLayer; }

		/**	@copydoc SetMesh() */
		MeshType GetMesh() const { return mMesh; }

		/**	Returns the material used for rendering a sub-mesh with the specified index. */
		MaterialType GetMaterial(u32 idx) const;

		/**	Returns the transform matrix that is applied to the object when its being rendered. */
		Matrix4 GetMatrix() const { return mTfrmMatrix; }

		/**
		 * Returns the transform matrix that is applied to the object when its being rendered. This transform matrix does
		 * not include scale values.
		 */
		Matrix4 GetMatrixNoScale() const { return mTfrmMatrixNoScale; }

	protected:
		/**
		 * Notifies the core object manager that this object is dependant on some other CoreObject(s), and the dependencies
		 * changed since the last call to this method. This will trigger a call to getCoreDependencies() to collect the
		 * new dependencies.
		 */
		virtual void MarkDependenciesDirtyInternal() {}

		/**	Marks the resource dependencies list as dirty and schedules it for rebuild. */
		virtual void MarkResourcesDirtyInternal() {}

		/** Triggered whenever the renderable's mesh changes. */
		virtual void OnMeshChanged() {}

		MeshType mMesh;
		Vector<MaterialType> mMaterials;
		u64 mLayer = 1;
		AABox mOverrideBounds;
		bool mUseOverrideBounds = false;
		bool mWriteVelocity = true;
		float mCullDistanceFactor = 1.0f;
		Matrix4 mTfrmMatrix = BsIdentity;
		Matrix4 mTfrmMatrixNoScale = BsIdentity;
		RenderableAnimType mAnimType = RenderableAnimType::None;
	};

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** @copydoc TRenderable */
	class B3D_CORE_EXPORT Renderable : public IReflectable, public CoreObject, public TRenderable<false>, public IResourceListener
	{
	public:
		/**	Gets world bounds of the mesh rendered by this object. */
		Bounds GetBounds() const;

		/** Determines the animation that will be used for animating the attached mesh. */
		void SetAnimation(const SPtr<Animation>& animation);

		/** @copydoc SetAnimation */
		const SPtr<Animation>& GetAnimation() const { return mAnimation; }

		/** Checks is the renderable animated or static. */
		bool IsAnimated() const { return mAnimation != nullptr; }

		/**	Retrieves an implementation of a renderable handler usable only from the core thread. */
		SPtr<ct::Renderable> GetCore() const;

		/**	Creates a new renderable handler instance. */
		static SPtr<Renderable> Create();

		/**
		 * @name Internal
		 * @{
		 */

		void UpdateStateInternal(const SceneObject& so, bool force = false) override;
		void Initialize() override;

		/** @} */
	protected:
		friend class ct::Renderable;
		struct FullSyncData;
		
		SPtr<ct::CoreObject> CreateCore() const override;

		void OnMeshChanged() override;

		/** Updates animation properties depending on the current mesh. */
		void RefreshAnimation();

		void MarkCoreDirtyInternal(ActorDirtyFlag flag = ActorDirtyFlag::Everything) override;
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

		void MarkDependenciesDirtyInternal() override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		void OnDependencyDirty(CoreObject* dependency, u32 dirtyFlags) override;

		void MarkResourcesDirtyInternal() override;
		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override;
		void NotifyResourceChanged(const HResource& resource) override;

		/**	Creates a new renderable instance without initializing it. */
		static SPtr<Renderable> CreateEmpty();

		SPtr<Animation> mAnimation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class RenderableRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	namespace ct
	{
		/** @copydoc TRenderable */
		class B3D_CORE_EXPORT Renderable : public CoreObject, public TRenderable<true>
		{
		public:
			~Renderable();

			/**	Gets world bounds of the mesh rendered by this object. */
			Bounds GetBounds() const;

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/** Returns the type of animation influencing this renderable, if any. */
			RenderableAnimType GetAnimType() const { return mAnimType; }

			/** Returns the identifier of the animation, if this object is animated using skeleton or blend shape animation. */
			u64 GetAnimationId() const { return mAnimationId; }

			/**
			 * Updates internal animation buffers from the contents of the provided animation data object. Does nothing if
			 * renderable is not affected by animation.
			 */
			void UpdateAnimationBuffers(const EvaluatedAnimationData& animData);

			/**
			 * Records information about previous frame's animation buffer data. Should be called once per frame, before the
			 * call to updateAnimationBuffers().
			 */
			void UpdatePrevFrameAnimationBuffers();

			/** Returns the GPU buffer containing element's bone matrices, if it has any. */
			const SPtr<GpuBuffer>& GetBoneMatrixBuffer() const { return mBoneMatrixBuffer; }

			/** Returns the GPU buffer containing element's bone matrices for the previous frame, if it has any. */
			const SPtr<GpuBuffer>& GetBonePrevMatrixBuffer() const { return mBonePrevMatrixBuffer; }

			/** Returns the vertex buffer containing element's morph shape vertices, if it has any. */
			const SPtr<GpuBuffer>& GetMorphShapeBuffer() const { return mMorphShapeBuffer; }

			/** Returns vertex declaration used for rendering meshes containing morph shape information. */
			const SPtr<VertexDescription>& GetMorphVertexDescription() const { return mMorphVertexDescription; }

		protected:
			friend class bs::Renderable;

			Renderable();

			/** @copydoc CoreObject::initialize */
			void Initialize() override;

			/** @copydoc CoreObject::syncToCore */
			void SyncToCore(const CoreSyncData& data) override;

			/** Creates any buffers required for renderable animation. Should be called whenever animation properties change. */
			void CreateAnimationBuffers();

			u32 mRendererId;
			u64 mAnimationId;
			u32 mMorphShapeVersion;

			SPtr<GpuBuffer> mBoneMatrixBuffer;
			SPtr<GpuBuffer> mBonePrevMatrixBuffer;
			SPtr<GpuBuffer> mMorphShapeBuffer;
			SPtr<VertexDescription> mMorphVertexDescription;
		};
	} // namespace ct

	/** @} */
} // namespace bs
