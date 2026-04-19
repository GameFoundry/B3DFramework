//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DRenderable.h"
#include "Renderer/B3DRendererObjectStorage.h"
#include "B3DApplication.h"
#include "Animation/B3DMorphShapes.h"
#include "Scene/B3DSceneObject.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DMaterial.h"
#include "Components/B3DAnimation.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "ECS/B3DRegistry.h"
#include "Math/B3DBounds.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneObjectFragments.h"
#include "Scene/B3DGameObjectCollection.h"
#include "RTTI/B3DRenderableRTTI.h"

using namespace b3d;

ecs::Renderable& ecs::CreateRenderable(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene, const Transform& transform)
{
	ecs::RenderableECSUtility::CreateFragmentsIfMissing(registry, entity);
	registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(transform));
	rendererScene->AllocateRenderableId(registry, entity);
	ecs::RenderableECSUtility::MarkDirty(registry, entity);

	return registry.GetComponents<ecs::Renderable>(entity);
}

void ecs::DestroyRenderable(ecs::Registry& registry, ecs::Entity entity)
{
	ecs::RenderableECSUtility::RemoveFragments(registry, entity);
}

ecs::Renderable& Renderable::GetFragment()
{
	return GetECSRegistry()->GetComponents<ecs::Renderable>(GetECSEntity());
}

const ecs::Renderable& Renderable::GetFragment() const
{
	return GetECSRegistry()->GetComponents<ecs::Renderable>(GetECSEntity());
}

void Renderable::SetMesh(const HMesh& mesh)
{
	ecs::Renderable& fragment = GetFragment();
	fragment.Mesh = mesh;

	u32 subMeshCount = 0;
	if(IsValid(mesh))
		subMeshCount = (u32)mesh->GetProperties().SubMeshes.size();

	fragment.Materials.resize(subMeshCount);

	DoOnMeshChanged();

	MarkCoreObjectDependenciesDirty();
	MarkReferencedResourcesDirty();
	MarkRenderProxyDataDirty();
}

void Renderable::SetMaterial(u32 index, const HMaterial& material)
{
	ecs::Renderable& fragment = GetFragment();
	if(index >= (u32)fragment.Materials.size())
		return;

	fragment.Materials[index] = material;

	MarkCoreObjectDependenciesDirty();
	MarkReferencedResourcesDirty();
	MarkRenderProxyDataDirty();
}

void Renderable::SetMaterials(const Vector<HMaterial>& materials)
{
	ecs::Renderable& fragment = GetFragment();
	const u32 materialCount = (u32)fragment.Materials.size();
	const u32 materialCountToAssign = std::min(materialCount, (u32)materials.size());

	for(u32 materialIndex = 0; materialIndex < materialCountToAssign; ++materialIndex)
		fragment.Materials[materialIndex] = materials[materialIndex];

	for(u32 materialIndex = materialCountToAssign; materialIndex < materialCount; ++materialIndex)
		fragment.Materials[materialIndex] = nullptr;

	MarkCoreObjectDependenciesDirty();
	MarkReferencedResourcesDirty();
	MarkRenderProxyDataDirty();
}

void Renderable::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		B3D_LOG(Warning, LogRenderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	GetFragment().Layer = layer;
	MarkRenderProxyDataDirty();
}

void Renderable::SetOverrideBounds(const AABox& bounds)
{
	ecs::Renderable& fragment = GetFragment();
	fragment.OverrideBounds = bounds;

	if(fragment.UseOverrideBounds)
		MarkRenderProxyDataDirty();
}

void Renderable::SetUseOverrideBounds(bool enable)
{
	ecs::Renderable& fragment = GetFragment();
	if(fragment.UseOverrideBounds == enable)
		return;

	fragment.UseOverrideBounds = enable;
	MarkRenderProxyDataDirty();
}

void Renderable::SetWriteVelocity(bool enable)
{
	ecs::Renderable& fragment = GetFragment();
	if(fragment.WriteVelocity == enable)
		return;

	fragment.WriteVelocity = enable;
	MarkRenderProxyDataDirty();
}

void Renderable::SetCullDistanceFactor(float factor)
{
	GetFragment().CullDistanceFactor = factor;
	MarkRenderProxyDataDirty();
}

void Renderable::MarkCoreObjectDependenciesDirty()
{
	CoreObject::MarkDependenciesDirty(); // TODO - Rename the base class method
}

void Renderable::MarkReferencedResourcesDirty()
{
	IResourceListener::MarkListenerResourcesDirty(); // TODO - Rename the base class method
}

