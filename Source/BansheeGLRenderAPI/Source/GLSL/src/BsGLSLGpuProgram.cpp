//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLSLGpuProgram.h"
#include "BsRenderAPI.h"
#include "BsException.h"
#include "BsGLSLParamParser.h"
#include "BsHardwareBufferManager.h"
#include "BsRenderStats.h"
#include "BsGpuParams.h"

namespace BansheeEngine 
{
	UINT32 GLSLGpuProgramCore::mVertexShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mFragmentShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mGeometryShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mDomainShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mHullShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mComputeShaderCount = 0;

	bool checkForGLSLError(const GLuint programObj, String& outErrorMsg)
	{
		StringStream stream;

		GLint linkCompileSuccess = 0;
		glGetProgramiv(programObj, GL_LINK_STATUS, &linkCompileSuccess);

		GLenum glErr;
		bool errorsFound = false;

		glErr = glGetError();
		while (glErr != GL_NO_ERROR)
		{
			const char* glerrStr = (const char*)gluErrorString(glErr);

			if (glerrStr)
			{
				if (errorsFound)
					stream << "\nPrevious errors: \n";

				stream << String(glerrStr) << std::endl;;
			}

			glErr = glGetError();
			errorsFound = true;
		}

		if ((errorsFound || !linkCompileSuccess) && programObj > 0)
		{
			GLint infologLength = 0;
			glGetProgramiv(programObj, GL_INFO_LOG_LENGTH, &infologLength);

			if (infologLength > 0)
			{
				GLint charsWritten = 0;

				GLchar* infoLog = (GLchar*)bs_alloc(sizeof(GLchar)* infologLength);

				glGetProgramInfoLog(programObj, infologLength, &charsWritten, infoLog);

				stream << "Compile and linker info log: \n";
				stream << String(infoLog);

				bs_free(infoLog);
			}
		}

		outErrorMsg = stream.str();

		return errorsFound || !linkCompileSuccess;
	}
	
	GLSLGpuProgramCore::GLSLGpuProgramCore(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuProgramCore(desc, deviceMask), mProgramID(0), mGLHandle(0)
    { }

	GLSLGpuProgramCore::~GLSLGpuProgramCore()
    { 
		if (mIsCompiled && mGLHandle != 0)
		{
			glDeleteProgram(mGLHandle);
			mGLHandle = 0;
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void GLSLGpuProgramCore::initialize()
	{
		static const char GLSL_VERSION_LINE[] = "#version 430\n";

		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileError = "Specified program is not supported by the current render system.";

			GpuProgramCore::initialize();
			return;
		}

		GLenum shaderType = 0x0000;
		switch (mProperties.getType())
		{
		case GPT_VERTEX_PROGRAM:
			shaderType = GL_VERTEX_SHADER;
			mProgramID = ++mVertexShaderCount;
			break;
		case GPT_FRAGMENT_PROGRAM:
			shaderType = GL_FRAGMENT_SHADER;
			mProgramID = ++mFragmentShaderCount;
			break;
		case GPT_GEOMETRY_PROGRAM:
			shaderType = GL_GEOMETRY_SHADER;
			mProgramID = ++mGeometryShaderCount;
			break;
		case GPT_HULL_PROGRAM:
			shaderType = GL_TESS_CONTROL_SHADER;
			mProgramID = ++mDomainShaderCount;
			break;
		case GPT_DOMAIN_PROGRAM:
			shaderType = GL_TESS_EVALUATION_SHADER;
			mProgramID = ++mHullShaderCount;
			break;
		case GPT_COMPUTE_PROGRAM:
			shaderType = GL_COMPUTE_SHADER;
			mProgramID = ++mComputeShaderCount;
			break;
		}

		// Add preprocessor extras and main source
		const String& source = mProperties.getSource();
		if (!source.empty())
		{
			Vector<GLchar*> lines;

			GLchar* firstLineData = (GLchar*)bs_stack_alloc(sizeof(GLSL_VERSION_LINE));
			memcpy(firstLineData, GLSL_VERSION_LINE, sizeof(GLSL_VERSION_LINE));

			lines.push_back(firstLineData);

			UINT32 lineLength = 0;
			for (UINT32 i = 0; i < source.size(); i++)
			{
				if (source[i] == '\n' || source[i] == '\r')
				{
					if (lineLength > 0)
					{
						assert(sizeof(source[i]) == sizeof(GLchar));

						GLchar* lineData = (GLchar*)bs_stack_alloc(sizeof(GLchar) * (lineLength + 2));
						memcpy(lineData, &source[i - lineLength], sizeof(GLchar) * lineLength);

						lineData[lineLength] = '\n';
						lineData[lineLength + 1] = '\0';

						lines.push_back(lineData);
						lineLength = 0;
					}
				}
				else
				{
					lineLength++;
				}
			}

			if (lineLength > 0)
			{
				UINT32 end = (UINT32)source.size() - 1;
				assert(sizeof(source[end]) == sizeof(GLchar));

				GLchar* lineData = (GLchar*)bs_stack_alloc(sizeof(GLchar) * (lineLength + 1));
				memcpy(lineData, &source[source.size() - lineLength], sizeof(GLchar) * lineLength);
				lineData[lineLength] = '\0';

				lines.push_back(lineData);
				lineLength = 0;
			}

			mGLHandle = glCreateShaderProgramv(shaderType, (GLsizei)lines.size(), (const GLchar**)lines.data());

			for (auto iter = lines.rbegin(); iter != lines.rend(); ++iter)
			{
				bs_stack_free(*iter);
			}

			mCompileError = "";
			mIsCompiled = !checkForGLSLError(mGLHandle, mCompileError);
		}

		if (mIsCompiled)
		{
			GLSLParamParser paramParser;
			paramParser.buildUniformDescriptions(mGLHandle, mProperties.getType(), *mParametersDesc);

			if (mProperties.getType() == GPT_VERTEX_PROGRAM)
			{
				List<VertexElement> elementList = paramParser.buildVertexDeclaration(mGLHandle);
				mInputDeclaration = HardwareBufferCoreManager::instance().createVertexDeclaration(elementList);
			}
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);
		GpuProgramCore::initialize();
	}

	bool GLSLGpuProgramCore::isSupported() const
	{
		if (!isRequiredCapabilitiesSupported())
			return false;

		RenderAPICore* rapi = BansheeEngine::RenderAPICore::instancePtr();
		return rapi->getCapabilities().isShaderProfileSupported("glsl");
	}
}

