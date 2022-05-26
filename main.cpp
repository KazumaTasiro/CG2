#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dinput.h>
#include <math.h>
#include <DirectXTex.h>
#include"Triangle.h"
#define DIRECTINPUT_VERSION  0x0800 //DirectInput�̃o�[�W�����w��


using namespace DirectX;
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

const float PI = 3.141592f;
//�E�B���h�E�v���V�[�W��
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//���b�Z�[�W�ɉ����ăQ�[���ŗL�̏������s��
	switch (msg) {
		//�E�B���h�E���j�󂳂ꂽ
	case WM_DESTROY:
		//OS�ɑ΂��āA�A�v���̏I����`����
		PostQuitMessage(0);
		return 0;
	}

	//�W���̃��b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//�R���\�[���̕����o��
	OutputDebugStringA("Hello,DirectX!!\n");

	//�E�B���h�E�T�C�Y
	const int window_width = 1280;
	const int window_height = 720;

	//�E�B���h�E�N���X�̐ݒ�
	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);			//�E�B���h�E�v���V�[�W����ݒ�
	w.lpfnWndProc = (WNDPROC)WindowProc;	//�E�B���h�E�v���V�[�W����ݒ�
	w.lpszClassName = L"DirectXGame";		//�E�B���h�E�N���X��
	w.hInstance = GetModuleHandle(nullptr); //�E�B���h�E�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW);//�J�[�\���w��

	//�E�B���h�E�N���X��OS�ɓo�^����
	RegisterClassEx(&w);
	//�E�B���h�E�T�C�Y�o�@X���W�@Y���W�@�����@�c���p
	RECT wrc = { 0,0,window_width,window_height };
	//�����ŃT�C�Y�𒲐�����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,
		L"DirectXGame",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	//�E�B���h�E��\����Ԃɂ���
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};//���b�Z�[�W

	//DIrectX���������������܂���
