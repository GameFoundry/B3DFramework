//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCAnimation.h"
#include "BsSceneObject.h"
#include "BsCRenderable.h"
#include "BsCAnimationRTTI.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	CAnimation::CAnimation(const HSceneObject& parent)
		: Component(parent), mWrapMode(AnimWrapMode::Loop), mSpeed(1.0f)
	{
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

	void CAnimation::restoreInternal()
	{
		if (mInternal == nullptr)
			mInternal = Animation::create();

		mInternal->setWrapMode(mWrapMode);
		mInternal->setSpeed(mSpeed);

		if (mDefaultClip.isLoaded())
			mInternal->play(mDefaultClip);

		HRenderable renderableComponent = SO()->getComponent<CRenderable>();
		if (renderableComponent == nullptr)
			return;

		SPtr<Renderable> renderable = renderableComponent->_getRenderable();

		if(renderable != nullptr)
			renderable->setAnimation(mInternal);
	}

	void CAnimation::destroyInternal()
	{
		HRenderable renderableComponent = SO()->getComponent<CRenderable>();
		if (renderableComponent == nullptr)
			return;

		SPtr<Renderable> renderable = renderableComponent->_getRenderable();

		if (renderable != nullptr)
			renderable->setAnimation(nullptr);

		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
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