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

namespace BansheeEngine 
{
	SPtr<VertexBufferCore> GLHardwareBufferCoreManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<GLVertexBufferCore> ret = bs_shared_ptr_new<GLVertexBufferCore>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> GLHardwareBufferCoreManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<GLIndexBufferCore> ret = bs_shared_ptr_new<GLIndexBufferCore>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBufferCore> GLHardwareBufferCoreManager::createGpuParamBlockBufferInternal(UINT32 size, GpuParamBlockUsage usage)
	{
		GLGpuParamBlockBufferCore* paramBlockBuffer = new (bs_alloc<GLGpuParamBlockBufferCore>()) GLGpuParamBlockBufferCore(size, usage);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<GLGpuParamBlockBufferCore>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> GLHardwareBufferCoreManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		GLGpuBufferCore* buffer = new (bs_alloc<GLGpuBufferCore>()) GLGpuBufferCore(desc, deviceMask);

		SPtr<GpuBufferCore> bufferPtr = bs_shared_ptr<GLGpuBufferCore>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

    GLenum GLHardwareBufferCoreManager::getGLUsage(GpuBufferUsage usage)
    {
        switch(usage)
        {
        case GBU_STATIC:
            return GL_STATIC_DRAW;
        case GBU_DYNAMIC:
            return GL_DYNAMIC_DRAW;
        default:
            return GL_DYNAMIC_DRAW;
        };
    }

    GLenum GLHardwareBufferCoreManager::getGLType(VertexElementType type)
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
}
