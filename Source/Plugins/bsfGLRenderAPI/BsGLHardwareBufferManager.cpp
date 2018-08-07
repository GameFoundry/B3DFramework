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

namespace bs { namespace ct
{
	SPtr<VertexBuffer> GLHardwareBufferManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<GLVertexBuffer> ret = bs_shared_ptr_new<GLVertexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBuffer> GLHardwareBufferManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<GLIndexBuffer> ret = bs_shared_ptr_new<GLIndexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBuffer> GLHardwareBufferManager::createGpuParamBlockBufferInternal(UINT32 size, 
		GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	{
		GLGpuParamBlockBuffer* paramBlockBuffer = 
			new (bs_alloc<GLGpuParamBlockBuffer>()) GLGpuParamBlockBuffer(size, usage, deviceMask);

		SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = bs_shared_ptr<GLGpuParamBlockBuffer>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBuffer> GLHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		GLGpuBuffer* buffer = new (bs_alloc<GLGpuBuffer>()) GLGpuBuffer(desc, deviceMask);

		SPtr<GpuBuffer> bufferPtr = bs_shared_ptr<GLGpuBuffer>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

	GLenum GLHardwareBufferManager::getGLUsage(GpuBufferUsage usage)
	{
		if((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
		{
			if ((usage & GBU_STATIC) != 0)
				return GL_STATIC_READ;

			return GL_DYNAMIC_READ;
		}
		else
		{
			if ((usage & GBU_STATIC) != 0)
				return GL_STATIC_DRAW;

			return GL_DYNAMIC_DRAW;
		}
	}

	GLenum GLHardwareBufferManager::getGLType(VertexElementType type)
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
}}