//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderable.h"

#include "BsCoreApplication.h"
#include "Private/RTTI/BsRenderableRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Mesh/BsMesh.h"
#include "Material/BsMaterial.h"
#include "Math/BsBounds.h"
#include "Renderer/BsRenderer.h"
#include "Animation/BsAnimation.h"
#include "Animation/BsMorphShapes.h"
#include "Animation/BsAnimationManager.h"
#include "Scene/BsSceneManager.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuDevice.h"

using namespace bs;

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
bool IsMeshValid(const SPtr<ct::Mesh>& mesh)
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

	u32 subMeshCount = 0;
	if(IsMeshValid(mesh))
		subMeshCount = (u32)mesh->GetProperties().SubMeshes.size();

	mMaterials.resize(subMeshCount);

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
		B3D_LOG(Warning, Renderer, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
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
		Bounds bounds = mesh->GetProperties().Bounds;
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
	handlerPtr->SetShared(handlerPtr);

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

namespace bs
{
	template <typename T, typename A = StdFrameAlloc<T>>
	using SyncFrameVector = std::vector<T, A>;

	template<bool Core, class FieldTypeA, class FieldTypeB>
	void CoreSyncField(FieldTypeA&& a, FieldTypeB&& b, std::enable_if_t<!Core>* = 0)
	{
		// TODO - Also add CoreSyncVector that is enabled for all std::vector types, and returns a SyncFrameVector

		a = std::move(GetCoreObject(RemoveHandle(b)));
	}

	template<bool Core, class FieldTypeA, class FieldTypeB>
	void CoreSyncField(FieldTypeA&& a, FieldTypeB&& b, std::enable_if_t<Core>* = 0)
	{
		b = std::move(a);
	}

	template<bool Core, class FieldTypeA, class FieldTypeB>
	void CoreSyncField(SyncFrameVector<FieldTypeA>& a, Vector<FieldTypeB>& b, std::enable_if_t<!Core>* = 0)
	{
		// TODO - Also add CoreSyncVector that is enabled for all std::vector types, and returns a SyncFrameVector

		a.resize(b.size());
		for(size_t index = 0; index < b.size(); ++index)
			a[index] = std::move(GetCoreObject(RemoveHandle(b[index])));
	}

	template<bool Core, class FieldTypeA, class FieldTypeB>
	void CoreSyncField(SyncFrameVector<FieldTypeA>& a, Vector<FieldTypeB>& b, std::enable_if_t<Core>* = 0)
	{
		b.resize(a.size());
		for(size_t index = 0; index < a.size(); ++index)
			a[index] = std::move(b[index]);
	}

	template <class T>
	struct CoreSyncType
	{
		typedef T Type;
		
	};

	template <class T>
	struct CoreSyncType<Vector<T>>
	{
		typedef SyncFrameVector<T> Type;
	};

	#define B3D_SYNC_BLOCK_BEGIN(ClassType, Name)                                                                                                               \
	struct Name \
	{                                                                                                                                             \
		typedef ClassType Type; \
		void Sync() \
		{                                                                                                                                         \
			SyncEntries();                                                                                                                        \
		}                                                                                                                                         \
                                                                                                                                                  \
		struct META_FirstEntry                                                                                                                    \
		{};                                                                                                                                       \
		void META_SyncPrevEntry(META_FirstEntry id)                                   \
		{}                                                                                                                                        \
                                                                                                                                                  \
		typedef META_FirstEntry

	#define B3D_SYNC_BLOCK_ENTRY(EntryName)                                                    \
	META_Entry_##EntryName;                                                                                            \
                                                                                                                   \
	struct META_NextEntry_##EntryName                                                                                  \
	{};                                                                                                            \
                                                                                                                   \
	void META_SyncPrevEntry(META_NextEntry_##EntryName id) \
	{                                                                                                              \
		META_SyncPrevEntry(META_Entry_##EntryName());                                                 \
	}                                                                                                              \
                                                                                                                   \
public:                                                                                                            \
	typename CoreSyncType<decltype(Type::EntryName)>::Type EntryName;                                                                     \
                                                                                                                   \
private:                                                                                                           \
	typedef META_NextEntry_##EntryName

#define B3D_SYNC_BLOCK_END                                                     \
	META_LastEntry;                                                             \
                                                                                \
	void SyncEntries()                                                          \
	{                                                                           \
		META_SyncPrevEntry(META_LastEntry()); \
	}                                                                           \
                                                                                \
	}                                                                           \
	;

	B3D_SYNC_BLOCK_BEGIN(Renderable, RenderableSyncData)
		B3D_SYNC_BLOCK_ENTRY(mLayer)
	B3D_SYNC_BLOCK_END

	struct Renderable::FullSyncData
	{
		FullSyncData(const Renderable& source, FrameAlloc* allocator)
			: Layer(source.mLayer)
			, OverrideBounds(source.mOverrideBounds)
			, UseOverrideBounds(source.mUseOverrideBounds)
			, WriteVelocity(source.mWriteVelocity)
			, AnimationType(source.mAnimType)
			, AnimationId(source.mAnimation != nullptr ? source.mAnimation->GetIdInternal() : (u64)-1)
			, CullDistanceFactor(source.mCullDistanceFactor)
			, Mesh(B3DGetCoreObject(source.mMesh))
			, Materials(allocator)
		{
			Materials.resize(source.mMaterials.size());
			for(size_t materialIndex = 0; materialIndex < source.mMaterials.size(); ++materialIndex)
				Materials[materialIndex] = B3DGetCoreObject(source.mMaterials[materialIndex]);
		}

		void Apply(ct::Renderable& destination)
		{
			destination.mLayer = Layer;
			destination.mOverrideBounds = OverrideBounds;
			destination.mUseOverrideBounds = UseOverrideBounds;
			destination.mWriteVelocity = WriteVelocity;
			destination.mAnimType = AnimationType;
			destination.mAnimationId = AnimationId;
			destination.mCullDistanceFactor = CullDistanceFactor;
			destination.mMesh = std::move(Mesh);

			destination.mMaterials.resize(Materials.size());
			for(size_t materialIndex = 0; materialIndex < Materials.size(); ++materialIndex)
				destination.mMaterials[materialIndex] = std::move(Materials[materialIndex]);
		}

		template<bool Core>
		void Sync(CoreVariantType<Renderable, Core>& object)
		{
			CoreSyncField<Core>(Layer, object.mLayer);
			CoreSyncField<Core>(OverrideBounds, object.mOverrideBounds);
			CoreSyncField<Core>(Mesh, object.mMesh);
			CoreSyncField<Core>(Materials, object.mMaterials);


			// SyncLast()
			// SyncPrevious_1
			// SyncPrevious_2 ... etc, similar to B3D_PARAMS
		}

		u64 Layer;
		AABox OverrideBounds;
		bool UseOverrideBounds;
		bool WriteVelocity;
		RenderableAnimType AnimationType;
		u64 AnimationId;
		float CullDistanceFactor;
		SPtr<ct::Mesh> Mesh;
		SyncFrameVector<SPtr<ct::Material>> Materials;
	};
}

CoreSyncData Renderable::SyncToCore(FrameAlloc* allocator)
{
	const u32 dirtyFlags = GetCoreDirtyFlags();
	u32 size = B3DRTTISize(dirtyFlags).Bytes;
	SceneActor::RttiEnumFields(RttiB3DCoreSyncSize(size), (ActorDirtyFlags)dirtyFlags);

	size = Math::DivideAndRoundUp(size, 4u) * 4u;
	u8* data = allocator->Alloc(size);
	Bitstream stream(data, size);

	B3DRTTIWrite(dirtyFlags, stream);
	SceneActor::RttiEnumFields(RttiCoreSyncWriter(stream), (ActorDirtyFlags)dirtyFlags);

	if(dirtyFlags != (u32)ActorDirtyFlag::Transform)
	{
		FullSyncData* output = allocator->Construct<FullSyncData>(*this, allocator);
		output->Sync<false>(*this);

		FullSyncData* dataR = (FullSyncData*)(data + size);
		int a = 5;
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
	handlerPtr->SetShared(handlerPtr);

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
		const u32 bufferSize = poseInfo.NumBones * 3 * sizeof(Vector4);
		u8* dest = (u8*)B3DStackAllocate(bufferSize);
		for(u32 j = 0; j < poseInfo.NumBones; j++)
		{
			const Matrix4& transform = animData.Transforms[poseInfo.StartIdx + j];
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
		bs::Renderable::FullSyncData* fullSyncData = (bs::Renderable::FullSyncData*)(data.GetBuffer() + data.GetBufferSize());
		fullSyncData->Apply(*this);

		fullSyncData->~FullSyncData(); // TODO - Destruct this properly through FrameAllocator it was constructed with
	}

	u32 updateEverythingFlag = (u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active | (u32)ActorDirtyFlag::Dependency;

	if((dirtyFlags & updateEverythingFlag) != 0)
	{
		CreateAnimationBuffers();

		// Create special vertex declaration if using morph shapes
		if(mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			SmallVector<VertexElement, 8> vertexElements = mMesh->GetVertexDescription()->GetElements();
			vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION, 1, 1));
			vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_NORMAL, 1, 1));

			mMorphVertexDescription = B3DMakeShared<VertexDescription>(vertexElements);
		}
		else
			mMorphVertexDescription = nullptr;

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
