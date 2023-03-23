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

namespace bs { namespace ct {
const char* MODULE_NAME = "bsfGLRenderAPI.dll";
}} // namespace bs::ct

using namespace bs;
using namespace bs::ct;

const char* B3DGetOpenGLErrorString(GLenum errorCode)
{
	switch(errorCode)
	{
	case GL_INVALID_OPERATION: return "INVALID_OPERATION";
	case GL_INVALID_ENUM: return "INVALID_ENUM";
	case GL_INVALID_VALUE: return "INVALID_VALUE";
	case GL_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION";
	}

	return nullptr;
}

void B3DCheckForOpenGLError(const char* function, const char* file, i32 line)
{
	GLenum errorCode = glGetError();
	if(errorCode != GL_NO_ERROR)
	{
		StringStream errorOutput;
		errorOutput << "OpenGL error in " << function << " [" << file << ":" << ToString(line) << "]:\n";

		while(errorCode != GL_NO_ERROR)
		{
			const char* errorString = ::B3DGetOpenGLErrorString(errorCode);
			if(errorString)
				errorOutput << "\t - " << errorString;

			errorCode = glGetError();
		}

		B3D_LOG(Warning, RenderBackend, errorOutput.str());
	}
}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
void OpenGlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
#endif

/************************************************************************/
/* 								PUBLIC INTERFACE                   		*/
/************************************************************************/

GLRenderAPI::GLRenderAPI()
{
	// Get our GLSupport
	mGLSupport = ct::GetGlSupport();

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		mColorWrite[i][0] = mColorWrite[i][1] = mColorWrite[i][2] = mColorWrite[i][3] = true;

	mCurrentContext = 0;
	mMainContext = 0;

	mGLInitialised = false;

	mMinFilter = FO_LINEAR;
	mMipFilter = FO_POINT;

	mProgramPipelineManager = B3DNew<GLSLProgramPipelineManager>();
}

const StringID& GLRenderAPI::GetName() const
{
	static StringID strName("GLRenderAPI");
	return strName;
}

void GLRenderAPI::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	mGLSupport->Start();
	mVideoModeInfo = mGLSupport->GetVideoModeInfo();

	CommandBufferManager::StartUp<GLCommandBufferManager>();
	bs::RenderWindowManager::StartUp<bs::GLRenderWindowManager>(this);
	RenderWindowManager::StartUp();

	RenderStateManager::StartUp();

	QueryManager::StartUp<GLQueryManager>();

	// Create main command buffer
	mMainCommandBuffer = std::static_pointer_cast<GLCommandBuffer>(CommandBuffer::Create(GQT_GRAPHICS));

	RenderAPI::Initialize();
}

void GLRenderAPI::InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow)
{
	// Get the context from the window and finish initialization
	SPtr<GLContext> context;
	primaryWindow->GetCustomAttribute("GLCONTEXT", &context);

	// Set main and current context
	mMainContext = context;
	mCurrentContext = mMainContext;

	// Set primary context as active
	if(mCurrentContext)
		mCurrentContext->SetCurrent(*primaryWindow);

	// Setup GLSupport
	mGLSupport->InitializeExtensions();

	mNumDevices = 1;
	mCurrentCapabilities = B3DNewMultiple<GpuDeviceCapabilities>(mNumDevices);
	InitCapabilities(mCurrentCapabilities[0]);

	InitFromCaps(mCurrentCapabilities);
	GLVertexArrayObjectManager::StartUp();

	glFrontFace(GL_CW);
	B3D_CHECK_GL_ERROR();

	// Ensure cubemaps are filtered across seams
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	B3D_CHECK_GL_ERROR();

	GPUInfo gpuInfo;
	gpuInfo.NumGpUs = 1;

	const char* vendor = (const char*)glGetString(GL_VENDOR);
	B3D_CHECK_GL_ERROR();

	const char* renderer = (const char*)glGetString(GL_RENDERER);
	B3D_CHECK_GL_ERROR();

	gpuInfo.Names[0] = String(vendor) + " " + String(renderer);

	PlatformUtility::SetGPUInfoInternal(gpuInfo);

	mGLInitialised = true;

	RenderAPI::InitializeWithWindow(primaryWindow);
}

void GLRenderAPI::DestroyCore()
{
	RenderAPI::DestroyCore();

	// Deleting the GLSL program factory
	if(mGLSLProgramFactory)
	{
		// Remove from manager safely
		GpuProgramManager::Instance().RemoveFactory("glsl");
		GpuProgramManager::Instance().RemoveFactory("glsl4_1");

		B3DDelete(mGLSLProgramFactory);
		mGLSLProgramFactory = nullptr;
	}

	// Deleting the hardware buffer manager.  Has to be done before the mGLSupport->stop().
	HardwareBufferManager::ShutDown();
	bs::HardwareBufferManager::ShutDown();
	GLRTTManager::ShutDown();

	for(u32 i = 0; i < kMaxVbCount; i++)
		mBoundVertexBuffers[i] = nullptr;

	mBoundVertexDeclaration = nullptr;
	mBoundIndexBuffer = nullptr;

	mCurrentVertexProgram = nullptr;
	mCurrentFragmentProgram = nullptr;
	mCurrentGeometryProgram = nullptr;
	mCurrentHullProgram = nullptr;
	mCurrentDomainProgram = nullptr;
	mCurrentComputeProgram = nullptr;
	mMainCommandBuffer = nullptr;

	if(mGLSupport)
		mGLSupport->Stop();

	TextureManager::ShutDown();
	bs::TextureManager::ShutDown();
	QueryManager::ShutDown();
	RenderWindowManager::ShutDown();
	bs::RenderWindowManager::ShutDown();
	RenderStateManager::ShutDown();
	GLVertexArrayObjectManager::ShutDown(); // Note: Needs to be after QueryManager shutdown as some resources might be waiting for queries to complete
	CommandBufferManager::ShutDown();

	mGLInitialised = false;

	if(mProgramPipelineManager != nullptr)
		B3DDelete(mProgramPipelineManager);

	if(mCurrentContext)
		mCurrentContext->EndCurrent();

	mCurrentContext = nullptr;
	mMainContext = nullptr;

	if(mGLSupport)
		B3DDelete(mGLSupport);

	if(mTextureInfos != nullptr)
		B3DDeleteMultiple(mTextureInfos, mNumTextureUnits);
}

