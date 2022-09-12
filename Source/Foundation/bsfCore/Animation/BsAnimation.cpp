//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsAnimation.h"
#include "Animation/BsAnimationManager.h"
#include "Animation/BsAnimationClip.h"
#include "Animation/BsAnimationUtility.h"
#include "Scene/BsSceneObject.h"
#include "Animation/BsMorphShapes.h"

namespace bs
{
	AnimationClipInfo::AnimationClipInfo(const HAnimationClip& clip)
		: Clip(clip)
	{ }

	AnimationProxy::AnimationProxy(UINT64 id)
		: Id(id)
	{ }

	AnimationProxy::~AnimationProxy()
	{
		clear();
	}

	void AnimationProxy::Clear()
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
					UINT32 numBones = skeleton->GetNumBones();
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

		for(UINT32 i = 0; i < numMorphShapes; i++)
		{
			morphShapeInfos[i].shape.~SPtr<MorphShape>();
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
		Vector<AnimationClipInfo>& clipInfos, const Vector<AnimatedSceneObject>& sceneObjects,
		const SPtr<MorphShapes>& morphShapes)
	{
		this->skeleton = skeleton;
		this->skeletonMask = mask;

		// Note: I could avoid having a separate allocation for LocalSkeletonPoses and use the same buffer as the rest
		// of AnimationProxy
		if (skeleton != nullptr)
			skeletonPose = LocalSkeletonPose(skeleton->GetNumBones());

		numSceneObjects = (UINT32)sceneObjects.size();
		if (numSceneObjects > 0)
			sceneObjectPose = LocalSkeletonPose(numSceneObjects, true);
		else
			sceneObjectPose = LocalSkeletonPose();

		rebuild(clipInfos, sceneObjects, morphShapes);
	}

	void AnimationProxy::rebuild(Vector<AnimationClipInfo>& clipInfos, const Vector<AnimatedSceneObject>& sceneObjects,
		const SPtr<MorphShapes>& morphShapes)
	{
		clear();

		bs_frame_mark();
		{
			FrameVector<bool> ClipLoadState(clipInfos.size());
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

				bool isLoaded = clipInfo.clip.IsLoaded();
				clipLoadState[clipIdx] = isLoaded;

				if (iterFind == tempLayers.end())
				{
					tempLayers.push_back(AnimationStateLayer());
					AnimationStateLayer& newLayer = tempLayers.back();

					newLayer.index = layer;
					newLayer.additive = isLoaded && clipInfo.clip->IsAdditive();
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
				numBones = skeleton->GetNumBones();
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

				SPtr<AnimationCurves> curves = clipInfo.clip->GetCurves();
				numPosCurves += (UINT32)curves->position.size();
				numRotCurves += (UINT32)curves->rotation.size();
				numScaleCurves += (UINT32)curves->scale.size();
			}

			numGenericCurves = 0;
			if(clipInfos.size() > 0 && clipLoadState[0])
			{
				SPtr<AnimationCurves> curves = clipInfos[0].clip->GetCurves();
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
					if (sceneObjects[i].so.IsDestroyed(true))
						continue;

					// Empty string always means root bone
					if (sceneObjects[i].curveName.empty())
					{
						UINT32 rootBoneIdx = skeleton->GetRootBoneIndex();
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
							if (skeleton->GetBoneInfo(j).name == sceneObjects[i].curveName)
							{
								mappedBoneIndices[i] = j;

								numBoneMappedSOs++;
								break;
							}
						}
					}
				}
			}

