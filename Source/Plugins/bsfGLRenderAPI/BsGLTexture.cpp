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

namespace bs { namespace ct
{
	GLTexture::GLTexture(GLSupport& support, const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData,
		GpuDeviceFlags deviceMask)
		: Texture(desc, initialData, deviceMask), mGLSupport(support)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	GLTexture::~GLTexture()
	{
		mSurfaceList.clear();
		glDeleteTextures(1, &mTextureID);
		BS_CHECK_GL_ERROR();

		ClearBufferViews();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Texture);
	}

	void GLTexture::Initialize()
	{
		UINT32 width = mProperties.GetWidth();
		UINT32 height = mProperties.GetHeight();
		UINT32 depth = mProperties.GetDepth();
		TextureType texType = mProperties.GetTextureType();
		int usage = mProperties.GetUsage();
		UINT32 numMips = mProperties.GetNumMipmaps();
		UINT32 numFaces = mProperties.GetNumFaces();

		// 0-sized textures aren't supported by the API
		width = std::max(width, 1U);
		height = std::max(height, 1U);

		PixelFormat pixFormat = mProperties.GetFormat();
		mInternalFormat = GLPixelUtil::GetClosestSupportedPf(pixFormat, texType, usage);

		if (pixFormat != mInternalFormat)
		{
			BS_LOG(Warning, RenderBackend, "Provided pixel format is not supported by the driver: {0}. "
				"Falling back on: {1}.", pixFormat, mInternalFormat);
		}

		// Check requested number of mipmaps
		UINT32 maxMips = PixelUtil::GetMaxMipmaps(width, height, depth, mProperties.GetFormat());
		if (numMips > maxMips)
		{
			BS_LOG(Error, RenderBackend, "Invalid number of mipmaps. Maximum allowed is: {0}", maxMips);
			numMips = maxMips;
		}

		if ((usage & TU_DEPTHSTENCIL) != 0)
		{
			if (texType != TEX_TYPE_2D && texType != TEX_TYPE_CUBE_MAP)
			{
				BS_LOG(Error, RenderBackend,
					"Only 2D and cubemap depth stencil textures are supported. Ignoring depth-stencil flag.");
				usage &= ~TU_DEPTHSTENCIL;
			}
		}

		// Include the base mip level
		numMips += 1;

		// Generate texture handle
		glGenTextures(1, &mTextureID);
		BS_CHECK_GL_ERROR();

		// Set texture type
		glBindTexture(GetGlTextureTarget(), mTextureID);
		BS_CHECK_GL_ERROR();

		if(mProperties.GetNumSamples() <= 1)
		{
			// This needs to be set otherwise the texture doesn't get rendered
			glTexParameteri(GetGlTextureTarget(), GL_TEXTURE_MAX_LEVEL, numMips - 1);
			BS_CHECK_GL_ERROR();
		}

		// Allocate internal buffer so that glTexSubImageXD can be used
		mGLFormat = GLPixelUtil::GetGlInternalFormat(mInternalFormat, mProperties.IsHardwareGammaEnabled());

		UINT32 sampleCount = mProperties.GetNumSamples();
		if((usage & (TU_RENDERTARGET | TU_DEPTHSTENCIL)) != 0 && mProperties.GetTextureType() == TEX_TYPE_2D && sampleCount > 1)
		{
			if (numFaces <= 1)
			{
				// Create immutable storage if available, fallback to mutable
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
				glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, mGLFormat, width, height, GL_TRUE);
				BS_CHECK_GL_ERROR();
#else
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, mGLFormat, width, height, GL_TRUE);
				BS_CHECK_GL_ERROR();
#endif
			}
			else
			{
				// Create immutable storage if available, fallback to mutable
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
				glTexStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, sampleCount, mGLFormat, width, height, numFaces, GL_TRUE);
				BS_CHECK_GL_ERROR();
#else
				glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, sampleCount, mGLFormat, width, height, numFaces, GL_TRUE);
				BS_CHECK_GL_ERROR();
