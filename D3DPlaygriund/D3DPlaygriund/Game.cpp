//
// Game.cpp
//

#pragma once

#include "pch.h"
#include "Game.h"
#include "SimpleVertex.h"
#include "ConstantBuffer.h"
#include "ConstantBufferChangesEveryFrame.h"
#include "ConstantBufferChangesEveryPrimitive.h"
#include "ConstantBufferChangesOnResize.h"
#include <vector>
#include "CubeMesh.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

#pragma region Constructor
Game::Game() noexcept(false)
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
}
#pragma endregion

#pragma region Initialization
// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	m_deviceResources->CreateVertexLayout();

	CreateConstantBuffers();

	InitializeTransformationMatricies(width, height);

	SetChangesOnResizeBuffer();

	SetShaders();

	GetGameObjects();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

void Game::GetGameObjects()
{
	this->meshes.push_back(new CubeMesh());
}

void Game::SetShaders()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	context->VSSetShader(m_deviceResources->GetVertexShader(), NULL, 0);
	context->PSSetShader(m_deviceResources->GetPixelShader(), NULL, 0);
}

void Game::CreateConstantBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();

	//Changes every primitive buffer
	D3D11_BUFFER_DESC bdep;
	ZeroMemory(&bdep, sizeof(bdep));
	bdep.Usage = D3D11_USAGE_DEFAULT;
	bdep.ByteWidth = sizeof(ConstantBufferChangesEveryPrimitive);
	bdep.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdep.CPUAccessFlags = 0;
	if (FAILED(device->CreateBuffer(&bdep, NULL, &g_pConstantBufferChangesEveryPrimitive)))
		return;

	//Changes every frame buffer
	D3D11_BUFFER_DESC bdef;
	ZeroMemory(&bdef, sizeof(bdef));
	bdef.Usage = D3D11_USAGE_DEFAULT;
	bdef.ByteWidth = sizeof(ConstantBufferChangesEveryFrame);
	bdef.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdef.CPUAccessFlags = 0;
	if (FAILED(device->CreateBuffer(&bdef, NULL, &g_pConstantBufferChangesEveryFrame)))
		return;

	//Chnages on resize buffer
	D3D11_BUFFER_DESC bdor;
	ZeroMemory(&bdor, sizeof(bdor));
	bdor.Usage = D3D11_USAGE_DEFAULT;
	bdor.ByteWidth = sizeof(ConstantBufferChangesOnResize);
	bdor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdor.CPUAccessFlags = 0;
	if (FAILED(device->CreateBuffer(&bdor, NULL, &g_pConstantBufferChangesOnResize)))
		return;
}

void Game::InitializeTransformationMatricies(int width, int height)
{

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);
}

void Game::SetChangesOnResizeBuffer()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	ConstantBufferChangesOnResize cb;
	cb.mProjection = XMMatrixTranspose(m_Projection);
	context->UpdateSubresource(g_pConstantBufferChangesOnResize, 0, NULL, &cb, 0, 0);
	context->VSSetConstantBuffers(2, 1, &g_pConstantBufferChangesOnResize);
}

void Game::SetChangesEveryFrameBuffer()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	ConstantBufferChangesEveryFrame cb;
	cb.mView = XMMatrixTranspose(m_View);
	context->UpdateSubresource(g_pConstantBufferChangesEveryFrame, 0, NULL, &cb, 0, 0);
	context->VSSetConstantBuffers(1, 1, &g_pConstantBufferChangesEveryFrame);
}

void Game::SetChangesEveryPrimBuffer(XMMATRIX worldMatrix)
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	ConstantBufferChangesEveryPrimitive cb;
	cb.mWorld = XMMatrixTranspose(worldMatrix);
	context->UpdateSubresource(g_pConstantBufferChangesEveryPrimitive, 0, NULL, &cb, 0, 0);
	context->VSSetConstantBuffers(0, 1, &g_pConstantBufferChangesEveryPrimitive);
}
#pragma endregion

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

	// Update our time
	static float t = 0.0f;
	static ULONGLONG timeStart = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	t = (timeCur - timeStart) / 1000.0f;

	//Iterate all game objects and execute game logic
	for (int i = 0; i < meshes.size(); i++)
	{
		meshes.at(i)->SetWorldMatrix(XMMatrixRotationRollPitchYaw(t, t, t));
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	//Update every frame buffer 
	SetChangesEveryFrameBuffer();

	//Render all meshes
	for (int i = 0; i < meshes.size(); i++)
	{
		SetChangesEveryPrimBuffer(meshes.at(i)->GetWorldMatrix());
		meshes.at(i)->Render(m_deviceResources->GetD3DDevice(), context);
	}

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);
	
	// Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);
	SetChangesOnResizeBuffer();
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