			if (morphShapes != nullptr)
			{
				numMorphChannels = morphShapes->GetNumChannels();
				numMorphVertices = morphShapes->GetNumVertices();

				numMorphShapes = 0;
				for (UINT32 i = 0; i < numMorphChannels; i++)
					numMorphShapes += morphShapes->GetChannel(i)->getNumShapes();
			}
			else
			{
				numMorphChannels = 0;
				numMorphShapes = 0;
				numMorphVertices = 0;
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
			UINT32 morphChannelSize = numMorphChannels * sizeof(MorphChannelInfo);
			UINT32 morphShapeSize = numMorphShapes * sizeof(MorphShapeInfo);

			UINT8* data = (UINT8*)bs_alloc(layersSize + clipsSize + boneMappingSize + posCacheSize + rotCacheSize +
				scaleCacheSize + genCacheSize + genericCurveOutputSize + sceneObjectIdsSize + sceneObjectTransformsSize +
				morphChannelSize + morphShapeSize);

			layers = (AnimationStateLayer*)data;
			memcpy(layers, tempLayers.Data(), layersSize);
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

			morphChannelInfos = (MorphChannelInfo*)data;
			data += morphChannelSize;

			morphShapeInfos = (MorphShapeInfo*)data;
			data += morphShapeSize;

			// Generate data required for morph shape animation
			if (morphShapes != nullptr)
			{
				UINT32 currentShapeIdx = 0;
				for (UINT32 i = 0; i < numMorphChannels; i++)
				{
					SPtr<MorphChannel> morphChannel = morphShapes->GetChannel(i);
					UINT32 numShapes = morphChannel->GetNumShapes();

					MorphChannelInfo& channelInfo = morphChannelInfos[i];
					channelInfo.weight = 0.0f;
					channelInfo.shapeStart = currentShapeIdx;
					channelInfo.shapeCount = numShapes;
					channelInfo.frameCurveIdx = (UINT32)-1;
					channelInfo.weightCurveIdx = (UINT32)-1;

					for (UINT32 j = 0; j < numShapes; j++)
					{
						MorphShapeInfo& shapeInfo = morphShapeInfos[currentShapeIdx];
						new (&shapeInfo.shape) SPtr<MorphShape>();

						SPtr<MorphShape> shape = morphChannel->GetShape(j);
						shapeInfo.shape = shape;
						shapeInfo.frameWeight = shape->GetWeight();
						shapeInfo.finalWeight = 0.0f;

						currentShapeIdx++;
					}
				}

				// Find any curves affecting morph shape animation
				if (!clipInfos.empty())
				{
					bool isClipValid = clipLoadState[0];
					if (isClipValid)
					{
						AnimationClipInfo& clipInfo = clipInfos[0];

						for (UINT32 i = 0; i < numMorphChannels; i++)
						{
							SPtr<MorphChannel> morphChannel = morphShapes->GetChannel(i);
							MorphChannelInfo& channelInfo = morphChannelInfos[i];

							clipInfo.clip->GetMorphMapping(morphChannel->getName(), channelInfo.frameCurveIdx,
								channelInfo.weightCurveIdx);
						}
					}
				}

				morphChannelWeightsDirty = true;
			}

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
						state.curves = clipInfo.clip->GetCurves();
						state.length = clipInfo.clip->GetLength();
						state.disabled = clipInfo.playbackType == AnimPlaybackType::None;
					}
					else
					{
						static SPtr<AnimationCurves> zeroCurves = bs_shared_ptr_new<AnimationCurves>();
						state.curves = zeroCurves;
						state.length = 0.0f;
						state.disabled = true;
					}

					// Wrap time if looping
					if (state.loop && state.length > 0.0f)
						state.time = Math::repeat(clipInfo.state.time, state.length);
					else
						state.time = clipInfo.state.time;

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
						clipInfo.curveVersion = clipInfo.clip->GetVersion();

