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

using namespace bs;

template <class T>
bool isMeshValid(const T& mesh)
{
	return false;
}

template <>
bool isMeshValid(const HMesh& mesh)
{
	return mesh.IsLoaded();
}

template <>
bool isMeshValid(const SPtr<ct::Mesh>& mesh)
{
	return mesh != nullptr;
}

template <bool Core>
TRenderable<Core>::TRenderable()
{
	mMaterials.resize(1);
}

template <bool Core>
void TRenderable<Core>::SetTransform(const Transform& transform)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	mTransform = transform;
	mTfrmMatrix = transform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(transform.GetPosition(), transform.GetRotation(), Vector3::kOne);

	MarkCoreDirtyInternal(ActorDirtyFlag::Transform);
}

template <bool Core>
void TRenderable<Core>::SetMesh(const MeshType& mesh)
{
	mMesh = mesh;

	int numSubMeshes = 0;
	if(isMeshValid(mesh))
		numSubMeshes = mesh->GetProperties().GetNumSubMeshes();

	mMaterials.resize(numSubMeshes);

	OnMeshChanged();

	MarkDependenciesDirtyInternal();
	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();
}

template <bool Core>
void TRenderable<Core>::SetMaterial(u32 idx, const MaterialType& material)
{
	if(idx >= (u32)mMaterials.size())
		return;

	mMaterials[idx] = material;

	MarkDependenciesDirtyInternal();
	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();
}

template <bool Core>
void TRenderable<Core>::SetMaterials(const Vector<MaterialType>& materials)
{
	u32 numMaterials = (u32)mMaterials.size();
	u32 min = std::min(numMaterials, (u32)materials.size());

	for(u32 i = 0; i < min; i++)
		mMaterials[i] = materials[i];

	for(u32 i = min; i < numMaterials; i++)
		mMaterials[i] = nullptr;

	MarkDependenciesDirtyInternal();
	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();
}

template <bool Core>
void TRenderable<Core>::SetMaterial(const MaterialType& material)
{
	SetMaterial(0, material);
}

template <bool Core>
typename TRenderable<Core>::MaterialType TRenderable<Core>::GetMaterial(u32 idx) const
{
	if(idx >= (u32)mMaterials.size())
		return nullptr;

	return mMaterials[idx];
}

template <bool Core>
void TRenderable<Core>::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		BS_LOG(Warning, Renderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	MarkCoreDirtyInternal();
}

template <bool Core>
void TRenderable<Core>::SetOverrideBounds(const AABox& bounds)
{
	mOverrideBounds = bounds;

	if(mUseOverrideBounds)
		MarkCoreDirtyInternal();
}

template <bool Core>
void TRenderable<Core>::SetUseOverrideBounds(bool enable)
{
	if(mUseOverrideBounds == enable)
		return;

	mUseOverrideBounds = enable;
	MarkCoreDirtyInternal();
}

template <bool Core>
void TRenderable<Core>::SetWriteVelocity(bool enable)
{
	if(mWriteVelocity == enable)
		return;

	mWriteVelocity = enable;
	MarkCoreDirtyInternal();
}

template <bool Core>
void TRenderable<Core>::SetCullDistanceFactor(float factor)
{
	mCullDistanceFactor = factor;

	MarkCoreDirtyInternal();
}

template class TRenderable<false>;
template class TRenderable<true>;

void Renderable::Initialize()
{
	CoreObject::Initialize();

	// Since we don't pass any information along to the core thread object on its construction, make sure the data
	// sync executes
	MarkCoreDirtyInternal();

	// If any resources were deserialized before initialization, make sure the listener is notified
	MarkResourcesDirtyInternal();
}

void Renderable::SetAnimation(const SPtr<Animation>& animation)
{
	mAnimation = animation;
	RefreshAnimation();

	MarkCoreDirtyInternal();
}

