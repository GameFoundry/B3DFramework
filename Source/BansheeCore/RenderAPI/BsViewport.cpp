//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "RenderAPI/BsViewport.h"
#include "RTTI/BsViewportRTTI.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Math/BsMath.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Allocators/BsFrameAlloc.h"

namespace bs 
{
	const Color ViewportBase::DEFAULT_CLEAR_COLOR = Color(0.0f, 0.3685f, 0.7969f);

	ViewportBase::ViewportBase(float x, float y, float width, float height)
		: mNormArea(x, y, width, height), mClearFlags(ClearFlagBits::Color | ClearFlagBits::Depth)
		, mClearColorValue(DEFAULT_CLEAR_COLOR), mClearDepthValue(1.0f), mClearStencilValue(0)
	{

	}

	void ViewportBase::setArea(const Rect2& area)
	{
		mNormArea = area;

		_markCoreDirty();
	}

	Rect2I ViewportBase::getPixelArea() const
	{
		float width = (float)getTargetWidth();
		float height = (float)getTargetHeight();
		
		Rect2I area;
		area.x = (int)(mNormArea.x * width);
		area.y = (int)(mNormArea.y * height);
		area.width = (int)(mNormArea.width * width);
		area.height = (int)(mNormArea.height * height);

		return area;
	}

	void ViewportBase::setClearFlags(ClearFlags flags)
	{
		mClearFlags = flags;

		_markCoreDirty();
	}

	void ViewportBase::setClearValues(const Color& clearColor, float clearDepth, UINT16 clearStencil)
	{
		mClearColorValue = clearColor;
		mClearDepthValue = clearDepth;
		mClearStencilValue = clearStencil;

		_markCoreDirty();
	}

	void ViewportBase::setClearColorValue(const Color& color)
	{
		mClearColorValue = color;

		_markCoreDirty();
	}

	void ViewportBase::setClearDepthValue(float depth)
	{
		mClearDepthValue = depth;

		_markCoreDirty();
	}

	void ViewportBase::setClearStencilValue(UINT16 value)
	{
		mClearStencilValue = value;

		_markCoreDirty();
	}

	Viewport::Viewport()
		:ViewportBase()
	{ }

	Viewport::Viewport(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
		:ViewportBase(x, y, width, height), mTarget(target)
	{

	}

	void Viewport::setTarget(const SPtr<RenderTarget>& target)
	{
		mTarget = target; 
		
		markDependenciesDirty();
		_markCoreDirty();
	}

	SPtr<ct::Viewport> Viewport::getCore() const
	{
		return std::static_pointer_cast<ct::Viewport>(mCoreSpecific);
	}

	void Viewport::_markCoreDirty()
	{
		markCoreDirty();
	}

	UINT32 Viewport::getTargetWidth() const
	{
		if (mTarget != nullptr)
			return mTarget->getProperties().width;

		return 0;
	}

	UINT32 Viewport::getTargetHeight() const
	{
		if(mTarget != nullptr)
			return mTarget->getProperties().height;

		return 0;
	}

	SPtr<ct::CoreObject> Viewport::createCore() const
	{
		SPtr<ct::RenderTarget> targetCore;
		if (mTarget != nullptr)
			targetCore = mTarget->getCore();

		ct::Viewport* viewport = new (bs_alloc<ct::Viewport>())
			ct::Viewport(targetCore, mNormArea.x, mNormArea.y, mNormArea.width, mNormArea.height);

		SPtr<ct::Viewport> viewportPtr = bs_shared_ptr<ct::Viewport>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);

		return viewportPtr;
	}

	CoreSyncData Viewport::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(mNormArea);
		size += rttiGetElemSize(mClearFlags);
		size += rttiGetElemSize(mClearColorValue);
		size += rttiGetElemSize(mClearDepthValue);
		size += rttiGetElemSize(mClearStencilValue);
		size += sizeof(SPtr<ct::RenderTarget>);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mNormArea, dataPtr);
		dataPtr = rttiWriteElem(mClearFlags, dataPtr);
		dataPtr = rttiWriteElem(mClearColorValue, dataPtr);
		dataPtr = rttiWriteElem(mClearDepthValue, dataPtr);
		dataPtr = rttiWriteElem(mClearStencilValue, dataPtr);

		SPtr<ct::RenderTarget>* rtPtr = new (dataPtr) SPtr<ct::RenderTarget>();
		if (mTarget != nullptr)
			*rtPtr = mTarget->getCore();
		else
			*rtPtr = nullptr;

		return CoreSyncData(buffer, size);
	}

	void Viewport::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mTarget != nullptr)
			dependencies.push_back(mTarget.get());
	}

	SPtr<Viewport> Viewport::create(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
	{
		Viewport* viewport = new (bs_alloc<Viewport>()) Viewport(target, x, y, width, height);
		SPtr<Viewport> viewportPtr = bs_core_ptr<Viewport>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);
		viewportPtr->initialize();

		return viewportPtr;
	}

	SPtr<Viewport> Viewport::createEmpty()
	{
		Viewport* viewport = new (bs_alloc<Viewport>()) Viewport();
		SPtr<Viewport> viewportPtr = bs_core_ptr<Viewport>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);

		return viewportPtr;
	}

	RTTITypeBase* Viewport::getRTTIStatic()
	{
		return ViewportRTTI::instance();
	}

	RTTITypeBase* Viewport::getRTTI() const
	{
		return Viewport::getRTTIStatic();
	}

	namespace ct
	{
	Viewport::Viewport(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
		:ViewportBase(x, y, width, height), mTarget(target)
	{

	}

	SPtr<Viewport> Viewport::create(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
	{
		Viewport* viewport = new (bs_alloc<Viewport>()) Viewport(target, x, y, width, height);

		SPtr<Viewport> viewportPtr = bs_shared_ptr<Viewport>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);
		viewportPtr->initialize();

		return viewportPtr;
	}

	UINT32 Viewport::getTargetWidth() const
	{
		if (mTarget != nullptr)
			return mTarget->getProperties().width;

		return 0;
	}

	UINT32 Viewport::getTargetHeight() const
	{
		if (mTarget != nullptr)
			return mTarget->getProperties().height;

		return 0;
	}

	void Viewport::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();
		dataPtr = rttiReadElem(mNormArea, dataPtr);
		dataPtr = rttiReadElem(mClearFlags, dataPtr);
		dataPtr = rttiReadElem(mClearColorValue, dataPtr);
		dataPtr = rttiReadElem(mClearDepthValue, dataPtr);
		dataPtr = rttiReadElem(mClearStencilValue, dataPtr);

		SPtr<RenderTarget>* rtPtr = (SPtr<RenderTarget>*)dataPtr;
		mTarget = *rtPtr;

		rtPtr->~SPtr<RenderTarget>();
	}
	}
}
