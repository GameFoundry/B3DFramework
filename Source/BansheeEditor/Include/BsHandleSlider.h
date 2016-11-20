//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsVector2I.h"
#include "BsMatrix4.h"
#include "BsQuaternion.h"

namespace bs
{
	/** @addtogroup Handles
	 *  @{
	 */

	/**
	 * Base class for all handle sliders. A handle slider is geometry that the user can interact with by selecting or 
	 * dragging (sliding) it. Sliders generally output a one- or multi-dimensional delta value that signals the drag 
	 * amount (and/or direction).
	 */
	class BS_ED_EXPORT HandleSlider
	{
	public:
		/** Possible states the slider can be in. */
		enum class State
		{
			Inactive, /**< Slider is not being interacted with. */
			Active, /**< Slider is clicked on and being dragged. */
			Hover /**< Slider is being hovered over but isn't clicked on. */
		};

		/**
		 * Constructs a new handle slider.
		 *
		 * @param[in]	fixedScale	If true the handle slider will always try to maintain the same visible area in the 
		 *							viewport regardless of distance from camera. 
		 * @param[in]	layer		Layer that allows filtering of which sliders are interacted with from a specific camera.
		 */
		HandleSlider(bool fixedScale, UINT64 layer);
		virtual ~HandleSlider() { }

		/**
		 * Attempts to find an intersection between the provided ray and the slider geometry.
		 *
		 * @param[in]	ray	Ray in world space to try to interect with geometry.
		 * @param[in]	t	Position of the intersection along the ray. Only if intersection happened.
		 * @return			Whether an intersection was detected.
		 */
		virtual bool intersects(const Ray& ray, float& t) const = 0;

		/**
		 * Updates a slider that is currently active (being dragged).
		 *
		 * @param[in]	camera		Camera through which we're interacting with the slider.
		 * @param[in]	inputDelta	Pointer movement since the last time this method was called.
		 */
		virtual void handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta) = 0;

		/**
		 * Updates the state of the slider. Must be called every frame.
		 *
		 * @param[in]	camera	Camera through which we're interacting with the slider.
		 */
		void update(const SPtr<Camera>& camera);

		/** Returns the state the slider is currently in. */
		State getState() const { return mState; }
		
		/**
		 * Returns if fixed scale is enabled. If enabled the handle slider will always try to maintain the same visible 
		 * area in the viewport regardless of distance from camera.
		 */
		bool getFixedScale() const { return mFixedScale; }

		/** Returns a layer that determines which sliders are interacted with from a specific camera. */
		UINT64 getLayer() const { return mLayer; }

		/**	Sets the world position of the slider. */
		void setPosition(const Vector3& position);

		/**	Sets the world rotation of the slider. */
		void setRotation(const Quaternion& rotation);

		/**	Sets the scale of the slider. */
		void setScale(const Vector3& scale);

		/**	Enables or disabled the slider, making it interactable or not. */
		void setEnabled(bool enabled);

		/**	Gets the world position of the slider. */
		const Vector3& getPosition() const { return mPosition; }

		/**	Gets the world rotation of the slider. */
		const Quaternion& getRotation() const { return mRotation; }

		/**	Gets the scale of the slider. */
		const Vector3& getScale() const { return mScale; }

		/**	Checks whether the slider can be interacted with or not. */
		bool getEnabled() const { return mEnabled; }

	protected:
		friend class HandleSliderManager;

		/**	Toggles the slider state to inactive. */
		void setInactive();

		/**
		 * Toggles the slider state to active.
		 *
		 * @param[in]	camera		Camera through which the slider was activated.
		 * @param[in]	pointerPos	Position of the pointer when the slider was activated.
		 */
		void setActive(const SPtr<Camera>& camera, const Vector2I& pointerPos);

		/**	Toggles the slider state to hovered. */
		void setHover();

		/**	Gets the slider transform depending on set position, rotation and scale values. */
		const Matrix4& getTransform() const;

		/**	Gets the inverse of the slider transform depending on set position, rotation and scale values. */
		const Matrix4& getTransformInv() const;

		/** Triggered when the slider state is changed to active. */
		virtual void activate(const SPtr<Camera>& camera, const Vector2I& pointerPos) { }

		/**	Triggered when the slider state is changed from active to some other state. */
		virtual void reset() { }

		/**	Updates the internal transform from the stored position, rotation and scale values. */
		void updateCachedTransform() const;

		/**
		 * Calculates amount of movement along the provided ray depending on pointer movement.
		 *
		 * @param[in]	camera			Camera on which the pointer movement is occurring.
		 * @param[in]	position		Position of the ray to calculate movement on.
		 * @param[in]	direction		Direction of the ray to calculate movement on. Must be normalized.
		 * @param[in]	pointerStart	Starting position of the pointer when movement started, in pixels relative to 
		 *								provided camera.
		 * @param[in]	pointerEnd		Current position of the pointer, in pixels relative to provided camera.
		 */
		float calcDelta(const SPtr<Camera>& camera, const Vector3& position, const Vector3& direction,
			const Vector2I& pointerStart, const Vector2I& pointerEnd);

		bool mFixedScale;
		UINT64 mLayer;

		Vector3 mPosition;
		Quaternion mRotation;
		Vector3 mScale;
		float mDistanceScale;

		Vector2I mStartPointerPos;
		Vector2I mCurrentPointerPos;
		State mState;
		bool mEnabled;

		mutable bool mTransformDirty;
		mutable Matrix4 mTransform;
		mutable Matrix4 mTransformInv;
	};

	/** @} */
}