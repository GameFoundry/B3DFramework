//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCAnimation.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRenderable.h"
#include "Components/BsCBone.h"
#include "Mesh/BsMesh.h"
#include "Animation/BsMorphShapes.h"
#include "Animation/BsAnimationClip.h"
#include "Private/RTTI/BsCAnimationRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace std::placeholders;

using namespace bs;

CAnimation::CAnimation()
{
	mNotifyFlags = TCF_Transform;
	SetFlag(ComponentFlag::AlwaysRun, true);

	SetName("Animation");
}

CAnimation::CAnimation(const HSceneObject& parent)
	: Component(parent)
{
	mNotifyFlags = TCF_Transform;
	SetFlag(ComponentFlag::AlwaysRun, true);

	SetName("Animation");
}

void CAnimation::SetDefaultClip(const HAnimationClip& clip)
{
	mDefaultClip = clip;

	if(clip.IsLoaded() && mInternal != nullptr && !mPreviewMode)
		mInternal->Play(clip);
}

void CAnimation::SetWrapMode(AnimWrapMode wrapMode)
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

void CAnimation::Blend1D(const Blend1DInfo& info, float t)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->Blend1D(info, t);
}

void CAnimation::Blend2D(const Blend2DInfo& info, const Vector2& t)
{
	if(mInternal != nullptr && !mPreviewMode)
		mInternal->Blend2D(info, t);
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
	for(u32 i = 0; i < (u32)channels.size(); i++)
	{
		if(channels[i]->GetName() == name)
		{
			mInternal->SetMorphChannelWeight(i, weight);
			break;
		}
	}
}

void CAnimation::SetBounds(const AABox& bounds)
{
	mBounds = bounds;

	if(mUseBounds)
	{
		if(mAnimatedRenderable != nullptr)
		{
			SPtr<Renderable> renderable = mAnimatedRenderable->GetInternalInternal();
			if(renderable != nullptr)
				renderable->SetOverrideBounds(bounds);

			if(mInternal != nullptr && !mPreviewMode)
			{
				AABox bounds = mBounds;

				bounds.TransformAffine(SO()->GetWorldMatrix());
				mInternal->SetBounds(bounds);
			}
		}
	}
}

void CAnimation::SetUseBounds(bool enable)
{
	mUseBounds = enable;

	UpdateBoundsInternal();
}

void CAnimation::SetEnableCull(bool enable)
{
	mEnableCull = enable;

	if(mInternal != nullptr && !mPreviewMode)
		mInternal->SetCulling(enable);
}

u32 CAnimation::GetNumClips() const
{
	if(mInternal != nullptr)
		return mInternal->GetNumClips();

	return 0;
}

HAnimationClip CAnimation::GetClip(u32 idx) const
{
	if(mInternal != nullptr)
		return mInternal->GetClip(idx);

	return HAnimationClip();
}

void CAnimation::OnInitialized()
{
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

	if(SceneManager::Instance().IsRunning())
		RestoreInternal(false);
}