Renderable::Renderable(const HSceneObject& parent)
	: Component(parent)
{
	SetName("Renderable");
	SetFlag(ComponentFlag::AlwaysRun, true);
	mNotifyFlags = (TransformChangedFlags)(TCF_Parent | TCF_Transform);
}

Renderable::Renderable()
	: Renderable(nullptr)
{ }

void Renderable::Initialize()
{
	SetShared(B3DStaticGameObjectCast<Renderable>(mThisHandle).GetShared());
	const bool created = ecs::RenderableECSUtility::CreateFragmentsIfMissing(*GetECSRegistry(), GetECSEntity());

	// Ensure at least 1 material slot for newly created fragments
	if(created)
		GetFragment().Materials.resize(1);

	// Initialize after adding everything to the registry, as initialize might require the ECS fragments
	Component::Initialize();
	CoreObject::Initialize();
}

void Renderable::OnCreated()
{
	// If any resources were deserialized before initialization, make sure the listener is notified
	MarkReferencedResourcesDirty();
}

void Renderable::OnBeginPlay()
{
	mAnimation = SO()->GetComponent<Animation>();
	if(mAnimation != nullptr)
	{
		RegisterAnimation(mAnimation);
		mAnimation->RegisterRenderable(B3DStaticGameObjectCast<Renderable>(mThisHandle));
	}
}

void Renderable::OnEnabled()
{
	const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::RenderableECSUtility::RegisterWithRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void Renderable::OnDisabled()
{
	const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::RenderableECSUtility::UnregisterFromRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void Renderable::OnDestroyed()
{
	if(mAnimation != nullptr)
		mAnimation->UnregisterRenderable();

	ecs::RenderableECSUtility::RemoveFragments(*GetECSRegistry(), GetECSEntity());
	CoreObject::Destroy();
}

void Renderable::OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity)
{
	ecs::RenderableECSUtility::ChangeScene(oldScene, oldEntity, *SceneObject()->GetScene(), GetECSEntity(), GetEnabled());
}

void Renderable::OnTransformChanged(TransformChangedFlags flags)
{
	const ecs::Renderable& fragment = GetFragment();

	// If skinned animation, don't include own transform since that will be handled by root bone animation
	bool ignoreOwnTransform;
	if(fragment.AnimType == RenderableAnimType::Skinned || fragment.AnimType == RenderableAnimType::SkinnedMorph)
		ignoreOwnTransform = mAnimation.IsValid() ? mAnimation->GetAnimatesRoot() : false;
	else
		ignoreOwnTransform = false;

	if(ignoreOwnTransform)
		SceneObject()->SetLocalTransform(Transform::kIdentity);

	MarkRenderProxyDataDirty(ComponentDirtyFlag::Transform);
}

Bounds Renderable::GetBounds() const
{
	const Transform& transform = SceneObject()->GetTransform();
	const ecs::Renderable& fragment = GetFragment();

	if(fragment.UseOverrideBounds)
	{
		Bounds bounds(fragment.OverrideBounds);
		bounds.TransformAffine(transform);
		return bounds;
	}

	HMesh mesh = GetMesh();
	if(!mesh.IsLoaded())
		return Bounds(transform.GetPosition(), Vector3::kZero, 0.0f);
	else
	{
		Bounds bounds = mesh->GetProperties().Bounds;
		bounds.TransformAffine(transform);
		return bounds;
	}
}

bool Renderable::CalculateBounds(Bounds& bounds)
{
	bounds = GetBounds();

	return true;
}

void Renderable::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	const ecs::Renderable& fragment = GetFragment();

	if(fragment.Mesh.IsLoaded())
		dependencies.push_back(fragment.Mesh.Get());

	for(const auto& material : fragment.Materials)
	{
		if(material.IsLoaded())
			dependencies.push_back(material.Get());
	}
}

void Renderable::OnDependencyDirty(CoreObject* dependency, u32 dirtyFlags)
{
	const ecs::Renderable& fragment = GetFragment();

	if(fragment.Mesh.IsLoaded(false) && fragment.Mesh.Get() == dependency)
	{
		GetECSRegistry()->AddTag<ecs::RenderableDirty>(GetECSEntity());
		return;
	}

	if(((u32)MaterialDirtyFlags::Shader & dirtyFlags) != 0)
		GetECSRegistry()->AddTag<ecs::RenderableDirty>(GetECSEntity());
}

void Renderable::GetListenerResources(Vector<HResource>& resources)
{
	const ecs::Renderable& fragment = GetFragment();

	if(fragment.Mesh != nullptr)
		resources.push_back(fragment.Mesh);

	for(const auto& material : fragment.Materials)
	{
		if(material != nullptr)
			resources.push_back(material);
	}
}

