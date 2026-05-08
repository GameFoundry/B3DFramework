//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUISpace.h"

#include "B3DGUIOptions.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
{
	class B3D_EXPORT GUIFixedSpaceRTTI : public TRTTIType<GUIFixedSpace, GUIElement, GUIFixedSpaceRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIFixedSpace";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIFixedSpace; }

		TShared<IReflectable> NewRttiObject() { return nullptr; }
	};

	class B3D_EXPORT GUIFlexibleSpaceRTTI : public TRTTIType<GUIFlexibleSpace, GUIElement, GUIFlexibleSpaceRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIFlexibleSpace";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIFlexibleSpace; }

		TShared<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace b3d

/** @} */
/** @endcond */

GUIFixedSpace::GUIFixedSpace(GUILogicalUnit size)
	: GUIElement(GUISizeConstraints::Create(GUIOptions(GUIOption::FixedWidth(size), GUIOption::FixedHeight(size)))), mSize(size)
{}

GUIConstrainedSizeRange GUIFixedSpace::CalculateConstrainedSizeRange() const
{
	GUIConstrainedSizeRange range;
	range.Optimal = CalculateUnconstrainedOptimalSize();
	range.Minimum = range.Optimal;
	range.Maximum = range.Optimal;

	return range;
}

GUIFixedSpace* GUIFixedSpace::Create(GUILogicalUnit size)
{
	return B3DNew<GUIFixedSpace>(size);
}

RTTIType* GUIFixedSpace::GetRttiStatic()
{
	return GUIFixedSpaceRTTI::Instance();
}

RTTIType* GUIFixedSpace::GetRtti() const
{
	return GetRttiStatic();
}

GUIFlexibleSpace::GUIFlexibleSpace()
	:GUIElement(GUISizeConstraints::Create(GUIOptions(GUIOption::ExpandingWidth(), GUIOption::ExpandingHeight())))
{
	
}

GUIConstrainedSizeRange GUIFlexibleSpace::CalculateConstrainedSizeRange() const
{
	GUIConstrainedSizeRange range;
	range.Optimal = CalculateUnconstrainedOptimalSize();
	range.Minimum = range.Optimal;
	range.Maximum = range.Optimal;

	return range;
}

GUIFlexibleSpace* GUIFlexibleSpace::Create()
{
	return B3DNew<GUIFlexibleSpace>();
}

RTTIType* GUIFlexibleSpace::GetRttiStatic()
{
	return GUIFlexibleSpaceRTTI::Instance();
}

RTTIType* GUIFlexibleSpace::GetRtti() const
{
	return GetRttiStatic();
}
