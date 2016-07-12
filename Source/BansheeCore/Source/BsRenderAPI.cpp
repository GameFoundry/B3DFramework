//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderAPI.h"

#include "BsCoreThread.h"
#include "BsViewport.h"
#include "BsRenderTarget.h"
#include "BsRenderWindow.h"
#include "BsResource.h"
#include "BsMesh.h"
#include "BsRenderStats.h"
#include "BsGpuParams.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsRasterizerState.h"
#include "BsGpuParamDesc.h"
#include "BsGpuBuffer.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsShader.h"

using namespace std::placeholders;

namespace BansheeEngine 
{
	void RenderAPI::setTexture(CoreAccessor& accessor, GpuProgramType gptype, UINT16 unit, const SPtr<Texture> &texPtr)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setTexture, RenderAPICore::instancePtr(), gptype, unit, texPtr->getCore()));
	}

	void RenderAPI::setLoadStoreTexture(CoreAccessor& accessor, GpuProgramType gptype, UINT16 unit, bool enabled, const SPtr<Texture>& texPtr,
		const TextureSurface& surface)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setLoadStoreTexture, RenderAPICore::instancePtr(), gptype, unit, enabled, texPtr->getCore(),
			surface));
	}

	void RenderAPI::setBuffer(CoreAccessor& accessor, GpuProgramType gptype, UINT16 unit, const SPtr<GpuBuffer>& buffer,
		bool loadStore)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setBuffer, RenderAPICore::instancePtr(), gptype, unit, 
			buffer->getCore(), loadStore));
	}

	void RenderAPI::setSamplerState(CoreAccessor& accessor, GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerState>& samplerState)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setSamplerState, RenderAPICore::instancePtr(), gptype, texUnit, samplerState->getCore()));
	}

	void RenderAPI::setBlendState(CoreAccessor& accessor, const SPtr<BlendState>& blendState)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setBlendState, RenderAPICore::instancePtr(), blendState->getCore()));
	}

	void RenderAPI::setRasterizerState(CoreAccessor& accessor, const SPtr<RasterizerState>& rasterizerState)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setRasterizerState, RenderAPICore::instancePtr(), rasterizerState->getCore()));
	}

	void RenderAPI::setDepthStencilState(CoreAccessor& accessor, const SPtr<DepthStencilState>& depthStencilState, UINT32 stencilRefValue)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setDepthStencilState, RenderAPICore::instancePtr(), depthStencilState->getCore(), stencilRefValue));
	}

	void RenderAPI::setVertexBuffers(CoreAccessor& accessor, UINT32 index, const Vector<SPtr<VertexBuffer>>& buffers)
	{
		Vector<SPtr<VertexBufferCore>> coreBuffers(buffers.size());
		for (UINT32 i = 0; i < (UINT32)buffers.size(); i++)
			coreBuffers[i] = buffers[i] != nullptr ? buffers[i]->getCore() : nullptr;

		std::function<void(RenderAPICore*, UINT32, const Vector<SPtr<VertexBufferCore>>&)> resizeFunc =
			[](RenderAPICore* rs, UINT32 idx, const Vector<SPtr<VertexBufferCore>>& _buffers)
		{
			rs->setVertexBuffers(idx, (SPtr<VertexBufferCore>*)_buffers.data(), (UINT32)_buffers.size());
		};

		accessor.queueCommand(std::bind(resizeFunc, RenderAPICore::instancePtr(), index, coreBuffers));
	}

	void RenderAPI::setIndexBuffer(CoreAccessor& accessor, const SPtr<IndexBuffer>& buffer)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setIndexBuffer, RenderAPICore::instancePtr(), buffer->getCore()));
	}

	void RenderAPI::setVertexDeclaration(CoreAccessor& accessor, const SPtr<VertexDeclaration>& vertexDeclaration)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setVertexDeclaration, RenderAPICore::instancePtr(), vertexDeclaration->getCore()));
	}

	void RenderAPI::setViewport(CoreAccessor& accessor, const Rect2& vp)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setViewport, RenderAPICore::instancePtr(), vp));
	}

	void RenderAPI::setDrawOperation(CoreAccessor& accessor, DrawOperationType op)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setDrawOperation, RenderAPICore::instancePtr(), op));
	}

	void RenderAPI::setClipPlanes(CoreAccessor& accessor, const PlaneList& clipPlanes)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setClipPlanes, RenderAPICore::instancePtr(), clipPlanes));
	}

	void RenderAPI::addClipPlane(CoreAccessor& accessor, const Plane& p)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::addClipPlane, RenderAPICore::instancePtr(), p));
	}

	void RenderAPI::resetClipPlanes(CoreAccessor& accessor)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::resetClipPlanes, RenderAPICore::instancePtr()));
	}

	void RenderAPI::setScissorRect(CoreAccessor& accessor, UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setScissorRect, RenderAPICore::instancePtr(), left, top, right, bottom));
	}

	void RenderAPI::setRenderTarget(CoreAccessor& accessor, const SPtr<RenderTarget>& target, bool readOnlyDepthStencil)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setRenderTarget, 
			RenderAPICore::instancePtr(), target->getCore(), readOnlyDepthStencil));
	}

	void RenderAPI::bindGpuProgram(CoreAccessor& accessor, const SPtr<GpuProgram>& prg)
	{
		prg->syncToCore(accessor);
		accessor.queueCommand(std::bind(&RenderAPICore::bindGpuProgram, RenderAPICore::instancePtr(), prg->getCore()));
	}

	void RenderAPI::unbindGpuProgram(CoreAccessor& accessor, GpuProgramType gptype)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::unbindGpuProgram, RenderAPICore::instancePtr(), gptype));
	}

	void RenderAPI::beginRender(CoreAccessor& accessor)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::beginFrame, RenderAPICore::instancePtr()));
	}

	void RenderAPI::endRender(CoreAccessor& accessor)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::endFrame, RenderAPICore::instancePtr()));
	}

	void RenderAPI::clearRenderTarget(CoreAccessor& accessor, UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::clearRenderTarget, RenderAPICore::instancePtr(), buffers, color,
			depth, stencil, targetMask));
	}

	void RenderAPI::clearViewport(CoreAccessor& accessor, UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::clearViewport, RenderAPICore::instancePtr(), buffers, color, depth,
			stencil, targetMask));
	}

	void RenderAPI::swapBuffers(CoreAccessor& accessor, const SPtr<RenderTarget>& target)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::swapBuffers, RenderAPICore::instancePtr(), target->getCore()));
	}

	void RenderAPI::draw(CoreAccessor& accessor, UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::draw, RenderAPICore::instancePtr(), vertexOffset, 
			vertexCount, instanceCount));
	}

	void RenderAPI::drawIndexed(CoreAccessor& accessor, UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, 
		UINT32 vertexCount, UINT32 instanceCount)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::drawIndexed, RenderAPICore::instancePtr(), startIndex, indexCount, 
			vertexOffset, vertexCount, instanceCount));
	}

	void RenderAPI::dispatchCompute(CoreAccessor& accessor, UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::dispatchCompute, RenderAPICore::instancePtr(), numGroupsX, 
			numGroupsY, numGroupsZ));
	}

	const VideoModeInfo& RenderAPI::getVideoModeInfo()
	{
		return RenderAPICore::instance().getVideoModeInfo();
	}

	void RenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{
		RenderAPICore::instance().convertProjectionMatrix(matrix, dest);
	}

	const RenderAPIInfo& RenderAPI::getAPIInfo()
	{
		return RenderAPICore::instance().getAPIInfo();
	}

    RenderAPICore::RenderAPICore()
        : mCullingMode(CULL_COUNTERCLOCKWISE)
        , mDisabledTexUnitsFrom(0)
        , mVertexProgramBound(false)
		, mGeometryProgramBound(false)
        , mFragmentProgramBound(false)
		, mDomainProgramBound(false)
		, mHullProgramBound(false)
		, mComputeProgramBound(false)
		, mClipPlanesDirty(true)
		, mCurrentCapabilities(nullptr)
    {
    }

    RenderAPICore::~RenderAPICore()
    {
		// Base classes need to call virtual destroy_internal method instead of a destructor

		bs_delete(mCurrentCapabilities);
		mCurrentCapabilities = nullptr;
    }

	SPtr<RenderWindow> RenderAPICore::initialize(const RENDER_WINDOW_DESC& primaryWindowDesc)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::initializePrepare, this), true);

		RENDER_WINDOW_DESC windowDesc = primaryWindowDesc;
		SPtr<RenderWindow> renderWindow = RenderWindow::create(windowDesc, nullptr);

		gCoreThread().queueCommand(std::bind(&RenderAPICore::initializeFinalize, this, renderWindow->getCore()), true);

		return renderWindow;
	}

	void RenderAPICore::initializePrepare()
	{
		// Do nothing
	}

	void RenderAPICore::initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow)
	{
		THROW_IF_NOT_CORE_THREAD;

		mVertexProgramBound = false;
		mGeometryProgramBound = false;
		mFragmentProgramBound = false;
		mDomainProgramBound = false;
		mHullProgramBound = false;
		mComputeProgramBound = false;
	}

	void RenderAPICore::destroy()
	{
		gCoreAccessor().queueCommand(std::bind(&RenderAPICore::destroyCore, this));
		gCoreThread().submitAccessors(true);
	}

	void RenderAPICore::destroyCore()
	{
		mActiveRenderTarget = nullptr;
	}

	const RenderAPICapabilities* RenderAPICore::getCapabilities(void) const 
	{ 
		return mCurrentCapabilities; 
	}

	const DriverVersion& RenderAPICore::getDriverVersion(void) const 
	{ 
		THROW_IF_NOT_CORE_THREAD;

		return mDriverVersion; 
	}

	void RenderAPICore::addClipPlane(const Plane &p)
	{
		THROW_IF_NOT_CORE_THREAD;

		mClipPlanes.push_back(p);
		mClipPlanesDirty = true;
	}

	void RenderAPICore::setClipPlanes(const PlaneList& clipPlanes)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (clipPlanes != mClipPlanes)
		{
			mClipPlanes = clipPlanes;
			mClipPlanesDirty = true;
		}
	}

	void RenderAPICore::resetClipPlanes()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (!mClipPlanes.empty())
		{
			mClipPlanes.clear();
			mClipPlanesDirty = true;
		}
	}

	void RenderAPICore::bindGpuProgram(const SPtr<GpuProgramCore>& prg)
	{
		THROW_IF_NOT_CORE_THREAD;

		switch(prg->getProperties().getType())
		{
		case GPT_VERTEX_PROGRAM:
			if (!mVertexProgramBound && !mClipPlanes.empty())
				mClipPlanesDirty = true;

			mVertexProgramBound = true;
			break;
		case GPT_GEOMETRY_PROGRAM:
			mGeometryProgramBound = true;
			break;
		case GPT_FRAGMENT_PROGRAM:
			mFragmentProgramBound = true;
			break;
		case GPT_DOMAIN_PROGRAM:
			mDomainProgramBound = true;
			break;
		case GPT_HULL_PROGRAM:
			mHullProgramBound = true;
			break;
		case GPT_COMPUTE_PROGRAM:
			mComputeProgramBound = true;
			break;
		}
	}

	void RenderAPICore::unbindGpuProgram(GpuProgramType gptype)
	{
		THROW_IF_NOT_CORE_THREAD;

		switch(gptype)
		{
		case GPT_VERTEX_PROGRAM:
			if (mVertexProgramBound && !mClipPlanes.empty())
				mClipPlanesDirty = true;

			mVertexProgramBound = false;
			break;
		case GPT_GEOMETRY_PROGRAM:
			mGeometryProgramBound = false;
			break;
		case GPT_FRAGMENT_PROGRAM:
			mFragmentProgramBound = false;
			break;
		case GPT_DOMAIN_PROGRAM:
			mDomainProgramBound = false;
			break;
		case GPT_HULL_PROGRAM:
			mHullProgramBound = false;
			break;
		case GPT_COMPUTE_PROGRAM:
			mComputeProgramBound = false;
			break;
		}
	}

	bool RenderAPICore::isGpuProgramBound(GpuProgramType gptype)
	{
		THROW_IF_NOT_CORE_THREAD;

	    switch(gptype)
	    {
        case GPT_VERTEX_PROGRAM:
            return mVertexProgramBound;
        case GPT_GEOMETRY_PROGRAM:
            return mGeometryProgramBound;
        case GPT_FRAGMENT_PROGRAM:
            return mFragmentProgramBound;
		case GPT_DOMAIN_PROGRAM:
			return mDomainProgramBound;
		case GPT_HULL_PROGRAM:
			return mHullProgramBound;
		case GPT_COMPUTE_PROGRAM:
			return mComputeProgramBound;
	    }

        return false;
	}
	
	void RenderAPICore::swapBuffers(const SPtr<RenderTargetCore>& target)
	{
		THROW_IF_NOT_CORE_THREAD;

		target->swapBuffers();

		BS_INC_RENDER_STAT(NumPresents);
	}
}