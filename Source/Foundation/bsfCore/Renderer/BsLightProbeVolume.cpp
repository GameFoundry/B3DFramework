//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsLightProbeVolume.h"
#include "Private/RTTI/BsLightProbeVolumeRTTI.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsLight.h"
#include "Image/BsTexture.h"
#include "Renderer/BsIBLUtility.h"
#include "Scene/BsSceneObject.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	LightProbeVolume::LightProbeVolume(const AABox& volume, const Vector3I& cellCount)
		:mVolume(volume), mCellCount(cellCount)
	{
		reset();
	}

	LightProbeVolume::~LightProbeVolume()
	{
		if (mRendererTask)
			mRendererTask->cancel();
	}

	UINT32 LightProbeVolume::addProbe(const Vector3& position)
	{
		UINT32 handle = mNextProbeId++;
		mProbes[handle] = ProbeInfo(LightProbeFlags::Clean, position);

		_markCoreDirty();
		return handle;
	}

	void LightProbeVolume::removeProbe(UINT32 handle)
	{
		auto iterFind = mProbes.find(handle);
		if (iterFind != mProbes.end() && mProbes.size() > 4)
		{
			iterFind->second.flags = LightProbeFlags::Removed;
			_markCoreDirty();
		}
	}

	void LightProbeVolume::setProbePosition(UINT32 handle, const Vector3& position)
	{
		auto iterFind = mProbes.find(handle);
		if (iterFind != mProbes.end())
		{
			iterFind->second.position = position;
			_markCoreDirty();
		}
	}

	Vector3 LightProbeVolume::getProbePosition(UINT32 handle) const
	{
		auto iterFind = mProbes.find(handle);
		if (iterFind != mProbes.end())
			return iterFind->second.position;
		
		return Vector3::ZERO;
	}

	Vector<LightProbeInfo> LightProbeVolume::getProbes() const
	{
		Vector<LightProbeInfo> output;

		for(auto& entry : mProbes)
		{
			if (entry.second.flags == LightProbeFlags::Removed || entry.second.flags == LightProbeFlags::Empty)
				continue;

			LightProbeInfo info;
			info.position = entry.second.position;
			info.handle = entry.first;
			info.shCoefficients = entry.second.coefficients;

			output.push_back(info);
		}

		return output;		
	}

	void LightProbeVolume::resize(const AABox& volume, const Vector3I& cellCount)
	{
		UINT32 numProbesX = std::max(1, mCellCount.x) + 1;
		UINT32 numProbesY = std::max(1, mCellCount.y) + 1;
		UINT32 numProbesZ = std::max(1, mCellCount.z) + 1;

		Vector3 size = mVolume.getSize();
		for(UINT32 z = 0; z < numProbesZ; ++z)
		{
			for(UINT32 y = 0; y < numProbesY; ++y)
			{
				for(UINT32 x = 0; x < numProbesX; ++x)
				{
					Vector3 position = mVolume.getMin();
					position.x += size.x * (x / (float)numProbesX);
					position.y += size.y * (y / (float)numProbesY);
					position.z += size.z * (z / (float)numProbesZ);

					if (mVolume.contains(position))
						continue;

					addProbe(position);
				}
			}
		}

		mVolume = volume;
		mCellCount = cellCount;

		_markCoreDirty();
	}

	void LightProbeVolume::reset()
	{
		UINT32 numProbesX = std::max(1, mCellCount.x) + 1;
		UINT32 numProbesY = std::max(1, mCellCount.y) + 1;
		UINT32 numProbesZ = std::max(1, mCellCount.z) + 1;

		UINT32 numProbes = numProbesX * numProbesY * numProbesZ;

		// Make sure there are adequate number of probes to fill the volume
		while((UINT32)mProbes.size() < numProbes)
			addProbe(Vector3::ZERO);

		UINT32 idx = 0;
		UINT32 rowPitch = numProbesX;
		UINT32 slicePitch = numProbesX * numProbesY;

		Vector3 size = mVolume.getSize();

		auto iter = mProbes.begin();
		while (iter != mProbes.end())
		{
			UINT32 x = idx % numProbesX;
			UINT32 y = (idx / rowPitch) % numProbesY;
			UINT32 z = (idx / slicePitch);

			Vector3 position = mVolume.getMin();
			position.x += size.x * (x / (float)(numProbesX - 1));
			position.y += size.y * (y / (float)(numProbesY - 1));
			position.z += size.z * (z / (float)(numProbesZ - 1));

			iter->second.position = position;
			iter->second.flags = LightProbeFlags::Clean;

			++idx;
			++iter;

			if (idx >= numProbes)
				break;
		}

		// Set remaining probes to removed state
		while(iter != mProbes.end())
		{
			iter->second.flags = LightProbeFlags::Removed;
			++iter;
		}

		_markCoreDirty();
	}

	void LightProbeVolume::clip()
	{
		for (auto& entry : mProbes)
		{
			if (!mVolume.contains(entry.second.position))
				entry.second.flags = LightProbeFlags::Removed;
		}

		_markCoreDirty();
	}

	void LightProbeVolume::renderProbe(UINT32 handle)
	{
		auto iterFind = mProbes.find(handle);
		if (iterFind != mProbes.end())
		{
			if (iterFind->second.flags == LightProbeFlags::Clean)
			{
				iterFind->second.flags = LightProbeFlags::Dirty;

				_markCoreDirty();
				runRenderProbeTask();
			}
		}
	}

	void LightProbeVolume::renderProbes()
	{
		bool anyModified = false;
		for(auto& entry : mProbes)
		{
			if (entry.second.flags == LightProbeFlags::Clean)
			{
				entry.second.flags = LightProbeFlags::Dirty;
				anyModified = true;
			}
		}

		if (anyModified)
		{
			_markCoreDirty();
			runRenderProbeTask();
		}
	}

	void LightProbeVolume::runRenderProbeTask()
	{
		// If a task is already running cancel it
		// Note: If the task is just about to start processing, cancelling it will skip the update this frame
		// (which might be fine if we just changed positions of dirty probes it was about to update, but it might also
		// waste a frame if those positions needed to be updated anyway). For now I'm ignoring it as it seems like a rare
		// enough situation, plus it's one that will only happen during development time.
		if (mRendererTask)
			mRendererTask->cancel();

		auto renderComplete = [this]()
		{
			mRendererTask = nullptr;
		};

		SPtr<ct::LightProbeVolume> coreProbeVolume = getCore();
		auto renderProbes = [coreProbeVolume]()
		{
			return coreProbeVolume->renderProbes(3);
		};

		mRendererTask = ct::RendererTask::create("RenderLightProbes", renderProbes);

		mRendererTask->onComplete.connect(renderComplete);
		ct::gRenderer()->addTask(mRendererTask);
	}

	void LightProbeVolume::updateCoefficients()
	{
		// Ensure all light probe coefficients are generated
		if (mRendererTask)
			mRendererTask->wait();

		ct::LightProbeVolume* coreVolume = getCore().get();

		Vector<LightProbeCoefficientInfo> coeffInfo;
		auto getSaveData = [coreVolume, &coeffInfo]()
		{
			coreVolume->getProbeCoefficients(coeffInfo);
		};

		gCoreThread().queueCommand(getSaveData);
		gCoreThread().submit(true);

		for(auto& entry : coeffInfo)
		{
			auto iterFind = mProbes.find(entry.handle);
			if (iterFind == mProbes.end())
				continue;

			iterFind->second.coefficients = entry.coefficients;
		}
	}

	SPtr<ct::LightProbeVolume> LightProbeVolume::getCore() const
	{
		return std::static_pointer_cast<ct::LightProbeVolume>(mCoreSpecific);
	}

	SPtr<LightProbeVolume> LightProbeVolume::create(const AABox& volume, const Vector3I& cellCount)
	{
		LightProbeVolume* probeVolume = new (bs_alloc<LightProbeVolume>()) LightProbeVolume(volume, cellCount);
		SPtr<LightProbeVolume> probeVolumePtr = bs_core_ptr<LightProbeVolume>(probeVolume);
		probeVolumePtr->_setThisPtr(probeVolumePtr);
		probeVolumePtr->initialize();

		return probeVolumePtr;
	}

	SPtr<LightProbeVolume> LightProbeVolume::createEmpty()
	{
		LightProbeVolume* probeVolume = new (bs_alloc<LightProbeVolume>()) LightProbeVolume();
		SPtr<LightProbeVolume> probleVolumePtr = bs_core_ptr<LightProbeVolume>(probeVolume);
		probleVolumePtr->_setThisPtr(probleVolumePtr);

		return probleVolumePtr;
	}

	SPtr<ct::CoreObject> LightProbeVolume::createCore() const
	{
		ct::LightProbeVolume* handler = new (bs_alloc<ct::LightProbeVolume>()) ct::LightProbeVolume(mProbes);
		SPtr<ct::LightProbeVolume> handlerPtr = bs_shared_ptr<ct::LightProbeVolume>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	CoreSyncData LightProbeVolume::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		UINT8* buffer = nullptr;

		bs_frame_mark();
		{
			FrameVector<std::pair<UINT32, ProbeInfo>> dirtyProbes;
			FrameVector<UINT32> removedProbes;
			for (auto& probe : mProbes)
			{
				if (probe.second.flags == LightProbeFlags::Dirty)
				{
					dirtyProbes.push_back(std::make_pair(probe.first, probe.second));
					probe.second.flags = LightProbeFlags::Clean;
				}
				else if (probe.second.flags == LightProbeFlags::Removed)
				{
					removedProbes.push_back(probe.first);
					probe.second.flags = LightProbeFlags::Empty;
				}
			}

			for (auto& probe : removedProbes)
				mProbes.erase(probe);

			UINT32 numDirtyProbes = (UINT32)dirtyProbes.size();
			UINT32 numRemovedProbes = (UINT32)removedProbes.size();

			size += csync_size((SceneActor&)*this);
			size += rtti_size(numDirtyProbes).bytes;
			size += rtti_size(numRemovedProbes).bytes;
			size += (sizeof(UINT32) + sizeof(Vector3) + sizeof(LightProbeFlags)) * numDirtyProbes;
			size += sizeof(UINT32) * numRemovedProbes;

			buffer = allocator->alloc(size);
			Bitstream stream(buffer, size);

			csync_write((SceneActor&)*this, stream);
			rtti_write(numDirtyProbes, stream);
			rtti_write(numRemovedProbes, stream);

			for (auto& entry : dirtyProbes)
			{
				rtti_write(entry.first, stream);
				rtti_write(entry.second.position, stream);
				rtti_write(entry.second.flags, stream);
			}

			for(auto& entry : removedProbes)
				rtti_write(entry, stream);
		}
		bs_frame_clear();

		return CoreSyncData(buffer, size);
	}

	void LightProbeVolume::_markCoreDirty(ActorDirtyFlag dirtyFlag)
	{
		markCoreDirty((UINT32)dirtyFlag);
	}

	RTTITypeBase* LightProbeVolume::getRTTIStatic()
	{
		return LightProbeVolumeRTTI::instance();
	}

	RTTITypeBase* LightProbeVolume::getRTTI() const
	{
		return LightProbeVolume::getRTTIStatic();
	}

	namespace ct
	{
	LightProbeVolume::LightProbeVolume(const UnorderedMap<UINT32, bs::LightProbeVolume::ProbeInfo>& probes)
	{
		mInitCoefficients.resize(probes.size());
		mProbePositions.resize(probes.size());
		mProbeInfos.resize(probes.size());

		UINT32 probeIdx = 0;
		for(auto& entry : probes)
		{
			mProbeMap[entry.first] = probeIdx;
			mProbePositions[probeIdx] = entry.second.position;
			
			LightProbeInfo probeInfo;
			probeInfo.flags = LightProbeFlags::Dirty;
			probeInfo.bufferIdx = probeIdx;
			probeInfo.handle = entry.first;

			mProbeInfos[probeIdx] = probeInfo;
			mInitCoefficients[probeIdx] = entry.second.coefficients;

			probeIdx++;
		}
	}

	LightProbeVolume::~LightProbeVolume()
	{
		gRenderer()->notifyLightProbeVolumeRemoved(this);
	}

	void LightProbeVolume::initialize()
	{
		// Set SH coefficients loaded from the file
		UINT32 numCoefficients = (UINT32)mInitCoefficients.size();
		assert(mInitCoefficients.size() == mProbeMap.size());

		resizeCoefficientTexture(std::max(32U, numCoefficients));

		SPtr<PixelData> coeffData = mCoefficients->getProperties().allocBuffer(0, 0);
		coeffData->setColors(Color::ZERO);

		UINT32 probesPerRow = coeffData->getWidth() / 9;
		UINT32 probeIdx = 0;
		for(UINT32 y = 0; y < coeffData->getHeight(); ++y)
		{
			for(UINT32 x = 0; x < probesPerRow; ++x)
			{
				if(probeIdx >= numCoefficients)
					break;

				for(UINT32 i = 0; i < 9; i++)
				{
					Color value;
					value.r = mInitCoefficients[probeIdx].coeffsR[i];
					value.g = mInitCoefficients[probeIdx].coeffsG[i];
					value.b = mInitCoefficients[probeIdx].coeffsB[i];

					coeffData->setColorAt(value, x * 9, y);
				}

				probeIdx++;
			}
		}

		mCoefficients->writeData(*coeffData, 0, 0, true);
		mInitCoefficients.clear();

		gRenderer()->notifyLightProbeVolumeAdded(this);
		CoreObject::initialize();
	}

	bool LightProbeVolume::renderProbes(UINT32 maxProbes)
	{
		// Probe map only contains active probes
		UINT32 numUsedProbes = (UINT32)mProbeMap.size();
		if(numUsedProbes > mCoeffBufferSize)
			resizeCoefficientTexture(std::max(32U, numUsedProbes * 2));

		UINT32 numProbeUpdates = 0;
		for (; mFirstDirtyProbe < (UINT32)mProbeInfos.size(); ++mFirstDirtyProbe)
		{
			LightProbeInfo& probeInfo = mProbeInfos[mFirstDirtyProbe];

			if(probeInfo.flags == LightProbeFlags::Dirty)
			{
				TEXTURE_DESC cubemapDesc;
				cubemapDesc.type = TEX_TYPE_CUBE_MAP;
				cubemapDesc.format = PF_RGBA16F;
				cubemapDesc.width = 256; // Note: Test different sizes and their effect on quality
				cubemapDesc.height = 256;
				cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

				SPtr<Texture> cubemap = Texture::create(cubemapDesc);

				Vector3 localPos = mProbePositions[mFirstDirtyProbe];

				const Transform& tfrm = getTransform();
				const Vector3& position = tfrm.getPosition();
				const Quaternion& rotation = tfrm.getRotation();
				Vector3 transformedPos = rotation.rotate(localPos) + position;

				gRenderer()->captureSceneCubeMap(cubemap, transformedPos, CaptureSettings());
				gIBLUtility().filterCubemapForIrradiance(cubemap, mCoefficients, probeInfo.bufferIdx);

				probeInfo.flags = LightProbeFlags::Clean;
				numProbeUpdates++;
			}

			if (maxProbes != 0 && numProbeUpdates >= maxProbes)
				break;
		}

		gRenderer()->notifyLightProbeVolumeUpdated(this);

		return mFirstDirtyProbe == (UINT32)mProbeInfos.size();
	}

	void LightProbeVolume::syncToCore(const CoreSyncData& data)
	{
		Bitstream stream(data.getBuffer(), data.getBufferSize());

		bool oldIsActive = mActive;

		csync_read((SceneActor&)*this, stream);

		UINT32 numDirtyProbes, numRemovedProbes;
		rtti_read(numDirtyProbes, stream);
		rtti_read(numRemovedProbes, stream);

		for (UINT32 i = 0; i < numDirtyProbes; ++i)
		{
			UINT32 handle;
			rtti_read(handle, stream);

			Vector3 position;
			rtti_read(position, stream);

			LightProbeFlags flags;
			rtti_read(flags, stream);

			auto iterFind = mProbeMap.find(handle);
			if(iterFind != mProbeMap.end())
			{
				// Update existing probe information
				UINT32 compactIdx = iterFind->second;
				
				mProbeInfos[compactIdx].flags = LightProbeFlags::Dirty;
				mProbePositions[compactIdx] = position;

				mFirstDirtyProbe = std::min(compactIdx, mFirstDirtyProbe);
			}
			else // Add a new probe
			{
				// Empty slots always start at a specific index because we always move them to the back of the array
				UINT32 emptyProbeStartIdx = (UINT32)mProbeMap.size();
				UINT32 numProbes = (UINT32)mProbeInfos.size();

				// Find an empty slot to place the probe information at
				UINT32 compactIdx = -1;
				for(UINT32 j = emptyProbeStartIdx; j < numProbes; ++j)
				{
					if(mProbeInfos[j].flags == LightProbeFlags::Empty)
					{
						compactIdx = j;
						break;
					}
				}

				// Found an empty slot
				if (compactIdx == (UINT32)-1)
				{
					compactIdx = (UINT32)mProbeInfos.size();

					LightProbeInfo info;
					info.flags = LightProbeFlags::Dirty;
					info.bufferIdx = compactIdx;
					info.handle = handle;

					mProbeInfos.push_back(info);
					mProbePositions.push_back(position);
				}
				else // No empty slot, add a new one
				{
					LightProbeInfo& info = mProbeInfos[compactIdx];
					info.flags = LightProbeFlags::Dirty;
					info.handle = handle;

					mProbePositions[compactIdx] = position;
				}

				mProbeMap[handle] = compactIdx;
				mFirstDirtyProbe = std::min(compactIdx, mFirstDirtyProbe);
			}
		}

		// Mark slots for removed probes as empty, and move them back to the end of the array
		for (UINT32 i = 0; i < numRemovedProbes; ++i)
		{
			UINT32 idx;
			rtti_read(idx, stream);

			auto iterFind = mProbeMap.find(idx);
			if(iterFind != mProbeMap.end())
			{
				UINT32 compactIdx = iterFind->second;
				
				LightProbeInfo& info = mProbeInfos[compactIdx];
				info.flags = LightProbeFlags::Empty;

				// Move the empty info to the back of the array so all non-empty probes are contiguous
				// Search from back to current index, and find first non-empty probe to switch switch
				UINT32 lastSearchIdx = (UINT32)mProbeInfos.size() - 1;
				while (lastSearchIdx >= (UINT32)compactIdx)
				{
					LightProbeFlags flags = mProbeInfos[lastSearchIdx].flags;
					if (flags != LightProbeFlags::Empty)
					{
						std::swap(mProbeInfos[i], mProbeInfos[lastSearchIdx]);
						std::swap(mProbePositions[i], mProbePositions[lastSearchIdx]);

						mProbeMap[mProbeInfos[lastSearchIdx].handle] = i;
						break;
					}

					lastSearchIdx--;
				}
				
				mProbeMap.erase(iterFind);
			}
		}

		if (oldIsActive != mActive)
		{
			if (mActive)
				gRenderer()->notifyLightProbeVolumeAdded(this);
			else
				gRenderer()->notifyLightProbeVolumeRemoved(this);
		}
	}

	void LightProbeVolume::getProbeCoefficients(Vector<LightProbeCoefficientInfo>& output) const
	{
		UINT32 numActiveProbes = (UINT32)mProbeMap.size();
		if (numActiveProbes == 0)
			return;

		output.resize(numActiveProbes);

		LightProbeSHCoefficients* coefficients = bs_stack_alloc<LightProbeSHCoefficients>(numActiveProbes);

		SPtr<PixelData> coeffData = mCoefficients->getProperties().allocBuffer(0, 0);
		mCoefficients->readData(*coeffData);

		UINT32 probesPerRow = coeffData->getWidth() / 9;
		UINT32 probeIdx = 0;
		for(UINT32 y = 0; y < coeffData->getHeight(); ++y)
		{
			for(UINT32 x = 0; x < probesPerRow; ++x)
			{
				if(probeIdx >= numActiveProbes)
					break;

				for(UINT32 i = 0; i < 9; i++)
				{
					Color value = coeffData->getColorAt(x * 9, y);

					coefficients[probeIdx].coeffsR[i] = value.r;
					coefficients[probeIdx].coeffsG[i] = value.g;
					coefficients[probeIdx].coeffsB[i] = value.b;
				}

				probeIdx++;
			}
		}

		for(UINT32 i = 0; i < numActiveProbes; ++i)
		{
			output[i].coefficients = coefficients[mProbeInfos[i].bufferIdx];
			output[i].handle = mProbeInfos[i].handle;
		}

		bs_stack_free(coefficients);
	}

	void LightProbeVolume::resizeCoefficientTexture(UINT32 count)
	{
		Vector2I texSize = IBLUtility::getSHCoeffTextureSize(count, 3);

		TEXTURE_DESC desc;
		desc.width = (UINT32)texSize.x;
		desc.height = (UINT32)texSize.y;
		desc.usage = TU_LOADSTORE | TU_RENDERTARGET;
		desc.format = PF_RGBA32F;

		SPtr<Texture> newTexture = Texture::create(desc);

		if (mCoefficients)
			mCoefficients->copy(newTexture);

		mCoefficients = newTexture;
		mCoeffBufferSize = count;
	}
}}
