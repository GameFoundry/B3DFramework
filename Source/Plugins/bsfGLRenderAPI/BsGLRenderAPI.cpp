//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLRenderAPI.h"
#include "RenderAPI/BsRenderAPI.h"
#include "BsGLTextureManager.h"
#include "BsGLIndexBuffer.h"
#include "BsGLUtil.h"
#include "GLSL/BsGLSLGpuProgram.h"
#include "Error/BsException.h"
#include "BsGLContext.h"
#include "BsGLSupport.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "BsGLRenderTexture.h"
#include "BsGLRenderWindowManager.h"
#include "GLSL/BsGLSLProgramPipelineManager.h"
#include "BsGLVertexArrayObjectManager.h"
#include "Managers/BsRenderStateManager.h"
#include "RenderAPI/BsGpuParams.h"
#include "BsGLGpuParamBlockBuffer.h"
#include "CoreThread/BsCoreThread.h"
#include "BsGLQueryManager.h"
#include "Debug/BsDebug.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "BsGLGpuBuffer.h"
#include "BsGLCommandBuffer.h"
#include "BsGLCommandBufferManager.h"
#include "BsGLTextureView.h"
#include "GLSL/BsGLSLParamParser.h"

namespace bs { namespace ct
{
	const char* MODULE_NAME = "BansheeGLRenderAPI.dll";

	const char* bs_get_gl_error_string(GLenum errorCode)
	{
		switch (errorCode) 
		{
			case GL_INVALID_OPERATION: return "INVALID_OPERATION";
			case GL_INVALID_ENUM: return "INVALID_ENUM";
			case GL_INVALID_VALUE: return "INVALID_VALUE";
			case GL_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
			case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION";
		}

		return nullptr;
	}

	void bs_check_gl_error(const char* function, const char* file, INT32 line)
	{
		GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			StringStream errorOutput;
			errorOutput << "OpenGL error in " << function << " [" << file << ":" << toString(line) << "]:\n";

			while (errorCode != GL_NO_ERROR)
			{
				const char* errorString = bs_get_gl_error_string(errorCode);
				if (errorString)
					errorOutput << "\t - " << errorString;

				errorCode = glGetError();
			}

			gDebug().logWarning(errorOutput.str());
		}
	}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
	void openGlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar
				*message, GLvoid *userParam);
