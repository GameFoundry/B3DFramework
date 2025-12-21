//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
#if !IS_B3D
	/** @addtogroup Application
	 *  @{
	 */

	/// <summary>Structure containing parameters for starting the application.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ApplicationCreateInformation
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ApplicationCreateInformation Default()
		{
			ApplicationCreateInformation value = new ApplicationCreateInformation();
			value.RenderApi = "";
			value.Renderer = "";
			value.Physics = "";
			value.Audio = "";
			value.Input = "";
			value.PhysicsCooking = true;
			value.AsyncAnimation = true;
			value.PrimaryWindow = RenderWindowCreateInformation.Default();
			value.Importers = null;

			return value;
		}

		public ApplicationCreateInformation(VideoMode videoMode, string title, bool fullscreen)
		{
			this.RenderApi = "";
			this.Renderer = "";
			this.Physics = "";
			this.Audio = "";
			this.Input = "";
			this.PhysicsCooking = true;
			this.AsyncAnimation = true;
			this.PrimaryWindow = RenderWindowCreateInformation.Default();
			this.Importers = null;
		}

		/// <summary>Name of the render system plugin to use.</summary>
		public string RenderApi;
		/// <summary>Name of the renderer plugin to use.</summary>
		public string Renderer;
		/// <summary>Name of physics plugin to use.</summary>
		public string Physics;
		/// <summary>Name of the audio plugin to use.</summary>
		public string Audio;
		/// <summary>Name of the input plugin to use.</summary>
		public string Input;
		/// <summary>
		/// True if physics cooking library should be loaded. Cooking is useful for creating collision meshes during development 
		/// type, but might be unnecessary in the final application. When turned off you can save on space by not shipping the 
		/// cooking library.
		/// </summary>
		public bool PhysicsCooking;
		/// <summary>
		/// True if animation should be evaluated at the same time while rendering is happening. This introduces a one frame 
		/// delay to all animations but can result in better performance. If false the animation will be forced to finish 
		/// evaluating before rendering starts, ensuring up-to-date frame but potentially blocking the rendering thread from 
		/// moving forward until the animation finishes.
		/// </summary>
		public bool AsyncAnimation;
		/// <summary>Describes the window to create during start-up.</summary>
		public RenderWindowCreateInformation PrimaryWindow;
		/// <summary>A list of importer plugins to load.</summary>
		public string[] Importers;
	}

	/** @} */
#endif
}
