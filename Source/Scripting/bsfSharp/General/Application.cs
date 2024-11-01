//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2024 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.IO;
using bs;

namespace bs
{
    /// <summary>
    /// Provides various functionality for running the application.
    /// </summary>
    public partial class Application
    {
        /// <summary>
        /// Called at editor start-up by the runtime, and any time assembly refresh occurs.
        /// </summary>
        internal static void OnInitialize()
        {
        }

        /// <summary>
        /// Called every frame by the runtime.
        /// </summary>
        internal static void OnUpdate()
        {
            SceneManager.Update();
        }

        /// <summary>
        /// Called before shut-down by the runtime.
        /// </summary>
        internal static void OnDestroy()
        { 
        }
    }

    /** @} */
}
