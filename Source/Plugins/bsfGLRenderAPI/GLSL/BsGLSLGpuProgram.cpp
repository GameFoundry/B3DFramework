//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GLSL/BsGLSLGpuProgram.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Error/BsException.h"
#include "GLSL/BsGLSLParamParser.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuParams.h"

namespace bs { namespace ct
{
	UINT32 GLSLGpuProgram::sVertexShaderCount = 0;
	UINT32 GLSLGpuProgram::sFragmentShaderCount = 0;
	UINT32 GLSLGpuProgram::sGeometryShaderCount = 0;
	UINT32 GLSLGpuProgram::sDomainShaderCount = 0;
	UINT32 GLSLGpuProgram::sHullShaderCount = 0;
	UINT32 GLSLGpuProgram::sComputeShaderCount = 0;

	bool checkForGLSLError(const GLuint programObj, String& outErrorMsg)
	{
		StringStream stream;

		GLint linkCompileSuccess = 0;
		glGetProgramiv(programObj, GL_LINK_STATUS, &linkCompileSuccess);
		BS_CHECK_GL_ERROR();

		if (!linkCompileSuccess && programObj > 0)
		{
			GLint infologLength = 0;
			glGetProgramiv(programObj, GL_INFO_LOG_LENGTH, &infologLength);
			BS_CHECK_GL_ERROR();

			if (infologLength > 0)
			{
				GLint charsWritten = 0;

				GLchar* infoLog = (GLchar*)bs_alloc(sizeof(GLchar)* infologLength);

				glGetProgramInfoLog(programObj, infologLength, &charsWritten, infoLog);
				BS_CHECK_GL_ERROR();

				stream << "Compile and linker info log: \n";
				stream << String(infoLog);

				bs_free(infoLog);
			}
		}

		outErrorMsg = stream.str();
		return !linkCompileSuccess;
	}
	
	GLSLGpuProgram::GLSLGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuProgram(desc, deviceMask)
	{ }

