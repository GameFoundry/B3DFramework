//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsAnimationCurve.h"

namespace BansheeEngine
{
	/** @addtogroup Animation
	 *  @{
	 */

	struct AnimationCurveMapping;

	/** A set of animation curves representing translation/rotation/scale and generic animation. */
	struct AnimationCurves
	{
		Vector<TNamedAnimationCurve<Vector3>> position;
		Vector<TNamedAnimationCurve<Quaternion>> rotation;
		Vector<TNamedAnimationCurve<Vector3>> scale;
		Vector<TNamedAnimationCurve<float>> generic;
	};

	/** Types of curves in an AnimationClip. */
	enum class CurveType
	{
		Position,
		Rotation,
		Scale,
		Generic
	};

	/** 
	 * Contains animation curves for translation/rotation/scale of scene objects/skeleton bones, as well as curves for
	 * generic property animation. 
	 */
	class BS_CORE_EXPORT AnimationClip : public Resource
	{
	public:
		virtual ~AnimationClip() { }

		/** 
		 * Registers a new curve used for animating position. 
		 *
		 * @param[in]	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param[in]	curve		Curve to add to the clip.
		 *
		 * @note Adding a new curve to a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void addPositionCurve(const String& name, const TAnimationCurve<Vector3>& curve);

		/** 
		 * Registers a new curve used for animating rotation. 
		 *
		 * @param[in]	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param[in]	curve		Curve to add to the clip.
		 *
		 * @note Adding a new curve to a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void addRotationCurve(const String& name, const TAnimationCurve<Quaternion>& curve);

		/** 
		 * Registers a new curve used for animating scale. 
		 *
		 * @param[in]	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param[in]	curve		Curve to add to the clip.
		 *
		 * @note Adding a new curve to a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void addScaleCurve(const String& name, const TAnimationCurve<Vector3>& curve);

		/** 
		 * Registers a new curve used for generic animation.
		 *
		 * @param[in]	name		Unique name of the curve. This can be used for retrieving the value of the curve
		 *							from animation.
		 * @param[in]	curve		Curve to add to the clip.
		 *
		 * @note Adding a new curve to a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void addGenericCurve(const String& name, const TAnimationCurve<float>& curve);

		/**
		 * Removes an existing curve from the clip.
		 *
		 * @param[in]	name	Name of the curve to remove.
		 *
		 * @note Removing curve from a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void removePositionCurve(const String& name);

		/**
		 * Removes an existing curve from the clip.
		 *
		 * @param[in]	name	Name of the curve to remove.
		 *
		 * @note Removing curve from a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void removeRotationCurve(const String& name);

		/**
		 * Removes an existing curve from the clip.
		 *
		 * @param[in]	name	Name of the curve to remove.
		 *
		 * @note Removing curve from a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void removeScaleCurve(const String& name);

		/**
		 * Removes an existing curve from the clip.
		 *
		 * @param[in]	name	Name of the curve to remove.
		 *
		 * @note Removing curve from a clip is a relatively slow operations and shouldn't be done at runtime.
		 */
		void removeGenericCurve(const String& name);

		/** 
		 * Returns all curves stored in the animation. Returned value will not be updated if the animation clip curves are
		 * added or removed. Caller must monitor for changes and retrieve a new set of curves when that happens.
		 */
		SPtr<AnimationCurves> getCurves() const { return mCurves; }

		/**
		 * Maps skeleton bone names to animation clip names, and returns a set of indices that can be easily used for
		 * locating an animation curve based on the bone index.
		 *
		 * @param[in]	skeleton	Skeleton to create the mapping for.
		 * @param[out]	mapping		Pre-allocated array that will receive output animation clip indices. The array must
		 *							be large enough to store an index for every bone in the @p skeleton. Bones that have
		 *							no related animation curves will be assigned value -1.
		 */
		void getBoneMapping(const Skeleton& skeleton, AnimationCurveMapping* mapping) const;

		/** 
		 * Returns a version that can be used for detecting modifications on the clip by external systems. Whenever the clip
		 * is modified the version is increased by one.
		 */
		UINT64 getVersion() const { return mVersion; }

		/** 
		 * Creates an animation clip with no curves. After creation make sure to register some animation curves before
		 * using it. 
		 */
		static HAnimationClip create();

		/** 
		 * Creates an animation clip with specified curves.
		 *
		 * @param[in]	curves	Curves to initialize the animation with.
		 */
		static HAnimationClip create(const SPtr<AnimationCurves>& curves);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Creates a new AnimationClip without initializing it. Use create() for normal use. */
		static SPtr<AnimationClip> _createPtr(const SPtr<AnimationCurves>& curves);

		/** @} */

	protected:
		AnimationClip();
		AnimationClip(const SPtr<AnimationCurves>& curves);

		/** @copydoc Resource::initialize() */
		void initialize() override;

		/** Creates a name -> curve index mapping for quicker curve lookup by name. */
		void buildNameMapping();

		UINT64 mVersion;

		/** 
		 * Contains all the animation curves in the clip. It's important this field is immutable so it may be used on other
		 * threads. This means any modifications to the field will require a brand new data structure to be generated and
		 * all existing data copied (plus the modification).
		 */
		SPtr<AnimationCurves> mCurves;

		/** 
		 * Contains a map from curve name to curve index. Indices are stored as specified in CurveType enum. 
		 */
		UnorderedMap<String, UINT32[4]> mNameMapping;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AnimationClipRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		/** 
		 * Creates an AnimationClip with no data. You must populate its data manually followed by a call to initialize().
		 *
		 * @note	For serialization use only.
		 */
		static SPtr<AnimationClip> createEmpty();
	};

	/** @} */
}