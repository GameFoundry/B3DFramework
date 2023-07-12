//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsRectOffset.h"
#include "Math/BsMatrix4.h"

namespace bs
{
	/** @addtogroup VectorGraphics
	 *  @{
	 */

	// TODO - Doc
	struct VectorShapeSettings
	{
		Size2 Size = Size2::kZero;
		float Scale = 1.0f;
		Matrix4 Transform = Matrix4::kIdentity;
		RectOffset Scale9GridBorder;
		bool UseAntialiasing = false;
		float DevicePixelRatio = 1.0f;
	};

	// TODO
	class VectorPath
	{
		
	};

	// TODO - Doc
	class VectorGraphics
	{
	public:
		void Render(const VectorPath& path, const VectorShapeSettings& settings);
		
	};


	/** @} */

} // namespace bs
