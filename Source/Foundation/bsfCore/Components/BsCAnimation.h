//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Animation/BsAnimation.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Animation
	 *
	 * @note	Wraps Animation as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(Animation)) CAnimation : public Component
	{
		/** Information about scene objects bound to a specific animation curve. */
		struct SceneObjectMappingInfo
		{
			HSceneObject SceneObject;
			bool IsMappedToBone;
			HBone Bone;
		};

	public:
		CAnimation(const HSceneObject& parent);
		virtual ~CAnimation() = default;

		/**
		 * Determines the default clip to play as soon as the component is enabled. If more control over playing clips is needed
		 * use the play(), blend() and crossFade() methods to queue clips for playback manually, and setState() method for
		 * modify their states individually.
		 */
		B3D_SCRIPT_EXPORT(ExportName(DefaultClip), Property(Setter))
		void SetDefaultClip(const HAnimationClip& clip);

		/** @copydoc SetDefaultClip */
		B3D_SCRIPT_EXPORT(ExportName(DefaultClip), Property(Getter))

		HAnimationClip GetDefaultClip() const { return mDefaultClip; }

		/** @copydoc Animation::SetWrapMode */
		B3D_SCRIPT_EXPORT(ExportName(WrapMode), Property(Setter))
		void SetWrapMode(AnimWrapMode wrapMode);

		/** @copydoc SetWrapMode */
		B3D_SCRIPT_EXPORT(ExportName(WrapMode), Property(Getter))
		AnimWrapMode GetWrapMode() const { return mWrapMode; }

		/** @copydoc Animation::SetSpeed */
		B3D_SCRIPT_EXPORT(ExportName(Speed), Property(Setter))
		void SetSpeed(float speed);

		/** @copydoc SetSpeed */
		B3D_SCRIPT_EXPORT(ExportName(Speed), Property(Getter))
		float GetSpeed() const { return mSpeed; }

		/** @copydoc Animation::Play */
		B3D_SCRIPT_EXPORT(ExportName(Play))
		void Play(const HAnimationClip& clip);

		/** @copydoc Animation::BlendAdditive */
		B3D_SCRIPT_EXPORT(ExportName(BlendAdditive))
		void BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength = 0.0f, u32 layer = 0);

		/** @copydoc Animation::Blend1D */
		B3D_SCRIPT_EXPORT(ExportName(Blend1D))
		void Blend1D(const Blend1DInfo& info, float t);

		/** @copydoc Animation::Blend2D */
		B3D_SCRIPT_EXPORT(ExportName(Blend2D))
		void Blend2D(const Blend2DInfo& info, const Vector2& t);

		/** @copydoc Animation::CrossFade */
		B3D_SCRIPT_EXPORT(ExportName(CrossFade))
		void CrossFade(const HAnimationClip& clip, float fadeLength);

		/** @copydoc Animation::Sample */
		B3D_SCRIPT_EXPORT(ExportName(Sample))
		void Sample(const HAnimationClip& clip, float time);

		/** @copydoc Animation::Stop */
		B3D_SCRIPT_EXPORT(ExportName(Stop))
		void Stop(u32 layer);

		/** @copydoc Animation::StopAll */
		B3D_SCRIPT_EXPORT(ExportName(StopAll))
		void StopAll();

		/** @copydoc Animation::IsPlaying */
		B3D_SCRIPT_EXPORT(ExportName(IsPlaying), Property(Getter))
		bool IsPlaying() const;

		/** @copydoc Animation::GetState */
		B3D_SCRIPT_EXPORT(ExportName(GetState))
		bool GetState(const HAnimationClip& clip, AnimationClipState& state);

		/** @copydoc Animation::SetState */
		B3D_SCRIPT_EXPORT(ExportName(SetState))
		void SetState(const HAnimationClip& clip, AnimationClipState state);

		/**
		 * Changes a weight of a single morph channel, determining how much of it to apply on top of the base mesh.
		 *
		 * @param name		Name of the morph channel to modify. This depends on the mesh the animation is currently
		 *					animating.
		 * @param weight	Weight that determines how much of the channel to apply to the mesh, in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMorphChannelWeight))
		void SetMorphChannelWeight(const String& name, float weight);

		/** Determines bounds that will be used for animation and mesh culling. Only relevant if setUseBounds() is set to true. */
		B3D_SCRIPT_EXPORT(ExportName(Bounds), Property(Setter))
		void SetBounds(const AABox& bounds);

		/** @copydoc SetBounds */
		B3D_SCRIPT_EXPORT(ExportName(Bounds), Property(Getter))
		const AABox& GetBounds() const { return mBounds; }

		/**
		 * Determines should animation bounds be used for visibility determination (culling). If false the bounds of the
		 * mesh attached to the relevant CRenderable component will be used instead.
		 */
		B3D_SCRIPT_EXPORT(ExportName(UseBounds), Property(Setter))
		void SetUseBounds(bool enable);

		/** @copydoc SetUseBounds */
		B3D_SCRIPT_EXPORT(ExportName(UseBounds), Property(Getter))

		bool GetUseBounds() const { return mUseBounds; }

		/** Enables or disables culling of the animation when out of view. Culled animation will not be evaluated. */
		B3D_SCRIPT_EXPORT(ExportName(Cull), Property(Setter))
		void SetEnableCull(bool enable);

		/** Checks whether the animation will be evaluated when it is out of view. */
		B3D_SCRIPT_EXPORT(ExportName(Cull), Property(Getter))

		bool GetEnableCull() const { return mEnableCull; }

		/** @copydoc Animation::GetNumClips */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		u32 GetNumClips() const;

		/** @copydoc Animation::GetClip */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		HAnimationClip GetClip(u32 idx) const;

		/** Triggered whenever an animation event is reached. */
		Event<void(const HAnimationClip&, const String&)> OnEventTriggered;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Animation implementation wrapped by this component. */
		SPtr<Animation> GetInternalInternal() const { return mInternal; }

		/**
		 * Registers a new bone component, creating a new transform mapping from the bone name to the scene object the
		 * component is attached to.
		 */
		void AddBoneInternal(HBone bone);

		/** Unregisters a bone component, removing the bone -> scene object mapping. */
		void RemoveBoneInternal(const HBone& bone);

		/** Called whenever the bone name the Bone component points to changes. */
		void NotifyBoneChangedInternal(const HBone& bone);

		/**
		 * Registers a Renderable component with the animation, should be called whenever a Renderable component is added
		 * to the same scene object as this component.
		 */
		void RegisterRenderableInternal(const HRenderable& renderable);

		/**
		 * Removes renderable from the animation component. Should be called when a Renderable component is removed from
		 * this scene object.
		 */
		void UnregisterRenderableInternal();

		/** Re-applies the bounds to the internal animation object, and the relevant renderable object if one exists. */
		void UpdateBoundsInternal(bool updateRenderable = true);

		/**
		 * Rebuilds internal curve -> property mapping about the currently playing animation clip. This mapping allows the
		 * animation component to know which property to assign which values from an animation curve. This should be called
		 * whenever playback for a new clip starts, or when clip curves change.
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		void RefreshClipMappingsInternal();

		/** @copydoc Animation::GetGenericCurveValue */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		bool GetGenericCurveValueInternal(u32 curveIdx, float& value);

		/**
		 * Preview mode allows certain operations on the component to be allowed (like basic animation playback),
		 * even when the component is not actively running. This is intended for use primarily by the animation editor.
		 * Preview mode ends automatically when the component is enabled (i.e. starts running normally), or when
		 * explicitly disabled. Returns true if the preview mode was enabled (which could fail if the component is
		 * currently running).
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		bool TogglePreviewModeInternal(bool enabled);

		/** Triggered when the list of properties animated via generic animation curves needs to be recreated (script only). */
		B3D_SCRIPT_EXPORT(ExportName(RebuildFloatProperties))
		std::function<void(const HAnimationClip&)> ScriptRebuildFloatPropertiesInternal;

		/** Triggered when generic animation curves values need be applied to the properties they effect (script only). */
		B3D_SCRIPT_EXPORT(ExportName(_UpdateFloatProperties))
		std::function<void()> ScriptUpdateFloatPropertiesInternal;

		/** Triggers a callback in script code when animation event is triggered (script only). */
		B3D_SCRIPT_EXPORT(ExportName(EventTriggered))
		std::function<void(const HAnimationClip&, const String&)> ScriptOnEventTriggeredInternal;

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnBeginPlay() override;
		void OnDestroyed() override;
		void Update() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

	protected:
		/** Creates the internal representation of the Animation and restores the values saved by the Component. */
		void RestoreInternal(bool previewMode);

		/** Destroys the internal Animation representation. */
		void DestroyInternal();

		/** Callback triggered whenever an animation event is triggered. */
		void EventTriggered(const HAnimationClip& clip, const String& name);

		/**
		 * Finds any scene objects that are mapped to bone transforms. Such object's transforms will be affected by
		 * skeleton bone animation.
		 */
		void SetBoneMappings();

		/**
		 * Finds any curves that affect a transform of a specific scene object, and ensures that animation properly updates
		 * those transforms. This does not include curves referencing bones.
		 */
		void UpdateSceneObjectMapping();

		/** @copydoc Animation::MapCurveToSceneObject */
		void MapCurveToSceneObject(const String& curve, const HSceneObject& so);

		/** @copydoc Animation::UnmapSceneObject */
		void UnmapSceneObject(const HSceneObject& so);

		/** Searches child scene objects for Bone components and returns any found ones. */
		Vector<HBone> FindChildBones();

		SPtr<Animation> mInternal;
		HRenderable mAnimatedRenderable;

		HAnimationClip mDefaultClip;
		HAnimationClip mPrimaryPlayingClip;
		AnimWrapMode mWrapMode = AnimWrapMode::Loop;
		float mSpeed = 1.0f;
		bool mEnableCull = true;
		bool mUseBounds = false;
		bool mPreviewMode = false;
		AABox mBounds;

		Vector<SceneObjectMappingInfo> mMappingInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAnimationRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CAnimation(); // Serialization only
	};

	/** @} */
} // namespace bs
