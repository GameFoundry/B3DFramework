//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsApplication.h"
#include "Material/BsMaterial.h"
#include "CoreThread/BsCoreThread.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "RenderAPI/BsCommandBuffer.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuPipelineState.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Mesh/BsMeshData.h"
#include "Math/BsQuaternion.h"
#include "Utility/BsTime.h"
#include "Renderer/BsRendererUtility.h"
#include "BsEngineConfig.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This example uses the low-level rendering API to render a textured cube mesh. This is opposed to using scene objects
// and components, in which case objects are rendered automatically based on their transform and other properties.
// 
// Using low-level rendering API gives you full control over rendering, similar to using Vulkan, DirectX or OpenGL APIs.
//
// In order to use the low-level rendering system we need to override the Application class so we get notified of updates
// and start-up/shut-down events. This is normally not necessary for a high level scene object based model.
//
// The rendering is performed on the core (i.e. rendering) thread, as opposed to the main thread, where majority of
// Banshee's code executes.
//
// The example first sets up necessary resources, like GPU programs, pipeline state, vertex & index buffers. Then every
// frame it binds the necessary rendering resources and executes the draw call.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace bs
{
	UINT32 windowResWidth = 1280;
	UINT32 windowResHeight = 720;

	// Declare the methods we'll use to do work on the core thread. Note the "ct" namespace, which we use because we render
	// on the core thread (ct = core thread). Every object usable on the core thread lives in this namespace.
	namespace ct
	{
		void setup(const SPtr<RenderWindow>& renderWindow);
		void render();
		void shutdown();
	}

	// Override the default Application so we can get notified when engine starts-up, shuts-down and when it executes
	// every frame
	class MyApplication : public Application
	{
	public:
		// Pass along the start-up structure to the parent, we don't need to handle it
		MyApplication(const START_UP_DESC& desc)
			:Application(desc)
		{ }

	private:
		// Called when the engine is first started up
		void onStartUp() override
		{
			// Ensure all parent systems are initialized first
			Application::onStartUp();

			// Get the primary window that was created during start-up. This will be the final destination for all our
			// rendering.
			SPtr<RenderWindow> renderWindow = getPrimaryWindow();

			// Get the version of the render window usable on the core thread, and send it along to setup()
			SPtr<ct::RenderWindow> renderWindowCore = renderWindow->getCore();

			// Initialize all the resources we need for rendering. Since we do rendering on a separate thread (the "core
			// thread"), we don't call the method directly, but rather queue it for execution using the CoreThread class.
			gCoreThread().queueCommand(std::bind(&ct::setup, renderWindowCore));
		}

		// Called when the engine is about to be shut down
		void onShutDown() override
		{
			gCoreThread().queueCommand(&ct::shutdown);

			// Shut-down engine components
			Application::onShutDown();
		}

		// Called every frame, before any other engine system (optionally use postUpdate())
		void preUpdate() override
		{
			gCoreThread().queueCommand(&ct::render);
		}
	};
}

using namespace bs;

// Main entry point into the application
#if BS_PLATFORM == BS_PLATFORM_WIN32
int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
)
#else
int main()
#endif
{
	// Define a video mode for the resolution of the primary rendering window.
	VideoMode videoMode(windowResWidth, windowResHeight);

	// Start-up the engine using our custom MyApplication class. This will also create the primary rendering window.
	// We provide the initial resolution of the window, its title and fullscreen state.
	Application::startUp<MyApplication>(videoMode, "Banshee Example App", false);

	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::instance().runMainLoop();

	Application::shutDown();

	return 0;
}

namespace bs { namespace ct
{
	// Declarations for some helper methods we'll use during setup
	void writeBoxVertices(const AABox& box, UINT8* positions, UINT8* uvs, UINT32 stride);
	void writeBoxIndices(UINT32* indices);
	const char* getVertexProgSource();
	const char* getFragmentProgSource();
	Matrix4 createWorldViewProjectionMatrix();

