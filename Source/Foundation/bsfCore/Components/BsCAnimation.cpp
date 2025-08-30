//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCAnimation.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsRenderable.h"
#include "Components/BsCBone.h"
#include "Mesh/BsMesh.h"
#include "Animation/BsMorphShapes.h"
#include "Animation/BsAnimationClip.h"
#include "Private/RTTI/BsCAnimationRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CAnimation::CAnimation(const HSceneObject& parent)
	: Component(parent)
{
	mNotifyFlags = TCF_Transform;
	SetFlag(ComponentFlag::AlwaysRun, true);

	SetName("Animation");
}

CAnimation::CAnimation()
	: CAnimation(nullptr)
{ }

void CAnimation::SetDefaultClip(const HAnimationClip& clip)
{
	mDefaultClip = clip;

	if(clip.IsLoaded() && mInternal != nullptr && !mPreviewMode)
		mInternal->Play(clip);
}

void CAnimation::SetWrapMode(AnimationWrapMode wrapMode)
{
	mWrapMode = wrapMode;

	if(mInternal != nullptr && !mPreviewMode)
		mInternal->SetWrapMode(wrapMode);
}

void CAnimation::SetSpeed(float speed)
{
	mSpeed = speed;

	if(mInternal != nullptr && !mPreviewMode)
		mInternal->SetSpeed(speed);
}

void CAnimation::Play(const HAnimationClip& clip)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->Play(clip);
}

void CAnimation::BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength, u32 layer)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->BlendAdditive(clip, weight, fadeLength, layer);
}

void CAnimation::Blend1D(const Blend1DInfo& info, float alpha)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->Blend1D(info, alpha);
}

void CAnimation::Blend2D(const Blend2DInfo& info, const Vector2& alpha)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->Blend2D(info, alpha);
}

void CAnimation::CrossFade(const HAnimationClip& clip, float fadeLength)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->CrossFade(clip, fadeLength);
}

void CAnimation::Sample(const HAnimationClip& clip, float time)
{
	if(mInternal != nullptr)
		mInternal->Sample(clip, time);
}

void CAnimation::Stop(u32 layer)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->Stop(layer);
}

void CAnimation::StopAll()
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->StopAll();
}

bool CAnimation::IsPlaying() const
{
	if(mInternal != nullptr)
		return mInternal->IsPlaying();

	return false;
}

bool CAnimation::GetState(const HAnimationClip& clip, AnimationClipState& state)
{
	if(mInternal != nullptr)
		return mInternal->GetState(clip, state);

	return false;
}

void CAnimation::SetState(const HAnimationClip& clip, AnimationClipState state)
{
	if(mInternal != nullptr)
		return mInternal->SetState(clip, state);
}

void CAnimation::SetMorphChannelWeight(const String& name, float weight)
{
	if(mInternal == nullptr)
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
			mInternal->SetMorphChannelWeight(morphChannelIndex, weight);
			break;
		}
	}
}

void CAnimation::SetCustomBounds(const AABox& bounds)
{
	mCustomBounds = bounds;

	if(mUseCustomBounds)
	{
		if(mAnimatedRenderable != nullptr)
		{
			SPtr<Renderable> renderable = mAnimatedRenderable.GetShared();
			if(renderable != nullptr)
				renderable->SetOverrideBounds(bounds);

			if(mInternal != nullptr && !mPreviewMode)
			{
				AABox bounds = mCustomBounds;

				bounds.TransformAffine(SO()->GetWorldMatrix());
				mInternal->SetBounds(bounds);
			}
		}
	}
}

void CAnimation::SetUseCustomBounds(bool enable)
{
	mUseCustomBounds = enable;

	UpdateBounds();
}

void CAnimation::SetEnableCull(bool enable)
{
	mEnableCull = enable;

	if(mInternal != nullptr && !mPreviewMode)
		mInternal->SetCulling(enable);
}

u32 CAnimation::GetClipCount() const
{
	if(mInternal != nullptr)
		return mInternal->GetClipCount();

	return 0;
}

HAnimationClip CAnimation::GetClip(u32 index) const
{
	if(mInternal != nullptr)
		return mInternal->GetClip(index);

	return HAnimationClip();
}

void CAnimation::OnDestroyed()
{
	DestroyInternal();
}

void CAnimation::OnDisabled()
{
	DestroyInternal();
}

