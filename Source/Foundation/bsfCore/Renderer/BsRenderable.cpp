//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderable.h"
#include "Private/RTTI/BsRenderableRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Mesh/BsMesh.h"
#include "Material/BsMaterial.h"
#include "Math/BsBounds.h"
#include "Renderer/BsRenderer.h"
#include "Animation/BsAnimation.h"
#include "Animation/BsMorphShapes.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Animation/BsAnimationManager.h"
#include "Scene/BsSceneManager.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	template<class T>
	bool IsMeshValid(const T& mesh) { return false; }

	template<>
	bool IsMeshValid(const HMesh& mesh) { return mesh.IsLoaded(); }

	template<>
	bool IsMeshValid(const SPtr<ct::Mesh>& mesh) { return mesh != nullptr; }

	template<bool Core>
	TRenderable<Core>::TRenderable()
	{
		mMaterials.Resize(1);
	}

	template <bool Core>
	void TRenderable<Core>::setTransform(const Transform& transform)
	{
		if (mMobility != ObjectMobility::Movable)
			return;

		mTransform = transform;
		mTfrmMatrix = transform.GetMatrix();
		mTfrmMatrixNoScale = Matrix4::TRS(transform.GetPosition(), transform.getRotation(), Vector3::ONE);

		_markCoreDirty(ActorDirtyFlag::Transform);
	}

	template<bool Core>
	void TRenderable<Core>::setMesh(const MeshType& mesh)
	{
		mMesh = mesh;

		int numSubMeshes = 0;
		if (isMeshValid(mesh))
			numSubMeshes = mesh->GetProperties().GetNumSubMeshes();

		mMaterials.Resize(numSubMeshes);

		onMeshChanged();

		_markDependenciesDirty();
		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setMaterial(UINT32 idx, const MaterialType& material)
	{
		if (idx >= (UINT32)mMaterials.size())
			return;

		mMaterials[idx] = material;

		_markDependenciesDirty();
		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setMaterials(const Vector<MaterialType>& materials)
	{
		UINT32 numMaterials = (UINT32)mMaterials.size();
		UINT32 min = std::min(numMaterials, (UINT32)materials.size());

		for (UINT32 i = 0; i < min; i++)
			mMaterials[i] = materials[i];

		for (UINT32 i = min; i < numMaterials; i++)
			mMaterials[i] = nullptr;

		_markDependenciesDirty();
		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setMaterial(const MaterialType& material)
	{
		setMaterial(0, material);
	}

	template <bool Core>
	typename TRenderable<Core>::MaterialType TRenderable<Core>::getMaterial(UINT32 idx) const
	{
		if(idx >= (UINT32)mMaterials.size())
			return nullptr;

		return mMaterials[idx];
	}

	template<bool Core>
	void TRenderable<Core>::setLayer(UINT64 layer)
	{
		const bool isPow2 = layer && !((layer - 1) & layer);

		if (!isPow2)
		{
			BS_LOG(Warning, Renderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
			return;
		}

		mLayer = layer;
		_markCoreDirty();
	}	
	
	template<bool Core>
	void TRenderable<Core>::setOverrideBounds(const AABox& bounds)
	{
		mOverrideBounds = bounds;

		if(mUseOverrideBounds)
			_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setUseOverrideBounds(bool enable)
	{
		if (mUseOverrideBounds == enable)
			return;

		mUseOverrideBounds = enable;
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setWriteVelocity(bool enable)
	{
		if (mWriteVelocity == enable)
			return;

		mWriteVelocity = enable;
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setCullDistanceFactor(float factor)
	{
		mCullDistanceFactor = factor;

		_markCoreDirty();
	}

	template class TRenderable < false >;
	template class TRenderable < true >;

	void Renderable::Initialize()
	{
		CoreObject::initialize();		

		// Since we don't pass any information along to the core thread object on its construction, make sure the data
		// sync executes
		_markCoreDirty();

		// If any resources were deserialized before initialization, make sure the listener is notified
		_markResourcesDirty();
	}


	void Renderable::SetAnimation(const SPtr<Animation>& animation)
	{
		mAnimation = animation;
		refreshAnimation();

		_markCoreDirty();
	}

	Bounds Renderable::GetBounds() const
	{
		if(mUseOverrideBounds)
		{
			Sphere Sphere(mOverrideBounds.GetCenter(), mOverrideBounds.getRadius());

			Bounds Bounds(mOverrideBounds, sphere);
			bounds.TransformAffine(mTfrmMatrix);

			return bounds;
		}

		HMesh mesh = getMesh();

		if (!mesh.IsLoaded())
		{
			const Transform& tfrm = getTransform();

			AABox Box(tfrm.GetPosition(), tfrm.getPosition());
			Sphere Sphere(tfrm.GetPosition(), 0.0f);

			return Bounds(box, sphere);
		}
		else
		{
			Bounds bounds = mesh->GetProperties().GetBounds();
			bounds.TransformAffine(mTfrmMatrix);

			return bounds;
		}
	}

	SPtr<ct::Renderable> Renderable::GetCore() const
	{
		return std::static_pointer_cast<ct::Renderable>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Renderable::CreateCore() const
	{
		ct::Renderable* handler = new (bs_alloc<ct::Renderable>()) ct::Renderable();
		SPtr<ct::Renderable> handlerPtr = bs_shared_ptr<ct::Renderable>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	void Renderable::OnMeshChanged()
	{
		refreshAnimation();
	}

	void Renderable::RefreshAnimation()
	{
		if (mAnimation == nullptr)
		{
			mAnimType = RenderableAnimType::None;
			return;
		}

		if (mMesh.IsLoaded(false))
		{
			SPtr<Skeleton> skeleton = mMesh->GetSkeleton();
			SPtr<MorphShapes> morphShapes = mMesh->GetMorphShapes();

			if (skeleton != nullptr && morphShapes != nullptr)
				mAnimType = RenderableAnimType::SkinnedMorph;
			else if (skeleton != nullptr)
				mAnimType = RenderableAnimType::Skinned;
			else if (morphShapes != nullptr)
				mAnimType = RenderableAnimType::Morph;
			else
				mAnimType = RenderableAnimType::None;

			mAnimation->SetSkeleton(mMesh->getSkeleton());
			mAnimation->SetMorphShapes(mMesh->getMorphShapes());
		}
		else
		{
			mAnimType = RenderableAnimType::None;

			mAnimation->SetSkeleton(nullptr);
			mAnimation->SetMorphShapes(nullptr);
		}
	}

	void Renderable::_updateState(const SceneObject& so, bool force)
	{
		UINT32 curHash = so.GetTransformHash();
		if (curHash != mHash || force)
		{
			// If skinned animation, don't include own transform since that will be handled by root bone animation
			bool ignoreOwnTransform;
			if (mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
				ignoreOwnTransform = mAnimation->_getAnimatesRoot();
			else
				ignoreOwnTransform = false;

			if (ignoreOwnTransform)
			{
				// Note: Technically we're checking child's hash but using parent's transform. Ideally we check the parent's
				// hash to reduce the number of required updates.
				HSceneObject parentSO = so.GetParent();
				if (parentSO != nullptr)
					setTransform(parentSO->GetTransform());
				else
					setTransform(Transform());
			}
			else
				setTransform(so.GetTransform());

			mHash = curHash;
		}

		// Hash now matches so transform won't be applied twice, so we can just call base class version
		SceneActor::_updateState(so, force);
	}

	void Renderable::_markCoreDirty(ActorDirtyFlag flag)
	{
		markCoreDirty((UINT32)flag);
	}

	void Renderable::_markDependenciesDirty()
	{
		markDependenciesDirty();
	}

	void Renderable::_markResourcesDirty()
	{
		markListenerResourcesDirty();
	}

	CoreSyncData Renderable::SyncToCore(FrameAlloc* allocator)
	{
		const UINT32 dirtyFlags = getCoreDirtyFlags();
		UINT32 size = rtti_size(dirtyFlags).bytes;
		SceneActor::rttiEnumFields(RttiCoreSyncSize(size), (ActorDirtyFlags)dirtyFlags);

		// The most common case if only the transform changed, so we sync only transform related options
		UINT32 numMaterials = 0;
		UINT64 animationId = 0;
		if(dirtyFlags != (UINT32)ActorDirtyFlag::Transform)
		{
			numMaterials = (UINT32)mMaterials.size();

			if (mAnimation != nullptr)
				animationId = mAnimation->_getId();
			else
				animationId = (UINT64)-1;

			size +=
				rtti_size(mLayer).bytes +
				rtti_size(mOverrideBounds).bytes +
				rtti_size(mUseOverrideBounds).bytes +
				rtti_size(mWriteVelocity).bytes +
				rtti_size(numMaterials).bytes +
				rtti_size(animationId).bytes +
				rtti_size(mAnimType).bytes +
				rtti_size(mCullDistanceFactor).bytes +
				sizeof(SPtr<ct::Mesh>) +
				numMaterials * sizeof(SPtr<ct::Material>);
		}


		UINT8* data = allocator->Alloc(size);
		Bitstream Stream(data, size);

		rtti_write(dirtyFlags, stream);
		SceneActor::rttiEnumFields(RttiCoreSyncWriter(stream), (ActorDirtyFlags)dirtyFlags);

		if(dirtyFlags != (UINT32)ActorDirtyFlag::Transform)
		{
			rtti_write(mLayer, stream);
			rtti_write(mOverrideBounds, stream);
			rtti_write(mUseOverrideBounds, stream);
			rtti_write(mWriteVelocity, stream);
			rtti_write(numMaterials, stream);
			rtti_write(animationId, stream);
			rtti_write(mAnimType, stream);
			rtti_write(mCullDistanceFactor, stream);

			SPtr<ct::Mesh>* mesh = new (stream.Cursor()) SPtr<ct::Mesh>();
			if (mMesh.IsLoaded())
				*mesh = mMesh->GetCore();

			stream.SkipBytes(sizeof(SPtr<ct::Mesh>));

			for (UINT32 i = 0; i < numMaterials; i++)
			{
				SPtr<ct::Material>* material = new (stream.Cursor())SPtr<ct::Material>();
				if (mMaterials[i].IsLoaded())
					*material = mMaterials[i]->GetCore();

				stream.SkipBytes(sizeof(SPtr<ct::Material>));
			}
		}

		return CoreSyncData(data, size);
	}

	void Renderable::GetCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mMesh.IsLoaded())
			dependencies.push_back(mMesh.Get());

		for (auto& material : mMaterials)
		{
			if (material.IsLoaded())
				dependencies.push_back(material.Get());
		}
	}

	void Renderable::OnDependencyDirty(CoreObject* dependency, UINT32 dirtyFlags)
	{
		if(mMesh.IsLoaded(false) && mMesh.get() == dependency)
		{
			CoreObject::onDependencyDirty(dependency, dirtyFlags);
			return;
		}

		if(((UINT32)MaterialDirtyFlags::Shader & dirtyFlags) != 0)
			CoreObject::onDependencyDirty(dependency, dirtyFlags);
	}

	void Renderable::GetListenerResources(Vector<HResource>& resources)
	{
		if (mMesh != nullptr)
			resources.push_back(mMesh);

		for (auto& material : mMaterials)
		{
			if (material != nullptr)
				resources.push_back(material);
		}
	}

	void Renderable::NotifyResourceLoaded(const HResource& resource)
	{
		if (resource == mMesh)
			onMeshChanged();

		markDependenciesDirty();
		markCoreDirty();
	}

	void Renderable::NotifyResourceChanged(const HResource& resource)
	{
		if(resource == mMesh)
			onMeshChanged();

		markDependenciesDirty();
		markCoreDirty();
	}

	SPtr<Renderable> Renderable::Create()
	{
		SPtr<Renderable> handlerPtr = createEmpty();
		handlerPtr->Initialize();

		return handlerPtr;
	}

	SPtr<Renderable> Renderable::CreateEmpty()
	{
		Renderable* handler = new (bs_alloc<Renderable>()) Renderable();
		SPtr<Renderable> handlerPtr = bs_core_ptr<Renderable>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	RTTITypeBase* Renderable::getRTTIStatic()
	{
		return RenderableRTTI::Instance();
	}

	RTTITypeBase* Renderable::getRTTI() const
	{
		return Renderable::GetRTTIStatic();
	}

	namespace ct
	{
	Renderable::Renderable()
		:mRendererId(0), mAnimationId((UINT64)-1), mMorphShapeVersion(0)
	{
	}

	Renderable::~Renderable()
	{
		if (mActive)
			gRenderer()->NotifyRenderableRemoved(this);
	}

	void Renderable::Initialize()
	{
		gRenderer()->NotifyRenderableAdded(this);

		CoreObject::initialize();
	}

	Bounds Renderable::GetBounds() const
	{
		if (mUseOverrideBounds)
		{
			Sphere Sphere(mOverrideBounds.GetCenter(), mOverrideBounds.getRadius());

			Bounds Bounds(mOverrideBounds, sphere);
			bounds.TransformAffine(mTfrmMatrix);

			return bounds;
		}

		SPtr<Mesh> mesh = getMesh();

		if (mesh == nullptr)
		{
			const Transform& tfrm = getTransform();

			AABox Box(tfrm.GetPosition(), tfrm.getPosition());
			Sphere Sphere(tfrm.GetPosition(), 0.0f);

			return Bounds(box, sphere);
		}
		else
		{
			Bounds bounds = mesh->GetProperties().GetBounds();
			bounds.TransformAffine(mTfrmMatrix);

			return bounds;
		}
	}

	SPtr<GpuBuffer> CreateBoneMatrixBuffer(UINT32 numBones)
	{
		GPU_BUFFER_DESC desc;
		desc.elementCount = numBones * 3;
		desc.elementSize = 0;
		desc.type = GBT_STANDARD;
		desc.format = BF_32X4F;
		desc.usage = GBU_DYNAMIC;

		SPtr<GpuBuffer> buffer = GpuBuffer::create(desc);
		UINT8* dest = (UINT8*)buffer->Lock(0, numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);

		// Initialize bone transforms to identity, so the object renders properly even if no animation is animating it
		for (UINT32 i = 0; i < numBones; i++)
		{
			memcpy(dest, &Matrix4::IDENTITY, 12 * sizeof(float)); // Assuming row-major format
			dest += 12 * sizeof(float);
		}

		buffer->Unlock();

		return buffer;
	}

	void Renderable::CreateAnimationBuffers()
	{
		if (mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			SPtr<Skeleton> skeleton = mMesh->GetSkeleton();
			UINT32 numBones = skeleton != nullptr ? skeleton->GetNumBones() : 0;

			if (numBones > 0)
			{
				mBoneMatrixBuffer = createBoneMatrixBuffer(numBones);

				if (mWriteVelocity)
					mBonePrevMatrixBuffer = createBoneMatrixBuffer(numBones);
				else
					mBonePrevMatrixBuffer = nullptr;
			}
			else
			{
				mBoneMatrixBuffer = nullptr;
				mBonePrevMatrixBuffer = nullptr;
			}
		}
		else
		{
			mBoneMatrixBuffer = nullptr;
			mBonePrevMatrixBuffer = nullptr;
		}

		if (mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			// Note: Not handling velocity writing for morph animations
			
			SPtr<MorphShapes> morphShapes = mMesh->GetMorphShapes();

			UINT32 vertexSize = sizeof(Vector3) + sizeof(UINT32);
			UINT32 numVertices = morphShapes->GetNumVertices();

			VERTEX_BUFFER_DESC desc;
			desc.vertexSize = vertexSize;
			desc.numVerts = numVertices;
			desc.usage = GBU_DYNAMIC;

			SPtr<VertexBuffer> vertexBuffer = VertexBuffer::create(desc);

			UINT32 totalSize = vertexSize * numVertices;
			UINT8* dest = (UINT8*)vertexBuffer->Lock(0, totalSize, GBL_WRITE_ONLY_DISCARD);
			memset(dest, 0, totalSize);
			vertexBuffer->Unlock();

			mMorphShapeBuffer = vertexBuffer;
		}
		else
			mMorphShapeBuffer = nullptr;

		mMorphShapeVersion = 0;
	}

	void Renderable::UpdateAnimationBuffers(const EvaluatedAnimationData& animData)
	{
		if (mAnimationId == (UINT64)-1)
			return;

		const EvaluatedAnimationData::AnimInfo* animInfo = nullptr;

		auto iterFind = animData.infos.find(mAnimationId);
		if (iterFind != animData.infos.end())
			animInfo = &iterFind->second;

		if (animInfo == nullptr)
			return;

		if (mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			const EvaluatedAnimationData::PoseInfo& poseInfo = animInfo->poseInfo;

			if (mWriteVelocity)
				std::swap(mBoneMatrixBuffer, mBonePrevMatrixBuffer);

			// Note: If multiple elements are using the same animation (not possible atm), this buffer should be shared by
			// all such elements
			UINT8* dest = (UINT8*)mBoneMatrixBuffer->Lock(0, poseInfo.numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);
			for (UINT32 j = 0; j < poseInfo.numBones; j++)
			{
				const Matrix4& transform = animData.transforms[poseInfo.startIdx + j];
				memcpy(dest, &transform, 12 * sizeof(float)); // Assuming row-major format

				dest += 12 * sizeof(float);
			}

			mBoneMatrixBuffer->Unlock();
		}

		if (mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			if (mMorphShapeVersion != animInfo->morphShapeInfo.version)
			{
				SPtr<MeshData> meshData = animInfo->morphShapeInfo.meshData;

				UINT32 bufferSize = meshData->GetSize();
				UINT8* data = meshData->GetData();

				mMorphShapeBuffer->WriteData(0, bufferSize, data, BWT_DISCARD);
				mMorphShapeVersion = animInfo->morphShapeInfo.version;
			}
		}
	}

	void Renderable::UpdatePrevFrameAnimationBuffers()
	{
		if (!mWriteVelocity)
			return;
		
		if (mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
			std::swap(mBoneMatrixBuffer, mBonePrevMatrixBuffer);
	}

	void Renderable::SyncToCore(const CoreSyncData& data)
	{
		Bitstream Stream(data.GetBuffer(), data.getBufferSize());

		mMaterials.Clear();

		UINT32 numMaterials = 0;
		UINT32 dirtyFlags = 0;
		bool oldIsActive = mActive;

		rtti_read(dirtyFlags, stream);
		SceneActor::rttiEnumFields(RttiCoreSyncReader(stream), (ActorDirtyFlags)dirtyFlags);

		mTfrmMatrix = mTransform.GetMatrix();
		mTfrmMatrixNoScale = Matrix4::TRS(mTransform.GetPosition(), mTransform.getRotation(), Vector3::ONE);

		if(dirtyFlags != (UINT32)ActorDirtyFlag::Transform)
		{
			rtti_read(mLayer, stream);
			rtti_read(mOverrideBounds, stream);
			rtti_read(mUseOverrideBounds, stream);
			rtti_read(mWriteVelocity, stream);
			rtti_read(numMaterials, stream);
			rtti_read(mAnimationId, stream);
			rtti_read(mAnimType, stream);
			rtti_read(mCullDistanceFactor, stream);

			SPtr<Mesh>* mesh = (SPtr<Mesh>*)stream.Cursor();
			mMesh = *mesh;
			mesh->~SPtr<Mesh>();
			stream.SkipBytes(sizeof(SPtr<Mesh>));

			for (UINT32 i = 0; i < numMaterials; i++)
			{
				SPtr<Material>* material = (SPtr<Material>*)stream.Cursor();
				mMaterials.push_back(*material);
				material->~SPtr<Material>();
				stream.SkipBytes(sizeof(SPtr<Material>));
			}
		}

		UINT32 updateEverythingFlag = (UINT32)ActorDirtyFlag::Everything
			| (UINT32)ActorDirtyFlag::Active
			| (UINT32)ActorDirtyFlag::Dependency;

		if((dirtyFlags & updateEverythingFlag) != 0)
		{
			createAnimationBuffers();

			// Create special vertex declaration if using morph shapes
			if (mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
			{
				SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
				*vertexDesc = * mMesh->GetVertexDesc();

				vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION, 1, 1);
				vertexDesc->AddVertElem(VET_UBYTE4_NORM, VES_NORMAL, 1, 1);

				mMorphVertexDeclaration = VertexDeclaration::create(vertexDesc);
			}
			else
				mMorphVertexDeclaration = nullptr;

			if (oldIsActive != mActive)
			{
				if (mActive)
					gRenderer()->NotifyRenderableAdded(this);
				else
					gRenderer()->NotifyRenderableRemoved(this);
			}
			else
			{
				gRenderer()->NotifyRenderableRemoved(this);
				gRenderer()->NotifyRenderableAdded(this);
			}
		}
		else if((dirtyFlags & (UINT32)ActorDirtyFlag::Mobility) != 0)
		{
				gRenderer()->NotifyRenderableRemoved(this);
				gRenderer()->NotifyRenderableAdded(this);
		}
		else if ((dirtyFlags & (UINT32)ActorDirtyFlag::Transform) != 0)
		{
			if (mActive)
				gRenderer()->NotifyRenderableUpdated(this);
		}
	}
	}
}
