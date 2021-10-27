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
	delete materialRed;
	delete materialGreen;
	delete materialBlue;
	delete materialWhiteRustyMetal;
	delete materialWhiteWood;
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
	directionalLight1.color = XMFLOAT3(1.0f, 0.2f, 0.2f);	//colored light
	//directionalLight1.color = XMFLOAT3(1.0f, 1.0f, 1.0f);	//white light
	directionalLight1.intensity = 0.35f;

	directionalLight2 = {};
	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.direction = XMFLOAT3(-1.0, 0.0, 0.0);	//points left
	directionalLight2.color = XMFLOAT3(0.2f, 0.2f, 1.0f);	//colored light
	//directionalLight2.color = XMFLOAT3(1.0f, 1.0f, 1.0f);	//white light
	directionalLight2.intensity = 0.35f;

	directionalLight3 = {};
	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.direction = XMFLOAT3(0.0, -1.0, 0.0);	//points down
	directionalLight3.color = XMFLOAT3(0.2f, 1.0f, 0.2f);	//colored light
	//directionalLight3.color = XMFLOAT3(1.0f, 1.0f, 1.0f);	//white light
	directionalLight3.intensity = 0.35f;

	pointLight1 = {};
	pointLight1.type = LIGHT_TYPE_POINT;
	pointLight1.color = XMFLOAT3(1.0, 1.0, 1.0);
	pointLight1.intensity = 10.0;
	pointLight1.position = XMFLOAT3(-2.5, 0.0, 0.5);
	pointLight1.range = 1.5f;

	pointLight2 = {};
	pointLight2.type = LIGHT_TYPE_POINT;
	pointLight2.color = XMFLOAT3(1.0, 1.0, 1.0);
	pointLight2.intensity = 10.0;
	pointLight2.position = XMFLOAT3(0.0, 2.0, 0.0);
	pointLight2.range = 1.5f;
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

	//creating textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/TexturesCom_Rust_albedo.tif").c_str(), 0, rustyMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/TexturesCom_Wood_PlanksOld_albedo.tif").c_str(), 0, woodSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/TexturesCom_Wall_ConcreteDamagedRebar_albedo.tif").c_str(), 0, damagedConcreteSRV.GetAddressOf());
	
	//creating roughness textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/TexturesCom_Rust_roughness.tif").c_str(), 0, rustyMetalRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/TexturesCom_Wood_PlanksOld3_roughness.tif").c_str(), 0, woodRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/TexturesCom_Wall_ConcreteDamagedRebar_roughness.tif").c_str(), 0, damagedConcreteRoughnessSRV.GetAddressOf());

	//creating sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 5;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device.Get()->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());

	//materials
	materialRed = new Material(XMFLOAT4(1.0f, 0.5f, 0.5f, 0.0f), 0.8f, pixelShader, vertexShader);
	materialGreen = new Material(XMFLOAT4(0.5f, 1.0f, 0.5f, 0.0f), 0.8f, pixelShader, vertexShader);
	materialBlue = new Material(XMFLOAT4(0.5f, 0.5f, 1.0f, 0.0f), 0.8f, pixelShader, vertexShader);
	materialWhiteRustyMetal = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), 0.5f, pixelShader, vertexShader);
	materialWhiteWood = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), 0.5f, pixelShader, vertexShader);
	materialWhiteConcrete = new Material(XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), 0.5f, pixelShader, vertexShader);

	materialWhiteRustyMetal->AddTextureSRV("SurfaceTexture", rustyMetalSRV);	//metal texture
	materialWhiteWood->AddTextureSRV("SurfaceTexture", woodSRV);	//wood texture
	materialWhiteConcrete->AddTextureSRV("SurfaceTexture", damagedConcreteSRV);	//damaged concrete texture

	materialWhiteRustyMetal->AddSampler("BasicSampler", samplerState);	//sampler
	materialWhiteWood->AddSampler("BasicSampler", samplerState);	//sampler
	materialWhiteConcrete->AddSampler("BasicSampler", samplerState);	//sampler

	materialWhiteRustyMetal->AddTextureSRV("SurfaceRoughness", rustyMetalRoughnessSRV);	//metal roughness
	materialWhiteWood->AddTextureSRV("SurfaceRoughness", woodRoughnessSRV);	//wood roughness
	materialWhiteConcrete->AddTextureSRV("SurfaceRoughness", damagedConcreteRoughnessSRV);	//damaged concrete roughness

	//pushing to entity list
	entityList.push_back(new Entity(mesh0, materialWhiteRustyMetal));
	entityList.push_back(new Entity(mesh0, materialWhiteRustyMetal));
	entityList.push_back(new Entity(mesh0, materialWhiteConcrete));
	entityList.push_back(new Entity(mesh1, materialWhiteConcrete));
	entityList.push_back(new Entity(mesh2, materialWhiteRustyMetal));
	
	//temporary just to make all entities visible
	entityList[1]->GetTransform()->MoveAbsolute(0.0f, -4.0f, 0.0f);
	entityList[2]->GetTransform()->MoveAbsolute(0.0f, 4.0f, 0.0f);
	entityList[3]->GetTransform()->MoveAbsolute(-4.0f, 0.0f, 0.0f);
	entityList[4]->GetTransform()->MoveAbsolute(4.0f, 0.0f, 0.0f);

	ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
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
		entityList[i]->GetTransform()->Rotate(0, 0, .1 * deltaTime);
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
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

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

	//creating buffer struct
	VertexShaderExternalData vsData;
	vsData.colorTint	= XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.world = XMFLOAT4X4(); //idk what to put here

	//draw entities
	for (int i = 0; i < entityList.size(); i++)
	{
		entityList[i]->Draw(context, camera, ambient);
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("directionalLight1", &directionalLight1, sizeof(directionalLight1));
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("directionalLight2", &directionalLight2, sizeof(directionalLight2));
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("directionalLight3", &directionalLight3, sizeof(directionalLight3));
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("pointLight1", &pointLight1, sizeof(pointLight1));
		entityList[i]->GetMaterial()->GetPixelShader()->SetData("pointLight2", &pointLight2, sizeof(pointLight2));
	}


	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}