//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DAnimation.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRenderable.h"
#include "Components/B3DBone.h"
#include "Mesh/B3DMesh.h"
#include "Animation/B3DMorphShapes.h"
#include "Animation/B3DAnimationClip.h"
#include "Animation/B3DAnimationUtility.h"
#include "Animation/B3DSkeleton.h"
#include "RTTI/B3DAnimationRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

AnimationClipInfo::AnimationClipInfo(const HAnimationClip& clip)
	: Clip(clip)
{}

AnimationProxy::AnimationProxy(u64 animationId)
	: AnimationId(animationId)
{}

AnimationProxy::~AnimationProxy()
{
	Clear();
}

void AnimationProxy::Clear()
{
	if(Layers == nullptr)
		return;

	for(u32 layerIndex = 0; layerIndex < LayerCount; layerIndex++)
	{
		AnimationStateLayer& layer = Layers[layerIndex];
		for(u32 stateIndex = 0; stateIndex < layer.StateCount; stateIndex++)
		{
			AnimationState& state = layer.States[stateIndex];

			if(state.Curves != nullptr)
			{
				{
					const u32 curveCount = (u32)state.Curves->Position.size();
					for(u32 curveIndex = 0; curveIndex < curveCount; curveIndex++)
						state.PositionCaches[curveIndex].~TCurveCache();
				}

				{
					const u32 curveCount = (u32)state.Curves->Rotation.size();
					for(u32 curveIndex = 0; curveIndex < curveCount; curveIndex++)
						state.RotationCaches[curveIndex].~TCurveCache();
				}

				{
					const u32 curveCount = (u32)state.Curves->Scale.size();
					for(u32 curveIndex = 0; curveIndex < curveCount; curveIndex++)
						state.ScaleCaches[curveIndex].~TCurveCache();
				}

				{
					const u32 curveCount = (u32)state.Curves->Generic.size();
					for(u32 curveIndex = 0; curveIndex < curveCount; curveIndex++)
						state.GenericCaches[curveIndex].~TCurveCache();
				}
			}

			if(Skeleton != nullptr)
			{
				const u32 boneCount = Skeleton->GetBoneCount();
				for(u32 boneIndex = 0; boneIndex < boneCount; boneIndex++)
					state.BoneToCurveMapping[boneIndex].~AnimationCurveMapping();
			}

			if(state.SceneObjectToCurveMapping != nullptr)
			{
				for(u32 sceneObjextIndex = 0; sceneObjextIndex < SceneObjectCount; sceneObjextIndex++)
					state.SceneObjectToCurveMapping[sceneObjextIndex].~AnimationCurveMapping();
			}

			state.~AnimationState();
		}

		layer.~AnimationStateLayer();
	}

	for(u32 morphShapeIndex = 0; morphShapeIndex < MorphShapeCount; morphShapeIndex++)
		MorphShapeInfos[morphShapeIndex].Shape.~SPtr<MorphShape>();

	// All of the memory is part of the same buffer, so we only need to free the first element
	B3DFree(Layers);
	Layers = nullptr;
	GenericCurveOutputs = nullptr;
	SceneObjectInfos = nullptr;
	SceneObjectTransforms = nullptr;

	LayerCount = 0;
	GenericCurveCount = 0;
}

void AnimationProxy::RebuildFull(const SPtr<b3d::Skeleton>& skeleton, const b3d::SkeletonMask& mask, Vector<AnimationClipInfo>& inOutClipInfos, const Vector<SceneObjectMappingCurveInfo>& sceneObjects, const SPtr<MorphShapes>& morphShapes)
{
	this->Skeleton = skeleton;
	this->SkeletonMask = mask;

	// Note: I could avoid having a separate allocation for LocalSkeletonPoses and use the same buffer as the rest
	// of AnimationProxy
	if(skeleton != nullptr)
		SkeletonPose = LocalSkeletonPose(skeleton->GetBoneCount());

	SceneObjectCount = (u32)sceneObjects.size();
	if(SceneObjectCount > 0)
		SceneObjectPose = LocalSkeletonPose(SceneObjectCount, true);
	else
		SceneObjectPose = LocalSkeletonPose();

	RebuildClips(inOutClipInfos, sceneObjects, morphShapes);
}

