//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DLightProbeVolume.h"
#include "B3DApplication.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Image/B3DTexture.h"
#include "RTTI/B3DLightProbeVolumeRTTI.h"
#include "Profiling/B3DProfilerGPU.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(LightProbeVolume, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<u32>, RemovedProbes)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<DirtyProbeInfo>, DirtyProbes)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(bool, mActive)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(SPtr<SceneInstance>, mSceneInstance)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END
}

LightProbeVolume::LightProbeVolume(const HSceneObject& parent, const AABox& volume, const Vector3I& cellCount)
	: Component(parent), mVolume(volume), mCellCount(cellCount)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("LightProbeVolume");
}

LightProbeVolume::LightProbeVolume()
	: LightProbeVolume(nullptr)
{ }

u32 LightProbeVolume::AddProbe(const Vector3& position)
{
	u32 handle = mNextProbeId++;
	mProbes[handle] = ProbeInfo(LightProbeFlags::Clean, position);

	MarkRenderProxyDataDirty();
	return handle;
}

void LightProbeVolume::RemoveProbe(u32 handle)
{
	auto iterFind = mProbes.find(handle);
	if(iterFind != mProbes.end() && mProbes.size() > 4)
	{
		iterFind->second.Flags = LightProbeFlags::Removed;
		MarkRenderProxyDataDirty();
	}
}

void LightProbeVolume::SetProbePosition(u32 handle, const Vector3& position)
{
	if(auto found = mProbes.find(handle); found != mProbes.end())
	{
		found->second.Position = position;
		MarkRenderProxyDataDirty();
	}
}

Vector3 LightProbeVolume::GetProbePosition(u32 handle) const
{
	if(auto found = mProbes.find(handle); found != mProbes.end())
		return found->second.Position;

	return Vector3::kZero;
}

Vector<LightProbeInfo> LightProbeVolume::GetProbes() const
{
	Vector<LightProbeInfo> output;

	for(auto& entry : mProbes)
	{
		if(entry.second.Flags == LightProbeFlags::Removed || entry.second.Flags == LightProbeFlags::Empty)
			continue;

		LightProbeInfo info;
		info.Position = entry.second.Position;
		info.Handle = entry.first;
		info.ShCoefficients = entry.second.Coefficients;

		output.push_back(info);
	}

	return output;
}

void LightProbeVolume::RenderProbe(u32 handle)
{
	if(!GetEnabled())
		return;

	auto iterFind = mProbes.find(handle);
	if(iterFind != mProbes.end())
	{
		if(iterFind->second.Flags == LightProbeFlags::Clean)
		{
			iterFind->second.Flags = LightProbeFlags::Dirty;

			MarkRenderProxyDataDirty();
			RunRenderProbeTask();
		}
	}
}

void LightProbeVolume::RenderProbes()
{
	if(!GetEnabled())
		return;

	bool anyModified = false;
	for(auto& entry : mProbes)
	{
		if(entry.second.Flags == LightProbeFlags::Clean)
		{
			entry.second.Flags = LightProbeFlags::Dirty;
			anyModified = true;
		}
	}

	if(anyModified)
	{
		MarkRenderProxyDataDirty();
		RunRenderProbeTask();
	}
}

void LightProbeVolume::Resize(const AABox& volume, const Vector3I& cellCount)
{
	const u32 probeCountX = std::max(1, mCellCount.X) + 1;
	const u32 probeCountY = std::max(1, mCellCount.Y) + 1;
	const u32 probeCountZ = std::max(1, mCellCount.Z) + 1;

	Vector3 size = mVolume.GetSize();
	for(u32 z = 0; z < probeCountZ; ++z)
	{
		for(u32 y = 0; y < probeCountY; ++y)
		{
			for(u32 x = 0; x < probeCountX; ++x)
			{
				Vector3 position = mVolume.Minimum;
				position.X += size.X * (x / (float)probeCountX);
				position.Y += size.Y * (y / (float)probeCountY);
				position.Z += size.Z * (z / (float)probeCountZ);

				if(mVolume.Contains(position))
					continue;

				AddProbe(position);
			}
		}
	}

	mVolume = volume;
	mCellCount = cellCount;

	MarkRenderProxyDataDirty();
}

