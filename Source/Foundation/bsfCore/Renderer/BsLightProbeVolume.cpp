//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsLightProbeVolume.h"

#include "BsCoreApplication.h"
#include "Private/RTTI/BsLightProbeVolumeRTTI.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsLight.h"
#include "Image/BsTexture.h"
#include "Renderer/BsIBLUtility.h"
#include "Scene/BsSceneObject.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "Profiling/BsProfilerGPU.h"
#include "RenderAPI/BsGpuPipelineState.h"

using namespace bs;

LightProbeVolume::LightProbeVolume(const AABox& volume, const Vector3I& cellCount)
	: mVolume(volume), mCellCount(cellCount)
{
	Reset();
}

LightProbeVolume::~LightProbeVolume()
{
	if(mRendererTask)
		mRendererTask->Cancel();
}

u32 LightProbeVolume::AddProbe(const Vector3& position)
{
	u32 handle = mNextProbeId++;
	mProbes[handle] = ProbeInfo(LightProbeFlags::Clean, position);

	MarkCoreDirtyInternal();
	return handle;
}

void LightProbeVolume::RemoveProbe(u32 handle)
{
	auto iterFind = mProbes.find(handle);
	if(iterFind != mProbes.end() && mProbes.size() > 4)
	{
		iterFind->second.Flags = LightProbeFlags::Removed;
		MarkCoreDirtyInternal();
	}
}

void LightProbeVolume::SetProbePosition(u32 handle, const Vector3& position)
{
	auto iterFind = mProbes.find(handle);
	if(iterFind != mProbes.end())
	{
		iterFind->second.Position = position;
		MarkCoreDirtyInternal();
	}
}

Vector3 LightProbeVolume::GetProbePosition(u32 handle) const
{
	auto iterFind = mProbes.find(handle);
	if(iterFind != mProbes.end())
		return iterFind->second.Position;

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

void LightProbeVolume::Resize(const AABox& volume, const Vector3I& cellCount)
{
	u32 numProbesX = std::max(1, mCellCount.X) + 1;
	u32 numProbesY = std::max(1, mCellCount.Y) + 1;
	u32 numProbesZ = std::max(1, mCellCount.Z) + 1;

	Vector3 size = mVolume.GetSize();
	for(u32 z = 0; z < numProbesZ; ++z)
	{
		for(u32 y = 0; y < numProbesY; ++y)
		{
			for(u32 x = 0; x < numProbesX; ++x)
			{
				Vector3 position = mVolume.GetMin();
				position.X += size.X * (x / (float)numProbesX);
				position.Y += size.Y * (y / (float)numProbesY);
				position.Z += size.Z * (z / (float)numProbesZ);

				if(mVolume.Contains(position))
					continue;

				AddProbe(position);
			}
		}
	}

	mVolume = volume;
	mCellCount = cellCount;

	MarkCoreDirtyInternal();
}

void LightProbeVolume::Reset()
{
	u32 numProbesX = std::max(1, mCellCount.X) + 1;
	u32 numProbesY = std::max(1, mCellCount.Y) + 1;
	u32 numProbesZ = std::max(1, mCellCount.Z) + 1;

	u32 numProbes = numProbesX * numProbesY * numProbesZ;

	// Make sure there are adequate number of probes to fill the volume
	while((u32)mProbes.size() < numProbes)
		AddProbe(Vector3::kZero);

	u32 idx = 0;
	u32 rowPitch = numProbesX;
	u32 slicePitch = numProbesX * numProbesY;

	Vector3 size = mVolume.GetSize();

	auto iter = mProbes.begin();
	while(iter != mProbes.end())
	{
		u32 x = idx % numProbesX;
		u32 y = (idx / rowPitch) % numProbesY;
		u32 z = (idx / slicePitch);

		Vector3 position = mVolume.GetMin();
		position.X += size.X * (x / (float)(numProbesX - 1));
		position.Y += size.Y * (y / (float)(numProbesY - 1));
		position.Z += size.Z * (z / (float)(numProbesZ - 1));

		iter->second.Position = position;
		iter->second.Flags = LightProbeFlags::Clean;

		++idx;
		++iter;

		if(idx >= numProbes)
			break;
	}

	// Set remaining probes to removed state
	while(iter != mProbes.end())
	{
		iter->second.Flags = LightProbeFlags::Removed;
		++iter;
	}

	MarkCoreDirtyInternal();
}

void LightProbeVolume::Clip()
{
	for(auto& entry : mProbes)
	{
		if(!mVolume.Contains(entry.second.Position))
			entry.second.Flags = LightProbeFlags::Removed;
	}

	MarkCoreDirtyInternal();
}

void LightProbeVolume::RenderProbe(u32 handle)
{
	auto iterFind = mProbes.find(handle);
	if(iterFind != mProbes.end())
	{
		if(iterFind->second.Flags == LightProbeFlags::Clean)
		{
			iterFind->second.Flags = LightProbeFlags::Dirty;

			MarkCoreDirtyInternal();
			RunRenderProbeTask();
		}
	}
}

void LightProbeVolume::RenderProbes()
{
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
		MarkCoreDirtyInternal();
		RunRenderProbeTask();
	}
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

	SPtr<ct::LightProbeVolume> coreProbeVolume = GetCore();
	auto renderProbes = [coreProbeVolume](ct::GpuCommandBufferPool& commandBufferPool)
	{
		SPtr<ct::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(ct::GpuCommandBufferCreateInformation::Create("LightProbeRendering"));
		GetProfilerGPU().BeginSample(*commandBuffer, "LightProbeRendering");
		const bool isDone = coreProbeVolume->RenderProbes(*commandBuffer, 3);
		GetProfilerGPU().EndSample(*commandBuffer, "LightProbeRendering");

		const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		gpuDevice->SubmitCommandBuffer(commandBuffer);

		return isDone;
	};

	mRendererTask = ct::RendererTask::Create("RenderLightProbes", renderProbes);

	mRendererTask->OnComplete.Connect(renderComplete);
	ct::GetRenderer()->AddTask(mRendererTask);
}

