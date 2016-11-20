//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCAnimation.h"
#include "BsSceneObject.h"
#include "BsCRenderable.h"
#include "BsCBone.h"
#include "BsCAnimationRTTI.h"

using namespace std::placeholders;

namespace bs
{
	CAnimation::CAnimation()
		:mWrapMode(AnimWrapMode::Loop), mSpeed(1.0f), mEnableCull(true), mUseBounds(false)
	{ }

	CAnimation::CAnimation(const HSceneObject& parent)
		: Component(parent), mWrapMode(AnimWrapMode::Loop), mSpeed(1.0f), mEnableCull(true), mUseBounds(false)
	{
		mNotifyFlags = TCF_Transform;

		setName("Animation");
	}

	void CAnimation::setDefaultClip(const HAnimationClip& clip)
	{
		mDefaultClip = clip;

		if(clip.isLoaded() && mInternal != nullptr)
		{
			mInternal->play(clip);
		}
	}

	void CAnimation::setWrapMode(AnimWrapMode wrapMode)
	{
		mWrapMode = wrapMode;

		if (mInternal != nullptr)
			mInternal->setWrapMode(wrapMode);
	}

	void CAnimation::setSpeed(float speed)
	{
		mSpeed = speed;

		if (mInternal != nullptr)
			mInternal->setSpeed(speed);
	}

	void CAnimation::play(const HAnimationClip& clip)
	{
		if (mInternal != nullptr)
			mInternal->play(clip);
	}

	void CAnimation::blendAdditive(const HAnimationClip& clip, float weight, float fadeLength, UINT32 layer)
	{
		if (mInternal != nullptr)
			mInternal->play(clip);
	}

	void CAnimation::blend1D(const Blend1DInfo& info, float t)
	{
		if (mInternal != nullptr)
			mInternal->blend1D(info, t);
	}

	void CAnimation::blend2D(const Blend2DInfo& info, const Vector2& t)
	{
		if (mInternal != nullptr)
			mInternal->blend2D(info, t);
	}

	void CAnimation::crossFade(const HAnimationClip& clip, float fadeLength)
	{
		if (mInternal != nullptr)
			mInternal->crossFade(clip, fadeLength);
	}

	void CAnimation::sample(const HAnimationClip& clip, float time)
	{
		if (mInternal != nullptr)
			mInternal->sample(clip, time);
	}

	void CAnimation::stop(UINT32 layer)
	{
		if (mInternal != nullptr)
			mInternal->stop(layer);
	}

	void CAnimation::stopAll()
	{
		if (mInternal != nullptr)
			mInternal->stopAll();
	}

	bool CAnimation::isPlaying() const
	{
		if (mInternal != nullptr)
			return mInternal->isPlaying();

		return false;
	}

	bool CAnimation::getState(const HAnimationClip& clip, AnimationClipState& state)
	{
		if (mInternal != nullptr)
			return mInternal->getState(clip, state);

		return false;
	}

	void CAnimation::setState(const HAnimationClip& clip, AnimationClipState state)
	{
		if (mInternal != nullptr)
			return mInternal->setState(clip, state);
	}

	void CAnimation::setMorphChannelWeight(UINT32 idx, float weight)
	{
		if (mInternal != nullptr)
			return mInternal->setMorphChannelWeight(idx, weight);
	}

	void CAnimation::setBounds(const AABox& bounds)
	{
		mBounds = bounds;

		if(mUseBounds)
		{
			if(mAnimatedRenderable != nullptr)
			{
				SPtr<Renderable> renderable = mAnimatedRenderable->_getRenderable();
				if (renderable != nullptr)
					renderable->setOverrideBounds(bounds);

				if(mInternal != nullptr)
				{
					AABox bounds = mBounds;
					bounds.transformAffine(SO()->getWorldTfrm());

					mInternal->setBounds(bounds);
				}
			}
		}
	}

	void CAnimation::setUseBounds(bool enable)
	{
		mUseBounds = enable;

		_updateBounds();
	}

	void CAnimation::setEnableCull(bool enable)
	{
		mEnableCull = enable;

		if (mInternal != nullptr)
			mInternal->setCulling(enable);
	}

	void CAnimation::onInitialized()
	{
		
	}

	void CAnimation::onDestroyed()
	{
		destroyInternal();
	}

	void CAnimation::onDisabled()
	{
		destroyInternal();
	}

	void CAnimation::onEnabled()
	{
		restoreInternal();
	}

