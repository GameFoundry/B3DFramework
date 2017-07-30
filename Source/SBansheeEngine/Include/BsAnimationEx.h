//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsAnimationClip.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for AnimationCurves, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(e:AnimationCurves) AnimationCurvesEx
	{
	public:
		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Position,pr:getter)
		static Vector<TNamedAnimationCurve<Vector3>> getPositionCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating scene object's position. */
		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Position,pr:setter)
		static void setPositionCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value);

		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Rotation,pr:getter)
		static Vector<TNamedAnimationCurve<Quaternion>> getRotationCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating scene object's rotation. */
		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Rotation,pr:setter)
		static void setRotationCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Quaternion>>& value);

		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Scale,pr:getter)
		static Vector<TNamedAnimationCurve<Vector3>> getScaleCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating scene object's scale. */
		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Scale,pr:setter)
		static void setScaleCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value);

		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Generic,pr:getter)
		static Vector<TNamedAnimationCurve<float>> getGenericCurves(const SPtr<AnimationCurves>& thisPtr);

		/** Curves for animating generic component properties. */
		BS_SCRIPT_EXPORT(e:AnimationCurves,n:Generic,pr:setter)
		static void setGenericCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<float>>& value);
	};

	/** Extension class for RootMotion, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(e:RootMotion) RootMotionEx
	{
	public:
		/** Animation curve representing the movement of the root bone. */
		BS_SCRIPT_EXPORT(e:RootMotion,n:Position,pr:getter)
		static TAnimationCurve<Vector3> getPositionCurves(const SPtr<RootMotion>& thisPtr);

		/** Animation curve representing the rotation of the root bone. */
		BS_SCRIPT_EXPORT(e:RootMotion,n:Rotation,pr:getter)
		static TAnimationCurve<Quaternion> getRotationCurves(const SPtr<RootMotion>& thisPtr);
	};

	/** @endcond */
	/** @} */
}