void LightProbeVolume::UpdateCoefficients()
{
	// Ensure all light probe coefficients are generated
	if(mRendererTask)
		mRendererTask->Wait();

	ct::LightProbeVolume* coreVolume = GetCore().get();

	Vector<LightProbeCoefficientInfo> coeffInfo;
	auto getSaveData = [coreVolume, &coeffInfo]()
	{
		coreVolume->GetProbeCoefficients(coeffInfo);
	};

	GetCoreThread().QueueCommand(getSaveData);
	GetCoreThread().Submit(true);

	for(auto& entry : coeffInfo)
	{
		auto iterFind = mProbes.find(entry.Handle);
		if(iterFind == mProbes.end())
			continue;

		iterFind->second.Coefficients = entry.Coefficients;
	}
}

SPtr<ct::LightProbeVolume> LightProbeVolume::GetCore() const
{
	return std::static_pointer_cast<ct::LightProbeVolume>(mCoreSpecific);
}

SPtr<LightProbeVolume> LightProbeVolume::Create(const AABox& volume, const Vector3I& cellCount)
{
	LightProbeVolume* probeVolume = new(B3DAllocate<LightProbeVolume>()) LightProbeVolume(volume, cellCount);
	SPtr<LightProbeVolume> probeVolumePtr = B3DMakeCoreFromExisting<LightProbeVolume>(probeVolume);
	probeVolumePtr->SetShared(probeVolumePtr);
	probeVolumePtr->Initialize();

	return probeVolumePtr;
}

SPtr<LightProbeVolume> LightProbeVolume::CreateEmpty()
{
	LightProbeVolume* probeVolume = new(B3DAllocate<LightProbeVolume>()) LightProbeVolume();
	SPtr<LightProbeVolume> probleVolumePtr = B3DMakeCoreFromExisting<LightProbeVolume>(probeVolume);
	probleVolumePtr->SetShared(probleVolumePtr);

	return probleVolumePtr;
}

SPtr<ct::CoreObject> LightProbeVolume::CreateCore() const
{
	ct::LightProbeVolume* handler = new(B3DAllocate<ct::LightProbeVolume>()) ct::LightProbeVolume(mProbes);
	SPtr<ct::LightProbeVolume> handlerPtr = B3DMakeSharedFromExisting<ct::LightProbeVolume>(handler);
	handlerPtr->SetShared(handlerPtr);

	return handlerPtr;
}

CoreSyncData LightProbeVolume::SyncToCore(FrameAlloc* allocator)
{
	u32 size = 0;
	u8* buffer = nullptr;

	B3DMarkAllocatorFrame();
	{
		FrameVector<std::pair<u32, ProbeInfo>> dirtyProbes;
		FrameVector<u32> removedProbes;
		for(auto& probe : mProbes)
		{
			if(probe.second.Flags == LightProbeFlags::Dirty)
			{
				dirtyProbes.push_back(std::make_pair(probe.first, probe.second));
				probe.second.Flags = LightProbeFlags::Clean;
			}
			else if(probe.second.Flags == LightProbeFlags::Removed)
			{
				removedProbes.push_back(probe.first);
				probe.second.Flags = LightProbeFlags::Empty;
			}
		}

		for(auto& probe : removedProbes)
			mProbes.erase(probe);

		u32 numDirtyProbes = (u32)dirtyProbes.size();
		u32 numRemovedProbes = (u32)removedProbes.size();

		size += CoreSyncGetSize((SceneActor&)*this);
		size += B3DRTTISize(numDirtyProbes).Bytes;
		size += B3DRTTISize(numRemovedProbes).Bytes;
		size += (sizeof(u32) + sizeof(Vector3) + sizeof(LightProbeFlags)) * numDirtyProbes;
		size += sizeof(u32) * numRemovedProbes;

		buffer = allocator->Alloc(size);
		Bitstream stream(buffer, size);

		B3DCoreSyncWrite((SceneActor&)*this, stream);
		B3DRTTIWrite(numDirtyProbes, stream);
		B3DRTTIWrite(numRemovedProbes, stream);

		for(auto& entry : dirtyProbes)
		{
			B3DRTTIWrite(entry.first, stream);
			B3DRTTIWrite(entry.second.Position, stream);
			B3DRTTIWrite(entry.second.Flags, stream);
		}

		for(auto& entry : removedProbes)
			B3DRTTIWrite(entry, stream);
	}
	B3DClearAllocatorFrame();

	return CoreSyncData(buffer, size);
}

