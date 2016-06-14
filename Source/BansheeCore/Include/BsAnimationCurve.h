//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup Animation
	 *  @{
	 */

	template <class T>
	struct TKeyframe
	{
		T value;
		T inTangent;
		T outTangent;
		float time;
	};

	template <class T>
	class BS_CORE_EXPORT TAnimationCurve
	{
	public:
		typedef TKeyframe<T> KeyFrame;

		TAnimationCurve(const Vector<KeyFrame>& keyframes);

		T evaluate(const AnimationInstanceData& animInstance, bool loop = true);

	private:
		void findKeys(const AnimationInstanceData& animInstance, UINT32& leftKey, UINT32& rightKey);
		void findKeys(float time, UINT32& leftKey, UINT32& rightKey);

		static const UINT32 CACHE_LOOKAHEAD;

		Vector<KeyFrame> mKeyframes;
		float mStart;
		float mEnd;
		float mLength;
	};

	/** @} */
}