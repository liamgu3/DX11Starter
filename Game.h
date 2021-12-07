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
#include "WICTextureLoader.h"
#include "Sky.h"
#include "DDSTextureLoader.h"

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
	void RenderShadowMap();

	
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

	std::shared_ptr<SimpleVertexShader> shadowVS;

	std::shared_ptr<SimplePixelShader> skyPixelShader;
	std::shared_ptr<SimpleVertexShader> skyVertexShader;


	//meshes, replace with shared_ptr when I figure out how to do those
	std::shared_ptr<Mesh> mesh0;
	std::shared_ptr<Mesh> mesh1;
	std::shared_ptr<Mesh> mesh2;
	std::shared_ptr<Mesh> mesh3;

	//for holding entities
	std::vector<Entity*> entityList;

	//Camera
	Camera* camera;

	//materials
	//Material* materialRed;
	//Material* materialBlue;
	//Material* materialGreen;
	//Material* materialWhiteRustyMetal;
	//Material* materialWhiteWood;
	//Material* materialWhiteConcrete;
	//Material* materialWhiteSciFiFabric;
	Material* matBronze;
	Material* matCobblestone;
	Material* matFloor;
	Material* matPaint;
	Material* matScratched;

	//Lights
	DirectX::XMFLOAT3 ambient;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;

	//textures - albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedAlbedoSRV;

	//textures - roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedRoughnessSRV;

	//textures - normalMap
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedNormalSRV;

	//textures - metalnessMap
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetalnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorMetalnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetalnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedMetalnessSRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
	Sky* skybox;

	//shadow stuff
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	DirectX::XMFLOAT4X4 shadowViewMatrix;
	DirectX::XMFLOAT4X4 shadowProjectionMatrix;
};