#endif
			}
		}
		else
		{
			// Create immutable storage if available, fallback to mutable
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			switch (texType)
			{
			case TEX_TYPE_1D:
			{
				if (numFaces <= 1)
				{
					glTexStorage1D(GL_TEXTURE_1D, numMips, mGLFormat, width);
					BS_CHECK_GL_ERROR();
				}
				else
				{
					glTexStorage2D(GL_TEXTURE_1D_ARRAY, numMips, mGLFormat, width, numFaces);
					BS_CHECK_GL_ERROR();
				}
			}
				break;
			case TEX_TYPE_2D:
			{
				if (numFaces <= 1)
				{
					glTexStorage2D(GL_TEXTURE_2D, numMips, mGLFormat, width, height);
					BS_CHECK_GL_ERROR();
				}
				else
				{
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, numMips, mGLFormat, width, height, numFaces);
					BS_CHECK_GL_ERROR();
				}
			}
				break;
			case TEX_TYPE_3D:
				glTexStorage3D(GL_TEXTURE_3D, numMips, mGLFormat, width, height, depth);
				BS_CHECK_GL_ERROR();
				break;
			case TEX_TYPE_CUBE_MAP:
			{
				if (numFaces <= 6)
				{
					glTexStorage2D(GL_TEXTURE_CUBE_MAP, numMips, mGLFormat, width, height);
					BS_CHECK_GL_ERROR();
				}
				else
				{
					glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, numMips, mGLFormat, width, height, numFaces);
					BS_CHECK_GL_ERROR();
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
					if (numFaces <= 1)
					{
						glTexImage2D(GL_TEXTURE_2D, 0, mGLFormat, width, height, 0,
							depthStencilFormat, depthStencilType, nullptr);
						BS_CHECK_GL_ERROR();
					}
					else
					{
						glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, mGLFormat, width, height, numFaces, 0,
							depthStencilFormat, depthStencilType, nullptr);
						BS_CHECK_GL_ERROR();
					}
				}
				else if(texType == TEX_TYPE_CUBE_MAP)
				{
					if (numFaces <= 6)
					{
						for (UINT32 face = 0; face < 6; face++)
						{
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, mGLFormat,
								width, height, 0, depthStencilFormat, depthStencilType, nullptr);
							BS_CHECK_GL_ERROR();
						}
					}
					else
					{
						glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, mGLFormat,
							width, height, numFaces, 0, depthStencilFormat, depthStencilType, nullptr);
						BS_CHECK_GL_ERROR();
					}
				}
				else
				{
					BS_LOG(Error, RenderBackend, "Unsupported texture type for depth-stencil attachment usage.");
				}
			}
			else
			{
				GLenum baseFormat = GLPixelUtil::getGLOriginFormat(mInternalFormat);
				GLenum baseDataType = GLPixelUtil::getGLOriginDataType(mInternalFormat);

				for (UINT32 mip = 0; mip < numMips; mip++)
				{
					switch (texType)
					{
					case TEX_TYPE_1D:
					{
						if (numFaces <= 1)
						{
							glTexImage1D(GL_TEXTURE_1D, mip, mGLFormat, width, 0, baseFormat, baseDataType, nullptr);
							BS_CHECK_GL_ERROR();
						}
						else
						{
							glTexImage2D(GL_TEXTURE_1D_ARRAY, mip, mGLFormat, width, numFaces, 0, baseFormat, baseDataType, nullptr);
							BS_CHECK_GL_ERROR();
						}
					}
					break;
					case TEX_TYPE_2D:
					{
						if (numFaces <= 1)
						{
							glTexImage2D(GL_TEXTURE_2D, mip, mGLFormat, width, height, 0, baseFormat, baseDataType, nullptr);
							BS_CHECK_GL_ERROR();
						}
						else
						{
							glTexImage3D(GL_TEXTURE_2D_ARRAY, mip, mGLFormat, width, height, numFaces, 0, baseFormat, baseDataType, nullptr);
							BS_CHECK_GL_ERROR();
						}
					}
					break;
					case TEX_TYPE_3D:
						glTexImage3D(GL_TEXTURE_3D, mip, mGLFormat, width, height,
							depth, 0, baseFormat, baseDataType, nullptr);
						BS_CHECK_GL_ERROR();
						break;
					case TEX_TYPE_CUBE_MAP:
					{
						if (numFaces <= 6)
						{
							for (UINT32 face = 0; face < 6; face++)
							{
								glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, mGLFormat,
									width, height, 0, baseFormat, baseDataType, nullptr);
								BS_CHECK_GL_ERROR();
							}
						}
						else
						{
							glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mip, mGLFormat,
								width, height, numFaces, 0, baseFormat, baseDataType, nullptr);
							BS_CHECK_GL_ERROR();
						}
					}
					break;
					}

					if(width > 1)
						width = width/2;

					if(height > 1)
						height = height/2;

					if(depth > 1)	
						depth = depth/2;
				}
			}
