//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsViewport.h"
#include "Private/RTTI/BsViewportRTTI.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Math/BsMath.h"
#include "RenderAPI/BsRenderAPI.h"
#include "CoreThread/BsCoreObjectSync.h"

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

	template <bool Core>
	template <class P>
	void TViewport<Core>::rttiEnumFields(P p)
	{
		p(mNormArea);
		p(mClearFlags);
		p(mClearColorValue);
		p(mClearDepthValue);
		p(mClearStencilValue);
		p(mTarget);
	}

	Viewport::Viewport(const SPtr<RenderTarget>& target, float x, float y, float width, float height)
		:TViewport(target, x, y, width, height)
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
		UINT32 size = csync_size(*this);

		UINT8* buffer = allocator->alloc(size);
		Bitstream stream(buffer, size);

		csync_write(*this, stream);

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
		:TViewport(target, x, y, width, height)
	{ }

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
		Bitstream stream(data.getBuffer(), data.getBufferSize());
		csync_read(*this, stream);
	}
	}
}
