//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>
	/// Represents the primary entry point for the core systems. Handles start-up, shutdown, primary loop and allows you to 
	/// load and unload plugins.
	/// </summary>
	[ShowInInspector]
	public partial class CoreApplication : ScriptObject
	{
		private CoreApplication(bool __dummy0) { }
		protected CoreApplication() { }

	}
}
