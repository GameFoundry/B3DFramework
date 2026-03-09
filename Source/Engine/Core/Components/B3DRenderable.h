//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "CoreObject/B3DCoreObject.h"
#include "Math/B3DBounds.h"
#include "Renderer/B3DRendererObjectStorage.h"
#include "Resources/B3DIResourceListener.h"
#include "Scene/B3DComponent.h"
#include "Math/B3DTransform.h"

namespace b3d
{
	struct EvaluatedAnimationData;
	class RenderableObjectStorageBase;

	namespace ecs
	{
		/** ECS fragment storing the persistent render object ID for a renderable. */
		struct RenderableId
		{
			RendererId Id = kInvalidRendererId;
		};
	}

	/** @addtogroup Components
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

	/** Common data used by both main and render thread variants of Renderable. */
	template <bool IsRenderProxy>
	struct B3D_EXPORT TRenderableData
	{
		using MeshType = CoreVariantHandleType<Mesh, IsRenderProxy>;
		using MaterialType = CoreVariantHandleType<Material, IsRenderProxy>;

		MeshType Mesh;
		Vector<MaterialType> Materials;
		u64 Layer = 1;
		AABox OverrideBounds;
		bool UseOverrideBounds = false;
		bool WriteVelocity = true;
		float CullDistanceFactor = 1.0f;
		RenderableAnimType AnimType = RenderableAnimType::None;
	};

	/** CRTP getter interface providing shared read access for both Renderable and RenderableProxy. */
	template <typename Derived, bool IsRenderProxy>
	class TRenderableGetters
	{
		using MeshType = CoreVariantHandleType<Mesh, IsRenderProxy>;
		using MaterialType = CoreVariantHandleType<Material, IsRenderProxy>;

	public:
		/**
		 * Determines the mesh to render. All sub-meshes of the mesh will be rendered, and you may set individual materials
		 * for each sub-mesh.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Getter))
		MeshType GetMesh() const { return GetData().Mesh; }

		/**	Returns the material used for rendering a sub-mesh with the specified index. */
		B3D_SCRIPT_EXPORT(ExportName(GetMaterial))
		MaterialType GetMaterial(u32 index) const
		{
			const auto& materials = GetData().Materials;
			if(index >= (u32)materials.size())
				return nullptr;

			return materials[index];
		}

		/**
		 * Determines all materials used for rendering this renderable. Each of the materials is used for rendering a single
		 * sub-mesh. If number of materials is larger than number of sub-meshes, they will be ignored. If lower, the
		 * remaining materials will be removed.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Materials), Property(Getter))
		const Vector<MaterialType>& GetMaterials() const { return GetData().Materials; }

		/**
		 * If enabled this renderable will write per-pixel velocity information when rendered. This is required for effects
		 * such as temporal anti-aliasing and motion blur, but comes with a minor performance overhead. If you are not using
		 * those effects you can disable this for a performance gain.
		 */
		B3D_SCRIPT_EXPORT(ExportName(WriteVelocity), Property(Getter))
		bool GetWriteVelocity() const { return GetData().WriteVelocity; }

		/** Factor to be applied to the cull distance set in the camera's render settings.  */
		B3D_SCRIPT_EXPORT(ExportName(CullDistance), Property(Getter))
		float GetCullDistanceFactor() const { return GetData().CullDistanceFactor; }

		/**
		 * Determines the layer bitfield that controls whether a renderable is considered visible in a specific camera.
		 * Renderable layer must match camera layer in order for the camera to render the component.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Layers), Property(Getter))
		u64 GetLayer() const { return GetData().Layer; }

		/** Returns the type of animation influencing this renderable, if any. */
		RenderableAnimType GetAnimType() const { return GetData().AnimType; }

	private:
		const TRenderableData<IsRenderProxy>& GetData() const
		{
			return static_cast<const Derived*>(this)->GetRenderableData();
		}
	};

	namespace ecs
	{
		class ECSRenderableRTTI;

		/** ECS fragment storing renderable visual data (mesh, materials, layer, etc.). */
		struct B3D_EXPORT Renderable : TRenderableData<false>, IReflectable
		{
			u64 AnimationId = (u64)-1;

			struct FullSyncPacket;
			struct TransformSyncPacket;

			friend class ECSRenderableRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};
	}

	/**
	 * Renderable represents any visible object in the scene. It has a mesh, bounds and a set of materials. Renderer will
	 * render any Renderable objects visible by a camera.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Renderable : public Component, public TRenderableGetters<Renderable, false>, public CoreObject, public IResourceListener
	{
		friend class TRenderableGetters<Renderable, false>;

		/** Returns the renderable data from the ECS fragment. Used by the CRTP getter interface. */
		const TRenderableData<false>& GetRenderableData() const { return GetFragment(); }

