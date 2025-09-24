//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Contains information about a single variation of a Shader. Each variation can have a separate set of #defines that 
	/// control shader compilation.
	/// </summary>
	[ShowInInspector]
	public partial class ShaderVariationParameters : ScriptObject
	{
		private ShaderVariationParameters(bool __dummy0) { }

		public ShaderVariationParameters()
		{
			Internal_ShaderVariationParameters(this);
		}

		/// <summary>Returns a list of names of all registered parameters.</summary>
		[NativeWrapper]
		public string[] ParamNames
		{
			get { return Internal_GetParameters(mCachedPtr); }
		}

		/// <summary>
		/// Returns the value of a signed integer parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public int GetI32(string name)
		{
			return Internal_GetI32(mCachedPtr, name);
		}

		/// <summary>
		/// Returns the value of a unsigned integer parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public int GetUI32(string name)
		{
			return Internal_GetUI32(mCachedPtr, name);
		}

		/// <summary>
		/// Returns the value of a float parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public float GetFloat(string name)
		{
			return Internal_GetFloat(mCachedPtr, name);
		}

		/// <summary>
		/// Returns the value of a boolean parameter with the specified name. Returns false if the parameter cannot be found.
		/// </summary>
		public bool GetBool(string name)
		{
			return Internal_GetBool(mCachedPtr, name);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetI32(string name, int value)
		{
			Internal_SetI32(mCachedPtr, name, value);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetU32(string name, int value)
		{
			Internal_SetU32(mCachedPtr, name, value);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetFloat(string name, float value)
		{
			Internal_SetFloat(mCachedPtr, name, value);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetBool(string name, bool value)
		{
			Internal_SetBool(mCachedPtr, name, value);
		}

		/// <summary>Removes a parameter with the specified name.</summary>
		public void RemoveParameter(string parameter)
		{
			Internal_RemoveParameter(mCachedPtr, parameter);
		}

		/// <summary>Checks if the variation has a parameter with the specified name.</summary>
		public bool HasParameter(string paramName)
		{
			return Internal_HasParameter(mCachedPtr, paramName);
		}

		/// <summary>Removes all parameters.</summary>
		public void ClearParameters()
		{
			Internal_ClearParameters(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ShaderVariationParameters(ShaderVariationParameters managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetI32(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetUI32(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFloat(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetBool(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetI32(IntPtr thisPtr, string name, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetU32(IntPtr thisPtr, string name, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFloat(IntPtr thisPtr, string name, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBool(IntPtr thisPtr, string name, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveParameter(IntPtr thisPtr, string parameter);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasParameter(IntPtr thisPtr, string paramName);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClearParameters(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string[] Internal_GetParameters(IntPtr thisPtr);
	}

	/** @} */
}
