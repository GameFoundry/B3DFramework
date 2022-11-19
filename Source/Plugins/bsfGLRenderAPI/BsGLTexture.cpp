//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "BsGLPixelFormat.h"
#include "BsGLPixelBuffer.h"
#include "Error/BsException.h"
#include "Utility/BsBitwise.h"
#include "CoreThread/BsCoreThread.h"
#include "Managers/BsTextureManager.h"
#include "BsGLRenderTexture.h"
#include "BsGLTextureView.h"
#include "Profiling/BsRenderStats.h"
#include "BsGLCommandBuffer.h"

using namespace bs;
using namespace bs::ct;

GLTexture::GLTexture(GLSupport& support, const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	: Texture(desc, initialData, deviceMask), mGLSupport(support)
{
	B3D_ASSERT((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
}

GLTexture::~GLTexture()
{
	mSurfaceList.clear();
	glDeleteTextures(1, &mTextureID);
	B3D_CHECK_GL_ERROR();

	ClearBufferViews();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
}

void GLTexture::Initialize()
{
	u32 width = mProperties.GetWidth();
	u32 height = mProperties.GetHeight();
	u32 depth = mProperties.GetDepth();
	TextureType texType = mProperties.GetTextureType();
	int usage = mProperties.GetUsage();
	u32 numMips = mProperties.GetNumMipmaps();
	u32 numFaces = mProperties.GetNumFaces();

	// 0-sized textures aren't supported by the API
	width = std::max(width, 1U);
	height = std::max(height, 1U);

	PixelFormat pixFormat = mProperties.GetFormat();
	mInternalFormat = GLPixelUtil::GetClosestSupportedPf(pixFormat, texType, usage);

	if(pixFormat != mInternalFormat)
	{
		B3D_LOG(Warning, RenderBackend, "Provided pixel format is not supported by the driver: {0}. "
									   "Falling back on: {1}.",
			   pixFormat, mInternalFormat);
	}

	// Check requested number of mipmaps
	u32 maxMips = PixelUtil::GetMaxMipmaps(width, height, depth, mProperties.GetFormat());
	if(numMips > maxMips)
	{
		B3D_LOG(Error, RenderBackend, "Invalid number of mipmaps. Maximum allowed is: {0}", maxMips);
		numMips = maxMips;
	}

	if((usage & TU_DEPTHSTENCIL) != 0)
	{
		if(texType != TEX_TYPE_2D && texType != TEX_TYPE_CUBE_MAP)
		{
			B3D_LOG(Error, RenderBackend, "Only 2D and cubemap depth stencil textures are supported. Ignoring depth-stencil flag.");
			usage &= ~TU_DEPTHSTENCIL;
		}
	}

	// Include the base mip level
	numMips += 1;

	// Generate texture handle
	glGenTextures(1, &mTextureID);
	B3D_CHECK_GL_ERROR();

	// Set texture type
	glBindTexture(GetGlTextureTarget(), mTextureID);
	B3D_CHECK_GL_ERROR();

	if(mProperties.GetNumSamples() <= 1)
	{
		// This needs to be set otherwise the texture doesn't get rendered
		glTexParameteri(GetGlTextureTarget(), GL_TEXTURE_MAX_LEVEL, numMips - 1);
		B3D_CHECK_GL_ERROR();
	}

	// Allocate internal buffer so that glTexSubImageXD can be used
	mGLFormat = GLPixelUtil::GetGlInternalFormat(mInternalFormat, mProperties.IsHardwareGammaEnabled());

	u32 sampleCount = mProperties.GetNumSamples();
	if((usage & (TU_RENDERTARGET | TU_DEPTHSTENCIL)) != 0 && mProperties.GetTextureType() == TEX_TYPE_2D && sampleCount > 1)
	{
		if(numFaces <= 1)
		{
			// Create immutable storage if available, fallback to mutable
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, mGLFormat, width, height, GL_TRUE);
			B3D_CHECK_GL_ERROR();
#else
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, mGLFormat, width, height, GL_TRUE);
			B3D_CHECK_GL_ERROR();
#endif
		}
		else
		{
			// Create immutable storage if available, fallback to mutable
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
			glTexStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, sampleCount, mGLFormat, width, height, numFaces, GL_TRUE);
			B3D_CHECK_GL_ERROR();
#else
			glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, sampleCount, mGLFormat, width, height, numFaces, GL_TRUE);
			B3D_CHECK_GL_ERROR();
#endif
		}
	}
	else
	{
		// Create immutable storage if available, fallback to mutable
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
		switch(texType)
		{
		case TEX_TYPE_1D:
			{
				if(numFaces <= 1)
				{
					glTexStorage1D(GL_TEXTURE_1D, numMips, mGLFormat, width);
					B3D_CHECK_GL_ERROR();
				}
				else
				{
					glTexStorage2D(GL_TEXTURE_1D_ARRAY, numMips, mGLFormat, width, numFaces);
					B3D_CHECK_GL_ERROR();
				}
			}
			break;
		case TEX_TYPE_2D:
			{
				if(numFaces <= 1)
				{
					glTexStorage2D(GL_TEXTURE_2D, numMips, mGLFormat, width, height);
					B3D_CHECK_GL_ERROR();
				}
				else
				{
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, numMips, mGLFormat, width, height, numFaces);
					B3D_CHECK_GL_ERROR();
				}
			}
			break;
		case TEX_TYPE_3D:
			glTexStorage3D(GL_TEXTURE_3D, numMips, mGLFormat, width, height, depth);
			B3D_CHECK_GL_ERROR();
			break;
		case TEX_TYPE_CUBE_MAP:
			{
				if(numFaces <= 6)
				{
					glTexStorage2D(GL_TEXTURE_CUBE_MAP, numMips, mGLFormat, width, height);
					B3D_CHECK_GL_ERROR();
				}
				else
				{
					glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, numMips, mGLFormat, width, height, numFaces);
					B3D_CHECK_GL_ERROR();
				}
			}
			break;
		}