Bounds Renderable::GetBounds() const
{
	if(mUseOverrideBounds)
	{
		Sphere sphere(mOverrideBounds.GetCenter(), mOverrideBounds.GetRadius());

		Bounds bounds(mOverrideBounds, sphere);
		bounds.TransformAffine(mTfrmMatrix);

		return bounds;
	}

	HMesh mesh = GetMesh();

	if(!mesh.IsLoaded())
	{
		const Transform& tfrm = GetTransform();

		AABox box(tfrm.GetPosition(), tfrm.GetPosition());
		Sphere sphere(tfrm.GetPosition(), 0.0f);

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
	ct::Renderable* handler = new(B3DAllocate<ct::Renderable>()) ct::Renderable();
	SPtr<ct::Renderable> handlerPtr = B3DMakeSharedFromExisting<ct::Renderable>(handler);
	handlerPtr->SetThisPtrInternal(handlerPtr);

	return handlerPtr;
}

void Renderable::OnMeshChanged()
{
	RefreshAnimation();
}

void Renderable::RefreshAnimation()
{
	if(mAnimation == nullptr)
	{
		mAnimType = RenderableAnimType::None;
		return;
	}

	if(mMesh.IsLoaded(false))
	{
		SPtr<Skeleton> skeleton = mMesh->GetSkeleton();
		SPtr<MorphShapes> morphShapes = mMesh->GetMorphShapes();

		if(skeleton != nullptr && morphShapes != nullptr)
			mAnimType = RenderableAnimType::SkinnedMorph;
		else if(skeleton != nullptr)
			mAnimType = RenderableAnimType::Skinned;
		else if(morphShapes != nullptr)
			mAnimType = RenderableAnimType::Morph;
		else
			mAnimType = RenderableAnimType::None;

		mAnimation->SetSkeleton(mMesh->GetSkeleton());
		mAnimation->SetMorphShapes(mMesh->GetMorphShapes());
	}
	else
	{
		mAnimType = RenderableAnimType::None;

		mAnimation->SetSkeleton(nullptr);
		mAnimation->SetMorphShapes(nullptr);
	}
}

void Renderable::UpdateStateInternal(const SceneObject& so, bool force)
{
	u32 curHash = so.GetTransformHash();
	if(curHash != mHash || force)
	{
		// If skinned animation, don't include own transform since that will be handled by root bone animation
		bool ignoreOwnTransform;
		if(mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
			ignoreOwnTransform = mAnimation->GetAnimatesRootInternal();
		else
			ignoreOwnTransform = false;

		if(ignoreOwnTransform)
		{
			// Note: Technically we're checking child's hash but using parent's transform. Ideally we check the parent's
			// hash to reduce the number of required updates.
			HSceneObject parentSO = so.GetParent();
			if(parentSO != nullptr)
				SetTransform(parentSO->GetTransform());
			else
				SetTransform(Transform());
		}
		else
			SetTransform(so.GetTransform());

		mHash = curHash;
	}

	// Hash now matches so transform won't be applied twice, so we can just call base class version
	SceneActor::UpdateStateInternal(so, force);
}

void Renderable::MarkCoreDirtyInternal(ActorDirtyFlag flag)
{
	MarkCoreDirty((u32)flag);
}

void Renderable::MarkDependenciesDirtyInternal()
{
	MarkDependenciesDirty();
}

void Renderable::MarkResourcesDirtyInternal()
{
	MarkListenerResourcesDirty();
}

CoreSyncData Renderable::SyncToCore(FrameAlloc* allocator)
{
	const u32 dirtyFlags = GetCoreDirtyFlags();
	u32 size = B3DRTTISize(dirtyFlags).Bytes;
	SceneActor::RttiEnumFields(RttiCoreSyncSize(size), (ActorDirtyFlags)dirtyFlags);

	// The most common case if only the transform changed, so we sync only transform related options
	u32 numMaterials = 0;
	u64 animationId = 0;
	if(dirtyFlags != (u32)ActorDirtyFlag::Transform)
	{
		numMaterials = (u32)mMaterials.size();

		if(mAnimation != nullptr)
			animationId = mAnimation->GetIdInternal();
		else
			animationId = (u64)-1;

		size +=
			B3DRTTISize(mLayer).Bytes +
			B3DRTTISize(mOverrideBounds).Bytes +
			B3DRTTISize(mUseOverrideBounds).Bytes +
			B3DRTTISize(mWriteVelocity).Bytes +
			B3DRTTISize(numMaterials).Bytes +
			B3DRTTISize(animationId).Bytes +
			B3DRTTISize(mAnimType).Bytes +
			B3DRTTISize(mCullDistanceFactor).Bytes +
			sizeof(SPtr<ct::Mesh>) +
			numMaterials * sizeof(SPtr<ct::Material>);
	}

	u8* data = allocator->Alloc(size);
	Bitstream stream(data, size);

	B3DRTTIWrite(dirtyFlags, stream);
	SceneActor::RttiEnumFields(RttiCoreSyncWriter(stream), (ActorDirtyFlags)dirtyFlags);

	if(dirtyFlags != (u32)ActorDirtyFlag::Transform)
	{
		B3DRTTIWrite(mLayer, stream);
		B3DRTTIWrite(mOverrideBounds, stream);
		B3DRTTIWrite(mUseOverrideBounds, stream);
		B3DRTTIWrite(mWriteVelocity, stream);
		B3DRTTIWrite(numMaterials, stream);
		B3DRTTIWrite(animationId, stream);
		B3DRTTIWrite(mAnimType, stream);
		B3DRTTIWrite(mCullDistanceFactor, stream);

		SPtr<ct::Mesh>* mesh = new(stream.Cursor()) SPtr<ct::Mesh>();
		if(mMesh.IsLoaded())
			*mesh = mMesh->GetCore();

		stream.SkipBytes(sizeof(SPtr<ct::Mesh>));

		for(u32 i = 0; i < numMaterials; i++)
		{
			SPtr<ct::Material>* material = new(stream.Cursor()) SPtr<ct::Material>();
			if(mMaterials[i].IsLoaded())
				*material = mMaterials[i]->GetCore();

			stream.SkipBytes(sizeof(SPtr<ct::Material>));
		}
	}

	return CoreSyncData(data, size);
}

void Renderable::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mMesh.IsLoaded())
		dependencies.push_back(mMesh.Get());

	for(auto& material : mMaterials)
	{
		if(material.IsLoaded())
			dependencies.push_back(material.Get());
	}
}