#ifdef _DEBUG
//�f�o�b�O���C���[���I����
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif
	HRESULT result;
	ID3D12Device* device = nullptr;
	IDXGIFactory7* dxgiFactory = nullptr;
	IDXGISwapChain4* swapChain = nullptr;
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12DescriptorHeap* rtvHeap = nullptr;

	//DXG�̃t�@�N�g���[�̐���
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//�A�_�v�^�̗񋓗p
	std::vector<IDXGIAdapter4*>adapters;
	//�����œ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter4* tmpAdapter = nullptr;
	//�p�t�H�[�}���X���������̂��珇�ɁA���ׂẴA�_�v�^�[��񋓂���
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}
	//���ƒǂ��ȃA�_�v�^��I�ʂ���
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		//�\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//�f�o�C�X�𗘗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i];
			break;
		}
	}
	//�Ή����x���̔z��
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK) {
			//�f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	assert(SUCCEEDED(result));
	// �R�}���h���X�g�𐶐�
	result = device->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator, nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));
	//�R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//�R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));
	// �X���b�v�`�F�[���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �F���̏���
	swapChainDesc.SampleDesc.Count = 1; // �}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // �o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2; // �o�b�t�@����2�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // �t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// �X���b�v�`�F�[���̐���
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue, hwnd, &swapChainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&swapChain);
	assert(SUCCEEDED(result));
	// �f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // �����_�[�^�[�Q�b�g�r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; // ���\��2��
	// �f�X�N���v�^�q�[�v�̐���
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
	// �o�b�N�o�b�t�@
	std::vector<ID3D12Resource*> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);
	// �X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď�������
	for (size_t i = 0; i < backBuffers.size(); i++) {
		// �X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		// �f�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		// �����\���ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		// �V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		// �����_�[�^�[�Q�b�g�r���[�̐���
		device->CreateRenderTargetView(backBuffers[i], &rtvDesc, rtvHandle);
	}
	// �t�F���X�̐���
	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	//DirectInput�̏�����
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
	//�L�[�{�[�h�f�o�C�X�̐���
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	//���̓f�[�^�`���̃Z�b�g
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	//�r���I���䃌�x���̃Z�b�g
	result = keyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
	//Vertex vertices[] = {
	//	// x      y     z       u     v
	//	{{-0.4f, -0.7f, 0.0f}, {0.0f, 1.0f}}, // ����
	//	{{-0.4f, +0.7f, 0.0f}, {0.0f, 0.0f}}, // ����
	//	{{+0.4f, -0.7f, 0.0f}, {1.0f, 1.0f}}, // �E��
	//	{{+0.4f, +0.7f, 0.0f}, {1.0f, 0.0f}}, // �E��
	//};
	DrawingMesh::Vertex vertex[] = { 
		{ {-0.1f, -0.1f, 0.0f}, {0.0f, 1.0f} },
		{ {-0.1f, +0.1f, 0.0f}, {0.0f, 0.0f} },
		{ {+0.1f, -0.1f, 0.0f}, {1.0f, 1.0f} },
		{ {+0.1f, +0.1f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex2[] = {
		{ {-0.5f, -0.1f, 0.0f}, {0.0f, 1.0f} },
		{ {-0.5f, +0.1f, 0.0f}, {0.0f, 0.0f} },
		{ {-0.4f, -0.1f, 0.0f}, {1.0f, 1.0f} },
		{ {-0.4f, +0.1f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex3[] = {
		{ {-0.5f, +0.4f, 0.0f}, {0.0f, 1.0f} },
		{ {-0.5f, +0.2f, 0.0f}, {0.0f, 0.0f} },
		{ {-0.4f, +0.4f, 0.0f}, {1.0f, 1.0f} },
		{ {-0.4f, +0.2f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex4[] = {
		{ {+0.5f, -0.1f, 0.0f}, {0.0f, 1.0f} },
		{ {+0.5f, +0.1f, 0.0f}, {0.0f, 0.0f} },
		{ {+0.4f, -0.1f, 0.0f}, {1.0f, 1.0f} },
		{ {+0.4f, +0.1f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex5[] = {
		{ {+0.5f, +0.4f, 0.0f}, {0.0f, 1.0f} },
		{ {+0.5f, +0.2f, 0.0f}, {0.0f, 0.0f} },
		{ {+0.4f, +0.4f, 0.0f}, {1.0f, 1.0f} },
		{ {+0.4f, +0.2f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex6[] = {
		{ {+0.7f, -0.1f, 0.0f}, {0.0f, 1.0f} },
		{ {+0.7f, +0.1f, 0.0f}, {0.0f, 0.0f} },
		{ {+0.6f, -0.1f, 0.0f}, {1.0f, 1.0f} },
		{ {+0.6f, +0.1f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex7[] = {
		{ {+0.7f, +0.4f, 0.0f}, {0.0f, 1.0f} },
		{ {+0.7f, +0.2f, 0.0f}, {0.0f, 0.0f} },
		{ {+0.6f, +0.4f, 0.0f}, {1.0f, 1.0f} },
		{ {+0.6f, +0.2f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex8[] = {
		{ {-0.7f, -0.1f, 0.0f}, {0.0f, 1.0f} },
		{ {-0.7f, +0.1f, 0.0f}, {0.0f, 0.0f} },
		{ {-0.6f, -0.1f, 0.0f}, {1.0f, 1.0f} },
		{ {-0.6f, +0.1f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex9[] = {
		{ {-0.7f, +0.4f, 0.0f}, {0.0f, 1.0f} },
		{ {-0.7f, +0.2f, 0.0f}, {0.0f, 0.0f} },
		{ {-0.6f, +0.4f, 0.0f}, {1.0f, 1.0f} },
		{ {-0.6f, +0.2f, 0.0f}, {1.0f, 0.0f} },
	};
	DrawingMesh::Vertex vertex10[] = {
		{ {-0.1f, +0.4f, 0.0f}, {0.0f, 1.0f} },
		{ {-0.1f, +0.2f, 0.0f}, {0.0f, 0.0f} },
		{ {+0.1f, +0.4f, 0.0f}, {1.0f, 1.0f} },
		{ {+0.1f, +0.2f, 0.0f}, {1.0f, 0.0f} },
	};

	DrawingMesh* mesh = new DrawingMesh(device,vertex,_countof(vertex));
	DrawingMesh* mesh2 = new DrawingMesh(device, vertex2, _countof(vertex));
	DrawingMesh* mesh3 = new DrawingMesh(device, vertex3, _countof(vertex));
	DrawingMesh* mesh4 = new DrawingMesh(device, vertex4, _countof(vertex));
	DrawingMesh* mesh5 = new DrawingMesh(device, vertex5, _countof(vertex));
	DrawingMesh* mesh6 = new DrawingMesh(device, vertex6, _countof(vertex));
	DrawingMesh* mesh7 = new DrawingMesh(device, vertex7, _countof(vertex));
	DrawingMesh* mesh8 = new DrawingMesh(device, vertex8, _countof(vertex));
	DrawingMesh* mesh9 = new DrawingMesh(device, vertex9, _countof(vertex));
	DrawingMesh* mesh10 = new DrawingMesh(device, vertex10, _countof(vertex));
	//DIrectX���������������܂�
	// �`�揉��������
	



	//�Q�[�����[�v
	while (true) {
		//���b�Z�[�W������H
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);//�L�[���̓��b�Z�[�W�̏���
			DispatchMessage(&msg);//�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
		}

		// X�{�^���ŏI�����b�Z�[�W��������Q�[�����[�v�𔲂���
		if (msg.message == WM_QUIT) {
			break;
		}
		//DirectX���t���[�������@��������
		//�L�[�{�[�h���̎擾�J�n
		keyboard->Acquire();

		//�S�L�[�̓��͏�Ԃ��擾����

		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key), key);

		//�����̂O�L�[��������Ă�����
		if (key[DIK_0]) {
			OutputDebugStringA("Hit 0\n");//�o�̓E�B���h�E�ɁuHit 0�v�ƕ\��
		}

		//DirectX���t���[�������@�����܂�

		// �o�b�N�o�b�t�@�̔ԍ����擾(2�Ȃ̂�0�Ԃ�1��)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
		// 1.���\�[�X�o���A�ŏ������݉\�ɕύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex]; // �o�b�N�o�b�t�@���w��
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT; // �\����Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; // �`���Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);

		// 2.�`���̕ύX
		// �����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		// 3.��ʃN���A R G B A
		FLOAT clearColor[] = { 0.1f,0.25f, 0.5f,0.0f }; // ���ۂ��F
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		/*if (key[DIK_SPACE]) {
			FLOAT clearColor[] = { 1.0f,0.2f,0.8f };
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}*/

		// 4.�`��R�}���h��������
		// �r���[�|�[�g�ݒ�R�}���h
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;//����
		viewport.Height = window_height;//�c��
		viewport.TopLeftX = 0;//����X
		viewport.TopLeftY = 0;//����Y
		viewport.MinDepth = 0.0f;//�ŏ��[�x�i�O�ł悢�j
		viewport.MaxDepth = 1.0f;//�ő�[�x�i�P�ł悢�j
		// �r���[�|�[�g�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetViewports(1, &viewport);
		//�V�U�[�Z�`
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;									// �؂蔲�����W��
		scissorRect.right = scissorRect.left + window_width;	// �؂蔲�����W�E
		scissorRect.top = 0;									// �؂蔲�����W��
		scissorRect.bottom = scissorRect.top + window_height;	// �؂蔲�����W��
		// �V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetScissorRects(1, &scissorRect);
		mesh->Draw(commandList);
		mesh2->Draw(commandList);
		mesh3->Draw(commandList);
		mesh4->Draw(commandList);
		mesh5->Draw(commandList);
		mesh6->Draw(commandList);
		mesh7->Draw(commandList);
		mesh8->Draw(commandList);
		mesh9->Draw(commandList);
		mesh10->Draw(commandList);

		// 4.�`��R�}���h�����܂�

		// 5.���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; // �`���Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT; // �\����Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);
		// ���߂̃N���[�Y
		result = commandList->Close();
		assert(SUCCEEDED(result));
		// �R�}���h���X�g�̎��s
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, commandLists);
		// ��ʂɕ\������o�b�t�@���t���b�v(���\�̓��ւ�)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));
		// �R�}���h�̎��s������҂�
		commandQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		// �L���[���N���A
		result = cmdAllocator->Reset();
		assert(SUCCEEDED(result));
		// �ĂуR�}���h���X�g�𒙂߂鏀��
		result = commandList->Reset(cmdAllocator, nullptr);
		assert(SUCCEEDED(result));
	}

	//�E�B���h�E�N���X��o�^����
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}