void Renderable::NotifyResourceLoaded(const HResource& resource)
{
	if(resource == GetFragment().Mesh)
		DoOnMeshChanged();

	MarkCoreObjectDependenciesDirty();
	MarkRenderProxyDataDirty();
}

void Renderable::NotifyResourceChanged(const HResource& resource)
{
	if(resource == GetFragment().Mesh)
		DoOnMeshChanged();

	MarkCoreObjectDependenciesDirty();
	MarkRenderProxyDataDirty();
}

void Renderable::RefreshAnimation()
{
	ecs::Renderable& fragment = GetFragment();

	if(!mAnimation.IsValid())
	{
		fragment.AnimType = RenderableAnimType::None;
		return;
	}

	if(fragment.Mesh.IsLoaded(false))
	{
		SPtr<Skeleton> skeleton = fragment.Mesh->GetSkeleton();
		SPtr<MorphShapes> morphShapes = fragment.Mesh->GetMorphShapes();

		if(skeleton != nullptr && morphShapes != nullptr)
			fragment.AnimType = RenderableAnimType::SkinnedMorph;
		else if(skeleton != nullptr)
			fragment.AnimType = RenderableAnimType::Skinned;
		else if(morphShapes != nullptr)
			fragment.AnimType = RenderableAnimType::Morph;
		else
			fragment.AnimType = RenderableAnimType::None;

		mAnimation->SetSkeleton(fragment.Mesh->GetSkeleton());
		mAnimation->SetMorphShapes(fragment.Mesh->GetMorphShapes());
	}
	else
	{
		fragment.AnimType = RenderableAnimType::None;

		mAnimation->SetSkeleton(nullptr);
		mAnimation->SetMorphShapes(nullptr);
	}
}

void Renderable::RegisterAnimation(const HAnimation& animation)
{
	mAnimation = animation;

	RefreshAnimation();
	GetFragment().AnimationId = animation->GetAnimationId();
	MarkRenderProxyDataDirty();
}

void Renderable::UnregisterAnimation()
{
	mAnimation = nullptr;

	RefreshAnimation();
	GetFragment().AnimationId = (u64)-1;
	MarkRenderProxyDataDirty();
}

void Renderable::DoOnMeshChanged()
{
	if(mAnimation != nullptr)
	{
		mAnimation->UpdateBounds(false);
		RefreshAnimation();
	}
}

void Renderable::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if(!SceneObject().IsValid())
		return;

	ecs::RenderableECSUtility::MarkDirty(*GetECSRegistry(), GetECSEntity(), flag);
}

RTTIType* ecs::Renderable::GetRttiStatic()
{
	return ecs::ECSRenderableRTTI::Instance();
}

RTTIType* ecs::Renderable::GetRtti() const
{
	return ecs::Renderable::GetRttiStatic();
}

RTTIType* Renderable::GetRttiStatic()
{
	return RenderableRTTI::Instance();
}

