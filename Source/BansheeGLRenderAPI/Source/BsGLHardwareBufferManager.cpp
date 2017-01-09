//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLHardwareBufferManager.h"
#include "BsGLVertexBuffer.h"
#include "BsGLIndexBuffer.h"
#include "BsGLGpuBuffer.h"
#include "BsHardwareBuffer.h"
#include "BsGLGpuParamBlockBuffer.h"
#include "BsRenderAPI.h"
#include "BsRenderAPICapabilities.h"

namespace bs { namespace ct
{
	SPtr<VertexBufferCore> GLHardwareBufferManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<GLVertexBuffer> ret = bs_shared_ptr_new<GLVertexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> GLHardwareBufferManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<GLIndexBuffer> ret = bs_shared_ptr_new<GLIndexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBufferCore> GLHardwareBufferManager::createGpuParamBlockBufferInternal(UINT32 size, 
		GpuParamBlockUsage usage, GpuDeviceFlags deviceMask)
	{
		GLGpuParamBlockBuffer* paramBlockBuffer = 
			new (bs_alloc<GLGpuParamBlockBuffer>()) GLGpuParamBlockBuffer(size, usage, deviceMask);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<GLGpuParamBlockBuffer>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> GLHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		GLGpuBuffer* buffer = new (bs_alloc<GLGpuBuffer>()) GLGpuBuffer(desc, deviceMask);

		SPtr<GpuBufferCore> bufferPtr = bs_shared_ptr<GLGpuBuffer>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

    GLenum GLHardwareBufferManager::getGLUsage(GpuBufferUsage usage)
    {
		if(usage & GBU_STATIC)
			return GL_STATIC_DRAW;

		if(usage & GBU_DYNAMIC)
			return GL_DYNAMIC_DRAW;

        return GL_DYNAMIC_DRAW;
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