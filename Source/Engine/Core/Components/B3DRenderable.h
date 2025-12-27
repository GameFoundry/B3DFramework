//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "CoreObject/B3DCoreObject.h"
#include "CoreObject/B3DRenderProxy.h"
#include "Math/B3DBounds.h"
#include "Resources/B3DIResourceListener.h"
#include "Scene/B3DComponent.h"
#include "Scene/B3DTransform.h"

namespace b3d
{
	struct EvaluatedAnimationData;

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

	/** Common code used both by main and render thread variants of Renderable. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TRenderable : public CoreVariantType<CoreObject, IsRenderProxy>, public std::conditional_t<IsRenderProxy, EmptyBase, IResourceListener>
	{
		using MeshType = CoreVariantHandleType<Mesh, IsRenderProxy>;
		using MaterialType = CoreVariantHandleType<Material, IsRenderProxy>;
		using Super = CoreVariantType<CoreObject, IsRenderProxy>;

	public:
		TRenderable();
		virtual ~TRenderable() = default;

		/**
		 * Determines the mesh to render. All sub-meshes of the mesh will be rendered, and you may set individual materials
		 * for each sub-mesh.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Setter))
		void SetMesh(const MeshType& mesh);

		/** @copydoc SetMesh */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Getter))
		MeshType GetMesh() const { return mMesh; }

		/**
		 * Sets a material that will be used for rendering a sub-mesh with the specified index. If a sub-mesh doesn't have
		 * a specific material set then the primary material will be used.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMaterial))
		void SetMaterial(u32 index, const MaterialType& material);

		/**	Returns the material used for rendering a sub-mesh with the specified index. */
		B3D_SCRIPT_EXPORT(ExportName(GetMaterial))
		MaterialType GetMaterial(u32 index) const;

		/**
		 * Sets the primary material to use for rendering. Any sub-mesh that doesn't have an explicit material set will use
		 * this material.
		 *
		 * @note	This is equivalent to calling SetMaterial(0, material).
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMaterial))
		void SetMaterial(const MaterialType& material) { SetMaterial(0, material); }

		/** @copydoc SetMaterials() */
		B3D_SCRIPT_EXPORT(ExportName(Materials), Property(Getter))
		const Vector<MaterialType>& GetMaterials() { return mMaterials; }

		/**
		 * Determines all materials used for rendering this renderable. Each of the materials is used for rendering a single
		 * sub-mesh. If number of materials is larger than number of sub-meshes, they will be ignored. If lower, the
		 * remaining materials will be removed.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Materials), Property(Setter))
		void SetMaterials(const Vector<MaterialType>& materials);

		/**
		 * Determines the layer bitfield that controls whether a renderable is considered visible in a specific camera.
		 * Renderable layer must match camera layer in order for the camera to render the component.
		 */
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

		/**
		 * If enabled this renderable will write per-pixel velocity information when rendered. This is required for effects
		 * such as temporal anti-aliasing and motion blur, but comes with a minor performance overhead. If you are not using
		 * those effects you can disable this for a performance gain.
		 */
		B3D_SCRIPT_EXPORT(ExportName(WriteVelocity), Property(Setter))
		void SetWriteVelocity(bool enable);

		/** @copydoc SetWriteVelocity */
		B3D_SCRIPT_EXPORT(ExportName(WriteVelocity), Property(Getter))
		bool GetWriteVelocity() const { return mWriteVelocity; }

		/** Factor to be applied to the cull distance set in the camera's render settings.  */
		B3D_SCRIPT_EXPORT(ExportName(CullDistance), Property(Setter))
		void SetCullDistanceFactor(float factor);

		/** @copydoc SetCullDistanceFactor() */
		B3D_SCRIPT_EXPORT(ExportName(CullDistance), Property(Getter))
		float GetCullDistanceFactor() const { return mCullDistanceFactor; }

		/** @copydoc SetLayer() */
		B3D_SCRIPT_EXPORT(ExportName(Layers), Property(Getter))
		u64 GetLayer() const { return mLayer; }

		/**	Returns the transform matrix that is applied to the object when its being rendered. */
		Matrix4 GetWorldTransformMatrix() const { return mWorldTransformMatrix; }

		/**
		 * Returns the transform matrix that is applied to the object when its being rendered. This transform matrix does
		 * not include scale values.
		 */
		Matrix4 GetWorldTransformMatrixWithoutScale() const { return mWorldTransformMatrixWithoutScale; }

	protected:
		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/** @copydoc CoreObject::MarkDependenciesDirty */
		void MarkCoreObjectDependenciesDirty();

		/** @copydoc IResourceListener::MarkListenerResourcesDirty */
		void MarkReferencedResourcesDirty();

		/** Triggered whenever the renderable's mesh changes. */
		virtual void DoOnMeshChanged() {}

		MeshType mMesh;
		Vector<MaterialType> mMaterials;
		u64 mLayer = 1;
		AABox mOverrideBounds;
		bool mUseOverrideBounds = false;
		bool mWriteVelocity = true;
		float mCullDistanceFactor = 1.0f;
		Matrix4 mWorldTransformMatrix = BsIdentity;
		Matrix4 mWorldTransformMatrixWithoutScale = BsIdentity;
		RenderableAnimType mAnimType = RenderableAnimType::None;
	};

	/**
	 * Renderable represents any visible object in the scene. It has a mesh, bounds and a set of materials. Renderer will
	 * render any Renderable objects visible by a camera.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Renderable : public Component, public TRenderable<false>
	{
	public:
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
		
		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		void OnDependencyDirty(CoreObject* dependency, u32 dirtyFlags) override;

		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override;
		void NotifyResourceChanged(const HResource& resource) override;
	private:
		HAnimation mAnimation;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;
		friend class render::Renderable;
		struct FullSyncPacket;
		struct TransformSyncPacket;

		Renderable(const HSceneObject& parent);

		void Initialize() override;
		void OnCreated() override;
		void OnBeginPlay() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnDestroyed() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

		void DoOnMeshChanged() override;

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
		/** @copydoc TRenderable */
		class B3D_EXPORT Renderable : public TRenderable<true>
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

			/** Returns the GPU buffer containing element's bone matrices, if it has any. */
			const SPtr<GpuBuffer>& GetBoneMatrixBuffer() const { return mBoneMatrixBuffer; }

			/** Returns the GPU buffer containing element's bone matrices for the previous frame, if it has any. */
			const SPtr<GpuBuffer>& GetPreviousBoneMatrixBuffer() const { return mPreviousBoneMatrixBuffer; }

			/** Returns the vertex buffer containing element's morph shape vertices, if it has any. */
			const SPtr<GpuBuffer>& GetMorphShapeBuffer() const { return mMorphShapeBuffer; }

			/** Returns vertex declaration used for rendering meshes containing morph shape information. */
			const SPtr<VertexDescription>& GetMorphVertexDescription() const { return mMorphVertexDescription; }

		protected:
			friend class b3d::Renderable;

			Renderable(const SPtr<SceneInstance>& scene);
			Renderable() = default;

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			/** Creates any buffers required for renderable animation. Should be called whenever animation properties change. */
			void CreateAnimationBuffers();

			u32 mRendererId = 0;
			u64 mAnimationId = (u64)-1;
			u32 mMorphShapeVersion = 0;

			SPtr<GpuBuffer> mBoneMatrixBuffer;
			SPtr<GpuBuffer> mPreviousBoneMatrixBuffer;
			SPtr<GpuBuffer> mMorphShapeBuffer;
			SPtr<VertexDescription> mMorphVertexDescription;

			Transform mTransform;
			bool mActive = true;
			SPtr<SceneInstance> mSceneInstance;
		};
	} // namespace render

	/** @} */
} // namespace b3d