RTTIType* Renderable::GetRtti() const
{
	return Renderable::GetRttiStatic();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::Renderable, FullSyncPacket, TRenderableData<true>)
		B3D_SYNC_BLOCK_ENTRY(Layer)
		B3D_SYNC_BLOCK_ENTRY(OverrideBounds)
		B3D_SYNC_BLOCK_ENTRY(UseOverrideBounds)
		B3D_SYNC_BLOCK_ENTRY(WriteVelocity)
		B3D_SYNC_BLOCK_ENTRY(AnimType)
		B3D_SYNC_BLOCK_ENTRY(CullDistanceFactor)
		B3D_SYNC_BLOCK_ENTRY(Mesh)
		B3D_SYNC_BLOCK_ENTRY(Materials)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(u64, AnimationId)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::Renderable, TransformSyncPacket, TRenderableData<true>)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	struct RenderableFullUpdateChannel : TRendererObjectECSSyncChannel
	<
		RenderableFullUpdateChannel,
		ecs::Renderable::FullSyncPacket,
		ecs::RenderableDirty,
		ecs::Renderable, ecs::WorldTransform, ecs::RenderableId
	>
	{
		void Write(RenderableObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToCreate(&allocator);
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToDestroy(&allocator);

			WritePackets(storage, allocator, [&renderStatesToCreate, &renderStatesToDestroy, &storage](ecs::Renderable::FullSyncPacket& packet, PackedRendererId rendererId)
			{
				render::RenderableProxy& proxy = storage.GetRenderableProxy(rendererId);

				bool wasRegistered = proxy.mRendererId != kInvalidPackedRendererId;
				proxy.mRendererId = rendererId;

				packet.ApplySyncData(&proxy.mData);

				proxy.mAnimationId = packet.AnimationId;
				proxy.mTransform = packet.TransformData;

				proxy.mWorldTransformMatrix = proxy.mTransform.GetMatrix();
				proxy.mWorldTransformMatrixWithoutScale = Matrix4::TRS(proxy.mTransform.GetPosition(), proxy.mTransform.GetRotation(), Vector3::kOne);

				proxy.CreateAnimationBuffers();

				if(proxy.mData.AnimType == RenderableAnimType::Morph || proxy.mData.AnimType == RenderableAnimType::SkinnedMorph)
				{
					TInlineArray<VertexElement, 8> vertexElements = proxy.mData.Mesh->GetVertexDescription()->GetElements();
					vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION, 1, 1));
					vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_NORMAL, 1, 1));

					proxy.mMorphVertexDescription = B3DMakeShared<VertexDescription>(vertexElements);
				}
				else
					proxy.mMorphVertexDescription = nullptr;

				if(wasRegistered)
					renderStatesToDestroy.push_back(rendererId);

				renderStatesToCreate.push_back(rendererId);
			});

			if(!renderStatesToDestroy.empty())
				storage.DestroyRenderState(renderStatesToDestroy);

			if(!renderStatesToCreate.empty())
				storage.CreateRenderState(renderStatesToCreate);
		}

		void CreateAndPopulatePacket(ecs::Renderable& fragment, ecs::WorldTransform& transform, ecs::RenderableId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.AnimationId = fragment.AnimationId;
			packet.TransformData = transform;
		}
	};

	struct RenderableTransformUpdateChannel : TRendererObjectECSSyncChannel
	<
		RenderableTransformUpdateChannel,
		ecs::Renderable::TransformSyncPacket,
		ecs::RenderableTransformDirty,
		ecs::Renderable, ecs::WorldTransform, ecs::RenderableId
	>
	{
		void Write(RenderableObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToUpdate(&allocator);

			WritePackets(storage, allocator, [&renderStatesToUpdate, &storage](ecs::Renderable::TransformSyncPacket& packet, PackedRendererId rendererId)
			{
				render::RenderableProxy& proxy = storage.GetRenderableProxy(rendererId);

				proxy.mTransform = packet.TransformData;
				proxy.mWorldTransformMatrix = proxy.mTransform.GetMatrix();
				proxy.mWorldTransformMatrixWithoutScale = Matrix4::TRS(proxy.mTransform.GetPosition(), proxy.mTransform.GetRotation(), Vector3::kOne);

				renderStatesToUpdate.push_back(rendererId);
			});

			if(!renderStatesToUpdate.empty())
				storage.UpdateRenderState(renderStatesToUpdate);
		}

		void CreateAndPopulatePacket(ecs::Renderable& fragment, ecs::WorldTransform& transform, ecs::RenderableId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	using RenderableSyncBatch = TRendererObjectECSSyncBatch<RenderableFullUpdateChannel, RenderableTransformUpdateChannel>;
} // namespace b3d

void* RenderableObjectStorageBase::SyncRead(ecs::Registry& registry, FrameAllocator& allocator)
{
	return RenderableSyncBatch::Read(*this, registry, allocator);
}

void RenderableObjectStorageBase::SyncWrite(void* rawData, FrameAllocator& allocator)
{
	RenderableSyncBatch::Write(*this, rawData, allocator);
}

namespace b3d { namespace render
{
static SPtr<GpuBuffer> CreateBoneMatrixBuffer(u32 boneCount)
{
	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	bufferCreateInformation.SimpleStorage.Count = boneCount * 3;
	bufferCreateInformation.SimpleStorage.Format = BF_32X4F;

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	SPtr<GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(bufferCreateInformation);

	GpuBufferMappedScope mapping = buffer->Map(GpuMapOption::Write);
	u8* currentWriteLocation = (u8*)mapping.GetMappedMemory();

	// Initialize bone transforms to identity, so the object renders properly even if no animation is animating it
	for(u32 boneIndex = 0; boneIndex < boneCount; ++boneIndex)
	{
		memcpy(currentWriteLocation, &Matrix4::kIdentity, 12 * sizeof(float)); // Assuming row-major format
		currentWriteLocation += 12 * sizeof(float);
	}

	return buffer;
}

Bounds RenderableProxy::GetBounds() const
{
	if(mData.UseOverrideBounds)
	{
		Bounds bounds(mData.OverrideBounds);
		bounds.TransformAffine(mWorldTransformMatrix);

		return bounds;
	}

	SPtr<Mesh> mesh = GetMesh();

	if(mesh == nullptr)
		return Bounds(mTransform.GetPosition(), Vector3::kZero, 0.0f);
	else
	{
		Bounds bounds = mesh->GetProperties().Bounds;
		bounds.TransformAffine(mWorldTransformMatrix);

		return bounds;
	}
}

void RenderableProxy::CreateAnimationBuffers()
{
	if(mData.AnimType == RenderableAnimType::Skinned || mData.AnimType == RenderableAnimType::SkinnedMorph)
	{
		SPtr<Skeleton> skeleton = mData.Mesh->GetSkeleton();
		u32 boneCount = skeleton != nullptr ? skeleton->GetBoneCount() : 0;

		if(boneCount > 0)
		{
			mBoneMatrixBuffer = CreateBoneMatrixBuffer(boneCount);

			if(mData.WriteVelocity)
				mPreviousBoneMatrixBuffer = CreateBoneMatrixBuffer(boneCount);
			else
				mPreviousBoneMatrixBuffer = nullptr;
		}
		else
		{
			mBoneMatrixBuffer = nullptr;
			mPreviousBoneMatrixBuffer = nullptr;
		}
	}
	else
	{
		mBoneMatrixBuffer = nullptr;
		mPreviousBoneMatrixBuffer = nullptr;
	}

	if(mData.AnimType == RenderableAnimType::Morph || mData.AnimType == RenderableAnimType::SkinnedMorph)
	{
		// Note: Not handling velocity writing for morph animations

		SPtr<MorphShapes> morphShapes = mData.Mesh->GetMorphShapes();

		const u32 vertexSize = sizeof(Vector3) + sizeof(u32);
		const u32 vertexCount = morphShapes->GetVertexCount();

		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		vertexBufferCreateInformation.Vertex.ElementSize = vertexSize;
		vertexBufferCreateInformation.Vertex.Count = vertexCount;

		const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
		SPtr<GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

		u32 totalSize = vertexSize * vertexCount;

		GpuBufferMappedScope mapping = vertexBuffer->Map(GpuMapOption::Write);
		memset(mapping.GetMappedMemory(), 0, totalSize);

		mMorphShapeBuffer = vertexBuffer;
	}
	else
		mMorphShapeBuffer = nullptr;

	mMorphShapeVersion = 0;
}

void RenderableProxy::UpdateAnimationBuffers(const EvaluatedAnimationData& animData)
{
	if(mAnimationId == (u64)-1)
		return;

	const EvaluatedAnimationData::AnimationInfo* animInfo = nullptr;

	auto found = animData.Infos.find(mAnimationId);
	if(found != animData.Infos.end())
		animInfo = &found->second;

	if(animInfo == nullptr)
		return;

	if(mData.AnimType == RenderableAnimType::Skinned || mData.AnimType == RenderableAnimType::SkinnedMorph)
	{
		const EvaluatedAnimationData::PoseInfo& poseInfo = animInfo->PoseInfo;

		if(mData.WriteVelocity)
			std::swap(mBoneMatrixBuffer, mPreviousBoneMatrixBuffer);

		// Note: If multiple elements are using the same animation (not possible atm), this buffer should be shared by
		// all such elements
		const u32 bufferSize = poseInfo.BoneCount * 3 * sizeof(Vector4);
		u8* const temporaryBuffer = (u8*)B3DStackAllocate(bufferSize);
		u8* currentWriteLocation = temporaryBuffer;
		for(u32 boneIndex = 0; boneIndex < poseInfo.BoneCount; ++boneIndex)
		{
			const Matrix4& transform = animData.Transforms[poseInfo.BoneStartIndex + boneIndex];
			memcpy(currentWriteLocation, &transform, 12 * sizeof(float)); // Assuming row-major format

			currentWriteLocation += 12 * sizeof(float);
		}

		GpuBufferUtility::Write(mBoneMatrixBuffer, 0, bufferSize, temporaryBuffer, GpuBufferWriteFlag::Discard);
		B3DStackFree(temporaryBuffer);
	}

	if(mData.AnimType == RenderableAnimType::Morph || mData.AnimType == RenderableAnimType::SkinnedMorph)
	{
		if(mMorphShapeVersion != animInfo->MorphShapeInfo.Version)
		{
			SPtr<MeshData> meshData = animInfo->MorphShapeInfo.MeshData;

			u32 bufferSize = meshData->GetSize();
			u8* data = meshData->GetData();

			GpuBufferUtility::Write(mMorphShapeBuffer, 0, bufferSize, data, GpuBufferWriteFlag::Discard);
			mMorphShapeVersion = animInfo->MorphShapeInfo.Version;
		}
	}
}
}}

