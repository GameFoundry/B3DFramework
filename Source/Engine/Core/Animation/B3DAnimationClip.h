//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"
#include "Animation/B3DAnimationCurve.h"
#include <array>

namespace b3d
{
	/** @addtogroup Animation
	 *  @{
	 */

	struct AnimationCurveMapping;

	/** A set of animation curves representing translation/rotation/scale and generic animation. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) AnimationCurves : public IScriptExportable
	{
		B3D_SCRIPT_EXPORT()
		AnimationCurves() = default;

		/**
		 * Registers a new curve used for animating position.
		 *
		 * @param	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param	curve		Curve to add to the clip.
		 */
		B3D_SCRIPT_EXPORT(ExportName(AddPositionCurve))
		void AddPositionCurve(const String& name, const TAnimationCurve<Vector3>& curve);

		/**
		 * Registers a new curve used for animating rotation.
		 *
		 * @param	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param	curve		Curve to add to the clip.
		 */
		B3D_SCRIPT_EXPORT(ExportName(AddRotationCurve))
		void AddRotationCurve(const String& name, const TAnimationCurve<Quaternion>& curve);

		/**
		 * Registers a new curve used for animating scale.
		 *
		 * @param	name		Unique name of the curve. This name will be used mapping the curve to the relevant bone
		 *							in a skeleton, if any.
		 * @param	curve		Curve to add to the clip.
		 */
		B3D_SCRIPT_EXPORT(ExportName(AddScaleCurve))
		void AddScaleCurve(const String& name, const TAnimationCurve<Vector3>& curve);

		/**
		 * Registers a new curve used for generic animation.
		 *
		 * @param	name		Unique name of the curve. This can be used for retrieving the value of the curve
		 *							from animation.
		 * @param	curve		Curve to add to the clip.
		 */
		B3D_SCRIPT_EXPORT(ExportName(AddGenericCurve))
		void AddGenericCurve(const String& name, const TAnimationCurve<float>& curve);

		/** Removes an existing curve from the clip. */
		B3D_SCRIPT_EXPORT(ExportName(RemovePositionCurve))
		void RemovePositionCurve(const String& name);

		/** Removes an existing curve from the clip. */
		B3D_SCRIPT_EXPORT(ExportName(RemoveRotationCurve))
		void RemoveRotationCurve(const String& name);

		/** Removes an existing curve from the clip. */
		B3D_SCRIPT_EXPORT(ExportName(RemoveScaleCurve))
		void RemoveScaleCurve(const String& name);

		/** Removes an existing curve from the clip. */
		B3D_SCRIPT_EXPORT(ExportName(RemoveGenericCurve))
		void RemoveGenericCurve(const String& name);

		/** Curves for animating scene object's position. */
		Vector<TNamedAnimationCurve<Vector3>> Position;

		/** Curves for animating scene object's rotation. */
		Vector<TNamedAnimationCurve<Quaternion>> Rotation;

		/** Curves for animating scene object's scale. */
		Vector<TNamedAnimationCurve<Vector3>> Scale;