void GLRenderAPI::SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<GraphicsPipelineState>& pipelineState)
	{
		THROW_IF_NOT_CORE_THREAD;

		BlendState* blendState;
		RasterizerState* rasterizerState;
		DepthStencilState* depthStencilState;
		if(pipelineState != nullptr)
		{
			mCurrentVertexProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->GetVertexProgram());
			mCurrentFragmentProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->GetFragmentProgram());
			mCurrentGeometryProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->GetGeometryProgram());
			mCurrentDomainProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->GetDomainProgram());
			mCurrentHullProgram = std::static_pointer_cast<GLSLGpuProgram>(pipelineState->GetHullProgram());

			blendState = pipelineState->GetBlendState().get();
			rasterizerState = pipelineState->GetRasterizerState().get();
			depthStencilState = pipelineState->GetDepthStencilState().get();

			if(blendState == nullptr)
				blendState = BlendState::GetDefault().get();

			if(rasterizerState == nullptr)
				rasterizerState = RasterizerState::GetDefault().get();

			if(depthStencilState == nullptr)
				depthStencilState = DepthStencilState::GetDefault().get();
		}
		else
		{
			mCurrentVertexProgram = nullptr;
			mCurrentFragmentProgram = nullptr;
			mCurrentGeometryProgram = nullptr;
			mCurrentDomainProgram = nullptr;
			mCurrentHullProgram = nullptr;

			blendState = BlendState::GetDefault().get();
			rasterizerState = RasterizerState::GetDefault().get();
			depthStencilState = DepthStencilState::GetDefault().get();
		}

		// Blend state
		{
			const BlendProperties& stateProps = blendState->GetProperties();

			// Alpha to coverage
			SetAlphaToCoverage(stateProps.GetAlphaToCoverageEnabled());

			for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
			{
				// Blending
				if(stateProps.GetBlendEnabled(i))
				{
					SetSceneBlending(i, stateProps.GetSrcBlend(i), stateProps.GetDstBlend(i), stateProps.GetAlphaSrcBlend(i), stateProps.GetAlphaDstBlend(i), stateProps.GetBlendOperation(i), stateProps.GetAlphaBlendOperation(i));
				}
				else
					SetSceneBlending(i, BF_ONE, BF_ZERO, BO_ADD);

				// Color write mask
				u8 writeMask = stateProps.GetRenderTargetWriteMask(i);
				SetColorBufferWriteEnabled(i, (writeMask & 0x1) != 0, (writeMask & 0x2) != 0, (writeMask & 0x4) != 0, (writeMask & 0x8) != 0);
			}
		}

		// Rasterizer state
		{
			const RasterizerProperties& stateProps = rasterizerState->GetProperties();

			SetDepthBias(stateProps.GetDepthBias(), stateProps.GetSlopeScaledDepthBias());
			SetCullingMode(stateProps.GetCullMode());
			SetPolygonMode(stateProps.GetPolygonMode());
			SetScissorTestEnable(stateProps.GetScissorEnable());
			SetMultisamplingEnable(stateProps.GetMultisampleEnable());
			SetDepthClipEnable(stateProps.GetDepthClipEnable());
			SetAntialiasedLineEnable(stateProps.GetAntialiasedLineEnable());
		}

		// Depth stencil state
		{
			const DepthStencilProperties& stateProps = depthStencilState->GetProperties();

			// Set stencil buffer options
			SetStencilCheckEnabled(stateProps.GetStencilEnable());

			SetStencilBufferOperations(stateProps.GetStencilFrontFailOp(), stateProps.GetStencilFrontZFailOp(), stateProps.GetStencilFrontPassOp(), true);
			SetStencilBufferFunc(stateProps.GetStencilFrontCompFunc(), stateProps.GetStencilReadMask(), true);

			SetStencilBufferOperations(stateProps.GetStencilBackFailOp(), stateProps.GetStencilBackZFailOp(), stateProps.GetStencilBackPassOp(), false);
			SetStencilBufferFunc(stateProps.GetStencilBackCompFunc(), stateProps.GetStencilReadMask(), false);

			SetStencilBufferWriteMask(stateProps.GetStencilWriteMask());

			// Set depth buffer options
			SetDepthBufferCheckEnabled(stateProps.GetDepthReadEnable());
			SetDepthBufferWriteEnabled(stateProps.GetDepthWriteEnable());
			SetDepthBufferFunction(stateProps.GetDepthComparisonFunc());
		}
	};

	auto execute = [=]()
	{ executeRef(pipelineState); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void GLRenderAPI::SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<ComputePipelineState>& pipelineState)
	{
		THROW_IF_NOT_CORE_THREAD;

		SPtr<GpuProgram> program;
		if(pipelineState != nullptr)
			program = pipelineState->GetProgram();

		if(program != nullptr && program->GetType() == GPT_COMPUTE_PROGRAM)
			mCurrentComputeProgram = std::static_pointer_cast<GLSLGpuProgram>(program);
		else
			mCurrentComputeProgram = nullptr;
	};

	auto execute = [=]()
	{ executeRef(pipelineState); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void GLRenderAPI::SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<GpuParams>& gpuParams)
	{
		THROW_IF_NOT_CORE_THREAD;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
		for(u32 i = 0; i < 8; i++)
		{
			glBindImageTexture(i, 0, 0, false, 0, GL_READ_WRITE, GL_R32F);
			B3D_CHECK_GL_ERROR();
		}
#endif

		B3DMarkAllocatorFrame();
		{
			FrameVector<u32> textureUnits;
			textureUnits.reserve(12);

			auto getTexUnit = [&](u32 binding)
			{
				for(u32 i = 0; i < (u32)textureUnits.size(); i++)
				{
					if(textureUnits[i] == binding)
						return i;
				}

				u32 unit = (u32)textureUnits.size();
				textureUnits.push_back(binding);

				return unit;
			};

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			u32 imageUnitCount = 0;
			auto getImageUnit = [&](u32 binding)
			{
				return imageUnitCount++;
			};
#endif

			u32 uniformUnitCount = 0;
			auto getUniformUnit = [&](u32 binding)
			{
				return uniformUnitCount++;
			};

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			u32 sharedStorageUnitCount = 0;
			auto getSharedStorageUnit = [&](u32 binding)
			{
				return sharedStorageUnitCount++;
			};
#endif

			const u32 numStages = 6;
			for(u32 i = 0; i < numStages; i++)
			{
				for(auto& entry : textureUnits)
					entry = (u32)-1;

				GpuProgramType type = (GpuProgramType)i;

				SPtr<GpuParamDesc> paramDesc = gpuParams->GetParameterInformation(type);
				if(paramDesc == nullptr)
					continue;

				for(auto& entry : paramDesc->Textures)
				{
					u32 binding = entry.second.Slot;
					SPtr<Texture> texture = gpuParams->GetSampledTexture(entry.second.Set, binding);
					const TextureSurface& surface = gpuParams->GetTextureSurface(entry.second.Set, binding);

					u32 unit = getTexUnit(binding);
					if(!ActivateGlTextureUnit(unit))
						continue;

					TextureInfo& texInfo = mTextureInfos[unit];

					GLTexture* glTex = static_cast<GLTexture*>(texture.get());
					GLenum newTextureType;
					GLuint texId;
					if(glTex != nullptr)
					{
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
						SPtr<TextureView> texView = glTex->RequestView(surface, GVU_DEFAULT);

						GLTextureView* glTexView = static_cast<GLTextureView*>(texView.get());

						newTextureType = glTexView->GetGlTextureTarget();
						texId = glTexView->GetGlid();
#else
						// Texture views are not supported, so if user requested a part of the texture surface report
						// a warning
						auto& props = texture->GetProperties();
						if(surface.mipLevel != 0 || surface.face != 0 ||
						   (surface.numMipLevels != 0 && surface.numMipLevels != props.getNumMipmaps()) ||
						   (surface.numFaces != 0 && surface.numFaces != props.getNumFaces()))
						{
							B3D_LOG(Warning, RenderBackend, "Attempting to bind only a part of a texture, but texture views are not supported. "
														   "Entire texture will be bound instead.");
						}

						newTextureType = glTex->GetGLTextureTarget();
						texId = glTex->GetGLID();
#endif
					}
					else
					{

						newTextureType = GLTexture::GetGlTextureTarget(entry.second.Type);
						texId = 0;
					}

					if(texInfo.Type != newTextureType)
					{
						glBindTexture(texInfo.Type, 0);
						B3D_CHECK_GL_ERROR();
					}

					glBindTexture(newTextureType, texId);
					B3D_CHECK_GL_ERROR();

					texInfo.Type = newTextureType;

					SPtr<GLSLGpuProgram> activeProgram = GetActiveProgram(type);
					if(activeProgram != nullptr)
					{
						GLuint glProgram = activeProgram->GetGlHandle();

						glProgramUniform1i(glProgram, binding, unit);
						B3D_CHECK_GL_ERROR();
					}
				}

				for(auto& entry : paramDesc->Samplers)
				{
					u32 binding = entry.second.Slot;
					SPtr<SamplerState> samplerState = gpuParams->GetSamplerState(entry.second.Set, binding);

					if(samplerState == nullptr)
						samplerState = SamplerState::GetDefault();

					u32 unit = getTexUnit(binding);
					if(!ActivateGlTextureUnit(unit))
						continue;

					// No sampler options for multisampled textures or buffers
					bool supportsSampler = mTextureInfos[unit].Type != GL_TEXTURE_2D_MULTISAMPLE &&
						mTextureInfos[unit].Type != GL_TEXTURE_2D_MULTISAMPLE_ARRAY &&
						mTextureInfos[unit].Type != GL_TEXTURE_BUFFER;

					if(supportsSampler)
					{
						const SamplerProperties& stateProps = samplerState->GetProperties();

						SetTextureFiltering(unit, FT_MIN, stateProps.GetTextureFiltering(FT_MIN));
						SetTextureFiltering(unit, FT_MAG, stateProps.GetTextureFiltering(FT_MAG));
						SetTextureFiltering(unit, FT_MIP, stateProps.GetTextureFiltering(FT_MIP));

						SetTextureAnisotropy(unit, stateProps.GetTextureAnisotropy());
						SetTextureCompareMode(unit, stateProps.GetComparisonFunction());

						SetTextureMipmapBias(unit, stateProps.GetTextureMipmapBias());
						SetTextureMipmapRange(unit, stateProps.GetMinimumMip(), stateProps.GetMaximumMip());

						const UVWAddressingMode& uvw = stateProps.GetTextureAddressingMode();
						SetTextureAddressingMode(unit, uvw);

						SetTextureBorderColor(unit, stateProps.GetBorderColor());
					}
				}

				for(auto& entry : paramDesc->Buffers)
				{
					u32 binding = entry.second.Slot;
					SPtr<GpuBuffer> buffer = gpuParams->GetStorageBuffer(entry.second.Set, binding);

					GLGpuBuffer* glBuffer = static_cast<GLGpuBuffer*>(buffer.get());

					switch(entry.second.Type)
					{
					case GPOT_BYTE_BUFFER: // Texture buffer (read-only, unstructured)
						{
							u32 unit = getTexUnit(binding);
							if(!ActivateGlTextureUnit(unit))
								continue;

							GLuint texId = 0;
							if(glBuffer != nullptr)
								texId = glBuffer->GetGlTextureId();

							if(mTextureInfos[unit].Type != GL_TEXTURE_BUFFER)
							{
								glBindTexture(mTextureInfos[unit].Type, 0);
								B3D_CHECK_GL_ERROR();
							}

							mTextureInfos[unit].Type = GL_TEXTURE_BUFFER;

							glBindTexture(GL_TEXTURE_BUFFER, texId);
							B3D_CHECK_GL_ERROR();

							SPtr<GLSLGpuProgram> activeProgram = GetActiveProgram(type);
							if(activeProgram != nullptr)
							{
								GLuint glProgram = activeProgram->GetGlHandle();

								glProgramUniform1i(glProgram, binding, unit);
								B3D_CHECK_GL_ERROR();
							}
						}
						break;
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
					case GPOT_RWBYTE_BUFFER: // Storage buffer (read/write, unstructured)
						{
							u32 unit = getImageUnit(binding);

							GLuint texId = 0;
							GLuint format = GL_R32F;
							if(glBuffer != nullptr)
							{
								texId = glBuffer->GetGlTextureId();
								format = glBuffer->GetGlFormat();
							}

							glBindImageTexture(unit, texId, 0, false, 0, GL_READ_WRITE, format);
							B3D_CHECK_GL_ERROR();

							SPtr<GLSLGpuProgram> activeProgram = GetActiveProgram(type);
							if(activeProgram != nullptr)
							{
								GLuint glProgram = activeProgram->GetGlHandle();

								glProgramUniform1i(glProgram, binding, unit);
								B3D_CHECK_GL_ERROR();
							}
						}
						break;
#endif
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
					case GPOT_RWSTRUCTURED_BUFFER: // Shared storage block (read/write, structured)
						{
							u32 unit = getSharedStorageUnit(binding);

							GLuint bufferId = 0;
							if(glBuffer != nullptr)
								bufferId = glBuffer->GetGlBufferId();

							glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, bufferId);
							B3D_CHECK_GL_ERROR();

							SPtr<GLSLGpuProgram> activeProgram = GetActiveProgram(type);
							if(activeProgram != nullptr)
							{
								GLuint glProgram = activeProgram->GetGlHandle();

								glShaderStorageBlockBinding(glProgram, binding, unit);
								B3D_CHECK_GL_ERROR();
							}
						}
						break;
#endif
					default:
						break;
					}
				}

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
				for(auto& entry : paramDesc->LoadStoreTextures)
				{
					u32 binding = entry.second.Slot;

					SPtr<Texture> texture = gpuParams->GetStorageTexture(entry.second.Set, binding);
					const TextureSurface& surface = gpuParams->GetStorageTextureSurface(entry.second.Set, binding);

					u32 unit = getImageUnit(binding);
					GLuint texId = 0;
					u32 mipLevel = 0;
					u32 face = 0;
					bool bindAllLayers = false;
					GLenum format = GL_R32F;

					if(texture != nullptr)
					{
						GLTexture* tex = static_cast<GLTexture*>(texture.get());
						auto& texProps = tex->GetProperties();

						bindAllLayers = texProps.GetFaceCount() == surface.FaceCount || surface.FaceCount == 0;

						if(!bindAllLayers && surface.FaceCount > 1)
						{
							B3D_LOG(Warning, RenderBackend, "Attempting to bind multiple faces of a load-store texture."
														   "You are allowed to bind either a single face, or all the faces of the texture. Only "
														   "the first face will be bound instead.");
						}

						if(surface.MipLevelCount > 1)
						{
							B3D_LOG(Warning, RenderBackend, "Attempting to bind multiple mip levels of a load-store "
														   "texture. This is not supported and only the first provided level will be bound.");
						}

						texId = tex->GetGlid();
						format = tex->GetGlFormat();
						mipLevel = surface.MipLevel;
						face = surface.Face;
					}

					glBindImageTexture(unit, texId, mipLevel, bindAllLayers, face, GL_READ_WRITE, format);
					B3D_CHECK_GL_ERROR();

					SPtr<GLSLGpuProgram> activeProgram = GetActiveProgram(type);
					if(activeProgram != nullptr)
					{
						GLuint glProgram = activeProgram->GetGlHandle();

						glProgramUniform1i(glProgram, binding, unit);
						B3D_CHECK_GL_ERROR();
					}
				}
#endif

				for(auto& entry : paramDesc->ParamBlocks)
				{
					u32 binding = entry.second.Slot;
					SPtr<GpuBuffer> buffer = gpuParams->GetUniformBuffer(entry.second.Set, binding);

					if(buffer == nullptr)
						continue;

					buffer->FlushToGpu();

					SPtr<GLSLGpuProgram> activeProgram = GetActiveProgram(type);
					GLuint glProgram = activeProgram->GetGlHandle();

					// 0 means uniforms are not in block, in which case we handle it specially
					if(binding == 0)
					{
						u8* uniformBufferData = (u8*)B3DStackAllocate(buffer->GetSize());
						buffer->Read(0, uniformBufferData, buffer->GetSize());

						for(auto iter = paramDesc->Params.begin(); iter != paramDesc->Params.end(); ++iter)
						{
							const GpuDataParameterInformation& param = iter->second;

							if(param.ParamBlockSlot != 0) // 0 means uniforms are not in a block
								continue;

							const u8* ptrData = uniformBufferData + param.CpuMemOffset * sizeof(u32);

							// Note: We don't transpose matrices here even though we don't use column major format
							// because they are assumed to be pre-transposed in the GpuParams buffer
							switch(param.Type)
							{
							case GPDT_FLOAT1:
								glProgramUniform1fv(glProgram, param.GpuMemOffset, param.ArraySize, (GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_FLOAT2:
								glProgramUniform2fv(glProgram, param.GpuMemOffset, param.ArraySize, (GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_FLOAT3:
								glProgramUniform3fv(glProgram, param.GpuMemOffset, param.ArraySize, (GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_FLOAT4:
								glProgramUniform4fv(glProgram, param.GpuMemOffset, param.ArraySize, (GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_2X2:
								glProgramUniformMatrix2fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_2X3:
								glProgramUniformMatrix3x2fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_2X4:
								glProgramUniformMatrix4x2fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_3X2:
								glProgramUniformMatrix2x3fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_3X3:
								glProgramUniformMatrix3fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_3X4:
								glProgramUniformMatrix4x3fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_4X2:
								glProgramUniformMatrix2x4fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_4X3:
								glProgramUniformMatrix3x4fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_MATRIX_4X4:
								glProgramUniformMatrix4fv(
									glProgram,
									param.GpuMemOffset,
									param.ArraySize,
									GL_FALSE,
									(GLfloat*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_INT1:
								glProgramUniform1iv(glProgram, param.GpuMemOffset, param.ArraySize, (GLint*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_INT2:
								glProgramUniform2iv(glProgram, param.GpuMemOffset, param.ArraySize, (GLint*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_INT3:
								glProgramUniform3iv(glProgram, param.GpuMemOffset, param.ArraySize, (GLint*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_INT4:
								glProgramUniform4iv(glProgram, param.GpuMemOffset, param.ArraySize, (GLint*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							case GPDT_BOOL:
								glProgramUniform1uiv(glProgram, param.GpuMemOffset, param.ArraySize, (GLuint*)ptrData);
								B3D_CHECK_GL_ERROR();
								break;
							default:
							case GPDT_UNKNOWN:
								break;
							}
						}

						if(uniformBufferData != nullptr)
							B3DStackFree(uniformBufferData);
					}
					else
					{
						const GLGpuParamBlockBuffer* glParamBlockBuffer = static_cast<const GLGpuParamBlockBuffer*>(buffer.get());

						u32 unit = getUniformUnit(binding - 1);
						glUniformBlockBinding(glProgram, binding - 1, unit);
						B3D_CHECK_GL_ERROR();

						glBindBufferBase(GL_UNIFORM_BUFFER, unit, glParamBlockBuffer->GetGlBufferId());
						B3D_CHECK_GL_ERROR();
					}
				}
			}
		}
		B3DClearAllocatorFrame();

		ActivateGlTextureUnit(0);
	};

	auto execute = [=]()
	{ executeRef(gpuParams); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumGpuParamBinds);
}

void GLRenderAPI::SetStencilRef(u32 stencilRefValue, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 stencilRefValue)
	{
		THROW_IF_NOT_CORE_THREAD;

		SetStencilRefValue(stencilRefValue);
	};

	auto execute = [=]()
	{ executeRef(stencilRefValue); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::SetViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const Rect2& area)
	{
		THROW_IF_NOT_CORE_THREAD;

		mViewportNorm = area;
		ApplyViewport();
	};

	auto execute = [=]()
	{ executeRef(area); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<RenderTarget>& target, u32 readOnlyFlags)
	{
		THROW_IF_NOT_CORE_THREAD;

		// Switch context if different from current one
		if(target != nullptr && target->GetProperties().IsWindow)
		{
			RenderWindow* window = static_cast<RenderWindow*>(target.get());

			SPtr<GLContext> newContext;
			target->GetCustomAttribute("GLCONTEXT", &newContext);
			if(newContext && mCurrentContext != newContext)
				SwitchContext(newContext, *window);
			else
				mCurrentContext->SetCurrent(*window);
		}

		// This must happen after context switch to ensure previous context is still alive
		mActiveRenderTarget = target;
		mActiveRenderTargetModified = false;

		GLFrameBufferObject* fbo = nullptr;

		if(target != nullptr)
			target->GetCustomAttribute("FBO", &fbo);

		if(fbo != nullptr)
		{
			fbo->Bind();

			// Enable / disable sRGB states
			if(target->GetProperties().HwGamma)
			{
				glEnable(GL_FRAMEBUFFER_SRGB);
				B3D_CHECK_GL_ERROR();
			}
			else
			{
				glDisable(GL_FRAMEBUFFER_SRGB);
				B3D_CHECK_GL_ERROR();
			}
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			B3D_CHECK_GL_ERROR();
		}

		ApplyViewport();
	};

	auto execute = [=]()
	{ executeRef(target, readOnlyFlags); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumRenderTargetChanges);
}

void GLRenderAPI::SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer)
{
#if B3D_DEBUG
	u32 lastIdx = index + numBuffers;
	if(lastIdx > kMaxVbCount)
	{
		B3D_LOG(Error, RenderBackend, "Provided vertex buffer slot range is invalid: {0} to {1}.", index, index + numBuffers);
		return;
	}
#endif

	auto executeRef = [&](u32 index, const SmallVector<SPtr<VertexBuffer>, 8>& buffers, u32 numBuffers)
	{
		THROW_IF_NOT_CORE_THREAD;

		for(u32 i = 0; i < numBuffers; i++)
			mBoundVertexBuffers[index + i] = buffers[i];
	};

	SmallVector<SPtr<VertexBuffer>, 8> _buffers;
	for(u32 i = 0; i < numBuffers; i++)
		_buffers.Add(buffers[i]);

	auto execute = [executeRef, index, buffers = std::move(_buffers), numBuffers]()
	{ executeRef(index, buffers, numBuffers); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::SetVertexDescription(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<VertexDeclaration>& vertexDeclaration)
	{
		THROW_IF_NOT_CORE_THREAD;

		mBoundVertexDeclaration = vertexDeclaration;
	};

	auto execute = [=]()
	{ executeRef(vertexDeclaration); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](DrawOperationType op)
	{
		THROW_IF_NOT_CORE_THREAD;

		mCurrentDrawOperation = op;
	};

	auto execute = [=]()
	{ executeRef(op); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<IndexBuffer>& buffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		mBoundIndexBuffer = buffer;
	};

	auto execute = [=]()
	{ executeRef(buffer); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 vertexOffset, u32 vertexCount, u32 instanceCount)
	{
		THROW_IF_NOT_CORE_THREAD;

		// Find the correct type to render
		GLint primType = GetGlDrawMode();
		BeginDraw();

		if(instanceCount <= 1)
		{
			glDrawArrays(primType, vertexOffset, vertexCount);
			B3D_CHECK_GL_ERROR();
		}
		else
		{
			glDrawArraysInstanced(primType, vertexOffset, vertexCount, instanceCount);
			B3D_CHECK_GL_ERROR();
		}

		EndDraw();
	};

	auto execute = [=]()
	{ executeRef(vertexOffset, vertexCount, instanceCount); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	u32 primCount = VertexCountToPrimCount(mCurrentDrawOperation, vertexCount);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);
}

void GLRenderAPI::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount,
						  u32 instanceCount)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(mBoundIndexBuffer == nullptr)
		{
			B3D_LOG(Warning, RenderBackend, "Cannot draw indexed because index buffer is not set.");
			return;
		}

		// Find the correct type to render
		GLint primType = GetGlDrawMode();

		BeginDraw();

		SPtr<GLIndexBuffer> indexBuffer = std::static_pointer_cast<GLIndexBuffer>(mBoundIndexBuffer);
		const IndexBufferProperties& ibProps = indexBuffer->GetProperties();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->GetGlBufferId());
		B3D_CHECK_GL_ERROR();

		GLenum indexType = (ibProps.GetType() == IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

		if(instanceCount <= 1)
		{
			glDrawElementsBaseVertex(
				primType,
				indexCount,
				indexType,
				(GLvoid*)(u64)(ibProps.GetIndexSize() * startIndex),
				vertexOffset);
			B3D_CHECK_GL_ERROR();
		}
		else
		{
			glDrawElementsInstancedBaseVertex(
				primType,
				indexCount,
				indexType,
				(GLvoid*)(u64)(ibProps.GetIndexSize() * startIndex),
				instanceCount,
				vertexOffset);
			B3D_CHECK_GL_ERROR();
		}

		EndDraw();
	};

	auto execute = [=]()
	{ executeRef(startIndex, indexCount, vertexOffset, vertexCount, instanceCount); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	u32 primCount = VertexCountToPrimCount(mCurrentDrawOperation, vertexCount);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);

	B3D_INCREMENT_RENDER_STATISTIC(NumIndexBufferBinds);
}

void GLRenderAPI::DispatchCompute(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(mCurrentComputeProgram == nullptr)
		{
			B3D_LOG(Warning, RenderBackend, "Cannot dispatch compute without a set compute program.");
			return;
		}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		glUseProgram(mCurrentComputeProgram->GetGlHandle());
		B3D_CHECK_GL_ERROR();

		glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
		B3D_CHECK_GL_ERROR();

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
#else
		B3D_LOG(Warning, RenderBackend, "Compute shaders not supported on current OpenGL version.");
#endif
	};

	auto execute = [=]()
	{ executeRef(numGroupsX, numGroupsY, numGroupsZ); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumComputeCalls);
}

void GLRenderAPI::SetScissorRect(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 left, u32 top, u32 right, u32 bottom)
	{
		THROW_IF_NOT_CORE_THREAD;

		mScissorTop = top;
		mScissorBottom = bottom;
		mScissorLeft = left;
		mScissorRight = right;

		if(mScissorEnabled)
			mScissorRectDirty = true;
	};

	auto execute = [=]()
	{ executeRef(left, top, right, bottom); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
	{
		if(mActiveRenderTarget == nullptr)
			return;

		const RenderTargetProperties& rtProps = mActiveRenderTarget->GetProperties();
		Rect2I clearRect(0, 0, rtProps.Width, rtProps.Height);

		ClearArea(buffers, color, depth, stencil, clearRect, targetMask);
	};

	auto execute = [=]()
	{ executeRef(buffers, color, depth, stencil, targetMask); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
	{
		Rect2I clearRect(mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight);

		ClearArea(buffers, color, depth, stencil, clearRect, targetMask);
	};

	auto execute = [=]()
	{ executeRef(buffers, color, depth, stencil, targetMask); };

	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void GLRenderAPI::SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	// Switch context if different from current one
	if(!target->GetProperties().IsWindow)
		return;

	SubmitCommandBuffer(mMainCommandBuffer, syncMask);

	RenderWindow* window = static_cast<RenderWindow*>(target.get());

	SPtr<GLContext> newContext;
	target->GetCustomAttribute("GLCONTEXT", &newContext);
	if(newContext && mCurrentContext != newContext)
		SwitchContext(newContext, *window);
	else
		mCurrentContext->SetCurrent(*window);

	target->SwapBuffers();

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

void GLRenderAPI::AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
{
	// We're not supporting this as we don't support command buffer command queuing at all (i.e. they are executed
	// straight away).
	B3D_LOG(Error, RenderBackend, "Secondary command buffers not supported on OpenGL.");
}

void GLRenderAPI::SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask)
{
	SPtr<GLCommandBuffer> cb = GetCb(commandBuffer);
	cb->ExecuteCommands();

	if(cb == mMainCommandBuffer)
		mMainCommandBuffer = std::static_pointer_cast<GLCommandBuffer>(CommandBuffer::Create(GQT_GRAPHICS));
}

SPtr<CommandBuffer> GLRenderAPI::GetMainCommandBuffer() const
{
	return mMainCommandBuffer;
}

SPtr<GLCommandBuffer> GLRenderAPI::GetCb(const SPtr<CommandBuffer>& buffer)
{
	if(buffer != nullptr)
		return std::static_pointer_cast<GLCommandBuffer>(buffer);

	return std::static_pointer_cast<GLCommandBuffer>(mMainCommandBuffer);
}

void GLRenderAPI::ClearArea(u32 buffers, const Color& color, float depth, u16 stencil, const Rect2I& clearRect, u8 targetMask)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mActiveRenderTarget == nullptr)
		return;

	u32 numColorBuffers = 1;
	bool colorMasks[B3D_MAXIMUM_RENDER_TARGET_COUNT] = { 0 };
	if(!mActiveRenderTarget->GetProperties().IsWindow)
	{
		RenderTexture* renderTexture = static_cast<RenderTexture*>(mActiveRenderTarget.get());

		for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		{
			SPtr<Texture> texture = renderTexture->GetColorTexture(i);
			if(texture)
				colorMasks[i] = !mColorWrite[i][0] || !mColorWrite[i][1] || !mColorWrite[i][2] || !mColorWrite[i][3];
		}
	}
	else
		colorMasks[0] = !mColorWrite[0][0] || !mColorWrite[0][1] || !mColorWrite[0][2] || !mColorWrite[0][3];

	// Disable scissor test as we want to clear the entire render surface
	GLboolean scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
	u32 oldScissorTop = mScissorTop;
	u32 oldScissorBottom = mScissorBottom;
	u32 oldScissorLeft = mScissorLeft;
	u32 oldScissorRight = mScissorRight;

	if(scissorTestEnabled)
	{
		glDisable(GL_SCISSOR_TEST);
		B3D_CHECK_GL_ERROR();
	}

	const RenderTargetProperties& rtProps = mActiveRenderTarget->GetProperties();

	bool clearEntireTarget = clearRect.Width == 0 || clearRect.Height == 0;
	clearEntireTarget |= (clearRect.X == 0 && clearRect.Y == 0 && clearRect.Width == rtProps.Width && clearRect.Height == rtProps.Height);

	if(!clearEntireTarget)
	{
		SetScissorRect(clearRect.X, clearRect.Y, clearRect.X + clearRect.Width, clearRect.Y + clearRect.Height);
		SetScissorTestEnable(true);
	}

	if(buffers & FBT_COLOR)
	{
		// Enable buffer for writing if it isn't
		for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		{
			if(colorMasks[i])
			{
				glColorMaski(i, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				B3D_CHECK_GL_ERROR();
			}
		}
	}
	if(buffers & FBT_DEPTH)
	{
		// Enable buffer for writing if it isn't
		if(!mDepthWrite)
		{
			glDepthMask(GL_TRUE);
			B3D_CHECK_GL_ERROR();
		}
	}
	if(buffers & FBT_STENCIL)
	{
		// Enable buffer for writing if it isn't
		glStencilMask(0xFFFFFFFF);
		B3D_CHECK_GL_ERROR();
	}

	if(targetMask == 0xFF)
	{
		GLbitfield flags = 0;
		if(buffers & FBT_COLOR)
		{
			flags |= GL_COLOR_BUFFER_BIT;

			glClearColor(color.R, color.G, color.B, color.A);
			B3D_CHECK_GL_ERROR();
		}

		if(buffers & FBT_DEPTH)
		{
			flags |= GL_DEPTH_BUFFER_BIT;

			glClearDepth(depth);
			B3D_CHECK_GL_ERROR();
		}

		if(buffers & FBT_STENCIL)
		{
			flags |= GL_STENCIL_BUFFER_BIT;

			glClearStencil(stencil);
			B3D_CHECK_GL_ERROR();
		}

		// Clear buffers
		glClear(flags);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		GLFrameBufferObject* fbo = nullptr;
		mActiveRenderTarget->GetCustomAttribute("FBO", &fbo);

		if(buffers & FBT_COLOR)
		{
			for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
			{
				if(fbo->HasColorBuffer(i) && ((1 << i) & targetMask) != 0)
				{
					glClearBufferfv(GL_COLOR, i, (GLfloat*)&color);
					B3D_CHECK_GL_ERROR();
				}
			}
		}

		if(buffers & FBT_DEPTH)
		{
			if(buffers & FBT_STENCIL)
			{
				glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
				B3D_CHECK_GL_ERROR();
			}
			else
			{
				glClearBufferfv(GL_DEPTH, 0, &depth);
				B3D_CHECK_GL_ERROR();
			}
		}
		else if(buffers & FBT_STENCIL)
		{
			i32 stencilClear = (i32)stencil;
			glClearBufferiv(GL_STENCIL, 0, &stencilClear);
			B3D_CHECK_GL_ERROR();
		}
	}

	if(!clearEntireTarget)
	{
		SetScissorTestEnable(false);
	}

	// Restore scissor test
	if(scissorTestEnabled)
	{
		glEnable(GL_SCISSOR_TEST);
		B3D_CHECK_GL_ERROR();

		mScissorTop = oldScissorTop;
		mScissorBottom = oldScissorBottom;
		mScissorLeft = oldScissorLeft;
		mScissorRight = oldScissorRight;
	}

	// Reset buffer write state
	if(!mDepthWrite && (buffers & FBT_DEPTH))
	{
		glDepthMask(GL_FALSE);
		B3D_CHECK_GL_ERROR();
	}

	if((buffers & FBT_COLOR))
	{
		for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		{
			if(colorMasks[i])
			{
				glColorMaski(i, mColorWrite[i][0], mColorWrite[i][1], mColorWrite[i][2], mColorWrite[i][3]);
				B3D_CHECK_GL_ERROR();
			}
		}
	}

	if(buffers & FBT_STENCIL)
	{
		glStencilMask(mStencilWriteMask);
		B3D_CHECK_GL_ERROR();
	}

	NotifyRenderTargetModified();
	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

/************************************************************************/
/* 								PRIVATE		                     		*/
/************************************************************************/

void GLRenderAPI::SetTextureAddressingMode(u16 unit, const UVWAddressingMode& uvw)
{
	glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_WRAP_S, GetTextureAddressingMode(uvw.U));
	B3D_CHECK_GL_ERROR();

	glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_WRAP_T, GetTextureAddressingMode(uvw.V));
	B3D_CHECK_GL_ERROR();

	glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_WRAP_R, GetTextureAddressingMode(uvw.W));
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetTextureBorderColor(u16 unit, const Color& color)
{
	GLfloat border[4] = { color.R, color.G, color.B, color.A };
	glTexParameterfv(mTextureInfos[unit].Type, GL_TEXTURE_BORDER_COLOR, border);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetTextureMipmapBias(u16 unit, float bias)
{
	glTexParameterf(mTextureInfos[unit].Type, GL_TEXTURE_LOD_BIAS, bias);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetTextureMipmapRange(u16 unit, float min, float max)
{
	glTexParameterf(mTextureInfos[unit].Type, GL_TEXTURE_MIN_LOD, min);
	B3D_CHECK_GL_ERROR();

	glTexParameterf(mTextureInfos[unit].Type, GL_TEXTURE_MAX_LOD, max);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetSceneBlending(u32 target, BlendFactor sourceFactor, BlendFactor destFactor, BlendOperation op)
{
	GLint sourceBlend = GetBlendMode(sourceFactor);
	GLint destBlend = GetBlendMode(destFactor);
	if(sourceFactor == BF_ONE && destFactor == BF_ZERO)
	{
		glDisablei(GL_BLEND, target);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glEnablei(GL_BLEND, target);
		B3D_CHECK_GL_ERROR();

		glBlendFunci(target, sourceBlend, destBlend);
		B3D_CHECK_GL_ERROR();
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

	glBlendEquationi(target, func);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetSceneBlending(u32 target, BlendFactor sourceFactor, BlendFactor destFactor, BlendFactor sourceFactorAlpha, BlendFactor destFactorAlpha, BlendOperation op, BlendOperation alphaOp)
{
	GLint sourceBlend = GetBlendMode(sourceFactor);
	GLint destBlend = GetBlendMode(destFactor);
	GLint sourceBlendAlpha = GetBlendMode(sourceFactorAlpha);
	GLint destBlendAlpha = GetBlendMode(destFactorAlpha);

	if(sourceFactor == BF_ONE && destFactor == BF_ZERO && sourceFactorAlpha == BF_ONE && destFactorAlpha == BF_ZERO)
	{
		glDisablei(GL_BLEND, target);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glEnablei(GL_BLEND, target);
		B3D_CHECK_GL_ERROR();

		glBlendFuncSeparatei(target, sourceBlend, destBlend, sourceBlendAlpha, destBlendAlpha);
		B3D_CHECK_GL_ERROR();
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

	glBlendEquationSeparatei(target, func, alphaFunc);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetAlphaToCoverage(bool enable)
{
	static bool lasta2c = false;

	if(enable != lasta2c)
	{
		if(enable)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			B3D_CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			B3D_CHECK_GL_ERROR();
		}

		lasta2c = enable;
	}
}

void GLRenderAPI::SetScissorTestEnable(bool enable)
{
	if(mActiveRenderTarget == nullptr)
		return;

	const RenderTargetProperties& rtProps = mActiveRenderTarget->GetProperties();
	GLsizei x, y, w, h;
	if(enable)
	{
		glEnable(GL_SCISSOR_TEST);
		B3D_CHECK_GL_ERROR();

		x = mScissorLeft;
		y = rtProps.Height - mScissorBottom;
		w = mScissorRight - mScissorLeft;
		h = mScissorBottom - mScissorTop;

		glScissor(x, y, w, h);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
		B3D_CHECK_GL_ERROR();

		// GL requires you to reset the scissor when disabling
		x = mViewportLeft;
		y = rtProps.Height - (mViewportTop + mViewportHeight);
		w = mViewportWidth;
		h = mViewportHeight;

		glScissor(x, y, w, h);
		B3D_CHECK_GL_ERROR();
	}

	mScissorEnabled = enable;
	mScissorRectDirty = false;
}

void GLRenderAPI::SetMultisamplingEnable(bool enable)
{
	if(enable)
	{
		glEnable(GL_MULTISAMPLE);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetDepthClipEnable(bool enable)
{
	if(!enable) // If clipping disabled, clamp is enabled
	{
		glEnable(GL_DEPTH_CLAMP);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_DEPTH_CLAMP);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetAntialiasedLineEnable(bool enable)
{
	if(enable)
	{
		glEnable(GL_LINE_SMOOTH);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetCullingMode(CullingMode mode)
{
	GLenum cullMode;

	switch(mode)
	{
	case CULL_NONE:
		glDisable(GL_CULL_FACE);
		B3D_CHECK_GL_ERROR();
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
	B3D_CHECK_GL_ERROR();

	glCullFace(cullMode);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetDepthBufferCheckEnabled(bool enabled)
{
	if(enabled)
	{
		glClearDepth(1.0f);
		B3D_CHECK_GL_ERROR();

		glEnable(GL_DEPTH_TEST);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetDepthBufferWriteEnabled(bool enabled)
{
	GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
	glDepthMask(flag);
	B3D_CHECK_GL_ERROR();

	mDepthWrite = enabled;
}

void GLRenderAPI::SetDepthBufferFunction(CompareFunction func)
{
	glDepthFunc(ConvertCompareFunction(func));
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetDepthBias(float constantBias, float slopeScaleBias)
{
	if(constantBias != 0 || slopeScaleBias != 0)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		B3D_CHECK_GL_ERROR();

		glEnable(GL_POLYGON_OFFSET_POINT);
		B3D_CHECK_GL_ERROR();

		glEnable(GL_POLYGON_OFFSET_LINE);
		B3D_CHECK_GL_ERROR();

		float scaledConstantBias = -constantBias * float((1 << 24) - 1); // Note: Assumes 24-bit depth buffer
		glPolygonOffset(slopeScaleBias, scaledConstantBias);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		B3D_CHECK_GL_ERROR();

		glDisable(GL_POLYGON_OFFSET_POINT);
		B3D_CHECK_GL_ERROR();

		glDisable(GL_POLYGON_OFFSET_LINE);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetColorBufferWriteEnabled(u32 target, bool red, bool green, bool blue, bool alpha)
{
	glColorMaski(target, red, green, blue, alpha);
	B3D_CHECK_GL_ERROR();

	if(target < B3D_MAXIMUM_RENDER_TARGET_COUNT)
	{
		mColorWrite[target][0] = red;
		mColorWrite[target][1] = blue;
		mColorWrite[target][2] = green;
		mColorWrite[target][3] = alpha;
	}
}

void GLRenderAPI::SetPolygonMode(PolygonMode level)
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
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetStencilCheckEnabled(bool enabled)
{
	if(enabled)
	{
		glEnable(GL_STENCIL_TEST);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_STENCIL_TEST);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetStencilBufferOperations(StencilOperation stencilFailOp, StencilOperation depthFailOp, StencilOperation passOp, bool front)
{
	if(front)
	{
		glStencilOpSeparate(
			GL_FRONT,
			ConvertStencilOp(stencilFailOp),
			ConvertStencilOp(depthFailOp),
			ConvertStencilOp(passOp));
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glStencilOpSeparate(
			GL_BACK,
			ConvertStencilOp(stencilFailOp),
			ConvertStencilOp(depthFailOp),
			ConvertStencilOp(passOp));
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetStencilBufferFunc(CompareFunction func, u32 mask, bool front)
{
	mStencilReadMask = mask;

	if(front)
	{
		mStencilCompareFront = func;
		glStencilFuncSeparate(GL_FRONT, ConvertCompareFunction(mStencilCompareFront), mStencilRefValue, mStencilReadMask);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		mStencilCompareBack = func;
		glStencilFuncSeparate(GL_BACK, ConvertCompareFunction(mStencilCompareBack), mStencilRefValue, mStencilReadMask);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::SetStencilBufferWriteMask(u32 mask)
{
	mStencilWriteMask = mask;

	glStencilMask(mask);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetStencilRefValue(u32 refValue)
{
	THROW_IF_NOT_CORE_THREAD;

	mStencilRefValue = refValue;

	glStencilFuncSeparate(GL_FRONT, ConvertCompareFunction(mStencilCompareFront), mStencilRefValue, mStencilReadMask);
	B3D_CHECK_GL_ERROR();

	glStencilFuncSeparate(GL_BACK, ConvertCompareFunction(mStencilCompareBack), mStencilRefValue, mStencilReadMask);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetTextureFiltering(u16 unit, FilterType ftype, FilterOptions fo)
{
	switch(ftype)
	{
	case FT_MIN:
		mMinFilter = fo;
		// Combine with existing mip filter
		glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_MIN_FILTER, GetCombinedMinMipFilter());
		B3D_CHECK_GL_ERROR();
		break;
	case FT_MAG:
		switch(fo)
		{
		case FO_ANISOTROPIC: // GL treats linear and aniso the same
		case FO_LINEAR:
			glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			B3D_CHECK_GL_ERROR();
			break;
		case FO_POINT:
		case FO_NONE:
			glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			B3D_CHECK_GL_ERROR();
			break;
		default:
			break;
		}
		break;
	case FT_MIP:
		mMipFilter = fo;
		// Combine with existing min filter
		glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_MIN_FILTER, GetCombinedMinMipFilter());
		B3D_CHECK_GL_ERROR();
		break;
	}
}

void GLRenderAPI::SetTextureAnisotropy(u16 unit, u32 maxAnisotropy)
{
	GLfloat maxSupportAnisotropy = 0;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupportAnisotropy);
	B3D_CHECK_GL_ERROR();

	if(maxAnisotropy > maxSupportAnisotropy)
		maxAnisotropy = maxSupportAnisotropy ? static_cast<u32>(maxSupportAnisotropy) : 1;

	if(maxAnisotropy < 1)
		maxAnisotropy = 1;

	glTexParameterf(mTextureInfos[unit].Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)maxAnisotropy);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::SetTextureCompareMode(u16 unit, CompareFunction compare)
{
	if(compare == CMPF_ALWAYS_PASS)
	{
		glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		B3D_CHECK_GL_ERROR();
	}
	else
	{
		glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		B3D_CHECK_GL_ERROR();

		glTexParameteri(mTextureInfos[unit].Type, GL_TEXTURE_COMPARE_FUNC, ConvertCompareFunction(compare));
		B3D_CHECK_GL_ERROR();
	}
}

bool GLRenderAPI::ActivateGlTextureUnit(u16 unit)
{
	if(mActiveTextureUnit != unit)
	{
		if(unit < GetCapabilities(0).NumCombinedTextureUnits)
		{
			glActiveTexture(GL_TEXTURE0 + unit);
			B3D_CHECK_GL_ERROR();

			mActiveTextureUnit = unit;
			return true;
		}
		else if(!unit)
		{
			// Always ok to use the first unit
			return true;
		}
		else
		{
			B3D_LOG(Warning, RenderBackend, "Provided texture unit index is higher than OpenGL supports. Provided: {0}. "
										   "Supported range: 0 .. {1}",
				   unit, GetCapabilities(0).NumCombinedTextureUnits - 1);
			return false;
		}
	}
	else
	{
		return true;
	}
}

void GLRenderAPI::BeginDraw()
{
	if(mDrawCallInProgress)
		B3D_EXCEPT(InternalErrorException, "Calling beginDraw without finishing previous draw call. Please call endDraw().");

	mDrawCallInProgress = true;

	if(mCurrentVertexProgram == nullptr)
	{
		B3D_LOG(Warning, RenderBackend, "Cannot render without a set vertex shader.");
		return;
	}

	if(mBoundVertexDeclaration == nullptr)
	{
		B3D_LOG(Warning, RenderBackend, "Cannot render without a set vertex declaration.");
		return;
	}

	if(mScissorRectDirty)
	{
		B3D_ASSERT(mScissorEnabled);
		SetScissorTestEnable(true);
	}

	const GLSLProgramPipeline* pipeline = mProgramPipelineManager->GetPipeline(mCurrentVertexProgram.get(), mCurrentFragmentProgram.get(), mCurrentGeometryProgram.get(), mCurrentHullProgram.get(), mCurrentDomainProgram.get());

	glUseProgram(0);
	B3D_CHECK_GL_ERROR();

	if(mActivePipeline != pipeline)
	{
		glBindProgramPipeline(pipeline->GlHandle);
		B3D_CHECK_GL_ERROR();

		mActivePipeline = pipeline;
	}

	const GLVertexArrayObject& vao = GLVertexArrayObjectManager::Instance().GetVao(
		mCurrentVertexProgram,
		mBoundVertexDeclaration,
		mBoundVertexBuffers);

	glBindVertexArray(vao.GetGlHandle());
	B3D_CHECK_GL_ERROR();

	B3D_INCREMENT_RENDER_STATISTIC(NumVertexBufferBinds);
}

void GLRenderAPI::EndDraw()
{
	if(!mDrawCallInProgress)
		return;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
#endif

	NotifyRenderTargetModified();
	mDrawCallInProgress = false;
}

GLfloat GLRenderAPI::GetCurrentAnisotropy(u16 unit)
{
	GLfloat curAniso = 0;
	glGetTexParameterfv(mTextureInfos[unit].Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, &curAniso);
	B3D_CHECK_GL_ERROR();

	return curAniso ? curAniso : 1;
}

GLint GLRenderAPI::ConvertStencilOp(StencilOperation op) const
{
	switch(op)
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

GLint GLRenderAPI::ConvertCompareFunction(CompareFunction func) const
{
	switch(func)
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

GLuint GLRenderAPI::GetCombinedMinMipFilter() const
{
	switch(mMinFilter)
	{
	case FO_ANISOTROPIC:
	case FO_LINEAR:
		switch(mMipFilter)
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
		switch(mMipFilter)
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

GLint GLRenderAPI::GetBlendMode(BlendFactor blendMode) const
{
	switch(blendMode)
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

GLint GLRenderAPI::GetTextureAddressingMode(TextureAddressingMode tam) const
{
	switch(tam)
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

GLint GLRenderAPI::GetGlDrawMode() const
{
	GLint primType;

	// Use adjacency if there is a geometry program and it requested adjacency info
	bool useAdjacency = (mCurrentGeometryProgram != nullptr && mCurrentGeometryProgram->IsAdjacencyInfoRequired());
	switch(mCurrentDrawOperation)
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

SPtr<GLSLGpuProgram> GLRenderAPI::GetActiveProgram(GpuProgramType gptype) const
{
	switch(gptype)
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
		B3D_EXCEPT(InvalidParametersException, "Unsupported gpu program type: " + ToString(gptype));
	}

	return nullptr;
}

void GLRenderAPI::InitFromCaps(GpuDeviceCapabilities* caps)
{
	if(caps->RenderApiName != GetName())
	{
		B3D_EXCEPT(InvalidParametersException, "Trying to initialize GLRenderAPI from RenderSystemCapabilities that do not support OpenGL");
	}

#if B3D_DEBUG && (BS_OPENGL_4_3 || BS_OPENGLES_3_2)
	if(mGLSupport->CheckExtension("GL_ARB_debug_output"))
	{
		glDebugMessageCallback(&OpenGlErrorCallback, 0);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif

	bs::HardwareBufferManager::StartUp();
	HardwareBufferManager::StartUp<GLHardwareBufferManager>();

	// GPU Program Manager setup
	mGLSLProgramFactory = B3DNew<GLSLProgramFactory>();
	if(caps->IsShaderProfileSupported("glsl")) // Check for most recent GLSL support
		GpuProgramManager::Instance().AddFactory("glsl", mGLSLProgramFactory);

	if(caps->IsShaderProfileSupported("glsl4_1")) // Check for OpenGL 4.1 compatible version
		GpuProgramManager::Instance().AddFactory("glsl4_1", mGLSLProgramFactory);

	GLRTTManager::StartUp<GLRTTManager>();

	mNumTextureUnits = caps->NumCombinedTextureUnits;
	mTextureInfos = B3DNewMultiple<TextureInfo>(mNumTextureUnits);

	bs::TextureManager::StartUp<bs::GLTextureManager>(std::ref(*mGLSupport));
	TextureManager::StartUp<GLTextureManager>(std::ref(*mGLSupport));
}

void GLRenderAPI::SwitchContext(const SPtr<GLContext>& context, const RenderWindow& window)
{
	// Unbind pipeline and rebind to new context later
	SetGraphicsPipeline(nullptr);

	if(mCurrentContext)
		mCurrentContext->EndCurrent();

	mCurrentContext = context;
	mCurrentContext->SetCurrent(window);

	// Must reset depth/colour write mask to according with user desired, otherwise, clearFrameBuffer would be wrong
	// because the value we recorded may be different from the real state stored in GL context.
	glDepthMask(mDepthWrite);
	B3D_CHECK_GL_ERROR();

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		glColorMask(mColorWrite[i][0], mColorWrite[i][1], mColorWrite[i][2], mColorWrite[i][3]);
	B3D_CHECK_GL_ERROR();

	glStencilMask(mStencilWriteMask);
	B3D_CHECK_GL_ERROR();
}

void GLRenderAPI::InitCapabilities(GpuDeviceCapabilities& caps) const
{
	Vector<String> tokens = StringUtil::Split(mGLSupport->GetGlVersion(), ".");

	DriverVersion driverVersion;
	if(!tokens.empty())
	{
		driverVersion.Major = Parsei32(tokens[0]);
		if(tokens.size() > 1)
			driverVersion.Minor = Parsei32(tokens[1]);
		if(tokens.size() > 2)
			driverVersion.Release = Parsei32(tokens[2]);
	}
	driverVersion.Build = 0;

	caps.DriverVersion = driverVersion;
	caps.RenderApiName = GetName();

	const char* deviceName = (const char*)glGetString(GL_RENDERER);
	caps.DeviceName = deviceName;

	const char* vendorName = (const char*)glGetString(GL_VENDOR);
	if(strstr(vendorName, "NVIDIA"))
		caps.DeviceVendor = GPU_NVIDIA;
	else if(strstr(vendorName, "ATI"))
		caps.DeviceVendor = GPU_AMD;
	else if(strstr(vendorName, "AMD"))
		caps.DeviceVendor = GPU_AMD;
	else if(strstr(vendorName, "Intel"))
		caps.DeviceVendor = GPU_INTEL;
	else
		caps.DeviceVendor = GPU_UNKNOWN;

#if BS_OPENGL_4_1
	caps.AddShaderProfile("glsl4_1");
#endif

#if BS_OPENGL_4_5
	caps.AddShaderProfile("glsl");
#endif

	caps.SetCapability(RSC_TEXTURE_COMPRESSION_BC);

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
	caps.SetCapability(RSC_GEOMETRY_PROGRAM);
#endif

#if BS_OPENGL_4_2 || BS_OPENGLES_3_2
	caps.SetCapability(RSC_LOAD_STORE);
	caps.SetCapability(RSC_LOAD_STORE_MSAA);
#endif

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
	caps.SetCapability(RSC_TEXTURE_VIEWS);
#endif

#if B3D_PLATFORM != B3D_PLATFORM_ID_MACOS
	caps.SetCapability(RSC_RENDER_TARGET_LAYERS);
#endif

	caps.Conventions.UvYAxis = Conventions::Axis::Up;
	caps.Conventions.MatrixOrder = Conventions::MatrixOrder::ColumnMajor;
	caps.MinDepth = -1.0f;
	caps.MaxDepth = 1.0f;

	GLint maxOutputVertices;

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxOutputVertices);
	B3D_CHECK_GL_ERROR();
#else
	maxOutputVertices = 0;
#endif

	caps.GeometryProgramNumOutputVertices = maxOutputVertices;

	// Max number of fragment shader textures
	GLint units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units);
	B3D_CHECK_GL_ERROR();

	caps.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = static_cast<u16>(units);

	// Max number of vertex shader textures
	GLint vUnits;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vUnits);
	B3D_CHECK_GL_ERROR();

	caps.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = static_cast<u16>(vUnits);

	GLint numUniformBlocks;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &numUniformBlocks);
	B3D_CHECK_GL_ERROR();

	caps.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] = numUniformBlocks;

	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &numUniformBlocks);
	B3D_CHECK_GL_ERROR();

	caps.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] = numUniformBlocks;

	{
		GLint geomUnits;

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &geomUnits);
		B3D_CHECK_GL_ERROR();
#else
		geomUnits = 0;
#endif

		caps.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = static_cast<u16>(geomUnits);

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &numUniformBlocks);
		B3D_CHECK_GL_ERROR();
#else
		numUniformBlocks = 0;
#endif

		caps.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] = numUniformBlocks;
	}

	if(mGLSupport->CheckExtension("GL_ARB_tessellation_shader"))
	{
#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		caps.SetCapability(RSC_TESSELLATION_PROGRAM);
#endif

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		glGetIntegerv(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, &numUniformBlocks);
		B3D_CHECK_GL_ERROR();
#else
		numUniformBlocks = 0;
#endif

		caps.NumGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] = numUniformBlocks;

#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, &numUniformBlocks);
		B3D_CHECK_GL_ERROR();
#else
		numUniformBlocks = 0;
#endif

		caps.NumGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] = numUniformBlocks;
	}

	if(mGLSupport->CheckExtension("GL_ARB_compute_shader"))
	{
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		caps.SetCapability(RSC_COMPUTE_PROGRAM);
#endif

		GLint computeUnits;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &computeUnits);
		B3D_CHECK_GL_ERROR();
#else
		computeUnits = 0;
#endif

		caps.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = static_cast<u16>(computeUnits);

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &numUniformBlocks);
		B3D_CHECK_GL_ERROR();
#else
		numUniformBlocks = 0;
#endif

		caps.NumGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM] = numUniformBlocks;

		// Max number of load-store textures
		GLint lsfUnits;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
		glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &lsfUnits);
		B3D_CHECK_GL_ERROR();
#else
		lsfUnits = 0;
#endif

		caps.NumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = static_cast<u16>(lsfUnits);

		GLint lscUnits;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &lscUnits);
		B3D_CHECK_GL_ERROR();
#else
		lscUnits = 0;
#endif

		caps.NumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = static_cast<u16>(lscUnits);

		GLint combinedLoadStoreTextureUnits;

#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
		glGetIntegerv(GL_MAX_IMAGE_UNITS, &combinedLoadStoreTextureUnits);
		B3D_CHECK_GL_ERROR();
#else
		combinedLoadStoreTextureUnits = 0;
#endif

		caps.NumCombinedLoadStoreTextureUnits = static_cast<u16>(combinedLoadStoreTextureUnits);
	}

	GLint combinedTexUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combinedTexUnits);
	B3D_CHECK_GL_ERROR();

	caps.NumCombinedTextureUnits = static_cast<u16>(combinedTexUnits);

	GLint combinedUniformBlockUnits;
	glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &combinedUniformBlockUnits);
	B3D_CHECK_GL_ERROR();

	caps.NumCombinedParamBlockBuffers = static_cast<u16>(combinedUniformBlockUnits);
	caps.NumMultiRenderTargets = 8;
}

void GLRenderAPI::MakeGlMatrix(GLfloat gl_matrix[16], const Matrix4& m)
{
	u32 x = 0;
	for(u32 i = 0; i < 4; i++)
	{
		for(u32 j = 0; j < 4; j++)
		{
			gl_matrix[x] = m[j][i];
			x++;
		}
	}
}

void GLRenderAPI::ApplyViewport()
{
	if(mActiveRenderTarget == nullptr)
		return;

	const RenderTargetProperties& rtProps = mActiveRenderTarget->GetProperties();

	// Calculate the "lower-left" corner of the viewport
	mViewportLeft = (u32)(rtProps.Width * mViewportNorm.X);
	mViewportTop = (u32)(rtProps.Height * mViewportNorm.Y);
	mViewportWidth = (u32)(rtProps.Width * mViewportNorm.Width);
	mViewportHeight = (u32)(rtProps.Height * mViewportNorm.Height);

	glViewport(mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight);
	B3D_CHECK_GL_ERROR();

	// Configure the viewport clipping
	if(!mScissorEnabled)
	{
		glEnable(GL_SCISSOR_TEST);
		B3D_CHECK_GL_ERROR();

		glScissor(mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight);
		B3D_CHECK_GL_ERROR();
	}
}

void GLRenderAPI::NotifyRenderTargetModified()
{
	if(mActiveRenderTarget == nullptr || mActiveRenderTargetModified)
		return;

	mActiveRenderTarget->TickUpdateCountInternal();
	mActiveRenderTargetModified = true;
}

/************************************************************************/
/* 								UTILITY		                     		*/
/************************************************************************/

void GLRenderAPI::ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
{
	dest = matrix;
}

GpuParameterBlockInformation GLRenderAPI::GenerateParamBlockDesc(const String& name, Vector<GpuDataParameterInformation>& params)
{
	GpuParameterBlockInformation block;
	block.BlockSize = 0;
	block.IsShareable = true;
	block.Name = name;
	block.Slot = 0;
	block.Set = 0;

	for(auto& param : params)
	{
		u32 size;

		if(param.Type == GPDT_STRUCT)
		{
			// Structs are always aligned and rounded up to vec4
			size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
			block.BlockSize = Math::DivideAndRoundUp(block.BlockSize, 4U) * 4;
		}
		else
			size = GLSLParamParser::CalcInterfaceBlockElementSizeAndOffset(param.Type, param.ArraySize, block.BlockSize);

		if(param.ArraySize > 1)
		{
			param.ElementSize = size;
			param.ArrayElementStride = size;
			param.CpuMemOffset = block.BlockSize;
			param.GpuMemOffset = 0;

			block.BlockSize += size * param.ArraySize;
		}
		else
		{
			param.ElementSize = size;
			param.ArrayElementStride = size;
			param.CpuMemOffset = block.BlockSize;
			param.GpuMemOffset = 0;

			block.BlockSize += size;
		}

		param.ParamBlockSlot = 0;
		param.ParamBlockSet = 0;
	}

	// Constant buffer size must always be a multiple of 16
	if(block.BlockSize % 4 != 0)
		block.BlockSize += (4 - (block.BlockSize % 4));

	return block;
}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
void OpenGlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	if(type != GL_DEBUG_TYPE_PERFORMANCE && type != GL_DEBUG_TYPE_OTHER)
	{
		B3D_EXCEPT(RenderingAPIException, "OpenGL error: " + String(message));
	}
}
#endif
