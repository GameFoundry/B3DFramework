//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuBackend.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "ThirdParty/D3D12MemAlloc.h"

#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuParameterSet.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuProgram.h"
#include "B3DD3D12Queries.h"
#include "B3DD3D12SamplerState.h"
#include "B3DD3D12Texture.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "Utility/B3DBitwise.h"

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32VideoModeInfo.h"
#else
	static_assert(false, "DirectX 12 is only supported on Windows.");
#endif

using namespace b3d;
using namespace b3d::render;

D3D12GpuDevice::D3D12GpuDevice(IDXGIAdapter4* adapter)
	: mAdapter(adapter)
{
	HRESULT hr;

	// Create D3D12 device
	hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to create D3D12 device");

	// Create command queues for each queue type
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	// Graphics queue (always present)
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ID3D12CommandQueue* graphicsQueue;
	hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&graphicsQueue));
	B3D_ASSERT(SUCCEEDED(hr));

	mQueueInfos[GQT_GRAPHICS].Queues.push_back(
		B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuQueue>()) D3D12GpuQueue(*this, GQT_GRAPHICS, 0, graphicsQueue))
	);

	// Compute queue (optional, use direct queue if not available)
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ID3D12CommandQueue* computeQueue;
	hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&computeQueue));
	if (SUCCEEDED(hr))
	{
		mQueueInfos[GQT_COMPUTE].Queues.push_back(
			B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuQueue>()) D3D12GpuQueue(*this, GQT_COMPUTE, 0, computeQueue))
		);
	}

	// Copy/Transfer queue (optional, use direct queue if not available)
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ID3D12CommandQueue* copyQueue;
	hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&copyQueue));
	if (SUCCEEDED(hr))
	{
		mQueueInfos[GQT_TRANSFER].Queues.push_back(
			B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuQueue>()) D3D12GpuQueue(*this, GQT_TRANSFER, 0, copyQueue))
		);
	}

	// Query timestamp frequency
	ID3D12CommandQueue* timestampQueue = mQueueInfos[GQT_GRAPHICS].Queues[0]->GetD3D12Handle();
	timestampQueue->GetTimestampFrequency(&mTimestampFrequency);

	// Initialize capabilities
	InitializeCapabilities();

	// Create descriptor manager
	mDescriptorManager = B3DNew<D3D12DescriptorManager>(*this);

	// Create memory allocator
	D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
	allocatorDesc.pDevice = mDevice.Get();
	allocatorDesc.pAdapter = mAdapter.Get();

	HRESULT hr = D3D12MA::CreateAllocator(&allocatorDesc, &mAllocator);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create D3D12 memory allocator");
	}

	// Initialize video mode information
#if B3D_PLATFORM_WIN32
	mVideoModeInfo = B3DMakeShared<Win32VideoModeInfo>();
#else
	static_assert(false, "mVideoModeInfo needs to be created.");
#endif

	mTransferBufferHelper = B3DMakeUnique<GpuTransferBufferHelper>(*this, GpuQueueId(GQT_GRAPHICS, 0));
}

D3D12GpuDevice::~D3D12GpuDevice()
{
	// Destroy transfer buffer helper before other cleanup
	mTransferBufferHelper.reset();

	// Clear cached sampler states
	mCachedSamplerStates.clear();

	// Release queues
	for (u32 queueUsageIndex = 0; queueUsageIndex < GQT_COUNT; queueUsageIndex++)
	{
		for (auto& queue : mQueueInfos[queueUsageIndex].Queues)
			queue = nullptr;
	}

	// Delete descriptor manager
	B3DDelete(mDescriptorManager);

	// Release memory allocator
	if (mAllocator)
	{
		mAllocator->Release();
		mAllocator = nullptr;
	}

	// Release device
	mDevice.Reset();
	mAdapter.Reset();
}

SPtr<GpuProgramBytecode> D3D12GpuDevice::CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const
{
	if (!IsGpuProgramLanguageSupported(createInformation.Language))
		return nullptr;

	// TODO: Implement HLSL compilation using DXC (DirectX Shader Compiler)
	// For now, return nullptr to indicate compilation is not yet implemented
	SPtr<GpuProgramBytecode> bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->compilerId = 0; // TODO: Define D3D12 compiler ID
	bytecode->compilerVersion = 0;

	B3D_LOG(Warning, LogRenderBackend, "D3D12 shader compilation not yet implemented");

	return bytecode;
}

SPtr<GpuQueue> D3D12GpuDevice::GetQueue(GpuQueueUsage usage, u32 index) const
{
	if (index >= GetQueueCount(usage))
		return nullptr;

	return mQueueInfos[(u32)usage].Queues[index];
}

SPtr<GpuCommandBufferPool> D3D12GpuDevice::CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuCommandBufferPool>()) D3D12GpuCommandBufferPool(*this, createInformation));
}

