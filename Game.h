#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Mesh.h"
#include "Entity.h"
#include <vector>
#include "Camera.h"
#include "Material.h"
#include "Lights.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateBasicGeometry();

	
	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBufferVS;
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;


	//meshes, replace with shared_ptr when I figure out how to do those
	std::shared_ptr<Mesh> mesh0;
	std::shared_ptr<Mesh> mesh1;
	std::shared_ptr<Mesh> mesh2;

	//for holding entities
	std::vector<Entity*> entityList;

	//Camera
	Camera* camera;

	//materials
	Material* materialRed;
	Material* materialBlue;
	Material* materialGreen;
	Material* materialWhite;

	//Lights
	DirectX::XMFLOAT3 ambient;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
};

