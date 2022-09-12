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

namespace bs
{
	CAnimation::CAnimation()
	{
		mNotifyFlags = TCF_Transform;
		setFlag(ComponentFlag::AlwaysRun, true);

		setName("Animation");
	}

	CAnimation::CAnimation(const HSceneObject& parent)
		: Component(parent)
	{
		mNotifyFlags = TCF_Transform;
		setFlag(ComponentFlag::AlwaysRun, true);

		setName("Animation");
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

		if (mInternal != nullptr && !mPreviewMode)
			mInternal->SetWrapMode(wrapMode);
	}

	void CAnimation::SetSpeed(float speed)
	{
		mSpeed = speed;

		if (mInternal != nullptr && !mPreviewMode)
			mInternal->SetSpeed(speed);
	}

	void CAnimation::Play(const HAnimationClip& clip)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->Play(clip);
	}

	void CAnimation::BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength, UINT32 layer)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->BlendAdditive(clip, weight, fadeLength, layer);
	}

	void CAnimation::Blend1D(const Blend1DInfo& info, float t)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->Blend1D(info, t);
	}

	void CAnimation::Blend2D(const Blend2DInfo& info, const Vector2& t)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->Blend2D(info, t);
	}

	void CAnimation::CrossFade(const HAnimationClip& clip, float fadeLength)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->CrossFade(clip, fadeLength);
	}

	void CAnimation::Sample(const HAnimationClip& clip, float time)
	{
		if (mInternal != nullptr)
			mInternal->Sample(clip, time);
	}

	void CAnimation::Stop(UINT32 layer)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->Stop(layer);
	}

	void CAnimation::StopAll()
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->StopAll();
	}

	bool CAnimation::IsPlaying() const
	{
		if (mInternal != nullptr)
			return mInternal->IsPlaying();

		return false;
	}

	bool CAnimation::GetState(const HAnimationClip& clip, AnimationClipState& state)
	{
		if (mInternal != nullptr)
			return mInternal->GetState(clip, state);

		return false;
	}

	void CAnimation::SetState(const HAnimationClip& clip, AnimationClipState state)
	{
		if (mInternal != nullptr)
			return mInternal->SetState(clip, state);
	}

	void CAnimation::SetMorphChannelWeight(const String& name, float weight)
	{
		if (mInternal == nullptr)
			return;

		if (mAnimatedRenderable == nullptr)
			return;

		HMesh mesh = mAnimatedRenderable->GetMesh();
		if (!mesh.IsLoaded())
			return;

		SPtr<MorphShapes> morphShapes = mesh->GetMorphShapes();
		if (morphShapes == nullptr)
			return;

		const Vector<SPtr<MorphChannel>>& channels = morphShapes->GetChannels();
		for (UINT32 i = 0; i < (UINT32)channels.size(); i++)
		{
			if (channels[i]->GetName() == name)
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
				SPtr<Renderable> renderable = mAnimatedRenderable->_getInternal();
				if (renderable != nullptr)
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

		_updateBounds();
	}

	void CAnimation::SetEnableCull(bool enable)
	{
		mEnableCull = enable;

		if (mInternal != nullptr && !mPreviewMode)
			mInternal->SetCulling(enable);
	}

	UINT32 CAnimation::GetNumClips() const
	{
		if (mInternal != nullptr)
			return mInternal->GetNumClips();

		return 0;
	}

	HAnimationClip CAnimation::GetClip(UINT32 idx) const
	{
		if (mInternal != nullptr)
			return mInternal->GetClip(idx);

		return HAnimationClip();
	}

	void CAnimation::OnInitialized()
	{
		
	}

	void CAnimation::OnDestroyed()
	{
		destroyInternal();
	}

	void CAnimation::OnDisabled()
	{
		destroyInternal();
	}

	void CAnimation::OnEnabled()
	{
		if(mPreviewMode)
		{
			destroyInternal();
			mPreviewMode = false;
		}
		
		if(SceneManager::instance().IsRunning())
			restoreInternal(false);
	}

	void CAnimation::Update()
	{
		const bool isRunning = SceneManager::instance().IsRunning();
		if(!isRunning && !mPreviewMode)
		{
			// Make sure attached CBone components match the position of the skeleton bones even when the component is not
			// otherwise running.

			HRenderable animatedRenderable = SO()->getComponent<CRenderable>();
			if(animatedRenderable)
			{
				HMesh mesh = animatedRenderable->GetMesh();
				if(mesh.IsLoaded())
				{
					const SPtr<Skeleton>& skeleton = mesh->GetSkeleton();
					if(skeleton)
					{
						for (auto& entry : mMappingInfos)
						{
							// We allow a null bone for the root bone mapping, should be non-null for everything else
							if(!entry.isMappedToBone || entry.bone == nullptr)
								continue;

							const UINT32 numBones = skeleton->GetNumBones();
							for (UINT32 j = 0; j < numBones; j++)
							{
								if (skeleton->GetBoneInfo(j).name == entry.bone->getBoneName())
								{
									Matrix4 bindPose = skeleton->GetInvBindPose(j).InverseAffine();
									bindPose = SO()->GetTransform().GetMatrix() * bindPose;

									Vector3 position, scale;
									Quaternion rotation;
									bindPose.Decomposition(position, rotation, scale);

									entry.sceneObject->SetWorldPosition(position);
									entry.sceneObject->SetWorldRotation(rotation);
									entry.sceneObject->SetWorldScale(scale);

									break;
								}
							}
						}
					}
				}
			}
		}

		if (mInternal == nullptr || !isRunning)
			return;

		HAnimationClip newPrimaryClip = mInternal->GetClip(0);
		if (newPrimaryClip != mPrimaryPlayingClip)
			_refreshClipMappings();

		if (_scriptUpdateFloatProperties)
			_scriptUpdateFloatProperties();
	}

	void CAnimation::OnTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->GetActive())
			return;

		if ((flags & (TCF_Transform)) != 0)
			_updateBounds(false);
	}

	void CAnimation::RestoreInternal(bool previewMode)
	{
		if (mInternal != nullptr)
			destroyInternal();

		mInternal = Animation::create();

		mAnimatedRenderable = SO()->getComponent<CRenderable>();

		if (!previewMode)
		{
			mInternal->onEventTriggered.Connect(std::bind(&CAnimation::eventTriggered, this, _1, _2));

			mInternal->SetWrapMode(mWrapMode);
			mInternal->SetSpeed(mSpeed);
			mInternal->SetCulling(mEnableCull);
		}

		_updateBounds();

		if (!previewMode)
		{
			if (mDefaultClip.IsLoaded())
				mInternal->Play(mDefaultClip);

			mPrimaryPlayingClip = mInternal->GetClip(0);
			if (mPrimaryPlayingClip.IsLoaded())
			{
				if (_scriptRebuildFloatProperties)
					_scriptRebuildFloatProperties(mPrimaryPlayingClip);
			}
		}

		setBoneMappings();

		if(!previewMode)
			updateSceneObjectMapping();

		if (mAnimatedRenderable != nullptr)
			mAnimatedRenderable->_registerAnimation(static_object_cast<CAnimation>(mThisHandle));
	}

	void CAnimation::DestroyInternal()
	{
		if (mAnimatedRenderable != nullptr)
			mAnimatedRenderable->_unregisterAnimation();

		mPrimaryPlayingClip = nullptr;

		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
	}

	bool CAnimation::_togglePreviewMode(bool enabled)
	{
		bool isRunning = SceneManager::instance().IsRunning();

		if(enabled)
		{
			// Cannot enable preview while running
			if (isRunning)
				return false;

			if(!mPreviewMode)
			{
				// Make sure not to re-enable preview mode if already enabled because it rebuilds the internal Animation
				// component, changing its ID. If animation evaluation is async then the new ID will not have any animation
				// attached for one frame. This can look weird when sampling the animation for preview purposes
				// (e.g. scrubbing in editor), in which case animation will reset to T pose for a single frame before
				// settling on the chosen frame.
				restoreInternal(true);
				mPreviewMode = true;
			}

			return true;
		}
		else
		{
			if (!isRunning)
				destroyInternal();

			mPreviewMode = false;
			return false;
		}
	}

	bool CAnimation::_getGenericCurveValue(UINT32 curveIdx, float& value)
	{
		if (mInternal == nullptr)
			return false;

		return mInternal->GetGenericCurveValue(curveIdx, value);
	}

	void CAnimation::MapCurveToSceneObject(const String& curve, const HSceneObject& so)
	{
		if (mInternal == nullptr)
			return;

		mInternal->MapCurveToSceneObject(curve, so);
	}

	void CAnimation::UnmapSceneObject(const HSceneObject& so)
	{
		if (mInternal == nullptr)
			return;

		mInternal->UnmapSceneObject(so);
	}

	void CAnimation::_addBone(HBone bone)
	{
		const HSceneObject& currentSO = bone->SO();

		SceneObjectMappingInfo newMapping;
		newMapping.sceneObject = currentSO;
		newMapping.isMappedToBone = true;
		newMapping.bone = std::move(bone);

		mMappingInfos.push_back(newMapping);

		if(mInternal)
			mInternal->MapCurveToSceneObject(newMapping.bone->getBoneName(), newMapping.sceneObject);
	}

	void CAnimation::_removeBone(const HBone& bone)
	{
		HSceneObject newSO;
		for (UINT32 i = 0; i < (UINT32)mMappingInfos.size(); i++)
		{
			if (mMappingInfos[i].bone == bone)
			{
				if(mInternal)
					mInternal->UnmapSceneObject(mMappingInfos[i].sceneObject);

				mMappingInfos.erase(mMappingInfos.begin() + i);
				i--;
			}
		}
	}

	void CAnimation::_notifyBoneChanged(const HBone& bone)
	{
		if (mInternal == nullptr)
			return;

		for (UINT32 i = 0; i < (UINT32)mMappingInfos.size(); i++)
		{
			if (mMappingInfos[i].bone == bone)
			{
				mInternal->UnmapSceneObject(mMappingInfos[i].sceneObject);
				mInternal->MapCurveToSceneObject(bone->getBoneName(), mMappingInfos[i].sceneObject);
				break;
			}
		}
	}

	void CAnimation::_registerRenderable(const HRenderable& renderable)
	{
		mAnimatedRenderable = renderable;

		_updateBounds();
	}

	void CAnimation::_unregisterRenderable()
	{
		mAnimatedRenderable = nullptr;
	}

	void CAnimation::_updateBounds(bool updateRenderable)
	{
		SPtr<Renderable> renderable;
		if (updateRenderable && mAnimatedRenderable != nullptr)
			renderable = mAnimatedRenderable->_getInternal();

		if (mUseBounds)
		{
			if (renderable != nullptr)
			{
				renderable->SetUseOverrideBounds(true);
				renderable->SetOverrideBounds(mBounds);
			}

			if (mInternal != nullptr)
			{
				AABox bounds = mBounds;
				bounds.TransformAffine(SO()->GetWorldMatrix());

				mInternal->SetBounds(bounds);
			}
		}
		else
		{
			if (renderable != nullptr)
				renderable->SetUseOverrideBounds(false);

			if (mInternal != nullptr)
			{
				AABox bounds;
				if (mAnimatedRenderable != nullptr)
					bounds = mAnimatedRenderable->GetBounds().GetBox();

				mInternal->SetBounds(bounds);
			}
		}
	}

	void CAnimation::SetBoneMappings()
	{
		mMappingInfos.Clear();

		SceneObjectMappingInfo rootMapping;
		rootMapping.sceneObject = SO();
		rootMapping.isMappedToBone = true;

		mMappingInfos.push_back(rootMapping);
		mInternal->MapCurveToSceneObject("", rootMapping.sceneObject);

		Vector<HBone> childBones = findChildBones();
		for (auto& entry : childBones)
			_addBone(entry);
	}

	void CAnimation::UpdateSceneObjectMapping()
	{
		Vector<SceneObjectMappingInfo> newMappingInfos;
		for(auto& entry : mMappingInfos)
		{
			if (entry.isMappedToBone)
				newMappingInfos.push_back(entry);
			else
				unmapSceneObject(entry.sceneObject);
		}

		if (mPrimaryPlayingClip.IsLoaded())
		{
			HSceneObject root = SO();

			const auto& findMappings = [&](const String& name, AnimationCurveFlags flags)
			{
				if (flags.IsSet(AnimationCurveFlag::ImportedCurve))
					return;

				HSceneObject currentSO = root->FindPath(name);

				bool found = false;
				for (UINT32 i = 0; i < (UINT32)newMappingInfos.size(); i++)
				{
					if (newMappingInfos[i].sceneObject == currentSO)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					SceneObjectMappingInfo newMappingInfo;
					newMappingInfo.isMappedToBone = false;
					newMappingInfo.sceneObject = currentSO;

					newMappingInfos.push_back(newMappingInfo);
					mapCurveToSceneObject(name, currentSO);
				}
			};

			SPtr<AnimationCurves> curves = mPrimaryPlayingClip->GetCurves();
			for(auto& curve : curves->position)
				findMappings(curve.name, curve.flags);

			for(auto& curve : curves->rotation)
				findMappings(curve.name, curve.flags);

			for(auto& curve : curves->scale)
				findMappings(curve.name, curve.flags);
		}

		mMappingInfos = newMappingInfos;
	}

	void CAnimation::_refreshClipMappings()
	{
		mPrimaryPlayingClip = mInternal->GetClip(0);

		if (_scriptRebuildFloatProperties)
			_scriptRebuildFloatProperties(mPrimaryPlayingClip);

		updateSceneObjectMapping();		
	}

	Vector<HBone> CAnimation::FindChildBones()
	{
		Stack<HSceneObject> todo;
		todo.Push(SO());

		Vector<HBone> bones;
		while (todo.size() > 0)
		{
			HSceneObject currentSO = todo.Top();
			todo.pop();

			HBone bone = currentSO->getComponent<CBone>();
			if (bone != nullptr)
			{
				bone->_setParent(static_object_cast<CAnimation>(getHandle()), true);
				bones.push_back(bone);
			}

			int childCount = currentSO->GetNumChildren();
			for (int i = 0; i < childCount; i++)
			{
				HSceneObject child = currentSO->GetChild(i);
				if (child->getComponent<CAnimation>() != nullptr)
					continue;

				todo.Push(child);
			}
		}

		return bones;
	}

	void CAnimation::EventTriggered(const HAnimationClip& clip, const String& name)
	{
		onEventTriggered(clip, name);

		if(_scriptOnEventTriggered)
			_scriptOnEventTriggered(clip, name);
	}

	RTTITypeBase* CAnimation::getRTTIStatic()
	{
		return CAnimationRTTI::Instance();
	}

	RTTITypeBase* CAnimation::getRTTI() const
	{
		return CAnimation::GetRTTIStatic();
	}
}