	// Fields where we'll store the resources required during calls to render(). These are initialized in setup()
	// and cleaned up in shutDown()
	SPtr<GraphicsPipelineState> gPipelineState;
	SPtr<Texture> gSurfaceTex;
	SPtr<SamplerState> gSurfaceSampler;
	SPtr<GpuParams> gGpuParams;
	SPtr<VertexDeclaration> gVertexDecl;
	SPtr<VertexBuffer> gVertexBuffer;
	SPtr<IndexBuffer> gIndexBuffer;
	SPtr<RenderTexture> gRenderTarget;
	SPtr<RenderWindow> gRenderWindow;
	bool gUseHLSL = true;
	bool gUseVKSL = false;

	const UINT32 NUM_VERTICES = 24;
	const UINT32 NUM_INDICES = 36;

	// Structure that will hold uniform block variables for the GPU programs
	struct UniformBlock
	{
		Matrix4 gMatWVP; // World view projection matrix
		Color gTint; // Tint to apply on top of the texture
	};

	// Initializes any resources required for rendering
	void setup(const SPtr<RenderWindow>& renderWindow)
	{
		// Determine which shading language to use (depending on the RenderAPI chosen during build)
		gUseHLSL = strcmp(BS_RENDER_API_MODULE, "BansheeD3D11RenderAPI") == 0;
		gUseVKSL = strcmp(BS_RENDER_API_MODULE, "BansheeVulkanRenderAPI") == 0;

		// This will be the primary output for our rendering (created by the main thread on start-up)
		gRenderWindow = renderWindow;

		// Create a vertex GPU program
		const char* vertProgSrc = getVertexProgSource();

		GPU_PROGRAM_DESC vertProgDesc;
		vertProgDesc.type = GPT_VERTEX_PROGRAM;
		vertProgDesc.entryPoint = "main";
		vertProgDesc.language = gUseHLSL ? "hlsl" : gUseVKSL ? "vksl" : "glsl4_1";
		vertProgDesc.source = vertProgSrc;

		SPtr<GpuProgram> vertProg = GpuProgram::create(vertProgDesc);

		// Create a fragment GPU program
		const char* fragProgSrc = getFragmentProgSource();

		GPU_PROGRAM_DESC fragProgDesc;
		fragProgDesc.type = GPT_FRAGMENT_PROGRAM;
		fragProgDesc.entryPoint = "main";
		fragProgDesc.language = gUseHLSL ? "hlsl" : gUseVKSL ? "vksl" : "glsl4_1";
		fragProgDesc.source = fragProgSrc;

		SPtr<GpuProgram> fragProg = GpuProgram::create(fragProgDesc);

		// Create a graphics pipeline state
		BLEND_STATE_DESC blendDesc;
		blendDesc.renderTargetDesc[0].blendEnable = true;
		blendDesc.renderTargetDesc[0].renderTargetWriteMask = 0b0111; // RGB, don't write to alpha
		blendDesc.renderTargetDesc[0].blendOp = BO_ADD;
		blendDesc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
		blendDesc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;

		DEPTH_STENCIL_STATE_DESC depthStencilDesc;
		depthStencilDesc.depthWriteEnable = false;
		depthStencilDesc.depthReadEnable = false;

		PIPELINE_STATE_DESC pipelineDesc;
		pipelineDesc.blendState = BlendState::create(blendDesc);
		pipelineDesc.depthStencilState = DepthStencilState::create(depthStencilDesc);
		pipelineDesc.vertexProgram = vertProg;
		pipelineDesc.fragmentProgram = fragProg;

		gPipelineState = GraphicsPipelineState::create(pipelineDesc);

		// Create an object containing GPU program parameters
		gGpuParams = GpuParams::create(gPipelineState);

		// Create a vertex declaration for shader inputs
		SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		gVertexDecl = VertexDeclaration::create(vertexDesc);

		// Create & fill the vertex buffer for a box mesh
		UINT32 vertexStride = vertexDesc->getVertexStride();

		VERTEX_BUFFER_DESC vbDesc;
		vbDesc.numVerts = NUM_VERTICES;
		vbDesc.vertexSize = vertexStride;

		gVertexBuffer = VertexBuffer::create(vbDesc);

		UINT8* vbData = (UINT8*)gVertexBuffer->lock(0, vertexStride * NUM_VERTICES, GBL_WRITE_ONLY_DISCARD);
		UINT8* positions = vbData + vertexDesc->getElementOffsetFromStream(VES_POSITION);
		UINT8* uvs = vbData + vertexDesc->getElementOffsetFromStream(VES_TEXCOORD);

		AABox box(Vector3::ONE * -10.0f, Vector3::ONE * 10.0f);
		writeBoxVertices(box, positions, uvs, vertexStride);

		gVertexBuffer->unlock();

		// Create & fill the index buffer for a box mesh
		INDEX_BUFFER_DESC ibDesc;
		ibDesc.numIndices = NUM_INDICES;
		ibDesc.indexType = IT_32BIT;

		gIndexBuffer = IndexBuffer::create(ibDesc);
		UINT32* ibData = (UINT32*)gIndexBuffer->lock(0, NUM_INDICES * sizeof(UINT32), GBL_WRITE_ONLY_DISCARD);
		writeBoxIndices(ibData);

		gIndexBuffer->unlock();

		// Create a simple 2x2 checkerboard texture to map to the object we're about to render
		SPtr<PixelData> pixelData = PixelData::create(2, 2, 1, PF_RGBA8);
		pixelData->setColorAt(Color::White, 0, 0);
		pixelData->setColorAt(Color::Black, 1, 0);
		pixelData->setColorAt(Color::White, 1, 1);
		pixelData->setColorAt(Color::Black, 0, 1);

		gSurfaceTex = Texture::create(pixelData);

		// Create a sampler state for the texture above
		SAMPLER_STATE_DESC samplerDesc;
		samplerDesc.minFilter = FO_POINT;
		samplerDesc.magFilter = FO_POINT;

		gSurfaceSampler = SamplerState::create(samplerDesc);

		// Create a color attachment texture for the render surface
		TEXTURE_DESC colorAttDesc;
		colorAttDesc.width = windowResWidth;
		colorAttDesc.height = windowResHeight;
		colorAttDesc.format = PF_RGBA8;
		colorAttDesc.usage = TU_RENDERTARGET;

		SPtr<Texture> colorAtt = Texture::create(colorAttDesc);

		// Create a depth attachment texture for the render surface
		TEXTURE_DESC depthAttDesc;
		depthAttDesc.width = windowResWidth;
		depthAttDesc.height = windowResHeight;
		depthAttDesc.format = PF_D32;
		depthAttDesc.usage = TU_DEPTHSTENCIL;

		SPtr<Texture> depthAtt = Texture::create(depthAttDesc);

		// Create the render surface
		RENDER_TEXTURE_DESC desc;
		desc.colorSurfaces[0].texture = colorAtt;
		desc.depthStencilSurface.texture = depthAtt;

		gRenderTarget = RenderTexture::create(desc);
	}

