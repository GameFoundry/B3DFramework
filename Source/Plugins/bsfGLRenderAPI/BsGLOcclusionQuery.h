//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of an occlusion query. */
	class GLOcclusionQuery : public OcclusionQuery
	{
	public:
		GLOcclusionQuery(bool binary, UINT32 deviceIdx);
		~GLOcclusionQuery();

		/** @copydoc OcclusionQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc OcclusionQuery::end */
		void end(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc OcclusionQuery::isReady */
		bool isReady() const override;

		/** @copydoc OcclusionQuery::getNumSamples */
		UINT32 getNumSamples() override;

	private:
		friend class QueryManager;

		/** Processes query results and saves them for later use. To be called when query has completed. */
		void finalize();

	private:
		GLuint mQueryObj = 0;
		bool mFinalized = false;
		bool mEndIssued = false;

		UINT32 mNumSamples = 0;
	};

	/** @} */
}}