#else
		if((usage & TU_DEPTHSTENCIL) != 0)
		{
			GLenum depthStencilType = GLPixelUtil::getDepthStencilTypeFromPF(mInternalFormat);
			GLenum depthStencilFormat = GLPixelUtil::getDepthStencilFormatFromPF(mInternalFormat);

			if(texType == TEX_TYPE_2D)
			{
				if(numFaces <= 1)
				{
					glTexImage2D(GL_TEXTURE_2D, 0, mGLFormat, width, height, 0, depthStencilFormat, depthStencilType, nullptr);
					B3D_CHECK_GL_ERROR();
				}
				else
				{
					glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, mGLFormat, width, height, numFaces, 0, depthStencilFormat, depthStencilType, nullptr);
					B3D_CHECK_GL_ERROR();
				}
			}
			else if(texType == TEX_TYPE_CUBE_MAP)
			{
				if(numFaces <= 6)
				{
					for(u32 face = 0; face < 6; face++)
					{
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, mGLFormat, width, height, 0, depthStencilFormat, depthStencilType, nullptr);
						B3D_CHECK_GL_ERROR();
					}
				}
				else
				{
					glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, mGLFormat, width, height, numFaces, 0, depthStencilFormat, depthStencilType, nullptr);
					B3D_CHECK_GL_ERROR();
				}
			}
			else
			{
				B3D_LOG(Error, RenderBackend, "Unsupported texture type for depth-stencil attachment usage.");
			}
		}
		else
		{
			GLenum baseFormat = GLPixelUtil::getGLOriginFormat(mInternalFormat);
			GLenum baseDataType = GLPixelUtil::getGLOriginDataType(mInternalFormat);

			for(u32 mip = 0; mip < numMips; mip++)
			{
				switch(texType)
				{
				case TEX_TYPE_1D:
					{
						if(numFaces <= 1)
						{
							glTexImage1D(GL_TEXTURE_1D, mip, mGLFormat, width, 0, baseFormat, baseDataType, nullptr);
							B3D_CHECK_GL_ERROR();
						}
						else
						{
							glTexImage2D(GL_TEXTURE_1D_ARRAY, mip, mGLFormat, width, numFaces, 0, baseFormat, baseDataType, nullptr);
							B3D_CHECK_GL_ERROR();
						}
					}
					break;
				case TEX_TYPE_2D:
					{
						if(numFaces <= 1)
						{
							glTexImage2D(GL_TEXTURE_2D, mip, mGLFormat, width, height, 0, baseFormat, baseDataType, nullptr);
							B3D_CHECK_GL_ERROR();
						}
						else
						{
							glTexImage3D(GL_TEXTURE_2D_ARRAY, mip, mGLFormat, width, height, numFaces, 0, baseFormat, baseDataType, nullptr);
							B3D_CHECK_GL_ERROR();
						}
					}
					break;
				case TEX_TYPE_3D:
					glTexImage3D(GL_TEXTURE_3D, mip, mGLFormat, width, height, depth, 0, baseFormat, baseDataType, nullptr);
					B3D_CHECK_GL_ERROR();
					break;
				case TEX_TYPE_CUBE_MAP:
					{
						if(numFaces <= 6)
						{
							for(u32 face = 0; face < 6; face++)
							{
								glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, mGLFormat, width, height, 0, baseFormat, baseDataType, nullptr);
								B3D_CHECK_GL_ERROR();
							}
						}
						else
						{
							glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mip, mGLFormat, width, height, numFaces, 0, baseFormat, baseDataType, nullptr);
							B3D_CHECK_GL_ERROR();
						}
					}
					break;
				}

				if(width > 1)
					width = width / 2;

				if(height > 1)
					height = height / 2;

				if(depth > 1)
					depth = depth / 2;
			}
		}