		/** Curves for animating generic component properties. */
		Vector<TNamedAnimationCurve<float>> Generic;
	};

	/** Contains a set of animation curves used for moving and rotating the root bone. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) RootMotion : public IScriptExportable
	{
		RootMotion() = default;

		RootMotion(const TAnimationCurve<Vector3>& position, const TAnimationCurve<Quaternion>& rotation)
			: Position(position), Rotation(rotation)
		{}

		/** Animation curve representing the movement of the root bone. */
		TAnimationCurve<Vector3> Position;

		/** Animation curve representing the rotation of the root bone. */
		TAnimationCurve<Quaternion> Rotation;
	};

	/** Event that is triggered when animation reaches a certain point. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportAsStruct(true)) AnimationEvent
	{
		AnimationEvent() = default;

		/**
		 * Constructs a new animation event.
		 *
		 * @param	name	Name used to identify the event when triggered.
		 * @param	time	Time at which to trigger the event, in seconds.
		 */
		AnimationEvent(const String& name, float time)
			: Name(name), Time(time)
		{}

		/** Name used to identify the event when triggered. */
		String Name;

		/** Time at which to trigger the event, in seconds. */
		float Time = 0.0f;
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) AnimationClip : public Resource
	{
	public:
		virtual ~AnimationClip() = default;

		/** @copydoc SetCurves() */
		B3D_SCRIPT_EXPORT(ExportName(Curves), Property(Getter))

		TShared<AnimationCurves> GetCurves() const { return mCurves; }

		/**
		 * A set of all curves stored in the animation. Returned value will not be updated if the animation clip curves are
		 * added or removed, as it is a copy of clip's internal values.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Curves), Property(Setter))
		void SetCurves(const AnimationCurves& curves);

		/** @copydoc SetEvents() */
		B3D_SCRIPT_EXPORT(ExportName(Events), Property(Getter))

		const Vector<AnimationEvent>& GetEvents() const { return mEvents; }

		/** A set of all events to be triggered as the animation is playing. */
		B3D_SCRIPT_EXPORT(ExportName(Events), Property(Setter))

		void SetEvents(const Vector<AnimationEvent>& events) { mEvents = events; }

		/**
		 * Returns a set of curves containing motion of the root bone. This allows the user to evaluate the root bone
		 * animation curves manually, instead of through the normal animation process. This property is only available
		 * if animation clip was imported with root motion import enabled.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RootMotion), Property(Getter))

		TShared<RootMotion> GetRootMotion() const { return mRootMotion; }

		/** Checks if animation clip has root motion curves separate from the normal animation curves. */
		B3D_SCRIPT_EXPORT(ExportName(HasRootMotion), Property(Getter))
		bool HasRootMotion() const;

		/**
		 * Maps skeleton bone names to animation curve names, and returns a set of indices that can be easily used for
		 * locating an animation curve based on the bone index.
		 *
		 * @param	skeleton		Skeleton to create the mapping for.
		 * @param	outMapping		Pre-allocated array that will receive output animation clip indices. The array must
		 *								be large enough to store an index for every bone in the @p skeleton. Bones that have
		 *								no related animation curves will be assigned value -1.
		 */
		void GetBoneMapping(const Skeleton& skeleton, AnimationCurveMapping* outMapping) const;

		/**
		 * Attempts to find translation/rotation/scale curves with the specified name and fills the mapping structure with
		 * their indices, which can then be used for quick lookup.
		 *
		 * @param	name			Name of the curves to look up.
		 * @param	outMapping		Triple containing the translation/rotation/scale indices of the found curves. Indices
		 *								will be -1 for curves that haven't been found.
		 */
		void GetCurveMapping(const String& name, AnimationCurveMapping& outMapping) const;

		/**
		 * Attempts to find a generic curve with the specified name and fills output with found index, which can then be
		 * used for quick lookup.
		 *
		 * @param	name				Name of the curve to look up.
		 * @param	outFrameIndex		Index of the curve animating the morph shape frames, or -1 if not found.
		 * @param	outWeightIndex		Index of the curve animating the channel weight, or -1 if not found.
		 */
		void GetMorphMapping(const String& name, u32& outFrameIndex, u32& outWeightIndex) const;

		/**
		 * Checks are the curves contained within the clip additive. Additive clips are intended to be added on top of
		 * other clips.
		 */
		B3D_SCRIPT_EXPORT(ExportName(IsAddtive), Property(Getter))

		bool IsAdditive() const { return mIsAdditive; }

		/** Returns the length of the animation clip, in seconds. */
		B3D_SCRIPT_EXPORT(ExportName(Length), Property(Getter))

		float GetLength() const { return mLength; }

		/** @copydoc SetSampleRate() */
		B3D_SCRIPT_EXPORT(ExportName(SampleRate), Property(Getter))

		u32 GetSampleRate() const { return mSampleRate; }

		/**
		 * Number of samples per second the animation clip curves were sampled at. This value is not used by the animation
		 * clip or curves directly since unevenly spaced keyframes are supported. But it can be of value when determining
		 * the original sample rate of an imported animation or similar.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SampleRate), Property(Setter))

		void SetSampleRate(u32 sampleRate) { mSampleRate = sampleRate; }

		/**
		 * Returns a version that can be used for detecting modifications on the clip by external systems. Whenever the clip
		 * is modified the version is increased by one.
		 */
		u64 GetVersion() const { return mVersion; }

		/**
		 * Creates an animation clip with no curves. After creation make sure to register some animation curves before
		 * using it.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(AnimationClip))
		static HAnimationClip Create(bool isAdditive = false);

		/**
		 * Creates an animation clip with specified curves.
		 *
		 * @param	curves		Curves to initialize the animation with.
		 * @param	isAdditive	Determines does the clip contain additive curve data. This will change the behaviour
		 *							how is the clip blended with other animations.
		 * @param	sampleRate	If animation uses evenly spaced keyframes, number of samples per second. Not relevant
		 *							if keyframes are unevenly spaced.
		 * @param	rootMotion	Optional set of curves that can be used for animating the root bone. Not used by the
		 *							animation system directly but is instead provided to the user for manual evaluation.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(AnimationClip))
		static HAnimationClip Create(const TShared<AnimationCurves>& curves, bool isAdditive = false, u32 sampleRate = 1, const TShared<RootMotion>& rootMotion = nullptr);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Creates a new AnimationClip without initializing it. Use Create() for normal use. */
		static TShared<AnimationClip> CreateShared(const TShared<AnimationCurves>& curves, bool isAdditive = false, u32 sampleRate = 1, const TShared<RootMotion>& rootMotion = nullptr);

		/** @} */

	protected:
		AnimationClip();
		AnimationClip(const TShared<AnimationCurves>& curves, bool isAdditive, u32 sampleRate, const TShared<RootMotion>& rootMotion);

		void Initialize() override;

		/** Creates a name -> curve index mapping for quicker curve lookup by name. */
		void BuildNameMapping();

		/** Calculate the length of the clip based on assigned curves. */
		void CalculateLength();

		u64 mVersion;

		/**
		 * Contains all the animation curves in the clip. It's important this field is immutable so it may be used on other
		 * threads. This means any modifications to the field will require a brand new data structure to be generated and
		 * all existing data copied (plus the modification).
		 */
		TShared<AnimationCurves> mCurves;

		/**
		 * A set of curves containing motion of the root bone. If this is non-empty it should be true that mCurves does not
		 * contain animation curves for the root bone. Root motion will not be evaluated through normal animation process
		 * but is instead provided for the user for manual evaluation.
		 */
		TShared<RootMotion> mRootMotion;

		/**
		 * Contains a map from curve name to curve index. Indices are stored as specified in CurveType enum.
		 */
		UnorderedMap<String, std::array<u32, (int)CurveType::Count>> mNameMapping;

		Vector<AnimationEvent> mEvents;
		bool mIsAdditive;
		float mLength;
		u32 mSampleRate;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AnimationClipRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

		/**
		 * Creates an AnimationClip with no data. You must populate its data manually followed by a call to Initialize().
		 *
		 * @note	For serialization use only.
		 */
		static TShared<AnimationClip> CreateEmpty();
	};

	/** @} */
} // namespace b3d
