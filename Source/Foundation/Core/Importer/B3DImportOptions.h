//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Importer
	 *  @{
	 */

	/**
	 * Base class for creating import options from. Import options are specific for each importer and control how is data
	 * imported.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), API(Framework), API(Editor)) ImportOptions : public IReflectable, public IScriptExportable
	{
	public:
		virtual ~ImportOptions() = default;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ImportOptionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