	public:

		/** @copydoc TRenderableGetters::GetMesh */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Setter))
		void SetMesh(const HMesh& mesh);

		/**
		 * Sets a material that will be used for rendering a sub-mesh with the specified index. If a sub-mesh doesn't have
		 * a specific material set then the primary material will be used.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMaterial))
		void SetMaterial(u32 index, const HMaterial& material);

		/**
		 * Sets the primary material to use for rendering. Any sub-mesh that doesn't have an explicit material set will use
		 * this material.
		 *
		 * @note	This is equivalent to calling SetMaterial(0, material).
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMaterial))
		void SetMaterial(const HMaterial& material) { SetMaterial(0, material); }

		/** @copydoc TRenderableGetters::GetMaterials() */
		B3D_SCRIPT_EXPORT(ExportName(Materials), Property(Setter))
		void SetMaterials(const Vector<HMaterial>& materials);

		/** @copydoc TRenderableGetters::GetLayer() */
		B3D_SCRIPT_EXPORT(ExportName(Layers), Property(Setter))
		void SetLayer(u64 layer);

		/**
		 * Sets bounds that will be used when determining if object is visible. Only relevant if SetUseOverrideBounds() is
		 * set to true.
		 *
		 * @param	bounds	Bounds in local space.
		 */
		void SetOverrideBounds(const AABox& bounds);

		/**
		 * Enables or disables override bounds. When enabled the bounds provided to SetOverrideBounds() will be used for
		 * determining object visibility, otherwise the bounds from the object's mesh will be used. Disabled by default.
		 */
		void SetUseOverrideBounds(bool enable);

		/** @copydoc TRenderableGetters::GetWriteVelocity */
		B3D_SCRIPT_EXPORT(ExportName(WriteVelocity), Property(Setter))
		void SetWriteVelocity(bool enable);

		/** @copydoc TRenderableGetters::GetCullDistanceFactor() */
		B3D_SCRIPT_EXPORT(ExportName(CullDistance), Property(Setter))
		void SetCullDistanceFactor(float factor);

		/**
		 * Determines the animation that will be used for animating the attached mesh. Note this will automatically be set if an
		 * animation component exists on the same scene object as the renderable.
		 */
		void SetAnimation(const HAnimation& animation);

		/** @copydoc SetAnimation */
		const HAnimation& GetAnimation() const { return mAnimation; }

		/** Checks is the renderable animated or static. */
		bool IsAnimated() const { return mAnimation != nullptr; }

		/**	Gets world bounds of the mesh rendered by this object. */
		B3D_SCRIPT_EXPORT(ExportName(Bounds), Property(Getter))
		Bounds GetBounds() const;

		/** @copydoc Component::CalculateBounds */
		bool CalculateBounds(Bounds& bounds) override;

		/** @name Internal
		 *  @{
		 */

		/** Registers an Animation component that will be used for animating the renderable's mesh. */
		void RegisterAnimation(const HAnimation& animation);

		/** Removes the Animation component, making the renderable rendered as a static object. */
		void UnregisterAnimation();

		/** @} */
	protected:
		/** Updates animation properties depending on the current mesh. */
		void RefreshAnimation();

		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		void OnDependencyDirty(CoreObject* dependency, u32 dirtyFlags) override;

		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override;
		void NotifyResourceChanged(const HResource& resource) override;
	private:
		HAnimation mAnimation;

		/** Returns the ECS fragment storing renderable data. */
		ecs::Renderable& GetFragment();

		/** Returns the ECS fragment storing renderable data (const). */
		const ecs::Renderable& GetFragment() const;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/** @copydoc CoreObject::MarkDependenciesDirty */
		void MarkCoreObjectDependenciesDirty();

		/** @copydoc IResourceListener::MarkListenerResourcesDirty */
		void MarkReferencedResourcesDirty();

		friend class SceneObject;
		friend class RenderableObjectStorageBase;
		friend struct RenderableSyncBatch;

		Renderable(const HSceneObject& parent);

		void Initialize() override;
		void OnCreated() override;
		void OnBeginPlay() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnDestroyed() override;
		void OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity) override;
		void OnTransformChanged(TransformChangedFlags flags) override;

		/** Triggered whenever the renderable's mesh changes. */
		void DoOnMeshChanged();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class RenderableRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Renderable(); // Serialization only
	};

	namespace render
	{
		/** Contains data and functionality for the render-side representation of a Renderable object. */
		class B3D_EXPORT RenderableProxy : public TRenderableGetters<RenderableProxy, true>
		{
			friend class TRenderableGetters<RenderableProxy, true>;
			friend class b3d::Renderable;
			friend class b3d::RenderableObjectStorageBase;

			/** Returns the renderable data. Used by the CRTP getter interface. */
			const TRenderableData<true>& GetRenderableData() const { return mData; }

		public:
			/**	Gets world bounds of the mesh rendered by this object. */
			Bounds GetBounds() const;

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(PackedRendererId id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			PackedRendererId GetRendererId() const { return mRendererId; }

			/** Returns the identifier of the animation, if this object is animated using skeleton or blend shape animation. */
			u64 GetAnimationId() const { return mAnimationId; }

			/**
			 * Updates internal animation buffers from the contents of the provided animation data object. Does nothing if
			 * renderable is not affected by animation.
			 */
			void UpdateAnimationBuffers(const EvaluatedAnimationData& animData);

			/** Returns the GPU buffer containing element's bone matrices, if it has any. */
			const SPtr<GpuBuffer>& GetBoneMatrixBuffer() const { return mBoneMatrixBuffer; }

			/** Returns the GPU buffer containing element's bone matrices for the previous frame, if it has any. */
			const SPtr<GpuBuffer>& GetPreviousBoneMatrixBuffer() const { return mPreviousBoneMatrixBuffer; }

			/** Returns the vertex buffer containing element's morph shape vertices, if it has any. */
			const SPtr<GpuBuffer>& GetMorphShapeBuffer() const { return mMorphShapeBuffer; }

			/** Returns vertex declaration used for rendering meshes containing morph shape information. */
			const SPtr<VertexDescription>& GetMorphVertexDescription() const { return mMorphVertexDescription; }

			/**	Returns the transform matrix that is applied to the object when its being rendered. */
			Matrix4 GetWorldTransformMatrix() const { return mWorldTransformMatrix; }

			/**
			 * Returns the transform matrix that is applied to the object when its being rendered. This transform matrix does
			 * not include scale values.
			 */
			Matrix4 GetWorldTransformMatrixWithoutScale() const { return mWorldTransformMatrixWithoutScale; }

		protected:
			/** Renderable data (mesh, materials, layer, etc.). */
			TRenderableData<true> mData;

			/** Creates any buffers required for renderable animation. Should be called whenever animation properties change. */
			void CreateAnimationBuffers();

			PackedRendererId mRendererId = kInvalidPackedRendererId;
			u64 mAnimationId = (u64)-1;
			u32 mMorphShapeVersion = 0;

			SPtr<GpuBuffer> mBoneMatrixBuffer;
			SPtr<GpuBuffer> mPreviousBoneMatrixBuffer;
			SPtr<GpuBuffer> mMorphShapeBuffer;
			SPtr<VertexDescription> mMorphVertexDescription;

			Transform mTransform;
			Matrix4 mWorldTransformMatrix = kIdentityTag;
			Matrix4 mWorldTransformMatrixWithoutScale = kIdentityTag;
		};
	} // namespace render

	/** Action determined by ApplyPacket for a dirty renderable during SyncWrite. */
	enum class RenderableAction : u8
	{
		Register,
		Reregister
	};

	/**
	 * Contains render thread representation of renderable objects, stored in packed arrays accessible by PackedRendererId.
	 * Implements main -> render thread synchronization logic for renderable objects.
	 */
	class B3D_EXPORT RenderableObjectStorageBase : public RendererObjectStorage
	{
	public:
		void* SyncRead(ecs::Registry& registry, FrameAllocator& allocator) override;
		void SyncWrite(void* batchData, FrameAllocator& allocator) override;

		/** Returns the renderable proxy at the given slot. */
		render::RenderableProxy& GetRenderableProxy(PackedRendererId slotId) { return mRenderableProxies[slotId]; }

		/** Returns the renderable proxy at the given slot (const). */
		const render::RenderableProxy& GetRenderableProxy(PackedRendererId slotId) const { return mRenderableProxies[slotId]; }

		/** Returns the total number of renderables. */
		u32 GetRenderableCount() const { return (u32)mRenderableProxies.size(); }

		/**
		 * Called once per frame for each new renderable being added, or a renderable whose data needs to be rebuilt
		 * after a significant update (in which DestroyRenderState will be called first).
		 */
		virtual void CreateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each renderable that is being removed, or a renderable that needs to be rebuilt
		 * after a significant update (in which CreateRenderState will be called right after).
		 */
		virtual void DestroyRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each renderable that needs to be updated after a minor update (e.g. one that doesn't
		 * require a full render state rebuild, such as a transform change).
		 */
		virtual void UpdateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

	protected:
		Vector<render::RenderableProxy> mRenderableProxies;

	private:
		/** Applies sync packet data to the renderable proxy. Returns the action needed for this renderable. */
		RenderableAction ApplyPacket(ecs::Renderable::FullSyncPacket& packet, render::RenderableProxy& proxy, PackedRendererId rendererId);
	};

	/** @} */
} // namespace b3d
