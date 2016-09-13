//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimation.h"
#include "BsAnimationManager.h"
#include "BsAnimationClip.h"
#include "BsAnimationUtility.h"
#include "BsSceneObject.h"

namespace BansheeEngine
{
	AnimationClipInfo::AnimationClipInfo()
		: fadeDirection(0.0f), fadeTime(0.0f), fadeLength(0.0f), curveVersion(0), layerIdx((UINT32)-1)
		, stateIdx((UINT32)-1), playbackType(AnimPlaybackType::Normal)
	{ }

	AnimationClipInfo::AnimationClipInfo(const HAnimationClip& clip)
		: fadeDirection(0.0f), fadeTime(0.0f), fadeLength(0.0f), clip(clip), curveVersion(0), layerIdx((UINT32)-1)
		, stateIdx((UINT32)-1), playbackType(AnimPlaybackType::Normal)
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
		: id(id), layers(nullptr), numLayers(0), numSceneObjects(0), sceneObjectInfos(nullptr)
		, sceneObjectTransforms(nullptr), mCullEnabled(true), numGenericCurves(0), genericCurveOutputs(nullptr)
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

				if(state.soToCurveMapping != nullptr)
				{
					for(UINT32 k = 0; k < numSceneObjects; k++)
						state.soToCurveMapping[k].~AnimationCurveMapping();
				}