void CAnimation::Update()
{
	const bool isRunning = SceneManager::Instance().IsRunning();
	if(!isRunning && !mPreviewMode)
	{
		// Make sure attached CBone components match the position of the skeleton bones even when the component is not
		// otherwise running.

		HRenderable animatedRenderable = SO()->GetComponent<CRenderable>();
		if(animatedRenderable)
		{
			HMesh mesh = animatedRenderable->GetMesh();
			if(mesh.IsLoaded())
			{
				const SPtr<Skeleton>& skeleton = mesh->GetSkeleton();
				if(skeleton)
				{
					for(auto& entry : mMappingInfos)
					{
						// We allow a null bone for the root bone mapping, should be non-null for everything else
						if(!entry.IsMappedToBone || entry.Bone == nullptr)
							continue;

						const u32 numBones = skeleton->GetNumBones();
						for(u32 j = 0; j < numBones; j++)
						{
							if(skeleton->GetBoneInfo(j).Name == entry.Bone->GetBoneName())
							{
								Matrix4 bindPose = skeleton->GetInvBindPose(j).InverseAffine();
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
	if(!SO()->GetActive())
		return;

	if((flags & (TCF_Transform)) != 0)
		UpdateBoundsInternal(false);
}

void CAnimation::RestoreInternal(bool previewMode)
{
	if(mInternal != nullptr)
		DestroyInternal();

	mInternal = Animation::Create();

	mAnimatedRenderable = SO()->GetComponent<CRenderable>();

	if(!previewMode)
	{
		mInternal->OnEventTriggered.Connect(std::bind(&CAnimation::EventTriggered, this, _1, _2));

		mInternal->SetWrapMode(mWrapMode);
		mInternal->SetSpeed(mSpeed);
		mInternal->SetCulling(mEnableCull);
	}

	UpdateBoundsInternal();

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
		mAnimatedRenderable->RegisterAnimationInternal(static_object_cast<CAnimation>(mThisHandle));
}

void CAnimation::DestroyInternal()
{
	if(mAnimatedRenderable != nullptr)
		mAnimatedRenderable->UnregisterAnimationInternal();

	mPrimaryPlayingClip = nullptr;

	// This should release the last reference and destroy the internal listener
	mInternal = nullptr;
}

bool CAnimation::TogglePreviewModeInternal(bool enabled)
{
	bool isRunning = SceneManager::Instance().IsRunning();

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

bool CAnimation::GetGenericCurveValueInternal(u32 curveIdx, float& value)
{
	if(mInternal == nullptr)
		return false;

	return mInternal->GetGenericCurveValue(curveIdx, value);
}

void CAnimation::MapCurveToSceneObject(const String& curve, const HSceneObject& so)
{
	if(mInternal == nullptr)
		return;

	mInternal->MapCurveToSceneObject(curve, so);
}

void CAnimation::UnmapSceneObject(const HSceneObject& so)
{
	if(mInternal == nullptr)
		return;

	mInternal->UnmapSceneObject(so);
}

void CAnimation::AddBoneInternal(HBone bone)
{
	const HSceneObject& currentSO = bone->SO();

	SceneObjectMappingInfo newMapping;
	newMapping.SceneObject = currentSO;
	newMapping.IsMappedToBone = true;
	newMapping.Bone = std::move(bone);

	mMappingInfos.push_back(newMapping);

	if(mInternal)
		mInternal->MapCurveToSceneObject(newMapping.Bone->GetBoneName(), newMapping.SceneObject);
}

void CAnimation::RemoveBoneInternal(const HBone& bone)
{
	HSceneObject newSO;
	for(u32 i = 0; i < (u32)mMappingInfos.size(); i++)
	{
		if(mMappingInfos[i].Bone == bone)
		{
			if(mInternal)
				mInternal->UnmapSceneObject(mMappingInfos[i].SceneObject);

			mMappingInfos.erase(mMappingInfos.begin() + i);
			i--;
		}
	}
}

void CAnimation::NotifyBoneChangedInternal(const HBone& bone)
{
	if(mInternal == nullptr)
		return;

	for(u32 i = 0; i < (u32)mMappingInfos.size(); i++)
	{
		if(mMappingInfos[i].Bone == bone)
		{
			mInternal->UnmapSceneObject(mMappingInfos[i].SceneObject);
			mInternal->MapCurveToSceneObject(bone->GetBoneName(), mMappingInfos[i].SceneObject);
			break;
		}
	}
}

void CAnimation::RegisterRenderableInternal(const HRenderable& renderable)
{
	mAnimatedRenderable = renderable;

	UpdateBoundsInternal();
}

void CAnimation::UnregisterRenderableInternal()
{
	mAnimatedRenderable = nullptr;
}

void CAnimation::UpdateBoundsInternal(bool updateRenderable)
{
	SPtr<Renderable> renderable;
	if(updateRenderable && mAnimatedRenderable != nullptr)
		renderable = mAnimatedRenderable->GetInternalInternal();

	if(mUseBounds)
	{
		if(renderable != nullptr)
		{
			renderable->SetUseOverrideBounds(true);
			renderable->SetOverrideBounds(mBounds);
		}

		if(mInternal != nullptr)
		{
			AABox bounds = mBounds;
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
	mMappingInfos.clear();

	SceneObjectMappingInfo rootMapping;
	rootMapping.SceneObject = SO();
	rootMapping.IsMappedToBone = true;

	mMappingInfos.push_back(rootMapping);
	mInternal->MapCurveToSceneObject("", rootMapping.SceneObject);

	Vector<HBone> childBones = FindChildBones();
	for(auto& entry : childBones)
		AddBoneInternal(entry);
}

void CAnimation::UpdateSceneObjectMapping()
{
	Vector<SceneObjectMappingInfo> newMappingInfos;
	for(auto& entry : mMappingInfos)
	{
		if(entry.IsMappedToBone)
			newMappingInfos.push_back(entry);
		else
			UnmapSceneObject(entry.SceneObject);
	}

	if(mPrimaryPlayingClip.IsLoaded())
	{
		HSceneObject root = SO();

		const auto& findMappings = [&](const String& name, AnimationCurveFlags flags)
		{
			if(flags.IsSet(AnimationCurveFlag::ImportedCurve))
				return;

			HSceneObject currentSO = root->FindPath(name);

			bool found = false;
			for(u32 i = 0; i < (u32)newMappingInfos.size(); i++)
			{
				if(newMappingInfos[i].SceneObject == currentSO)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				SceneObjectMappingInfo newMappingInfo;
				newMappingInfo.IsMappedToBone = false;
				newMappingInfo.SceneObject = currentSO;

				newMappingInfos.push_back(newMappingInfo);
				MapCurveToSceneObject(name, currentSO);
			}
		};

		SPtr<AnimationCurves> curves = mPrimaryPlayingClip->GetCurves();
		for(auto& curve : curves->Position)
			findMappings(curve.Name, curve.Flags);

		for(auto& curve : curves->Rotation)
			findMappings(curve.Name, curve.Flags);

		for(auto& curve : curves->Scale)
			findMappings(curve.Name, curve.Flags);
	}

	mMappingInfos = newMappingInfos;
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
	while(todo.size() > 0)
	{
		HSceneObject currentSO = todo.top();
		todo.pop();

		HBone bone = currentSO->GetComponent<CBone>();
		if(bone != nullptr)
		{
			bone->SetParentInternal(static_object_cast<CAnimation>(GetHandle()), true);
			bones.push_back(bone);
		}

		int childCount = currentSO->GetNumChildren();
		for(int i = 0; i < childCount; i++)
		{
			HSceneObject child = currentSO->GetChild(i);
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

RTTITypeBase* CAnimation::GetRttiStatic()
{
	return CAnimationRTTI::Instance();
}

RTTITypeBase* CAnimation::GetRtti() const
{
	return CAnimation::GetRttiStatic();
}
