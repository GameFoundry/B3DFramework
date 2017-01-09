//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderable.h"
#include "BsRenderableRTTI.h"
#include "BsSceneObject.h"
#include "BsBuiltinResources.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsRenderQueue.h"
#include "BsBounds.h"
#include "BsRenderer.h"
#include "BsAnimation.h"
#include "BsFrameAlloc.h"
#include "BsMorphShapes.h"
#include "BsGpuBuffer.h"
#include "BsAnimationManager.h"

namespace bs
{
	template<class T>
	bool isMeshValid(const T& mesh) { return false; }

	template<>
	bool isMeshValid(const HMesh& mesh) { return mesh.isLoaded(); }

	template<>
	bool isMeshValid(const SPtr<ct::MeshCore>& mesh) { return mesh != nullptr; }

	template<bool Core>
	TRenderable<Core>::TRenderable()
		: mLayer(1), mUseOverrideBounds(false), mPosition(BsZero), mTransform(BsIdentity), mTransformNoScale(BsIdentity)
		, mIsActive(true), mAnimType(RenderableAnimType::None)
	{
		mMaterials.resize(1);
	}

	template<bool Core>
	TRenderable<Core>::~TRenderable()
	{

	}

	template<bool Core>
	void TRenderable<Core>::setMesh(const MeshType& mesh)
	{
		mMesh = mesh;

		int numSubMeshes = 0;
		if (isMeshValid(mesh))
			numSubMeshes = mesh->getProperties().getNumSubMeshes();

		mMaterials.resize(numSubMeshes);

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

	template<bool Core>
	void TRenderable<Core>::setLayer(UINT64 layer)
	{
		bool isPow2 = layer && !((layer - 1) & layer);

		if (!isPow2)
		{
			LOGWRN("Invalid layer provided. Only one layer bit may be set. Ignoring.");
			return;
		}

		mLayer = layer;
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setTransform(const Matrix4& transform, const Matrix4& transformNoScale)
	{
		mTransform = transform;
		mTransformNoScale = transformNoScale;
		mPosition = mTransform.getTranslation();

		_markCoreDirty(RenderableDirtyFlag::Transform);
	}

	template<bool Core>
	void TRenderable<Core>::setIsActive(bool active)
	{
		mIsActive = active;
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

	template class TRenderable < false >;
	template class TRenderable < true >;

	Renderable::Renderable()
		:mLastUpdateHash(0)
	{
		
	}

	void Renderable::setAnimation(const SPtr<Animation>& animation)
	{
		mAnimation = animation;
		refreshAnimation();

		_markCoreDirty();
	}

	Bounds Renderable::getBounds() const
	{
		if(mUseOverrideBounds)
		{
			Sphere sphere(mOverrideBounds.getCenter(), mOverrideBounds.getRadius());

			Bounds bounds(mOverrideBounds, sphere);
			bounds.transformAffine(mTransform);

			return bounds;
		}

		HMesh mesh = getMesh();

		if (!mesh.isLoaded())
		{
			AABox box(mPosition, mPosition);
			Sphere sphere(mPosition, 0.0f);

			return Bounds(box, sphere);
		}
		else
		{
			Bounds bounds = mesh->getProperties().getBounds();
			bounds.transformAffine(mTransform);

			return bounds;
		}
	}

	SPtr<ct::RenderableCore> Renderable::getCore() const
	{
		return std::static_pointer_cast<ct::RenderableCore>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Renderable::createCore() const
	{
		ct::RenderableCore* handler = new (bs_alloc<ct::RenderableCore>()) ct::RenderableCore();
		SPtr<ct::RenderableCore> handlerPtr = bs_shared_ptr<ct::RenderableCore>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	void Renderable::onMeshChanged()
	{
		refreshAnimation();
	}

	void Renderable::refreshAnimation()
	{
		if (mAnimation == nullptr)
		{
			mAnimType = RenderableAnimType::None;
			return;
		}

		if (mMesh.isLoaded(false))
		{
			SPtr<Skeleton> skeleton = mMesh->getSkeleton();
			SPtr<MorphShapes> morphShapes = mMesh->getMorphShapes();

			if (skeleton != nullptr && morphShapes != nullptr)
				mAnimType = RenderableAnimType::SkinnedMorph;
			else if (skeleton != nullptr)
				mAnimType = RenderableAnimType::Skinned;
			else if (morphShapes != nullptr)
				mAnimType = RenderableAnimType::Morph;
			else
				mAnimType = RenderableAnimType::None;

			mAnimation->setSkeleton(mMesh->getSkeleton());
			mAnimation->setMorphShapes(mMesh->getMorphShapes());
		}
		else
		{
			mAnimType = RenderableAnimType::None;

			mAnimation->setSkeleton(nullptr);
			mAnimation->setMorphShapes(nullptr);
		}
	}

	void Renderable::_updateTransform(const HSceneObject& so, bool force)
	{
		UINT32 curHash = so->getTransformHash();
		if (curHash != _getLastModifiedHash() || force)
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
				HSceneObject parentSO = so->getParent();
				if (parentSO != nullptr)
				{
					Matrix4 transformNoScale = Matrix4::TRS(parentSO->getWorldPosition(), parentSO->getWorldRotation(),
						Vector3::ONE);
					setTransform(parentSO->getWorldTfrm(), transformNoScale);
				}
				else
					setTransform(Matrix4::IDENTITY, Matrix4::IDENTITY);
			}
			else
			{
				Matrix4 transformNoScale = Matrix4::TRS(so->getWorldPosition(), so->getWorldRotation(), Vector3::ONE);
				setTransform(so->getWorldTfrm(), transformNoScale);
			}

			_setLastModifiedHash(curHash);
		}
	}

	void Renderable::_markCoreDirty(RenderableDirtyFlag flag)
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

	CoreSyncData Renderable::syncToCore(FrameAlloc* allocator)
	{
		UINT32 numMaterials = (UINT32)mMaterials.size();

		UINT64 animationId;
		if (mAnimation != nullptr)
			animationId = mAnimation->_getId();
		else
			animationId = (UINT64)-1;

		UINT32 size = rttiGetElemSize(mLayer) + 
			rttiGetElemSize(mOverrideBounds) + 
			rttiGetElemSize(mUseOverrideBounds) +
			rttiGetElemSize(numMaterials) + 
			rttiGetElemSize(mTransform) +
			rttiGetElemSize(mTransformNoScale) +
			rttiGetElemSize(mPosition) +
			rttiGetElemSize(mIsActive) +
			rttiGetElemSize(animationId) +
			rttiGetElemSize(mAnimType) + 
			rttiGetElemSize(getCoreDirtyFlags()) +
			sizeof(SPtr<ct::MeshCore>) +
			numMaterials * sizeof(SPtr<ct::MaterialCore>);

		UINT8* data = allocator->alloc(size);
		char* dataPtr = (char*)data;
		dataPtr = rttiWriteElem(mLayer, dataPtr);
		dataPtr = rttiWriteElem(mOverrideBounds, dataPtr);
		dataPtr = rttiWriteElem(mUseOverrideBounds, dataPtr);
		dataPtr = rttiWriteElem(numMaterials, dataPtr);
		dataPtr = rttiWriteElem(mTransform, dataPtr);
		dataPtr = rttiWriteElem(mTransformNoScale, dataPtr);
		dataPtr = rttiWriteElem(mPosition, dataPtr);
		dataPtr = rttiWriteElem(mIsActive, dataPtr);
		dataPtr = rttiWriteElem(animationId, dataPtr);
		dataPtr = rttiWriteElem(mAnimType, dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);

		SPtr<ct::MeshCore>* mesh = new (dataPtr) SPtr<ct::MeshCore>();
		if (mMesh.isLoaded())
			*mesh = mMesh->getCore();

		dataPtr += sizeof(SPtr<ct::MeshCore>);

		for (UINT32 i = 0; i < numMaterials; i++)
		{
			SPtr<ct::MaterialCore>* material = new (dataPtr)SPtr<ct::MaterialCore>();
			if (mMaterials[i].isLoaded())
				*material = mMaterials[i]->getCore();

			dataPtr += sizeof(SPtr<ct::MaterialCore>);
		}

		return CoreSyncData(data, size);
	}

	void Renderable::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mMesh.isLoaded())
			dependencies.push_back(mMesh.get());

		for (auto& material : mMaterials)
		{
			if (material.isLoaded())
				dependencies.push_back(material.get());
		}
	}

	void Renderable::getListenerResources(Vector<HResource>& resources)
	{
		if (mMesh != nullptr)
			resources.push_back(mMesh);

		for (auto& material : mMaterials)
		{
			if (material != nullptr)
				resources.push_back(material);
		}
	}

	void Renderable::notifyResourceLoaded(const HResource& resource)
	{
		if (resource == mMesh)
			onMeshChanged();

		markDependenciesDirty();
		markCoreDirty();
	}

	void Renderable::notifyResourceChanged(const HResource& resource)
	{
		if(resource == mMesh)
			onMeshChanged();

		markDependenciesDirty();
		markCoreDirty();
	}

	SPtr<Renderable> Renderable::create()
	{
		SPtr<Renderable> handlerPtr = createEmpty();
		handlerPtr->initialize();

		return handlerPtr;
	}

	SPtr<Renderable> Renderable::createEmpty()
	{
		Renderable* handler = new (bs_alloc<Renderable>()) Renderable();
		SPtr<Renderable> handlerPtr = bs_core_ptr<Renderable>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	RTTITypeBase* Renderable::getRTTIStatic()
	{
		return RenderableRTTI::instance();
	}

	RTTITypeBase* Renderable::getRTTI() const
	{
		return Renderable::getRTTIStatic();
	}

	namespace ct
	{
	RenderableCore::RenderableCore() 
		:mRendererId(0), mAnimationId((UINT64)-1), mMorphShapeVersion(0)
	{
	}

	RenderableCore::~RenderableCore()
	{
		if (mIsActive)
			gRenderer()->notifyRenderableRemoved(this);
	}

	void RenderableCore::initialize()
	{
		gRenderer()->notifyRenderableAdded(this);

		CoreObject::initialize();
	}

	Bounds RenderableCore::getBounds() const
	{
		if (mUseOverrideBounds)
		{
			Sphere sphere(mOverrideBounds.getCenter(), mOverrideBounds.getRadius());

			Bounds bounds(mOverrideBounds, sphere);
			bounds.transformAffine(mTransform);

			return bounds;
		}

		SPtr<MeshCore> mesh = getMesh();

		if (mesh == nullptr)
		{
			AABox box(mPosition, mPosition);
			Sphere sphere(mPosition, 0.0f);

			return Bounds(box, sphere);
		}
		else
		{
			Bounds bounds = mesh->getProperties().getBounds();
			bounds.transformAffine(mTransform);

			return bounds;
		}
	}

	void RenderableCore::createAnimationBuffers()
	{
		if (mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			SPtr<Skeleton> skeleton = mMesh->getSkeleton();
			UINT32 numBones = skeleton != nullptr ? skeleton->getNumBones() : 0;

			if (numBones > 0)
			{
				GPU_BUFFER_DESC desc;
				desc.elementCount = numBones * 3;
				desc.elementSize = 0;
				desc.type = GBT_STANDARD;
				desc.format = BF_32X4F;
				desc.usage = GBU_DYNAMIC;

				SPtr<GpuBufferCore> buffer = GpuBufferCore::create(desc);
				UINT8* dest = (UINT8*)buffer->lock(0, numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);

				// Initialize bone transforms to identity, so the object renders properly even if no animation is animating it
				for (UINT32 i = 0; i < numBones; i++)
				{
					memcpy(dest, &Matrix4::IDENTITY, 12 * sizeof(float)); // Assuming row-major format

					dest += 12 * sizeof(float);
				}

				buffer->unlock();

				mBoneMatrixBuffer = buffer;
			}
			else
				mBoneMatrixBuffer = nullptr;
		}
		else
			mBoneMatrixBuffer = nullptr;

		if (mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			SPtr<MorphShapes> morphShapes = mMesh->getMorphShapes();

			UINT32 vertexSize = sizeof(Vector3) + sizeof(UINT32);
			UINT32 numVertices = morphShapes->getNumVertices();

			VERTEX_BUFFER_DESC desc;
			desc.vertexSize = vertexSize;
			desc.numVerts = numVertices;
			desc.usage = GBU_DYNAMIC;

			SPtr<VertexBufferCore> vertexBuffer = VertexBufferCore::create(desc);

			UINT32 totalSize = vertexSize * numVertices;
			UINT8* dest = (UINT8*)vertexBuffer->lock(0, totalSize, GBL_WRITE_ONLY_DISCARD);
			memset(dest, 0, totalSize);
			vertexBuffer->unlock();

			mMorphShapeBuffer = vertexBuffer;
		}
		else
			mMorphShapeBuffer = nullptr;

		mMorphShapeVersion = 0;
	}

	void RenderableCore::updateAnimationBuffers(const RendererAnimationData& animData)
	{
		if (mAnimationId == (UINT64)-1)
			return;

		const RendererAnimationData::AnimInfo* animInfo = nullptr;

		auto iterFind = animData.infos.find(mAnimationId);
		if (iterFind != animData.infos.end())
			animInfo = &iterFind->second;

		if (animInfo == nullptr)
			return;

		if (mAnimType == RenderableAnimType::Skinned || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			const RendererAnimationData::PoseInfo& poseInfo = animInfo->poseInfo;

			// Note: If multiple elements are using the same animation (not possible atm), this buffer should be shared by
			// all such elements
			UINT8* dest = (UINT8*)mBoneMatrixBuffer->lock(0, poseInfo.numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);
			for (UINT32 j = 0; j < poseInfo.numBones; j++)
			{
				const Matrix4& transform = animData.transforms[poseInfo.startIdx + j];
				memcpy(dest, &transform, 12 * sizeof(float)); // Assuming row-major format

				dest += 12 * sizeof(float);
			}

			mBoneMatrixBuffer->unlock();
		}

		if (mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
		{
			if (mMorphShapeVersion < animInfo->morphShapeInfo.version)
			{
				SPtr<MeshData> meshData = animInfo->morphShapeInfo.meshData;

				UINT32 bufferSize = meshData->getSize();
				UINT8* data = meshData->getData();

				mMorphShapeBuffer->writeData(0, bufferSize, data, BWT_DISCARD);
				mMorphShapeVersion = animInfo->morphShapeInfo.version;
			}
		}
	}

	void RenderableCore::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		mMaterials.clear();

		UINT32 numMaterials = 0;
		UINT32 dirtyFlags = 0;
		bool oldIsActive = mIsActive;

		dataPtr = rttiReadElem(mLayer, dataPtr);
		dataPtr = rttiReadElem(mOverrideBounds, dataPtr);
		dataPtr = rttiReadElem(mUseOverrideBounds, dataPtr);
		dataPtr = rttiReadElem(numMaterials, dataPtr);
		dataPtr = rttiReadElem(mTransform, dataPtr);
		dataPtr = rttiReadElem(mTransformNoScale, dataPtr);
		dataPtr = rttiReadElem(mPosition, dataPtr);
		dataPtr = rttiReadElem(mIsActive, dataPtr);
		dataPtr = rttiReadElem(mAnimationId, dataPtr);
		dataPtr = rttiReadElem(mAnimType, dataPtr);
		dataPtr = rttiReadElem(dirtyFlags, dataPtr);

		SPtr<MeshCore>* mesh = (SPtr<MeshCore>*)dataPtr;
		mMesh = *mesh;
		mesh->~SPtr<MeshCore>();
		dataPtr += sizeof(SPtr<MeshCore>);

		for (UINT32 i = 0; i < numMaterials; i++)
		{
			SPtr<MaterialCore>* material = (SPtr<MaterialCore>*)dataPtr;
			mMaterials.push_back(*material);
			material->~SPtr<MaterialCore>();
			dataPtr += sizeof(SPtr<MaterialCore>);
		}

		if (dirtyFlags == (UINT32)RenderableDirtyFlag::Transform)
		{
			if (mIsActive)
				gRenderer()->notifyRenderableUpdated(this);
		}
		else
		{
			createAnimationBuffers();

			// Create special vertex declaration if using morph shapes
			if (mAnimType == RenderableAnimType::Morph || mAnimType == RenderableAnimType::SkinnedMorph)
			{
				SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
				*vertexDesc = * mMesh->getVertexDesc();

				vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION, 1, 1);
				vertexDesc->addVertElem(VET_UBYTE4_NORM, VES_NORMAL, 1, 1);

				mMorphVertexDeclaration = VertexDeclarationCore::create(vertexDesc);
			}
			else
				mMorphVertexDeclaration = nullptr;

			if (oldIsActive != mIsActive)
			{
				if (mIsActive)
					gRenderer()->notifyRenderableAdded(this);
				else
					gRenderer()->notifyRenderableRemoved(this);
			}
			else
			{
				gRenderer()->notifyRenderableRemoved(this);
				gRenderer()->notifyRenderableAdded(this);
			}
		}
	}
	}
}