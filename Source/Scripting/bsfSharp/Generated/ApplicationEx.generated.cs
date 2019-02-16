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
	/** @addtogroup Application
	 *  @{
	 */

	/// <summary>Script-exported API for Application.</summary>
	[ShowInInspector]
	public partial class Application : ScriptObject
	{
		private Application(bool __dummy0) { }
		protected Application() { }

		public static void StartUp(StartUpDesc desc)
		{
			Internal_startUp(ref desc);
		}

		public static void RunMainLoop()
		{
			Internal_runMainLoop();
		}

		public static void ShutDown()
		{
			Internal_shutDown();
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_startUp(ref StartUpDesc desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_runMainLoop();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_shutDown();
	}

	/** @} */
#endif
}
