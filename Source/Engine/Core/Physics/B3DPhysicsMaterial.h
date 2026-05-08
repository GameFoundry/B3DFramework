//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/**
	 * Material that controls how two physical objects interact with each other. Materials of both objects are used during
	 * their interaction and their combined values are used.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) PhysicsMaterial : public Resource
	{
	public:
		virtual ~PhysicsMaterial() = default;

		/**
		 * Controls friction when two in-contact objects are not moving lateral to each other (for example how difficult
		 * it is to get an object moving from a static state while it is in contact with other object(s)).
		 */
		B3D_SCRIPT_EXPORT(ExportName(StaticFriction), Property(Setter))
		virtual void SetStaticFriction(float value) = 0;

		/** @copydoc SetStaticFriction() */
		B3D_SCRIPT_EXPORT(ExportName(StaticFriction), Property(Getter))
		virtual float GetStaticFriction() const = 0;

		/**
		 * Controls friction when two in-contact objects are moving lateral to each other (for example how quickly does an
		 * object slow down when sliding along another object).
		 */
		B3D_SCRIPT_EXPORT(ExportName(DynamicFriction), Property(Setter))
		virtual void SetDynamicFriction(float value) = 0;

		/** @copydoc SetDynamicFriction() */
		B3D_SCRIPT_EXPORT(ExportName(DynamicFriction), Property(Getter))
		virtual float GetDynamicFriction() const = 0;

		/**
		 * Controls "bounciness" of an object during a collision. Value of 1 means the collision is elastic, and value of 0
		 * means the value is inelastic. Must be in [0, 1] range.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Restitution), Property(Setter))
		virtual void SetRestitutionCoefficient(float value) = 0;

		/** @copydoc SetRestitutionCoefficient() */
		B3D_SCRIPT_EXPORT(ExportName(Restitution), Property(Getter))
		virtual float GetRestitutionCoefficient() const = 0;

		/**
		 * Creates a new physics material.
		 *
		 * @param	staticFriction	Controls friction when two in-contact objects are not moving lateral to each other
		 *								(for example how difficult is to get an object moving from a static state while it
		 *								is in contact other object(s)).
		 * @param	dynamicFriction	Sets dynamic friction of the material. Controls friction when two in-contact objects
		 *								are moving lateral to each other (for example how quickly does an object slow down
		 *								when sliding along another object).
		 * @param	restitution		Controls "bounciness" of an object during a collision. Value of 1 means the
		 *								collision is elastic, and value of 0 means the value is inelastic. Must be in
		 *								[0, 1] range.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(PhysicsMaterial))
		static HPhysicsMaterial Create(float staticFriction = 0.0f, float dynamicFriction = 0.0f, float restitution = 0.0f);

		/** @name Internal
		 *  @{
		 */

		/**
		 * @copydoc create()
		 *
		 * For internal use. Requires manual initialization after creation.
		 */
		static TShared<PhysicsMaterial> CreatePtrInternal(float staticFriction = 0.0f, float dynamicFriction = 0.0f, float restitution = 0.0f);

		/** @} */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PhysicsMaterialRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
