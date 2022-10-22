//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Animation/BsAnimationClip.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for AnimationCurves, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves)) AnimationCurvesEx
	{
	public:
		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Position,pr:getter)
		static Vector<TNamedAnimationCurve<Vector3>> GetPositionCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating scene object's position. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Position,pr:setter)
		static void SetPositionCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value);

		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Rotation,pr:getter)
		static Vector<TNamedAnimationCurve<Quaternion>> GetRotationCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating scene object's rotation. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Rotation,pr:setter)
		static void SetRotationCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Quaternion>>& value);

		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Scale,pr:getter)
		static Vector<TNamedAnimationCurve<Vector3>> GetScaleCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating scene object's scale. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Scale,pr:setter)
		static void SetScaleCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value);

		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Generic,pr:getter)
		static Vector<TNamedAnimationCurve<float>> GetGenericCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating generic component properties. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(AnimationCurves),n:Generic,pr:setter)
		static void SetGenericCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<float>>& value);
	};

	/** Extension class for RootMotion, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(ExtensionMethodForType(RootMotion)) RootMotionEx
	{
	public:
		/** Animation curve representing the movement of the root bone. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RootMotion),n:Position,pr:getter)
		static TAnimationCurve<Vector3> GetPositionCurves(const SPtr<RootMotion>& thisPtr);

		/** Animation curve representing the rotation of the root bone. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RootMotion),n:Rotation,pr:getter)
		static TAnimationCurve<Quaternion> GetRotationCurves(const SPtr<RootMotion>& thisPtr);
	};

	/** @endcond */
	/** @} */
}
