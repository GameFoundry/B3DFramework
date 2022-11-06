//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Contains information about a single variation of a Shader. Each variation can have a separate set of #defines that 
	/// control shader compilation.
	/// </summary>
	[ShowInInspector]
	public partial class ShaderVariation : ScriptObject
	{
		private ShaderVariation(bool __dummy0) { }

		public ShaderVariation()
		{
			Internal_ShaderVariation(this);
		}

		/// <summary>Returns a list of names of all registered parameters.</summary>
		[NativeWrapper]
		public string[] ParamNames
		{
			get { return Internal_GetParamNames(mCachedPtr); }
		}

		/// <summary>
		/// Returns the value of a signed integer parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public int GetInt(string name)
		{
			return Internal_GetInt(mCachedPtr, name);
		}

		/// <summary>
		/// Returns the value of a unsigned integer parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public int GetUInt(string name)
		{
			return Internal_GetUInt(mCachedPtr, name);
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
		public void SetInt(string name, int value)
		{
			Internal_SetInt(mCachedPtr, name, value);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetUInt(string name, int value)
		{
			Internal_SetUInt(mCachedPtr, name, value);
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
		public void RemoveParam(string paramName)
		{
			Internal_RemoveParam(mCachedPtr, paramName);
		}

		/// <summary>Checks if the variation has a parameter with the specified name.</summary>
		public bool HasParam(string paramName)
		{
			return Internal_HasParam(mCachedPtr, paramName);
		}

		/// <summary>Removes all parameters.</summary>
		public void ClearParams()
		{
			Internal_ClearParams(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ShaderVariation(ShaderVariation managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetInt(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetUInt(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFloat(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetBool(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInt(IntPtr thisPtr, string name, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUInt(IntPtr thisPtr, string name, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFloat(IntPtr thisPtr, string name, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBool(IntPtr thisPtr, string name, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveParam(IntPtr thisPtr, string paramName);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasParam(IntPtr thisPtr, string paramName);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClearParams(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string[] Internal_GetParamNames(IntPtr thisPtr);
	}

	/** @} */
}
