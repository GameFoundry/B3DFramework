//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderable.h"

#include "BsCoreApplication.h"
#include "BsRendererScene.h"
#include "Private/RTTI/BsRenderableRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Mesh/BsMesh.h"
#include "Material/BsMaterial.h"
#include "Math/BsBounds.h"
#include "Renderer/BsRenderer.h"
#include "Animation/BsAnimation.h"
#include "Animation/BsMorphShapes.h"
#include "Animation/BsAnimationScene.h"
#include "Scene/BsSceneManager.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuDevice.h"

using namespace b3d;

template <class T>
bool IsMeshValid(const T& mesh)
{
	return false;
}

template <>
bool IsMeshValid(const HMesh& mesh)
{
	return mesh.IsLoaded();
}

template <>
bool IsMeshValid(const SPtr<render::Mesh>& mesh)
{
	return mesh != nullptr;
}

template <bool IsRenderProxy>
TRenderable<IsRenderProxy>::TRenderable(const SPtr<SceneInstanceType>& scene)
	:Super(scene)
{
	mMaterials.resize(1);
}

template <bool IsRenderProxy>
TRenderable<IsRenderProxy>::TRenderable()
	:TRenderable(nullptr)
{ }

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetTransform(const Transform& transform)
{
	if(this->mMobility != ObjectMobility::Movable)
		return;

	this->mTransform = transform;
	mTfrmMatrix = transform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(transform.GetPosition(), transform.GetRotation(), Vector3::kOne);

	this->MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag::Transform);
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetMesh(const MeshType& mesh)
{
	mMesh = mesh;

	u32 subMeshCount = 0;
	if(IsMeshValid(mesh))
		subMeshCount = (u32)mesh->GetProperties().SubMeshes.size();

	mMaterials.resize(subMeshCount);

	OnMeshChanged();

	MarkDependenciesDirtyInternal();
	MarkResourcesDirtyInternal();
	this->MarkSceneActorRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetMaterial(u32 idx, const MaterialType& material)
{
	if(idx >= (u32)mMaterials.size())
		return;

	mMaterials[idx] = material;

	MarkDependenciesDirtyInternal();
	MarkResourcesDirtyInternal();
	this->MarkSceneActorRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetMaterials(const Vector<MaterialType>& materials)
{
	u32 numMaterials = (u32)mMaterials.size();
	u32 min = std::min(numMaterials, (u32)materials.size());

	for(u32 i = 0; i < min; i++)
		mMaterials[i] = materials[i];

	for(u32 i = min; i < numMaterials; i++)
		mMaterials[i] = nullptr;

	MarkDependenciesDirtyInternal();
	MarkResourcesDirtyInternal();
	this->MarkSceneActorRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetMaterial(const MaterialType& material)
{
	SetMaterial(0, material);
}

template <bool IsRenderProxy>
typename TRenderable<IsRenderProxy>::MaterialType TRenderable<IsRenderProxy>::GetMaterial(u32 idx) const
{
	if(idx >= (u32)mMaterials.size())
		return nullptr;

	return mMaterials[idx];
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		B3D_LOG(Warning, Renderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	this->MarkSceneActorRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetOverrideBounds(const AABox& bounds)
{
	mOverrideBounds = bounds;

	if(mUseOverrideBounds)
		this->MarkSceneActorRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetUseOverrideBounds(bool enable)
{
	if(mUseOverrideBounds == enable)
		return;

	mUseOverrideBounds = enable;
	this->MarkSceneActorRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetWriteVelocity(bool enable)
{
	if(mWriteVelocity == enable)
		return;

	mWriteVelocity = enable;
	this->MarkSceneActorRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TRenderable<IsRenderProxy>::SetCullDistanceFactor(float factor)
{
	mCullDistanceFactor = factor;

	this->MarkSceneActorRenderProxyDataDirty();
}

template class TRenderable<false>;
template class TRenderable<true>;

Renderable::Renderable(const SPtr<SceneInstance>& sceneInstance)
	: TRenderable(sceneInstance)
{ }

void Renderable::Initialize()
{
	CoreObject::Initialize();

	// Since we don't pass any information along to the render thread object on its construction, make sure the data
	// sync executes
	MarkSceneActorRenderProxyDataDirty();

	// If any resources were deserialized before initialization, make sure the listener is notified
	MarkResourcesDirtyInternal();
}

void Renderable::SetAnimation(const SPtr<Animation>& animation)
{
	mAnimation = animation;
	RefreshAnimation();

	MarkSceneActorRenderProxyDataDirty();
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
		Bounds bounds = mesh->GetProperties().Bounds;
		bounds.TransformAffine(mTfrmMatrix);

		return bounds;
	}
}

SPtr<render::RenderProxy> Renderable::CreateRenderProxy() const
{
	render::Renderable* renderProxy = new(B3DAllocate<render::Renderable>()) render::Renderable(B3DGetRenderProxy(mSceneInstance));
	SPtr<render::Renderable> renderProxyShared = B3DMakeSharedFromExisting<render::Renderable>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
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

void Renderable::UpdateStateFromSceneObject(const SceneObject& so, bool force)
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
	SceneActor::UpdateStateFromSceneObject(so, force);
}

void Renderable::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flag)
{
	MarkRenderProxyDataDirty((u32)flag);
}

void Renderable::MarkDependenciesDirtyInternal()
{
	MarkDependenciesDirty();
}

void Renderable::MarkResourcesDirtyInternal()
{
	MarkListenerResourcesDirty();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Renderable, FullSyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mLayer)
		B3D_SYNC_BLOCK_ENTRY(mOverrideBounds)
		B3D_SYNC_BLOCK_ENTRY(mUseOverrideBounds)
		B3D_SYNC_BLOCK_ENTRY(mWriteVelocity)
		B3D_SYNC_BLOCK_ENTRY(mAnimType)
		B3D_SYNC_BLOCK_ENTRY(mCullDistanceFactor)
		B3D_SYNC_BLOCK_ENTRY(mMesh)
		B3D_SYNC_BLOCK_ENTRY(mMaterials)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(u64, mAnimationId)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(Renderable, ActorSyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* Renderable::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	RenderProxySyncPacket* const sceneActorSyncPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);
	if(flags != (u32)ActorDirtyFlag::Transform)
	{
		FullSyncPacket* renderableSyncPacket = allocator.Construct<FullSyncPacket>(*this, allocator, flags);
		renderableSyncPacket->mAnimationId = mAnimation != nullptr ? mAnimation->GetIdInternal() : (u64)-1;
		renderableSyncPacket->SceneActorPacket = sceneActorSyncPacket;

		return renderableSyncPacket;
	}
	else
	{
		ActorSyncPacket* actorSyncPacket = allocator.Construct<ActorSyncPacket>(*this, allocator, flags);
		actorSyncPacket->SceneActorPacket = sceneActorSyncPacket;

		return actorSyncPacket;
	}
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
	MarkRenderProxyDataDirty();
}

void Renderable::NotifyResourceChanged(const HResource& resource)
{
	if(resource == mMesh)
		OnMeshChanged();

	MarkDependenciesDirty();
	MarkRenderProxyDataDirty();
}

SPtr<Renderable> Renderable::Create(const SPtr<SceneInstance>& sceneInstance)
{
	Renderable* handler = new(B3DAllocate<Renderable>()) Renderable(sceneInstance);
	SPtr<Renderable> handlerPtr = B3DMakeSharedFromExisting<Renderable>(handler);
	handlerPtr->SetShared(handlerPtr);
	handlerPtr->Initialize();

	return handlerPtr;
}

SPtr<Renderable> Renderable::CreateEmpty()
{
	Renderable* handler = new(B3DAllocate<Renderable>()) Renderable();
	SPtr<Renderable> handlerPtr = B3DMakeSharedFromExisting<Renderable>(handler);
	handlerPtr->SetShared(handlerPtr);

	return handlerPtr;
}

RTTIType* Renderable::GetRttiStatic()
{
	return RenderableRTTI::Instance();
}

RTTIType* Renderable::GetRtti() const
{
	return Renderable::GetRttiStatic();
}

namespace b3d { namespace render
{
Renderable::Renderable(const SPtr<SceneInstance>& sceneInstance)
	: TRenderable(sceneInstance)
{ }

Renderable::~Renderable()
{
	if(mActive)
	{
		const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
		rendererScene->UnregisterRenderable(this);
	}
}

void Renderable::Initialize()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterRenderable(this);

	RenderProxy::Initialize();
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
		Bounds bounds = mesh->GetProperties().Bounds;
		bounds.TransformAffine(mTfrmMatrix);

		return bounds;
	}
}

SPtr<GpuBuffer> CreateBoneMatrixBuffer(u32 numBones)
{
	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	bufferCreateInformation.SimpleStorage.Count = numBones * 3;
	bufferCreateInformation.SimpleStorage.Format = BF_32X4F;

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	SPtr<GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(bufferCreateInformation);

	const u32 bufferSize = numBones * 3 * sizeof(Vector4);
	u8* dest = (u8*)B3DStackAllocate(bufferSize);

	// Initialize bone transforms to identity, so the object renders properly even if no animation is animating it
	for(u32 i = 0; i < numBones; i++)
	{
		memcpy(dest, &Matrix4::kIdentity, 12 * sizeof(float)); // Assuming row-major format
		dest += 12 * sizeof(float);
	}

	buffer->WriteData(0, bufferSize, dest, BWT_DISCARD);
	B3DStackFree(dest);

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
			mBoneMatrixBuffer = CreateBoneMatrixBuffer(numBones);

			if(mWriteVelocity)
				mBonePrevMatrixBuffer = CreateBoneMatrixBuffer(numBones);
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

		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		vertexBufferCreateInformation.Vertex.ElementSize = vertexSize;
		vertexBufferCreateInformation.Vertex.Count = numVertices;

		const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		SPtr<GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

		u32 totalSize = vertexSize * numVertices;
		u8* dest = (u8*)B3DStackAllocate(totalSize);
		memset(dest, 0, totalSize);

		vertexBuffer->WriteData(0, totalSize, dest, BWT_DISCARD);
		B3DStackFree(dest);

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

	const EvaluatedAnimationData::AnimationInfo* animInfo = nullptr;

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
		const u32 bufferSize = poseInfo.BoneCount * 3 * sizeof(Vector4);
		u8* dest = (u8*)B3DStackAllocate(bufferSize);
		for(u32 j = 0; j < poseInfo.BoneCount; j++)
		{
			const Matrix4& transform = animData.Transforms[poseInfo.BoneStartIndex + j];
			memcpy(dest, &transform, 12 * sizeof(float)); // Assuming row-major format

			dest += 12 * sizeof(float);
		}

		mBoneMatrixBuffer->WriteData(0, bufferSize, dest, BWT_DISCARD);
		B3DStackFree(dest);
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

void Renderable::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	RenderProxySyncPacket* const syncPacket = data.GetSyncPacket();
	if(syncPacket == nullptr)
		return;

	bool oldIsActive = mActive;

	syncPacket->ApplySyncData(this);

	mTfrmMatrix = mTransform.GetMatrix();
	mTfrmMatrixNoScale = Matrix4::TRS(mTransform.GetPosition(), mTransform.GetRotation(), Vector3::kOne);

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();

	const u32 flags = syncPacket->Flags;
	const u32 updateEverythingFlag = (u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active | (u32)ActorDirtyFlag::Dependency;
	if((flags & updateEverythingFlag) != 0)
	{
		CreateAnimationBuffers();

		// Create special vertex declaration if using morph shapes
		if(mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			TInlineArray<VertexElement, 8> vertexElements = mMesh->GetVertexDescription()->GetElements();
			vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION, 1, 1));
			vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_NORMAL, 1, 1));

			mMorphVertexDescription = B3DMakeShared<VertexDescription>(vertexElements);
		}
		else
			mMorphVertexDescription = nullptr;

		if(oldIsActive != mActive)
		{
			if(mActive)
				rendererScene->RegisterRenderable(this);
			else
				rendererScene->UnregisterRenderable(this);
		}
		else
		{
			rendererScene->UnregisterRenderable(this);
			rendererScene->RegisterRenderable(this);
		}
	}
	else if((flags & (u32)ActorDirtyFlag::Mobility) != 0)
	{
		rendererScene->UnregisterRenderable(this);
		rendererScene->RegisterRenderable(this);
	}
	else if((flags & (u32)ActorDirtyFlag::Transform) != 0)
	{
		if(mActive)
			rendererScene->UpdateRenderable(this);
	}
}
}}
