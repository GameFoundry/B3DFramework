//********************************* bs::framework - Copyright 2025 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

using System;
using System.Runtime.CompilerServices;
using System.Runtime.Loader;

namespace bs
{
    /** @cond INTEROP */

    class CustomAssemblyLoadContext : AssemblyLoadContext
    {
        public CustomAssemblyLoadContext() : base("Custom", true) { }
    }

    class AssemblyLoader
    {
        private static WeakReference contextWeakReference;

        static void CreateContext()
        {
            // TODO - Ensure not already created
            AssemblyLoadContext currentContext = new CustomAssemblyLoadContext();
            contextWeakReference = new WeakReference(currentContext);
        }

        static void LoadAssembly(string path)
        {
            ((AssemblyLoadContext)contextWeakReference.Target).LoadFromAssemblyPath(path);
        }

        static void UnloadContext()
        {
            // TODO - Ensure not already created
            if (contextWeakReference.IsAlive)
            {
                ((AssemblyLoadContext)contextWeakReference.Target).Unload();

                for (int i = 0; contextWeakReference.IsAlive && (i < 1000); i++)
                {
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                }

                if (contextWeakReference.IsAlive)
                    throw new ArgumentException();
            }
        }

        //private static void CurrentContext_Unloading(AssemblyLoadContext obj)
        //{
        //}
    }

    /** @endcond */
}