	GLSLGpuProgram::~GLSLGpuProgram()
	{ 
		if (mIsCompiled && mGLHandle != 0)
		{
			glDeleteProgram(mGLHandle);
			BS_CHECK_GL_ERROR();

			mGLHandle = 0;
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void GLSLGpuProgram::initialize()
	{

#if BS_OPENGL_4_5
		static const char* VERSION_CHARS = "450";
#elif BS_OPENGL_4_4
		static const char* VERSION_CHARS = "440";
#elif BS_OPENGL_4_3
		static const char* VERSION_CHARS = "430";
#elif BS_OPENGL_4_2
		static const char* VERSION_CHARS = "420";
#else
		static const char* VERSION_CHARS = "410";
#endif
		
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileMessages = "Specified GPU program type is not supported by the current render system.";

			GpuProgram::initialize();
			return;
		}

		GLenum shaderType = 0x0000;
		switch (mType)
		{
		case GPT_VERTEX_PROGRAM:
			shaderType = GL_VERTEX_SHADER;
			mProgramID = ++sVertexShaderCount;
			break;
		case GPT_FRAGMENT_PROGRAM:
			shaderType = GL_FRAGMENT_SHADER;
			mProgramID = ++sFragmentShaderCount;
			break;
#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
		case GPT_GEOMETRY_PROGRAM:
			shaderType = GL_GEOMETRY_SHADER;
			mProgramID = ++sGeometryShaderCount;
			break;
		case GPT_HULL_PROGRAM:
			shaderType = GL_TESS_CONTROL_SHADER;
			mProgramID = ++sDomainShaderCount;
			break;
		case GPT_DOMAIN_PROGRAM:
			shaderType = GL_TESS_EVALUATION_SHADER;
			mProgramID = ++sHullShaderCount;
			break;
#endif
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		case GPT_COMPUTE_PROGRAM:
			shaderType = GL_COMPUTE_SHADER;
			mProgramID = ++sComputeShaderCount;
			break;
#endif
		default:
			break;
		}

		// Add preprocessor extras and main source
		const String& source = mSource;
		if (!source.empty())
		{
			Vector<GLchar*> lines;

			const char* versionStr = "#version ";
			UINT32 versionStrLen = (UINT32)strlen(versionStr);

			UINT32 lineLength = 0;
			INT32 versionLineNum = -1;
			for (UINT32 i = 0; i < source.size(); i++)
			{
				if (source[i] == '\n' || source[i] == '\r')
				{
					assert(sizeof(source[i]) == sizeof(GLchar));

					GLchar* lineData = (GLchar*)bs_stack_alloc(sizeof(GLchar) * (lineLength + 2));
					memcpy(lineData, &source[i - lineLength], sizeof(GLchar) * lineLength);

					lineData[lineLength] = '\n';
					lineData[lineLength + 1] = '\0';

					if(versionLineNum == -1 && lineLength >= versionStrLen)
					{
						bool isEqual = true;
						for (UINT32 j = 0; j < versionStrLen; ++j)
						{
							if(lineData[j] != versionStr[j])
							{
								isEqual = false;
								break;
							}
						}

						if (isEqual)
							versionLineNum = (INT32)lines.size();
					}

					lines.push_back(lineData);
					lineLength = 0;
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

			int numInsertedLines = 0;
			if(versionLineNum == -1)
			{
				char versionLine[50];
				strcpy(versionLine, "#version ");
				strcat(versionLine, VERSION_CHARS);
				strcat(versionLine, "\n");

				UINT32 length = (UINT32)strlen(versionLine) + 1;

				GLchar* extraLineData = (GLchar*)bs_stack_alloc(length);
				memcpy(extraLineData, versionLine, length);

				lines.insert(lines.begin(), extraLineData);
				numInsertedLines++;
			}

			char versionDefine[50];
			strcpy(versionDefine, "#define OPENGL");
			strcat(versionDefine, VERSION_CHARS);
			strcat(versionDefine, "\n");

			const char* EXTRA_LINES[] =
				{ 
					"#define OPENGL\n",
					versionDefine
				};

			UINT32 numExtraLines = sizeof(EXTRA_LINES) / sizeof(EXTRA_LINES[0]);
			UINT32 extraLineOffset = versionLineNum != -1 ? versionLineNum + 1 : 0;
			for (UINT32 i = 0; i < numExtraLines; i++)
			{
				UINT32 length = (UINT32)strlen(EXTRA_LINES[i]) + 1;

				GLchar* extraLineData = (GLchar*)bs_stack_alloc(length);
				memcpy(extraLineData, EXTRA_LINES[i], length);

				lines.insert(lines.begin() + extraLineOffset + numInsertedLines, extraLineData);
				numInsertedLines++;
			}

			StringStream codeStream;
			for(auto& entry : lines)
				codeStream << entry;

			for (INT32 i = numInsertedLines - 1; i >= 0; i--)
				bs_stack_free(lines[extraLineOffset + i]);

			if (numInsertedLines > 0)
				lines.erase(lines.begin() + extraLineOffset, lines.begin() + extraLineOffset + numInsertedLines);

			for (auto iter = lines.rbegin(); iter != lines.rend(); ++iter)
				bs_stack_free(*iter);

			String code = codeStream.str();
			const char* codeRaw = code.c_str();
			mGLHandle = glCreateShaderProgramv(shaderType, 1, (const GLchar**)&codeRaw);
			BS_CHECK_GL_ERROR();

			mCompileMessages = "";
			mIsCompiled = !checkForGLSLError(mGLHandle, mCompileMessages);
		}

		if (mIsCompiled)
		{
			GLSLParamParser paramParser;
			paramParser.buildUniformDescriptions(mGLHandle, mType, *mParametersDesc);

			if (mType == GPT_VERTEX_PROGRAM)
			{
				Vector<VertexElement> elementList = paramParser.buildVertexDeclaration(mGLHandle);
				mInputDeclaration = HardwareBufferManager::instance().createVertexDeclaration(elementList);
			}
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);
		GpuProgram::initialize();
	}

	bool GLSLGpuProgram::isSupported() const
	{
		RenderAPI* rapi = RenderAPI::instancePtr();
		const RenderAPICapabilities& caps = rapi->getCapabilities(0);

		switch (mType)
		{
		case GPT_GEOMETRY_PROGRAM:
#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
			return caps.hasCapability(RSC_GEOMETRY_PROGRAM);
#else
			return false;
#endif
		case GPT_HULL_PROGRAM:
		case GPT_DOMAIN_PROGRAM:
#if BS_OPENGL_4_1 || BS_OPENGLES_3_2
			return caps.hasCapability(RSC_TESSELLATION_PROGRAM);
#else
			return false;
#endif
		case GPT_COMPUTE_PROGRAM:
#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			return caps.hasCapability(RSC_COMPUTE_PROGRAM);
#else
			return false;
#endif
		default:
			return true;
		}
	}
}}