				state.~AnimationState();
			}

			layer.~AnimationStateLayer();
		}

		// All of the memory is part of the same buffer, so we only need to free the first element
		bs_free(layers);
		layers = nullptr;
		genericCurveOutputs = nullptr;
		sceneObjectInfos = nullptr;
		sceneObjectTransforms = nullptr;

		numLayers = 0;
		numGenericCurves = 0;
	}

	void AnimationProxy::rebuild(const SPtr<Skeleton>& skeleton, const SkeletonMask& mask, 
		Vector<AnimationClipInfo>& clipInfos, const Vector<AnimatedSceneObject>& sceneObjects)
	{
		this->skeleton = skeleton;
		this->skeletonMask = mask;

		// Note: I could avoid having a separate allocation for LocalSkeletonPoses and use the same buffer as the rest
		// of AnimationProxy
		if (skeleton != nullptr)
			skeletonPose = LocalSkeletonPose(skeleton->getNumBones());

		numSceneObjects = (UINT32)sceneObjects.size();
		if (numSceneObjects > 0)
			sceneObjectPose = LocalSkeletonPose(numSceneObjects);
		else
			sceneObjectPose = LocalSkeletonPose();

		rebuild(clipInfos, sceneObjects);
	}

	void AnimationProxy::rebuild(Vector<AnimationClipInfo>& clipInfos, const Vector<AnimatedSceneObject>& sceneObjects)
	{
		clear();

		bs_frame_mark();
		{
			FrameVector<bool> clipLoadState(clipInfos.size());
			FrameVector<AnimationStateLayer> tempLayers;
			UINT32 clipIdx = 0;
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

				bool isLoaded = clipInfo.clip.isLoaded();
				clipLoadState[clipIdx] = isLoaded;

				if (iterFind == tempLayers.end())
				{
					tempLayers.push_back(AnimationStateLayer());
					AnimationStateLayer& newLayer = tempLayers.back();

					newLayer.index = layer;
					newLayer.additive = isLoaded && clipInfo.clip->isAdditive();
				}

				clipIdx++;
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

			clipIdx = 0;
			for (auto& clipInfo : clipInfos)
			{
				bool isLoaded = clipLoadState[clipIdx++];
				if (!isLoaded)
					continue;

				SPtr<AnimationCurves> curves = clipInfo.clip->getCurves();
				numPosCurves += (UINT32)curves->position.size();
				numRotCurves += (UINT32)curves->rotation.size();
				numScaleCurves += (UINT32)curves->scale.size();
			}

			numGenericCurves = 0;
			if(clipInfos.size() > 0 && clipLoadState[0])
			{
				SPtr<AnimationCurves> curves = clipInfos[0].clip->getCurves();
				numGenericCurves = (UINT32)curves->generic.size();
			}

			UINT32* mappedBoneIndices = (UINT32*)bs_frame_alloc(sizeof(UINT32) * numSceneObjects);
			for (UINT32 i = 0; i < numSceneObjects; i++)
				mappedBoneIndices[i] = -1;

			UINT32 numBoneMappedSOs = 0;
			if (skeleton != nullptr)
			{
				for (UINT32 i = 0; i < numSceneObjects; i++)
				{
					if (sceneObjects[i].so.isDestroyed(true))
						continue;

					// Empty string always means root bone
					if (sceneObjects[i].curveName.empty())
					{
						UINT32 rootBoneIdx = skeleton->getRootBoneIndex();
						if (rootBoneIdx != (UINT32)-1)
						{
							mappedBoneIndices[i] = rootBoneIdx;
							numBoneMappedSOs++;
						}
					}
					else
					{
						for (UINT32 j = 0; j < numBones; j++)
						{
							if (skeleton->getBoneInfo(j).name == sceneObjects[i].curveName)
							{
								mappedBoneIndices[i] = j;

								numBoneMappedSOs++;
								break;
							}
						}
					}
				}
			}

			UINT32 numBoneMappings = numBones * numClips;
			UINT32 layersSize = sizeof(AnimationStateLayer) * numLayers;
			UINT32 clipsSize = sizeof(AnimationState) * numClips;
			UINT32 boneMappingSize = numBoneMappings * sizeof(AnimationCurveMapping);
			UINT32 posCacheSize = numPosCurves * sizeof(TCurveCache<Vector3>);
			UINT32 rotCacheSize = numRotCurves * sizeof(TCurveCache<Quaternion>);
			UINT32 scaleCacheSize = numScaleCurves * sizeof(TCurveCache<Vector3>);
			UINT32 genCacheSize = numGenericCurves * sizeof(TCurveCache<float>);
			UINT32 genericCurveOutputSize = numGenericCurves * sizeof(float);
			UINT32 sceneObjectIdsSize = numSceneObjects * sizeof(AnimatedSceneObjectInfo);
			UINT32 sceneObjectTransformsSize = numBoneMappedSOs * sizeof(Matrix4);

			UINT8* data = (UINT8*)bs_alloc(layersSize + clipsSize + boneMappingSize + posCacheSize + rotCacheSize + 
				scaleCacheSize + genCacheSize + genericCurveOutputSize + sceneObjectIdsSize + sceneObjectTransformsSize);

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
			for (UINT32 i = 0; i < numGenericCurves; i++)
				new (&genCache[i]) TCurveCache<float>();

			data += genCacheSize;

			genericCurveOutputs = (float*)data;
			data += genericCurveOutputSize;

			sceneObjectInfos = (AnimatedSceneObjectInfo*)data;
			data += sceneObjectIdsSize;

			sceneObjectTransforms = (Matrix4*)data;
			for (UINT32 i = 0; i < numBoneMappedSOs; i++)
				sceneObjectTransforms[i] = Matrix4::IDENTITY;

			data += sceneObjectTransformsSize;

			UINT32 curLayerIdx = 0;
			UINT32 curStateIdx = 0;

			// Note: Hidden dependency. First clip info must be in layers[0].states[0] (needed for generic curves which only
			// use the primary clip).
			for(UINT32 i = 0; i < numLayers; i++)
			{
				AnimationStateLayer& layer = layers[i];

				layer.states = &states[curStateIdx];
				layer.numStates = 0;

				UINT32 localStateIdx = 0;
				for(UINT32 j = 0; j < (UINT32)clipInfos.size(); j++)
				{
					AnimationClipInfo& clipInfo = clipInfos[j];

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
					bool isClipValid = clipLoadState[j];
					if (isClipValid)
					{
						state.curves = clipInfo.clip->getCurves();
						state.disabled = clipInfo.playbackType == AnimPlaybackType::None;
					}
					else
					{
						static SPtr<AnimationCurves> zeroCurves = bs_shared_ptr_new<AnimationCurves>();
						state.curves = zeroCurves;
						state.disabled = true;
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

			Matrix4 invRootTransform(BsIdentity);
			for (UINT32 i = 0; i < numSceneObjects; i++)
			{
				if(sceneObjects[i].curveName.empty())
				{
					HSceneObject so = sceneObjects[i].so;
					if (!so.isDestroyed(true))
						invRootTransform = so->getWorldTfrm().inverseAffine();

					break;
				}				
			}

			UINT32 boneIdx = 0;
			for(UINT32 i = 0; i < numSceneObjects; i++)
			{
				HSceneObject so = sceneObjects[i].so;
				AnimatedSceneObjectInfo& soInfo = sceneObjectInfos[i];
				soInfo.id = so.getInstanceId();
				soInfo.boneIdx = mappedBoneIndices[i];

				bool isSOValid = !so.isDestroyed(true);
				if (isSOValid)
					soInfo.hash = so->getTransformHash();
				else
					soInfo.hash = 0;

				soInfo.layerIdx = (UINT32)-1;
				soInfo.stateIdx = (UINT32)-1;

				// If no bone mapping, find curves directly
				if(soInfo.boneIdx == -1)
				{
					soInfo.curveIndices = { (UINT32)-1, (UINT32)-1, (UINT32)-1 };

					if (isSOValid)
					{
						for (UINT32 j = 0; j < (UINT32)clipInfos.size(); j++)
						{
							AnimationClipInfo& clipInfo = clipInfos[j];

							soInfo.layerIdx = clipInfo.layerIdx;
							soInfo.stateIdx = clipInfo.stateIdx;

							bool isClipValid = clipLoadState[j];
							if (isClipValid)
							{
								// Note: If there are multiple clips with the relevant curve name, we only use the first

								clipInfo.clip->getCurveMapping(sceneObjects[i].curveName, soInfo.curveIndices);
								break;
							}
						}
					}
				}
				else
				{
					// No need to check if SO is valid, if it has a bone connection it must be
					sceneObjectTransforms[boneIdx] = so->getWorldTfrm() * invRootTransform;
					boneIdx++;
				}
			}

			bs_frame_free(mappedBoneIndices);
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

			bool isLoaded = clipInfo.clip.isLoaded();
			state.disabled = !isLoaded || clipInfo.playbackType == AnimPlaybackType::None;
		}
	}

	void AnimationProxy::updateTransforms(const Vector<AnimatedSceneObject>& sceneObjects)
	{
		Matrix4 invRootTransform(BsIdentity);
		for (UINT32 i = 0; i < numSceneObjects; i++)
		{
			if (sceneObjects[i].curveName.empty())
			{
				HSceneObject so = sceneObjects[i].so;
				if (!so.isDestroyed(true))
					invRootTransform = so->getWorldTfrm().inverseAffine();

				break;
			}
		}

		UINT32 boneIdx = 0;
		for (UINT32 i = 0; i < numSceneObjects; i++)
		{
			HSceneObject so = sceneObjects[i].so;
			if (so.isDestroyed(true))
			{
				sceneObjectInfos[i].hash = 0;
				continue;
			}

			sceneObjectInfos[i].hash = so->getTransformHash();

			if (sceneObjectInfos[i].boneIdx == -1)
				continue;

			sceneObjectTransforms[boneIdx] = sceneObjects[i].so->getWorldTfrm() * invRootTransform;
			boneIdx++;
		}
	}

	void AnimationProxy::updateTime(const Vector<AnimationClipInfo>& clipInfos)
	{
		for (auto& clipInfo : clipInfos)
		{
			AnimationState& state = layers[clipInfo.layerIdx].states[clipInfo.stateIdx];
			state.time = clipInfo.state.time;

			bool isLoaded = clipInfo.clip.isLoaded();
			state.disabled = !isLoaded || clipInfo.playbackType == AnimPlaybackType::None;
		}
	}

	Animation::Animation()
		: mDefaultWrapMode(AnimWrapMode::Loop), mDefaultSpeed(1.0f), mCull(true), mDirty(AnimDirtyStateFlag::Skeleton)
		, mGenericCurveValuesValid(false)
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

	void Animation::setMask(const SkeletonMask& mask)
	{
		mSkeletonMask = mask;
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

	void Animation::setBounds(const AABox& bounds)
	{
		mBounds = bounds;

		mDirty |= AnimDirtyStateFlag::Culling;
	}

	void Animation::setCulling(bool cull)
	{
		mCull = cull;

		mDirty |= AnimDirtyStateFlag::Culling;
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
			clipInfo->playbackType = AnimPlaybackType::Normal;
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

			clipInfo->playbackType = AnimPlaybackType::Normal;
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

				clipInfo->playbackType = AnimPlaybackType::Normal;
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

			topLeftClipInfo->playbackType = AnimPlaybackType::Normal;
		}

		AnimationClipInfo* topRightClipInfo = addClip(info.topRightClip, (UINT32)-1, false);
		if (topRightClipInfo != nullptr)
		{
			topRightClipInfo->state.time = 0.0f;
			topRightClipInfo->state.stopped = true;
			topLeftClipInfo->state.speed = 0.0f;
			topRightClipInfo->state.weight = t.x * (1.0f - t.y);
			topRightClipInfo->state.wrapMode = AnimWrapMode::Clamp;

			topRightClipInfo->playbackType = AnimPlaybackType::Normal;
		}

		AnimationClipInfo* botLeftClipInfo = addClip(info.botLeftClip, (UINT32)-1, false);
		if (botLeftClipInfo != nullptr)
		{
			botLeftClipInfo->state.time = 0.0f;
			botLeftClipInfo->state.stopped = true;
			topLeftClipInfo->state.speed = 0.0f;
			botLeftClipInfo->state.weight = (1.0f - t.x) * t.y;
			botLeftClipInfo->state.wrapMode = AnimWrapMode::Clamp;

			botLeftClipInfo->playbackType = AnimPlaybackType::Normal;
		}

		AnimationClipInfo* botRightClipInfo = addClip(info.botRightClip, (UINT32)-1, false);
		if (botRightClipInfo != nullptr)
		{
			botRightClipInfo->state.time = 0.0f;
			botRightClipInfo->state.stopped = true;
			botRightClipInfo->state.speed = 0.0f;
			botRightClipInfo->state.weight = t.x * t.y;
			botRightClipInfo->state.wrapMode = AnimWrapMode::Clamp;

			botRightClipInfo->playbackType = AnimPlaybackType::Normal;
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
			clipInfo->playbackType = AnimPlaybackType::Normal;

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

	void Animation::sample(const HAnimationClip& clip, float time)
	{
		AnimationClipInfo* clipInfo = addClip(clip, (UINT32)-1);
		if (clipInfo != nullptr)
		{
			clipInfo->state.time = time;
			clipInfo->state.speed = 0.0f;
			clipInfo->state.weight = 1.0f;
			clipInfo->state.wrapMode = mDefaultWrapMode;
			clipInfo->playbackType = AnimPlaybackType::Sampled;
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
			newInfo.state.layer = layer;

			output = &newInfo;
		}
		
		return output;
	}

	void Animation::getListenerResources(Vector<HResource>& resources)
	{
		for (auto& entry : mClipInfos)
		{
			if(entry.clip != nullptr)
				resources.push_back(entry.clip);
		}
	}

	void Animation::notifyResourceLoaded(const HResource& resource)
	{
		mDirty |= AnimDirtyStateFlag::Layout;
	}

	void Animation::notifyResourceChanged(const HResource& resource)
	{
		mDirty |= AnimDirtyStateFlag::Layout;
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

				// Internally we store unclamped time, so clamp/loop it
				float clipLength = 0.0f;
				if (clip.isLoaded())
					clipLength = clip->getLength();

				bool loop = clipInfo.state.wrapMode == AnimWrapMode::Loop;
				AnimationUtility::wrapTime(clipInfo.state.time, 0.0f, clipLength, loop);

				return true;
			}
		}
		
		return false;
	}

	void Animation::setState(const HAnimationClip& clip, AnimationClipState state)
	{
		if (state.layer == 0)
			state.layer = (UINT32)-1;
		else
			state.layer -= 1;

		AnimationClipInfo* clipInfo = addClip(clip, state.layer, false);

		if (clipInfo == nullptr)
			return;

		clipInfo->state = state;
		clipInfo->playbackType = AnimPlaybackType::Normal;

		mDirty |= AnimDirtyStateFlag::Value;
	}

	UINT32 Animation::getNumClips() const
	{
		return (UINT32)mClipInfos.size();
	}

	HAnimationClip Animation::getClip(UINT32 idx) const
	{
		if (idx >= (UINT32)mClipInfos.size())
			return HAnimationClip();

		return mClipInfos[idx].clip;
	}

	void Animation::triggerEvents(float lastFrameTime, float delta)
	{
		for (auto& clipInfo : mClipInfos)
		{
			if (!clipInfo.clip.isLoaded())
				continue;

			const Vector<AnimationEvent>& events = clipInfo.clip->getEvents();
			bool loop = clipInfo.state.wrapMode == AnimWrapMode::Loop;

			float start = lastFrameTime;
			float end = start + delta;

			float clipLength = clipInfo.clip->getLength();
			AnimationUtility::wrapTime(start, 0.0f, clipLength, loop);
			AnimationUtility::wrapTime(end, 0.0f, clipLength, loop);

			if (start < end)
			{
				for (auto& event : events)
				{
					if (event.time > start && event.time <= end)
						onEventTriggered(clipInfo.clip, event.name);
				}
			}
			else if(end < start) // End is looped, but start is not
			{
				for (auto& event : events)
				{
					if (event.time > start && event.time < clipLength && event.time > 0 && event.time <= end)
						onEventTriggered(clipInfo.clip, event.name);
				}
			}
		}
	}

	void Animation::mapCurveToSceneObject(const String& curve, const HSceneObject& so)
	{
		AnimatedSceneObject animSo = { so, curve };
		mSceneObjects[so.getInstanceId()] = animSo;

		mDirty |= AnimDirtyStateFlag::Skeleton;
	}

	void Animation::unmapSceneObject(const HSceneObject& so)
	{
		mSceneObjects.erase(so.getInstanceId());

		mDirty |= AnimDirtyStateFlag::Skeleton;
	}

	bool Animation::getGenericCurveValue(UINT32 curveIdx, float& value)
	{
		if (!mGenericCurveValuesValid || curveIdx >= (UINT32)mGenericCurveOutputs.size())
			return false;

		value = mGenericCurveOutputs[curveIdx];
		return true;
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

			HAnimationClip clip = clipInfo.clip;
			float clipLength = 0.0f;
			if (clip.isLoaded())
			{
				if (clipInfo.curveVersion != clip->getVersion())
					mDirty |= AnimDirtyStateFlag::Layout;

				clipLength = clip->getLength();
			}

			float fadeTime = clipInfo.fadeTime + scaledTimeDelta;
			clipInfo.fadeTime = Math::clamp(fadeTime, 0.0f, clipInfo.fadeLength);
		}

		if (mDirty.isSet(AnimDirtyStateFlag::Culling))
		{
			mAnimProxy->mCullEnabled = mCull;
			mAnimProxy->mBounds = mBounds;

			mDirty.unset(AnimDirtyStateFlag::Culling);
		}

		auto getAnimatedSOList = [&]()
		{
			Vector<AnimatedSceneObject> animatedSO(mSceneObjects.size());
			UINT32 idx = 0;
			for (auto& entry : mSceneObjects)
				animatedSO[idx++] = entry.second;

			return animatedSO;
		};

		bool didFullRebuild = false;
		if((UINT32)mDirty == 0) // Clean
		{
			mAnimProxy->updateTime(mClipInfos);
		}
		else
		{
			if (mDirty.isSet(AnimDirtyStateFlag::Skeleton))
			{
				Vector<AnimatedSceneObject> animatedSOs = getAnimatedSOList();

				mAnimProxy->rebuild(mSkeleton, mSkeletonMask, mClipInfos, animatedSOs);
				didFullRebuild = true;
			}
			else if (mDirty.isSet(AnimDirtyStateFlag::Layout))
			{
				Vector<AnimatedSceneObject> animatedSOs = getAnimatedSOList();

				mAnimProxy->rebuild(mClipInfos, animatedSOs);
				didFullRebuild = true;
			}
			else if (mDirty.isSet(AnimDirtyStateFlag::Value))
				mAnimProxy->updateValues(mClipInfos);
		}

		// Check if there are dirty transforms
		if (!didFullRebuild)
		{
			for (UINT32 i = 0; i < mAnimProxy->numSceneObjects; i++)
			{
				AnimatedSceneObjectInfo& soInfo = mAnimProxy->sceneObjectInfos[i];

				auto iterFind = mSceneObjects.find(soInfo.id);
				if (iterFind == mSceneObjects.end())
				{
					assert(false); // Should never happen
					continue;
				}

				UINT32 hash;

				HSceneObject so = iterFind->second.so;
				if (so.isDestroyed(true))
					hash = 0;
				else
					hash = so->getTransformHash();

				if (hash != mAnimProxy->sceneObjectInfos[i].hash)
				{
					Vector<AnimatedSceneObject> animatedSOs = getAnimatedSOList();
					mAnimProxy->updateTransforms(animatedSOs);
					break;
				}
			}
		}

		// Disable sampled animations
		for (auto& clipInfo : mClipInfos)
		{
			if (clipInfo.playbackType == AnimPlaybackType::Sampled)
				clipInfo.playbackType = AnimPlaybackType::None;
		}

		mDirty = AnimDirtyState();
	}

	void Animation::updateFromProxy()
	{
		HSceneObject rootSO;

		// Write TRS animation results to relevant SceneObjects
		for(UINT32 i = 0; i < mAnimProxy->numSceneObjects; i++)
		{
			AnimatedSceneObjectInfo& soInfo = mAnimProxy->sceneObjectInfos[i];

			auto iterFind = mSceneObjects.find(soInfo.id);
			if (iterFind == mSceneObjects.end())
				continue;

			HSceneObject so = iterFind->second.so;
			if (iterFind->second.curveName.empty())
				rootSO = so;

			if (so.isDestroyed(true))
				continue;

			if(soInfo.boneIdx != -1)
			{
				if (mAnimProxy->skeletonPose.hasOverride[soInfo.boneIdx])
					continue;

				Vector3 position = mAnimProxy->skeletonPose.positions[soInfo.boneIdx];
				Quaternion rotation = mAnimProxy->skeletonPose.rotations[soInfo.boneIdx];
				Vector3 scale = mAnimProxy->skeletonPose.scales[soInfo.boneIdx];

				const SPtr<Skeleton>& skeleton = mAnimProxy->skeleton;

				UINT32 parentBoneIdx = skeleton->getBoneInfo(soInfo.boneIdx).parent;
				if (parentBoneIdx == -1)
				{
					so->setPosition(position);
					so->setRotation(rotation);
					so->setScale(scale);
				}
				else
				{
					while(parentBoneIdx != -1)
					{
						// Update rotation
						const Quaternion& parentOrientation = mAnimProxy->skeletonPose.rotations[parentBoneIdx];
						rotation = parentOrientation * rotation;

						// Update scale
						const Vector3& parentScale = mAnimProxy->skeletonPose.scales[parentBoneIdx];
						scale = parentScale * scale;

						// Update position
						position = parentOrientation.rotate(parentScale * position);
						position += mAnimProxy->skeletonPose.positions[parentBoneIdx];

						parentBoneIdx = skeleton->getBoneInfo(parentBoneIdx).parent;
					}

					// Search for root if not already found
					if(rootSO == nullptr)
					{
						for(auto& entry : mSceneObjects)
						{
							if (entry.second.curveName.empty())
								rootSO = entry.second.so;
						}
					}

					Vector3 rootParentPos = Vector3::ZERO;
					Quaternion rootParentRot = Quaternion::IDENTITY;
					Vector3 rootParentScale = Vector3::ONE;

					if(rootSO.isDestroyed(true))
					{
						HSceneObject rootParent = rootSO->getParent();
						if(!rootParent.isDestroyed(true))
						{
							rootParentPos = rootParent->getWorldPosition();
							rootParentRot = rootParent->getWorldRotation();
							rootParentScale = rootParent->getWorldScale();
						}
					}

					// Transform from space relative to root's parent to world space
					rotation = rootParentRot * rotation;

					scale = rootParentScale * scale;

					position = rootParentRot.rotate(rootParentScale * position);
					position += rootParentPos;

					so->setWorldPosition(position);
					so->setWorldRotation(rotation);
					so->setWorldScale(scale);
				}
			}
			else
			{
				if (mAnimProxy->sceneObjectPose.hasOverride[i])
					continue;

				so->setPosition(mAnimProxy->sceneObjectPose.positions[i]);
				so->setRotation(mAnimProxy->sceneObjectPose.rotations[i]);
				so->setScale(mAnimProxy->sceneObjectPose.scales[i]);
			}
		}

		// Must ensure that clip in the proxy and current primary clip are the same
		mGenericCurveValuesValid = false;
		if(mAnimProxy->numLayers > 0 && mAnimProxy->layers[0].numStates > 0)
		{
			const AnimationState& state = mAnimProxy->layers[0].states[0];

			if(!state.disabled && mClipInfos.size() > 0)
			{
				const AnimationClipInfo& clipInfo = mClipInfos[0];

				if (clipInfo.stateIdx == 0 && clipInfo.layerIdx == 0)
				{
					if (clipInfo.clip.isLoaded() && clipInfo.curveVersion == clipInfo.clip->getVersion())
					{
						UINT32 numGenericCurves = (UINT32)clipInfo.clip->getCurves()->generic.size();
						mGenericCurveValuesValid = numGenericCurves == mAnimProxy->numGenericCurves;
					}
				}
			}
		}

		if(mGenericCurveValuesValid)
		{
			mGenericCurveOutputs.resize(mAnimProxy->numGenericCurves);

			memcpy(mGenericCurveOutputs.data(), mAnimProxy->genericCurveOutputs, mAnimProxy->numGenericCurves * sizeof(float));
		}
	}
}