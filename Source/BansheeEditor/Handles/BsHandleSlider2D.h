//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2018 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "Handles/BsHandleSlider.h"
#include "Math/BsVector2.h"

namespace bs
{
	/** @addtogroup Handles
	 *  @{
	 */

	/** Constraint that determines in which direction can a HandleSlider2D be moved. */
	enum class Slider2DConstraint
	{
		None, /**< Slider can be moved in both dimensions. */
		Horizontal, /**< Slider can only be moved horizontally. */
		Vertical /**< Slider can only be moved vertically. */
	};

	/**
	 * Handle slider that is positioned in screen-space, and reports 2D movement in screen space (in pixels). When setting
	 * the position the Z coordinate will be ignored, and XY coordinates will be interpreted as pixels relative to the
	 * camera its viewed through.
	 */
	class BS_ED_EXPORT HandleSlider2D : public HandleSlider
	{
	public:
		/**
		 * Constructs a new 2D slider.
		 *
		 * @param[in]	width		Width of the area of the slider that can be interacted with, in pixels.
		 * @param[in]	height		Height of the area of the slider that can be interacted with, in pixels.
		 * @param[in]	layer		Layer that allows filtering of which sliders are interacted with from a specific camera.
		 * @param[in]	constraint	Optional constraint that determines in which direction is the slider allowed to be
		 *							moved in.
		 */
		HandleSlider2D(UINT32 width, UINT32 height, UINT64 layer, Slider2DConstraint constraint = Slider2DConstraint::None);
		~HandleSlider2D();

		/** @copydoc HandleSlider::intersects */
		bool intersects(const Vector2I& screenPos, const Ray& ray, float& t) const override;

		/** @copydoc HandleSlider::handleInput */
		void handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta) override;

		/**
		 * Returns a delta value that is the result of dragging/sliding the pointer. This changes every frame and will be
		 * zero unless the slider is active. The value is in screen space (pixels).
		 */
		Vector2I getDelta() const { return mDelta; }

	protected:
		/** @copydoc HandleSlider::activate */
		void activate(const SPtr<Camera>& camera, const Vector2I& pointerPos) override;

		/** @copydoc HandleSlider::reset */
		void reset() override { mDelta = Vector2I::ZERO; }

		UINT32 mWidth, mHeight;
		Slider2DConstraint mConstraint;

		Vector2I mDelta;
		Vector2I mStartPosition;
	};

	/** @} */
}
