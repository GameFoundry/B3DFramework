//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimation.h"
#include "BsAnimationManager.h"
#include "BsAnimationClip.h"

namespace BansheeEngine
{
	AnimationClipInfo::AnimationClipInfo()
		: fadeDirection(0.0f), fadeTime(0.0f), fadeLength(0.0f), layerIdx(0), curveVersion(0), stateIdx(0)
	{ }

	AnimationClipInfo::AnimationClipInfo(const HAnimationClip& clip)
		: fadeDirection(0.0f), fadeTime(0.0f), fadeLength(0.0f), clip(clip), curveVersion(0), layerIdx(0), stateIdx(0)
	{ }

	Blend1DInfo::Blend1DInfo(UINT32 numClips)
		: clips(nullptr), numClips(numClips)
	{
		if (numClips > 0)
			clips = bs_newN<BlendClipInfo>(numClips);
	}

	Blend1DInfo::~Blend1DInfo()
	{
		if(clips != nullptr)
			bs_deleteN(clips, numClips);
	}

	AnimationProxy::AnimationProxy(UINT64 id)
		:id(id), layers(nullptr), numLayers(0), genericCurveOutputs(nullptr)
	{ }

	AnimationProxy::~AnimationProxy()
	{
		clear();
	}

	void AnimationProxy::clear()
	{
		if (layers == nullptr)
			return;

		for(UINT32 i = 0; i < numLayers; i++)
		{
			AnimationStateLayer& layer = layers[i];
			for(UINT32 j = 0; j < layer.numStates; j++)
			{
				AnimationState& state = layer.states[j];

				if(state.curves != nullptr)
				{
					{
						UINT32 numCurves = (UINT32)state.curves->position.size();
						for (UINT32 k = 0; k < numCurves; k++)
							state.positionCaches[k].~TCurveCache();
					}

					{
						UINT32 numCurves = (UINT32)state.curves->rotation.size();
						for (UINT32 k = 0; k < numCurves; k++)
							state.rotationCaches[k].~TCurveCache();
					}

					{
						UINT32 numCurves = (UINT32)state.curves->scale.size();
						for (UINT32 k = 0; k < numCurves; k++)
							state.scaleCaches[k].~TCurveCache();
					}

					{
						UINT32 numCurves = (UINT32)state.curves->generic.size();
						for (UINT32 k = 0; k < numCurves; k++)
							state.genericCaches[k].~TCurveCache();
					}
				}

				if(skeleton != nullptr)
				{
					UINT32 numBones = skeleton->getNumBones();
					for (UINT32 k = 0; k < numBones; k++)
						state.boneToCurveMapping[k].~AnimationCurveMapping();
				}

				state.~AnimationState();
			}

			layer.~AnimationStateLayer();
		}

		// All the memory is part of the same buffer, so we only need to free the first element
		bs_free(layers);
		layers = nullptr;
		genericCurveOutputs = nullptr;

		numLayers = 0;
	}

	void AnimationProxy::rebuild(const SPtr<Skeleton>& skeleton, Vector<AnimationClipInfo>& clipInfos)
	{
		this->skeleton = skeleton;

		// Note: I could avoid having a separate allocation for LocalSkeletonPose and use the same buffer as the rest
		// of AnimationProxy
		if (skeleton != nullptr)
			localPose = LocalSkeletonPose(skeleton->getNumBones());
		else
		{
			UINT32 numPosCurves = 0;
			UINT32 numRotCurves = 0;
			UINT32 numScaleCurves = 0;

			// Note: I'm recalculating this both here and in follow-up rebuild() call, it could be avoided.
			for (auto& clipInfo : clipInfos)
			{
				if (!clipInfo.clip.isLoaded())
					continue;

				SPtr<AnimationCurves> curves = clipInfo.clip->getCurves();
				numPosCurves += (UINT32)curves->position.size();
				numRotCurves += (UINT32)curves->rotation.size();
				numScaleCurves += (UINT32)curves->scale.size();
			}

			localPose = LocalSkeletonPose(numPosCurves, numRotCurves, numScaleCurves);
		}

		rebuild(clipInfos);
	}