void LightProbeVolume::Reset()
{
	const u32 probeCountX = std::max(1, mCellCount.X) + 1;
	const u32 probeCountY = std::max(1, mCellCount.Y) + 1;
	const u32 probeCountZ = std::max(1, mCellCount.Z) + 1;

	const u32 probeCount = probeCountX * probeCountY * probeCountZ;

	// Make sure there are adequate number of probes to fill the volume
	while((u32)mProbes.size() < probeCount)
		AddProbe(Vector3::kZero);

	u32 idx = 0;
	u32 rowPitch = probeCountX;
	u32 slicePitch = probeCountX * probeCountY;

	Vector3 size = mVolume.GetSize();

	auto iter = mProbes.begin();
	while(iter != mProbes.end())
	{
		u32 x = idx % probeCountX;
		u32 y = (idx / rowPitch) % probeCountY;
		u32 z = (idx / slicePitch);

		Vector3 position = mVolume.Minimum;
		position.X += size.X * (x / (float)(probeCountX - 1));
		position.Y += size.Y * (y / (float)(probeCountY - 1));
		position.Z += size.Z * (z / (float)(probeCountZ - 1));

		iter->second.Position = position;
		iter->second.Flags = LightProbeFlags::Clean;

		++idx;
		++iter;

		if(idx >= probeCount)
			break;
	}

	// Set remaining probes to removed state
	while(iter != mProbes.end())
	{
		iter->second.Flags = LightProbeFlags::Removed;
		++iter;
	}

	MarkRenderProxyDataDirty();
}

void LightProbeVolume::Clip()
{
	for(auto& entry : mProbes)
	{
		if(!mVolume.Contains(entry.second.Position))
			entry.second.Flags = LightProbeFlags::Removed;
	}

	MarkRenderProxyDataDirty();
}

void LightProbeVolume::RunRenderProbeTask()
{
	// If a task is already running cancel it
	// Note: If the task is just about to start processing, cancelling it will skip the update this frame
	// (which might be fine if we just changed positions of dirty probes it was about to update, but it might also
	// waste a frame if those positions needed to be updated anyway). For now I'm ignoring it as it seems like a rare
	// enough situation, plus it's one that will only happen during development time.
	if(mRendererTask)
		mRendererTask->Cancel();

	auto renderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<render::LightProbeVolume> renderProxy = B3DGetRenderProxy(this);
	auto renderProbes = [renderProxy](render::GpuCommandBufferPool& commandBufferPool)
	{
		SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("LightProbeRendering"));
		GetProfilerGPU().BeginSample(*commandBuffer, "LightProbeRendering");
		const bool isDone = renderProxy->RenderProbes(*commandBuffer, 3);
		GetProfilerGPU().EndSample(*commandBuffer, "LightProbeRendering");

		const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
		gpuDevice->SubmitCommandBuffer(commandBuffer);

		return isDone;
	};

	mRendererTask = render::RendererTask::Create("RenderLightProbes", renderProbes);

	mRendererTask->OnComplete.Connect(renderComplete);
	render::GetRenderer()->AddTask(mRendererTask);
}

void LightProbeVolume::UpdateCoefficients()
{
	// Ensure all light probe coefficients are generated
	if(mRendererTask)
		mRendererTask->Wait();

	render::LightProbeVolume* renderProxy = B3DGetRenderProxy(this).get();

	Vector<LightProbeCoefficientInfo> coeffInfo;
	auto getSaveData = [renderProxy, &coeffInfo]()
	{
		renderProxy->GetProbeCoefficients(coeffInfo);
	};

	GetRenderThread().PostCommand(getSaveData, "LightProbeVolume::GetProbeCoefficients", true);

	for(auto& entry : coeffInfo)
	{
		auto iterFind = mProbes.find(entry.Handle);
		if(iterFind == mProbes.end())
			continue;

		iterFind->second.Coefficients = entry.Coefficients;
	}
}

