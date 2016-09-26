//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLMultiRenderTexture.h"
#include "BsGLTexture.h"

namespace BansheeEngine
{
	GLMultiRenderTextureCore::GLMultiRenderTextureCore(const MULTI_RENDER_TEXTURE_CORE_DESC& desc)
		:MultiRenderTextureCore(desc), mFB(nullptr), mProperties(desc)
	{ }

	GLMultiRenderTextureCore::~GLMultiRenderTextureCore()
	{ 
		if (mFB != nullptr)
			bs_delete(mFB);
	}

	void GLMultiRenderTextureCore::initialize()
	{
		MultiRenderTextureCore::initialize();

		if (mFB != nullptr)
			bs_delete(mFB);

		mFB = bs_new<GLFrameBufferObject>();

		for (size_t i = 0; i < mColorSurfaces.size(); i++)
		{
			if (mColorSurfaces[i] != nullptr)
			{
				GLTextureCore* glColorSurface = static_cast<GLTextureCore*>(mColorSurfaces[i]->getTexture().get());
				GLSurfaceDesc surfaceDesc;
				surfaceDesc.numSamples = getProperties().getMultisampleCount();

				if (mColorSurfaces[i]->getNumArraySlices() == 1) // Binding a single texture layer
				{
					surfaceDesc.allLayers = glColorSurface->getProperties().getNumFaces() == 1;

					if (glColorSurface->getProperties().getTextureType() != TEX_TYPE_3D)
					{
						surfaceDesc.zoffset = 0;
						surfaceDesc.buffer = glColorSurface->getBuffer(mColorSurfaces[i]->getFirstArraySlice(),
							mColorSurfaces[i]->getMostDetailedMip());
					}
					else
					{
						surfaceDesc.zoffset = mColorSurfaces[i]->getFirstArraySlice();
						surfaceDesc.buffer = glColorSurface->getBuffer(0, mColorSurfaces[i]->getMostDetailedMip());
					}
				}
				else // Binding an array of textures or a range of 3D texture slices
				{
					surfaceDesc.allLayers = true;

					if (glColorSurface->getProperties().getTextureType() != TEX_TYPE_3D)
					{
						if (mColorSurfaces[i]->getNumArraySlices() != glColorSurface->getProperties().getNumFaces())
							LOGWRN("OpenGL doesn't support binding of arbitrary ranges for array textures. The entire range will be bound instead.");

						surfaceDesc.zoffset = 0;
						surfaceDesc.buffer = glColorSurface->getBuffer(0, mColorSurfaces[i]->getMostDetailedMip());
					}
					else
					{
						if (mColorSurfaces[i]->getNumArraySlices() != glColorSurface->getProperties().getDepth())
							LOGWRN("OpenGL doesn't support binding of arbitrary ranges for array textures. The entire range will be bound instead.");

						surfaceDesc.zoffset = 0;
						surfaceDesc.buffer = glColorSurface->getBuffer(0, mColorSurfaces[i]->getMostDetailedMip());
					}

					
				}

				mFB->bindSurface((UINT32)i, surfaceDesc);
			}
			else
			{
				mFB->unbindSurface((UINT32)i);
			}
		}

		if (mDepthStencilSurface != nullptr)
		{
			GLTextureCore* glDepthStencilSurface = static_cast<GLTextureCore*>(mDepthStencilSurface->getTexture().get());
			SPtr<GLPixelBuffer> depthStencilBuffer = nullptr;

			if (glDepthStencilSurface->getProperties().getTextureType() != TEX_TYPE_3D)
			{
				depthStencilBuffer = glDepthStencilSurface->getBuffer(mDepthStencilSurface->getDesc().firstArraySlice,
					mDepthStencilSurface->getDesc().mostDetailMip);
			}

			mFB->bindDepthStencil(depthStencilBuffer);
		}
		else
		{
			mFB->unbindDepthStencil();
		}
	}

	void GLMultiRenderTextureCore::getCustomAttribute(const String& name, void* pData) const
	{
		if(name=="FBO")
		{
			*static_cast<GLFrameBufferObject **>(pData) = mFB;
		}
		else if (name == "GL_FBOID" || name == "GL_MULTISAMPLEFBOID")
		{
			*static_cast<GLuint*>(pData) = mFB->getGLFBOID();
		}
	}

	GLMultiRenderTexture::GLMultiRenderTexture(const MULTI_RENDER_TEXTURE_DESC& desc)
		:MultiRenderTexture(desc), mProperties(desc)
	{ }
}