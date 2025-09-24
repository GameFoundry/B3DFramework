//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DArea2.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DArea2.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Math/B3DArea2.h"
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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_int32_t__int32_t_ : public TScriptTypeDefinition<ScriptTArea2_int32_t__int32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<int,int>")

		static MonoObject* Box(const TArea2<int32_t, int32_t>& value);
		static TArea2<int32_t, int32_t> Unbox(MonoObject* value);

	private:
		ScriptTArea2_int32_t__int32_t_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_int32_t__uint32_t_ : public TScriptTypeDefinition<ScriptTArea2_int32_t__uint32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<int,int>")

		static MonoObject* Box(const TArea2<int32_t, uint32_t>& value);
		static TArea2<int32_t, uint32_t> Unbox(MonoObject* value);

	private:
		ScriptTArea2_int32_t__uint32_t_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_float__float_ : public TScriptTypeDefinition<ScriptTArea2_float__float_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<float,float>")

		static MonoObject* Box(const TArea2<float, float>& value);
		static TArea2<float, float> Unbox(MonoObject* value);

	private:
		ScriptTArea2_float__float_();

	};

	struct __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop
	{
		TUnitValue<int32_t, LogicalPixel> X;
		TUnitValue<int32_t, LogicalPixel> Y;
		TUnitValue<int32_t, LogicalPixel> Width;
		TUnitValue<int32_t, LogicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__ : public TScriptTypeDefinition<ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<b3d::TUnitValue<int,b3d::LogicalPixel>,b3d::TUnitValue<int,b3d::LogicalPixel>>")

		static MonoObject* Box(const __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop& value);
		static __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop Unbox(MonoObject* value);
		static TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>> FromInterop(const __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop& value);
		static __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop ToInterop(const TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>>& value);

	private:
		ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__();

	};

	struct __TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop
	{
		TUnitValue<float, LogicalPixel> X;
		TUnitValue<float, LogicalPixel> Y;
		TUnitValue<float, LogicalPixel> Width;
		TUnitValue<float, LogicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__ : public TScriptTypeDefinition<ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<b3d::TUnitValue<float,b3d::LogicalPixel>,b3d::TUnitValue<float,b3d::LogicalPixel>>")

		static MonoObject* Box(const __TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop& value);
		static __TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop Unbox(MonoObject* value);
		static TArea2<TUnitValue<float, LogicalPixel>, TUnitValue<float, LogicalPixel>> FromInterop(const __TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop& value);
		static __TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop ToInterop(const TArea2<TUnitValue<float, LogicalPixel>, TUnitValue<float, LogicalPixel>>& value);

	private:
		ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__();

	};

	struct __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop
	{
		TUnitValue<int32_t, PhysicalPixel> X;
		TUnitValue<int32_t, PhysicalPixel> Y;
		TUnitValue<int32_t, PhysicalPixel> Width;
		TUnitValue<int32_t, PhysicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__ : public TScriptTypeDefinition<ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<b3d::TUnitValue<int,b3d::PhysicalPixel>,b3d::TUnitValue<int,b3d::PhysicalPixel>>")

		static MonoObject* Box(const __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop& value);
		static __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop Unbox(MonoObject* value);
		static TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> FromInterop(const __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop& value);
		static __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop ToInterop(const TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>>& value);

	private:
		ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__();

	};

	struct __TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop
	{
		TUnitValue<float, PhysicalPixel> X;
		TUnitValue<float, PhysicalPixel> Y;
		TUnitValue<float, PhysicalPixel> Width;
		TUnitValue<float, PhysicalPixel> Height;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__ : public TScriptTypeDefinition<ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<b3d::TUnitValue<float,b3d::PhysicalPixel>,b3d::TUnitValue<float,b3d::PhysicalPixel>>")

		static MonoObject* Box(const __TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop& value);
		static __TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop Unbox(MonoObject* value);
		static TArea2<TUnitValue<float, PhysicalPixel>, TUnitValue<float, PhysicalPixel>> FromInterop(const __TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop& value);
		static __TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop ToInterop(const TArea2<TUnitValue<float, PhysicalPixel>, TUnitValue<float, PhysicalPixel>>& value);

	private:
		ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__();

	};
}
