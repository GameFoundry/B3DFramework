//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "BsGLPixelFormat.h"
#include "BsGLPixelBuffer.h"
#include "BsException.h"
#include "BsBitwise.h"
#include "BsCoreThread.h"
#include "BsTextureManager.h"
#include "BsGLRenderTexture.h"
#include "BsGLTextureView.h"
#include "BsRenderStats.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	GLTexture::GLTexture(GLSupport& support, const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, 
		GpuDeviceFlags deviceMask)
		: Texture(desc, initialData, deviceMask),
		mTextureID(0), mGLFormat(0), mInternalFormat(PF_UNKNOWN), mGLSupport(support)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	GLTexture::~GLTexture()
    { 
		mSurfaceList.clear();
		glDeleteTextures(1, &mTextureID);

		clearBufferViews();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Texture);
	}

	void GLTexture::initialize()
	{
		UINT32 width = mProperties.getWidth();
		UINT32 height = mProperties.getHeight();
		UINT32 depth = mProperties.getDepth();
		TextureType texType = mProperties.getTextureType();
		int usage = mProperties.getUsage();
		UINT32 numMips = mProperties.getNumMipmaps();
		UINT32 numFaces = mProperties.getNumFaces();

		PixelFormat pixFormat = mProperties.getFormat();
		mInternalFormat = GLPixelUtil::getClosestSupportedPF(pixFormat, texType, usage);

		if (pixFormat != mInternalFormat)
		{
			LOGWRN(StringUtil::format("Provided pixel format is not supported by the driver: {0}. Falling back on: {1}.",
									  pixFormat, mInternalFormat));
		}

		// Check requested number of mipmaps
		UINT32 maxMips = PixelUtil::getMaxMipmaps(width, height, depth, mProperties.getFormat());
		if (numMips > maxMips)
		{
			LOGERR("Invalid number of mipmaps. Maximum allowed is: " + toString(maxMips));
			numMips = maxMips;
		}

		if ((usage & TU_DEPTHSTENCIL) != 0)
		{
			if (texType != TEX_TYPE_2D && texType != TEX_TYPE_CUBE_MAP)
			{
				LOGERR("Only 2D and cubemap depth stencil textures are supported. Ignoring depth-stencil flag.");
				usage &= ~TU_DEPTHSTENCIL;
			}
		}

		// Include the base mip level
		numMips += 1;

		// Generate texture handle
		glGenTextures(1, &mTextureID);

		// Set texture type
		glBindTexture(getGLTextureTarget(), mTextureID);

		// This needs to be set otherwise the texture doesn't get rendered
		glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, numMips - 1);

		// Allocate internal buffer so that glTexSubImageXD can be used
		mGLFormat = GLPixelUtil::getGLInternalFormat(mInternalFormat, mProperties.isHardwareGammaEnabled());

		UINT32 sampleCount = mProperties.getNumSamples();
		if((usage & (TU_RENDERTARGET | TU_DEPTHSTENCIL)) != 0 && mProperties.getTextureType() == TEX_TYPE_2D && sampleCount > 1)
		{
			if (numFaces <= 1)
				glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, mGLFormat, width, height, GL_FALSE);
			else
				glTexStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, sampleCount, mGLFormat, width, height, numFaces, GL_FALSE);
		}
		else
		{
			switch (texType)
			{
			case TEX_TYPE_1D:
			{
				if (numFaces <= 1)
					glTexStorage1D(GL_TEXTURE_1D, numMips, mGLFormat, width);
				else
					glTexStorage2D(GL_TEXTURE_1D_ARRAY, numMips, mGLFormat, width, numFaces);
			}
				break;
			case TEX_TYPE_2D:
			{
				if (numFaces <= 1)
					glTexStorage2D(GL_TEXTURE_2D, numMips, mGLFormat, width, height);
				else
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, numMips, mGLFormat, width, height, numFaces);
			}
				break;
			case TEX_TYPE_3D:
				glTexStorage3D(GL_TEXTURE_3D, numMips, mGLFormat, width, height, depth);
				break;
			case TEX_TYPE_CUBE_MAP:
			{
				if (numFaces <= 6)
					glTexStorage2D(GL_TEXTURE_CUBE_MAP, numMips, mGLFormat, width, height);
				else
					glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, numMips, mGLFormat, width, height, numFaces);
			}
				break;
			}
		}

		createSurfaceList();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Texture);
		Texture::initialize();
	}

    GLenum GLTexture::getGLTextureTarget() const
    {
		switch (mProperties.getTextureType())
        {
            case TEX_TYPE_1D:
				if(mProperties.getNumFaces() <= 1)
					return GL_TEXTURE_1D;
				else
					return GL_TEXTURE_1D_ARRAY;
            case TEX_TYPE_2D:
				if (mProperties.getNumSamples() > 1)
				{
					if (mProperties.getNumFaces() <= 1)
						return GL_TEXTURE_2D_MULTISAMPLE;
					else
						return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
				}
				else
				{
					if (mProperties.getNumFaces() <= 1)
						return GL_TEXTURE_2D;
					else
						return GL_TEXTURE_2D_ARRAY;
				}
            case TEX_TYPE_3D:
                return GL_TEXTURE_3D;
            case TEX_TYPE_CUBE_MAP:
				if (mProperties.getNumFaces() <= 6)
					return GL_TEXTURE_CUBE_MAP;
				else
					return GL_TEXTURE_CUBE_MAP_ARRAY;
            default:
                return 0;
        };
    }

	GLuint GLTexture::getGLID() const
	{
		THROW_IF_NOT_CORE_THREAD;

		return mTextureID;
	}

	PixelData GLTexture::lockImpl(GpuLockOptions options, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx,
									  UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
			BS_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

		if(mLockedBuffer != nullptr)
			BS_EXCEPT(InternalErrorException, "Trying to lock a buffer that's already locked.");

		UINT32 mipWidth = std::max(1u, mProperties.getWidth() >> mipLevel);
		UINT32 mipHeight = std::max(1u, mProperties.getHeight() >> mipLevel);
		UINT32 mipDepth = std::max(1u, mProperties.getDepth() >> mipLevel);

		PixelData lockedArea(mipWidth, mipHeight, mipDepth, mProperties.getFormat());

		mLockedBuffer = getBuffer(face, mipLevel);
		lockedArea.setExternalBuffer((UINT8*)mLockedBuffer->lock(options));

		return lockedArea;
	}

	void GLTexture::unlockImpl()
	{
		if (mLockedBuffer == nullptr)
		{
			LOGERR("Trying to unlock a buffer that's not locked.");
			return;
		}

		mLockedBuffer->unlock();
		mLockedBuffer = nullptr;
	}

	void GLTexture::readDataImpl(PixelData& dest, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx, UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		if(dest.getFormat() != mInternalFormat)
		{
			PixelData temp(dest.getExtents(), mInternalFormat);
			temp.allocateInternalBuffer();

			getBuffer(face, mipLevel)->download(temp);
			PixelUtil::bulkPixelConversion(temp, dest);
		}
		else
			getBuffer(face, mipLevel)->download(dest);
	}

	void GLTexture::writeDataImpl(const PixelData& src, UINT32 mipLevel, UINT32 face, bool discardWholeBuffer,
								  UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		if (src.getFormat() != mInternalFormat)
		{
			PixelData temp(src.getExtents(), mInternalFormat);
			temp.allocateInternalBuffer();

			PixelUtil::bulkPixelConversion(src, temp);
			getBuffer(face, mipLevel)->upload(temp, temp.getExtents());
		}
		else
			getBuffer(face, mipLevel)->upload(src, src.getExtents());
	}

	void GLTexture::copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel,
								 const SPtr<Texture>& target, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [this](UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel, 
			const SPtr<Texture>& target)
		{
			GLTexture* destTex = static_cast<GLTexture*>(target.get());
			GLTextureBuffer *src = static_cast<GLTextureBuffer*>(getBuffer(srcFace, srcMipLevel).get());

			destTex->getBuffer(destFace, destMipLevel)->blitFromTexture(src);
		};

		if (commandBuffer == nullptr)
			executeRef(srcFace, srcMipLevel, destFace, destMipLevel, target);
		else
		{
			auto execute = [=]() { executeRef(srcFace, srcMipLevel, destFace, destMipLevel, target); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLTexture::createSurfaceList()
	{
		mSurfaceList.clear();
		
		for (UINT32 face = 0; face < mProperties.getNumFaces(); face++)
		{
			for (UINT32 mip = 0; mip <= mProperties.getNumMipmaps(); mip++)
			{
                GLPixelBuffer *buf = bs_new<GLTextureBuffer>(getGLTextureTarget(), mTextureID, face, mip,
					static_cast<GpuBufferUsage>(mProperties.getUsage()), mInternalFormat, mProperties.getNumSamples());

				mSurfaceList.push_back(bs_shared_ptr<GLPixelBuffer>(buf));
                if(buf->getWidth() == 0 || buf->getHeight() == 0 || buf->getDepth() == 0)
                {
					BS_EXCEPT(RenderingAPIException, 
                        "Zero sized texture surface on texture face "
						+ toString(face) 
						+ " mipmap "+toString(mip)
						+ ". Probably, the GL driver refused to create the texture.");
                }
			}
		}
	}
	
	SPtr<GLPixelBuffer> GLTexture::getBuffer(UINT32 face, UINT32 mipmap)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(face >= mProperties.getNumFaces())
			BS_EXCEPT(InvalidParametersException, "Face index out of range");

		if (mipmap > mProperties.getNumMipmaps())
			BS_EXCEPT(InvalidParametersException, "Mipmap index out of range");

		unsigned int idx = face * (mProperties.getNumMipmaps() + 1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}

	SPtr<TextureView> GLTexture::createView(const TEXTURE_VIEW_DESC& desc)
	{
		return bs_shared_ptr<GLTextureView>(new (bs_alloc<GLTextureView>()) GLTextureView(this, desc));
	}
}}
