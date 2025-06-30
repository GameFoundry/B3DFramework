//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Scene
	 *  @{
	 */

	/// <summary>Interface that provides editor-specific information about a scene instance.</summary>
	[ShowInInspector]
	public partial class IEditorSceneInstance : ScriptObject
	{
		private IEditorSceneInstance(bool __dummy0) { }
		protected IEditorSceneInstance() { }

	}

	/** @} */
}