void AnimationProxy::RebuildClips(Vector<AnimationClipInfo>& inOutClipInfos, const Vector<SceneObjectMappingCurveInfo>& sceneObjects, const SPtr<MorphShapes>& morphShapes)
{
	Clear();

	B3DMarkAllocatorFrame();
	{
		FrameVector<bool> clipLoadState(inOutClipInfos.size());
		FrameVector<AnimationStateLayer> tempLayers;
		u32 clipIndex = 0;
		for(auto& clipInfo : inOutClipInfos)
		{
			u32 layer = clipInfo.State.Layer;
			if(layer == ~0u)
				layer = 0;
			else
				layer += 1;

			auto found = std::find_if(tempLayers.begin(), tempLayers.end(), [&](auto& x)
										 { return x.Index == layer; });

			bool isLoaded = clipInfo.Clip.IsLoaded();
			clipLoadState[clipIndex] = isLoaded;

			if(found == tempLayers.end())
			{
				tempLayers.push_back(AnimationStateLayer());
				AnimationStateLayer& newLayer = tempLayers.back();

				newLayer.Index = layer;
				newLayer.Additive = isLoaded && clipInfo.Clip->IsAdditive();
			}

			clipIndex++;
		}

		std::sort(tempLayers.begin(), tempLayers.end(), [&](auto& x, auto& y)
				  { return x.Index < y.Index; });

		LayerCount = (u32)tempLayers.size();
		const u32 clipCount = (u32)inOutClipInfos.size();
		u32 boneCount;

		if(Skeleton != nullptr)
			boneCount = Skeleton->GetBoneCount();
		else
			boneCount = 0;

		u32 positionCurveCount = 0;
		u32 rotationCurveCount = 0;
		u32 scaleCurveCount = 0;

		clipIndex = 0;
		for(auto& clipInfo : inOutClipInfos)
		{
			bool isLoaded = clipLoadState[clipIndex++];
			if(!isLoaded)
				continue;

			SPtr<AnimationCurves> curves = clipInfo.Clip->GetCurves();
			positionCurveCount += (u32)curves->Position.size();
			rotationCurveCount += (u32)curves->Rotation.size();
			scaleCurveCount += (u32)curves->Scale.size();
		}

		GenericCurveCount = 0;
		if(!inOutClipInfos.empty() && clipLoadState[0])
		{
			SPtr<AnimationCurves> curves = inOutClipInfos[0].Clip->GetCurves();
			GenericCurveCount = (u32)curves->Generic.size();
		}

		u32* mappedBoneIndices = (u32*)B3DFrameAllocate(sizeof(u32) * SceneObjectCount);
		for(u32 sceneObjectIndex = 0; sceneObjectIndex < SceneObjectCount; sceneObjectIndex++)
			mappedBoneIndices[sceneObjectIndex] = -1;

		u32 boneMappedSceneObjectCount = 0;
		if(Skeleton != nullptr)
		{
			for(u32 sceneObjectIndex = 0; sceneObjectIndex < SceneObjectCount; sceneObjectIndex++)
			{
				if(sceneObjects[sceneObjectIndex].Object.IsDestroyed(true))
					continue;

				// Empty string always means root bone
				if(sceneObjects[sceneObjectIndex].CurveName.empty())
				{
					const u32 rootBoneIndex = Skeleton->GetRootBoneIndex();
					if(rootBoneIndex != ~0u)
					{
						mappedBoneIndices[sceneObjectIndex] = rootBoneIndex;
						boneMappedSceneObjectCount++;
					}
				}
				else
				{
					for(u32 boneIndex = 0; boneIndex < boneCount; boneIndex++)
					{
						if(Skeleton->GetBoneInfo(boneIndex).Name == sceneObjects[sceneObjectIndex].CurveName)
						{
							mappedBoneIndices[sceneObjectIndex] = boneIndex;

							boneMappedSceneObjectCount++;
							break;
						}
					}
				}
			}
		}

		if(morphShapes != nullptr)
		{
			MorphChannelCount = morphShapes->GetChannelCount();
			MorphVertexCount = morphShapes->GetVertexCount();

			MorphShapeCount = 0;
			for(u32 morphChannelIndex = 0; morphChannelIndex < MorphChannelCount; morphChannelIndex++)
				MorphShapeCount += morphShapes->GetChannel(morphChannelIndex)->GetShapeCount();
		}
		else
		{
			MorphChannelCount = 0;
			MorphShapeCount = 0;
			MorphVertexCount = 0;
		}

		u32 boneMappingCount = boneCount * clipCount;
		u32 layersSize = sizeof(AnimationStateLayer) * LayerCount;
		u32 clipsSize = sizeof(AnimationState) * clipCount;
		u32 boneMappingSize = boneMappingCount * sizeof(AnimationCurveMapping);
		u32 positionCacheSize = positionCurveCount * sizeof(TCurveCache<Vector3>);
		u32 rotationCacheSize = rotationCurveCount * sizeof(TCurveCache<Quaternion>);
		u32 scaleCacheSize = scaleCurveCount * sizeof(TCurveCache<Vector3>);
		u32 genericCacheSize = GenericCurveCount * sizeof(TCurveCache<float>);
		u32 genericCurveOutputSize = GenericCurveCount * sizeof(float);
		u32 sceneObjectIdsSize = SceneObjectCount * sizeof(AnimatedSceneObjectInfo);
		u32 sceneObjectTransformsSize = boneMappedSceneObjectCount * sizeof(Matrix4);
		u32 morphChannelSize = MorphChannelCount * sizeof(MorphChannelInfo);
		u32 morphShapeSize = MorphShapeCount * sizeof(MorphShapeInfo);

		u8* data = (u8*)B3DAllocate(layersSize + clipsSize + boneMappingSize + positionCacheSize + rotationCacheSize + scaleCacheSize + genericCacheSize + genericCurveOutputSize + sceneObjectIdsSize + sceneObjectTransformsSize + morphChannelSize + morphShapeSize);

		Layers = (AnimationStateLayer*)data;
		memcpy(Layers, tempLayers.data(), layersSize);
		data += layersSize;

		AnimationState* states = (AnimationState*)data;
		for(u32 clipIndex = 0; clipIndex < clipCount; clipIndex++)
			new(&states[clipIndex]) AnimationState();

		data += clipsSize;

		AnimationCurveMapping* boneMappings = (AnimationCurveMapping*)data;
		for(u32 boneMappingIndex = 0; boneMappingIndex < boneMappingCount; boneMappingIndex++)
			new(&boneMappings[boneMappingIndex]) AnimationCurveMapping();

		data += boneMappingSize;

		TCurveCache<Vector3>* positionCache = (TCurveCache<Vector3>*)data;
		for(u32 curveIndex = 0; curveIndex < positionCurveCount; curveIndex++)
			new(&positionCache[curveIndex]) TCurveCache<Vector3>();

		data += positionCacheSize;

		TCurveCache<Quaternion>* rotationCache = (TCurveCache<Quaternion>*)data;
		for(u32 curveIndex = 0; curveIndex < rotationCurveCount; curveIndex++)
			new(&rotationCache[curveIndex]) TCurveCache<Quaternion>();

		data += rotationCacheSize;

		TCurveCache<Vector3>* scaleCache = (TCurveCache<Vector3>*)data;
		for(u32 curveIndex = 0; curveIndex < scaleCurveCount; curveIndex++)
			new(&scaleCache[curveIndex]) TCurveCache<Vector3>();

		data += scaleCacheSize;

		TCurveCache<float>* genericCurveCache = (TCurveCache<float>*)data;
		for(u32 curveIndex = 0; curveIndex < GenericCurveCount; curveIndex++)
			new(&genericCurveCache[curveIndex]) TCurveCache<float>();

		data += genericCacheSize;

		GenericCurveOutputs = (float*)data;
		data += genericCurveOutputSize;

		SceneObjectInfos = (AnimatedSceneObjectInfo*)data;
		data += sceneObjectIdsSize;

		SceneObjectTransforms = (Matrix4*)data;
		for(u32 transformIndex = 0; transformIndex < boneMappedSceneObjectCount; transformIndex++)
			SceneObjectTransforms[transformIndex] = Matrix4::kIdentity;

		data += sceneObjectTransformsSize;

		MorphChannelInfos = (MorphChannelInfo*)data;
		data += morphChannelSize;

		MorphShapeInfos = (MorphShapeInfo*)data;
		data += morphShapeSize;

		// Generate data required for morph shape animation
		if(morphShapes != nullptr)
		{
			u32 currentShapeIndex = 0;
			for(u32 morphChannelIndex = 0; morphChannelIndex < MorphChannelCount; morphChannelIndex++)
			{
				SPtr<MorphChannel> morphChannel = morphShapes->GetChannel(morphChannelIndex);
				const u32 shapeCount = morphChannel->GetShapeCount();

				MorphChannelInfo& channelInfo = MorphChannelInfos[morphChannelIndex];
				channelInfo.Weight = 0.0f;
				channelInfo.ShapeStart = currentShapeIndex;
				channelInfo.ShapeCount = shapeCount;
				channelInfo.FrameCurveIndex = ~0u;
				channelInfo.WeightCurveIdx = ~0u;

				for(u32 shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++)
				{
					MorphShapeInfo& shapeInfo = MorphShapeInfos[currentShapeIndex];
					new(&shapeInfo.Shape) SPtr<MorphShape>();

					SPtr<MorphShape> shape = morphChannel->GetShape(shapeIndex);
					shapeInfo.Shape = shape;
					shapeInfo.FrameWeight = shape->GetWeight();
					shapeInfo.FinalWeight = 0.0f;

					currentShapeIndex++;
				}
			}

			// Find any curves affecting morph shape animation
			if(!inOutClipInfos.empty())
			{
				const bool isClipValid = clipLoadState[0];
				if(isClipValid)
				{
					AnimationClipInfo& clipInfo = inOutClipInfos[0];

					for(u32 morphChannelIndex = 0; morphChannelIndex < MorphChannelCount; morphChannelIndex++)
					{
						SPtr<MorphChannel> morphChannel = morphShapes->GetChannel(morphChannelIndex);
						MorphChannelInfo& channelInfo = MorphChannelInfos[morphChannelIndex];

						clipInfo.Clip->GetMorphMapping(morphChannel->GetName(), channelInfo.FrameCurveIndex, channelInfo.WeightCurveIdx);
					}
				}
			}

			MorphChannelWeightsDirty = true;
		}

		u32 currentLayerIndex = 0;
		u32 currentStateIndex = 0;

		// Note: Hidden dependency. First clip info must be in layers[0].states[0] (needed for generic curves which only
		// use the primary clip).
		for(u32 layerIndex = 0; layerIndex < LayerCount; layerIndex++)
		{
			AnimationStateLayer& layer = Layers[layerIndex];

			layer.States = &states[currentStateIndex];
			layer.StateCount = 0;

			u32 localStateIdx = 0;
			for(u32 clipInfoIndex = 0; clipInfoIndex < (u32)inOutClipInfos.size(); clipInfoIndex++)
			{
				AnimationClipInfo& clipInfo = inOutClipInfos[clipInfoIndex];

				u32 clipLayer = clipInfo.State.Layer;
				if(clipLayer == (u32)-1)
					clipLayer = 0;
				else
					clipLayer += 1;

				if(clipLayer != layer.Index)
					continue;

				AnimationState& state = states[currentStateIndex];
				state.Loop = clipInfo.State.WrapMode == AnimationWrapMode::Loop;

				// Calculate weight if fading is active
				float weight = clipInfo.State.Weight;

				//// Assumes time is clamped to [0, fadeLength] and fadeLength != 0
				if(clipInfo.FadeDirection < 0.0f)
				{
					float t = clipInfo.FadeTime / clipInfo.FadeLength;
					weight *= (1.0f - t);
				}
				else if(clipInfo.FadeDirection > 0.0f)
				{
					float t = clipInfo.FadeTime / clipInfo.FadeLength;
					weight *= t;
				}

				state.Weight = weight;

				// Set up individual curves and their caches
				bool isClipValid = clipLoadState[clipInfoIndex];
				if(isClipValid)
				{
					state.Curves = clipInfo.Clip->GetCurves();
					state.Length = clipInfo.Clip->GetLength();
					state.Disabled = clipInfo.PlaybackType == AnimationPlaybackType::None;
				}
				else
				{
					static SPtr<AnimationCurves> zeroCurves = B3DMakeShared<AnimationCurves>();
					state.Curves = zeroCurves;
					state.Length = 0.0f;
					state.Disabled = true;
				}

				// Wrap time if looping
				if(state.Loop && state.Length > 0.0f)
					state.Time = Math::Repeat(clipInfo.State.Time, state.Length);
				else
					state.Time = clipInfo.State.Time;

				state.PositionCaches = positionCache;
				positionCache += state.Curves->Position.size();

				state.RotationCaches = rotationCache;
				rotationCache += state.Curves->Rotation.size();

				state.ScaleCaches = scaleCache;
				scaleCache += state.Curves->Scale.size();

				state.GenericCaches = genericCurveCache;
				genericCurveCache += state.Curves->Generic.size();

				clipInfo.LayerIndex = currentLayerIndex;
				clipInfo.StateIndex = localStateIdx;

				if(isClipValid)
					clipInfo.CurveVersion = clipInfo.Clip->GetVersion();

				// Set up bone mapping
				if(Skeleton != nullptr)
				{
					state.BoneToCurveMapping = &boneMappings[currentStateIndex * boneCount];

					if(isClipValid)
					{
						clipInfo.Clip->GetBoneMapping(*Skeleton, state.BoneToCurveMapping);
					}
					else
					{
						AnimationCurveMapping emptyMapping = { ~0u, ~0u, ~0u };

						for(u32 boneIndex = 0; boneIndex < boneCount; boneIndex++)
							state.BoneToCurveMapping[boneIndex] = emptyMapping;
					}
				}
				else
					state.BoneToCurveMapping = nullptr;

				layer.StateCount++;
				currentStateIndex++;
				localStateIdx++;
			}

			currentLayerIndex++;

			// Must be larger than zero otherwise the layer.states pointer will point to data held by some other layer
			B3D_ASSERT(layer.StateCount > 0);
		}

		Matrix4 invRootTransform(BsIdentity);
		for(u32 sceneObjectIndex = 0; sceneObjectIndex < SceneObjectCount; sceneObjectIndex++)
		{
			if(sceneObjects[sceneObjectIndex].CurveName.empty())
			{
				HSceneObject so = sceneObjects[sceneObjectIndex].Object;
				if(!so.IsDestroyed(true))
					invRootTransform = so->GetWorldMatrix().InverseAffine();

				break;
			}
		}

		u32 boneIndex = 0;
		for(u32 sceneObjectIndex = 0; sceneObjectIndex < SceneObjectCount; sceneObjectIndex++)
		{
			HSceneObject sceneObject = sceneObjects[sceneObjectIndex].Object;
			AnimatedSceneObjectInfo& animatedSceneObjectInfo = SceneObjectInfos[sceneObjectIndex];
			animatedSceneObjectInfo.SceneObjectId = sceneObject.GetId();
			animatedSceneObjectInfo.BoneIndex = (i32)mappedBoneIndices[sceneObjectIndex];

			bool isSceneObjectValid = !sceneObject.IsDestroyed(true);
			if(isSceneObjectValid)
				animatedSceneObjectInfo.Hash = sceneObject->GetTransformHash();
			else
				animatedSceneObjectInfo.Hash = 0;

			animatedSceneObjectInfo.LayerIndex = ~0u;
			animatedSceneObjectInfo.StateIndex = ~0u;

			// If no bone mapping, find curves directly
			if(animatedSceneObjectInfo.BoneIndex == -1)
			{
				animatedSceneObjectInfo.CurveIndices = { ~0u, ~0u, ~0u };

				if(isSceneObjectValid)
				{
					for(u32 clipInfoIndex = 0; clipInfoIndex < (u32)inOutClipInfos.size(); clipInfoIndex++)
					{
						AnimationClipInfo& clipInfo = inOutClipInfos[clipInfoIndex];

						animatedSceneObjectInfo.LayerIndex = clipInfo.LayerIndex;
						animatedSceneObjectInfo.StateIndex = clipInfo.StateIndex;

						bool isClipValid = clipLoadState[clipInfoIndex];
						if(isClipValid)
						{
							// Note: If there are multiple clips with the relevant curve name, we only use the first
							clipInfo.Clip->GetCurveMapping(sceneObjects[sceneObjectIndex].CurveName, animatedSceneObjectInfo.CurveIndices);
							break;
						}
					}
				}
			}
			else
			{
				// No need to check if SO is valid, if it has a bone connection it must be
				SceneObjectTransforms[boneIndex] = sceneObject->GetWorldMatrix() * invRootTransform;
				boneIndex++;
			}
		}

		B3DFrameFree(mappedBoneIndices);
	}
	B3DClearAllocatorFrame();
}

