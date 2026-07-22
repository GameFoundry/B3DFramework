//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>Contains import options you may use to control how is a shader imported.</summary>
	[ShowInInspector]
	public partial class ShaderImportOptions : ImportOptions
	{
		private ShaderImportOptions(bool __dummy0) { }

		/// <summary>Creates a new import options object that allows you to customize how are meshes imported.</summary>
		public ShaderImportOptions()
		{
			Internal_Create(this);
		}

		/// <summary>
		/// Low-level shading language identifiers (for example &quot;hlsl&quot;, &quot;vksl&quot;) the BSL shader should be 
		/// converted into. This ultimately controls on which render backends it will be able to run on.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public string[] Languages
		{
			get { return Internal_GetLanguages(mCachedPtr); }
			set { Internal_SetLanguages(mCachedPtr, value); }
		}

		/// <summary>
		/// Sets a define and its value. Replaces an existing define if one already exists with the provided name.
		/// </summary>
		/// <param name="define">Name of the define.</param>
		/// <param name="value">Value to assign to the define.</param>
		public void SetDefine(string define, string value)
		{
			Internal_SetDefine(mCachedPtr, define, value);
		}

		/// <summary>Checks if the define exists and returns its value if it does.</summary>
		/// <param name="define">Name of the define to get the value for.</param>
		/// <param name="outValue">Value of the define. Only defined if the method returns true.</param>
		/// <returns>True if the define was found, false otherwise.</returns>
		public bool GetDefine(string define, out string outValue)
		{
			return Internal_GetDefine(mCachedPtr, define, out outValue);
		}

		/// <summary>Checks if the provided define exists.</summary>
		/// <param name="define">Name of the define to check.</param>
		/// <returns>True if the define was found, false otherwise.</returns>
		public bool HasDefine(string define)
		{
			return Internal_HasDefine(mCachedPtr, define);
		}

		/// <summary>Unregisters a previously set define.</summary>
		/// <param name="define">Name of the define to unregister.</param>
		public void RemoveDefine(string define)
		{
			Internal_RemoveDefine(mCachedPtr, define);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDefine(IntPtr thisPtr, string define, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetDefine(IntPtr thisPtr, string define, out string outValue);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasDefine(IntPtr thisPtr, string define);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveDefine(IntPtr thisPtr, string define);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string[] Internal_GetLanguages(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLanguages(IntPtr thisPtr, string[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ShaderImportOptions managedInstance);
	}

	/** @} */
#endif
}