void Renderable::OnDependencyDirty(CoreObject* dependency, u32 dirtyFlags)
{
	if(mMesh.IsLoaded(false) && mMesh.Get() == dependency)
	{
		CoreObject::OnDependencyDirty(dependency, dirtyFlags);
		return;
	}

	if(((u32)MaterialDirtyFlags::Shader & dirtyFlags) != 0)
		CoreObject::OnDependencyDirty(dependency, dirtyFlags);
}

void Renderable::GetListenerResources(Vector<HResource>& resources)
{
	if(mMesh != nullptr)
		resources.push_back(mMesh);

	for(auto& material : mMaterials)
	{
		if(material != nullptr)
			resources.push_back(material);
	}
}

void Renderable::NotifyResourceLoaded(const HResource& resource)
{
	if(resource == mMesh)
		OnMeshChanged();

	MarkDependenciesDirty();
	MarkCoreDirty();
}

void Renderable::NotifyResourceChanged(const HResource& resource)
{
	if(resource == mMesh)
		OnMeshChanged();

	MarkDependenciesDirty();
	MarkCoreDirty();
}

SPtr<Renderable> Renderable::Create()
{
	SPtr<Renderable> handlerPtr = CreateEmpty();
	handlerPtr->Initialize();

	return handlerPtr;
}

SPtr<Renderable> Renderable::CreateEmpty()
{
	Renderable* handler = new(B3DAllocate<Renderable>()) Renderable();
	SPtr<Renderable> handlerPtr = B3DMakeCoreFromExisting<Renderable>(handler);
	handlerPtr->SetThisPtrInternal(handlerPtr);

	return handlerPtr;
}

RTTITypeBase* Renderable::GetRttiStatic()
{
	return RenderableRTTI::Instance();
}

RTTITypeBase* Renderable::GetRtti() const
{
	return Renderable::GetRttiStatic();
}

