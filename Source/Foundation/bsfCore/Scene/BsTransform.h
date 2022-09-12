//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Math/BsMatrix4.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	/** @addtogroup Scene
	 *  @{
	 */

	/**
	 * Contains information about 3D object's position, rotation and scale, and provides methods to manipulate it.
	 */
	class BS_CORE_EXPORT Transform : public IReflectable
	{
	public:
		Transform() = default;
		Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale);

		/**	Sets the local position of the object. */
		void SetPosition(const Vector3& position) { mPosition = position; }

		/**	Gets the local position of the object. */
		const Vector3& GetPosition() const { return mPosition; }

		/** Shorthand for getPosition(). */
		const Vector3& Pos() const { return mPosition; }

		/**	Sets the local rotation of the object. */
		void SetRotation(const Quaternion& rotation) { mRotation = rotation; }

		/**	Gets the local rotation of the object. */
		const Quaternion& GetRotation() const { return mRotation; }

		/** Shorthand for getRotation(). */
		const Quaternion& Rot() const { return mRotation; }

		/**	Sets the local scale of the object. */
		void SetScale(const Vector3& scale) { mScale = scale; }

		/**	Gets the local scale of the object. */
		const Vector3& GetScale() const { return mScale; }

		/** Shorthand for getScale(). */
		const Vector3& Scl() const { return mScale; }

		/**
		 * Converts the provided world position to a space relative to the provided parent, and sets it as the current
		 * transform's position.
		 */
		void SetWorldPosition(const Vector3& position, const Transform& parent);

		/**
		 * Converts the provided world rotation to a space relative to the provided parent, and sets it as the current
		 * transform's rotation.
		 */
		void SetWorldRotation(const Quaternion& rotation, const Transform& parent);

		/**
		 * Converts the provided world scale to a space relative to the provided parent, and sets it as the current
		 * transform's scale.
		 */
		void SetWorldScale(const Vector3& scale, const Transform& parent);

		/** Builds the transform matrix from current translation, rotation and scale properties. */
		Matrix4 GetMatrix() const;

		/** Builds the inverse transform matrix from current translation, rotation and scale properties. */
		Matrix4 GetInvMatrix() const;

		/**
		 * Makes the current transform relative to the provided transform. In another words, converts from a world
		 * coordinate system to one local to the provided transform.
		 */
		void MakeLocal(const Transform& parent);

		/**
		 * Makes the current transform absolute. In another words, converts from a local coordinate system relative to
		 * the provided transform, to a world coordinate system.
		 */
		void MakeWorld(const Transform& parent);

		/**
		 * Orients the object so it is looking at the provided @p location (world space) where @p up is used for
		 * determining the location of the object's Y axis.
		 */
		void LookAt(const Vector3& location, const Vector3& up = Vector3::UNIT_Y);

		/**	Moves the object's position by the vector offset provided along world axes. */
		void Move(const Vector3& vec);

		/**	Moves the object's position by the vector offset provided along it's own axes (relative to orientation). */
		void MoveRelative(const Vector3& vec);

		/**
		 * Gets the negative Z (forward) axis of the object.
		 *
		 * @return	Forward axis of the object.
		 */
		Vector3 GetForward() const { return getRotation().Rotate(-Vector3::UNIT_Z); }

		/**
		 * Gets the Y (up) axis of the object.
		 *
		 * @return	Up axis of the object.
		 */
		Vector3 GetUp() const { return getRotation().Rotate(Vector3::UNIT_Y); }

		/**
		 * Gets the X (right) axis of the object.
		 *
		 * @return	Right axis of the object.
		 */
		Vector3 GetRight() const { return getRotation().Rotate(Vector3::UNIT_X); }

		/**
		 * Rotates the game object so it's forward axis faces the provided direction.
		 *
		 * @param[in]	forwardDir	The forward direction to face.
		 *
		 * @note	Local forward axis is considered to be negative Z.
		 */
		void SetForward(const Vector3& forwardDir);

		/**	Rotate the object around an arbitrary axis. */
		void Rotate(const Vector3& axis, const Radian& angle);

		/**	Rotate the object around an arbitrary axis using a Quaternion. */
		void Rotate(const Quaternion& q);

		/**
		 * Rotates around local Z axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Roll(const Radian& angle);

		/**
		 * Rotates around Y axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Yaw(const Radian& angle);

		/**
		 * Rotates around X axis
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Pitch(const Radian& angle);

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void RttiEnumFields(P p)
		{
			p(mPosition);
			p(mRotation);
			p(mScale);
		}

		static Transform IDENTITY;
	private:
		Vector3 mPosition = Vector3::ZERO;
		Quaternion mRotation = Quaternion::IDENTITY;
		Vector3 mScale = Vector3::ONE;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TransformRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