	void AnimationProxy::rebuild(Vector<AnimationClipInfo>& clipInfos)
	{
		clear();

		bs_frame_mark();
		{
			FrameVector<AnimationStateLayer> tempLayers;
			for (auto& clipInfo : clipInfos)
			{
				UINT32 layer = clipInfo.state.layer;
				if (layer == (UINT32)-1)
					layer = 0;
				else
					layer += 1;

				auto iterFind = std::find_if(tempLayers.begin(), tempLayers.end(), 
					[&](auto& x)
				{
					return x.index == layer;
				});

				if (iterFind == tempLayers.end())
				{
					tempLayers.push_back(AnimationStateLayer());
					AnimationStateLayer& newLayer = tempLayers.back();

					newLayer.index = layer;
					newLayer.additive = clipInfo.clip.isLoaded() && clipInfo.clip->isAdditive();
				}
			}

			std::sort(tempLayers.begin(), tempLayers.end(), 
				[&](auto& x, auto& y)
			{
				return x.index < y.index;
			});

			numLayers = (UINT32)tempLayers.size();
			UINT32 numClips = (UINT32)clipInfos.size();
			UINT32 numBones;
			
			if (skeleton != nullptr)
				numBones = skeleton->getNumBones();
			else
				numBones = 0;

			UINT32 numPosCurves = 0;
			UINT32 numRotCurves = 0;
			UINT32 numScaleCurves = 0;
			UINT32 numGenCurves = 0;

			for (auto& clipInfo : clipInfos)
			{
				if (!clipInfo.clip.isLoaded())
					continue;

				SPtr<AnimationCurves> curves = clipInfo.clip->getCurves();
				numPosCurves += (UINT32)curves->position.size();
				numRotCurves += (UINT32)curves->rotation.size();
				numScaleCurves += (UINT32)curves->scale.size();
				numGenCurves += (UINT32)curves->generic.size();
			}

			UINT32 numBoneMappings = numBones * numClips;
			UINT32 layersSize = sizeof(AnimationStateLayer) * numLayers;
			UINT32 clipsSize = sizeof(AnimationState) * numClips;
			UINT32 boneMappingSize = numBoneMappings * sizeof(AnimationCurveMapping);
			UINT32 posCacheSize = numPosCurves * sizeof(TCurveCache<Vector3>);
			UINT32 rotCacheSize = numRotCurves * sizeof(TCurveCache<Quaternion>);
			UINT32 scaleCacheSize = numScaleCurves * sizeof(TCurveCache<Vector3>);
			UINT32 genCacheSize = numGenCurves * sizeof(TCurveCache<float>);
			UINT32 genericCurveOutputSize = numGenCurves * sizeof(float);

			UINT8* data = (UINT8*)bs_alloc(layersSize + clipsSize + boneMappingSize + posCacheSize + rotCacheSize + 
				scaleCacheSize + genCacheSize + genericCurveOutputSize);

			layers = (AnimationStateLayer*)data;
			memcpy(layers, tempLayers.data(), layersSize);
			data += layersSize;

			AnimationState* states = (AnimationState*)data;
			for(UINT32 i = 0; i < numClips; i++)
				new (&states[i]) AnimationState();

			data += clipsSize;

			AnimationCurveMapping* boneMappings = (AnimationCurveMapping*)data;
			for (UINT32 i = 0; i < numBoneMappings; i++)
				new (&boneMappings[i]) AnimationCurveMapping();

			data += boneMappingSize;

			TCurveCache<Vector3>* posCache = (TCurveCache<Vector3>*)data;
			for (UINT32 i = 0; i < numPosCurves; i++)
				new (&posCache[i]) TCurveCache<Vector3>();

			data += posCacheSize;

			TCurveCache<Quaternion>* rotCache = (TCurveCache<Quaternion>*)data;
			for (UINT32 i = 0; i < numRotCurves; i++)
				new (&rotCache[i]) TCurveCache<Quaternion>();

			data += rotCacheSize;

			TCurveCache<Vector3>* scaleCache = (TCurveCache<Vector3>*)data;
			for (UINT32 i = 0; i < numScaleCurves; i++)
				new (&scaleCache[i]) TCurveCache<Vector3>();

			data += scaleCacheSize;

			TCurveCache<float>* genCache = (TCurveCache<float>*)data;
			for (UINT32 i = 0; i < numGenCurves; i++)
				new (&genCache[i]) TCurveCache<float>();

			data += genCacheSize;

			genericCurveOutputs = (float*)data;

			UINT32 curLayerIdx = 0;
			UINT32 curStateIdx = 0;

			for(UINT32 i = 0; i < numLayers; i++)
			{
				AnimationStateLayer& layer = layers[i];

				layer.states = &states[curStateIdx];
				layer.numStates = 0;

				UINT32 localStateIdx = 0;
				for(auto& clipInfo : clipInfos)
				{
					UINT32 clipLayer = clipInfo.state.layer;
					if (clipLayer == (UINT32)-1)
						clipLayer = 0;
					else
						clipLayer += 1;

					if (clipLayer != layer.index)
						continue;

					AnimationState& state = states[curStateIdx];
					state.loop = clipInfo.state.wrapMode == AnimWrapMode::Loop;
					state.time = clipInfo.state.time;

					// Calculate weight if fading is active
					float weight = clipInfo.state.weight;

					//// Assumes time is clamped to [0, fadeLength] and fadeLength != 0
					if(clipInfo.fadeDirection < 0.0f)
					{
						float t = clipInfo.fadeTime / clipInfo.fadeLength;
						weight *= (1.0f - t);
					}
					else if(clipInfo.fadeDirection > 0.0f)
					{
						float t = clipInfo.fadeTime / clipInfo.fadeLength;
						weight *= t;
					}

					state.weight = weight;

					// Set up individual curves and their caches
					bool isClipValid = clipInfo.clip.isLoaded();
					if(isClipValid)
						state.curves = clipInfo.clip->getCurves();
					else
					{
						static SPtr<AnimationCurves> zeroCurves = bs_shared_ptr_new<AnimationCurves>();
						state.curves = zeroCurves;
					}

					state.positionCaches = posCache;
					posCache += state.curves->position.size();

					state.rotationCaches = rotCache;
					rotCache += state.curves->rotation.size();

					state.scaleCaches = scaleCache;
					scaleCache += state.curves->scale.size();

					state.genericCaches = genCache;
					genCache += state.curves->generic.size();

					clipInfo.layerIdx = curLayerIdx;
					clipInfo.stateIdx = localStateIdx;

					if(isClipValid)
						clipInfo.curveVersion = clipInfo.clip->getVersion();

					// Set up bone mapping
					if (skeleton != nullptr)
					{
						state.boneToCurveMapping = &boneMappings[curStateIdx * numBones];

						if (isClipValid)
						{
							clipInfo.clip->getBoneMapping(*skeleton, state.boneToCurveMapping);
						}
						else
						{
							AnimationCurveMapping emptyMapping = { (UINT32)-1, (UINT32)-1, (UINT32)-1 };

							for (UINT32 i = 0; i < numBones; i++)
								state.boneToCurveMapping[i] = emptyMapping;
						}
					}
					else
						state.boneToCurveMapping = nullptr;

					layer.numStates++;
					curStateIdx++;
					localStateIdx++;
				}

				curLayerIdx++;

				// Must be larger than zero otherwise the layer.states pointer will point to data held by some other layer
				assert(layer.numStates > 0);
			}			
		}
		bs_frame_clear();
	}

