//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Contains information about 3D object's position, rotation and scale, and provides methods to manipulate it.
	 */
	template<typename T>
	class TTransform
	{
	public:
		TTransform() = default;
		TTransform(const TVector3<T>& position, const TQuaternion<T>& rotation, const TVector3<T>& scale);

		/**	Sets the local position of the object. */
		void SetPosition(const TVector3<T>& position) { mPosition = position; }

		/**	Gets the local position of the object. */
		const TVector3<T>& GetPosition() const { return mPosition; }

		/**	Sets the local rotation of the object. */
		void SetRotation(const TQuaternion<T>& rotation) { mRotation = rotation; }

		/**	Gets the local rotation of the object. */
		const TQuaternion<T>& GetRotation() const { return mRotation; }

		/**	Sets the local scale of the object. */
		void SetScale(const TVector3<T>& scale) { mScale = scale; }

		/**	Gets the local scale of the object. */
		const TVector3<T>& GetScale() const { return mScale; }

		/**
		 * Converts the provided world position to a space relative to the provided parent, and sets it as the current
		 * transform's position.
		 */
		void SetWorldPosition(const TVector3<T>& position, const TTransform& parent);

		/**
		 * Converts the provided world rotation to a space relative to the provided parent, and sets it as the current
		 * transform's rotation.
		 */
		void SetWorldRotation(const TQuaternion<T>& rotation, const TTransform& parent);

		/**
		 * Converts the provided world scale to a space relative to the provided parent, and sets it as the current
		 * transform's scale.
		 */
		void SetWorldScale(const TVector3<T>& scale, const TTransform& parent);

		/** Builds the transform matrix from current translation, rotation and scale properties. */
		TMatrix4<T> GetMatrix() const;

		/** Builds the inverse transform matrix from current translation, rotation and scale properties. */
		TMatrix4<T> GetInvMatrix() const;

		/**
		 * Makes the current transform relative to the provided transform. In another words, converts from a world
		 * coordinate system to one local to the provided transform.
		 */
		void MakeLocal(const TTransform& parent);

		/**
		 * Makes the current transform absolute. In another words, converts from a local coordinate system relative to
		 * the provided transform, to a world coordinate system.
		 */
		void MakeWorld(const TTransform& parent);

		/**
		 * Orients the object so it is looking at the provided @p location (world space) where @p up is used for
		 * determining the location of the object's Y axis.
		 */
		void LookAt(const TVector3<T>& location, const TVector3<T>& up = TVector3<T>::kUnitY);

		/**	Moves the object's position by the vector offset provided along world axes. */
		void Move(const TVector3<T>& vec);

		/**	Moves the object's position by the vector offset provided along it's own axes (relative to orientation). */
		void MoveRelative(const TVector3<T>& vec);

		/**
		 * Gets the negative Z (forward) axis of the object.
		 *
		 * @return	Forward axis of the object.
		 */
		TVector3<T> GetForward() const { return GetRotation().Rotate(-TVector3<T>::kUnitZ); }

		/**
		 * Gets the Y (up) axis of the object.
		 *
		 * @return	Up axis of the object.
		 */
		TVector3<T> GetUp() const { return GetRotation().Rotate(TVector3<T>::kUnitY); }

		/**
		 * Gets the X (right) axis of the object.
		 *
		 * @return	Right axis of the object.
		 */
		TVector3<T> GetRight() const { return GetRotation().Rotate(TVector3<T>::kUnitX); }

		/**
		 * Rotates the game object so it's forward axis faces the provided direction.
		 *
		 * @param[in]	forwardDir	The forward direction to face.
		 *
		 * @note	Local forward axis is considered to be negative Z.
		 */
		void SetForward(const TVector3<T>& forwardDir);

		/**	Rotate the object around an arbitrary axis. */
		void Rotate(const TVector3<T>& axis, const TRadian<T>& angle);

		/**	Rotate the object around an arbitrary axis using a Quaternion. */
		void Rotate(const TQuaternion<T>& q);

		/**
		 * Rotates around local Z axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Roll(const TRadian<T>& angle);

		/**
		 * Rotates around Y axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Yaw(const TRadian<T>& angle);

		/**
		 * Rotates around X axis
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Pitch(const TRadian<T>& angle);

		static TTransform kIdentity;

	private:
		TVector3<T> mPosition = TVector3<T>::kZero;
		TQuaternion<T> mRotation = TQuaternion<T>::kIdentity;
		TVector3<T> mScale = TVector3<T>::kOne;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		template <class U> friend struct RTTIPlainType;
	};

	template<> TTransform<float> TTransform<float>::kIdentity;
	template<> TTransform<double> TTransform<double>::kIdentity;

	extern template class TTransform<float>;
	extern template class TTransform<double>;

	/** @} */
} // namespace b3d
