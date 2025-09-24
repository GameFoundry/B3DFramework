//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_float_ : public TScriptTypeDefinition<ScriptTVector2_float_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<float>")

		static MonoObject* Box(const TVector2<float>& value);
		static TVector2<float> Unbox(MonoObject* value);

	private:
		ScriptTVector2_float_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_double_ : public TScriptTypeDefinition<ScriptTVector2_double_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<double>")

		static MonoObject* Box(const TVector2<double>& value);
		static TVector2<double> Unbox(MonoObject* value);

	private:
		ScriptTVector2_double_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_int32_t_ : public TScriptTypeDefinition<ScriptTVector2_int32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<int>")

		static MonoObject* Box(const TVector2<int32_t>& value);
		static TVector2<int32_t> Unbox(MonoObject* value);

	private:
		ScriptTVector2_int32_t_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_uint32_t_ : public TScriptTypeDefinition<ScriptTVector2_uint32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<int>")

		static MonoObject* Box(const TVector2<uint32_t>& value);
		static TVector2<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptTVector2_uint32_t_();

	};

	struct __TVector2_TUnitValue_int32_t__LogicalPixel__Interop
	{
		TUnitValue<int32_t, LogicalPixel> X;
		TUnitValue<int32_t, LogicalPixel> Y;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_TUnitValue_int32_t__LogicalPixel__ : public TScriptTypeDefinition<ScriptTVector2_TUnitValue_int32_t__LogicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<b3d::TUnitValue<int,b3d::LogicalPixel>>")

		static MonoObject* Box(const __TVector2_TUnitValue_int32_t__LogicalPixel__Interop& value);
		static __TVector2_TUnitValue_int32_t__LogicalPixel__Interop Unbox(MonoObject* value);
		static TVector2<TUnitValue<int32_t, LogicalPixel>> FromInterop(const __TVector2_TUnitValue_int32_t__LogicalPixel__Interop& value);
		static __TVector2_TUnitValue_int32_t__LogicalPixel__Interop ToInterop(const TVector2<TUnitValue<int32_t, LogicalPixel>>& value);

	private:
		ScriptTVector2_TUnitValue_int32_t__LogicalPixel__();

	};

	struct __TVector2_TUnitValue_float__LogicalPixel__Interop
	{
		TUnitValue<float, LogicalPixel> X;
		TUnitValue<float, LogicalPixel> Y;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_TUnitValue_float__LogicalPixel__ : public TScriptTypeDefinition<ScriptTVector2_TUnitValue_float__LogicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<b3d::TUnitValue<float,b3d::LogicalPixel>>")

		static MonoObject* Box(const __TVector2_TUnitValue_float__LogicalPixel__Interop& value);
		static __TVector2_TUnitValue_float__LogicalPixel__Interop Unbox(MonoObject* value);
		static TVector2<TUnitValue<float, LogicalPixel>> FromInterop(const __TVector2_TUnitValue_float__LogicalPixel__Interop& value);
		static __TVector2_TUnitValue_float__LogicalPixel__Interop ToInterop(const TVector2<TUnitValue<float, LogicalPixel>>& value);

	private:
		ScriptTVector2_TUnitValue_float__LogicalPixel__();

	};

	struct __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop
	{
		TUnitValue<int32_t, PhysicalPixel> X;
		TUnitValue<int32_t, PhysicalPixel> Y;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__ : public TScriptTypeDefinition<ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<b3d::TUnitValue<int,b3d::PhysicalPixel>>")

		static MonoObject* Box(const __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop& value);
		static __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop Unbox(MonoObject* value);
		static TVector2<TUnitValue<int32_t, PhysicalPixel>> FromInterop(const __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop& value);
		static __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop ToInterop(const TVector2<TUnitValue<int32_t, PhysicalPixel>>& value);

	private:
		ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__();

	};

	struct __TVector2_TUnitValue_float__PhysicalPixel__Interop
	{
		TUnitValue<float, PhysicalPixel> X;
		TUnitValue<float, PhysicalPixel> Y;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTVector2_TUnitValue_float__PhysicalPixel__ : public TScriptTypeDefinition<ScriptTVector2_TUnitValue_float__PhysicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TVector2<b3d::TUnitValue<float,b3d::PhysicalPixel>>")

		static MonoObject* Box(const __TVector2_TUnitValue_float__PhysicalPixel__Interop& value);
		static __TVector2_TUnitValue_float__PhysicalPixel__Interop Unbox(MonoObject* value);
		static TVector2<TUnitValue<float, PhysicalPixel>> FromInterop(const __TVector2_TUnitValue_float__PhysicalPixel__Interop& value);
		static __TVector2_TUnitValue_float__PhysicalPixel__Interop ToInterop(const TVector2<TUnitValue<float, PhysicalPixel>>& value);

	private:
		ScriptTVector2_TUnitValue_float__PhysicalPixel__();

	};
}