	void AnimationProxy::updateValues(const Vector<AnimationClipInfo>& clipInfos)
	{
		for(auto& clipInfo : clipInfos)
		{
			AnimationState& state = layers[clipInfo.layerIdx].states[clipInfo.stateIdx];

			state.loop = clipInfo.state.wrapMode == AnimWrapMode::Loop;
			state.weight = clipInfo.state.weight;
			state.time = clipInfo.state.time;
		}
	}

	void AnimationProxy::updateTime(const Vector<AnimationClipInfo>& clipInfos)
	{
		for (auto& clipInfo : clipInfos)
		{
			AnimationState& state = layers[clipInfo.layerIdx].states[clipInfo.stateIdx];
			state.time = clipInfo.state.time;
		}
	}

	Animation::Animation()
		: mDefaultWrapMode(AnimWrapMode::Loop), mDefaultSpeed(1.0f), mDirty(AnimDirtyStateFlag::Skeleton)
	{
		mId = AnimationManager::instance().registerAnimation(this);
		mAnimProxy = bs_shared_ptr_new<AnimationProxy>(mId);
	}

	Animation::~Animation()
	{
		AnimationManager::instance().unregisterAnimation(mId);
	}

	void Animation::setSkeleton(const SPtr<Skeleton>& skeleton)
	{
		mSkeleton = skeleton;
		mDirty |= AnimDirtyStateFlag::Skeleton;
	}

