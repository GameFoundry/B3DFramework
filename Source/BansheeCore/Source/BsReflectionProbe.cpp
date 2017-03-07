//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsReflectionProbe.h"
#include "BsReflectionProbeRTTI.h"
#include "BsSceneObject.h"
#include "BsFrameAlloc.h"
#include "BsTexture.h"
#include "BsRenderer.h"
#include "BsUUID.h"

namespace bs
{
	ReflectionProbeBase::ReflectionProbeBase()
		: mPosition(BsZero), mRotation(BsIdentity), mScale(1.0f, 1.0f, 1.0f), mType(ReflectionProbeType::Box), mRadius(1.0f)
		, mExtents(1.0f, 1.0f, 1.0f), mTransitionDistance(1.0f), mIsActive(true), mBounds(Vector3::ZERO, 1.0f)
	{ }

	ReflectionProbeBase::ReflectionProbeBase(ReflectionProbeType type, float radius, const Vector3& extents)
		: mPosition(BsZero), mRotation(BsIdentity), mScale(1.0f, 1.0f, 1.0f), mType(type), mRadius(radius)
		, mExtents(extents), mTransitionDistance(1.0f), mIsActive(true), mBounds(Vector3::ZERO, 1.0f)
	{ }

	void ReflectionProbeBase::updateBounds()
	{
		switch (mType)
		{
		case ReflectionProbeType::Sphere:
			mBounds = Sphere(mPosition, mRadius * std::max(std::max(mScale.x, mScale.y), mScale.z));
			break;
		case ReflectionProbeType::Box:
		case ReflectionProbeType::Plane:
			mBounds = Sphere(mPosition, (mExtents * mScale).length());
			break;
		}
	}

	template <bool Core>
	TReflectionProbe<Core>::TReflectionProbe()
		:ReflectionProbeBase()
	{ }

	template <bool Core>
	TReflectionProbe<Core>::TReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
		: ReflectionProbeBase(type, radius, extents)
	{ }

	template <bool Core>
	void TReflectionProbe<Core>::generate()
	{
		if (mCustomTexture != nullptr)
			_markCoreDirty();
	}

	template class TReflectionProbe<true>;
	template class TReflectionProbe<false>;

	ReflectionProbe::ReflectionProbe()
		:mLastUpdateHash(0)
	{

	}

	ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
		: TReflectionProbe(type, radius, extents), mLastUpdateHash(0)
	{
		// Calling virtual method is okay here because this is the most derived type
		updateBounds();
	}

	SPtr<ct::ReflectionProbe> ReflectionProbe::getCore() const
	{
		return std::static_pointer_cast<ct::ReflectionProbe>(mCoreSpecific);
	}