void LightProbeVolume::MarkCoreDirtyInternal(ActorDirtyFlag dirtyFlag)
{
	MarkCoreDirty((u32)dirtyFlag);
}

RTTITypeBase* LightProbeVolume::GetRttiStatic()
{
	return LightProbeVolumeRTTI::Instance();
}

RTTITypeBase* LightProbeVolume::GetRtti() const
{
	return LightProbeVolume::GetRttiStatic();
}

namespace bs { namespace ct
{
LightProbeVolume::LightProbeVolume(const UnorderedMap<u32, bs::LightProbeVolume::ProbeInfo>& probes)
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
	GetRenderer()->NotifyLightProbeVolumeRemoved(this);
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

	GetRenderer()->NotifyLightProbeVolumeAdded(this);
	CoreObject::Initialize();
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

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

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

			const Transform& tfrm = GetTransform();
			const Vector3& position = tfrm.GetPosition();
			const Quaternion& rotation = tfrm.GetRotation();
			Vector3 transformedPos = rotation.Rotate(localPos) + position;

			GetRenderer()->CaptureSceneCubeMap(commandBuffer, cubemap, transformedPos, CaptureSettings());
			GetIBLUtility().FilterCubemapForIrradiance(commandBuffer, cubemap, mCoefficients, probeInfo.BufferIdx);

			probeInfo.Flags = LightProbeFlags::Clean;
			numProbeUpdates++;
		}

		if(maxProbes != 0 && numProbeUpdates >= maxProbes)
			break;
	}

	GetRenderer()->NotifyLightProbeVolumeUpdated(this);

	return mFirstDirtyProbe == (u32)mProbeInfos.size();
}

void LightProbeVolume::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	bool oldIsActive = mActive;

	B3DCoreSyncRead((SceneActor&)*this, stream);

	u32 numDirtyProbes, numRemovedProbes;
	B3DRTTIRead(numDirtyProbes, stream);
	B3DRTTIRead(numRemovedProbes, stream);

	for(u32 i = 0; i < numDirtyProbes; ++i)
	{
		u32 handle;
		B3DRTTIRead(handle, stream);

		Vector3 position;
		B3DRTTIRead(position, stream);

		LightProbeFlags flags;
		B3DRTTIRead(flags, stream);

		auto iterFind = mProbeMap.find(handle);
		if(iterFind != mProbeMap.end())
		{
			// Update existing probe information
			u32 compactIdx = iterFind->second;

			mProbeInfos[compactIdx].Flags = LightProbeFlags::Dirty;
			mProbePositions[compactIdx] = position;

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
				info.Handle = handle;

				mProbeInfos.push_back(info);
				mProbePositions.push_back(position);
			}
			else // No empty slot, add a new one
			{
				LightProbeInfo& info = mProbeInfos[compactIdx];
				info.Flags = LightProbeFlags::Dirty;
				info.Handle = handle;

				mProbePositions[compactIdx] = position;
			}

			mProbeMap[handle] = compactIdx;
			mFirstDirtyProbe = std::min(compactIdx, mFirstDirtyProbe);
		}
	}

	// Mark slots for removed probes as empty, and move them back to the end of the array
	for(u32 i = 0; i < numRemovedProbes; ++i)
	{
		u32 idx;
		B3DRTTIRead(idx, stream);

		auto iterFind = mProbeMap.find(idx);
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
					std::swap(mProbeInfos[i], mProbeInfos[lastSearchIdx]);
					std::swap(mProbePositions[i], mProbePositions[lastSearchIdx]);

					mProbeMap[mProbeInfos[lastSearchIdx].Handle] = i;
					break;
				}

				lastSearchIdx--;
			}

			mProbeMap.erase(iterFind);
		}
	}

	if(oldIsActive != mActive)
	{
		if(mActive)
			GetRenderer()->NotifyLightProbeVolumeAdded(this);
		else
			GetRenderer()->NotifyLightProbeVolumeRemoved(this);
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
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
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