	void Animation::setWrapMode(AnimWrapMode wrapMode)
	{
		mDefaultWrapMode = wrapMode;

		for (auto& clipInfo : mClipInfos)
			clipInfo.state.wrapMode = wrapMode;

		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::setSpeed(float speed)
	{
		mDefaultSpeed = speed;

		for (auto& clipInfo : mClipInfos)
		{
			// Special case: Ignore non-moving ones
			if(!clipInfo.state.stopped)
				clipInfo.state.speed = speed;
		}

		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::play(const HAnimationClip& clip)
	{
		AnimationClipInfo* clipInfo = addClip(clip, (UINT32)-1);
		if(clipInfo != nullptr)
		{
			clipInfo->state.time = 0.0f;
			clipInfo->state.speed = mDefaultSpeed;
			clipInfo->state.weight = 1.0f;
			clipInfo->state.wrapMode = mDefaultWrapMode;
		}

		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::blendAdditive(const HAnimationClip& clip, float weight, float fadeLength, UINT32 layer)
	{
		if(clip != nullptr && !clip->isAdditive())
		{
			LOGWRN("blendAdditive() called with a clip that doesn't contain additive animation. Ignoring.");

			// Stop any clips on this layer, even if invalid
			HAnimationClip nullClip;
			addClip(nullClip, layer);

			return;
		}

		AnimationClipInfo* clipInfo = addClip(clip, layer);
		if (clipInfo != nullptr)
		{
			clipInfo->state.time = 0.0f;
			clipInfo->state.speed = mDefaultSpeed;
			clipInfo->state.weight = weight;
			clipInfo->state.wrapMode = mDefaultWrapMode;

			if(fadeLength > 0.0f)
			{
				clipInfo->fadeDirection = 1.0f;
				clipInfo->fadeTime = 0.0f;
				clipInfo->fadeLength = fadeLength;
			}

			mDirty |= AnimDirtyStateFlag::Value;
		}
	}

	void Animation::blend1D(const Blend1DInfo& info, float t)
	{
		if (info.numClips == 0)
			return;

		// Find valid range
		float startPos = 0.0f;
		float endPos = 0.0f;

		for (UINT32 i = 0; i < info.numClips; i++)
		{
			startPos = std::min(startPos, info.clips[i].position);
			endPos = std::min(endPos, info.clips[i].position);
		}

		float length = endPos - startPos;
		if(Math::approxEquals(length, 0.0f) || info.numClips < 2)
		{
			play(info.clips[0].clip);
			return;
		}

		// Clamp or loop time
		bool loop = mDefaultWrapMode == AnimWrapMode::Loop;
		if (t < startPos)
		{
			if (loop)
				t = t - std::floor(t / length) * length;
			else // Clamping
				t = startPos;
		}

		if (t > endPos)
		{
			if (loop)
				t = t - std::floor(t / length) * length;
			else // Clamping
				t = endPos;
		}

		// Find keys to blend between
		UINT32 leftKey = 0;
		UINT32 rightKey = 0;

		INT32 start = 0;
		INT32 searchLength = (INT32)info.numClips;

		while (searchLength > 0)
		{
			INT32 half = searchLength >> 1;
			INT32 mid = start + half;

			if (t < info.clips[mid].position)
			{
				searchLength = half;
			}
			else
			{
				start = mid + 1;
				searchLength -= (half + 1);
			}
		}

		leftKey = std::max(0, start - 1);
		rightKey = std::min(start, (INT32)info.numClips - 1);

		float interpLength = info.clips[rightKey].position - info.clips[leftKey].position;
		t = (t - info.clips[leftKey].position) / interpLength;

		// Add clips and set weights
		for(UINT32 i = 0; i < info.numClips; i++)
		{
			AnimationClipInfo* clipInfo = addClip(info.clips[i].clip, (UINT32)-1, i == 0);
			if (clipInfo != nullptr)
			{
				clipInfo->state.time = 0.0f;
				clipInfo->state.stopped = true;
				clipInfo->state.speed = 0.0f;
				clipInfo->state.wrapMode = AnimWrapMode::Clamp;

				if (i == leftKey)
					clipInfo->state.weight = 1.0f - t;
				else if (i == rightKey)
					clipInfo->state.weight = t;
				else
					clipInfo->state.weight = 0.0f;
			}
		}

		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::blend2D(const Blend2DInfo& info, const Vector2& t)
	{
		AnimationClipInfo* topLeftClipInfo = addClip(info.topLeftClip, (UINT32)-1, true);
		if (topLeftClipInfo != nullptr)
		{
			topLeftClipInfo->state.time = 0.0f;
			topLeftClipInfo->state.stopped = true;
			topLeftClipInfo->state.speed = 0.0f;
			topLeftClipInfo->state.weight = (1.0f - t.x) * (1.0f - t.y);
			topLeftClipInfo->state.wrapMode = AnimWrapMode::Clamp;
		}

		AnimationClipInfo* topRightClipInfo = addClip(info.topRightClip, (UINT32)-1, false);
		if (topRightClipInfo != nullptr)
		{
			topRightClipInfo->state.time = 0.0f;
			topRightClipInfo->state.stopped = true;
			topLeftClipInfo->state.speed = 0.0f;
			topRightClipInfo->state.weight = t.x * (1.0f - t.y);
			topRightClipInfo->state.wrapMode = AnimWrapMode::Clamp;
		}

		AnimationClipInfo* botLeftClipInfo = addClip(info.botLeftClip, (UINT32)-1, false);
		if (botLeftClipInfo != nullptr)
		{
			botLeftClipInfo->state.time = 0.0f;
			botLeftClipInfo->state.stopped = true;
			topLeftClipInfo->state.speed = 0.0f;
			botLeftClipInfo->state.weight = (1.0f - t.x) * t.y;
			botLeftClipInfo->state.wrapMode = AnimWrapMode::Clamp;
		}

		AnimationClipInfo* botRightClipInfo = addClip(info.botRightClip, (UINT32)-1, false);
		if (botRightClipInfo != nullptr)
		{
			botRightClipInfo->state.time = 0.0f;
			botRightClipInfo->state.stopped = true;
			botRightClipInfo->state.speed = 0.0f;
			botRightClipInfo->state.weight = t.x * t.y;
			botRightClipInfo->state.wrapMode = AnimWrapMode::Clamp;
		}

		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::crossFade(const HAnimationClip& clip, float fadeLength)
	{
		bool isFading = fadeLength > 0.0f;
		if(!isFading)
		{
			play(clip);
			return;
		}

		AnimationClipInfo* clipInfo = addClip(clip, (UINT32)-1, false);
		if (clipInfo != nullptr)
		{
			clipInfo->state.time = 0.0f;
			clipInfo->state.speed = mDefaultSpeed;
			clipInfo->state.weight = 1.0f;
			clipInfo->state.wrapMode = mDefaultWrapMode;

			// Set up fade lengths
			clipInfo->fadeDirection = 1.0f;
			clipInfo->fadeTime = 0.0f;
			clipInfo->fadeLength = fadeLength;

			for (auto& entry : mClipInfos)
			{
				if (entry.state.layer == (UINT32)-1 && entry.clip != clip)
				{
					// If other clips are already cross-fading, we need to persist their current weight before starting
					// a new crossfade. We do that by adjusting the fade times.
					if(clipInfo->fadeDirection != 0 && clipInfo->fadeTime < clipInfo->fadeLength)
					{
						float t = clipInfo->fadeTime / clipInfo->fadeLength;
						if (clipInfo->fadeDirection < 0.0f)
							t = (1.0f - t);

						clipInfo->state.weight *= t;
					}

					clipInfo->fadeDirection = -1.0f;
					clipInfo->fadeTime = 0.0f;
					clipInfo->fadeLength = fadeLength;
				}
			}
		}

		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::stop(UINT32 layer)
	{
		bs_frame_mark();
		{
			FrameVector<AnimationClipInfo> newClips;
			for (auto& clipInfo : mClipInfos)
			{
				if (clipInfo.state.layer != layer)
					newClips.push_back(clipInfo);
				else
					mDirty |= AnimDirtyStateFlag::Layout;
			}

			mClipInfos.resize(newClips.size());
			memcpy(mClipInfos.data(), newClips.data(), sizeof(AnimationClipInfo) * newClips.size());
		}
		bs_frame_clear();
	}

	void Animation::stopAll()
	{
		mClipInfos.clear();
		mDirty |= AnimDirtyStateFlag::Layout;
	}

	AnimationClipInfo* Animation::addClip(const HAnimationClip& clip, UINT32 layer, bool stopExisting)
	{
		AnimationClipInfo* output = nullptr;
		bool hasExisting = false;

		// Search for existing
		for (auto& clipInfo : mClipInfos)
		{
			if (clipInfo.state.layer == layer)
			{
				if (clipInfo.clip == clip)
					output = &clipInfo;
				else if (stopExisting)
					hasExisting = true;
			}
		}

		// Doesn't exist or found extra animations, rebuild
		if (output == nullptr || hasExisting)
		{
			bs_frame_mark();
			{
				FrameVector<AnimationClipInfo> newClips;
				for (auto& clipInfo : mClipInfos)
				{
					if (!stopExisting || clipInfo.state.layer != layer || clipInfo.clip == clip)
						newClips.push_back(clipInfo);
				}

				if (output == nullptr && clip != nullptr)
					newClips.push_back(AnimationClipInfo());

				mClipInfos.resize(newClips.size());
				memcpy(mClipInfos.data(), newClips.data(), sizeof(AnimationClipInfo) * newClips.size());

				mDirty |= AnimDirtyStateFlag::Layout;
			}
			bs_frame_clear();
		}

		// If new clip was added, get its address
		if (output == nullptr && clip != nullptr)
		{
			AnimationClipInfo& newInfo = mClipInfos.back();
			newInfo.clip = clip;
			newInfo.layerIdx = layer;

			output = &newInfo;
		}
		
		return output;
	}

	bool Animation::isPlaying() const
	{
		for(auto& clipInfo : mClipInfos)
		{
			if (clipInfo.clip.isLoaded())
				return true;
		}

		return false;
	}

	bool Animation::getState(const HAnimationClip& clip, AnimationClipState& state)
	{
		if (clip == nullptr)
			return false;

		for (auto& clipInfo : mClipInfos)
		{
			if (clipInfo.clip == clip)
			{
				state = clipInfo.state;
				return true;
			}
		}
		
		return false;
	}

	void Animation::setState(const HAnimationClip& clip, AnimationClipState state)
	{
		AnimationClipInfo* clipInfo = addClip(clip, state.layer, false);

		if (clipInfo == nullptr)
			return;

		clipInfo->state = state;
		mDirty |= AnimDirtyStateFlag::Value;
	}

	SPtr<Animation> Animation::create()
	{
		Animation* anim = new (bs_alloc<Animation>()) Animation();

		SPtr<Animation> animPtr = bs_core_ptr(anim);
		animPtr->_setThisPtr(animPtr);
		animPtr->initialize();

		return animPtr;
	}

	void Animation::updateAnimProxy(float timeDelta)
	{
		// Check if any of the clip curves are dirty and advance time, perform fading
		for (auto& clipInfo : mClipInfos)
		{
			float scaledTimeDelta = timeDelta * clipInfo.state.speed;
			clipInfo.state.time += scaledTimeDelta;

			if (clipInfo.clip.isLoaded() && clipInfo.curveVersion != clipInfo.clip->getVersion())
				mDirty |= AnimDirtyStateFlag::Layout;

			float fadeTime = clipInfo.fadeTime + scaledTimeDelta;
			clipInfo.fadeTime = Math::clamp(fadeTime, 0.0f, clipInfo.fadeLength);
		}

		if((UINT32)mDirty == 0) // Clean
		{
			mAnimProxy->updateTime(mClipInfos);
		}
		else
		{
			if (mDirty.isSet(AnimDirtyStateFlag::Skeleton))
				mAnimProxy->rebuild(mSkeleton, mClipInfos);
			else if (mDirty.isSet(AnimDirtyStateFlag::Layout))
				mAnimProxy->rebuild(mClipInfos);
			else if (mDirty.isSet(AnimDirtyStateFlag::Value))
				mAnimProxy->updateValues(mClipInfos);
		}

		mDirty = AnimDirtyState();
	}
}