//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DColorRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DSubmeshRTTI.h"
#include "VectorGraphics/B3DNVGVectorGraphics.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(NVGVertex, TID_NVGVertex)

	template<>
	struct RTTIPlainType<NVGRenderUniforms> : RTTIPlainTypeHelper<NVGRenderUniforms, TID_NVGRenderUniforms, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(NVGRenderUniforms& object, Processor& processor, u8 version)
		{
			processor(object.ScissorMatrix);
			processor(object.PaintMatrix);
			processor(object.InnerColor);
			processor(object.OuterColor);
			processor(object.ScissorExtents);
			processor(object.ScissorScale);
			processor(object.Extent);
			processor(object.Radius);
			processor(object.Feather);
			processor(object.StrokeMultiplier);
			processor(object.StrokeThreshold);
		}
	};

	template<>
	struct RTTIPlainType<NVGRenderCommand> : RTTIPlainTypeHelper<NVGRenderCommand, TID_NVGRenderCommand, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(NVGRenderCommand& object, Processor& processor, u8 version)
		{
			processor(object.Type);
			processor(object.BlendMode);
			processor(object.PrimaryPassUniforms);
			processor(object.SecondaryPassUniforms);
		}
	};

	template<>
	struct RTTIPlainType<render::NVGPathRenderData> : RTTIPlainTypeHelper<render::NVGPathRenderData, TID_NVGPathRenderData, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(render::NVGPathRenderData& object, Processor& processor, u8 version)
		{
			processor(object.Vertices);
			processor(object.Indices);
			processor(object.Submeshes);
			processor(object.RenderCommands);
		}
	};

	class B3D_EXPORT NVGVectorPathRenderableRTTI : public TRTTIType<render::NVGVectorPathRenderable, render::VectorPathRenderable, NVGVectorPathRenderableRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mRawRenderData, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "NVGVectorPathRenderable";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_NVGVectorPathRenderable;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<render::NVGVectorPathRenderable>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
