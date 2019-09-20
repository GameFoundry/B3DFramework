//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsDecal.h"
#include "Private/RTTI/BsDecalRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsRenderer.h"
#include "Material/BsMaterial.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	DecalBase::DecalBase()
	{
		updateBounds();
	}

	DecalBase::DecalBase(const Vector2& size, float maxDistance)
		: mSize(size), mMaxDistance(maxDistance)
	{
		updateBounds();
	}

	void DecalBase::setLayer(UINT64 layer)
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

	void DecalBase::setTransform(const Transform& transform)
	{
		if (mMobility != ObjectMobility::Movable)
			return;

		mTransform = transform;
		mTfrmMatrix = transform.getMatrix();
		mTfrmMatrixNoScale = Matrix4::TRS(transform.getPosition(), transform.getRotation(), Vector3::ONE);

		_markCoreDirty(ActorDirtyFlag::Transform);
	}

	void DecalBase::updateBounds()
	{
		const Vector2& extents = mSize * 0.5f;

		AABox localAABB(
			Vector3(-extents.x, -extents.y, -mMaxDistance),
			Vector3(extents.x, extents.y, 0.0f)
		);

		localAABB.transformAffine(mTfrmMatrix);

		mBounds = Bounds(localAABB, Sphere(localAABB.getCenter(), localAABB.getRadius()));
	}

	template <bool Core>
	template <class P>
	void TDecal<Core>::rttiEnumFields(P p)
	{
		p(mSize);
		p(mMaxDistance);
		p(mMaterial);
		p(mBounds);
		p(mLayer);
		p(mLayerMask);
	}

	Decal::Decal(const HMaterial& material, const Vector2& size, float maxDistance)
		:TDecal(material, size, maxDistance)
	{
		// Calling virtual method is okay here because this is the most derived type
		updateBounds();
	}

	SPtr<ct::Decal> Decal::getCore() const
	{
		return std::static_pointer_cast<ct::Decal>(mCoreSpecific);
	}

	SPtr<Decal> Decal::create(const HMaterial& material, const Vector2& size, float maxDistance)
	{
		Decal* decal = new (bs_alloc<Decal>()) Decal(material, size, maxDistance);
		SPtr<Decal> decalPtr = bs_core_ptr<Decal>(decal);
		decalPtr->_setThisPtr(decalPtr);
		decalPtr->initialize();

		return decalPtr;
	}

	SPtr<Decal> Decal::createEmpty()
	{
		Decal* decal = new (bs_alloc<Decal>()) Decal();
		SPtr<Decal> decalPtr = bs_core_ptr<Decal>(decal);
		decalPtr->_setThisPtr(decalPtr);

		return decalPtr;
	}

	SPtr<ct::CoreObject> Decal::createCore() const
	{
		SPtr<ct::Material> material;
		if(mMaterial.isLoaded(false))
			material = mMaterial->getCore();

		ct::Decal* decal = new (bs_alloc<ct::Decal>()) ct::Decal(material, mSize, mMaxDistance);
		SPtr<ct::Decal> decalPtr = bs_shared_ptr<ct::Decal>(decal);
		decalPtr->_setThisPtr(decalPtr);

		return decalPtr;
	}

	void Decal::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mMaterial.isLoaded())
			dependencies.push_back(mMaterial.get());
	}

	CoreSyncData Decal::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rtti_size(getCoreDirtyFlags()).bytes;
		size += csync_size((SceneActor&)*this);
		size += csync_size(*this);

		UINT8* buffer = allocator->alloc(size);

		Bitstream stream(buffer, size);
		rtti_write(getCoreDirtyFlags(), stream);
		csync_write((SceneActor&)*this, stream);
		csync_write(*this, stream);

		return CoreSyncData(buffer, size);
	}

	void Decal::_markCoreDirty(ActorDirtyFlag flags)
	{
		markCoreDirty((UINT32)flags);
	}

	RTTITypeBase* Decal::getRTTIStatic()
	{
		return DecalRTTI::instance();
	}

	RTTITypeBase* Decal::getRTTI() const
	{
		return Decal::getRTTIStatic();
	}

	template class TDecal<true>;
	template class TDecal<false>;

	namespace ct
	{
	Decal::Decal(const SPtr<Material>& material, const Vector2& size, float maxDistance)
		: TDecal(material, size, maxDistance)
	{ }

	Decal::~Decal()
	{
		gRenderer()->notifyDecalRemoved(this);
	}

	void Decal::initialize()
	{
		updateBounds();
		gRenderer()->notifyDecalAdded(this);

		CoreObject::initialize();
	}

	void Decal::syncToCore(const CoreSyncData& data)
	{
		Bitstream stream(data.getBuffer(), data.getBufferSize());

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mActive;

		rtti_read(dirtyFlags, stream);
		csync_read((SceneActor&)*this, stream);
		csync_read(*this, stream);

		mTfrmMatrix = mTransform.getMatrix();
		mTfrmMatrixNoScale = Matrix4::TRS(mTransform.getPosition(), mTransform.getRotation(), Vector3::ONE);

		updateBounds();

		if (dirtyFlags == (UINT32)ActorDirtyFlag::Transform)
		{
			if (mActive)
				gRenderer()->notifyDecalUpdated(this);
		}
		else
		{
			if (oldIsActive != mActive)
			{
				if (mActive)
					gRenderer()->notifyDecalAdded(this);
				else
					gRenderer()->notifyDecalRemoved(this);
			}
			else
			{
				gRenderer()->notifyDecalRemoved(this);
				gRenderer()->notifyDecalAdded(this);
			}
		}
	}
}}
