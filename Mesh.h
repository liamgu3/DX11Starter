#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Vertex.h"
#include <memory>

class Mesh
{

	//replace pointers with shared_ptr when I figure out how to do those
public:
	Mesh(
		Vertex* vertices, 
		int vertexCount, 
		unsigned int* indices, 
		int indexCount, 
		Microsoft::WRL::ComPtr<ID3D11Device> device, 
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);

	Mesh(const char* objFile, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	~Mesh();

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	int GetIndexCount();
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	void Draw();

private:
	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
	int _indexCount;
};