#endif
	}

	CreateSurfaceList();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Texture);
	Texture::Initialize();
}

GLenum GLTexture::GetGlTextureTarget() const
{
	return GetGlTextureTarget(mProperties.GetTextureType(), mProperties.GetNumSamples(), mProperties.GetNumFaces());
}

GLuint GLTexture::GetGlid() const
{
	THROW_IF_NOT_CORE_THREAD;

	return mTextureID;
}

GLenum GLTexture::GetGlTextureTarget(TextureType type, u32 numSamples, u32 numFaces)
{
	switch(type)
	{
	case TEX_TYPE_1D:
		if(numFaces <= 1)
			return GL_TEXTURE_1D;
		else
			return GL_TEXTURE_1D_ARRAY;
	case TEX_TYPE_2D:
		if(numSamples > 1)
		{
			if(numFaces <= 1)
				return GL_TEXTURE_2D_MULTISAMPLE;
			else
				return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
		}
		else
		{
			if(numFaces <= 1)
				return GL_TEXTURE_2D;
			else
				return GL_TEXTURE_2D_ARRAY;
		}
	case TEX_TYPE_3D:
		return GL_TEXTURE_3D;
	case TEX_TYPE_CUBE_MAP:
		if(numFaces <= 6)
			return GL_TEXTURE_CUBE_MAP;
		else
			return GL_TEXTURE_CUBE_MAP_ARRAY;
	default:
		return 0;
	};
}

GLenum GLTexture::GetGlTextureTarget(GpuParamObjectType type)
{
	switch(type)
	{
	case GPOT_TEXTURE1D:
		return GL_TEXTURE_1D;
	case GPOT_TEXTURE2D:
		return GL_TEXTURE_2D;
	case GPOT_TEXTURE2DMS:
		return GL_TEXTURE_2D_MULTISAMPLE;
	case GPOT_TEXTURE3D:
		return GL_TEXTURE_3D;
	case GPOT_TEXTURECUBE:
		return GL_TEXTURE_CUBE_MAP;
	case GPOT_TEXTURE1DARRAY:
		return GL_TEXTURE_1D_ARRAY;
	case GPOT_TEXTURE2DARRAY:
		return GL_TEXTURE_2D_ARRAY;
	case GPOT_TEXTURE2DMSARRAY:
		return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
	case GPOT_TEXTURECUBEARRAY:
		return GL_TEXTURE_CUBE_MAP_ARRAY;
	default:
		return GL_TEXTURE_2D;
	}
}

PixelData GLTexture::LockImpl(GpuLockOptions options, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	if(mProperties.GetNumSamples() > 1)
		B3D_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

	if(mLockedBuffer != nullptr)
		B3D_EXCEPT(InternalErrorException, "Trying to lock a buffer that's already locked.");

	u32 mipWidth = std::max(1u, mProperties.GetWidth() >> mipLevel);
	u32 mipHeight = std::max(1u, mProperties.GetHeight() >> mipLevel);
	u32 mipDepth = std::max(1u, mProperties.GetDepth() >> mipLevel);

	PixelData lockedArea(mipWidth, mipHeight, mipDepth, mProperties.GetFormat());

	mLockedBuffer = GetBuffer(face, mipLevel);
	lockedArea.SetExternalBuffer((u8*)mLockedBuffer->Lock(options));

	return lockedArea;
}

void GLTexture::UnlockImpl()
{
	if(mLockedBuffer == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Trying to unlock a buffer that's not locked.");
		return;
	}

	mLockedBuffer->Unlock();
	mLockedBuffer = nullptr;
}

void GLTexture::ReadDataImpl(PixelData& dest, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	if(mProperties.GetNumSamples() > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return;
	}

	if(dest.GetFormat() != mInternalFormat)
	{
		PixelData temp(dest.GetExtents(), mInternalFormat);
		temp.AllocateInternalBuffer();

		GetBuffer(face, mipLevel)->Download(temp);
		PixelUtil::BulkPixelConversion(temp, dest);
	}
	else
		GetBuffer(face, mipLevel)->Download(dest);
}