#endif

	/************************************************************************/
	/* 								PUBLIC INTERFACE                   		*/
	/************************************************************************/

	GLRenderAPI::GLRenderAPI()
		: mViewportNorm(0.0f, 0.0f, 1.0f, 1.0f)
		, mScissorTop(0), mScissorBottom(720), mScissorLeft(0), mScissorRight(1280)
		, mViewportLeft(0), mViewportTop(0), mViewportWidth(0), mViewportHeight(0)
		, mScissorEnabled(false)
		, mStencilReadMask(0xFFFFFFFF)
		, mStencilWriteMask(0xFFFFFFFF)
		, mStencilRefValue(0)
		, mStencilCompareFront(CMPF_ALWAYS_PASS)
		, mStencilCompareBack(CMPF_ALWAYS_PASS)
		, mNumTextureUnits(0)
		, mTextureInfos(nullptr)
		, mDepthWrite(true)
		, mGLSLProgramFactory(nullptr)
		, mProgramPipelineManager(nullptr)
		, mActivePipeline(nullptr)
		, mCurrentDrawOperation(DOT_TRIANGLE_LIST)
		, mDrawCallInProgress(false)
		, mActiveTextureUnit(-1)
	{
		// Get our GLSupport
		mGLSupport = ct::getGLSupport();

		mColorWrite[0] = mColorWrite[1] = mColorWrite[2] = mColorWrite[3] = true;

		mCurrentContext = 0;
		mMainContext = 0;

		mGLInitialised = false;

		mMinFilter = FO_LINEAR;
		mMipFilter = FO_POINT;

		mProgramPipelineManager = bs_new<GLSLProgramPipelineManager>();
	}

	GLRenderAPI::~GLRenderAPI()
	{

	}

	const StringID& GLRenderAPI::getName() const
	{
		static StringID strName("GLRenderAPI");
		return strName;
	}

	void GLRenderAPI::initialize()
	{
		THROW_IF_NOT_CORE_THREAD;

		mGLSupport->start();
		mVideoModeInfo = mGLSupport->getVideoModeInfo();

		CommandBufferManager::startUp<GLCommandBufferManager>();
		bs::RenderWindowManager::startUp<bs::GLRenderWindowManager>(this);
		RenderWindowManager::startUp();

		RenderStateManager::startUp();

		QueryManager::startUp<GLQueryManager>();

		RenderAPI::initialize();
	}

	void GLRenderAPI::initializeWithWindow(const SPtr<RenderWindow>& primaryWindow)
	{
		// Get the context from the window and finish initialization
		SPtr<GLContext> context;
		primaryWindow->getCustomAttribute("GLCONTEXT", &context);

		// Set main and current context
		mMainContext = context;
		mCurrentContext = mMainContext;

		// Set primary context as active
		if (mCurrentContext)
			mCurrentContext->setCurrent(*primaryWindow);

		// Setup GLSupport
		mGLSupport->initializeExtensions();

		mNumDevices = 1;
		mCurrentCapabilities = bs_newN<RenderAPICapabilities>(mNumDevices);
		initCapabilities(mCurrentCapabilities[0]);		

		initFromCaps(mCurrentCapabilities);
		GLVertexArrayObjectManager::startUp();

		glFrontFace(GL_CW);
		BS_CHECK_GL_ERROR();

		// Ensure cubemaps are filtered across seams
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		BS_CHECK_GL_ERROR();

		GPUInfo gpuInfo;
		gpuInfo.numGPUs = 1;

		const char* vendor = (const char*)glGetString(GL_VENDOR);
		BS_CHECK_GL_ERROR();

		const char* renderer = (const char*)glGetString(GL_RENDERER);
		BS_CHECK_GL_ERROR();

		gpuInfo.names[0] = String(vendor) + " " + String(renderer);

		PlatformUtility::_setGPUInfo(gpuInfo);

		mGLInitialised = true;

		RenderAPI::initializeWithWindow(primaryWindow);
	}

	void GLRenderAPI::destroyCore()
	{
		RenderAPI::destroyCore();

		// Deleting the GLSL program factory
		if (mGLSLProgramFactory)
		{
			// Remove from manager safely
			GpuProgramManager::instance().removeFactory("glsl");
			GpuProgramManager::instance().removeFactory("glsl4_1");

			bs_delete(mGLSLProgramFactory);
			mGLSLProgramFactory = nullptr;
		}

		// Deleting the hardware buffer manager.  Has to be done before the mGLSupport->stop().
		HardwareBufferManager::shutDown();
		bs::HardwareBufferManager::shutDown();
		GLRTTManager::shutDown();

		for (UINT32 i = 0; i < MAX_VB_COUNT; i++)
			mBoundVertexBuffers[i] = nullptr;

		mBoundVertexDeclaration = nullptr;
		mBoundIndexBuffer = nullptr;

		mCurrentVertexProgram = nullptr;
		mCurrentFragmentProgram = nullptr;
		mCurrentGeometryProgram = nullptr;
		mCurrentHullProgram = nullptr;
		mCurrentDomainProgram = nullptr;
		mCurrentComputeProgram = nullptr;

		if (mGLSupport)
			mGLSupport->stop();

		TextureManager::shutDown();
		bs::TextureManager::shutDown();
		QueryManager::shutDown();
		RenderWindowManager::shutDown();
		bs::RenderWindowManager::shutDown();
		RenderStateManager::shutDown();
		GLVertexArrayObjectManager::shutDown(); // Note: Needs to be after QueryManager shutdown as some resources might be waiting for queries to complete
		CommandBufferManager::shutDown();

		mGLInitialised = false;

		if(mProgramPipelineManager != nullptr)
			bs_delete(mProgramPipelineManager);

		if(mCurrentContext)
			mCurrentContext->endCurrent();

		mCurrentContext = nullptr;
		mMainContext = nullptr;

		if(mGLSupport)
			bs_delete(mGLSupport);

		if (mTextureInfos != nullptr)
			bs_deleteN(mTextureInfos, mNumTextureUnits);
	}

	void GLRenderAPI::setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<GraphicsPipelineState>& pipelineState)
		{
			THROW_IF_NOT_CORE_THREAD;

			BlendState* blendState;
			RasterizerState* rasterizerState;
			DepthStencilState* depthStencilState;
			if (pipelineState != nullptr)
			{
				mCurrentVertexProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->getVertexProgram());
				mCurrentFragmentProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->getFragmentProgram());
				mCurrentGeometryProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->getGeometryProgram());
				mCurrentDomainProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->getDomainProgram());
				mCurrentHullProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->getHullProgram());

				blendState = pipelineState->getBlendState().get();
				rasterizerState = pipelineState->getRasterizerState().get();
				depthStencilState = pipelineState->getDepthStencilState().get();

				if (blendState == nullptr)
					blendState = BlendState::getDefault().get();

				if (rasterizerState == nullptr)
					rasterizerState = RasterizerState::getDefault().get();

				if(depthStencilState == nullptr)
					depthStencilState = DepthStencilState::getDefault().get();
			}
			else
			{
				mCurrentVertexProgram = nullptr;
				mCurrentFragmentProgram = nullptr;
				mCurrentGeometryProgram = nullptr;
				mCurrentDomainProgram = nullptr;
				mCurrentHullProgram = nullptr;

				blendState = BlendState::getDefault().get();
				rasterizerState = RasterizerState::getDefault().get();
				depthStencilState = DepthStencilState::getDefault().get();
			}

			// Blend state
			{
				const BlendProperties& stateProps = blendState->getProperties();

				// Alpha to coverage
				setAlphaToCoverage(stateProps.getAlphaToCoverageEnabled());

				// Blend states
				// OpenGL doesn't allow us to specify blend state per render target, so we just use the first one.
				if (stateProps.getBlendEnabled(0))
				{
					setSceneBlending(stateProps.getSrcBlend(0), stateProps.getDstBlend(0), stateProps.getAlphaSrcBlend(0),
						stateProps.getAlphaDstBlend(0), stateProps.getBlendOperation(0), stateProps.getAlphaBlendOperation(0));
				}
				else
				{
					setSceneBlending(BF_ONE, BF_ZERO, BO_ADD);
				}

				// Color write mask
				UINT8 writeMask = stateProps.getRenderTargetWriteMask(0);
				setColorBufferWriteEnabled((writeMask & 0x1) != 0, (writeMask & 0x2) != 0, (writeMask & 0x4) != 0, (writeMask & 0x8) != 0);
			}

			// Rasterizer state
			{
				const RasterizerProperties& stateProps = rasterizerState->getProperties();

				setDepthBias(stateProps.getDepthBias(), stateProps.getSlopeScaledDepthBias());
				setCullingMode(stateProps.getCullMode());
				setPolygonMode(stateProps.getPolygonMode());
				setScissorTestEnable(stateProps.getScissorEnable());
				setMultisamplingEnable(stateProps.getMultisampleEnable());
				setDepthClipEnable(stateProps.getDepthClipEnable());
				setAntialiasedLineEnable(stateProps.getAntialiasedLineEnable());
			}

			// Depth stencil state
			{
				const DepthStencilProperties& stateProps = depthStencilState->getProperties();

				// Set stencil buffer options
				setStencilCheckEnabled(stateProps.getStencilEnable());

				setStencilBufferOperations(stateProps.getStencilFrontFailOp(), stateProps.getStencilFrontZFailOp(), 
					stateProps.getStencilFrontPassOp(), true);
				setStencilBufferFunc(stateProps.getStencilFrontCompFunc(), stateProps.getStencilReadMask(), true);

				setStencilBufferOperations(stateProps.getStencilBackFailOp(), stateProps.getStencilBackZFailOp(), 
					stateProps.getStencilBackPassOp(), false);
				setStencilBufferFunc(stateProps.getStencilBackCompFunc(), stateProps.getStencilReadMask(), false);

				setStencilBufferWriteMask(stateProps.getStencilWriteMask());

				// Set depth buffer options
				setDepthBufferCheckEnabled(stateProps.getDepthReadEnable());
				setDepthBufferWriteEnabled(stateProps.getDepthWriteEnable());
				setDepthBufferFunction(stateProps.getDepthComparisonFunc());
			}
		};

		if (commandBuffer == nullptr)
			executeRef(pipelineState);
		else
		{
			auto execute = [=]() { executeRef(pipelineState); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void GLRenderAPI::setComputePipeline(const SPtr<ComputePipelineState>& pipelineState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<ComputePipelineState>& pipelineState)
		{
			THROW_IF_NOT_CORE_THREAD;

			SPtr<GpuProgram> program;
			if (pipelineState != nullptr)
				program = pipelineState->getProgram();

			if (program != nullptr && program->getType() == GPT_COMPUTE_PROGRAM)
				mCurrentComputeProgram = std::static_pointer_cast<GLSLGpuProgram>(program);
			else
				mCurrentComputeProgram = nullptr;
		};

		if (commandBuffer == nullptr)
			executeRef(pipelineState);
		else
		{
			auto execute = [=]() { executeRef(pipelineState); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void GLRenderAPI::setGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<GpuParams>& gpuParams)
		{
			THROW_IF_NOT_CORE_THREAD;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			for (UINT32 i = 0; i < 8; i++)
			{
				glBindImageTexture(i, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);
				BS_CHECK_GL_ERROR();
			}
#endif

			bs_frame_mark();
			{
				FrameVector<UINT32> textureUnits;
				textureUnits.reserve(12);

				auto getTexUnit = [&](UINT32 binding)
				{
					for(UINT32 i = 0; i < (UINT32)textureUnits.size(); i++)
					{
						if (textureUnits[i] == binding)
							return i;
					}

					UINT32 unit = (UINT32)textureUnits.size();
					textureUnits.push_back(binding);

					return unit;
				};

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
				UINT32 imageUnitCount = 0;
				auto getImageUnit = [&](UINT32 binding)
				{
					return imageUnitCount++;
				};
#endif

				UINT32 uniformUnitCount = 0;
				auto getUniformUnit = [&](UINT32 binding)
				{
					return uniformUnitCount++;
				};

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
				UINT32 sharedStorageUnitCount = 0;
				auto getSharedStorageUnit = [&](UINT32 binding)
				{
					return sharedStorageUnitCount++;
				};
#endif

				const UINT32 numStages = 6;
				for(UINT32 i = 0; i < numStages; i++)
				{
					for(auto& entry : textureUnits)
						entry = (UINT32)-1;

					GpuProgramType type = (GpuProgramType)i;

					SPtr<GpuParamDesc> paramDesc = gpuParams->getParamDesc(type);
					if (paramDesc == nullptr)
						continue;

					for (auto& entry : paramDesc->textures)
					{
						UINT32 binding = entry.second.slot;
						SPtr<Texture> texture = gpuParams->getTexture(entry.second.set, binding);
						const TextureSurface& surface = gpuParams->getTextureSurface(entry.second.set, binding);

						UINT32 unit = getTexUnit(binding);
						if (!activateGLTextureUnit(unit))
							continue;

						TextureInfo& texInfo = mTextureInfos[unit];

						GLTexture* glTex = static_cast<GLTexture*>(texture.get());
						GLenum newTextureType;
						GLuint texId;
						if (glTex != nullptr)
						{
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1 
							SPtr<TextureView> texView = glTex->requestView(
								surface.mipLevel,
								surface.numMipLevels,
								surface.face,
								surface.numFaces,
								GVU_DEFAULT);

							GLTextureView* glTexView = static_cast<GLTextureView*>(texView.get());

							newTextureType = glTexView->getGLTextureTarget();
							texId = glTexView->getGLID();
#else
							// Texture views are not supported, so if user requested a part of the texture surface report
							// a warning
							auto& props = texture->getProperties();
							if (surface.mipLevel != 0 || surface.face != 0 ||
								(surface.numMipLevels != 0 && surface.numMipLevels != props.getNumMipmaps()) ||
								(surface.numFaces != 0 && surface.numFaces != props.getNumFaces()))
							{
								LOGWRN("Attempting to bind only a part of a texture, but texture views are not supported. "
									"Entire texture will be bound instead.");
							}

							newTextureType = glTex->getGLTextureTarget();
							texId = glTex->getGLID();
#endif
						}
						else
						{

							newTextureType = GLTexture::getGLTextureTarget(entry.second.type);
							texId = 0;
						}

						if (texInfo.type != newTextureType)
						{
							glBindTexture(texInfo.type, 0);
							BS_CHECK_GL_ERROR();
						}

						glBindTexture(newTextureType, texId);
						BS_CHECK_GL_ERROR();

						texInfo.type = newTextureType;

						SPtr<GLSLGpuProgram> activeProgram = getActiveProgram(type);
						if (activeProgram != nullptr)
						{
							GLuint glProgram = activeProgram->getGLHandle();

							glProgramUniform1i(glProgram, binding, unit);
							BS_CHECK_GL_ERROR();
						}
					}

					for(auto& entry : paramDesc->samplers)
					{
						UINT32 binding = entry.second.slot;
						SPtr<SamplerState> samplerState = gpuParams->getSamplerState(entry.second.set, binding);

						if (samplerState == nullptr)
							samplerState = SamplerState::getDefault();

						UINT32 unit = getTexUnit(binding);
						if (!activateGLTextureUnit(unit))
							continue;

						// No sampler options for multisampled textures or buffers
						bool supportsSampler = mTextureInfos[unit].type != GL_TEXTURE_2D_MULTISAMPLE &&
							mTextureInfos[unit].type != GL_TEXTURE_2D_MULTISAMPLE_ARRAY &&
							mTextureInfos[unit].type != GL_TEXTURE_BUFFER;

						if (supportsSampler)
						{
							const SamplerProperties& stateProps = samplerState->getProperties();

							setTextureFiltering(unit, FT_MIN, stateProps.getTextureFiltering(FT_MIN));
							setTextureFiltering(unit, FT_MAG, stateProps.getTextureFiltering(FT_MAG));
							setTextureFiltering(unit, FT_MIP, stateProps.getTextureFiltering(FT_MIP));

							setTextureAnisotropy(unit, stateProps.getTextureAnisotropy());
							setTextureCompareMode(unit, stateProps.getComparisonFunction());

							setTextureMipmapBias(unit, stateProps.getTextureMipmapBias());
							setTextureMipmapRange(unit, stateProps.getMinimumMip(), stateProps.getMaximumMip());

							const UVWAddressingMode& uvw = stateProps.getTextureAddressingMode();
							setTextureAddressingMode(unit, uvw);

							setTextureBorderColor(unit, stateProps.getBorderColor());
						}
					}

					for(auto& entry : paramDesc->buffers)
					{
						UINT32 binding = entry.second.slot;
						SPtr<GpuBuffer> buffer = gpuParams->getBuffer(entry.second.set, binding);

						GLGpuBuffer* glBuffer = static_cast<GLGpuBuffer*>(buffer.get());

						switch(entry.second.type)
						{
						case GPOT_BYTE_BUFFER: // Texture buffer (read-only, unstructured)
							{
								UINT32 unit = getTexUnit(binding);
								if (!activateGLTextureUnit(unit))
									continue;

								GLuint texId = 0;
								if (glBuffer != nullptr)
									texId = glBuffer->getGLTextureId();

								if (mTextureInfos[unit].type != GL_TEXTURE_BUFFER)
								{
									glBindTexture(mTextureInfos[unit].type, 0);
									BS_CHECK_GL_ERROR();
								}

								mTextureInfos[unit].type = GL_TEXTURE_BUFFER;

								glBindTexture(GL_TEXTURE_BUFFER, texId);
								BS_CHECK_GL_ERROR();

								SPtr<GLSLGpuProgram> activeProgram = getActiveProgram(type);
								if (activeProgram != nullptr)
								{
									GLuint glProgram = activeProgram->getGLHandle();

									glProgramUniform1i(glProgram, binding, unit);
									BS_CHECK_GL_ERROR();
								}
							}
							break;
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
						case GPOT_RWBYTE_BUFFER: // Storage buffer (read/write, unstructured)
							{
								UINT32 unit = getImageUnit(binding);

								GLuint texId = 0;
								GLuint format = GL_R32F;
								if (glBuffer != nullptr)
								{
									texId = glBuffer->getGLTextureId();
									format = glBuffer->getGLFormat();
								}

								glBindImageTexture(unit, texId, 0, false, 0, GL_READ_WRITE, format);
								BS_CHECK_GL_ERROR();

								SPtr<GLSLGpuProgram> activeProgram = getActiveProgram(type);
								if (activeProgram != nullptr)
								{
									GLuint glProgram = activeProgram->getGLHandle();

									glProgramUniform1i(glProgram, binding, unit);
									BS_CHECK_GL_ERROR();
								}
							}
							break;
#endif
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
						case GPOT_RWSTRUCTURED_BUFFER: // Shared storage block (read/write, structured)
							{
								UINT32 unit = getSharedStorageUnit(binding);

								GLuint bufferId = 0;
								if (glBuffer != nullptr)
									bufferId = glBuffer->getGLBufferId();

								glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, bufferId);
								BS_CHECK_GL_ERROR();

								SPtr<GLSLGpuProgram> activeProgram = getActiveProgram(type);
								if (activeProgram != nullptr)
								{
									GLuint glProgram = activeProgram->getGLHandle();

									glShaderStorageBlockBinding(glProgram, binding, unit);
									BS_CHECK_GL_ERROR();
								}
							}
							break;
#endif
						default:
							break;
						}
					}

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
					for(auto& entry : paramDesc->loadStoreTextures)
					{
						UINT32 binding = entry.second.slot;

						SPtr<Texture> texture = gpuParams->getLoadStoreTexture(entry.second.set, binding);
						const TextureSurface& surface = gpuParams->getLoadStoreSurface(entry.second.set, binding);

						UINT32 unit = getImageUnit(binding);
						GLuint texId = 0;
						UINT32 mipLevel = 0;
						UINT32 face = 0;
						bool bindAllLayers = false;
						GLenum format = GL_R32F;

						if (texture != nullptr)
						{
							GLTexture* tex = static_cast<GLTexture*>(texture.get());
							auto& texProps = tex->getProperties();

							bindAllLayers = texProps.getNumFaces() == surface.numFaces || surface.numFaces == 0;

							if(!bindAllLayers && surface.numFaces > 1)
							{
								LOGWRN("Attempting to bind multiple faces of a load-store texture. You are allowed to bind \
									either a single face, or all the faces of the texture. Only the first face will \
									be bound instead.");
							}

							if(surface.numMipLevels > 1)
							{
								LOGWRN("Attempting to bind multiple mip levels of a load-store texture. This is not \
									supported and only the first provided level will be bound.");
							}

							texId = tex->getGLID();
							format = tex->getGLFormat();
							mipLevel = surface.mipLevel;
							face = surface.face;
						}

						glBindImageTexture(unit, texId, mipLevel, bindAllLayers, face, GL_READ_WRITE, format);
						BS_CHECK_GL_ERROR();

						SPtr<GLSLGpuProgram> activeProgram = getActiveProgram(type);
						if (activeProgram != nullptr)
						{
							GLuint glProgram = activeProgram->getGLHandle();

							glProgramUniform1i(glProgram, binding, unit);
							BS_CHECK_GL_ERROR();
						}
					}
#endif

					for (auto& entry : paramDesc->paramBlocks)
					{
						UINT32 binding = entry.second.slot;
						SPtr<GpuParamBlockBuffer> buffer = gpuParams->getParamBlockBuffer(entry.second.set, binding);
						
						if (buffer == nullptr)
							continue;

						buffer->flushToGPU();

						SPtr<GLSLGpuProgram> activeProgram = getActiveProgram(type);
						GLuint glProgram = activeProgram->getGLHandle();

						// 0 means uniforms are not in block, in which case we handle it specially
						if (binding == 0)
						{
							UINT8* uniformBufferData = (UINT8*)bs_stack_alloc(buffer->getSize());
							buffer->read(0, uniformBufferData, buffer->getSize());

							for (auto iter = paramDesc->params.begin(); iter != paramDesc->params.end(); ++iter)
							{
								const GpuParamDataDesc& param = iter->second;

								if (param.paramBlockSlot != 0) // 0 means uniforms are not in a block
									continue;

								const UINT8* ptrData = uniformBufferData + param.cpuMemOffset * sizeof(UINT32);

								// Note: We don't transpose matrices here even though we don't use column major format
								// because they are assumed to be pre-transposed in the GpuParams buffer
								switch (param.type)
								{
								case GPDT_FLOAT1:
									glProgramUniform1fv(glProgram, param.gpuMemOffset, param.arraySize, (GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_FLOAT2:
									glProgramUniform2fv(glProgram, param.gpuMemOffset, param.arraySize, (GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_FLOAT3:
									glProgramUniform3fv(glProgram, param.gpuMemOffset, param.arraySize, (GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_FLOAT4:
									glProgramUniform4fv(glProgram, param.gpuMemOffset, param.arraySize, (GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_2X2:
									glProgramUniformMatrix2fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_2X3:
									glProgramUniformMatrix3x2fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_2X4:
									glProgramUniformMatrix4x2fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_3X2:
									glProgramUniformMatrix2x3fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_3X3:
									glProgramUniformMatrix3fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_3X4:
									glProgramUniformMatrix4x3fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_4X2:
									glProgramUniformMatrix2x4fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_4X3:
									glProgramUniformMatrix3x4fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_MATRIX_4X4:
									glProgramUniformMatrix4fv(
										glProgram,
										param.gpuMemOffset,
										param.arraySize,
										GL_FALSE,
										(GLfloat*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_INT1:
									glProgramUniform1iv(glProgram, param.gpuMemOffset, param.arraySize, (GLint*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_INT2:
									glProgramUniform2iv(glProgram, param.gpuMemOffset, param.arraySize, (GLint*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_INT3:
									glProgramUniform3iv(glProgram, param.gpuMemOffset, param.arraySize, (GLint*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_INT4:
									glProgramUniform4iv(glProgram, param.gpuMemOffset, param.arraySize, (GLint*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								case GPDT_BOOL:
									glProgramUniform1uiv(glProgram, param.gpuMemOffset, param.arraySize, (GLuint*)ptrData);
									BS_CHECK_GL_ERROR();
									break;
								default:
								case GPDT_UNKNOWN:
									break;
								}
							}

							if (uniformBufferData != nullptr)
								bs_stack_free(uniformBufferData);
						}
						else
						{
							const GLGpuParamBlockBuffer* glParamBlockBuffer = static_cast<const GLGpuParamBlockBuffer*>(buffer.get());

							UINT32 unit = getUniformUnit(binding - 1);
							glUniformBlockBinding(glProgram, binding - 1, unit);
							BS_CHECK_GL_ERROR();

							glBindBufferBase(GL_UNIFORM_BUFFER, unit, glParamBlockBuffer->getGLBufferId());
							BS_CHECK_GL_ERROR();
						}
					}
				}
			}
			bs_frame_clear();

			activateGLTextureUnit(0);
		};

		if (commandBuffer == nullptr)
			executeRef(gpuParams);
		else
		{
			auto execute = [=]() { executeRef(gpuParams); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumGpuParamBinds);
	}

	void GLRenderAPI::setStencilRef(UINT32 stencilRefValue, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 stencilRefValue)
		{
			THROW_IF_NOT_CORE_THREAD;

			setStencilRefValue(stencilRefValue);
		};

		if (commandBuffer == nullptr)
			executeRef(stencilRefValue);
		else
		{
			auto execute = [=]() { executeRef(stencilRefValue); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::setViewport(const Rect2& area,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const Rect2& area)
		{
			THROW_IF_NOT_CORE_THREAD;

			mViewportNorm = area;
			applyViewport();
		};

		if (commandBuffer == nullptr)
			executeRef(area);
		else
		{
			auto execute = [=]() { executeRef(area); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::setRenderTarget(const SPtr<RenderTarget>& target, UINT32 readOnlyFlags, 
		RenderSurfaceMask loadMask, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<RenderTarget>& target, UINT32 readOnlyFlags)
		{
			THROW_IF_NOT_CORE_THREAD;

			// Switch context if different from current one
			if (target != nullptr && target->getProperties().isWindow)
			{
				RenderWindow* window = static_cast<RenderWindow*>(target.get());

				SPtr<GLContext> newContext;
				target->getCustomAttribute("GLCONTEXT", &newContext);
				if (newContext && mCurrentContext != newContext)
					switchContext(newContext, *window);
				else
					mCurrentContext->setCurrent(*window);
			}

			// This must happen after context switch to ensure previous context is still alive
			mActiveRenderTarget = target;

			GLFrameBufferObject* fbo = nullptr;

			if (target != nullptr)
				target->getCustomAttribute("FBO", &fbo);

			if (fbo != nullptr)
			{
				fbo->bind();

				// Enable / disable sRGB states
				if (target->getProperties().hwGamma)
				{
					glEnable(GL_FRAMEBUFFER_SRGB);
					BS_CHECK_GL_ERROR();
				}
				else
				{
					glDisable(GL_FRAMEBUFFER_SRGB);
					BS_CHECK_GL_ERROR();
				}
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				BS_CHECK_GL_ERROR();
			}

			applyViewport();
		};

		if (commandBuffer == nullptr)
			executeRef(target, readOnlyFlags);
		else
		{
			auto execute = [=]() { executeRef(target, readOnlyFlags); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumRenderTargetChanges);
	}

	void GLRenderAPI::setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
#if BS_DEBUG_MODE
		UINT32 lastIdx = index + numBuffers;
		if(lastIdx > MAX_VB_COUNT)
		{
			LOGERR("Provided vertex buffer slot range is invalid: " + toString(index) + " to " + 
				toString(index + numBuffers) + ".");
			return;
		}
#endif

		auto executeRef = [&](UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers)
		{
			THROW_IF_NOT_CORE_THREAD;

			std::array<SPtr<VertexBuffer>, MAX_VB_COUNT> boundBuffers;
			for (UINT32 i = 0; i < numBuffers; i++)
				boundBuffers[index + i] = buffers[i];

			for (UINT32 i = 0; i < numBuffers; i++)
				mBoundVertexBuffers[index + i] = boundBuffers[index + i];
		};

		if (commandBuffer == nullptr)
			executeRef(index, buffers, numBuffers);
		else
		{
			auto execute = [=]() { executeRef(index, buffers, numBuffers); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<VertexDeclaration>& vertexDeclaration)
		{
			THROW_IF_NOT_CORE_THREAD;

			mBoundVertexDeclaration = vertexDeclaration;
		};

		if (commandBuffer == nullptr)
			executeRef(vertexDeclaration);
		else
		{
			auto execute = [=]() { executeRef(vertexDeclaration); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::setDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](DrawOperationType op)
		{
			THROW_IF_NOT_CORE_THREAD;

			mCurrentDrawOperation = op;
		};

		if (commandBuffer == nullptr)
			executeRef(op);
		else
		{
			auto execute = [=]() { executeRef(op); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);

			cb->mCurrentDrawOperation = op;
		}
	}

	void GLRenderAPI::setIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<IndexBuffer>& buffer)
		{
			THROW_IF_NOT_CORE_THREAD;

			mBoundIndexBuffer = buffer;
		};

		if (commandBuffer == nullptr)
			executeRef(buffer);
		else
		{
			auto execute = [=]() { executeRef(buffer); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
		{
			THROW_IF_NOT_CORE_THREAD;

			// Find the correct type to render
			GLint primType = getGLDrawMode();
			beginDraw();

			if (instanceCount <= 1)
			{
				glDrawArrays(primType, vertexOffset, vertexCount);
				BS_CHECK_GL_ERROR();
			}
			else
			{
				glDrawArraysInstanced(primType, vertexOffset, vertexCount, instanceCount);
				BS_CHECK_GL_ERROR();
			}

			endDraw();
		};

		UINT32 primCount;
		if (commandBuffer == nullptr)
		{
			executeRef(vertexOffset, vertexCount, instanceCount);

			primCount = vertexCountToPrimCount(mCurrentDrawOperation, vertexCount);
		}
		else
		{
			auto execute = [=]() { executeRef(vertexOffset, vertexCount, instanceCount); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);

			primCount = vertexCountToPrimCount(cb->mCurrentDrawOperation, vertexCount);
		}

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void GLRenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount,
		UINT32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount,
			UINT32 instanceCount)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (mBoundIndexBuffer == nullptr)
			{
				LOGWRN("Cannot draw indexed because index buffer is not set.");
				return;
			}

			// Find the correct type to render
			GLint primType = getGLDrawMode();

			beginDraw();

			SPtr<GLIndexBuffer> indexBuffer = std::static_pointer_cast<GLIndexBuffer>(mBoundIndexBuffer);
			const IndexBufferProperties& ibProps = indexBuffer->getProperties();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->getGLBufferId());
			BS_CHECK_GL_ERROR();

			GLenum indexType = (ibProps.getType() == IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

			if (instanceCount <= 1)
			{
				glDrawElementsBaseVertex(
					primType,
					indexCount,
					indexType,
					(GLvoid*)(UINT64)(ibProps.getIndexSize() * startIndex),
					vertexOffset);
				BS_CHECK_GL_ERROR();
			}
			else
			{
				glDrawElementsInstancedBaseVertex(
					primType,
					indexCount,
					indexType,
					(GLvoid*)(UINT64)(ibProps.getIndexSize() * startIndex),
					instanceCount,
					vertexOffset);
				BS_CHECK_GL_ERROR();
			}

			endDraw();
		};

		UINT32 primCount;
		if (commandBuffer == nullptr)
		{
			executeRef(startIndex, indexCount, vertexOffset, vertexCount, instanceCount);

			primCount = vertexCountToPrimCount(mCurrentDrawOperation, vertexCount);
		}
		else
		{
			auto execute = [=]() { executeRef(startIndex, indexCount, vertexOffset, vertexCount, instanceCount); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);

			primCount = vertexCountToPrimCount(cb->mCurrentDrawOperation, vertexCount);
		}

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);

		BS_INC_RENDER_STAT(NumIndexBufferBinds);
	}

	void GLRenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (mCurrentComputeProgram == nullptr)
			{
				LOGWRN("Cannot dispatch compute without a set compute program.");
				return;
			}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			glUseProgram(mCurrentComputeProgram->getGLHandle());
			BS_CHECK_GL_ERROR();

			glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
			BS_CHECK_GL_ERROR();

			glMemoryBarrier(GL_ALL_BARRIER_BITS);
#else
			LOGWRN("Compute shaders not supported on current OpenGL version.");
#endif
		};

		if (commandBuffer == nullptr)
			executeRef(numGroupsX, numGroupsY, numGroupsZ);
		else
		{
			auto execute = [=]() { executeRef(numGroupsX, numGroupsY, numGroupsZ); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumComputeCalls);
	}

	void GLRenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
		{
			THROW_IF_NOT_CORE_THREAD;

			mScissorTop = top;
			mScissorBottom = bottom;
			mScissorLeft = left;
			mScissorRight = right;
		};

		if (commandBuffer == nullptr)
			executeRef(left, top, right, bottom);
		else
		{
			auto execute = [=]() { executeRef(left, top, right, bottom); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
		{
			if (mActiveRenderTarget == nullptr)
				return;

			const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();
			Rect2I clearRect(0, 0, rtProps.width, rtProps.height);

			clearArea(buffers, color, depth, stencil, clearRect, targetMask);
		};

		if (commandBuffer == nullptr)
			executeRef(buffers, color, depth, stencil, targetMask);
		else
		{
			auto execute = [=]() { executeRef(buffers, color, depth, stencil, targetMask); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
		{
			Rect2I clearRect(mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight);

			clearArea(buffers, color, depth, stencil, clearRect, targetMask);
		};

		if (commandBuffer == nullptr)
			executeRef(buffers, color, depth, stencil, targetMask);
		else
		{
			auto execute = [=]() { executeRef(buffers, color, depth, stencil, targetMask); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void GLRenderAPI::swapBuffers(const SPtr<RenderTarget>& target, UINT32 syncMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		// Switch context if different from current one
		if(!target->getProperties().isWindow)
			return;

		RenderWindow* window = static_cast<RenderWindow*>(target.get());

		SPtr<GLContext> newContext;
		target->getCustomAttribute("GLCONTEXT", &newContext);
		if (newContext && mCurrentContext != newContext)
			switchContext(newContext, *window);
		else
			mCurrentContext->setCurrent(*window);

		target->swapBuffers();
	
		BS_INC_RENDER_STAT(NumPresents);
	}

	void GLRenderAPI::addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
	{
		SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
		SPtr<GLCommandBuffer> secondaryCb = std::static_pointer_cast<GLCommandBuffer>(secondary);

		cb->appendSecondary(secondaryCb);
	}

	void GLRenderAPI::submitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, UINT32 syncMask)
	{
		SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
		if (cb == nullptr)
			return;

		cb->executeCommands();
		cb->clear();
	}

	void GLRenderAPI::clearArea(UINT32 buffers, const Color& color, float depth, UINT16 stencil, const Rect2I& clearRect, 
		UINT8 targetMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(mActiveRenderTarget == nullptr)
			return;

		bool colorMask = !mColorWrite[0] || !mColorWrite[1] || !mColorWrite[2] || !mColorWrite[3]; 

		// Disable scissor test as we want to clear the entire render surface
		GLboolean scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
		UINT32 oldScissorTop = mScissorTop;
		UINT32 oldScissorBottom = mScissorBottom;
		UINT32 oldScissorLeft = mScissorLeft;
		UINT32 oldScissorRight = mScissorRight;

		if (scissorTestEnabled)
		{
			glDisable(GL_SCISSOR_TEST);
			BS_CHECK_GL_ERROR();
		}

		const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();

		bool clearEntireTarget = clearRect.width == 0 || clearRect.height == 0;
		clearEntireTarget |= (clearRect.x == 0 && clearRect.y == 0 && clearRect.width == rtProps.width && clearRect.height == rtProps.height);

		if (!clearEntireTarget)
		{
			setScissorRect(clearRect.x, clearRect.y, clearRect.x + clearRect.width, clearRect.y + clearRect.height);
			setScissorTestEnable(true);
		}

		if (buffers & FBT_COLOR)
		{
			// Enable buffer for writing if it isn't
			if (colorMask)
			{
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				BS_CHECK_GL_ERROR();
			}
		}
		if (buffers & FBT_DEPTH)
		{
			// Enable buffer for writing if it isn't
			if (!mDepthWrite)
			{
				glDepthMask(GL_TRUE);
				BS_CHECK_GL_ERROR();
			}
		}
		if (buffers & FBT_STENCIL)
		{
			// Enable buffer for writing if it isn't
			glStencilMask(0xFFFFFFFF);
			BS_CHECK_GL_ERROR();
		}

		if (targetMask == 0xFF)
		{
			GLbitfield flags = 0;
			if (buffers & FBT_COLOR)
			{
				flags |= GL_COLOR_BUFFER_BIT;

				glClearColor(color.r, color.g, color.b, color.a);
				BS_CHECK_GL_ERROR();
			}

			if (buffers & FBT_DEPTH)
			{
				flags |= GL_DEPTH_BUFFER_BIT;

				glClearDepth(depth);
				BS_CHECK_GL_ERROR();
			}

			if (buffers & FBT_STENCIL)
			{
				flags |= GL_STENCIL_BUFFER_BIT;

				glClearStencil(stencil);
				BS_CHECK_GL_ERROR();
			}

			// Clear buffers
			glClear(flags);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			GLFrameBufferObject* fbo = nullptr;
			mActiveRenderTarget->getCustomAttribute("FBO", &fbo);

			if (buffers & FBT_COLOR)
			{
				for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
				{
					if (fbo->hasColorBuffer(i) && ((1 << i) & targetMask) != 0)
					{
						glClearBufferfv(GL_COLOR, i, (GLfloat*)&color);
						BS_CHECK_GL_ERROR();
					}
				}
			}

			if (buffers & FBT_DEPTH)
			{
				if (buffers & FBT_STENCIL)
				{
					glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
					BS_CHECK_GL_ERROR();
				}
				else
				{
					glClearBufferfv(GL_DEPTH, 0, &depth);
					BS_CHECK_GL_ERROR();
				}
			}
			else if (buffers & FBT_STENCIL)
			{
				INT32 stencilClear = (INT32)stencil;
				glClearBufferiv(GL_STENCIL, 0, &stencilClear);
				BS_CHECK_GL_ERROR();
			}
		}

		if (!clearEntireTarget)
		{
			setScissorTestEnable(false);
		}

		// Restore scissor test
		if (scissorTestEnabled)
		{
			glEnable(GL_SCISSOR_TEST);
			BS_CHECK_GL_ERROR();

			mScissorTop = oldScissorTop;
			mScissorBottom = oldScissorBottom;
			mScissorLeft = oldScissorLeft;
			mScissorRight = oldScissorRight;
		}

		// Reset buffer write state
		if (!mDepthWrite && (buffers & FBT_DEPTH))
		{
			glDepthMask(GL_FALSE);
			BS_CHECK_GL_ERROR();
		}

		if (colorMask && (buffers & FBT_COLOR))
		{
			glColorMask(mColorWrite[0], mColorWrite[1], mColorWrite[2], mColorWrite[3]);
			BS_CHECK_GL_ERROR();
		}

		if (buffers & FBT_STENCIL)
		{
			glStencilMask(mStencilWriteMask);
			BS_CHECK_GL_ERROR();
		}

		BS_INC_RENDER_STAT(NumClears);
	}

	/************************************************************************/
	/* 								PRIVATE		                     		*/
	/************************************************************************/

	void GLRenderAPI::setTextureAddressingMode(UINT16 unit, const UVWAddressingMode& uvw)
	{
		glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_WRAP_S, getTextureAddressingMode(uvw.u));
		BS_CHECK_GL_ERROR();

		glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_WRAP_T, getTextureAddressingMode(uvw.v));
		BS_CHECK_GL_ERROR();

		glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_WRAP_R, getTextureAddressingMode(uvw.w));
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setTextureBorderColor(UINT16 unit, const Color& color)
	{
		GLfloat border[4] = { color.r, color.g, color.b, color.a };
		glTexParameterfv(mTextureInfos[unit].type, GL_TEXTURE_BORDER_COLOR, border);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setTextureMipmapBias(UINT16 unit, float bias)
	{
		glTexParameterf(mTextureInfos[unit].type, GL_TEXTURE_LOD_BIAS, bias);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setTextureMipmapRange(UINT16 unit, float min, float max)
	{
		glTexParameterf(mTextureInfos[unit].type, GL_TEXTURE_MIN_LOD, min);
		BS_CHECK_GL_ERROR();

		glTexParameterf(mTextureInfos[unit].type, GL_TEXTURE_MAX_LOD, max);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setSceneBlending(BlendFactor sourceFactor, BlendFactor destFactor, BlendOperation op)
	{
		GLint sourceBlend = getBlendMode(sourceFactor);
		GLint destBlend = getBlendMode(destFactor);
		if(sourceFactor == BF_ONE && destFactor == BF_ZERO)
		{
			glDisable(GL_BLEND);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glEnable(GL_BLEND);
			BS_CHECK_GL_ERROR();

			glBlendFunc(sourceBlend, destBlend);
			BS_CHECK_GL_ERROR();
		}

		GLint func = GL_FUNC_ADD;
		switch(op)
		{
		case BO_ADD:
			func = GL_FUNC_ADD;
			break;
		case BO_SUBTRACT:
			func = GL_FUNC_SUBTRACT;
			break;
		case BO_REVERSE_SUBTRACT:
			func = GL_FUNC_REVERSE_SUBTRACT;
			break;
		case BO_MIN:
			func = GL_MIN;
			break;
		case BO_MAX:
			func = GL_MAX;
			break;
		}

		glBlendEquation(func);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setSceneBlending(BlendFactor sourceFactor, BlendFactor destFactor, 
		BlendFactor sourceFactorAlpha, BlendFactor destFactorAlpha, BlendOperation op, BlendOperation alphaOp)
	{
		GLint sourceBlend = getBlendMode(sourceFactor);
		GLint destBlend = getBlendMode(destFactor);
		GLint sourceBlendAlpha = getBlendMode(sourceFactorAlpha);
		GLint destBlendAlpha = getBlendMode(destFactorAlpha);

		if(sourceFactor == BF_ONE && destFactor == BF_ZERO && sourceFactorAlpha == BF_ONE && destFactorAlpha == BF_ZERO)
		{
			glDisable(GL_BLEND);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glEnable(GL_BLEND);
			BS_CHECK_GL_ERROR();

			glBlendFuncSeparate(sourceBlend, destBlend, sourceBlendAlpha, destBlendAlpha);
			BS_CHECK_GL_ERROR();
		}

		GLint func = GL_FUNC_ADD, alphaFunc = GL_FUNC_ADD;

		switch(op)
		{
		case BO_ADD:
			func = GL_FUNC_ADD;
			break;
		case BO_SUBTRACT:
			func = GL_FUNC_SUBTRACT;
			break;
		case BO_REVERSE_SUBTRACT:
			func = GL_FUNC_REVERSE_SUBTRACT;
			break;
		case BO_MIN:
			func = GL_MIN;
			break;
		case BO_MAX:
			func = GL_MAX;
			break;
		}

		switch(alphaOp)
		{
		case BO_ADD:
			alphaFunc = GL_FUNC_ADD;
			break;
		case BO_SUBTRACT:
			alphaFunc = GL_FUNC_SUBTRACT;
			break;
		case BO_REVERSE_SUBTRACT:
			alphaFunc = GL_FUNC_REVERSE_SUBTRACT;
			break;
		case BO_MIN:
			alphaFunc = GL_MIN;
			break;
		case BO_MAX:
			alphaFunc = GL_MAX;
			break;
		}

		glBlendEquationSeparate(func, alphaFunc);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setAlphaToCoverage(bool enable)
	{
		static bool lasta2c = false;

		if (enable != lasta2c)
		{
			if (enable)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				BS_CHECK_GL_ERROR();
			}
			else
			{
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				BS_CHECK_GL_ERROR();
			}

			lasta2c = enable;
		}
	}

	void GLRenderAPI::setScissorTestEnable(bool enable)
	{
		if (mActiveRenderTarget == nullptr)
			return;

		const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();
		GLsizei x, y, w, h;
		if (enable)
		{
			glEnable(GL_SCISSOR_TEST);
			BS_CHECK_GL_ERROR();

			x = mScissorLeft;
			y = rtProps.height - mScissorBottom;
			w = mScissorRight - mScissorLeft;
			h = mScissorBottom - mScissorTop;

			glScissor(x, y, w, h);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
			BS_CHECK_GL_ERROR();

			// GL requires you to reset the scissor when disabling
			x = mViewportLeft;
			y = rtProps.height - (mViewportTop + mViewportHeight); 
			w = mViewportWidth;
			h = mViewportHeight;

			glScissor(x, y, w, h);
			BS_CHECK_GL_ERROR();
		}

		mScissorEnabled = enable;
	}

	void GLRenderAPI::setMultisamplingEnable(bool enable)
	{
		if (enable)
		{
			glEnable(GL_MULTISAMPLE);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
			BS_CHECK_GL_ERROR();
		}
	}

	void GLRenderAPI::setDepthClipEnable(bool enable)
	{
		if (!enable) // If clipping disabled, clamp is enabled
		{
			glEnable(GL_DEPTH_CLAMP);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_DEPTH_CLAMP);
			BS_CHECK_GL_ERROR();
		}
	}

	void GLRenderAPI::setAntialiasedLineEnable(bool enable)
	{
		if (enable)
		{
			glEnable(GL_LINE_SMOOTH);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_LINE_SMOOTH);
			BS_CHECK_GL_ERROR();
		}
	}


	void GLRenderAPI::setCullingMode(CullingMode mode)
	{
		GLenum cullMode;

		switch( mode )
		{
		case CULL_NONE:
			glDisable(GL_CULL_FACE);
			BS_CHECK_GL_ERROR();
			return;
		default:
		case CULL_CLOCKWISE:
			cullMode = GL_FRONT;
			break;
		case CULL_COUNTERCLOCKWISE:
			cullMode = GL_BACK;
			break;
		}

		glEnable(GL_CULL_FACE);
		BS_CHECK_GL_ERROR();

		glCullFace(cullMode);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setDepthBufferCheckEnabled(bool enabled)
	{
		if (enabled)
		{
			glClearDepth(1.0f);
			BS_CHECK_GL_ERROR();

			glEnable(GL_DEPTH_TEST);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
			BS_CHECK_GL_ERROR();
		}
	}

	void GLRenderAPI::setDepthBufferWriteEnabled(bool enabled)
	{
		GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
		glDepthMask(flag);  
		BS_CHECK_GL_ERROR();

		mDepthWrite = enabled;
	}

	void GLRenderAPI::setDepthBufferFunction(CompareFunction func)
	{
		glDepthFunc(convertCompareFunction(func));
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setDepthBias(float constantBias, float slopeScaleBias)
	{
		if (constantBias != 0 || slopeScaleBias != 0)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			BS_CHECK_GL_ERROR();

			glEnable(GL_POLYGON_OFFSET_POINT);
			BS_CHECK_GL_ERROR();

			glEnable(GL_POLYGON_OFFSET_LINE);
			BS_CHECK_GL_ERROR();

			float scaledConstantBias = -constantBias * float((1 << 24) - 1); // Note: Assumes 24-bit depth buffer
			glPolygonOffset(slopeScaleBias, scaledConstantBias);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
			BS_CHECK_GL_ERROR();

			glDisable(GL_POLYGON_OFFSET_POINT);
			BS_CHECK_GL_ERROR();

			glDisable(GL_POLYGON_OFFSET_LINE);
			BS_CHECK_GL_ERROR();
		}
	}

	void GLRenderAPI::setColorBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
	{
		glColorMask(red, green, blue, alpha);
		BS_CHECK_GL_ERROR();

		mColorWrite[0] = red;
		mColorWrite[1] = blue;
		mColorWrite[2] = green;
		mColorWrite[3] = alpha;
	}

	void GLRenderAPI::setPolygonMode(PolygonMode level)
	{
		GLenum glmode;
		switch(level)
		{
		case PM_WIREFRAME:
			glmode = GL_LINE;
			break;
		default:
		case PM_SOLID:
			glmode = GL_FILL;
			break;
		}

		glPolygonMode(GL_FRONT_AND_BACK, glmode);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setStencilCheckEnabled(bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_STENCIL_TEST);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
			BS_CHECK_GL_ERROR();
		}
	}

	void GLRenderAPI::setStencilBufferOperations(StencilOperation stencilFailOp,
		StencilOperation depthFailOp, StencilOperation passOp, bool front)
	{
		if (front)
		{
			glStencilOpSeparate(
				GL_FRONT,
				convertStencilOp(stencilFailOp),
				convertStencilOp(depthFailOp),
				convertStencilOp(passOp));
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glStencilOpSeparate(
				GL_BACK,
				convertStencilOp(stencilFailOp),
				convertStencilOp(depthFailOp),
				convertStencilOp(passOp));
			BS_CHECK_GL_ERROR();
		}
	}

	void GLRenderAPI::setStencilBufferFunc(CompareFunction func, UINT32 mask, bool front)
	{
		mStencilReadMask = mask;

		if(front)
		{
			mStencilCompareFront = func;
			glStencilFuncSeparate(GL_FRONT, convertCompareFunction(mStencilCompareFront), mStencilRefValue, mStencilReadMask);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			mStencilCompareBack = func;
			glStencilFuncSeparate(GL_BACK, convertCompareFunction(mStencilCompareBack), mStencilRefValue, mStencilReadMask);
			BS_CHECK_GL_ERROR();
		}
	}

	void GLRenderAPI::setStencilBufferWriteMask(UINT32 mask)
	{
		mStencilWriteMask = mask;

		glStencilMask(mask);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setStencilRefValue(UINT32 refValue)
	{
		THROW_IF_NOT_CORE_THREAD;

		mStencilRefValue = refValue;

		glStencilFuncSeparate(GL_FRONT, convertCompareFunction(mStencilCompareFront), mStencilRefValue, mStencilReadMask);
		BS_CHECK_GL_ERROR();

		glStencilFuncSeparate(GL_BACK, convertCompareFunction(mStencilCompareBack), mStencilRefValue, mStencilReadMask);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setTextureFiltering(UINT16 unit, FilterType ftype, FilterOptions fo)
	{
		switch(ftype)
		{
		case FT_MIN:
			mMinFilter = fo;
			// Combine with existing mip filter
			glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_MIN_FILTER, getCombinedMinMipFilter());
			BS_CHECK_GL_ERROR();
			break;
		case FT_MAG:
			switch (fo)
			{
			case FO_ANISOTROPIC: // GL treats linear and aniso the same
			case FO_LINEAR:
				glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				BS_CHECK_GL_ERROR();
				break;
			case FO_POINT:
			case FO_NONE:
				glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				BS_CHECK_GL_ERROR();
				break;
			default:
				break;
			}
			break;
		case FT_MIP:
			mMipFilter = fo;
			// Combine with existing min filter
			glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_MIN_FILTER, getCombinedMinMipFilter());
			BS_CHECK_GL_ERROR();
			break;
		}
	}

	void GLRenderAPI::setTextureAnisotropy(UINT16 unit, UINT32 maxAnisotropy)
	{
		GLfloat maxSupportAnisotropy = 0;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupportAnisotropy);
		BS_CHECK_GL_ERROR();

		if (maxAnisotropy > maxSupportAnisotropy)
			maxAnisotropy = maxSupportAnisotropy ? static_cast<UINT32>(maxSupportAnisotropy) : 1;

		if(maxAnisotropy < 1)
			maxAnisotropy = 1;

		glTexParameterf(mTextureInfos[unit].type, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)maxAnisotropy);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::setTextureCompareMode(UINT16 unit, CompareFunction compare)
	{
		if (compare == CMPF_ALWAYS_PASS)
		{
			glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			BS_CHECK_GL_ERROR();
		}
		else
		{
			glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			BS_CHECK_GL_ERROR();

			glTexParameteri(mTextureInfos[unit].type, GL_TEXTURE_COMPARE_FUNC, convertCompareFunction(compare));
			BS_CHECK_GL_ERROR();
		}
	}

	bool GLRenderAPI::activateGLTextureUnit(UINT16 unit)
	{
		if (mActiveTextureUnit != unit)
		{
			if (unit < getCapabilities(0).getNumCombinedTextureUnits())
			{
				glActiveTexture(GL_TEXTURE0 + unit);
				BS_CHECK_GL_ERROR();

				mActiveTextureUnit = unit;
				return true;
			}
			else if (!unit)
			{
				// Always ok to use the first unit
				return true;
			}
			else
			{
				LOGWRN("Provided texture unit index is higher than OpenGL supports. Provided: " + toString(unit) + 
					". Supported range: 0 .. " + toString(getCapabilities(0).getNumCombinedTextureUnits() - 1));
				return false;
			}
		}
		else
		{
			return true;
		}
	}

	void GLRenderAPI::beginDraw()
	{
		if(mDrawCallInProgress)
			BS_EXCEPT(InternalErrorException, "Calling beginDraw without finishing previous draw call. Please call endDraw().");

		mDrawCallInProgress = true;

		if(mCurrentVertexProgram == nullptr)
		{
			LOGWRN("Cannot render without a set vertex shader.");
			return;
		}

		if(mBoundVertexDeclaration == nullptr)
		{
			LOGWRN("Cannot render without a set vertex declaration.");
			return;
		}

		const GLSLProgramPipeline* pipeline = mProgramPipelineManager->getPipeline(mCurrentVertexProgram.get(), 
			mCurrentFragmentProgram.get(), mCurrentGeometryProgram.get(), mCurrentHullProgram.get(), mCurrentDomainProgram.get());

		glUseProgram(0);
		BS_CHECK_GL_ERROR();

		if(mActivePipeline != pipeline)
		{
			glBindProgramPipeline(pipeline->glHandle);
			BS_CHECK_GL_ERROR();

			mActivePipeline = pipeline;
		}

		const GLVertexArrayObject& vao = GLVertexArrayObjectManager::instance().getVAO(
			mCurrentVertexProgram,
			mBoundVertexDeclaration,
			mBoundVertexBuffers);

		glBindVertexArray(vao.getGLHandle()); 
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT(NumVertexBufferBinds);
	}

	void GLRenderAPI::endDraw()
	{
		if(!mDrawCallInProgress)
			return;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
#endif

		mDrawCallInProgress = false;
	}

	GLfloat GLRenderAPI::getCurrentAnisotropy(UINT16 unit)
	{
		GLfloat curAniso = 0;
		glGetTexParameterfv(mTextureInfos[unit].type, GL_TEXTURE_MAX_ANISOTROPY_EXT, &curAniso);
		BS_CHECK_GL_ERROR();

		return curAniso ? curAniso : 1;
	}

	GLint GLRenderAPI::convertStencilOp(StencilOperation op) const
	{
		switch (op)
		{
		case SOP_KEEP:
			return GL_KEEP;
		case SOP_ZERO:
			return GL_ZERO;
		case SOP_REPLACE:
			return GL_REPLACE;
		case SOP_INCREMENT:
			return GL_INCR;
		case SOP_DECREMENT:
			return GL_DECR;
		case SOP_INCREMENT_WRAP:
			return GL_INCR_WRAP;
		case SOP_DECREMENT_WRAP:
			return GL_DECR_WRAP;
		case SOP_INVERT:
			return GL_INVERT;
		}

		return SOP_KEEP;
	}

	GLint GLRenderAPI::convertCompareFunction(CompareFunction func) const
	{
		switch (func)
		{
		case CMPF_ALWAYS_FAIL:
			return GL_NEVER;
		case CMPF_ALWAYS_PASS:
			return GL_ALWAYS;
		case CMPF_LESS:
			return GL_LESS;
		case CMPF_LESS_EQUAL:
			return GL_LEQUAL;
		case CMPF_EQUAL:
			return GL_EQUAL;
		case CMPF_NOT_EQUAL:
			return GL_NOTEQUAL;
		case CMPF_GREATER_EQUAL:
			return GL_GEQUAL;
		case CMPF_GREATER:
			return GL_GREATER;
		}

		return GL_ALWAYS;
	}

	GLuint GLRenderAPI::getCombinedMinMipFilter() const
	{
		switch (mMinFilter)
		{
		case FO_ANISOTROPIC:
		case FO_LINEAR:
			switch (mMipFilter)
			{
			case FO_ANISOTROPIC:
			case FO_LINEAR:
				// Linear min, linear mip
				return GL_LINEAR_MIPMAP_LINEAR;
			case FO_POINT:
				// Linear min, point mip
				return GL_LINEAR_MIPMAP_NEAREST;
			case FO_NONE:
				// Linear min, no mip
				return GL_LINEAR;
			default:
				break;
			}
			break;
		case FO_POINT:
		case FO_NONE:
			switch (mMipFilter)
			{
			case FO_ANISOTROPIC:
			case FO_LINEAR:
				// Nearest min, linear mip
				return GL_NEAREST_MIPMAP_LINEAR;
			case FO_POINT:
				// Nearest min, point mip
				return GL_NEAREST_MIPMAP_NEAREST;
			case FO_NONE:
				// Nearest min, no mip
				return GL_NEAREST;
			default:
				break;
			}
			break;
		default:
			break;
		}

		// Should never get here
		return 0;
	}

	GLint GLRenderAPI::getBlendMode(BlendFactor blendMode) const
	{
		switch (blendMode)
		{
		case BF_ONE:
			return GL_ONE;
		case BF_ZERO:
			return GL_ZERO;
		case BF_DEST_COLOR:
			return GL_DST_COLOR;
		case BF_SOURCE_COLOR:
			return GL_SRC_COLOR;
		case BF_INV_DEST_COLOR:
			return GL_ONE_MINUS_DST_COLOR;
		case BF_INV_SOURCE_COLOR:
			return GL_ONE_MINUS_SRC_COLOR;
		case BF_DEST_ALPHA:
			return GL_DST_ALPHA;
		case BF_SOURCE_ALPHA:
			return GL_SRC_ALPHA;
		case BF_INV_DEST_ALPHA:
			return GL_ONE_MINUS_DST_ALPHA;
		case BF_INV_SOURCE_ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		}

		return GL_ONE;
	}

	GLint GLRenderAPI::getTextureAddressingMode(TextureAddressingMode tam) const
	{
		switch (tam)
		{
		default:
		case TAM_WRAP:
			return GL_REPEAT;
		case TAM_MIRROR:
			return GL_MIRRORED_REPEAT;
		case TAM_CLAMP:
			return GL_CLAMP_TO_EDGE;
		case TAM_BORDER:
			return GL_CLAMP_TO_BORDER;
		}
	}

	GLint GLRenderAPI::getGLDrawMode() const
	{
		GLint primType;

		// Use adjacency if there is a geometry program and it requested adjacency info
		bool useAdjacency = (mCurrentGeometryProgram != nullptr && mCurrentGeometryProgram->isAdjacencyInfoRequired());
		switch (mCurrentDrawOperation)
		{
		case DOT_POINT_LIST:
			primType = GL_POINTS;
			break;
		case DOT_LINE_LIST:
			primType = useAdjacency ? GL_LINES_ADJACENCY : GL_LINES;
			break;
		case DOT_LINE_STRIP:
			primType = useAdjacency ? GL_LINE_STRIP_ADJACENCY : GL_LINE_STRIP;
			break;
		default:
		case DOT_TRIANGLE_LIST:
			primType = useAdjacency ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;
			break;
		case DOT_TRIANGLE_STRIP:
			primType = useAdjacency ? GL_TRIANGLE_STRIP_ADJACENCY : GL_TRIANGLE_STRIP;
			break;
		case DOT_TRIANGLE_FAN:
			primType = GL_TRIANGLE_FAN;
			break;
		}

		return primType;
	}

	SPtr<GLSLGpuProgram> GLRenderAPI::getActiveProgram(GpuProgramType gptype) const
	{
		switch (gptype)
		{
		case GPT_VERTEX_PROGRAM:
			return mCurrentVertexProgram;
		case GPT_FRAGMENT_PROGRAM:
			return mCurrentFragmentProgram;
		case GPT_GEOMETRY_PROGRAM:
			return mCurrentGeometryProgram;
		case GPT_DOMAIN_PROGRAM:
			return mCurrentDomainProgram;
		case GPT_HULL_PROGRAM:
			return mCurrentHullProgram;
		case GPT_COMPUTE_PROGRAM:
			return mCurrentComputeProgram;
		default:
			BS_EXCEPT(InvalidParametersException, "Unsupported gpu program type: " + toString(gptype));
		}

		return nullptr;
	}

	void GLRenderAPI::initFromCaps(RenderAPICapabilities* caps)
	{
		if(caps->getRenderAPIName() != getName())
		{
			BS_EXCEPT(InvalidParametersException, 
				"Trying to initialize GLRenderAPI from RenderSystemCapabilities that do not support OpenGL");
		}

#if BS_DEBUG_MODE && (BS_OPENGL_4_3 || BS_OPENGLES_3_2)
		if (mGLSupport->checkExtension("GL_ARB_debug_output"))
		{
			glDebugMessageCallback(&openGlErrorCallback, 0);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		}
#endif

		bs::HardwareBufferManager::startUp();
		HardwareBufferManager::startUp<GLHardwareBufferManager>();

		// GPU Program Manager setup
		mGLSLProgramFactory = bs_new<GLSLProgramFactory>();
		if(caps->isShaderProfileSupported("glsl")) // Check for most recent GLSL support
			GpuProgramManager::instance().addFactory("glsl", mGLSLProgramFactory);

		if(caps->isShaderProfileSupported("glsl4_1")) // Check for OpenGL 4.1 compatible version
			GpuProgramManager::instance().addFactory("glsl4_1", mGLSLProgramFactory);

		GLRTTManager::startUp<GLRTTManager>();

		mNumTextureUnits = caps->getNumCombinedTextureUnits();
		mTextureInfos = bs_newN<TextureInfo>(mNumTextureUnits);
		
		bs::TextureManager::startUp<bs::GLTextureManager>(std::ref(*mGLSupport));
		TextureManager::startUp<GLTextureManager>(std::ref(*mGLSupport));
	}

	void GLRenderAPI::switchContext(const SPtr<GLContext>& context, const RenderWindow& window)
	{
		// Unbind pipeline and rebind to new context later	
		setGraphicsPipeline(nullptr);

		if (mCurrentContext)
			mCurrentContext->endCurrent();

		mCurrentContext = context;
		mCurrentContext->setCurrent(window);

		// Must reset depth/colour write mask to according with user desired, otherwise, clearFrameBuffer would be wrong 
		// because the value we recorded may be different from the real state stored in GL context.
		glDepthMask(mDepthWrite);
		BS_CHECK_GL_ERROR();

		glColorMask(mColorWrite[0], mColorWrite[1], mColorWrite[2], mColorWrite[3]);
		BS_CHECK_GL_ERROR();

		glStencilMask(mStencilWriteMask);
		BS_CHECK_GL_ERROR();
	}

	void GLRenderAPI::initCapabilities(RenderAPICapabilities& caps) const
	{
		Vector<String> tokens = StringUtil::split(mGLSupport->getGLVersion(), ".");

		DriverVersion driverVersion;
		if (!tokens.empty())
		{
			driverVersion.major = parseINT32(tokens[0]);
			if (tokens.size() > 1)
				driverVersion.minor = parseINT32(tokens[1]);
			if (tokens.size() > 2)
				driverVersion.release = parseINT32(tokens[2]);
		}
		driverVersion.build = 0;

		caps.setDriverVersion(driverVersion);
		caps.setRenderAPIName(getName());

		const char* deviceName = (const char*)glGetString(GL_RENDERER);
		caps.setDeviceName(deviceName);

		const char* vendorName = (const char*)glGetString(GL_VENDOR);
		if (strstr(vendorName, "NVIDIA"))
			caps.setVendor(GPU_NVIDIA);
		else if (strstr(vendorName, "ATI"))
			caps.setVendor(GPU_AMD);
		else if (strstr(vendorName, "AMD"))
			caps.setVendor(GPU_AMD);
		else if (strstr(vendorName, "Intel"))
			caps.setVendor(GPU_INTEL);
		else
			caps.setVendor(GPU_UNKNOWN);

#if BS_OPENGL_4_1
		caps.addShaderProfile("glsl4_1");
#endif

#if BS_OPENGL_4_5
		caps.addShaderProfile("glsl");
#endif

		caps.setCapability(RSC_TEXTURE_COMPRESSION_BC);

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		caps.setCapability(RSC_GEOMETRY_PROGRAM);
#endif

		GLint maxOutputVertices;

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxOutputVertices);
		BS_CHECK_GL_ERROR();
#else
		maxOutputVertices = 0;
#endif

		caps.setGeometryProgramNumOutputVertices(maxOutputVertices);

		// Max number of fragment shader textures
		GLint units;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units);
		BS_CHECK_GL_ERROR();

		caps.setNumTextureUnits(GPT_FRAGMENT_PROGRAM, static_cast<UINT16>(units));

		// Max number of vertex shader textures
		GLint vUnits;
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vUnits);
		BS_CHECK_GL_ERROR();

		caps.setNumTextureUnits(GPT_VERTEX_PROGRAM, static_cast<UINT16>(vUnits));

		GLint numUniformBlocks;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &numUniformBlocks);
		BS_CHECK_GL_ERROR();

		caps.setNumGpuParamBlockBuffers(GPT_VERTEX_PROGRAM, numUniformBlocks);

		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &numUniformBlocks);
		BS_CHECK_GL_ERROR();

		caps.setNumGpuParamBlockBuffers(GPT_FRAGMENT_PROGRAM, numUniformBlocks);

		{
			GLint geomUnits;

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
			glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &geomUnits);
			BS_CHECK_GL_ERROR();
#else
			geomUnits = 0;
#endif

			caps.setNumTextureUnits(GPT_GEOMETRY_PROGRAM, static_cast<UINT16>(geomUnits));

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &numUniformBlocks);
			BS_CHECK_GL_ERROR();
#else
			numUniformBlocks = 0;
#endif

			caps.setNumGpuParamBlockBuffers(GPT_GEOMETRY_PROGRAM, numUniformBlocks);
		}

		if (mGLSupport->checkExtension("GL_ARB_tessellation_shader"))
		{
#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
			caps.setCapability(RSC_TESSELLATION_PROGRAM);
#endif

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
			glGetIntegerv(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, &numUniformBlocks);
			BS_CHECK_GL_ERROR();
#else
			numUniformBlocks = 0;
#endif

			caps.setNumGpuParamBlockBuffers(GPT_HULL_PROGRAM, numUniformBlocks);

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
			glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, &numUniformBlocks);
			BS_CHECK_GL_ERROR();
#else
			numUniformBlocks = 0;
#endif

			caps.setNumGpuParamBlockBuffers(GPT_DOMAIN_PROGRAM, numUniformBlocks);
		}

		if (mGLSupport->checkExtension("GL_ARB_compute_shader")) 
		{
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			caps.setCapability(RSC_COMPUTE_PROGRAM);
#endif

			GLint computeUnits;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &computeUnits);
			BS_CHECK_GL_ERROR();
#else
			computeUnits = 0;
#endif

			caps.setNumTextureUnits(GPT_COMPUTE_PROGRAM, static_cast<UINT16>(computeUnits));

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &numUniformBlocks);
			BS_CHECK_GL_ERROR();
#else
			numUniformBlocks = 0;
#endif

			caps.setNumGpuParamBlockBuffers(GPT_COMPUTE_PROGRAM, numUniformBlocks);

			// Max number of load-store textures
			GLint lsfUnits;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &lsfUnits);
			BS_CHECK_GL_ERROR();
#else
			lsfUnits = 0;
#endif

			caps.setNumLoadStoreTextureUnits(GPT_FRAGMENT_PROGRAM, static_cast<UINT16>(lsfUnits));

			GLint lscUnits;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &lscUnits);
			BS_CHECK_GL_ERROR();
#else
			lscUnits = 0;
#endif

			caps.setNumLoadStoreTextureUnits(GPT_COMPUTE_PROGRAM, static_cast<UINT16>(lscUnits));

			GLint combinedLoadStoreTextureUnits;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			glGetIntegerv(GL_MAX_IMAGE_UNITS, &combinedLoadStoreTextureUnits);
			BS_CHECK_GL_ERROR();
#else
			combinedLoadStoreTextureUnits = 0;
#endif

			caps.setNumCombinedLoadStoreTextureUnits(static_cast<UINT16>(combinedLoadStoreTextureUnits));
		}

		GLint combinedTexUnits;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combinedTexUnits);
		BS_CHECK_GL_ERROR();

		caps.setNumCombinedTextureUnits(static_cast<UINT16>(combinedTexUnits));

		GLint combinedUniformBlockUnits;
		glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &combinedUniformBlockUnits);
		BS_CHECK_GL_ERROR();

		caps.setNumCombinedGpuParamBlockBuffers(static_cast<UINT16>(combinedUniformBlockUnits));

		caps.setNumMultiRenderTargets(8);
	}

	void GLRenderAPI::makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m)
	{
		UINT32 x = 0;
		for (UINT32 i = 0; i < 4; i++)
		{
			for (UINT32 j = 0; j < 4; j++)
			{
				gl_matrix[x] = m[j][i];
				x++;
			}
		}
	}

	void GLRenderAPI::applyViewport()
	{
		if (mActiveRenderTarget == nullptr)
			return;

		const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();

		// Calculate the "lower-left" corner of the viewport
		mViewportLeft = (UINT32)(rtProps.width * mViewportNorm.x);
		mViewportTop = (UINT32)(rtProps.height * mViewportNorm.y);
		mViewportWidth = (UINT32)(rtProps.width * mViewportNorm.width);
		mViewportHeight = (UINT32)(rtProps.height * mViewportNorm.height);

		glViewport(mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight);
		BS_CHECK_GL_ERROR();

		// Configure the viewport clipping
		if (!mScissorEnabled)
		{
			glEnable(GL_SCISSOR_TEST);
			BS_CHECK_GL_ERROR();

			glScissor(mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight);
			BS_CHECK_GL_ERROR();
		}
	}

	/************************************************************************/
	/* 								UTILITY		                     		*/
	/************************************************************************/

	void GLRenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{
		dest = matrix;
	}

	const RenderAPIInfo& GLRenderAPI::getAPIInfo() const
	{
		RenderAPIFeatures featureFlags =
			RenderAPIFeatureFlag::UVYAxisUp |
			RenderAPIFeatureFlag::ColumnMajorMatrices |
			RenderAPIFeatureFlag::MSAAImageStores;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		featureFlags |= RenderAPIFeatureFlag::TextureViews;
		featureFlags |= RenderAPIFeatureFlag::Compute;
#endif

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
		featureFlags |= RenderAPIFeatureFlag::LoadStore;
#endif

#if BS_PLATFORM != BS_PLATFORM_OSX
		featureFlags |= RenderAPIFeatureFlag::RenderTargetLayers;
#endif

		static RenderAPIInfo info(0.0f, 0.0f, -1.0f, 1.0f, VET_COLOR_ABGR, featureFlags);
								  
		return info;
	}

	GpuParamBlockDesc GLRenderAPI::generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
	{
		GpuParamBlockDesc block;
		block.blockSize = 0;
		block.isShareable = true;
		block.name = name;
		block.slot = 0;
		block.set = 0;

		for (auto& param : params)
		{
			UINT32 size;
			
			if(param.type == GPDT_STRUCT)
			{
				// Structs are always aligned and rounded up to vec4
				size = Math::divideAndRoundUp(param.elementSize, 16U) * 4;
				block.blockSize = Math::divideAndRoundUp(block.blockSize, 4U) * 4;
			}
			else
				size = GLSLParamParser::calcInterfaceBlockElementSizeAndOffset(param.type, param.arraySize, block.blockSize);

			if (param.arraySize > 1)
			{
				param.elementSize = size;
				param.arrayElementStride = size;
				param.cpuMemOffset = block.blockSize;
				param.gpuMemOffset = 0;
				
				block.blockSize += size * param.arraySize;
			}
			else
			{
				param.elementSize = size;
				param.arrayElementStride = size;
				param.cpuMemOffset = block.blockSize;
				param.gpuMemOffset = 0;

				block.blockSize += size;
			}

			param.paramBlockSlot = 0;
			param.paramBlockSet = 0;
		}

		// Constant buffer size must always be a multiple of 16
		if (block.blockSize % 4 != 0)
			block.blockSize += (4 - (block.blockSize % 4));

		return block;
	}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
	void openGlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar *message, GLvoid *userParam)
	{
		if (type != GL_DEBUG_TYPE_PERFORMANCE && type != GL_DEBUG_TYPE_OTHER)
		{
			BS_EXCEPT(RenderingAPIException, "OpenGL error: " + String(message));
		}
	}
#endif
}}