void CAnimation::OnEnabled()
{
	if(mPreviewMode)
	{
		DestroyInternal();
		mPreviewMode = false;
	}

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	if(scene->IsRunning())
		RestoreInternal(false);
}

void CAnimation::Update()
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

	if(mInternal == nullptr || !isRunning)
		return;

	HAnimationClip newPrimaryClip = mInternal->GetClip(0);
	if(newPrimaryClip != mPrimaryPlayingClip)
		RefreshClipMappingsInternal();

	if(ScriptUpdateFloatPropertiesInternal)
		ScriptUpdateFloatPropertiesInternal();
}

void CAnimation::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & (TCF_Transform)) != 0)
		UpdateBounds(false);
}

void CAnimation::RestoreInternal(bool previewMode)
{
	if(mInternal != nullptr)
		DestroyInternal();

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	mInternal = Animation::Create(scene);

	mAnimatedRenderable = SO()->GetComponent<Renderable>();

	if(!previewMode)
	{
		mInternal->OnEventTriggered.Connect([this](const HAnimationClip& clip, const String& name) { EventTriggered(clip, name); });

		mInternal->SetWrapMode(mWrapMode);
		mInternal->SetSpeed(mSpeed);
		mInternal->SetCulling(mEnableCull);
	}

	UpdateBounds();

	if(!previewMode)
	{
		if(mDefaultClip.IsLoaded())
			mInternal->Play(mDefaultClip);

		mPrimaryPlayingClip = mInternal->GetClip(0);
		if(mPrimaryPlayingClip.IsLoaded())
		{
			if(ScriptRebuildFloatPropertiesInternal)
				ScriptRebuildFloatPropertiesInternal(mPrimaryPlayingClip);
		}
	}

	SetBoneMappings();

	if(!previewMode)
		UpdateSceneObjectMapping();

	if(mAnimatedRenderable != nullptr)
		mAnimatedRenderable->RegisterAnimation(B3DStaticGameObjectCast<CAnimation>(mThisHandle));
}

void CAnimation::DestroyInternal()
{
	if(mAnimatedRenderable != nullptr)
		mAnimatedRenderable->UnregisterAnimation();

	mPrimaryPlayingClip = nullptr;

	// This should release the last reference and destroy the internal listener
	mInternal = nullptr;
}

bool CAnimation::TogglePreviewModeInternal(bool enabled)
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
			RestoreInternal(true);
			mPreviewMode = true;
		}

		return true;
	}
	else
	{
		if(!isRunning)
			DestroyInternal();

		mPreviewMode = false;
		return false;
	}
}

bool CAnimation::GetGenericCurveValueInternal(u32 index, float& outValue)
{
	if(mInternal == nullptr)
		return false;

	return mInternal->GetGenericCurveValue(index, outValue);
}

void CAnimation::MapCurveToSceneObject(const String& curve, const HSceneObject& sceneObject)
{
	if(mInternal == nullptr)
		return;

	mInternal->MapCurveToSceneObject(curve, sceneObject);
}

void CAnimation::UnmapSceneObject(const HSceneObject& sceneObject)
{
	if(mInternal == nullptr)
		return;

	mInternal->UnmapSceneObject(sceneObject);
}

void CAnimation::AddBone(const HBone& bone)
{
	const HSceneObject& currentSO = bone->SO();

	SceneObjectMappingInfo newMapping;
	newMapping.SceneObject = currentSO;
	newMapping.IsMappedToBone = true;
	newMapping.Bone = bone;

	mMappedSceneObjectInfos.push_back(newMapping);

	if(mInternal)
		mInternal->MapCurveToSceneObject(newMapping.Bone->GetBoneName(), newMapping.SceneObject);
}

void CAnimation::RemoveBone(const HBone& bone)
{
	for(u32 mappingIndex = 0; mappingIndex < (u32)mMappedSceneObjectInfos.size(); mappingIndex++)
	{
		if(mMappedSceneObjectInfos[mappingIndex].Bone == bone)
		{
			if(mInternal)
				mInternal->UnmapSceneObject(mMappedSceneObjectInfos[mappingIndex].SceneObject);

			mMappedSceneObjectInfos.erase(mMappedSceneObjectInfos.begin() + mappingIndex);
			mappingIndex--;
		}
	}
}