					// Set up bone mapping
					if (skeleton != nullptr)
					{
						state.boneToCurveMapping = &boneMappings[curStateIdx * numBones];

						if (isClipValid)
						{
							clipInfo.clip->GetBoneMapping(*skeleton, state.boneToCurveMapping);
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

			Matrix4 InvRootTransform(BsIdentity);
			for (UINT32 i = 0; i < numSceneObjects; i++)
			{
				if(sceneObjects[i].curveName.empty())
				{
					HSceneObject so = sceneObjects[i].so;
					if (!so.IsDestroyed(true))
						invRootTransform = so->GetWorldMatrix().InverseAffine();

					break;
				}
			}

			UINT32 boneIdx = 0;
			for(UINT32 i = 0; i < numSceneObjects; i++)
			{
				HSceneObject so = sceneObjects[i].so;
				AnimatedSceneObjectInfo& soInfo = sceneObjectInfos[i];
				soInfo.id = so.GetInstanceId();
				soInfo.boneIdx = mappedBoneIndices[i];

				bool isSOValid = !so.IsDestroyed(true);
				if (isSOValid)
					soInfo.hash = so->GetTransformHash();
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

								clipInfo.clip->GetCurveMapping(sceneObjects[i].curveName, soInfo.curveIndices);
								break;
							}
						}
					}
				}
				else
				{
					// No need to check if SO is valid, if it has a bone connection it must be
					sceneObjectTransforms[boneIdx] = so->GetWorldMatrix() * invRootTransform;
					boneIdx++;
				}
			}

			bs_frame_free(mappedBoneIndices);
		}
		bs_frame_clear();
	}

	void AnimationProxy::UpdateClipInfos(const Vector<AnimationClipInfo>& clipInfos)
	{
		for(auto& clipInfo : clipInfos)
		{
			AnimationState& state = layers[clipInfo.layerIdx].states[clipInfo.stateIdx];

			state.loop = clipInfo.state.wrapMode == AnimWrapMode::Loop;
			state.weight = clipInfo.state.weight;

			// Wrap time if looping
			if (state.loop && state.length > 0.0f)
				state.time = Math::repeat(clipInfo.state.time, state.length);
			else
				state.time = clipInfo.state.time;

			bool isLoaded = clipInfo.clip.IsLoaded();
			state.disabled = !isLoaded || clipInfo.playbackType == AnimPlaybackType::None;
		}
	}

	void AnimationProxy::UpdateMorphChannelWeights(const Vector<float>& weights)
	{
		UINT32 numWeights = (UINT32)weights.size();
		for(UINT32 i = 0; i < numMorphChannels; i++)
		{
			if (i < numWeights)
				morphChannelInfos[i].weight = weights[i];
			else
				morphChannelInfos[i].weight = 0.0f;
		}

		morphChannelWeightsDirty = true;
	}

	void AnimationProxy::UpdateTransforms(const Vector<AnimatedSceneObject>& sceneObjects)
	{
		Matrix4 InvRootTransform(BsIdentity);
		for (UINT32 i = 0; i < numSceneObjects; i++)
		{
			if (sceneObjects[i].curveName.empty())
			{
				HSceneObject so = sceneObjects[i].so;
				if (!so.IsDestroyed(true))
					invRootTransform = so->GetWorldMatrix().InverseAffine();

				break;
			}
		}

		UINT32 boneIdx = 0;
		for (UINT32 i = 0; i < numSceneObjects; i++)
		{
			HSceneObject so = sceneObjects[i].so;
			if (so.IsDestroyed(true))
			{
				sceneObjectInfos[i].hash = 0;
				continue;
			}

			sceneObjectInfos[i].hash = so->GetTransformHash();

			if (sceneObjectInfos[i].boneIdx == -1)
				continue;

			sceneObjectTransforms[boneIdx] = sceneObjects[i].so->GetWorldMatrix() * invRootTransform;
			boneIdx++;
		}
	}

	void AnimationProxy::UpdateTime(const Vector<AnimationClipInfo>& clipInfos)
	{
		for (auto& clipInfo : clipInfos)
		{
			AnimationState& state = layers[clipInfo.layerIdx].states[clipInfo.stateIdx];

			// Wrap time if looping
			if (state.loop && state.length > 0.0f)
				state.time = Math::repeat(clipInfo.state.time, state.length);
			else
				state.time = clipInfo.state.time;

			bool isLoaded = clipInfo.clip.IsLoaded();
			state.disabled = !isLoaded || clipInfo.playbackType == AnimPlaybackType::None;
		}
	}

	Animation::Animation()
	{
		mId = AnimationManager::instance().RegisterAnimation(this);
		mAnimProxy = bs_shared_ptr_new<AnimationProxy>(mId);
	}

	Animation::~Animation()
	{
		AnimationManager::instance().UnregisterAnimation(mId);
	}

	void Animation::SetSkeleton(const SPtr<Skeleton>& skeleton)
	{
		mSkeleton = skeleton;
		mDirty |= AnimDirtyStateFlag::All;
	}

	void Animation::SetMorphShapes(const SPtr<MorphShapes>& morphShapes)
	{
		mMorphShapes = morphShapes;

		UINT32 numChannels;
		if (mMorphShapes != nullptr)
			numChannels = mMorphShapes->GetNumChannels();
		else
			numChannels = 0;

		mMorphChannelWeights.Assign(numChannels, 0.0f);
		if (numChannels > 0)
			mMorphChannelWeights[0] = 1.0f;

		mDirty |= AnimDirtyStateFlag::Layout;
		mDirty |= AnimDirtyStateFlag::MorphWeights;
	}

	void Animation::SetMorphChannelWeight(UINT32 idx, float weight)
	{
		UINT32 numShapes = (UINT32)mMorphChannelWeights.size();
		if (idx >= numShapes)
			return;

		mMorphChannelWeights[idx] = weight;
		mDirty |= AnimDirtyStateFlag::MorphWeights;
	}

	void Animation::SetMask(const SkeletonMask& mask)
	{
		mSkeletonMask = mask;
		mDirty |= AnimDirtyStateFlag::All;
	}

	void Animation::SetWrapMode(AnimWrapMode wrapMode)
	{
		mDefaultWrapMode = wrapMode;

		for (auto& clipInfo : mClipInfos)
			clipInfo.state.wrapMode = wrapMode;

		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::SetSpeed(float speed)
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

	void Animation::SetBounds(const AABox& bounds)
	{
		mBounds = bounds;

		mDirty |= AnimDirtyStateFlag::Culling;
	}

	void Animation::SetCulling(bool cull)
	{
		mCull = cull;

		mDirty |= AnimDirtyStateFlag::Culling;
	}

	void Animation::Play(const HAnimationClip& clip)
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

		mSampleStep = AnimSampleStep::None;
		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength, UINT32 layer)
	{
		if(clip != nullptr && !clip->IsAdditive())
		{
			BS_LOG(Warning, Renderer,
				"blendAdditive() called with a clip that doesn't contain additive animation. Ignoring.");

			// Stop any clips on this layer, even if invalid
			HAnimationClip nullClip;
			addClip(nullClip, layer);

			mSampleStep = AnimSampleStep::None;
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

			mSampleStep = AnimSampleStep::None;
			mDirty |= AnimDirtyStateFlag::Value;
		}
	}

	void Animation::Blend1D(const Blend1DInfo& info, float t)
	{
		if (info.clips.size() == 0)
			return;

		// Find valid range
		float startPos = 0.0f;
		float endPos = 0.0f;

		for (UINT32 i = 0; i < (UINT32)info.clips.size(); i++)
		{
			startPos = std::min(startPos, info.clips[i].position);
			endPos = std::min(endPos, info.clips[i].position);
		}

		float length = endPos - startPos;
		if(Math::approxEquals(length, 0.0f) || info.clips.size() < 2)
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
		INT32 searchLength = (INT32)info.clips.size();

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
		rightKey = std::min(start, (INT32)info.clips.size() - 1);

		float interpLength = info.clips[rightKey].position - info.clips[leftKey].position;
		t = (t - info.clips[leftKey].position) / interpLength;

		// Add clips and set weights
		for(UINT32 i = 0; i < (UINT32)info.clips.size(); i++)
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

		mSampleStep = AnimSampleStep::None;
		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::Blend2D(const Blend2DInfo& info, const Vector2& t)
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
			topRightClipInfo->state.speed = 0.0f;
			topRightClipInfo->state.weight = t.x * (1.0f - t.y);
			topRightClipInfo->state.wrapMode = AnimWrapMode::Clamp;

			topRightClipInfo->playbackType = AnimPlaybackType::Normal;
		}

		AnimationClipInfo* botLeftClipInfo = addClip(info.botLeftClip, (UINT32)-1, false);
		if (botLeftClipInfo != nullptr)
		{
			botLeftClipInfo->state.time = 0.0f;
			botLeftClipInfo->state.stopped = true;
			botLeftClipInfo->state.speed = 0.0f;
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

		mSampleStep = AnimSampleStep::None;
		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::CrossFade(const HAnimationClip& clip, float fadeLength)
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

		mSampleStep = AnimSampleStep::None;
		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::Sample(const HAnimationClip& clip, float time)
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

		mSampleStep = AnimSampleStep::Frame;
		mDirty |= AnimDirtyStateFlag::Value;
	}

	void Animation::Stop(UINT32 layer)
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

			mClipInfos.Resize(newClips.size());
			for(UINT32 i = 0; i < (UINT32)newClips.size(); i++)
				mClipInfos[i] = newClips[i];
		}
		bs_frame_clear();
	}

	void Animation::StopAll()
	{
		mClipInfos.Clear();

		mSampleStep = AnimSampleStep::None;
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

				mClipInfos.Resize(newClips.size());
				for(UINT32 i = 0; i < (UINT32)newClips.size(); i++)
					mClipInfos[i] = newClips[i];

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

	bool Animation::_getAnimatesRoot() const
	{
		if (mSkeleton == nullptr)
			return false;

		UINT32 rootBoneIdx = mSkeleton->GetRootBoneIndex();
		if (rootBoneIdx == (UINT32)-1)
			return false;

		String rootBoneName = mSkeleton->GetBoneInfo(rootBoneIdx).name;
		for (auto& entry : mClipInfos)
		{
			if (entry.clip.IsLoaded())
			{
				HAnimationClip clip = entry.clip;
				if(!clip->HasRootMotion())
				{
					AnimationCurveMapping mapping;
					clip->GetCurveMapping(rootBoneName, mapping);

					if (mapping.position != (UINT32)-1)
						return true;

					if (mapping.rotation != (UINT32)-1)
						return true;

					if (mapping.scale != (UINT32)-1)
						return true;
				}
			}
		}

		return false;
	}

	void Animation::GetListenerResources(Vector<HResource>& resources)
	{
		for (auto& entry : mClipInfos)
		{
			if(entry.clip != nullptr)
				resources.push_back(entry.clip);
		}
	}

	void Animation::NotifyResourceLoaded(const HResource& resource)
	{
		mDirty |= AnimDirtyStateFlag::Layout;
	}

	void Animation::NotifyResourceChanged(const HResource& resource)
	{
		mDirty |= AnimDirtyStateFlag::Layout;
	}

	bool Animation::IsPlaying() const
	{
		for(auto& clipInfo : mClipInfos)
		{
			if (clipInfo.clip.IsLoaded())
				return true;
		}

		return false;
	}

	bool Animation::GetState(const HAnimationClip& clip, AnimationClipState& state)
	{
		if (clip == nullptr)
			return false;

		for (auto& clipInfo : mClipInfos)
		{
			if (clipInfo.clip == clip)
			{
				state = clipInfo.state;

				if (state.layer == (UINT32)-1)
					state.layer = 0;
				else
					state.layer += 1;

				// Internally we store unclamped time, so clamp/loop it
				float clipLength = 0.0f;
				if (clip.IsLoaded())
					clipLength = clip->GetLength();

				bool loop = clipInfo.state.wrapMode == AnimWrapMode::Loop;
				AnimationUtility::wrapTime(clipInfo.state.time, 0.0f, clipLength, loop);

				return true;
			}
		}

		return false;
	}

	void Animation::SetState(const HAnimationClip& clip, AnimationClipState state)
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

		mSampleStep = AnimSampleStep::None;
		mDirty |= AnimDirtyStateFlag::Value;
	}

	UINT32 Animation::GetNumClips() const
	{
		return (UINT32)mClipInfos.size();
	}

	HAnimationClip Animation::GetClip(UINT32 idx) const
	{
		if (idx >= (UINT32)mClipInfos.size())
			return HAnimationClip();

		return mClipInfos[idx].clip;
	}

	void Animation::TriggerEvents(float delta)
	{
		for (auto& clipInfo : mClipInfos)
		{
			if (!clipInfo.clip.IsLoaded())
				continue;

			const Vector<AnimationEvent>& events = clipInfo.clip->GetEvents();
			bool loop = clipInfo.state.wrapMode == AnimWrapMode::Loop;

			float start = std::max(clipInfo.state.time - delta, 0.0f);
			float end = clipInfo.state.time;
			float clipLength = clipInfo.clip->GetLength();

			float wrappedStart = start;
			float wrappedEnd = end;
			AnimationUtility::wrapTime(wrappedStart, 0.0f, clipLength, loop);
			AnimationUtility::wrapTime(wrappedEnd, 0.0f, clipLength, loop);

			if(!loop)
			{
				for (auto& event : events)
				{
					if (event.time >= wrappedStart && (event.time < wrappedEnd ||
						(event.time == clipLength && start < clipLength && end >= clipLength)))
						onEventTriggered(clipInfo.clip, event.name);
				}
			}
			else
			{
				if (wrappedStart < wrappedEnd)
				{
					for (auto& event : events)
					{
						if (event.time >= wrappedStart && event.time < wrappedEnd)
							onEventTriggered(clipInfo.clip, event.name);
					}
				}
				else if (wrappedEnd < wrappedStart) // End is looped, but start is not
				{
					for (auto& event : events)
					{
						if ((event.time >= wrappedStart && event.time <= clipLength) || (event.time >= 0 && event.time < wrappedEnd))
							onEventTriggered(clipInfo.clip, event.name);
					}
				}
			}
		}
	}

	void Animation::MapCurveToSceneObject(const String& curve, const HSceneObject& so)
	{
		AnimatedSceneObject animSo = { so, curve };
		mSceneObjects[so.GetInstanceId()] = animSo;

		mDirty |= AnimDirtyStateFlag::All;
	}

	void Animation::UnmapSceneObject(const HSceneObject& so)
	{
		mSceneObjects.erase(so.getInstanceId());

		mDirty |= AnimDirtyStateFlag::All;
	}

	bool Animation::GetGenericCurveValue(UINT32 curveIdx, float& value)
	{
		if (!mGenericCurveValuesValid || curveIdx >= (UINT32)mGenericCurveOutputs.size())
			return false;

		value = mGenericCurveOutputs[curveIdx];
		return true;
	}

	SPtr<Animation> Animation::Create()
	{
		Animation* anim = new (bs_alloc<Animation>()) Animation();

		SPtr<Animation> animPtr = bs_core_ptr(anim);
		animPtr->_setThisPtr(animPtr);
		animPtr->Initialize();

		return animPtr;
	}

	void Animation::UpdateAnimProxy(float timeDelta)
	{
		// Check if any of the clip curves are dirty and advance time, perform fading
		for (auto& clipInfo : mClipInfos)
		{
			float scaledTimeDelta = timeDelta * clipInfo.state.speed;
			clipInfo.state.time += scaledTimeDelta;

			HAnimationClip clip = clipInfo.clip;
			if (clip.IsLoaded())
			{
				if (clipInfo.curveVersion != clip->GetVersion())
					mDirty |= AnimDirtyStateFlag::Layout;
			}

			float fadeTime = clipInfo.fadeTime + scaledTimeDelta;
			clipInfo.fadeTime = Math::clamp(fadeTime, 0.0f, clipInfo.fadeLength);
		}

		if(mSampleStep == AnimSampleStep::None)
			mAnimProxy->sampleStep = AnimSampleStep::None;
		else if(mSampleStep == AnimSampleStep::Frame)
		{
			if(mAnimProxy->sampleStep == AnimSampleStep::None)
				mAnimProxy->sampleStep = AnimSampleStep::Frame;
			else
				mAnimProxy->sampleStep = AnimSampleStep::Done;
		}

		if (mDirty.IsSet(AnimDirtyStateFlag::Culling))
		{
			mAnimProxy->mCullEnabled = mCull;
			mAnimProxy->mBounds = mBounds;

			mDirty.Unset(AnimDirtyStateFlag::Culling);
		}

		auto getAnimatedSOList = [&]()
		{
			Vector<AnimatedSceneObject> AnimatedSO(mSceneObjects.size());
			UINT32 idx = 0;
			for (auto& entry : mSceneObjects)
				animatedSO[idx++] = entry.second;

			return animatedSO;
		};

		bool didFullRebuild = false;
		if((UINT32)mDirty == 0) // Clean
		{
			mAnimProxy->UpdateTime(mClipInfos);
		}
		else
		{
			if (mDirty.IsSet(AnimDirtyStateFlag::All))
			{
				Vector<AnimatedSceneObject> animatedSOs = getAnimatedSOList();

				mAnimProxy->Rebuild(mSkeleton, mSkeletonMask, mClipInfos, animatedSOs, mMorphShapes);
				didFullRebuild = true;
			}
			else if (mDirty.IsSet(AnimDirtyStateFlag::Layout))
			{
				Vector<AnimatedSceneObject> animatedSOs = getAnimatedSOList();

				mAnimProxy->Rebuild(mClipInfos, animatedSOs, mMorphShapes);
				didFullRebuild = true;
			}
			else if(mDirty.IsSet(AnimDirtyStateFlag::Value))
				mAnimProxy->UpdateClipInfos(mClipInfos);

			if (mDirty.IsSet(AnimDirtyStateFlag::MorphWeights) || didFullRebuild)
				mAnimProxy->UpdateMorphChannelWeights(mMorphChannelWeights);
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
				if (so.IsDestroyed(true))
					hash = 0;
				else
					hash = so->GetTransformHash();

				if (hash != mAnimProxy->sceneObjectInfos[i].hash)
				{
					Vector<AnimatedSceneObject> animatedSOs = getAnimatedSOList();
					mAnimProxy->UpdateTransforms(animatedSOs);
					break;
				}
			}
		}

		mDirty = AnimDirtyState();
	}

	void Animation::UpdateFromProxy()
	{
		// When sampling a single frame we don't want to keep updating the scene objects so they can be moved through other
		// means (e.g. for the purposes of recording new keyframes if running from the editor).
		const bool disableSOUpdates = mAnimProxy->sampleStep == AnimSampleStep::Done;
		if(disableSOUpdates)
			return;

		// If the object was culled, then we have no valid data to read back
		if(mAnimProxy->wasCulled)
			return;

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

			if (so.IsDestroyed(true))
				continue;

			if(soInfo.boneIdx != -1)
			{
				if (mAnimProxy->skeletonPose.hasOverride[soInfo.boneIdx])
					continue;

				Vector3 position = mAnimProxy->skeletonPose.positions[soInfo.boneIdx];
				Quaternion rotation = mAnimProxy->skeletonPose.rotations[soInfo.boneIdx];
				Vector3 scale = mAnimProxy->skeletonPose.scales[soInfo.boneIdx];

				const SPtr<Skeleton>& skeleton = mAnimProxy->skeleton;

				UINT32 parentBoneIdx = skeleton->GetBoneInfo(soInfo.boneIdx).parent;
				if (parentBoneIdx == (UINT32)-1)
				{
					so->SetPosition(position);
					so->SetRotation(rotation);
					so->SetScale(scale);
				}
				else
				{
					while(parentBoneIdx != (UINT32)-1)
					{
						// Update rotation
						const Quaternion& parentOrientation = mAnimProxy->skeletonPose.rotations[parentBoneIdx];
						rotation = parentOrientation * rotation;

						// Update scale
						const Vector3& parentScale = mAnimProxy->skeletonPose.scales[parentBoneIdx];
						scale = parentScale * scale;

						// Update position
						position = parentOrientation.Rotate(parentScale * position);
						position += mAnimProxy->skeletonPose.positions[parentBoneIdx];

						parentBoneIdx = skeleton->GetBoneInfo(parentBoneIdx).parent;
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

					while(rootSO && rootSO.IsDestroyed(true))
						rootSO = rootSO->GetParent();

					Vector3 parentPos = Vector3::ZERO;
					Quaternion parentRot = Quaternion::IDENTITY;
					Vector3 parentScale = Vector3::ONE;

					if(!rootSO.IsDestroyed(true))
					{
						const Transform& tfrm = rootSO->GetTransform();
						parentPos = tfrm.GetPosition();
						parentRot = tfrm.GetRotation();
						parentScale = tfrm.GetScale();
					}

					// Transform from space relative to root's parent to world space
					rotation = parentRot * rotation;

					scale = parentScale * scale;

					position = parentRot.Rotate(parentScale * position);
					position += parentPos;

					so->SetWorldPosition(position);
					so->SetWorldRotation(rotation);
					so->SetWorldScale(scale);
				}
			}
			else
			{
				if (!mAnimProxy->sceneObjectPose.hasOverride[i * 3 + 0])
					so->SetPosition(mAnimProxy->sceneObjectPose.positions[i]);

				if (!mAnimProxy->sceneObjectPose.hasOverride[i * 3 + 1])
					so->SetRotation(mAnimProxy->sceneObjectPose.rotations[i]);

				if (!mAnimProxy->sceneObjectPose.hasOverride[i * 3 + 2])
					so->SetScale(mAnimProxy->sceneObjectPose.scales[i]);
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
					if (clipInfo.clip.IsLoaded() && clipInfo.curveVersion == clipInfo.clip->GetVersion())
					{
						UINT32 numGenericCurves = (UINT32)clipInfo.clip->GetCurves()->generic.size();
						mGenericCurveValuesValid = numGenericCurves == mAnimProxy->numGenericCurves;
					}
				}
			}
		}

		if(mGenericCurveValuesValid)
		{
			mGenericCurveOutputs.Resize(mAnimProxy->numGenericCurves);

			memcpy(mGenericCurveOutputs.Data(), mAnimProxy->genericCurveOutputs, mAnimProxy->numGenericCurves * sizeof(float));
		}
	}
}
