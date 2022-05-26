#pragma once
#include <d3d12.h>
#include <math.h>
#include <DirectXMath.h>

class DrawingMesh
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 pos; // xyz���W
		DirectX::XMFLOAT2 uv;  // uv���W
	};

	DrawingMesh(ID3D12Device* device, Vertex vertex[], int verticesCount);
	~DrawingMesh();
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList);

	// �C���f�b�N�X�f�[�^
	unsigned short indices[6] = {
	0, 1, 2, // �O�p�`1��
	1, 2, 3, // �O�p�`2��
	};
	HRESULT result;
	D3D12_HEAP_PROPERTIES heapProp{};
	ID3D12PipelineState* pipelineState = nullptr;
	ID3D12RootSignature* rootSignature = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	ID3D12Resource* constBuffMaterial = nullptr;
	ID3D12DescriptorHeap* srvHeap = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView{};
};