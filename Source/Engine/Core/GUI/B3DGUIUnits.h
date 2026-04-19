//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DUnitValue.h"
#include "Math/B3DSize2.h"
#include "Math/B3DVector2.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Location in 2D space in physical pixels. Physical pixels represent actual pixels as displayed on the output monitor. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true)) PhysicalPixel { };

	/**
	 * Location in 2D space in logical pixels. Logical pixels are defined at 1/96th of one logical inch. Logical pixels are transformed
	 * into physical pixels by scaling it by the display's DPI scale. If your display is set to 96 DPI, then one logical pixel equals one physical pixel.
	 */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true)) LogicalPixel { };

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TUnitValue<i32, LogicalPixel>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TUnitValue<float, LogicalPixel>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TUnitValue<i32, PhysicalPixel>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TUnitValue<float, PhysicalPixel>;

	template<> const TVector2<TUnitValue<i32, LogicalPixel>> TVector2<TUnitValue<i32, LogicalPixel>>::kZero{kZeroTag};
	template<> const TVector2<TUnitValue<float, LogicalPixel>> TVector2<TUnitValue<float, LogicalPixel>>::kZero{kZeroTag};
	template<> const TVector2<TUnitValue<i32, PhysicalPixel>> TVector2<TUnitValue<i32, PhysicalPixel>>::kZero{kZeroTag};
	template<> const TVector2<TUnitValue<float, PhysicalPixel>> TVector2<TUnitValue<float, PhysicalPixel>>::kZero{kZeroTag};

	template<> const TVector2<TUnitValue<i32, LogicalPixel>> TVector2<TUnitValue<i32, LogicalPixel>>::kOne{1, 1};
	template<> const TVector2<TUnitValue<float, LogicalPixel>> TVector2<TUnitValue<float, LogicalPixel>>::kOne{1.0f, 1.0f};
	template<> const TVector2<TUnitValue<i32, PhysicalPixel>> TVector2<TUnitValue<i32, PhysicalPixel>>::kOne{1, 1};
	template<> const TVector2<TUnitValue<float, PhysicalPixel>> TVector2<TUnitValue<float, PhysicalPixel>>::kOne{1.0f, 1.0f};

	template<> const TVector2<TUnitValue<i32, LogicalPixel>> TVector2<TUnitValue<i32, LogicalPixel>>::kUnitX{1, 0};
	template<> const TVector2<TUnitValue<float, LogicalPixel>> TVector2<TUnitValue<float, LogicalPixel>>::kUnitX{1.0f, 0.0f};
	template<> const TVector2<TUnitValue<i32, PhysicalPixel>> TVector2<TUnitValue<i32, PhysicalPixel>>::kUnitX{1, 0};
	template<> const TVector2<TUnitValue<float, PhysicalPixel>> TVector2<TUnitValue<float, PhysicalPixel>>::kUnitX{1.0f, 0.0f};

	template<> const TVector2<TUnitValue<i32, LogicalPixel>> TVector2<TUnitValue<i32, LogicalPixel>>::kUnitY{0, 1};
	template<> const TVector2<TUnitValue<float, LogicalPixel>> TVector2<TUnitValue<float, LogicalPixel>>::kUnitY{0.0f, 1.0f};
	template<> const TVector2<TUnitValue<i32, PhysicalPixel>> TVector2<TUnitValue<i32, PhysicalPixel>>::kUnitY{0, 1};
	template<> const TVector2<TUnitValue<float, PhysicalPixel>> TVector2<TUnitValue<float, PhysicalPixel>>::kUnitY{0.0f, 1.0f};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TVector2<TUnitValue<i32, LogicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TVector2<TUnitValue<float, LogicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TVector2<TUnitValue<i32, PhysicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TVector2<TUnitValue<float, PhysicalPixel>>;
	
	template<> const TSize2<TUnitValue<i32, LogicalPixel>> TSize2<TUnitValue<i32, LogicalPixel>>::kZero{kZeroTag};
	template<> const TSize2<TUnitValue<float, LogicalPixel>> TSize2<TUnitValue<float, LogicalPixel>>::kZero{kZeroTag};
	template<> const TSize2<TUnitValue<i32, PhysicalPixel>> TSize2<TUnitValue<i32, PhysicalPixel>>::kZero{kZeroTag};
	template<> const TSize2<TUnitValue<float, PhysicalPixel>> TSize2<TUnitValue<float, PhysicalPixel>>::kZero{kZeroTag};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TSize2<TUnitValue<i32, LogicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TSize2<TUnitValue<float, LogicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TSize2<TUnitValue<i32, PhysicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TSize2<TUnitValue<float, PhysicalPixel>>;

	template<> const TArea2<TUnitValue<i32, LogicalPixel>> TArea2<TUnitValue<i32, LogicalPixel>>::kEmpty;
	template<> const TArea2<TUnitValue<float, LogicalPixel>> TArea2<TUnitValue<float, LogicalPixel>>::kEmpty;
	template<> const TArea2<TUnitValue<i32, PhysicalPixel>> TArea2<TUnitValue<i32, PhysicalPixel>>::kEmpty;
	template<> const TArea2<TUnitValue<float, PhysicalPixel>> TArea2<TUnitValue<float, PhysicalPixel>>::kEmpty;

	using GUIPhysicalUnit = TUnitValue<i32, PhysicalPixel>;
	using GUIPhysicalUnitF = TUnitValue<float, PhysicalPixel>;

	using GUILogicalUnit = TUnitValue<i32, LogicalPixel>;
	using GUILogicalUnitF = TUnitValue<float, LogicalPixel>;

	using GUIPhysicalPoint = TVector2<GUIPhysicalUnit>;
	using GUIPhysicalPointF = TVector2<GUIPhysicalUnitF>;

	using GUILogicalPoint = TVector2<GUILogicalUnit>;
	using GUILogicalPointF = TVector2<GUILogicalUnitF>;

	using GUIPhysicalSize = TSize2<GUIPhysicalUnit>;
	using GUIPhysicalSizeF = TSize2<GUIPhysicalUnitF>;

	using GUILogicalSize = TSize2<GUILogicalUnit>;
	using GUILogicalSizeF = TSize2<GUILogicalUnitF>;

	using GUIPhysicalArea = TArea2<GUIPhysicalUnit>;
	using GUIPhysicalAreaF = TArea2<GUIPhysicalUnitF>;

	using GUILogicalArea = TArea2<GUILogicalUnit>;
	using GUILogicalAreaF = TArea2<GUILogicalUnitF>;

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TArea2<TUnitValue<i32, LogicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TArea2<TUnitValue<float, LogicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TArea2<TUnitValue<i32, PhysicalPixel>>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) TArea2<TUnitValue<float, PhysicalPixel>>;

	template <typename T>
	struct B3DIsUnitValue<TUnitValue<T, PhysicalPixel>> : std::true_type
	{
		using UnderlyingType = T;
	};

	template <typename T>
	struct B3DIsUnitValue<TUnitValue<T, LogicalPixel>> : std::true_type
	{
		using UnderlyingType = T;
	};

	/** @} */
} // namespace b3d
