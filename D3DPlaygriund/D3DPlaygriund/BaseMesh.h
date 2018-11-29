#pragma once

#include "pch.h"
#include "SimpleVertex.h"
#include "DeviceResources.h"

using namespace DirectX;

struct BaseMesh
{
public:
	virtual void Render(ID3D11Device1* device, ID3D11DeviceContext1* context) = 0;
	virtual void Translate(XMFLOAT3 translation);
	virtual void Rotate(XMFLOAT3 rotation);
	virtual void Scale(XMFLOAT3 scale);
	virtual XMMATRIX GetWorldMatrix();
	virtual void SetWorldMatrix(XMMATRIX worldMatrix);
protected:
	XMMATRIX m_worldmatrix;
	XMFLOAT3 m_Translation;
	XMFLOAT3 m_Rotation;
	XMFLOAT3 m_Scale;
};