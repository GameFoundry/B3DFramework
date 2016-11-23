//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsViewport.h"
#include "BsViewportRTTI.h"
#include "BsException.h"
#include "BsRenderTarget.h"
#include "BsMath.h"
#include "BsRenderAPI.h"
#include "BsFrameAlloc.h"

namespace bs 
{
	const Color ViewportBase::DEFAULT_CLEAR_COLOR = Color(0.0f, 0.3685f, 0.7969f);

	ViewportBase::ViewportBase(float x, float y, float width, float height)
		: mNormArea(x, y, width, height), mRequiresColorClear(true), mRequiresDepthClear(true)
		, mRequiresStencilClear(false), mClearColor(DEFAULT_CLEAR_COLOR), mDepthClearValue(1.0f), mStencilClearValue(0)
    {

    }

	void ViewportBase::setArea(float x, float y, float width, float height)
    {
        mNormArea.x = x;
        mNormArea.y = y;
        mNormArea.width = width;
        mNormArea.height = height;

		_markCoreDirty();
    }

	Rect2I ViewportBase::getArea() const
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

	void ViewportBase::setRequiresClear(bool colorClear, bool depthClear, bool stencilClear)
	{
		mRequiresColorClear = colorClear;
		mRequiresDepthClear = depthClear;
		mRequiresStencilClear = stencilClear;

		_markCoreDirty();
	}

	void ViewportBase::setClearValues(const Color& clearColor, float clearDepth, UINT16 clearStencil)
	{
		mClearColor = clearColor;
		mDepthClearValue = clearDepth;
		mStencilClearValue = clearStencil;

		_markCoreDirty();
	}

	INT32 ViewportBase::getX() const
	{ 
		return (INT32)(mNormArea.x * getTargetWidth());
	}

	INT32 ViewportBase::getY() const
	{ 
		return (INT32)(mNormArea.y * getTargetHeight());
	}

	INT32 ViewportBase::getWidth() const
	{ 
		return (INT32)(mNormArea.width * getTargetWidth());
	}

	INT32 ViewportBase::getHeight() const
	{ 
		return (INT32)(mNormArea.height * getTargetHeight());
	}

	ViewportCore::ViewportCore(const SPtr<RenderTargetCore>& target, float x, float y, float width, float height)
		:ViewportBase(x, y, width, height), mTarget(target)
	{

	}

	SPtr<ViewportCore> ViewportCore::create(const SPtr<RenderTargetCore>& target, float x, float y, float width, float height)
	{
		ViewportCore* viewport = new (bs_alloc<Viewport>()) ViewportCore(target, x, y, width, height);

		SPtr<ViewportCore> viewportPtr = bs_shared_ptr<ViewportCore>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);
		viewportPtr->initialize();

		return viewportPtr;
	}

	UINT32 ViewportCore::getTargetWidth() const
	{
		if (mTarget != nullptr)
			return mTarget->getProperties().getWidth();

		return 0;
	}

	UINT32 ViewportCore::getTargetHeight() const
	{
		if (mTarget != nullptr)
			return mTarget->getProperties().getHeight();

		return 0;
	}

	void ViewportCore::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();
		dataPtr = rttiReadElem(mNormArea, dataPtr);
		dataPtr = rttiReadElem(mRequiresColorClear, dataPtr);
		dataPtr = rttiReadElem(mRequiresDepthClear, dataPtr);
		dataPtr = rttiReadElem(mRequiresStencilClear, dataPtr);
		dataPtr = rttiReadElem(mClearColor, dataPtr);
		dataPtr = rttiReadElem(mDepthClearValue, dataPtr);
		dataPtr = rttiReadElem(mStencilClearValue, dataPtr);

		SPtr<RenderTargetCore>* rtPtr = (SPtr<RenderTargetCore>*)dataPtr;
		mTarget = *rtPtr;

		rtPtr->~SPtr<RenderTargetCore>();
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

	SPtr<ViewportCore> Viewport::getCore() const
	{
		return std::static_pointer_cast<ViewportCore>(mCoreSpecific);
	}

	void Viewport::_markCoreDirty()
	{
		markCoreDirty();
	}

	UINT32 Viewport::getTargetWidth() const
	{
		if(mTarget != nullptr)
			return mTarget->getProperties().getWidth();

		return 0;
	}

	UINT32 Viewport::getTargetHeight() const
	{
		if(mTarget != nullptr)
			return mTarget->getProperties().getHeight();

		return 0;
	}

	SPtr<CoreObjectCore> Viewport::createCore() const
	{
		SPtr<RenderTargetCore> targetCore;
		if (mTarget != nullptr)
			targetCore = mTarget->getCore();

		ViewportCore* viewport = new (bs_alloc<Viewport>()) 
			ViewportCore(targetCore, mNormArea.x, mNormArea.y, mNormArea.width, mNormArea.height);

		SPtr<ViewportCore> viewportPtr = bs_shared_ptr<ViewportCore>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);

		return viewportPtr;
	}

	CoreSyncData Viewport::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(mNormArea);
		size += rttiGetElemSize(mRequiresColorClear);
		size += rttiGetElemSize(mRequiresDepthClear);
		size += rttiGetElemSize(mRequiresStencilClear);
		size += rttiGetElemSize(mClearColor);
		size += rttiGetElemSize(mDepthClearValue);
		size += rttiGetElemSize(mStencilClearValue);
		size += sizeof(SPtr<RenderTargetCore>);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mNormArea, dataPtr);
		dataPtr = rttiWriteElem(mRequiresColorClear, dataPtr);
		dataPtr = rttiWriteElem(mRequiresDepthClear, dataPtr);
		dataPtr = rttiWriteElem(mRequiresStencilClear, dataPtr);
		dataPtr = rttiWriteElem(mClearColor, dataPtr);
		dataPtr = rttiWriteElem(mDepthClearValue, dataPtr);
		dataPtr = rttiWriteElem(mStencilClearValue, dataPtr);

		SPtr<RenderTargetCore>* rtPtr = new (dataPtr) SPtr<RenderTargetCore>();
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
}