SPtr<render::RenderProxy> LightProbeVolume::CreateRenderProxy() const
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	render::LightProbeVolume* renderProxy = new(B3DAllocate<render::LightProbeVolume>()) render::LightProbeVolume(B3DGetRenderProxy(scene), mProbes);
	SPtr<render::LightProbeVolume> renderProxyShared = B3DMakeSharedFromExisting<render::LightProbeVolume>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* LightProbeVolume::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	auto* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->mActive = GetEnabled();
	syncPacket->mSceneInstance = B3DGetRenderProxy(SceneObject()->GetScene());
	syncPacket->mTransform = SceneObject()->GetTransform();
	
	for(auto& probe : mProbes)
	{
		if(probe.second.Flags == LightProbeFlags::Dirty)
		{
			syncPacket->DirtyProbes.push_back(DirtyProbeInfo(probe.first, probe.second.Position, probe.second.Flags));
			probe.second.Flags = LightProbeFlags::Clean;
		}
		else if(probe.second.Flags == LightProbeFlags::Removed)
		{
			syncPacket->RemovedProbes.push_back(probe.first);
			probe.second.Flags = LightProbeFlags::Empty;
		}
	}

	for(auto& probe : syncPacket->RemovedProbes)
		mProbes.erase(probe);

	return syncPacket;
}

void LightProbeVolume::Initialize()
{
	SetShared(B3DStaticGameObjectCast<LightProbeVolume>(mThisHandle).GetShared());

	Component::Initialize();
	CoreObject::Initialize();
}

void LightProbeVolume::OnCreated()
{
	Reset();
}

void LightProbeVolume::OnDestroyed()
{
	if(mRendererTask)
		mRendererTask->Cancel();

	CoreObject::Destroy();
}

RTTIType* LightProbeVolume::GetRttiStatic()
{
	return LightProbeVolumeRTTI::Instance();
}

RTTIType* LightProbeVolume::GetRtti() const
{
	return LightProbeVolume::GetRttiStatic();
}

