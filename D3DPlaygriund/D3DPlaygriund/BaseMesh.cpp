#include "pch.h"
#include "BaseMesh.h"

void BaseMesh::Translate(XMFLOAT3 translation)
{
	this->m_Translation = translation;
	this->m_worldmatrix = XMMatrixTranslation(translation.x, translation.y, translation.z);
}

void BaseMesh::Rotate(XMFLOAT3 rotation)
{
	this->m_Rotation = rotation;
	this->m_worldmatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
}

void BaseMesh::Scale(XMFLOAT3 scale)
{
	this->m_Scale = scale;
	this->m_worldmatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
}

XMMATRIX BaseMesh::GetWorldMatrix()
{
	return this->m_worldmatrix;
}

void BaseMesh::SetWorldMatrix(XMMATRIX worldMatrix)
{
	this->m_worldmatrix = worldMatrix;
}