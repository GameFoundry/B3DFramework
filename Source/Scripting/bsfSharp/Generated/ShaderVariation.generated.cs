//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			get { return Internal_getParamNames(mCachedPtr); }
		}

		/// <summary>
		/// Returns the value of a signed integer parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public int GetInt(string name)
		{
			return Internal_getInt(mCachedPtr, name);
		}

		/// <summary>
		/// Returns the value of a unsigned integer parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public int GetUInt(string name)
		{
			return Internal_getUInt(mCachedPtr, name);
		}

		/// <summary>
		/// Returns the value of a float parameter with the specified name. Returns 0 if the parameter cannot be found.
		/// </summary>
		public float GetFloat(string name)
		{
			return Internal_getFloat(mCachedPtr, name);
		}

		/// <summary>
		/// Returns the value of a boolean parameter with the specified name. Returns false if the parameter cannot be found.
		/// </summary>
		public bool GetBool(string name)
		{
			return Internal_getBool(mCachedPtr, name);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetInt(string name, int value)
		{
			Internal_setInt(mCachedPtr, name, value);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetUInt(string name, int value)
		{
			Internal_setUInt(mCachedPtr, name, value);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetFloat(string name, float value)
		{
			Internal_setFloat(mCachedPtr, name, value);
		}

		/// <summary>
		/// Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name will be 
		/// overwritten.
		/// </summary>
		public void SetBool(string name, bool value)
		{
			Internal_setBool(mCachedPtr, name, value);
		}

		/// <summary>Removes a parameter with the specified name.</summary>
		public void RemoveParam(string paramName)
		{
			Internal_removeParam(mCachedPtr, paramName);
		}

		/// <summary>Checks if the variation has a parameter with the specified name.</summary>
		public bool HasParam(string paramName)
		{
			return Internal_hasParam(mCachedPtr, paramName);
		}

		/// <summary>Removes all parameters.</summary>
		public void ClearParams()
		{
			Internal_clearParams(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ShaderVariation(ShaderVariation managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getInt(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getUInt(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getFloat(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getBool(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInt(IntPtr thisPtr, string name, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUInt(IntPtr thisPtr, string name, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFloat(IntPtr thisPtr, string name, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setBool(IntPtr thisPtr, string name, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeParam(IntPtr thisPtr, string paramName);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_hasParam(IntPtr thisPtr, string paramName);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_clearParams(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string[] Internal_getParamNames(IntPtr thisPtr);
	}

	/** @} */
}
