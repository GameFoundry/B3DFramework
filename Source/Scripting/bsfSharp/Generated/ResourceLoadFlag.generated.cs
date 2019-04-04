//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/// <summary>Flags that can be used to control resource loading.</summary>
	public enum ResourceLoadFlag
	{
		/// <summary>No flags.</summary>
		None = 0,
		/// <summary>If enabled all resources referenced by the root resource will be loaded as well.</summary>
		LoadDependencies = 1,
		/// <summary>
		/// If enabled the resource system will keep an internal reference to the resource so it doesn&apos;t get destroyed when 
		/// it goes out of scope. You can call Resources::release() to release the internal reference. Each call to load will 
		/// create a new internal reference and therefore must be followed by the same number of release calls. If dependencies 
		/// are being loaded, they will not have internal references created regardless of this parameter.
		/// </summary>
		KeepInternalRef = 2,
		/// <summary>
		/// Determines if the loaded resource keeps original data loaded. Sometime resources will process loaded data and discard 
		/// the original (e.g. uncompressing audio on load). This flag can prevent the resource from discarding the original 
		/// data. The original data might be required for saving the resource (via Resources::save), but will use up extra 
		/// memory. Normally you want to keep this enabled if you plan on saving the resource to disk.
		/// </summary>
		KeepSourceData = 4,
		/// <summary>Default set of flags used for resource loading.</summary>
		Default = 3
	}

	/** @} */
}
