#include "stdafx.h"
#include "PortalMaterial.h"

PortalMaterial::PortalMaterial()
	: Material<PortalMaterial>(L"Effects/PortalShader.fx")
{
}

void PortalMaterial::InitializeEffectVariables()
{

}

void PortalMaterial::UpdatePortalSurface(ID3D11ShaderResourceView* pSRV) {

	SetVariable_Texture(L"gDiffuseMap", pSRV);
}

void PortalMaterial::SetPlayerCameraVariable(XMMATRIX CameraWVP) {
	SetVariable_Matrix(L"gPlayerWorldViewProj", reinterpret_cast<float*>(&CameraWVP));
}