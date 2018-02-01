//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "Handles/BsHandleSlider.h"
#include "Math/BsSphere.h"

namespace bs
{
	/** @addtogroup Handles
	 *  @{
	 */

	/**
	 * Handle slider represented by a sphere collider. The slider doesn't allow dragging (it doesn't report a delta value
	 * like other sliders) but can be used for picking (selection) purposes.
	 */
	class BS_ED_EXPORT HandleSliderSphere : public HandleSlider
	{
	public:
		/**
		 * Constructs a new sphere slider.
		 *
		 * @param[in]	radius		Radius of the collider sphere.
		 * @param[in]	fixedScale	If true the handle slider will always try to maintain the same visible area in the
		 *							viewport regardless of distance from camera.
		 * @param[in]	layer		Layer that allows filtering of which sliders are interacted with from a specific camera.
		 */
		HandleSliderSphere(float radius, bool fixedScale, UINT64 layer);
		~HandleSliderSphere();

		/** @copydoc	HandleSlider::intersects */
		bool intersects(const Vector2I& screenPos, const Ray& ray, float& t) const override;

		/** @copydoc	HandleSlider::handleInput */
		void handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta) override;

	protected:
		Sphere mSphereCollider;
	};

	/** @} */
}
