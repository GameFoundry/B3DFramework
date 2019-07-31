//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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
			Internal_create(this);
		}

		/// <summary>
		/// Flags that control which shading languages should the BSL shader be converted into. This ultimately controls on which 
		/// render backends it will be able to run on.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ShadingLanguageFlags Languages
		{
			get { return Internal_getlanguages(mCachedPtr); }
			set { Internal_setlanguages(mCachedPtr, value); }
		}

		/// <summary>
		/// Sets a define and its value. Replaces an existing define if one already exists with the provided name.
		/// </summary>
		/// <param name="define">Name of the define.</param>
		/// <param name="value">Value to assign to the define.</param>
		public void SetDefine(string define, string value)
		{
			Internal_setDefine(mCachedPtr, define, value);
		}

		/// <summary>Checks if the define exists and returns its value if it does.</summary>
		/// <param name="define">Name of the define to get the value for.</param>
		/// <param name="value">value of the define. Only defined if the method returns true.</param>
		/// <returns>True if the define was found, false otherwise.</returns>
		public bool GetDefine(string define, out string value)
		{
			return Internal_getDefine(mCachedPtr, define, out value);
		}

		/// <summary>Checks if the provided define exists.</summary>
		/// <param name="define">Name of the define to check.</param>
		/// <returns>True if the define was found, false otherwise.</returns>
		public bool HasDefine(string define)
		{
			return Internal_hasDefine(mCachedPtr, define);
		}

		/// <summary>Unregisters a previously set define.</summary>
		/// <param name="define">Name of the define to unregister.</param>
		public void RemoveDefine(string define)
		{
			Internal_removeDefine(mCachedPtr, define);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setDefine(IntPtr thisPtr, string define, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getDefine(IntPtr thisPtr, string define, out string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_hasDefine(IntPtr thisPtr, string define);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeDefine(IntPtr thisPtr, string define);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ShadingLanguageFlags Internal_getlanguages(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setlanguages(IntPtr thisPtr, ShadingLanguageFlags value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ShaderImportOptions managedInstance);
	}

	/** @} */
#endif
}