	SPtr<ReflectionProbe> ReflectionProbe::createSphere(float radius)
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Sphere, radius, Vector3::ZERO);
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);
		probePtr->mUUID = UUIDGenerator::generateRandom();
		probePtr->initialize();

		return probePtr;
	}

	SPtr<ReflectionProbe> ReflectionProbe::createBox(const Vector3& extents)
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Box, 0.0f, extents);
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);
		probePtr->mUUID = UUIDGenerator::generateRandom();
		probePtr->initialize();

		return probePtr;
	}

	SPtr<ReflectionProbe> ReflectionProbe::createPlane(const Vector3& extents)
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Plane, 0.0f, extents);
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);
		probePtr->mUUID = UUIDGenerator::generateRandom();
		probePtr->initialize();

		return probePtr;
	}

	SPtr<ReflectionProbe> ReflectionProbe::createEmpty()
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe();
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->_setThisPtr(probePtr);

		return probePtr;
	}

	SPtr<ct::CoreObject> ReflectionProbe::createCore() const
	{
		ct::ReflectionProbe* probe = new (bs_alloc<ct::ReflectionProbe>()) ct::ReflectionProbe(mType, mRadius, mExtents);
		SPtr<ct::ReflectionProbe> probePtr = bs_shared_ptr<ct::ReflectionProbe>(probe);
		probePtr->mUUID = mUUID;
		probePtr->_setThisPtr(probePtr);

		return probePtr;
	}

	CoreSyncData ReflectionProbe::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(mPosition);
		size += rttiGetElemSize(mRotation);
		size += rttiGetElemSize(mScale);
		size += rttiGetElemSize(mType);
		size += rttiGetElemSize(mRadius);
		size += rttiGetElemSize(mExtents);
		size += rttiGetElemSize(mTransitionDistance);
		size += rttiGetElemSize(mIsActive);
		size += rttiGetElemSize(getCoreDirtyFlags());
		size += rttiGetElemSize(mBounds);
		size += rttiGetElemSize(sizeof(SPtr<ct::Texture>));
		size += rttiGetElemSize(mUUID);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mPosition, dataPtr);
		dataPtr = rttiWriteElem(mRotation, dataPtr);
		dataPtr = rttiWriteElem(mScale, dataPtr);
		dataPtr = rttiWriteElem(mType, dataPtr);
		dataPtr = rttiWriteElem(mRadius, dataPtr);
		dataPtr = rttiWriteElem(mExtents, dataPtr);
		dataPtr = rttiWriteElem(mTransitionDistance, dataPtr);
		dataPtr = rttiWriteElem(mIsActive, dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);
		dataPtr = rttiWriteElem(mBounds, dataPtr);
		dataPtr = rttiWriteElem(mUUID, dataPtr);

		SPtr<ct::Texture>* customTexture = new (dataPtr)SPtr<ct::Texture>();
		if (mCustomTexture.isLoaded(false))
			*customTexture = mCustomTexture->getCore();
		else
			*customTexture = nullptr;

		dataPtr += sizeof(SPtr<ct::Texture>);

		return CoreSyncData(buffer, size);
	}

	void ReflectionProbe::_updateTransform(const HSceneObject& parent)
	{
		UINT32 curHash = parent->getTransformHash();
		if (curHash != _getLastModifiedHash())
		{
			setPosition(parent->getWorldPosition());
			setRotation(parent->getWorldRotation());
			setScale(parent->getWorldScale());

			_setLastModifiedHash(curHash);
		}
	}

	void ReflectionProbe::_markCoreDirty(ReflectionProbeDirtyFlag flags)
	{
		markCoreDirty((UINT32)flags);
	}

	RTTITypeBase* ReflectionProbe::getRTTIStatic()
	{
		return ReflectionProbeRTTI::instance();
	}

	RTTITypeBase* ReflectionProbe::getRTTI() const
	{
		return ReflectionProbe::getRTTIStatic();
	}

	namespace ct
	{
	ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
			: TReflectionProbe(type, radius, extents), mRendererId(0)
	{

	}

	ReflectionProbe::~ReflectionProbe()
	{
		gRenderer()->notifyReflectionProbeRemoved(this);
	}

	void ReflectionProbe::initialize()
	{
		updateBounds();
		gRenderer()->notifyReflectionProbeAdded(this);

		CoreObject::initialize();
	}

	void ReflectionProbe::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mIsActive;
		ReflectionProbeType oldType = mType;

		dataPtr = rttiReadElem(mPosition, dataPtr);
		dataPtr = rttiReadElem(mRotation, dataPtr);
		dataPtr = rttiReadElem(mScale, dataPtr);
		dataPtr = rttiReadElem(mType, dataPtr);
		dataPtr = rttiReadElem(mRadius, dataPtr);
		dataPtr = rttiReadElem(mExtents, dataPtr);
		dataPtr = rttiReadElem(mTransitionDistance, dataPtr);
		dataPtr = rttiReadElem(mIsActive, dataPtr);
		dataPtr = rttiReadElem(dirtyFlags, dataPtr);
		dataPtr = rttiReadElem(mBounds, dataPtr);
		dataPtr = rttiReadElem(mUUID, dataPtr);

		SPtr<Texture>* texture = (SPtr<Texture>*)dataPtr;

		mCustomTexture = *texture;
		texture->~SPtr<Texture>();
		dataPtr += sizeof(SPtr<Texture>);

		updateBounds();

		if (dirtyFlags == (UINT32)ReflectionProbeDirtyFlag::Transform)
		{
			if (mIsActive)
				gRenderer()->notifyReflectionProbeUpdated(this);
		}
		else
		{
			if (oldIsActive != mIsActive)
			{
				if (mIsActive)
					gRenderer()->notifyReflectionProbeAdded(this);
				else
				{
					ReflectionProbeType newType = mType;
					mType = oldType;
					gRenderer()->notifyReflectionProbeRemoved(this);
					mType = newType;
				}
			}
			else
			{
				ReflectionProbeType newType = mType;
				mType = oldType;
				gRenderer()->notifyReflectionProbeRemoved(this);
				mType = newType;

				gRenderer()->notifyReflectionProbeAdded(this);
			}
		}
	}

}}