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

	/// <summary>Structure containing parameters for starting the application.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct StartUpDesc
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static StartUpDesc Default()
		{
			StartUpDesc value = new StartUpDesc();
			value.renderAPI = "";
			value.renderer = "";
			value.physics = "";
			value.audio = "";
			value.input = "";
			value.physicsCooking = true;
			value.primaryWindowDesc = new RenderWindowDesc();
			value.importers = "";

			return value;
		}

		/// <summary>Name of the render system plugin to use.</summary>
		public string renderAPI;
		/// <summary>Name of the renderer plugin to use.</summary>
		public string renderer;
		/// <summary>Name of physics plugin to use.</summary>
		public string physics;
		/// <summary>Name of the audio plugin to use.</summary>
		public string audio;
		/// <summary>Name of the input plugin to use.</summary>
		public string input;
		/// <summary>
		/// True if physics cooking library should be loaded. Cooking is useful for creating collision meshes during development 
		/// type, but might be unnecessary in the final application. When turned off you can save on space by not shipping the 
		/// cooking library.
		/// </summary>
		public bool physicsCooking;
		/// <summary>Describes the window to create during start-up.</summary>
		public RenderWindowDesc primaryWindowDesc;
		/// <summary>A list of importer plugins to load.</summary>
		public string[] importers;
	}

	/** @} */
#endif
}