SPtr<render::Texture> D3D12GpuDevice::CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	D3D12Texture* rawTexture = new (B3DAllocate<D3D12Texture>()) D3D12Texture(createInformation, *this);

	// Default: standalone (calling-thread deletion)
	// With RenderProxy flag: forward destruction to render thread
	SPtr<Texture> output = flags.IsSet(GpuObjectCreateFlag::RenderProxy)
		? B3DMakeSharedFromExisting(rawTexture)
		: MakeSharedStandalone<Texture>(rawTexture);

	output->SetShared(output);

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<render::GpuBuffer> D3D12GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	D3D12GpuBuffer* rawBuffer = new (B3DAllocate<D3D12GpuBuffer>()) D3D12GpuBuffer(createInformation, *this);

	// Default: standalone (calling-thread deletion)
	// With RenderProxy flag: forward destruction to render thread
	SPtr<GpuBuffer> output = flags.IsSet(GpuObjectCreateFlag::RenderProxy)
		? B3DMakeSharedFromExisting(rawBuffer)
		: MakeSharedStandalone<GpuBuffer>(rawBuffer);

	output->SetShared(output);

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<GpuQueryPool> D3D12GpuDevice::CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation)
{
	return B3DMakeShared<D3D12GpuQueryPool>(*this, createInformation);
}

SPtr<SamplerState> D3D12GpuDevice::CreateSamplerState(const SamplerStateCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<SamplerState> output = B3DMakeSharedFromExisting(new (B3DAllocate<D3D12SamplerState>()) D3D12SamplerState(createInformation, *this));

	if (!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<EventQuery> D3D12GpuDevice::CreateEventQuery()
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<D3D12EventQuery>()) D3D12EventQuery(*this));
}

SPtr<TimerQuery> D3D12GpuDevice::CreateTimerQuery()
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<D3D12TimerQuery>()) D3D12TimerQuery(*this));
}

SPtr<OcclusionQuery> D3D12GpuDevice::CreateOcclusionQuery(bool isBinary)
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<D3D12OcclusionQuery>()) D3D12OcclusionQuery(isBinary, *this));
}

SPtr<GpuProgram> D3D12GpuDevice::CreateGpuProgram(const GpuProgramCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<GpuProgram> output = B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuProgram>()) D3D12GpuProgram(createInformation, *this));

	if (!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<GpuGraphicsPipelineState> D3D12GpuDevice::CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<D3D12GpuGraphicsPipelineState> output = B3DMakeSharedFromExisting<D3D12GpuGraphicsPipelineState>(
		new (B3DAllocate<D3D12GpuGraphicsPipelineState>()) D3D12GpuGraphicsPipelineState(createInformation, *this));

	if (!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<GpuComputePipelineState> D3D12GpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<D3D12GpuComputePipelineState> output = B3DMakeSharedFromExisting<D3D12GpuComputePipelineState>(
		new (B3DAllocate<D3D12GpuComputePipelineState>()) D3D12GpuComputePipelineState(createInformation, *this));

	if (!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<GpuPipelineParameterLayout> D3D12GpuDevice::CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting<D3D12GpuPipelineParameterLayout>(
		new (B3DAllocate<D3D12GpuPipelineParameterLayout>()) D3D12GpuPipelineParameterLayout(createInformation, *this));
}

SPtr<GpuPipelineParameterSetLayout> D3D12GpuDevice::CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription)
{
	return B3DMakeShared<GpuPipelineParameterSetLayout>(parameterDescription);
}

UPtr<GpuParameterSetPool> D3D12GpuDevice::CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation)
{
	// TODO: Implement D3D12-specific descriptor heap pool when needed
	// For now, return nullptr - D3D12 uses descriptor heaps differently than Vulkan
	return nullptr;
}

void D3D12GpuDevice::WaitUntilIdle()
{
	// Wait for all queues to finish
	for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		const u32 queueCount = GetQueueCount((GpuQueueUsage)queueTypeIndex);
		for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			SPtr<D3D12GpuQueue> queue = std::static_pointer_cast<D3D12GpuQueue>(GetQueue((GpuQueueUsage)queueTypeIndex, queueIndex));
			if (queue)
				queue->WaitUntilIdle();
		}
	}
}

void D3D12GpuDevice::BeginFrame()
{
	// TODO: Implement frame begin logic (descriptor heap management, etc.)
}

void D3D12GpuDevice::EndFrame()
{
	SubmitTransferCommandBuffers();

	// Advance transfer buffer helper pools to next frame
	mTransferBufferHelper->EndFrame();

	// TODO: Implement frame end logic
}

void D3D12GpuDevice::PresentRenderWindow(const SPtr<render::RenderWindow>& renderWindow, u32 syncMask)
{
	SPtr<GpuQueue> queue = GetQueue(GQT_GRAPHICS, 0);
	if (!B3D_ENSURE(queue))
		return;

	queue->PresentRenderWindow(renderWindow, syncMask);
}