	// Render the box, called every frame
	void render()
	{
		// Fill out the uniform block variables
		UniformBlock uniformBlock;
		uniformBlock.gMatWVP = createWorldViewProjectionMatrix();
		uniformBlock.gTint = Color(1.0f, 1.0f, 1.0f, 0.5f);

		// Create a uniform block buffer for holding the uniform variables
		SPtr<GpuParamBlockBuffer> uniformBuffer = GpuParamBlockBuffer::create(sizeof(UniformBlock));
		uniformBuffer->write(0, &uniformBlock, sizeof(uniformBlock));

		// Assign the uniform buffer & texture
		gGpuParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Params", uniformBuffer);
		gGpuParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, "Params", uniformBuffer);

		gGpuParams->setTexture(GPT_FRAGMENT_PROGRAM, "gMainTexture", gSurfaceTex);

		// HLSL uses separate sampler states, so we need to use a different name for the sampler
		if(gUseHLSL)
			gGpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gMainTexSamp", gSurfaceSampler);
		else
			gGpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gMainTexture", gSurfaceSampler);

		// Create a command buffer
		SPtr<CommandBuffer> cmds = CommandBuffer::create(GQT_GRAPHICS);

		// Get the primary render API access point
		RenderAPI& rapi = RenderAPI::instance();

		// Bind render surface & clear it
		rapi.setRenderTarget(gRenderTarget, 0, RT_NONE, cmds);
		rapi.clearRenderTarget(FBT_COLOR | FBT_DEPTH, Color::Blue, 1, 0, 0xFF, cmds);

