//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DSize2.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DSize2.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DSize2.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DSize2.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_float_ : public TScriptTypeDefinition<ScriptTSize2_float_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<float>")

		static MonoObject* Box(const TSize2<float>& value);
		static TSize2<float> Unbox(MonoObject* value);

	private:
		ScriptTSize2_float_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_double_ : public TScriptTypeDefinition<ScriptTSize2_double_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<double>")

		static MonoObject* Box(const TSize2<double>& value);
		static TSize2<double> Unbox(MonoObject* value);

	private:
		ScriptTSize2_double_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_uint32_t_ : public TScriptTypeDefinition<ScriptTSize2_uint32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<int>")

		static MonoObject* Box(const TSize2<uint32_t>& value);
		static TSize2<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptTSize2_uint32_t_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_int32_t_ : public TScriptTypeDefinition<ScriptTSize2_int32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<int>")

		static MonoObject* Box(const TSize2<int32_t>& value);
		static TSize2<int32_t> Unbox(MonoObject* value);

	private:
		ScriptTSize2_int32_t_();

	};

	struct __TSize2_TUnitValue_int32_t__LogicalPixel__Interop
	{
		TUnitValue<int32_t, LogicalPixel> Width;
		TUnitValue<int32_t, LogicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_TUnitValue_int32_t__LogicalPixel__ : public TScriptTypeDefinition<ScriptTSize2_TUnitValue_int32_t__LogicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<b3d::TUnitValue<int,b3d::LogicalPixel>>")

		static MonoObject* Box(const __TSize2_TUnitValue_int32_t__LogicalPixel__Interop& value);
		static __TSize2_TUnitValue_int32_t__LogicalPixel__Interop Unbox(MonoObject* value);
		static TSize2<TUnitValue<int32_t, LogicalPixel>> FromInterop(const __TSize2_TUnitValue_int32_t__LogicalPixel__Interop& value);
		static __TSize2_TUnitValue_int32_t__LogicalPixel__Interop ToInterop(const TSize2<TUnitValue<int32_t, LogicalPixel>>& value);

	private:
		ScriptTSize2_TUnitValue_int32_t__LogicalPixel__();

	};

	struct __TSize2_TUnitValue_float__LogicalPixel__Interop
	{
		TUnitValue<float, LogicalPixel> Width;
		TUnitValue<float, LogicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_TUnitValue_float__LogicalPixel__ : public TScriptTypeDefinition<ScriptTSize2_TUnitValue_float__LogicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<b3d::TUnitValue<float,b3d::LogicalPixel>>")

		static MonoObject* Box(const __TSize2_TUnitValue_float__LogicalPixel__Interop& value);
		static __TSize2_TUnitValue_float__LogicalPixel__Interop Unbox(MonoObject* value);
		static TSize2<TUnitValue<float, LogicalPixel>> FromInterop(const __TSize2_TUnitValue_float__LogicalPixel__Interop& value);
		static __TSize2_TUnitValue_float__LogicalPixel__Interop ToInterop(const TSize2<TUnitValue<float, LogicalPixel>>& value);

	private:
		ScriptTSize2_TUnitValue_float__LogicalPixel__();

	};

	struct __TSize2_TUnitValue_int32_t__PhysicalPixel__Interop
	{
		TUnitValue<int32_t, PhysicalPixel> Width;
		TUnitValue<int32_t, PhysicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__ : public TScriptTypeDefinition<ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<b3d::TUnitValue<int,b3d::PhysicalPixel>>")

		static MonoObject* Box(const __TSize2_TUnitValue_int32_t__PhysicalPixel__Interop& value);
		static __TSize2_TUnitValue_int32_t__PhysicalPixel__Interop Unbox(MonoObject* value);
		static TSize2<TUnitValue<int32_t, PhysicalPixel>> FromInterop(const __TSize2_TUnitValue_int32_t__PhysicalPixel__Interop& value);
		static __TSize2_TUnitValue_int32_t__PhysicalPixel__Interop ToInterop(const TSize2<TUnitValue<int32_t, PhysicalPixel>>& value);

	private:
		ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__();

	};

	struct __TSize2_TUnitValue_float__PhysicalPixel__Interop
	{
		TUnitValue<float, PhysicalPixel> Width;
		TUnitValue<float, PhysicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_TUnitValue_float__PhysicalPixel__ : public TScriptTypeDefinition<ScriptTSize2_TUnitValue_float__PhysicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<b3d::TUnitValue<float,b3d::PhysicalPixel>>")

		static MonoObject* Box(const __TSize2_TUnitValue_float__PhysicalPixel__Interop& value);
		static __TSize2_TUnitValue_float__PhysicalPixel__Interop Unbox(MonoObject* value);
		static TSize2<TUnitValue<float, PhysicalPixel>> FromInterop(const __TSize2_TUnitValue_float__PhysicalPixel__Interop& value);
		static __TSize2_TUnitValue_float__PhysicalPixel__Interop ToInterop(const TSize2<TUnitValue<float, PhysicalPixel>>& value);

	private:
		ScriptTSize2_TUnitValue_float__PhysicalPixel__();

	};
}
