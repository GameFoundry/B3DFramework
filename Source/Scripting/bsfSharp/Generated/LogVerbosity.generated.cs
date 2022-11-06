//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Debug
	 *  @{
	 */

	/// <summary>Represents verbosity level at which a specific log message will be displayed.</summary>
	public enum LogVerbosity
	{
		/// <summary>Fatal error happened that application cannot recover from and will crash.</summary>
		Fatal = 0,
		/// <summary>An error happened that will not result in an immediate crash but may cause serious problems.</summary>
		Error = 1,
		/// <summary>
		/// Something went wrong but the application will not crash, although invalid behaviour might be observed.
		/// </summary>
		Warning = 2,
		/// <summary>An informational message will be logged, can be used for debugging and tracing.</summary>
		Info = 3,
		/// <summary>Same as Info, but the message will only be logged to the log file and not any console output.</summary>
		Log = 4,
		/// <summary>
		/// Messages that can provide additional information and warnings, but are too spammy to be displayed under normal 
		/// circumstances.
		/// </summary>
		Verbose = 5,
		/// <summary>Same as Verbose, but for even spammier messages.</summary>
		VeryVerbose = 6,
		/// <summary>Meta-type encompassing all verbosity types. Should not be used for logging directly.</summary>
		Any = 7
	}

	/** @} */
}
