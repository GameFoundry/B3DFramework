//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup GUI
     *  @{
     */

    public partial class LocString
    {
        /// <summary>
        /// Converts a normal string into a localized string by using the normal string as the identifier.
        /// </summary>
        /// <param name="identifier">Localized string identifier.</param>
        /// <returns>Localized string with the provided identifier, from the default string table.</returns>
        public static implicit operator LocString(string identifier)
        {
            return new LocString(identifier);
        }

        /// <summary>
        /// Retrieves localized text for the active language from the localized string.
        /// </summary>
        /// <param name="text">Localized string to retrieve the text from.</param>
        /// <returns>Translated text for the currently active language.</returns>
        public static explicit operator string(LocString text)
        {
            return Internal_getValue(text.mCachedPtr);
        }
    }

    /** @} */
}
