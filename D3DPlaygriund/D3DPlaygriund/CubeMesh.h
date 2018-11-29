#pragma once

#include "pch.h"
#include "SimpleVertex.h"
#include "DeviceResources.h"
#include "BaseMesh.h"

using namespace DirectX;

struct CubeMesh : public BaseMesh
{
public:
	CubeMesh();
	void Render(ID3D11Device1* device, ID3D11DeviceContext1* context) override;
private:
	SimpleVertex vertices[8];
	WORD indices[36];
};