namespace b3d { namespace render
{
LightProbeVolume::LightProbeVolume(const SPtr<SceneInstance>& scene, const UnorderedMap<u32, b3d::LightProbeVolume::ProbeInfo>& probes)
	: mSceneInstance(scene)
{
	mInitCoefficients.resize(probes.size());
	mProbePositions.resize(probes.size());
	mProbeInfos.resize(probes.size());

	u32 probeIdx = 0;
	for(auto& entry : probes)
	{
		mProbeMap[entry.first] = probeIdx;
		mProbePositions[probeIdx] = entry.second.Position;

		LightProbeInfo probeInfo;
		probeInfo.Flags = LightProbeFlags::Dirty;
		probeInfo.BufferIdx = probeIdx;
		probeInfo.Handle = entry.first;

		mProbeInfos[probeIdx] = probeInfo;
		mInitCoefficients[probeIdx] = entry.second.Coefficients;

		probeIdx++;
	}
}

LightProbeVolume::~LightProbeVolume()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->UnregisterLightProbeVolume(this);
}

void LightProbeVolume::Initialize()
{
	// Set SH coefficients loaded from the file
	u32 numCoefficients = (u32)mInitCoefficients.size();
	B3D_ASSERT(mInitCoefficients.size() == mProbeMap.size());

	ResizeCoefficientTexture(std::max(32U, numCoefficients));

	SPtr<PixelData> coeffData = mCoefficients->GetProperties().AllocBuffer(0, 0);
	coeffData->SetColors(Color::kZero);

	u32 probesPerRow = coeffData->GetWidth() / 9;
	u32 probeIdx = 0;
	for(u32 y = 0; y < coeffData->GetHeight(); ++y)
	{
		for(u32 x = 0; x < probesPerRow; ++x)
		{
			if(probeIdx >= numCoefficients)
				break;

			for(u32 i = 0; i < 9; i++)
			{
				Color value;
				value.R = mInitCoefficients[probeIdx].CoeffsR[i];
				value.G = mInitCoefficients[probeIdx].CoeffsG[i];
				value.B = mInitCoefficients[probeIdx].CoeffsB[i];

				coeffData->SetColorAt(value, x * 9, y);
			}

			probeIdx++;
		}
	}

	mCoefficients->WriteData(*coeffData, 0, 0, true);
	mInitCoefficients.clear();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterLightProbeVolume(this);
	RenderProxy::Initialize();
}

bool LightProbeVolume::RenderProbes(GpuCommandBuffer& commandBuffer, u32 maxProbes)
{
	// Probe map only contains active probes
	u32 numUsedProbes = (u32)mProbeMap.size();
	if (numUsedProbes > mCoeffBufferSize)
	{
		const SPtr<Texture> oldTexture = mCoefficients;

		ResizeCoefficientTexture(std::max(32U, numUsedProbes * 2));

		if (oldTexture)
			oldTexture->Copy(commandBuffer, mCoefficients);
	}

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();

	u32 numProbeUpdates = 0;
	for(; mFirstDirtyProbe < (u32)mProbeInfos.size(); ++mFirstDirtyProbe)
	{
		LightProbeInfo& probeInfo = mProbeInfos[mFirstDirtyProbe];

		if(probeInfo.Flags == LightProbeFlags::Dirty)
		{
			TextureCreateInformation cubemapDesc;
			cubemapDesc.Name = "LightProbeVolume Cubemap";
			cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
			cubemapDesc.Format = PF_RGBA16F;
			cubemapDesc.Width = 256; // Note: Test different sizes and their effect on quality
			cubemapDesc.Height = 256;
			cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

			SPtr<Texture> cubemap = gpuDevice->CreateTexture(cubemapDesc);

			Vector3 localPos = mProbePositions[mFirstDirtyProbe];

			const Vector3& position = mTransform.GetPosition();
			const Quaternion& rotation = mTransform.GetRotation();
			Vector3 transformedPos = rotation.Rotate(localPos) + position;

			GetRenderer()->CaptureSceneCubeMap(*rendererScene, commandBuffer, cubemap, transformedPos, CaptureSettings());
			GetIBLUtility().FilterCubemapForIrradiance(commandBuffer, cubemap, mCoefficients, probeInfo.BufferIdx);

			probeInfo.Flags = LightProbeFlags::Clean;
			numProbeUpdates++;
		}

		if(maxProbes != 0 && numProbeUpdates >= maxProbes)
			break;
	}

	rendererScene->UpdateLightProbeVolume(this);

	return mFirstDirtyProbe == (u32)mProbeInfos.size();
}

void LightProbeVolume::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::LightProbeVolume::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	syncPacket->ApplySyncData(this);

	for(const auto& dirtyProbe : syncPacket->DirtyProbes)
	{
		auto iterFind = mProbeMap.find(dirtyProbe.ProbeIndex);
		if(iterFind != mProbeMap.end())
		{
			// Update existing probe information
			u32 compactIdx = iterFind->second;

			mProbeInfos[compactIdx].Flags = LightProbeFlags::Dirty;
			mProbePositions[compactIdx] = dirtyProbe.Position;

			mFirstDirtyProbe = std::min(compactIdx, mFirstDirtyProbe);
		}
		else // Add a new probe
		{
			// Empty slots always start at a specific index because we always move them to the back of the array
			u32 emptyProbeStartIdx = (u32)mProbeMap.size();
			u32 numProbes = (u32)mProbeInfos.size();

			// Find an empty slot to place the probe information at
			u32 compactIdx = -1;
			for(u32 j = emptyProbeStartIdx; j < numProbes; ++j)
			{
				if(mProbeInfos[j].Flags == LightProbeFlags::Empty)
				{
					compactIdx = j;
					break;
				}
			}

			// Found an empty slot
			if(compactIdx == (u32)-1)
			{
				compactIdx = (u32)mProbeInfos.size();

				LightProbeInfo info;
				info.Flags = LightProbeFlags::Dirty;
				info.BufferIdx = compactIdx;
				info.Handle = dirtyProbe.ProbeIndex;

				mProbeInfos.push_back(info);
				mProbePositions.push_back(dirtyProbe.Position);
			}
			else // No empty slot, add a new one
			{
				LightProbeInfo& info = mProbeInfos[compactIdx];
				info.Flags = LightProbeFlags::Dirty;
				info.Handle = dirtyProbe.ProbeIndex;

				mProbePositions[compactIdx] = dirtyProbe.Position;
			}

			mProbeMap[dirtyProbe.ProbeIndex] = compactIdx;
			mFirstDirtyProbe = std::min(compactIdx, mFirstDirtyProbe);
		}
	}