void AnimationProxy::UpdateClipInfos(const Vector<AnimationClipInfo>& clipInfos)
{
	for(auto& clipInfo : clipInfos)
	{
		AnimationState& state = Layers[clipInfo.LayerIndex].States[clipInfo.StateIndex];

		state.Loop = clipInfo.State.WrapMode == AnimationWrapMode::Loop;
		state.Weight = clipInfo.State.Weight;

		// Wrap time if looping
		if(state.Loop && state.Length > 0.0f)
			state.Time = Math::Repeat(clipInfo.State.Time, state.Length);
		else
			state.Time = clipInfo.State.Time;

		bool isLoaded = clipInfo.Clip.IsLoaded();
		state.Disabled = !isLoaded || clipInfo.PlaybackType == AnimationPlaybackType::None;
	}
}

void AnimationProxy::UpdateMorphChannelWeights(const Vector<float>& weights)
{
	const u32 weightCount = (u32)weights.size();
	for(u32 morphChannelIndex = 0; morphChannelIndex < MorphChannelCount; morphChannelIndex++)
	{
		if(morphChannelIndex < weightCount)
			MorphChannelInfos[morphChannelIndex].Weight = weights[morphChannelIndex];
		else
			MorphChannelInfos[morphChannelIndex].Weight = 0.0f;
	}

	MorphChannelWeightsDirty = true;
}

void AnimationProxy::UpdateTransforms(const Vector<SceneObjectMappingCurveInfo>& sceneObjects)
{
	Matrix4 invRootTransform(BsIdentity);
	for(u32 sceneObjectIndex = 0; sceneObjectIndex < SceneObjectCount; sceneObjectIndex++)
	{
		if(sceneObjects[sceneObjectIndex].CurveName.empty())
		{
			HSceneObject so = sceneObjects[sceneObjectIndex].Object;
			if(!so.IsDestroyed(true))
				invRootTransform = so->GetWorldMatrix().InverseAffine();

			break;
		}
	}

	u32 boneIndex = 0;
	for(u32 sceneObjectIndex = 0; sceneObjectIndex < SceneObjectCount; sceneObjectIndex++)
	{
		HSceneObject sceneObject = sceneObjects[sceneObjectIndex].Object;
		if(sceneObject.IsDestroyed(true))
		{
			SceneObjectInfos[sceneObjectIndex].Hash = 0;
			continue;
		}

		SceneObjectInfos[sceneObjectIndex].Hash = sceneObject->GetTransformHash();

		if(SceneObjectInfos[sceneObjectIndex].BoneIndex == -1)
			continue;

		SceneObjectTransforms[boneIndex] = sceneObjects[sceneObjectIndex].Object->GetWorldMatrix() * invRootTransform;
		boneIndex++;
	}
}

void AnimationProxy::UpdateTime(const Vector<AnimationClipInfo>& clipInfos)
{
	for(auto& clipInfo : clipInfos)
	{
		AnimationState& state = Layers[clipInfo.LayerIndex].States[clipInfo.StateIndex];

		// Wrap time if looping
		if(state.Loop && state.Length > 0.0f)
			state.Time = Math::Repeat(clipInfo.State.Time, state.Length);
		else
			state.Time = clipInfo.State.Time;

		bool isLoaded = clipInfo.Clip.IsLoaded();
		state.Disabled = !isLoaded || clipInfo.PlaybackType == AnimationPlaybackType::None;
	}
}


Animation::Animation(const HSceneObject& parent)
	: Component(parent)
{
	mNotifyFlags = TCF_Transform;
	SetFlag(ComponentFlag::AlwaysRun, true);

	SetName("Animation");
}

Animation::Animation()
	: Animation(nullptr)
{ }

void Animation::SetDefaultClip(const HAnimationClip& clip)
{
	mDefaultClip = clip;

	if(clip.IsLoaded() && mAnimationId != ~0u && !mPreviewMode)
		Play(clip);
}

