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
		/** 
		 * Registers a new curve used for animating position. 
		 *
		 * @param[in]	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param[in]	curve		Curve to add to the clip.
		 */
		void addPositionCurve(const String& name, const TAnimationCurve<Vector3>& curve);

		/** 
		 * Registers a new curve used for animating rotation. 
		 *
		 * @param[in]	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param[in]	curve		Curve to add to the clip.
		 */
		void addRotationCurve(const String& name, const TAnimationCurve<Quaternion>& curve);

		/** 
		 * Registers a new curve used for animating scale. 
		 *
		 * @param[in]	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param[in]	curve		Curve to add to the clip.
		 */
		void addScaleCurve(const String& name, const TAnimationCurve<Vector3>& curve);

		/** 
		 * Registers a new curve used for generic animation.
		 *
		 * @param[in]	name		Unique name of the curve. This can be used for retrieving the value of the curve
		 *							from animation.
		 * @param[in]	curve		Curve to add to the clip.
		 */
		void addGenericCurve(const String& name, const TAnimationCurve<float>& curve);

		/** Removes an existing curve from the clip. */
		void removePositionCurve(const String& name);

		/** Removes an existing curve from the clip. */
		void removeRotationCurve(const String& name);

		/** Removes an existing curve from the clip. */
		void removeScaleCurve(const String& name);

		/** Removes an existing curve from the clip. */
		void removeGenericCurve(const String& name);

		Vector<TNamedAnimationCurve<Vector3>> position;
		Vector<TNamedAnimationCurve<Quaternion>> rotation;
		Vector<TNamedAnimationCurve<Vector3>> scale;
		Vector<TNamedAnimationCurve<float>> generic;
	};

	/** Contains a set of animation curves used for moving and rotating the root bone. */
	struct RootMotion
	{
		RootMotion() { }
		RootMotion(const TAnimationCurve<Vector3>& position, const TAnimationCurve<Quaternion>& rotation)
			:position(position), rotation(rotation)
		{ }

		TAnimationCurve<Vector3> position;
		TAnimationCurve<Quaternion> rotation;
	};

	/** Event that is triggered when animation reaches a certain point. */
	struct AnimationEvent
	{
		AnimationEvent()
			:time(0.0f)
		{ }

		AnimationEvent(const String& name, float time)
			:name(name), time(time)
		{ }

		String name;
		float time;
	};

	/** Types of curves in an AnimationClip. */
	enum class CurveType
	{
		Position,
		Rotation,
		Scale,
		Generic,
		MorphFrame,
		MorphWeight,
		Count // Keep at end
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
		 * Returns all curves stored in the animation. Returned value will not be updated if the animation clip curves are
		 * added or removed. Caller must monitor for changes and retrieve a new set of curves when that happens.
		 */
		SPtr<AnimationCurves> getCurves() const { return mCurves; }

		/** Assigns a new set of curves to be used by the animation. The clip will store a copy of this object.*/
		void setCurves(const AnimationCurves& curves);

		/** Returns all events that will be triggered by the animation. */
		const Vector<AnimationEvent>& getEvents() const { return mEvents; }

		/** Sets events that will be triggered as the animation is playing. */
		void setEvents(const Vector<AnimationEvent>& events) { mEvents = events; }

		/** 
		 * Returns a set of curves containing motion of the root bone. This allows the user to evaluate the root bone
		 * animation curves manually, instead of through the normal animation process. This property is only available
		 * if animation clip was imported with root motion import enabled. 
		 */
		SPtr<RootMotion> getRootMotion() const { return mRootMotion; }

		/** Checks if animation clip has root motion curves separate from the normal animation curves. */
		bool hasRootMotion() const;

		/**
		 * Maps skeleton bone names to animation curve names, and returns a set of indices that can be easily used for
		 * locating an animation curve based on the bone index.
		 *
		 * @param[in]	skeleton	Skeleton to create the mapping for.
		 * @param[out]	mapping		Pre-allocated array that will receive output animation clip indices. The array must
		 *							be large enough to store an index for every bone in the @p skeleton. Bones that have
		 *							no related animation curves will be assigned value -1.
		 */
		void getBoneMapping(const Skeleton& skeleton, AnimationCurveMapping* mapping) const;

		/** 
		 * Attempts to find translation/rotation/scale curves with the specified name and fills the mapping structure with
		 * their indices, which can then be used for quick lookup.
		 *
		 * @param[in]	name		Name of the curves to look up.
		 * @param[out]	mapping		Triple containing the translation/rotation/scale indices of the found curves. Indices
		 *							will be -1 for curves that haven't been found.
		 */
		void getCurveMapping(const String& name, AnimationCurveMapping& mapping) const;

		/** 
		 * Attempts to find a generic curve with the specified name and fills output with found index, which can then be
		 * used for quick lookup.
		 *
		 * @param[in]	name		Name of the curve to look up.
		 * @param[out]	frameIdx	Index of the curve animating the morph shape frames, or -1 if not found.
		 * @param[out]	weightIdx	Index of the curve animating the channel weight, or -1 if not found.
		 */
		void getMorphMapping(const String& name, UINT32& frameIdx, UINT32& weightIdx) const;

		/** 
		 * Checks are the curves contained within the clip additive. Additive clips are intended to be added on top of
		 * other clips.
		 */
		bool isAdditive() const { return mIsAdditive; }

		/** Returns the length of the animation clip, in seconds. */
		float getLength() const { return mLength; }

		/** 
		 * Returns the number of samples per second the animation clip curves were sampled at.
		 *
		 * @note	This value is not used by the animation clip or curves directly since unevenly spaced keyframes are
		 *			supported. But it can be of value when determining the original sample rate of an imported animation
		 *			or similar.
		 */
		UINT32 getSampleRate() const { return mSampleRate; }

		/** 
		 * Sets the number of samples per second the animation clip curves were sampled at.
		 *
		 * @see	getSampleRate()
		 */
		void setSampleRate(UINT32 sampleRate) { mSampleRate = sampleRate; }

		/** 
		 * Returns a version that can be used for detecting modifications on the clip by external systems. Whenever the clip
		 * is modified the version is increased by one.
		 */
		UINT64 getVersion() const { return mVersion; }

		/** 
		 * Creates an animation clip with no curves. After creation make sure to register some animation curves before
		 * using it. 
		 */
		static HAnimationClip create(bool isAdditive = false);

		/** 
		 * Creates an animation clip with specified curves.
		 *
		 * @param[in]	curves		Curves to initialize the animation with.
		 * @param[in]	isAdditive	Determines does the clip contain additive curve data. This will change the behaviour
		 *							how is the clip blended with other animations.
		 * @param[in]	sampleRate	If animation uses evenly spaced keyframes, number of samples per second. Not relevant
		 *							if keyframes are unevenly spaced.
		 * @param[in]	rootMotion	Optional set of curves that can be used for animating the root bone. Not used by the
		 *							animation system directly but is instead provided to the user for manual evaluation.
		 */
		static HAnimationClip create(const SPtr<AnimationCurves>& curves, bool isAdditive = false, UINT32 sampleRate = 1, 
			const SPtr<RootMotion>& rootMotion = nullptr);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Creates a new AnimationClip without initializing it. Use create() for normal use. */
		static SPtr<AnimationClip> _createPtr(const SPtr<AnimationCurves>& curves, bool isAdditive = false, 
			UINT32 sampleRate = 1, const SPtr<RootMotion>& rootMotion = nullptr);

		/** @} */

	protected:
		AnimationClip();
		AnimationClip(const SPtr<AnimationCurves>& curves, bool isAdditive, UINT32 sampleRate, 
			const SPtr<RootMotion>& rootMotion);

		/** @copydoc Resource::initialize() */
		void initialize() override;

		/** Creates a name -> curve index mapping for quicker curve lookup by name. */
		void buildNameMapping();

		/** Calculate the length of the clip based on assigned curves. */
		void calculateLength();

		UINT64 mVersion;

		/** 
		 * Contains all the animation curves in the clip. It's important this field is immutable so it may be used on other
		 * threads. This means any modifications to the field will require a brand new data structure to be generated and
		 * all existing data copied (plus the modification).
		 */
		SPtr<AnimationCurves> mCurves;

		/**
		 * A set of curves containing motion of the root bone. If this is non-empty it should be true that mCurves does not
		 * contain animation curves for the root bone. Root motion will not be evaluated through normal animation process
		 * but is instead provided for the user for manual evaluation. 
		 */
		SPtr<RootMotion> mRootMotion;

		/** 
		 * Contains a map from curve name to curve index. Indices are stored as specified in CurveType enum. 
		 */
		UnorderedMap<String, UINT32[(int)CurveType::Count]> mNameMapping;

		Vector<AnimationEvent> mEvents;
		bool mIsAdditive;
		float mLength;
		UINT32 mSampleRate;

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