namespace bs { namespace ct
{
Renderable::Renderable()
	: mRendererId(0), mAnimationId((u64)-1), mMorphShapeVersion(0)
{
}

Renderable::~Renderable()
{
	if(mActive)
		GetRenderer()->NotifyRenderableRemoved(this);
}

void Renderable::Initialize()
{
	GetRenderer()->NotifyRenderableAdded(this);

	CoreObject::Initialize();
}

Bounds Renderable::GetBounds() const
{
	if(mUseOverrideBounds)
	{
		Sphere sphere(mOverrideBounds.GetCenter(), mOverrideBounds.GetRadius());

		Bounds bounds(mOverrideBounds, sphere);
		bounds.TransformAffine(mTfrmMatrix);

		return bounds;
	}

	SPtr<Mesh> mesh = GetMesh();

	if(mesh == nullptr)
	{
		const Transform& tfrm = GetTransform();

		AABox box(tfrm.GetPosition(), tfrm.GetPosition());
		Sphere sphere(tfrm.GetPosition(), 0.0f);

		return Bounds(box, sphere);
	}
	else
	{
		Bounds bounds = mesh->GetProperties().GetBounds();
		bounds.TransformAffine(mTfrmMatrix);

		return bounds;
	}
}

SPtr<GpuBuffer> createBoneMatrixBuffer(u32 numBones)
{
	GPU_BUFFER_DESC desc;
	desc.ElementCount = numBones * 3;
	desc.ElementSize = 0;
	desc.Type = GBT_STANDARD;
	desc.Format = BF_32X4F;
	desc.Usage = GBU_DYNAMIC;

	SPtr<GpuBuffer> buffer = GpuBuffer::Create(desc);
	u8* dest = (u8*)buffer->Lock(0, numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);

	// Initialize bone transforms to identity, so the object renders properly even if no animation is animating it
	for(u32 i = 0; i < numBones; i++)
	{
		memcpy(dest, &Matrix4::kIdentity, 12 * sizeof(float)); // Assuming row-major format
		dest += 12 * sizeof(float);
	}

	buffer->Unlock();

	return buffer;
}

void Renderable::CreateAnimationBuffers()
{
	if(mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
	{
		SPtr<Skeleton> skeleton = mMesh->GetSkeleton();
		u32 numBones = skeleton != nullptr ? skeleton->GetNumBones() : 0;

		if(numBones > 0)
		{
			mBoneMatrixBuffer = createBoneMatrixBuffer(numBones);

			if(mWriteVelocity)
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

	if(mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
	{
		// Note: Not handling velocity writing for morph animations

		SPtr<MorphShapes> morphShapes = mMesh->GetMorphShapes();

		u32 vertexSize = sizeof(Vector3) + sizeof(u32);
		u32 numVertices = morphShapes->GetNumVertices();

		VERTEX_BUFFER_DESC desc;
		desc.VertexSize = vertexSize;
		desc.NumVerts = numVertices;
		desc.Usage = GBU_DYNAMIC;

		SPtr<VertexBuffer> vertexBuffer = VertexBuffer::Create(desc);

		u32 totalSize = vertexSize * numVertices;
		u8* dest = (u8*)vertexBuffer->Lock(0, totalSize, GBL_WRITE_ONLY_DISCARD);
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
	if(mAnimationId == (u64)-1)
		return;

	const EvaluatedAnimationData::AnimInfo* animInfo = nullptr;

	auto iterFind = animData.Infos.find(mAnimationId);
	if(iterFind != animData.Infos.end())
		animInfo = &iterFind->second;

	if(animInfo == nullptr)
		return;

	if(mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
	{
		const EvaluatedAnimationData::PoseInfo& poseInfo = animInfo->PoseInfo;

		if(mWriteVelocity)
			std::swap(mBoneMatrixBuffer, mBonePrevMatrixBuffer);

		// Note: If multiple elements are using the same animation (not possible atm), this buffer should be shared by
		// all such elements
		u8* dest = (u8*)mBoneMatrixBuffer->Lock(0, poseInfo.NumBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);
		for(u32 j = 0; j < poseInfo.NumBones; j++)
		{
			const Matrix4& transform = animData.Transforms[poseInfo.StartIdx + j];
			memcpy(dest, &transform, 12 * sizeof(float)); // Assuming row-major format

			dest += 12 * sizeof(float);
		}

		mBoneMatrixBuffer->Unlock();
	}

	if(mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
	{
		if(mMorphShapeVersion != animInfo->MorphShapeInfo.Version)
		{
			SPtr<MeshData> meshData = animInfo->MorphShapeInfo.MeshData;

			u32 bufferSize = meshData->GetSize();
			u8* data = meshData->GetData();

			mMorphShapeBuffer->WriteData(0, bufferSize, data, BWT_DISCARD);
			mMorphShapeVersion = animInfo->MorphShapeInfo.Version;
		}
	}
}

void Renderable::UpdatePrevFrameAnimationBuffers()
{
	if(!mWriteVelocity)
		return;

	if(mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
		std::swap(mBoneMatrixBuffer, mBonePrevMatrixBuffer);
}

void Renderable::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	mMaterials.clear();

	u32 numMaterials = 0;
	u32 dirtyFlags = 0;
	bool oldIsActive = mActive;

	B3DRTTIRead(dirtyFlags, stream);
	SceneActor::RttiEnumFields(RttiCoreSyncReader(stream), (ActorDirtyFlags)dirtyFlags);

	mTfrmMatrix = mTransform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(mTransform.GetPosition(), mTransform.GetRotation(), Vector3::kOne);

	if(dirtyFlags != (u32)ActorDirtyFlag::Transform)
	{
		B3DRTTIRead(mLayer, stream);
		B3DRTTIRead(mOverrideBounds, stream);
		B3DRTTIRead(mUseOverrideBounds, stream);
		B3DRTTIRead(mWriteVelocity, stream);
		B3DRTTIRead(numMaterials, stream);
		B3DRTTIRead(mAnimationId, stream);
		B3DRTTIRead(mAnimType, stream);
		B3DRTTIRead(mCullDistanceFactor, stream);

		SPtr<Mesh>* mesh = (SPtr<Mesh>*)stream.Cursor();
		mMesh = *mesh;
		mesh->~SPtr<Mesh>();
		stream.SkipBytes(sizeof(SPtr<Mesh>));

		for(u32 i = 0; i < numMaterials; i++)
		{
			SPtr<Material>* material = (SPtr<Material>*)stream.Cursor();
			mMaterials.push_back(*material);
			material->~SPtr<Material>();
			stream.SkipBytes(sizeof(SPtr<Material>));
		}
	}

	u32 updateEverythingFlag = (u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active | (u32)ActorDirtyFlag::Dependency;

	if((dirtyFlags & updateEverythingFlag) != 0)
	{
		CreateAnimationBuffers();

		// Create special vertex declaration if using morph shapes
		if(mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::Create();
			*vertexDesc = *mMesh->GetVertexDesc();

			vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION, 1, 1);
			vertexDesc->AddVertElem(VET_UBYTE4_NORM, VES_NORMAL, 1, 1);

			mMorphVertexDeclaration = VertexDeclaration::Create(vertexDesc);
		}
		else
			mMorphVertexDeclaration = nullptr;

		if(oldIsActive != mActive)
		{
			if(mActive)
				GetRenderer()->NotifyRenderableAdded(this);
			else
				GetRenderer()->NotifyRenderableRemoved(this);
		}
		else
		{
			GetRenderer()->NotifyRenderableRemoved(this);
			GetRenderer()->NotifyRenderableAdded(this);
		}
	}
	else if((dirtyFlags & (u32)ActorDirtyFlag::Mobility) != 0)
	{
		GetRenderer()->NotifyRenderableRemoved(this);
		GetRenderer()->NotifyRenderableAdded(this);
	}
	else if((dirtyFlags & (u32)ActorDirtyFlag::Transform) != 0)
	{
		if(mActive)
			GetRenderer()->NotifyRenderableUpdated(this);
	}
}
}}