void D3D12GpuDevice::ConvertProjectionMatrix(const Matrix4& input, Matrix4& output)
{
	output = input;

	// D3D12 uses depth range [0,1] (same as Vulkan)
	// No Y-axis flip needed for D3D12 (unlike Vulkan)
	// Convert depth range from [-1,1] to [0,1]
	output[2][0] = (output[2][0] + output[3][0]) / 2;
	output[2][1] = (output[2][1] + output[3][1]) / 2;
	output[2][2] = (output[2][2] + output[3][2]) / 2;
	output[2][3] = (output[2][3] + output[3][3]) / 2;
}

GpuUniformBufferInformation D3D12GpuDevice::GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms)
{
	GpuUniformBufferInformation buffer;
	buffer.Size = 0;
	buffer.IsShareable = true;
	buffer.Name = name;
	buffer.Slot = 0;
	buffer.Set = 0;

	// D3D12 uses HLSL constant buffer packing rules (similar to std140)
	for (auto& param : inOutUniforms)
	{
		u32 size;
		if (param.Type == GPDT_STRUCT)
		{
			// Structs are always aligned and rounded up to vec4 (16 bytes)
			size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
			buffer.Size = Math::DivideAndRoundUp(buffer.Size, 4U) * 4;
		}
		else
		{
			// Calculate size based on HLSL packing rules
			size = D3D12Utility::CalcConstantBufferElementSizeAndOffset(param.Type, param.ArraySize, buffer.Size);
		}

		param.ElementSize = size;
		param.ArrayElementStride = size;
		param.CpuOffset = buffer.Size;
		param.GpuOffset = 0;
		buffer.Size += size * param.ArraySize;
		param.ParentUniformBufferSlot = 0;
		param.ParentUniformBufferSet = 0;
	}

	// Constant buffer size must always be a multiple of 16 bytes (256 bits)
	if (buffer.Size % 4 != 0)
		buffer.Size += (4 - (buffer.Size % 4));

	return buffer;
}

float D3D12GpuDevice::ConvertTimestampToMilliseconds(u64 timestamp)
{
	if (mTimestampFrequency == 0)
		return 0.0f;

	const double timestampToMs = 1000.0 / (double)mTimestampFrequency;
	return (float)((double)timestamp * timestampToMs);
}

void D3D12GpuDevice::InitializeCapabilities()
{
	DXGI_ADAPTER_DESC3 adapterDesc;
	mAdapter->GetDesc3(&adapterDesc);

	// Convert adapter description to char string
	char deviceName[128];
	wcstombs(deviceName, adapterDesc.Description, sizeof(deviceName));

	mCapabilities.DeviceName = deviceName;
	mCapabilities.DriverVersion = "Unknown"; // D3D12 doesn't provide easy access to driver version

	// Query feature support
	D3D12_FEATURE_DATA_D3D12_OPTIONS options;
	if (SUCCEEDED(mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
	{
		mCapabilities.MaxBoundVertexBuffers = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
		mCapabilities.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
		mCapabilities.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
		mCapabilities.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
		mCapabilities.NumTextureUnitsPerStage[GPT_HULL_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
		mCapabilities.NumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
		mCapabilities.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;

		mCapabilities.NumUniformBlocksPerStage[GPT_VERTEX_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		mCapabilities.NumUniformBlocksPerStage[GPT_FRAGMENT_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		mCapabilities.NumUniformBlocksPerStage[GPT_GEOMETRY_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		mCapabilities.NumUniformBlocksPerStage[GPT_HULL_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		mCapabilities.NumUniformBlocksPerStage[GPT_DOMAIN_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		mCapabilities.NumUniformBlocksPerStage[GPT_COMPUTE_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;

		mCapabilities.MaximumRenderTargets = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;

		// Check for additional capabilities
		mCapabilities.HasGeometryShaders = true;
		mCapabilities.HasTessellationShaders = true;
		mCapabilities.HasComputeShaders = true;
	}

	// Query texture capabilities
	D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
	formatSupport.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (SUCCEEDED(mDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
	{
		mCapabilities.MaxTextureSize = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		mCapabilities.MaxCubeTextureSize = D3D12_REQ_TEXTURECUBE_DIMENSION;
		mCapabilities.MaxTextureArraySlices = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
	}

	// Set vendor
	switch (adapterDesc.VendorId)
	{
	case 0x10DE:
		mCapabilities.Vendor = GPU_NVIDIA;
		break;
	case 0x1002:
	case 0x1022:
		mCapabilities.Vendor = GPU_AMD;
		break;
	case 0x163C:
	case 0x8086:
	case 0x8087:
		mCapabilities.Vendor = GPU_INTEL;
		break;
	default:
		mCapabilities.Vendor = GPU_UNKNOWN;
		break;
	}

	B3D_LOG(Info, LogRenderBackend, "D3D12 Device: {0}", mCapabilities.DeviceName);
}