void Animation::SetWrapMode(AnimationWrapMode wrapMode)
{
	if(mWrapMode == wrapMode)
		return;

	mWrapMode = wrapMode;

	for(auto& clipInfo : mClipInfos)
		clipInfo.State.WrapMode = wrapMode;

	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::SetSpeed(float speed)
{
	mSpeed = speed;

	for(auto& clipInfo : mClipInfos)
	{
		// Special case: Ignore non-moving ones
		if(!clipInfo.State.Stopped)
			clipInfo.State.Speed = speed;
	}

	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::Play(const HAnimationClip& clip)
{
	if(!IsAnimationProxyValid() || mPreviewMode)
		return;

	AnimationClipInfo* clipInfo = GetOrCreateClipInfo(clip, ~0u);
	if(clipInfo != nullptr)
	{
		clipInfo->State.Time = 0.0f;
		clipInfo->State.Speed = mSpeed;
		clipInfo->State.Weight = 1.0f;
		clipInfo->State.WrapMode = mWrapMode;
		clipInfo->PlaybackType = AnimationPlaybackType::Normal;
	}

	mSampleStep = AnimationSampleStep::None;
	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength, u32 layer)
{
	if(!IsAnimationProxyValid() || mPreviewMode)
		return;

	if(clip != nullptr && !clip->IsAdditive())
	{
		B3D_LOG(Warning, Renderer, "BlendAdditive() called with a clip that doesn't contain additive animation. Ignoring.");

		// Stop any clips on this layer, even if invalid
		HAnimationClip nullClip;
		GetOrCreateClipInfo(nullClip, layer);

		mSampleStep = AnimationSampleStep::None;
		return;
	}

	AnimationClipInfo* clipInfo = GetOrCreateClipInfo(clip, layer);
	if(clipInfo != nullptr)
	{
		clipInfo->State.Time = 0.0f;
		clipInfo->State.Speed = mSpeed;
		clipInfo->State.Weight = weight;
		clipInfo->State.WrapMode = mWrapMode;

		if(fadeLength > 0.0f)
		{
			clipInfo->FadeDirection = 1.0f;
			clipInfo->FadeTime = 0.0f;
			clipInfo->FadeLength = fadeLength;
		}

		clipInfo->PlaybackType = AnimationPlaybackType::Normal;

		mSampleStep = AnimationSampleStep::None;
		mDirty |= AnimationDirtyStateFlag::Value;
	}
}

void Animation::Blend1D(const Blend1DInfo& info, float alpha)
{
	if(!IsAnimationProxyValid() || mPreviewMode)
		return;

	if(info.Clips.empty())
		return;

	// Find valid range
	float startPos = 0.0f;
	float endPos = 0.0f;

	for(u32 clipIndex = 0; clipIndex < (u32)info.Clips.size(); clipIndex++)
	{
		startPos = std::min(startPos, info.Clips[clipIndex].Position);
		endPos = std::min(endPos, info.Clips[clipIndex].Position);
	}

	float length = endPos - startPos;
	if(Math::ApproxEquals(length, 0.0f) || info.Clips.size() < 2)
	{
		Play(info.Clips[0].Clip);
		return;
	}

	// Clamp or loop time
	bool loop = mWrapMode == AnimationWrapMode::Loop;
	if(alpha < startPos)
	{
		if(loop)
			alpha = alpha - std::floor(alpha / length) * length;
		else // Clamping
			alpha = startPos;
	}

	if(alpha > endPos)
	{
		if(loop)
			alpha = alpha - std::floor(alpha / length) * length;
		else // Clamping
			alpha = endPos;
	}

	// Find keys to blend between
	i32 start = 0;
	i32 searchLength = (i32)info.Clips.size();

	while(searchLength > 0)
	{
		i32 half = searchLength >> 1;
		i32 mid = start + half;

		if(alpha < info.Clips[mid].Position)
		{
			searchLength = half;
		}
		else
		{
			start = mid + 1;
			searchLength -= (half + 1);
		}
	}

	u32 leftKey = std::max(0, start - 1);
	u32 rightKey = std::min(start, (i32)info.Clips.size() - 1);

	float interpLength = info.Clips[rightKey].Position - info.Clips[leftKey].Position;
	alpha = (alpha - info.Clips[leftKey].Position) / interpLength;

	// Add clips and set weights
	for(u32 clipIndex = 0; clipIndex < (u32)info.Clips.size(); clipIndex++)
	{
		AnimationClipInfo* clipInfo = GetOrCreateClipInfo(info.Clips[clipIndex].Clip, ~0u, clipIndex == 0);
		if(clipInfo != nullptr)
		{
			clipInfo->State.Time = 0.0f;
			clipInfo->State.Stopped = true;
			clipInfo->State.Speed = 0.0f;
			clipInfo->State.WrapMode = AnimationWrapMode::Clamp;

			if(clipIndex == leftKey)
				clipInfo->State.Weight = 1.0f - alpha;
			else if(clipIndex == rightKey)
				clipInfo->State.Weight = alpha;
			else
				clipInfo->State.Weight = 0.0f;

			clipInfo->PlaybackType = AnimationPlaybackType::Normal;
		}
	}

	mSampleStep = AnimationSampleStep::None;
	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::Blend2D(const Blend2DInfo& info, const Vector2& alpha)
{
	if(!IsAnimationProxyValid() || mPreviewMode)
		return;

	AnimationClipInfo* topLeftClipInfo = GetOrCreateClipInfo(info.TopLeftClip, ~0u, true);
	if(topLeftClipInfo != nullptr)
	{
		topLeftClipInfo->State.Time = 0.0f;
		topLeftClipInfo->State.Stopped = true;
		topLeftClipInfo->State.Speed = 0.0f;
		topLeftClipInfo->State.Weight = (1.0f - alpha.X) * (1.0f - alpha.Y);
		topLeftClipInfo->State.WrapMode = AnimationWrapMode::Clamp;

		topLeftClipInfo->PlaybackType = AnimationPlaybackType::Normal;
	}

	AnimationClipInfo* topRightClipInfo = GetOrCreateClipInfo(info.TopRightClip, ~0u, false);
	if(topRightClipInfo != nullptr)
	{
		topRightClipInfo->State.Time = 0.0f;
		topRightClipInfo->State.Stopped = true;
		topRightClipInfo->State.Speed = 0.0f;
		topRightClipInfo->State.Weight = alpha.X * (1.0f - alpha.Y);
		topRightClipInfo->State.WrapMode = AnimationWrapMode::Clamp;

		topRightClipInfo->PlaybackType = AnimationPlaybackType::Normal;
	}

	AnimationClipInfo* botLeftClipInfo = GetOrCreateClipInfo(info.BottomLeftClip, ~0u, false);
	if(botLeftClipInfo != nullptr)
	{
		botLeftClipInfo->State.Time = 0.0f;
		botLeftClipInfo->State.Stopped = true;
		botLeftClipInfo->State.Speed = 0.0f;
		botLeftClipInfo->State.Weight = (1.0f - alpha.X) * alpha.Y;
		botLeftClipInfo->State.WrapMode = AnimationWrapMode::Clamp;

		botLeftClipInfo->PlaybackType = AnimationPlaybackType::Normal;
	}

	AnimationClipInfo* botRightClipInfo = GetOrCreateClipInfo(info.BottomRightClip, ~0u, false);
	if(botRightClipInfo != nullptr)
	{
		botRightClipInfo->State.Time = 0.0f;
		botRightClipInfo->State.Stopped = true;
		botRightClipInfo->State.Speed = 0.0f;
		botRightClipInfo->State.Weight = alpha.X * alpha.Y;
		botRightClipInfo->State.WrapMode = AnimationWrapMode::Clamp;

		botRightClipInfo->PlaybackType = AnimationPlaybackType::Normal;
	}

	mSampleStep = AnimationSampleStep::None;
	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::CrossFade(const HAnimationClip& clip, float fadeLength)
{
	if(!IsAnimationProxyValid() || mPreviewMode)
		return;

	const bool isFading = fadeLength > 0.0f;
	if(!isFading)
	{
		Play(clip);
		return;
	}

	AnimationClipInfo* clipInfo = GetOrCreateClipInfo(clip, ~0u, false);
	if(clipInfo != nullptr)
	{
		clipInfo->State.Time = 0.0f;
		clipInfo->State.Speed = mSpeed;
		clipInfo->State.Weight = 1.0f;
		clipInfo->State.WrapMode = mWrapMode;
		clipInfo->PlaybackType = AnimationPlaybackType::Normal;

		// Set up fade lengths
		clipInfo->FadeDirection = 1.0f;
		clipInfo->FadeTime = 0.0f;
		clipInfo->FadeLength = fadeLength;

		for(auto& entry : mClipInfos)
		{
			if(entry.State.Layer == ~0u && entry.Clip != clip)
			{
				// If other clips are already cross-fading, we need to persist their current weight before starting
				// a new crossfade. We do that by adjusting the fade times.
				if(clipInfo->FadeDirection != 0 && clipInfo->FadeTime < clipInfo->FadeLength)
				{
					float t = clipInfo->FadeTime / clipInfo->FadeLength;
					if(clipInfo->FadeDirection < 0.0f)
						t = (1.0f - t);

					clipInfo->State.Weight *= t;
				}

				clipInfo->FadeDirection = -1.0f;
				clipInfo->FadeTime = 0.0f;
				clipInfo->FadeLength = fadeLength;
			}
		}
	}

	mSampleStep = AnimationSampleStep::None;
	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::Sample(const HAnimationClip& clip, float time)
{
	if(!IsAnimationProxyValid())
		return;

	AnimationClipInfo* clipInfo = GetOrCreateClipInfo(clip, ~0u);
	if(clipInfo != nullptr)
	{
		clipInfo->State.Time = time;
		clipInfo->State.Speed = 0.0f;
		clipInfo->State.Weight = 1.0f;
		clipInfo->State.WrapMode = mWrapMode;
		clipInfo->PlaybackType = AnimationPlaybackType::Sampled;
	}

	mSampleStep = AnimationSampleStep::Frame;
	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::Stop(u32 layer)
{
	if(!IsAnimationProxyValid() || mPreviewMode)
		return;

	B3DMarkAllocatorFrame();
	{
		FrameVector<AnimationClipInfo> newClips;
		for(auto& clipInfo : mClipInfos)
		{
			if(clipInfo.State.Layer != layer)
				newClips.push_back(clipInfo);
			else
				mDirty |= AnimationDirtyStateFlag::Layout;
		}

		mClipInfos.resize(newClips.size());
		for(u32 clipIndex = 0; clipIndex < (u32)newClips.size(); clipIndex++)
			mClipInfos[clipIndex] = newClips[clipIndex];
	}
	B3DClearAllocatorFrame();
}

void Animation::StopAll()
{
	if(!IsAnimationProxyValid() || mPreviewMode)
		return;

	mClipInfos.clear();

	mSampleStep = AnimationSampleStep::None;
	mDirty |= AnimationDirtyStateFlag::Layout;
}

bool Animation::IsPlaying() const
{
	if(IsAnimationProxyValid())
	{
		for(auto& clipInfo : mClipInfos)
		{
			if(clipInfo.Clip.IsLoaded())
				return true;
		}
	}

	return false;
}

bool Animation::GetState(const HAnimationClip& clip, AnimationClipState& outState)
{
	if(!IsAnimationProxyValid())
		return false;

	if(clip == nullptr)
		return false;

	for(auto& clipInfo : mClipInfos)
	{
		if(clipInfo.Clip == clip)
		{
			outState = clipInfo.State;

			if(outState.Layer == ~0u)
				outState.Layer = 0;
			else
				outState.Layer += 1;

			// Internally we store unclamped time, so clamp/loop it
			float clipLength = 0.0f;
			if(clip.IsLoaded())
				clipLength = clip->GetLength();

			bool loop = clipInfo.State.WrapMode == AnimationWrapMode::Loop;
			AnimationUtility::WrapTime(clipInfo.State.Time, 0.0f, clipLength, loop);

			return true;
		}
	}

	return false;
}

void Animation::SetState(const HAnimationClip& clip, AnimationClipState state)
{
	if(!IsAnimationProxyValid())
		return;

	if(state.Layer == 0)
		state.Layer = ~0u;
	else
		state.Layer -= 1;

	AnimationClipInfo* clipInfo = GetOrCreateClipInfo(clip, state.Layer, false);

	if(clipInfo == nullptr)
		return;

	clipInfo->State = state;
	clipInfo->PlaybackType = AnimationPlaybackType::Normal;

	mSampleStep = AnimationSampleStep::None;
	mDirty |= AnimationDirtyStateFlag::Value;
}

void Animation::SetSkeleton(const SPtr<Skeleton>& skeleton)
{
	mSkeleton = skeleton;
	mDirty |= AnimationDirtyStateFlag::All;
}

void Animation::SetMorphShapes(const SPtr<MorphShapes>& morphShapes)
{
	mMorphShapes = morphShapes;

	u32 channelCount;
	if(mMorphShapes != nullptr)
		channelCount = mMorphShapes->GetChannelCount();
	else
		channelCount = 0;

	mMorphChannelWeights.assign(channelCount, 0.0f);
	if(channelCount > 0)
		mMorphChannelWeights[0] = 1.0f;

	mDirty |= AnimationDirtyStateFlag::Layout;
	mDirty |= AnimationDirtyStateFlag::MorphWeights;
}

void Animation::SetMask(const SkeletonMask& mask)
{
	mSkeletonMask = mask;
	mDirty |= AnimationDirtyStateFlag::All;
}

void Animation::SetMorphChannelWeight(const String& name, float weight)
{
	if(!IsAnimationProxyValid())
		return;

	if(mAnimatedRenderable == nullptr)
		return;

	HMesh mesh = mAnimatedRenderable->GetMesh();
	if(!mesh.IsLoaded())
		return;

	SPtr<MorphShapes> morphShapes = mesh->GetMorphShapes();
	if(morphShapes == nullptr)
		return;

	const Vector<SPtr<MorphChannel>>& channels = morphShapes->GetChannels();
	for(u32 morphChannelIndex = 0; morphChannelIndex < (u32)channels.size(); morphChannelIndex++)
	{
		if(channels[morphChannelIndex]->GetName() == name)
		{
			const u32 weightCount = (u32)mMorphChannelWeights.size();
			if(morphChannelIndex < weightCount)
			{
				mMorphChannelWeights[morphChannelIndex] = weight;
				mDirty |= AnimationDirtyStateFlag::MorphWeights;
			}

			break;
		}
	}
}

void Animation::SetCustomBounds(const AABox& bounds)
{
	mCustomBounds = bounds;

	if(mUseCustomBounds)
	{
		if(mAnimatedRenderable != nullptr)
		{
			SPtr<Renderable> renderable = mAnimatedRenderable.GetShared();
			if(renderable != nullptr)
				renderable->SetOverrideBounds(bounds);

			if(!mPreviewMode)
			{
				mCullingBounds = mCustomBounds;
				mCullingBounds.TransformAffine(SO()->GetWorldMatrix());

				mDirty |= AnimationDirtyStateFlag::Culling;
			}
		}
	}
}

void Animation::SetUseCustomBounds(bool enable)
{
	mUseCustomBounds = enable;

	UpdateBounds();
}

void Animation::SetEnableCull(bool enable)
{
	if(mEnableCull == enable)
		return;

	mEnableCull = enable;
	mDirty |= AnimationDirtyStateFlag::Culling;
}

u32 Animation::GetClipCount() const
{
	if(!IsAnimationProxyValid())
		return 0;

	return (u32)mClipInfos.size();

	return 0;
}

HAnimationClip Animation::GetClip(u32 index) const
{
	if(!IsAnimationProxyValid() || index >= (u32)mClipInfos.size())
		return nullptr;

	return mClipInfos[index].Clip;
}

void Animation::TriggerEvents(float timeDelta)
{
	if(mPreviewMode)
		return;

	for(auto& clipInfo : mClipInfos)
	{
		if(!clipInfo.Clip.IsLoaded())
			continue;

		const Vector<AnimationEvent>& events = clipInfo.Clip->GetEvents();
		bool loop = clipInfo.State.WrapMode == AnimationWrapMode::Loop;

		float start = std::max(clipInfo.State.Time - timeDelta, 0.0f);
		float end = clipInfo.State.Time;
		float clipLength = clipInfo.Clip->GetLength();

		float wrappedStart = start;
		float wrappedEnd = end;
		AnimationUtility::WrapTime(wrappedStart, 0.0f, clipLength, loop);
		AnimationUtility::WrapTime(wrappedEnd, 0.0f, clipLength, loop);

		if(!loop)
		{
			for(auto& event : events)
			{
				if(event.Time >= wrappedStart && (event.Time < wrappedEnd || (event.Time == clipLength && start < clipLength && end >= clipLength)))
					NotifyAnimationEventTriggered(clipInfo.Clip, event.Name);
			}
		}
		else
		{
			if(wrappedStart < wrappedEnd)
			{
				for(auto& event : events)
				{
					if(event.Time >= wrappedStart && event.Time < wrappedEnd)
						NotifyAnimationEventTriggered(clipInfo.Clip, event.Name);
				}
			}
			else if(wrappedEnd < wrappedStart) // End is looped, but start is not
			{
				for(auto& event : events)
				{
					if((event.Time >= wrappedStart && event.Time <= clipLength) || (event.Time >= 0 && event.Time < wrappedEnd))
						NotifyAnimationEventTriggered(clipInfo.Clip, event.Name);
				}
			}
		}
	}
}

void Animation::UpdateAnimationProxy(float timeDelta)
{
	// Check if any of the clip curves are dirty and advance time, perform fading
	for(auto& clipInfo : mClipInfos)
	{
		float scaledTimeDelta = timeDelta * clipInfo.State.Speed;
		clipInfo.State.Time += scaledTimeDelta;

		HAnimationClip clip = clipInfo.Clip;
		if(clip.IsLoaded())
		{
			if(clipInfo.CurveVersion != clip->GetVersion())
				mDirty |= AnimationDirtyStateFlag::Layout;
		}

		float fadeTime = clipInfo.FadeTime + scaledTimeDelta;
		clipInfo.FadeTime = Math::Clamp(fadeTime, 0.0f, clipInfo.FadeLength);
	}

	if(mSampleStep == AnimationSampleStep::None)
		mAnimationProxy->SampleStep = AnimationSampleStep::None;
	else if(mSampleStep == AnimationSampleStep::Frame)
	{
		if(mAnimationProxy->SampleStep == AnimationSampleStep::None)
			mAnimationProxy->SampleStep = AnimationSampleStep::Frame;
		else
			mAnimationProxy->SampleStep = AnimationSampleStep::Done;
	}

	if(mDirty.IsSet(AnimationDirtyStateFlag::Culling))
	{
		mAnimationProxy->CullEnabled = mEnableCull;
		mAnimationProxy->Bounds = mCullingBounds;

		mDirty.Unset(AnimationDirtyStateFlag::Culling);
	}

	auto fnGetAnimatedSceneObjects = [&]()
	{
		Vector<SceneObjectMappingCurveInfo> animatedSceneObjects(mMappedSceneObjectsById.size());
		u32 sceneObjectIndex = 0;
		for(auto& entry : mMappedSceneObjectsById)
			animatedSceneObjects[sceneObjectIndex++] = entry.second;

		return animatedSceneObjects;
	};

	bool didFullRebuild = false;
	if((u32)mDirty == 0) // Clean
	{
		mAnimationProxy->UpdateTime(mClipInfos);
	}
	else
	{
		if(mDirty.IsSet(AnimationDirtyStateFlag::All))
		{
			Vector<SceneObjectMappingCurveInfo> animatedSceneObjects = fnGetAnimatedSceneObjects();

			mAnimationProxy->RebuildFull(mSkeleton, mSkeletonMask, mClipInfos, animatedSceneObjects, mMorphShapes);
			didFullRebuild = true;
		}
		else if(mDirty.IsSet(AnimationDirtyStateFlag::Layout))
		{
			Vector<SceneObjectMappingCurveInfo> animatedSceneObjects = fnGetAnimatedSceneObjects();

			mAnimationProxy->RebuildClips(mClipInfos, animatedSceneObjects, mMorphShapes);
			didFullRebuild = true;
		}
		else if(mDirty.IsSet(AnimationDirtyStateFlag::Value))
			mAnimationProxy->UpdateClipInfos(mClipInfos);

		if(mDirty.IsSet(AnimationDirtyStateFlag::MorphWeights) || didFullRebuild)
			mAnimationProxy->UpdateMorphChannelWeights(mMorphChannelWeights);
	}

	// Check if there are dirty transforms
	if(!didFullRebuild)
	{
		for(u32 sceneObjectIndex = 0; sceneObjectIndex < mAnimationProxy->SceneObjectCount; sceneObjectIndex++)
		{
			AnimatedSceneObjectInfo& soInfo = mAnimationProxy->SceneObjectInfos[sceneObjectIndex];

			auto it = mMappedSceneObjectsById.find(soInfo.SceneObjectId);
			if(it == mMappedSceneObjectsById.end())
			{
				B3D_ASSERT(false); // Should never happen
				continue;
			}

			u32 hash;

			HSceneObject sceneObject = it->second.Object;
			if(sceneObject.IsDestroyed(true))
				hash = 0;
			else
				hash = sceneObject->GetTransformHash();

			if(hash != mAnimationProxy->SceneObjectInfos[sceneObjectIndex].Hash)
			{
				Vector<SceneObjectMappingCurveInfo> animatedSOs = fnGetAnimatedSceneObjects();
				mAnimationProxy->UpdateTransforms(animatedSOs);
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
	const bool disableSceneObjectUpdates = mAnimationProxy->SampleStep == AnimationSampleStep::Done;
	if(disableSceneObjectUpdates)
		return;

	// If the object was culled, then we have no valid data to read back
	if(mAnimationProxy->WasCulled)
		return;

	HSceneObject rootSceneObject;

	// Write TRS animation results to relevant SceneObjects
	for(u32 sceneObjectIndex = 0; sceneObjectIndex < mAnimationProxy->SceneObjectCount; sceneObjectIndex++)
	{
		AnimatedSceneObjectInfo& soInfo = mAnimationProxy->SceneObjectInfos[sceneObjectIndex];

		auto it = mMappedSceneObjectsById.find(soInfo.SceneObjectId);
		if(it == mMappedSceneObjectsById.end())
			continue;

		HSceneObject sceneObject = it->second.Object;
		if(it->second.CurveName.empty())
			rootSceneObject = sceneObject;

		if(sceneObject.IsDestroyed(true))
			continue;

		if(soInfo.BoneIndex != -1)
		{
			if(mAnimationProxy->SkeletonPose.HasOverride[soInfo.BoneIndex])
				continue;

			Vector3 position = mAnimationProxy->SkeletonPose.Positions[soInfo.BoneIndex];
			Quaternion rotation = mAnimationProxy->SkeletonPose.Rotations[soInfo.BoneIndex];
			Vector3 scale = mAnimationProxy->SkeletonPose.Scales[soInfo.BoneIndex];

			const SPtr<Skeleton>& skeleton = mAnimationProxy->Skeleton;

			u32 parentBoneIndex = skeleton->GetBoneInfo(soInfo.BoneIndex).Parent;
			if(parentBoneIndex == (u32)-1)
			{
				sceneObject->SetPosition(position);
				sceneObject->SetRotation(rotation);
				sceneObject->SetScale(scale);
			}
			else
			{
				while(parentBoneIndex != (u32)-1)
				{
					// Update rotation
					const Quaternion& parentOrientation = mAnimationProxy->SkeletonPose.Rotations[parentBoneIndex];
					rotation = parentOrientation * rotation;

					// Update scale
					const Vector3& parentScale = mAnimationProxy->SkeletonPose.Scales[parentBoneIndex];
					scale = parentScale * scale;

					// Update position
					position = parentOrientation.Rotate(parentScale * position);
					position += mAnimationProxy->SkeletonPose.Positions[parentBoneIndex];

					parentBoneIndex = skeleton->GetBoneInfo(parentBoneIndex).Parent;
				}

				// Search for root if not already found
				if(rootSceneObject == nullptr)
				{
					for(auto& entry : mMappedSceneObjectsById)
					{
						if(entry.second.CurveName.empty())
							rootSceneObject = entry.second.Object;
					}
				}

				while(rootSceneObject && rootSceneObject.IsDestroyed(true))
					rootSceneObject = rootSceneObject->GetParent();

				Vector3 parentPos = Vector3::kZero;
				Quaternion parentRot = Quaternion::kIdentity;
				Vector3 parentScale = Vector3::kOne;

				if(!rootSceneObject.IsDestroyed(true))
				{
					const Transform& tfrm = rootSceneObject->GetTransform();
					parentPos = tfrm.GetPosition();
					parentRot = tfrm.GetRotation();
					parentScale = tfrm.GetScale();
				}

				// Transform from space relative to root's parent to world space
				rotation = parentRot * rotation;

				scale = parentScale * scale;

				position = parentRot.Rotate(parentScale * position);
				position += parentPos;

				sceneObject->SetWorldPosition(position);
				sceneObject->SetWorldRotation(rotation);
				sceneObject->SetWorldScale(scale);
			}
		}
		else
		{
			if(!mAnimationProxy->SceneObjectPose.HasOverride[sceneObjectIndex * 3 + 0])
				sceneObject->SetPosition(mAnimationProxy->SceneObjectPose.Positions[sceneObjectIndex]);

			if(!mAnimationProxy->SceneObjectPose.HasOverride[sceneObjectIndex * 3 + 1])
				sceneObject->SetRotation(mAnimationProxy->SceneObjectPose.Rotations[sceneObjectIndex]);

			if(!mAnimationProxy->SceneObjectPose.HasOverride[sceneObjectIndex * 3 + 2])
				sceneObject->SetScale(mAnimationProxy->SceneObjectPose.Scales[sceneObjectIndex]);
		}
	}

	// Must ensure that clip in the proxy and current primary clip are the same
	mGenericCurveValuesValid = false;
	if(mAnimationProxy->LayerCount > 0 && mAnimationProxy->Layers[0].StateCount > 0)
	{
		const AnimationState& state = mAnimationProxy->Layers[0].States[0];

		if(!state.Disabled && !mClipInfos.empty())
		{
			const AnimationClipInfo& clipInfo = mClipInfos[0];

			if(clipInfo.StateIndex == 0 && clipInfo.LayerIndex == 0)
			{
				if(clipInfo.Clip.IsLoaded() && clipInfo.CurveVersion == clipInfo.Clip->GetVersion())
				{
					u32 genericCurveCount = (u32)clipInfo.Clip->GetCurves()->Generic.size();
					mGenericCurveValuesValid = genericCurveCount == mAnimationProxy->GenericCurveCount;
				}
			}
		}
	}

	if(mGenericCurveValuesValid)
	{
		mGenericCurveOutputs.resize(mAnimationProxy->GenericCurveCount);

		memcpy(mGenericCurveOutputs.data(), mAnimationProxy->GenericCurveOutputs, mAnimationProxy->GenericCurveCount * sizeof(float));
	}
}

AnimationClipInfo* Animation::GetOrCreateClipInfo(const HAnimationClip& clip, u32 layer, bool stopExisting)
{
	AnimationClipInfo* output = nullptr;
	bool hasExisting = false;

	// Search for existing
	for(auto& clipInfo : mClipInfos)
	{
		if(clipInfo.State.Layer == layer)
		{
			if(clipInfo.Clip == clip)
				output = &clipInfo;
			else if(stopExisting)
				hasExisting = true;
		}
	}

	// Doesn't exist or found extra animations, rebuild
	if(output == nullptr || hasExisting)
	{
		B3DMarkAllocatorFrame();
		{
			FrameVector<AnimationClipInfo> newClips;
			for(auto& clipInfo : mClipInfos)
			{
				if(!stopExisting || clipInfo.State.Layer != layer || clipInfo.Clip == clip)
					newClips.push_back(clipInfo);
			}

			if(output == nullptr && clip != nullptr)
				newClips.push_back(AnimationClipInfo());

			mClipInfos.resize(newClips.size());
			for(u32 clipIndex = 0; clipIndex < (u32)newClips.size(); clipIndex++)
				mClipInfos[clipIndex] = newClips[clipIndex];

			mDirty |= AnimationDirtyStateFlag::Layout;
		}
		B3DClearAllocatorFrame();
	}

	// If new clip was added, get its address
	if(output == nullptr && clip != nullptr)
	{
		AnimationClipInfo& newInfo = mClipInfos.back();
		newInfo.Clip = clip;
		newInfo.State.Layer = layer;

		output = &newInfo;
	}

	return output;
}

void Animation::OnDestroyed()
{
	DestroyAnimationProxy();
}

void Animation::OnDisabled()
{
	DestroyAnimationProxy();
}

void Animation::OnEnabled()
{
	if(mPreviewMode)
	{
		DestroyAnimationProxy();
		mPreviewMode = false;
	}

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	if(scene->IsRunning())
		CreateAnimationProxy(false);
}

void Animation::Update()
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const bool isRunning = scene->IsRunning();

	if(!isRunning && !mPreviewMode)
	{
		// Make sure attached CBone components match the position of the skeleton bones even when the component is not
		// otherwise running.

		HRenderable animatedRenderable = SO()->GetComponent<Renderable>();
		if(animatedRenderable)
		{
			HMesh mesh = animatedRenderable->GetMesh();
			if(mesh.IsLoaded())
			{
				const SPtr<Skeleton>& skeleton = mesh->GetSkeleton();
				if(skeleton)
				{
					for(auto& entry : mMappedSceneObjectInfos)
					{
						// We allow a null bone for the root bone mapping, should be non-null for everything else
						if(!entry.IsMappedToBone || entry.Bone == nullptr)
							continue;

						const u32 boneCount = skeleton->GetBoneCount();
						for(u32 boneIndex = 0; boneIndex < boneCount; boneIndex++)
						{
							if(skeleton->GetBoneInfo(boneIndex).Name == entry.Bone->GetBoneName())
							{
								Matrix4 bindPose = skeleton->GetInvBindPose(boneIndex).InverseAffine();
								bindPose = SO()->GetTransform().GetMatrix() * bindPose;

								Vector3 position, scale;
								Quaternion rotation;
								bindPose.Decomposition(position, rotation, scale);

								entry.SceneObject->SetWorldPosition(position);
								entry.SceneObject->SetWorldRotation(rotation);
								entry.SceneObject->SetWorldScale(scale);

								break;
							}
						}
					}
				}
			}
		}
	}

	if(!IsAnimationProxyValid() || !isRunning)
		return;

	HAnimationClip newPrimaryClip = !mClipInfos.empty() ? mClipInfos[0].Clip : nullptr;
	if(newPrimaryClip != mPrimaryPlayingClip)
		RefreshClipMappingsInternal();

	if(ScriptUpdateFloatPropertiesInternal)
		ScriptUpdateFloatPropertiesInternal();
}

void Animation::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & (TCF_Transform)) != 0)
		UpdateBounds(false);
}

void Animation::CreateAnimationProxy(bool previewMode)
{
	if(IsAnimationProxyValid())
		DestroyAnimationProxy();

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const SPtr<AnimationScene>& animationScene = scene->GetAnimationScene();

	mAnimationId = animationScene->RegisterAnimation(this);
	mAnimationProxy = B3DMakeShared<AnimationProxy>(mAnimationId);
	mDirty = AnimationDirtyStateFlag::All;

	mAnimatedRenderable = SO()->GetComponent<Renderable>();

	UpdateBounds();

	if(!previewMode)
	{
		if(mDefaultClip.IsLoaded())
			Play(mDefaultClip);

		mPrimaryPlayingClip = !mClipInfos.empty() ? mClipInfos[0].Clip : nullptr;
		if(mPrimaryPlayingClip.IsLoaded())
		{
			if(ScriptRebuildFloatPropertiesInternal)
				ScriptRebuildFloatPropertiesInternal(mPrimaryPlayingClip);
		}
	}

	RebuildBoneMappings();

	if(!previewMode)
		RebuildGenericMappings();

	if(mAnimatedRenderable != nullptr)
		mAnimatedRenderable->RegisterAnimation(B3DStaticGameObjectCast<Animation>(mThisHandle));
}

void Animation::DestroyAnimationProxy()
{
	if(mAnimatedRenderable != nullptr)
		mAnimatedRenderable->UnregisterAnimation();

	mPrimaryPlayingClip = nullptr;

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	const SPtr<AnimationScene>& animationScene = scene->GetAnimationScene();

	animationScene->UnregisterAnimation(mAnimationId);

	mAnimationId = ~0ull;
	mAnimationProxy = nullptr;
	mClipInfos.clear();
	mMorphChannelWeights.clear();
}

bool Animation::TogglePreviewModeInternal(bool enabled)
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	bool isRunning = scene->IsRunning();

	if(enabled)
	{
		// Cannot enable preview while running
		if(isRunning)
			return false;

		if(!mPreviewMode)
		{
			// Make sure not to re-enable preview mode if already enabled because it rebuilds the internal Animation
			// component, changing its ID. If animation evaluation is async then the new ID will not have any animation
			// attached for one frame. This can look weird when sampling the animation for preview purposes
			// (e.g. scrubbing in editor), in which case animation will reset to T pose for a single frame before
			// settling on the chosen frame.
			CreateAnimationProxy(true);
			mPreviewMode = true;
		}

		return true;
	}
	else
	{
		if(!isRunning)
			DestroyAnimationProxy();

		mPreviewMode = false;
		return false;
	}
}

bool Animation::GetGenericCurveValueInternal(u32 curveIndex, float& outValue)
{
	if(!IsAnimationProxyValid() || !mGenericCurveValuesValid || curveIndex >= (u32)mGenericCurveOutputs.size())
		return false;

	outValue = mGenericCurveOutputs[curveIndex];
	return true;
}

void Animation::AddBone(const HBone& bone)
{
	const HSceneObject& sceneObject = bone->SO();

	SceneObjectMappingInfo mapping;
	mapping.SceneObject = sceneObject;
	mapping.IsMappedToBone = true;
	mapping.Bone = bone;

	mMappedSceneObjectInfos.push_back(mapping);

	SceneObjectMappingCurveInfo curveMapping = { sceneObject, bone->GetBoneName() };
	mMappedSceneObjectsById[sceneObject.GetId()] = curveMapping;

	mDirty |= AnimationDirtyStateFlag::All;
}

void Animation::RemoveBone(const HBone& bone)
{
	for(u32 mappingIndex = 0; mappingIndex < (u32)mMappedSceneObjectInfos.size(); mappingIndex++)
	{
		if(mMappedSceneObjectInfos[mappingIndex].Bone == bone)
		{
			mMappedSceneObjectsById.erase(mMappedSceneObjectInfos[mappingIndex].SceneObject.GetId());
			mMappedSceneObjectInfos.erase(mMappedSceneObjectInfos.begin() + mappingIndex);

			mDirty |= AnimationDirtyStateFlag::All;
			mappingIndex--;
		}
	}
}

void Animation::NotifyBoneNameChanged(const HBone& bone)
{
	if(!IsAnimationProxyValid())
		return;

	for(u32 mappedSceneObjectIndex = 0; mappedSceneObjectIndex < (u32)mMappedSceneObjectInfos.size(); mappedSceneObjectIndex++)
	{
		if(mMappedSceneObjectInfos[mappedSceneObjectIndex].Bone == bone)
		{
			mMappedSceneObjectsById.erase(mMappedSceneObjectInfos[mappedSceneObjectIndex].SceneObject.GetId());

			const UUID& sceneObjectId = mMappedSceneObjectInfos[mappedSceneObjectIndex].SceneObject.GetId();
			mMappedSceneObjectsById[sceneObjectId].CurveName = bone->GetBoneName();

			mDirty |= AnimationDirtyStateFlag::All;
			break;
		}
	}
}

bool Animation::GetAnimatesRoot() const
{
	if(mSkeleton == nullptr)
		return false;

	const u32 rootBoneIndex = mSkeleton->GetRootBoneIndex();
	if(rootBoneIndex == ~0u)
		return false;

	String rootBoneName = mSkeleton->GetBoneInfo(rootBoneIndex).Name;
	for(auto& entry : mClipInfos)
	{
		if(entry.Clip.IsLoaded())
		{
			HAnimationClip clip = entry.Clip;
			if(!clip->HasRootMotion())
			{
				AnimationCurveMapping mapping;
				clip->GetCurveMapping(rootBoneName, mapping);

				if(mapping.Position != ~0u)
					return true;

				if(mapping.Rotation != ~0u)
					return true;

				if(mapping.Scale != ~0u)
					return true;
			}
		}
	}

	return false;
}

void Animation::RegisterRenderable(const HRenderable& renderable)
{
	mAnimatedRenderable = renderable;

	UpdateBounds();
}

void Animation::UnregisterRenderable()
{
	mAnimatedRenderable = nullptr;
}

void Animation::UpdateBounds(bool updateRenderable)
{
	SPtr<Renderable> renderable;
	if(updateRenderable && mAnimatedRenderable != nullptr)
		renderable = mAnimatedRenderable.GetShared();

	if(mUseCustomBounds)
	{
		if(renderable != nullptr)
		{
			renderable->SetUseOverrideBounds(true);
			renderable->SetOverrideBounds(mCustomBounds);
		}

		mCullingBounds = mCustomBounds;
		mCullingBounds.TransformAffine(SO()->GetWorldMatrix());

	}
	else
	{
		if(renderable != nullptr)
			renderable->SetUseOverrideBounds(false);

		AABox bounds;
		if(mAnimatedRenderable != nullptr)
			bounds = mAnimatedRenderable->GetBounds().GetBox();

		mCullingBounds = bounds;
	}

	mDirty |= AnimationDirtyStateFlag::Culling;
}

void Animation::RebuildBoneMappings()
{
	mMappedSceneObjectInfos.clear();
	mMappedSceneObjectsById.clear();

	SceneObjectMappingInfo rootMapping;
	rootMapping.SceneObject = SO();
	rootMapping.IsMappedToBone = true;

	mMappedSceneObjectInfos.push_back(rootMapping);

	SceneObjectMappingCurveInfo animatedSceneObject = { rootMapping.SceneObject, "" };
	mMappedSceneObjectsById[rootMapping.SceneObject.GetId()] = animatedSceneObject;

	Vector<HBone> childBones = FindChildBones();
	for(auto& entry : childBones)
		AddBone(entry);

	mDirty |= AnimationDirtyStateFlag::All;
}

void Animation::RebuildGenericMappings()
{
	Vector<SceneObjectMappingInfo> newMappingInfos;
	for(auto& entry : mMappedSceneObjectInfos)
	{
		if(entry.IsMappedToBone)
			newMappingInfos.push_back(entry);
		else
		{
			mMappedSceneObjectsById.erase(entry.SceneObject.GetId());
			mDirty |= AnimationDirtyStateFlag::All;
		}
	}

	if(mPrimaryPlayingClip.IsLoaded())
	{
		HSceneObject rootSceneObject = SO();

		const auto& fnFindSceneObjectMapping = [&](const String& name, AnimationCurveFlags flags)
		{
			if(flags.IsSet(AnimationCurveFlag::ImportedCurve))
				return;

			HSceneObject currentSceneObject = rootSceneObject->FindPath(name);

			bool found = false;
			for(u32 mappingInfoIndex = 0; mappingInfoIndex < (u32)newMappingInfos.size(); mappingInfoIndex++)
			{
				if(newMappingInfos[mappingInfoIndex].SceneObject == currentSceneObject)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				SceneObjectMappingInfo mapping;
				mapping.IsMappedToBone = false;
				mapping.SceneObject = currentSceneObject;

				newMappingInfos.push_back(mapping);

				SceneObjectMappingCurveInfo curveMapping = { currentSceneObject, name };
				mMappedSceneObjectsById[currentSceneObject.GetId()] = curveMapping;

				mDirty |= AnimationDirtyStateFlag::All;
			}
		};

		SPtr<AnimationCurves> curves = mPrimaryPlayingClip->GetCurves();
		for(auto& curve : curves->Position)
			fnFindSceneObjectMapping(curve.Name, curve.Flags);

		for(auto& curve : curves->Rotation)
			fnFindSceneObjectMapping(curve.Name, curve.Flags);

		for(auto& curve : curves->Scale)
			fnFindSceneObjectMapping(curve.Name, curve.Flags);
	}

	mMappedSceneObjectInfos = newMappingInfos;
}

void Animation::RefreshClipMappingsInternal()
{
	mPrimaryPlayingClip = !mClipInfos.empty() ? mClipInfos[0].Clip : nullptr;

	if(ScriptRebuildFloatPropertiesInternal)
		ScriptRebuildFloatPropertiesInternal(mPrimaryPlayingClip);

	RebuildGenericMappings();
}

Vector<HBone> Animation::FindChildBones()
{
	Stack<HSceneObject> todo;
	todo.push(SO());

	Vector<HBone> bones;
	while(!todo.empty())
	{
		HSceneObject currentSceneObject = todo.top();
		todo.pop();

		HBone bone = currentSceneObject->GetComponent<Bone>();
		if(bone != nullptr)
		{
			bone->SetParentAnimation(B3DStaticGameObjectCast<Animation>(GetHandle()), true);
			bones.push_back(bone);
		}

		int childCount = currentSceneObject->GetChildCount();
		for(int childIndex = 0; childIndex < childCount; childIndex++)
		{
			HSceneObject child = currentSceneObject->GetChild(childIndex);
			if(child->GetComponent<Animation>() != nullptr)
				continue;

			todo.push(child);
		}
	}

	return bones;
}

void Animation::NotifyAnimationEventTriggered(const HAnimationClip& clip, const String& name)
{
	OnEventTriggered(clip, name);

	if(ScriptOnEventTriggeredInternal)
		ScriptOnEventTriggeredInternal(clip, name);
}

void Animation::GetListenerResources(Vector<HResource>& resources)
{
	for(auto& entry : mClipInfos)
	{
		if(entry.Clip != nullptr)
			resources.push_back(entry.Clip);
	}
}

void Animation::NotifyResourceLoaded(const HResource& resource)
{
	mDirty |= AnimationDirtyStateFlag::Layout;
}

void Animation::NotifyResourceChanged(const HResource& resource)
{
	mDirty |= AnimationDirtyStateFlag::Layout;
}

RTTIType* Animation::GetRttiStatic()
{
	return AnimationRTTI::Instance();
}

RTTIType* Animation::GetRtti() const
{
	return Animation::GetRttiStatic();
}