	// Mark slots for removed probes as empty, and move them back to the end of the array
	for(u32 index = 0; index < (u32)syncPacket->RemovedProbes.size(); ++index)
	{
		const u32 removedProbeIndex = syncPacket->RemovedProbes[index];

		auto iterFind = mProbeMap.find(removedProbeIndex);
		if(iterFind != mProbeMap.end())
		{
			u32 compactIdx = iterFind->second;

			LightProbeInfo& info = mProbeInfos[compactIdx];
			info.Flags = LightProbeFlags::Empty;

			// Move the empty info to the back of the array so all non-empty probes are contiguous
			// Search from back to current index, and find first non-empty probe to switch switch
			u32 lastSearchIdx = (u32)mProbeInfos.size() - 1;
			while(lastSearchIdx >= (u32)compactIdx)
			{
				LightProbeFlags flags = mProbeInfos[lastSearchIdx].Flags;
				if(flags != LightProbeFlags::Empty)
				{
					std::swap(mProbeInfos[index], mProbeInfos[lastSearchIdx]);
					std::swap(mProbePositions[index], mProbePositions[lastSearchIdx]);

					mProbeMap[mProbeInfos[lastSearchIdx].Handle] = index;
					break;
				}

				lastSearchIdx--;
			}

			mProbeMap.erase(iterFind);
		}
	}

	if(oldIsActive != mActive)
	{
		const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
		if(mActive)
			rendererScene->RegisterLightProbeVolume(this);
		else
			rendererScene->UnregisterLightProbeVolume(this);
	}
}

void LightProbeVolume::GetProbeCoefficients(Vector<LightProbeCoefficientInfo>& output) const
{
	u32 numActiveProbes = (u32)mProbeMap.size();
	if(numActiveProbes == 0)
		return;

	output.resize(numActiveProbes);

	LightProbeSHCoefficients* coefficients = B3DStackAllocate<LightProbeSHCoefficients>(numActiveProbes);

	SPtr<PixelData> coeffData = mCoefficients->GetProperties().AllocBuffer(0, 0);
	mCoefficients->ReadData(*coeffData);

	u32 probesPerRow = coeffData->GetWidth() / 9;
	u32 probeIdx = 0;
	for(u32 y = 0; y < coeffData->GetHeight(); ++y)
	{
		for(u32 x = 0; x < probesPerRow; ++x)
		{
			if(probeIdx >= numActiveProbes)
				break;

			for(u32 i = 0; i < 9; i++)
			{
				Color value = coeffData->GetColorAt(x * 9, y);

				coefficients[probeIdx].CoeffsR[i] = value.R;
				coefficients[probeIdx].CoeffsG[i] = value.G;
				coefficients[probeIdx].CoeffsB[i] = value.B;
			}

			probeIdx++;
		}
	}

	for(u32 i = 0; i < numActiveProbes; ++i)
	{
		output[i].Coefficients = coefficients[mProbeInfos[i].BufferIdx];
		output[i].Handle = mProbeInfos[i].Handle;
	}

	B3DStackFree(coefficients);
}

void LightProbeVolume::ResizeCoefficientTexture(u32 count)
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	Vector2I texSize = IBLUtility::GetShCoeffTextureSize(count, 3);

	TextureCreateInformation desc;
	desc.Name = "LightProbeVolume coefficients";
	desc.Width = (u32)texSize.X;
	desc.Height = (u32)texSize.Y;
	desc.Usage = TU_LOADSTORE | TU_RENDERTARGET;
	desc.Format = PF_RGBA32F;

	mCoefficients = gpuDevice->CreateTexture(desc);
	mCoeffBufferSize = count;
}
}}