		// Bind the pipeline state
		rapi.setGraphicsPipeline(gPipelineState, cmds);

		// Set the vertex & index buffers, as well as vertex declaration and draw type
		rapi.setVertexBuffers(0, &gVertexBuffer, 1, cmds);
		rapi.setIndexBuffer(gIndexBuffer, cmds);
		rapi.setVertexDeclaration(gVertexDecl, cmds);
		rapi.setDrawOperation(DOT_TRIANGLE_LIST, cmds);

		// Bind the GPU program parameters (i.e. resource descriptors)
		rapi.setGpuParams(gGpuParams, cmds);

		// Draw
		rapi.drawIndexed(0, NUM_INDICES, 0, NUM_VERTICES, 1, cmds);

		// Submit the command buffer
		rapi.submitCommandBuffer(cmds);

		// Blit the image from the render texture, to the render window
		rapi.setRenderTarget(gRenderWindow);

		// Get the color attachment
		SPtr<Texture> colorTexture = gRenderTarget->getColorTexture(0);

		// Use the helper RendererUtility to draw a full-screen quad of the provided texture and output it to the currently
		// bound render target. Internally this uses the same calls we used above, just with a different pipeline and mesh.
		gRendererUtility().blit(colorTexture);

		// Present the rendered image to the user
		rapi.swapBuffers(gRenderWindow);
	}

	// Clean up any resources
	void shutdown()
	{
		gPipelineState = nullptr;
		gSurfaceTex = nullptr;
		gGpuParams = nullptr;
		gVertexDecl = nullptr;
		gVertexBuffer = nullptr;
		gIndexBuffer = nullptr;
		gRenderTarget = nullptr;
		gRenderWindow = nullptr;
		gSurfaceSampler = nullptr;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////HELPER METHODS/////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	void writeBoxVertices(const AABox& box, UINT8* positions, UINT8* uvs, UINT32 stride)
	{
		AABox::Corner vertOrder[] =
		{
			AABox::NEAR_LEFT_BOTTOM,	AABox::NEAR_RIGHT_BOTTOM,	AABox::NEAR_RIGHT_TOP,		AABox::NEAR_LEFT_TOP,
			AABox::FAR_RIGHT_BOTTOM,	AABox::FAR_LEFT_BOTTOM,		AABox::FAR_LEFT_TOP,		AABox::FAR_RIGHT_TOP,
			AABox::FAR_LEFT_BOTTOM,		AABox::NEAR_LEFT_BOTTOM,	AABox::NEAR_LEFT_TOP,		AABox::FAR_LEFT_TOP,
			AABox::NEAR_RIGHT_BOTTOM,	AABox::FAR_RIGHT_BOTTOM,	AABox::FAR_RIGHT_TOP,		AABox::NEAR_RIGHT_TOP,
			AABox::FAR_LEFT_TOP,		AABox::NEAR_LEFT_TOP,		AABox::NEAR_RIGHT_TOP,		AABox::FAR_RIGHT_TOP,
			AABox::FAR_LEFT_BOTTOM,		AABox::FAR_RIGHT_BOTTOM,	AABox::NEAR_RIGHT_BOTTOM,	AABox::NEAR_LEFT_BOTTOM
		};

		for (auto& entry : vertOrder)
		{
			Vector3 pos = box.getCorner(entry);
			memcpy(positions, &pos, sizeof(pos));

			positions += stride;
		}

		for (UINT32 i = 0; i < 6; i++)
		{
			Vector2 uv;

			uv = Vector2(0.0f, 1.0f);
			memcpy(uvs, &uv, sizeof(uv));
			uvs += stride;

			uv = Vector2(1.0f, 1.0f);
			memcpy(uvs, &uv, sizeof(uv));
			uvs += stride;

			uv = Vector2(1.0f, 0.0f);
			memcpy(uvs, &uv, sizeof(uv));
			uvs += stride;

			uv = Vector2(0.0f, 0.0f);
			memcpy(uvs, &uv, sizeof(uv));
			uvs += stride;
		}
	}

	void writeBoxIndices(UINT32* indices)
	{
		for (UINT32 face = 0; face < 6; face++)
		{
			UINT32 faceVertOffset = face * 4;

			indices[face * 6 + 0] = faceVertOffset + 2;
			indices[face * 6 + 1] = faceVertOffset + 1;
			indices[face * 6 + 2] = faceVertOffset + 0;
			indices[face * 6 + 3] = faceVertOffset + 0;
			indices[face * 6 + 4] = faceVertOffset + 3;
			indices[face * 6 + 5] = faceVertOffset + 2;
		}
	}

	const char* getVertexProgSource()
	{
		if(gUseHLSL)
		{
			static const char* src = R"(
cbuffer Params
{
	float4x4 gMatWVP;
	float4 gTint;
}	

void main(
	in float3 inPos : POSITION,
	in float2 uv : TEXCOORD0,
	out float4 oPosition : SV_Position,
	out float2 oUv : TEXCOORD0)
{
	oPosition = mul(gMatWVP, float4(inPos.xyz, 1));
	oUv = uv;
}
)";

			return src;
		}
		else if(gUseVKSL)
		{
			static const char* src = R"(
layout (binding = 0, std140) uniform Params
{
	mat4 gMatWVP;
	vec4 gTint;
};

layout (location = 0) in vec3 bs_position;
layout (location = 1) in vec2 bs_texcoord0;

layout (location = 0) out vec2 texcoord0;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = gMatWVP * vec4(bs_position.xyz, 1);
	texcoord0 = bs_texcoord0;
}
)";

