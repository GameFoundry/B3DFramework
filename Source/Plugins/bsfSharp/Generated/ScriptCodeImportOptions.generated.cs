//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>
	/// Contains import options you may use to control how is a file containing script source code importer.
	/// </summary>
	[ShowInInspector]
	public partial class ScriptCodeImportOptions : ImportOptions
	{
		private ScriptCodeImportOptions(bool __dummy0) { }

		/// <summary>Creates a new import options object that allows you to customize how is script code imported.</summary>
		public ScriptCodeImportOptions()
		{
			Internal_create(this);
		}

		/// <summary>Determines whether the script is editor-only or a normal game script.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EditorScript
		{
			get { return Internal_geteditorScript(mCachedPtr); }
			set { Internal_seteditorScript(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_geteditorScript(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_seteditorScript(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ScriptCodeImportOptions managedInstance);
	}

	/** @} */
#endif
}