#endif
		}

		CreateSurfaceList();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Texture);
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

	GLenum GLTexture::GetGlTextureTarget(TextureType type, UINT32 numSamples, UINT32 numFaces)
	{
		switch (type)
		{
		case TEX_TYPE_1D:
			if (numFaces <= 1)
				return GL_TEXTURE_1D;
			else
				return GL_TEXTURE_1D_ARRAY;
		case TEX_TYPE_2D:
			if (numSamples > 1)
			{
				if (numFaces <= 1)
					return GL_TEXTURE_2D_MULTISAMPLE;
				else
					return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
			}
			else
			{
				if (numFaces <= 1)
					return GL_TEXTURE_2D;
				else
					return GL_TEXTURE_2D_ARRAY;
			}
		case TEX_TYPE_3D:
			return GL_TEXTURE_3D;
		case TEX_TYPE_CUBE_MAP:
			if (numFaces <= 6)
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

	PixelData GLTexture::LockImpl(GpuLockOptions options, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx,
									  UINT32 queueIdx)
	{
		if (mProperties.GetNumSamples() > 1)
			BS_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

		if(mLockedBuffer != nullptr)
			BS_EXCEPT(InternalErrorException, "Trying to lock a buffer that's already locked.");

		UINT32 mipWidth = std::max(1u, mProperties.GetWidth() >> mipLevel);
		UINT32 mipHeight = std::max(1u, mProperties.GetHeight() >> mipLevel);
		UINT32 mipDepth = std::max(1u, mProperties.GetDepth() >> mipLevel);

		PixelData lockedArea(mipWidth, mipHeight, mipDepth, mProperties.GetFormat());

		mLockedBuffer = GetBuffer(face, mipLevel);
		lockedArea.SetExternalBuffer((UINT8*)mLockedBuffer->Lock(options));

		return lockedArea;
	}

	void GLTexture::UnlockImpl()
	{
		if (mLockedBuffer == nullptr)
		{
			BS_LOG(Error, RenderBackend, "Trying to unlock a buffer that's not locked.");
			return;
		}

		mLockedBuffer->Unlock();
		mLockedBuffer = nullptr;
	}

	void GLTexture::ReadDataImpl(PixelData& dest, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx, UINT32 queueIdx)
	{
		if (mProperties.GetNumSamples() > 1)
		{
			BS_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
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

	void GLTexture::WriteDataImpl(const PixelData& src, UINT32 mipLevel, UINT32 face, bool discardWholeBuffer,
								  UINT32 queueIdx)
	{
		if (mProperties.GetNumSamples() > 1)
		{
			BS_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		if (src.GetFormat() != mInternalFormat)
		{
			PixelData temp(src.GetExtents(), mInternalFormat);
			temp.AllocateInternalBuffer();

			PixelUtil::BulkPixelConversion(src, temp);
			GetBuffer(face, mipLevel)->Upload(temp, temp.GetExtents());
		}
		else
			GetBuffer(face, mipLevel)->Upload(src, src.GetExtents());
	}

	void GLTexture::CopyImpl(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [this](const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc)
		{
			GLTexture* destTex = static_cast<GLTexture*>(target.get());
			GLTextureBuffer* dest = static_cast<GLTextureBuffer*>(destTex->GetBuffer(desc.dstFace, desc.dstMip).get());
			GLTextureBuffer* src = static_cast<GLTextureBuffer*>(GetBuffer(desc.srcFace, desc.srcMip).get());

			bool copyEntireSurface = desc.srcVolume.GetWidth() == 0 ||
				desc.srcVolume.GetHeight() == 0 ||
				desc.srcVolume.GetDepth() == 0;

			PixelVolume srcVolume = desc.srcVolume;

			PixelVolume dstVolume;
			dstVolume.left = (UINT32)desc.dstPosition.x;
			dstVolume.top = (UINT32)desc.dstPosition.y;
			dstVolume.front = (UINT32)desc.dstPosition.z;

			if(copyEntireSurface)
			{
				srcVolume.right = srcVolume.left + src->GetWidth();
				srcVolume.bottom = srcVolume.top + src->GetHeight();
				srcVolume.back = srcVolume.front + src->GetDepth();

				dstVolume.right = dstVolume.left + src->GetWidth();
				dstVolume.bottom = dstVolume.top + src->GetHeight();
				dstVolume.back = dstVolume.front + src->GetDepth();
			}
			else
			{
				dstVolume.right = dstVolume.left + desc.srcVolume.GetWidth();
				dstVolume.bottom = dstVolume.top + desc.srcVolume.GetHeight();
				dstVolume.back = dstVolume.front + desc.srcVolume.GetDepth();
			}

			dest->BlitFromTexture(src, srcVolume, dstVolume);
		};

		if (commandBuffer == nullptr)
			executeRef(target, desc);
		else
		{
			auto execute = [=]() { executeRef(target, desc); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->QueueCommand(execute);
		}
	}

	void GLTexture::CreateSurfaceList()
	{
		mSurfaceList.clear();
		
		for (UINT32 face = 0; face < mProperties.GetNumFaces(); face++)
		{
			for (UINT32 mip = 0; mip <= mProperties.GetNumMipmaps(); mip++)
			{
				GLPixelBuffer *buf = bs_new<GLTextureBuffer>(GetGlTextureTarget(), mTextureID, face, mip, mInternalFormat,
					static_cast<GpuBufferUsage>(mProperties.GetUsage()),
					mProperties.IsHardwareGammaEnabled(),
					mProperties.GetNumSamples());

				mSurfaceList.push_back(bs_shared_ptr<GLPixelBuffer>(buf));
				if(buf->GetWidth() == 0 || buf->GetHeight() == 0 || buf->GetDepth() == 0)
				{
					BS_EXCEPT(RenderingAPIException,
						"Zero sized texture surface on texture face " + toString(face) + " mipmap " + toString(mip)
						+ ". Probably, the GL driver refused to create the texture.");
				}
			}
		}
	}
	
	SPtr<GLPixelBuffer> GLTexture::GetBuffer(UINT32 face, UINT32 mipmap)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(face >= mProperties.GetNumFaces())
			BS_EXCEPT(InvalidParametersException, "Face index out of range");

		if (mipmap > mProperties.GetNumMipmaps())
			BS_EXCEPT(InvalidParametersException, "Mipmap index out of range");

		unsigned int idx = face * (mProperties.GetNumMipmaps() + 1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}

	SPtr<TextureView> GLTexture::CreateView(const TEXTURE_VIEW_DESC& desc)
	{
		return bs_shared_ptr<GLTextureView>(new (bs_alloc<GLTextureView>()) GLTextureView(this, desc));
	}
}}