			return src;
		}
		else
		{
			static const char* src = R"(
layout (std140) uniform Params
{
	mat4 gMatWVP;
	vec4 gTint;
};

in vec3 bs_position;
in vec2 bs_texcoord0;

out vec2 texcoord0;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = gMatWVP * vec4(bs_position.xyz, 1);
	texcoord0 = bs_texcoord0;
}
)";
			return src;
		}
	}

	const char* getFragmentProgSource()
	{
		if (gUseHLSL)
		{
			static const char* src = R"(
cbuffer Params
{
	float4x4 gMatWVP;
	float4 gTint;
}

SamplerState gMainTexSamp : register(s0);
Texture2D gMainTexture : register(t0);

float4 main(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
{
	float4 color = gMainTexture.Sample(gMainTexSamp, uv);
	return color * gTint;
}
)";

			return src;
		}
		else if(gUseVKSL)
		{
			static const char* src = R"(
layout (binding = 0, std140) uniform Params
{
	mat4 gMatWVP;
	vec4 gTint;
};

layout (binding = 1) uniform sampler2D gMainTexture;

layout (location = 0) in vec2 texcoord0;
layout (location = 0) out vec4 fragColor;

void main()
{
	vec4 color = texture(gMainTexture, texcoord0.st);
	fragColor = color * gTint;
}
)";

			return src;
		}
		else
		{

			static const char* src = R"(
layout (std140) uniform Params
{
	mat4 gMatWVP;
	vec4 gTint;
};

uniform sampler2D gMainTexture;

in vec2 texcoord0;
out vec4 fragColor;

void main()
{
	vec4 color = texture(gMainTexture, texcoord0.st);
	fragColor = color * gTint;
}
)";
			return src;
		}
	}

	Matrix4 createWorldViewProjectionMatrix()
	{
		Matrix4 proj = Matrix4::projectionPerspective(Degree(75.0f), 16.0f / 9.0f, 0.05f, 1000.0f);
		bs::RenderAPI::convertProjectionMatrix(proj, proj);

		Vector3 cameraPos = Vector3(0.0f, -20.0f, 50.0f);
		Vector3 lookDir = -Vector3::normalize(cameraPos);

		Quaternion cameraRot(BsIdentity);
		cameraRot.lookRotation(lookDir);

		Matrix4 view = Matrix4::view(cameraPos, cameraRot);

		Quaternion rotation(Vector3::UNIT_Y, Degree(gTime().getTime() * 90.0f));
		Matrix4 world = Matrix4::TRS(Vector3::ZERO, rotation, Vector3::ONE);

		Matrix4 viewProj = proj * view * world;

		// GLSL uses column major matrices, so transpose
		if(!gUseHLSL)
			viewProj = viewProj.transpose();

		return viewProj;
	}
}}