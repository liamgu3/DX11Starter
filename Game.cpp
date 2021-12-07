#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "BufferStructs.h"
#include "SimpleShader.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	camera = 0;
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Game

	//deleting pointers in entityList
	for (Entity* entity : entityList)
	{
		delete entity;
	}

	delete camera;

	//deleting materials
	delete matBronze;
	delete matCobblestone;
	delete matFloor;
	delete matPaint;
	delete matScratched;

	delete skybox;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();

	CreateBasicGeometry();
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//getting byte width of buffer
	unsigned int size = sizeof(VertexShaderExternalData);
	size = (size + 15) / 16 * 16;

	//creating camera
	camera = new Camera(0, 0, -5, (float)width / height, 4.0f, 2.0f, XM_PIDIV2);

	//initializing lights
	directionalLight1 = {};
	directionalLight1.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.direction = XMFLOAT3(1.0, 0.0, 0.0);	//points right
	//directionalLight1.color = XMFLOAT3(1.0f, 0.2f, 0.2f);	//colored red light
	directionalLight1.color = XMFLOAT3(1.0f, 1.0f, 1.0f);	//white light
	directionalLight1.intensity = 1.0f;

	directionalLight2 = {};
	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.direction = XMFLOAT3(-1.0, 0.0, 0.0);	//points left
	//directionalLight2.color = XMFLOAT3(0.2f, 0.2f, 1.0f);	//colored blue light
	directionalLight2.color = XMFLOAT3(1.0f, 1.0f, 1.0f);	//white light
	directionalLight2.intensity = 1.0f;

	directionalLight3 = {};
	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.direction = XMFLOAT3(0.0, -1.0, 0.0);	//points down
	//directionalLight3.color = XMFLOAT3(0.2f, 1.0f, 0.2f);	//colored green light
	directionalLight3.color = XMFLOAT3(1.0f, 1.0f, 1.0f);	//white light
	directionalLight3.intensity = 2.0f;
	directionalLight3.castsShadows = 1;		//does cast shadows

	pointLight1 = {};
	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.color = XMFLOAT3(1.0, 1.0, 1.0);
	pointLight1.intensity = 2.0f;
	pointLight1.position = XMFLOAT3(-2.0, 0.0, -1.0);
	pointLight1.range = 3.0f;

	pointLight2 = {};
	pointLight2.type = LIGHT_TYPE_POINT;
	pointLight2.color = XMFLOAT3(1.0, 1.0, 1.0);
	pointLight2.intensity = 2.0f;
	pointLight2.position = XMFLOAT3(2.0, 2.0, 0.0);
	pointLight2.range = 3.0f;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::shared_ptr<SimpleVertexShader>(new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShader.cso").c_str()));
	pixelShader = std::shared_ptr<SimplePixelShader>(new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShader.cso").c_str()));
	//pixelShader = std::shared_ptr<SimplePixelShader>(new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"CustomPS.cso").c_str()));

	shadowVS = std::shared_ptr<SimpleVertexShader>(new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"ShadowVS.cso").c_str()));

	//sky shaders
	skyVertexShader = std::shared_ptr<SimpleVertexShader>(new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"SkyVertexShader.cso").c_str()));
	skyPixelShader = std::shared_ptr<SimplePixelShader>(new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"SkyPixelShader.cso").c_str()));
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in memory
	//    over to a DirectX-controlled data structure (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	//Vertex vertices0[] =
	//{
	//	{ XMFLOAT3(+0.0f, +0.25f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },
	//	{ XMFLOAT3(+0.25f, -0.25f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },
	//	{ XMFLOAT3(-0.25f, -0.25f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },
	//};

	// Set up the indices, which tell us which vertices to use and in which order
	// - This is somewhat redundant for just 3 vertices (it's a simple example)
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices0[] = { 0, 1, 2 };

	//mesh0 = std::make_shared<Mesh>(vertices0, 3, indices0, 3, device, context);
	mesh0 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device, context);

	//Vertex vertices1[] =
	//{
	//	{ XMFLOAT3(-0.75f, -0.5f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },
	//	{ XMFLOAT3(-0.5f, -0.5f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },
	//	{ XMFLOAT3(-0.5f, -0.75f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },
	//	{ XMFLOAT3(-0.75f, -0.75f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },
	//};

	unsigned int indices1[] = { 0, 1, 2, 0, 2, 3 };

	//mesh1 = std::make_shared<Mesh>(vertices1, 4, indices1, 6, device, context);
	mesh1 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device, context);

	//Vertex vertices2[] =
	//{													//PENTAGON
	//	{ XMFLOAT3(+0.5f, +0.5f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },	//bottom left
	//	{ XMFLOAT3(+0.75f, +0.5f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },		//bottom right
	//	{ XMFLOAT3(+0.25f, +0.75f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },		//left
	//	{ XMFLOAT3(+1.0f, +0.75f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },		//right
	//	{ XMFLOAT3(+0.625f, +1.0f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(+0.0f, +0.0f) },	//top
	//};

	unsigned int indices2[] = { 0, 3, 1, 0, 4, 3, 0, 2, 4 };

	//mesh2 = std::make_shared<Mesh>(vertices2, 5, indices2, 9, device, context);
	mesh2 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device, context);

	mesh3 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device, context);

	//creating textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(), 0, bronzeAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(), 0, cobblestoneAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_albedo.png").c_str(), 0, floorAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(), 0, paintAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_albedo.png").c_str(), 0, scratchedAlbedoSRV.GetAddressOf());
	
	//creating roughness textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(), 0, bronzeRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(), 0, cobblestoneRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_roughness.png").c_str(), 0, floorRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(), 0, paintRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_roughness.png").c_str(), 0, scratchedRoughnessSRV.GetAddressOf());

	//creating normalMap textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(), 0, bronzeNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(), 0, cobblestoneNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_normals.png").c_str(), 0, floorNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_normals.png").c_str(), 0, paintNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_normals.png").c_str(), 0, scratchedNormalSRV.GetAddressOf());

	//creating metalnessMap textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(), 0, bronzeMetalnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(), 0, cobblestoneMetalnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_metal.png").c_str(), 0, floorMetalnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_metal.png").c_str(), 0, paintMetalnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_metal.png").c_str(), 0, scratchedMetalnessSRV.GetAddressOf());

	//creating sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 5;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device.Get()->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());

	//OLD MATERIALS
	//materialRed = new Material(XMFLOAT4(1.0f, 0.5f, 0.5f, 0.0f), 0.8f, pixelShader, vertexShader);
	//materialGreen = new Material(XMFLOAT4(0.5f, 1.0f, 0.5f, 0.0f), 0.8f, pixelShader, vertexShader);
	//materialBlue = new Material(XMFLOAT4(0.5f, 0.5f, 1.0f, 0.0f), 0.8f, pixelShader, vertexShader);
	//materialWhiteRustyMetal = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), 0.5f, pixelShader, vertexShader);
	//materialWhiteWood = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), 0.5f, pixelShader, vertexShader);
	//materialWhiteConcrete = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), 0.5f, pixelShader, vertexShader);
	//materialWhiteSciFiFabric = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), 0.5f, pixelShader, vertexShader);

	//materials
	matBronze = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, pixelShader, vertexShader);
	matCobblestone = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, pixelShader, vertexShader);
	matFloor = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, pixelShader, vertexShader);
	matPaint = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, pixelShader, vertexShader);
	matScratched = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, pixelShader, vertexShader);

	//albedos
	matBronze->AddTextureSRV("SurfaceTexture", bronzeAlbedoSRV);
	matCobblestone->AddTextureSRV("SurfaceTexture", cobblestoneAlbedoSRV);
	matFloor->AddTextureSRV("SurfaceTexture", floorAlbedoSRV);
	matPaint->AddTextureSRV("SurfaceTexture", paintAlbedoSRV);
	matScratched->AddTextureSRV("SurfaceTexture", scratchedAlbedoSRV);

	//roughness
	matBronze->AddTextureSRV("SurfaceRoughness", bronzeRoughnessSRV);
	matCobblestone->AddTextureSRV("SurfaceRoughness", cobblestoneRoughnessSRV);
	matFloor->AddTextureSRV("SurfaceRoughness", floorRoughnessSRV);
	matPaint->AddTextureSRV("SurfaceRoughness", paintRoughnessSRV);
	matScratched->AddTextureSRV("SurfaceRoughness", scratchedRoughnessSRV);

	//normalMaps
	matBronze->AddTextureSRV("NormalMap", bronzeNormalSRV);
	matCobblestone->AddTextureSRV("NormalMap", cobblestoneNormalSRV);
	matFloor->AddTextureSRV("NormalMap", floorNormalSRV);
	matPaint->AddTextureSRV("NormalMap", paintNormalSRV);
	matScratched->AddTextureSRV("NormalMap", scratchedNormalSRV);

	//metalnessMaps
	matBronze->AddTextureSRV("MetalnessMap", bronzeMetalnessSRV);
	matCobblestone->AddTextureSRV("MetalnessMap", cobblestoneMetalnessSRV);
	matFloor->AddTextureSRV("MetalnessMap", floorMetalnessSRV);
	matPaint->AddTextureSRV("MetalnessMap", paintMetalnessSRV);
	matScratched->AddTextureSRV("MetalnessMap", scratchedMetalnessSRV);

	//samplers
	matBronze->AddSampler("BasicSampler", samplerState);
	matCobblestone->AddSampler("BasicSampler", samplerState);
	matFloor->AddSampler("BasicSampler", samplerState);
	matPaint->AddSampler("BasicSampler", samplerState);
	matScratched->AddSampler("BasicSampler", samplerState);


	//pushing to entity list
	entityList.push_back(new Entity(mesh0, matCobblestone));
	entityList[0]->GetTransform()->SetScale(50, 50, 50);
	entityList[0]->GetTransform()->SetPosition(0, -27.5, 0);

	entityList.push_back(new Entity(mesh1, matBronze));
	entityList.push_back(new Entity(mesh1, matCobblestone));
	entityList.push_back(new Entity(mesh1, matFloor));
	entityList.push_back(new Entity(mesh1, matPaint));
	entityList.push_back(new Entity(mesh1, matScratched));
	
	//place objects
	entityList[2]->GetTransform()->MoveAbsolute(0.0f, -4.0f, 0.0f);
	entityList[3]->GetTransform()->MoveAbsolute(0.0f, 4.0f, 0.0f);
	entityList[4]->GetTransform()->MoveAbsolute(-4.0f, 0.0f, 0.0f);
	entityList[5]->GetTransform()->MoveAbsolute(4.0f, 0.0f, 0.0f);

	ambient = XMFLOAT3(0.05f, 0.05f, 0.15f);

	//FINAL PROJECT: SHADOW MAPPING
	
	//texture to draw shadow map to
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = 2048;
	shadowDesc.Height = 2048;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	//depth stencil view for using texture as a depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowDSV.GetAddressOf());

	//SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowSRV.GetAddressOf());

	//special comparison sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;	//comparison filter
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	//Rasterizer State
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000;
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	//Creating fake camera for rendering shadow map

	//view matrix
	XMMATRIX shadowView = XMMatrixLookAtLH(
		XMVectorSet(0, 30, 0, 0),
		XMVectorSet(0, 0, 0, 0),
		XMVectorSet(0, 1, 0, 0)
	);
	XMStoreFloat4x4(&shadowViewMatrix, shadowView);

	//orthographic projection matrix
	XMMATRIX shadowProj = XMMatrixOrthographicLH(2048, 2048, 0.1f, 100.0f);	//might need to make size bigger if required to encompass scene
	XMStoreFloat4x4(&shadowProjectionMatrix, shadowProj);

	//creating sky
	CreateDDSTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Skies/SunnyCubeMap.dds").c_str(), 0, skySRV.GetAddressOf());
	skybox = new Sky(mesh0, samplerState, device, skySRV, skyPixelShader, skyVertexShader);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	//updating projection matrix on window resize
	if (camera)
		camera->UpdateProjectionMatrix((float)width / height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//making entities move
	for (int i = 0; i < entityList.size(); i++)
	{
		if (i != 0)
		{
			entityList[i]->GetTransform()->Rotate(0, .1f * deltaTime, 0);
		}
	}

	camera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);


	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	context->VSSetShader(vertexShader.get()->GetDirectXShader().Get(), 0, 0);
	context->PSSetShader(pixelShader.get()->GetDirectXShader().Get(), 0, 0);


	// Ensure the pipeline knows how to interpret the data (numbers)
	// from the vertex buffer.  
	// - If all of your 3D models use the exact same vertex layout,
	//    this could simply be done once in Init()
	// - However, this isn't always the case (but might be for this course)
	//context->IASetInputLayout(inputLayout.Get());	//to remove

	// Render the shadow map before rendering anything to the screen
	RenderShadowMap();

	//creating buffer struct
	VertexShaderExternalData vsData;
	vsData.colorTint	= XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.world = XMFLOAT4X4(); //idk what to put here

	//draw entities
	for (int i = 0; i < entityList.size(); i++)
	{	
		//shadow data set here so that it doesn't have to be passed to entity
		std::shared_ptr<SimpleVertexShader> vs = entityList[i]->GetMaterial()->GetVertexShader();
		vs->SetMatrix4x4("shadowView", shadowViewMatrix);
		vs->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);

		std::shared_ptr<SimplePixelShader> ps = entityList[i]->GetMaterial()->GetPixelShader();
		ps->SetShaderResourceView("ShadowMap", shadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSampler);

		entityList[i]->Draw(context, camera, ambient);
		//entityList[i]->GetMaterial()->GetPixelShader()->SetData("directionalLight1", &directionalLight1, sizeof(directionalLight1));
		//entityList[i]->GetMaterial()->GetPixelShader()->SetData("directionalLight2", &directionalLight2, sizeof(directionalLight2));
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("directionalLight3", &directionalLight3, sizeof(directionalLight3));
		//entityList[i]->GetMaterial()->GetPixelShader()->SetData("pointLight1", &pointLight1, sizeof(pointLight1));
		//entityList[i]->GetMaterial()->GetPixelShader()->SetData("pointLight2", &pointLight2, sizeof(pointLight2));
	}

	//draw sky
	skybox->Draw(context, camera);

	//need to unbind shadow map as a srv because using it as depth buffer
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->PSSetShaderResources(0, 16, nullSRVs);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}

void Game::RenderShadowMap()
{
	//initializing pipeline setup - clear shadow map
	context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer.Get());

	//viewport matching shadow map resolution
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)2048;
	viewport.Height = (float)2048;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	//turning on the shadow map vertex shader, turning off pixel shader
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);
	context->PSSetShader(0, 0, 0);	//no pixel shader

	//draw all entities
	for (int i = 0; i < entityList.size(); i++)
	{
		shadowVS->SetMatrix4x4("world", entityList[i]->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();

		//draw mesh
		entityList[i]->Draw(context, camera, ambient);
	}

	//after rendering shadow map, return to rendering screen
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
	viewport.Width = (float)this->width;
	viewport.Height = (float)this->height;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}