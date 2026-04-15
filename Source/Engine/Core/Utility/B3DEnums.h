//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Platform
	 *  @{
	 */

	/**	Available cursor types. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Platform)) CursorType
	{
		Arrow,
		ArrowDrag,
		ArrowLeftRight,
		Wait,
		IBeam,
		SizeNESW,
		SizeNS,
		SizeNWSE,
		SizeWE,
		Deny,

		// Keep at the end
		Count
	};

	/** @} */

	/** @addtogroup 2D
	 *  @{
	 */

	/**	Type of scaling modes for GUI images. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) TextureScaleMode
	{
		StretchToFit, /**< Image will stretch non-uniformly in all dimensions in order to cover the assigned area fully. */
		ScaleToFit, /**< Image will scale uniformly until one dimension is aligned with the assigned area. Remaining dimension might have empty space. */
		CropToFit, /**< Image will scale uniformly until both dimensions are larger or aligned with the assigned area. Remaining dimension might be cropped. */
		RepeatToFit /**< Image will keep its original size, but will repeat in order to fill the assigned area. */
	};

	/** @} */

	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Types of GUI meshes that can be output by GUI elements. */
	enum class GUIMeshType
	{
		/** Mesh containing a set of vertices as 2D position followed by 2D UV coordinates. */
		Triangle,
		/** Mesh containing a set of vertices as 2D position followed by vertex indices. */
		Line
	};

	/** @} */

	/** @addtogroup Profiling
	 *  @{
	 */

	/**	Determines type of data to display on the profiler overlay. */
	enum class ProfilerOverlayType
	{
		/** Display CPU samples on the overlay. */
		CPUSamples,

		/** Display GPU samples on the overlay. */
		GPUSamples
	};

	/** @} */
} // namespace b3d
