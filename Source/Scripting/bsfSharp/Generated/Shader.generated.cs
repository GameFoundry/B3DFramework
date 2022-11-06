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
	/// Contains definitions of GPU programs used for rendering, as well as a set of global parameters to control those 
	/// programs.
	/// </summary>
	[ShowInInspector]
	public partial class Shader : Resource
	{
		private Shader(bool __dummy0) { }
		protected Shader() { }

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Shader> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>
		/// Returns the list of all variation parameters supported by this shader, possible values of each parameter and other 
		/// meta-data.
		/// </summary>
		[NativeWrapper]
		public ShaderVariationParamInfo[] VariationParams
		{
			get { return Internal_GetVariationParams(mCachedPtr); }
		}

		/// <summary>Returns information about all parameters available in the shader.</summary>
		[NativeWrapper]
		public ShaderParameter[] Parameters
		{
			get { return Internal_GetParameters(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Shader>(Shader x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Shader> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ShaderVariationParamInfo[] Internal_GetVariationParams(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ShaderParameter[] Internal_GetParameters(IntPtr thisPtr);
	}

	/** @} */
}