void CAnimation::NotifyBoneChanged(const HBone& bone)
{
	if(mInternal == nullptr)
		return;

	for(u32 i = 0; i < (u32)mMappedSceneObjectInfos.size(); i++)
	{
		if(mMappedSceneObjectInfos[i].Bone == bone)
		{
			mInternal->UnmapSceneObject(mMappedSceneObjectInfos[i].SceneObject);
			mInternal->MapCurveToSceneObject(bone->GetBoneName(), mMappedSceneObjectInfos[i].SceneObject);
			break;
		}
	}
}

void CAnimation::RegisterRenderable(const HRenderable& renderable)
{
	mAnimatedRenderable = renderable;

	UpdateBounds();
}

void CAnimation::UnregisterRenderable()
{
	mAnimatedRenderable = nullptr;
}

void CAnimation::UpdateBounds(bool updateRenderable)
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

		if(mInternal != nullptr)
		{
			AABox bounds = mCustomBounds;
			bounds.TransformAffine(SO()->GetWorldMatrix());

			mInternal->SetBounds(bounds);
		}
	}
	else
	{
		if(renderable != nullptr)
			renderable->SetUseOverrideBounds(false);

		if(mInternal != nullptr)
		{
			AABox bounds;
			if(mAnimatedRenderable != nullptr)
				bounds = mAnimatedRenderable->GetBounds().GetBox();

			mInternal->SetBounds(bounds);
		}
	}
}

void CAnimation::SetBoneMappings()
{
	mMappedSceneObjectInfos.clear();

	SceneObjectMappingInfo rootMapping;
	rootMapping.SceneObject = SO();
	rootMapping.IsMappedToBone = true;

	mMappedSceneObjectInfos.push_back(rootMapping);
	mInternal->MapCurveToSceneObject("", rootMapping.SceneObject);

	Vector<HBone> childBones = FindChildBones();
	for(auto& entry : childBones)
		AddBone(entry);
}

void CAnimation::UpdateSceneObjectMapping()
{
	Vector<SceneObjectMappingInfo> newMappingInfos;
	for(auto& entry : mMappedSceneObjectInfos)
	{
		if(entry.IsMappedToBone)
			newMappingInfos.push_back(entry);
		else
			UnmapSceneObject(entry.SceneObject);
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
			for(u32 i = 0; i < (u32)newMappingInfos.size(); i++)
			{
				if(newMappingInfos[i].SceneObject == currentSceneObject)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				SceneObjectMappingInfo newMappingInfo;
				newMappingInfo.IsMappedToBone = false;
				newMappingInfo.SceneObject = currentSceneObject;

				newMappingInfos.push_back(newMappingInfo);
				MapCurveToSceneObject(name, currentSceneObject);
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

void CAnimation::RefreshClipMappingsInternal()
{
	mPrimaryPlayingClip = mInternal->GetClip(0);

	if(ScriptRebuildFloatPropertiesInternal)
		ScriptRebuildFloatPropertiesInternal(mPrimaryPlayingClip);

	UpdateSceneObjectMapping();
}

Vector<HBone> CAnimation::FindChildBones()
{
	Stack<HSceneObject> todo;
	todo.push(SO());

	Vector<HBone> bones;
	while(!todo.empty())
	{
		HSceneObject currentSceneObject = todo.top();
		todo.pop();

		HBone bone = currentSceneObject->GetComponent<CBone>();
		if(bone != nullptr)
		{
			bone->SetParentInternal(B3DStaticGameObjectCast<CAnimation>(GetHandle()), true);
			bones.push_back(bone);
		}

		int childCount = currentSceneObject->GetChildCount();
		for(int i = 0; i < childCount; i++)
		{
			HSceneObject child = currentSceneObject->GetChild(i);
			if(child->GetComponent<CAnimation>() != nullptr)
				continue;

			todo.push(child);
		}
	}

	return bones;
}

void CAnimation::EventTriggered(const HAnimationClip& clip, const String& name)
{
	OnEventTriggered(clip, name);

	if(ScriptOnEventTriggeredInternal)
		ScriptOnEventTriggeredInternal(clip, name);
}

RTTIType* CAnimation::GetRttiStatic()
{
	return CAnimationRTTI::Instance();
}

RTTIType* CAnimation::GetRtti() const
{
	return CAnimation::GetRttiStatic();
}