	void CAnimation::onTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive())
			return;

		if ((flags & (TCF_Transform)) != 0)
			_updateBounds(false);
	}

	void CAnimation::restoreInternal()
	{
		if (mInternal == nullptr)
		{
			mInternal = Animation::create();
			mInternal->onEventTriggered.connect(std::bind(&CAnimation::eventTriggered, this, _1, _2));
		}

		mAnimatedRenderable = SO()->getComponent<CRenderable>();

		mInternal->setWrapMode(mWrapMode);
		mInternal->setSpeed(mSpeed);
		mInternal->setCulling(mEnableCull);

		_updateBounds();

		if (mDefaultClip.isLoaded())
			mInternal->play(mDefaultClip);

		setBoneMappings();

		if (mAnimatedRenderable != nullptr)
			mAnimatedRenderable->_registerAnimation(mThisHandle);
	}

	void CAnimation::destroyInternal()
	{
		if (mAnimatedRenderable != nullptr)
			mAnimatedRenderable->_unregisterAnimation();

		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
	}

	void CAnimation::_addBone(const HBone& bone)
	{
		if (mInternal == nullptr)
			return;

		HSceneObject currentSO = bone->SO();

		SceneObjectMappingInfo newMapping;
		newMapping.sceneObject = currentSO;
		newMapping.isMappedToBone = true;
		newMapping.bone = bone;

		mMappingInfos.push_back(newMapping);
		mInternal->mapCurveToSceneObject(bone->getName(), newMapping.sceneObject);
	}

	void CAnimation::_removeBone(const HBone& bone)
	{
		if (mInternal == nullptr)
			return;

		HSceneObject newSO;
		for (UINT32 i = 0; i < (UINT32)mMappingInfos.size(); i++)
		{
			if (mMappingInfos[i].bone == bone)
			{
				mMappingInfos.erase(mMappingInfos.begin() + i);
				mInternal->unmapSceneObject(mMappingInfos[i].sceneObject);
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
				mInternal->unmapSceneObject(mMappingInfos[i].sceneObject);
				mInternal->mapCurveToSceneObject(bone->getName(), mMappingInfos[i].sceneObject);
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
			renderable = mAnimatedRenderable->_getRenderable();

		if (mUseBounds)
		{
			if (renderable != nullptr)
			{
				renderable->setUseOverrideBounds(true);
				renderable->setOverrideBounds(mBounds);
			}

			if (mInternal != nullptr)
			{
				AABox bounds = mBounds;
				bounds.transformAffine(SO()->getWorldTfrm());

				mInternal->setBounds(bounds);
			}
		}
		else
		{
			if (renderable != nullptr)
				renderable->setUseOverrideBounds(false);

			if (mInternal != nullptr)
			{
				AABox bounds;
				if (mAnimatedRenderable != nullptr)
					bounds = mAnimatedRenderable->getBounds().getBox();

				mInternal->setBounds(bounds);
			}
		}
	}

	void CAnimation::setBoneMappings()
	{
		mMappingInfos.clear();

		SceneObjectMappingInfo rootMapping;
		rootMapping.sceneObject = SO();
		rootMapping.isMappedToBone = true;

		mMappingInfos.push_back(rootMapping);
		mInternal->mapCurveToSceneObject("", rootMapping.sceneObject);

		Vector<HBone> childBones = findChildBones();
		for (auto& entry : childBones)
			_addBone(entry);
	}

	Vector<HBone> CAnimation::findChildBones()
	{
		Stack<HSceneObject> todo;
		todo.push(SO());

		Vector<HBone> bones;
		while (todo.size() > 0)
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			HBone bone = currentSO->getComponent<CBone>();
			if (bone != nullptr)
			{
				bone->_setParent(getHandle(), true);
				bones.push_back(bone);
			}

			int childCount = currentSO->getNumChildren();
			for (int i = 0; i < childCount; i++)
			{
				HSceneObject child = currentSO->getChild(i);
				if (child->getComponent<CAnimation>() != nullptr)
					continue;

				todo.push(child);
			}
		}

		return bones;
	}

	void CAnimation::eventTriggered(const HAnimationClip& clip, const String& name)
	{
		onEventTriggered(clip, name);
	}

	RTTITypeBase* CAnimation::getRTTIStatic()
	{
		return CAnimationRTTI::instance();
	}

	RTTITypeBase* CAnimation::getRTTI() const
	{
		return CAnimation::getRTTIStatic();
	}
}