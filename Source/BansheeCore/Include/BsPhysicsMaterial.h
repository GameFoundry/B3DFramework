//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** Physics material that controls how objects react when they collide. */
	class BS_CORE_EXPORT PhysicsMaterial : public Resource
	{
	public:
		virtual ~PhysicsMaterial() { }

		/** 
		  * Sets static friction of the material. Controls friction when two in-contact objects are not moving lateral to
		  * each other (for example how difficult is to get an object moving from a static state while it is in contact 
		  * other object(s)).
		  */
		virtual void setStaticFriction(float value) = 0;

		/** Gets static friction of the material. */
		virtual float getStaticFriction() const = 0;

		/**
		 * Sets dynamic friction of the material. Controls friction when two in-contact objects are moving lateral to each
		 * other (for example how quickly does an object slow down when sliding along another object).
		 */
		virtual void setDynamicFriction(float value) = 0;

		/** Gets dynamic friction of the material .*/
		virtual float getDynamicFriction() const = 0;

		/** 
		 * Sets restitution coefficient of the material. Controls "bounciness" of an object during a collision. Value of 1
		 * means the collision is elastic, and value of 0 means the value is inelastic. Must be in [0, 1] range.
		 */
		virtual void setRestitutionCoefficient(float value) = 0;

		/** Gets restitution coefficient of the material. */
		virtual float getRestitutionCoefficient() const = 0;

		/** 
		 * Creates a new physics material. 
		 *
		 * @param[in]	staticFriction	Controls friction when two in-contact objects are not moving lateral to each other
		 *								(for example how difficult is to get an object moving from a static state while it
		 *								is in contact other object(s)).
		 * @param[in]	dynamicFriction	Sets dynamic friction of the material. Controls friction when two in-contact objects
		 *								are moving lateral to each other (for example how quickly does an object slow down
		 *								when sliding along another object).
		 * @param[in]	restitution		Controls "bounciness" of an object during a collision. Value of 1 means the
		 *								collision is elastic, and value of 0 means the value is inelastic. Must be in
		 *								[0, 1] range.
		 */
		static HPhysicsMaterial create(float staticFriction = 0.0f, float dynamicFriction = 0.0f, float restitution = 0.0f);

		/** @name Internal
		 *  @{
		 */

		/** 
		 * @copydoc create()
		 *
		 * For internal use. Requires manual initialization after creation.
		 */
		static SPtr<PhysicsMaterial> _createPtr(float staticFriction = 0.0f, float dynamicFriction = 0.0f, 
			float restitution = 0.0f);

		/** @} */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PhysicsMaterialRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}