void GLTexture::WriteDataImpl(const PixelData& src, u32 mipLevel, u32 face, bool discardWholeBuffer, u32 queueIdx)
{
	if(mProperties.GetNumSamples() > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return;
	}

	if(src.GetFormat() != mInternalFormat)
	{
		PixelData temp(src.GetExtents(), mInternalFormat);
		temp.AllocateInternalBuffer();

		PixelUtil::BulkPixelConversion(src, temp);
		GetBuffer(face, mipLevel)->Upload(temp, temp.GetExtents());
	}
	else
		GetBuffer(face, mipLevel)->Upload(src, src.GetExtents());
}

void GLTexture::CopyImpl(const SPtr<Texture>& target, const TextureCopyInformation& desc, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [this](const SPtr<Texture>& target, const TextureCopyInformation& desc)
	{
		GLTexture* destTex = static_cast<GLTexture*>(target.get());
		GLTextureBuffer* dest = static_cast<GLTextureBuffer*>(destTex->GetBuffer(desc.DestinationFace, desc.DestinationMip).get());
		GLTextureBuffer* src = static_cast<GLTextureBuffer*>(GetBuffer(desc.SourceFace, desc.SourceMip).get());

		bool copyEntireSurface = desc.SourceVolume.GetWidth() == 0 ||
			desc.SourceVolume.GetHeight() == 0 ||
			desc.SourceVolume.GetDepth() == 0;

		PixelVolume srcVolume = desc.SourceVolume;

		PixelVolume dstVolume;
		dstVolume.Left = (u32)desc.DestinationPosition.X;
		dstVolume.Top = (u32)desc.DestinationPosition.Y;
		dstVolume.Front = (u32)desc.DestinationPosition.Z;

		if(copyEntireSurface)
		{
			srcVolume.Right = srcVolume.Left + src->GetWidth();
			srcVolume.Bottom = srcVolume.Top + src->GetHeight();
			srcVolume.Back = srcVolume.Front + src->GetDepth();

			dstVolume.Right = dstVolume.Left + src->GetWidth();
			dstVolume.Bottom = dstVolume.Top + src->GetHeight();
			dstVolume.Back = dstVolume.Front + src->GetDepth();
		}
		else
		{
			dstVolume.Right = dstVolume.Left + desc.SourceVolume.GetWidth();
			dstVolume.Bottom = dstVolume.Top + desc.SourceVolume.GetHeight();
			dstVolume.Back = dstVolume.Front + desc.SourceVolume.GetDepth();
		}

		dest->BlitFromTexture(src, srcVolume, dstVolume);
	};

	if(commandBuffer == nullptr)
		executeRef(target, desc);
	else
	{
		auto execute = [=]()
		{ executeRef(target, desc); };

		SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
		cb->QueueCommand(execute);
	}
}

void GLTexture::CreateSurfaceList()
{
	mSurfaceList.clear();

	for(u32 face = 0; face < mProperties.GetNumFaces(); face++)
	{
		for(u32 mip = 0; mip <= mProperties.GetNumMipmaps(); mip++)
		{
			GLPixelBuffer* buf = B3DNew<GLTextureBuffer>(GetGlTextureTarget(), mTextureID, face, mip, mInternalFormat, static_cast<GpuBufferUsage>(mProperties.GetUsage()), mProperties.IsHardwareGammaEnabled(), mProperties.GetNumSamples());

			mSurfaceList.push_back(B3DMakeSharedFromExisting<GLPixelBuffer>(buf));
			if(buf->GetWidth() == 0 || buf->GetHeight() == 0 || buf->GetDepth() == 0)
			{
				B3D_EXCEPT(RenderingAPIException, "Zero sized texture surface on texture face " + ToString(face) + " mipmap " + ToString(mip) + ". Probably, the GL driver refused to create the texture.");
			}
		}
	}
}

SPtr<GLPixelBuffer> GLTexture::GetBuffer(u32 face, u32 mipmap)
{
	THROW_IF_NOT_CORE_THREAD;

	if(face >= mProperties.GetNumFaces())
		B3D_EXCEPT(InvalidParametersException, "Face index out of range");

	if(mipmap > mProperties.GetNumMipmaps())
		B3D_EXCEPT(InvalidParametersException, "Mipmap index out of range");

	unsigned int idx = face * (mProperties.GetNumMipmaps() + 1) + mipmap;
	B3D_ASSERT(idx < mSurfaceList.size());
	return mSurfaceList[idx];
}

SPtr<TextureView> GLTexture::CreateView(const TextureViewInformation& desc)
{
	return B3DMakeSharedFromExisting<GLTextureView>(new(B3DAllocate<GLTextureView>()) GLTextureView(this, desc));
}
