//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLHardwareBufferManager.h"
#include "BsGLVertexBuffer.h"
#include "BsGLIndexBuffer.h"
#include "BsGLGpuBuffer.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "BsGLGpuParamBlockBuffer.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderAPICapabilities.h"

using namespace bs;
using namespace bs::ct;

SPtr<ct::VertexBuffer> GLHardwareBufferManager::CreateVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<GLVertexBuffer> ret = B3DMakeShared<GLVertexBuffer>(desc, deviceMask);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<ct::IndexBuffer> GLHardwareBufferManager::CreateIndexBufferInternal(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<GLIndexBuffer> ret = B3DMakeShared<GLIndexBuffer>(desc, deviceMask);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<ct::GpuParamBlockBuffer> GLHardwareBufferManager::CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
{
	GLGpuParamBlockBuffer* paramBlockBuffer =
		new(B3DAllocate<GLGpuParamBlockBuffer>()) GLGpuParamBlockBuffer(size, usage, deviceMask);

	SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = B3DMakeSharedFromExisting<GLGpuParamBlockBuffer>(paramBlockBuffer);
	paramBlockBufferPtr->SetThisPtrInternal(paramBlockBufferPtr);

	return paramBlockBufferPtr;
}

SPtr<ct::GpuBuffer> GLHardwareBufferManager::CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	GLGpuBuffer* buffer = new(B3DAllocate<GLGpuBuffer>()) GLGpuBuffer(desc, deviceMask);

	SPtr<GpuBuffer> bufferPtr = B3DMakeSharedFromExisting<GLGpuBuffer>(buffer);
	bufferPtr->SetThisPtrInternal(bufferPtr);

	return bufferPtr;
}

SPtr<ct::GpuBuffer> GLHardwareBufferManager::CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
{
	GLGpuBuffer* buffer = new(B3DAllocate<GLGpuBuffer>()) GLGpuBuffer(desc, std::move(underlyingBuffer));

	SPtr<GpuBuffer> bufferPtr = B3DMakeSharedFromExisting<GLGpuBuffer>(buffer);
	bufferPtr->SetThisPtrInternal(bufferPtr);

	return bufferPtr;
}

GLenum GLHardwareBufferManager::GetGlUsage(GpuBufferUsage usage)
{
	if((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
	{
		if((usage & GBU_STATIC) != 0)
			return GL_STATIC_READ;

		return GL_DYNAMIC_READ;
	}
	else
	{
		if((usage & GBU_STATIC) != 0)
			return GL_STATIC_DRAW;

		return GL_DYNAMIC_DRAW;
	}
}

GLenum GLHardwareBufferManager::GetGlType(VertexElementType type)
{
	switch(type)
	{
	case VET_FLOAT1:
	case VET_FLOAT2:
	case VET_FLOAT3:
	case VET_FLOAT4:
		return GL_FLOAT;
	case VET_SHORT1:
	case VET_SHORT2:
	case VET_SHORT4:
		return GL_SHORT;
	case VET_USHORT1:
	case VET_USHORT2:
	case VET_USHORT4:
		return GL_UNSIGNED_SHORT;
	case VET_INT1:
	case VET_INT2:
	case VET_INT3:
	case VET_INT4:
		return GL_INT;
	case VET_UINT1:
	case VET_UINT2:
	case VET_UINT3:
	case VET_UINT4:
		return GL_UNSIGNED_INT;
	case VET_COLOR:
	case VET_COLOR_ABGR:
	case VET_COLOR_ARGB:
	case VET_UBYTE4:
	case VET_UBYTE4_NORM:
		return GL_UNSIGNED_BYTE;
	default:
		return 0;
	};
}
