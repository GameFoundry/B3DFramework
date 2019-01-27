//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

namespace BansheeEngine
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Contains a list of layers that can be used for controlling which <see cref="Renderable"/> is output to which
    /// camera. A maximum of 64 layers are supported.
    /// </summary>
    public static class Layers // Note: Placeholder class, need functionality to edit and persist layer names
    {
        private static string[] names;
        private static UInt64[] values;

        /// <summary>
        /// Returns the names of all available layers.
        /// </summary>
        public static string[] Names
        {
            get
            {
                if (names == null)
                {
                    names = new string[64];
                    for (int i = 0; i < names.Length; i++)
                        names[i] = "Layer_" + i;
                }

                return names;
            }
        }

        /// <summary>
        /// Returns the values of all available layers.
        /// </summary>
        public static UInt64[] Values
        {
            get
            {
                if (values == null)
                {
                    values = new UInt64[64];
                    for (int i = 0; i < values.Length; i++)
                        values[i] = 1UL << i;
                }

                return values;
            }
        }
    }

    /** @} */
}
