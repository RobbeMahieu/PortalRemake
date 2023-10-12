#include "stdafx.h"
#include "ShadowMapRenderer.h"
#include "Misc/ShadowMapMaterial.h"

ShadowMapRenderer::~ShadowMapRenderer()
{
	SafeDelete(m_pShadowRenderTarget)
}

void ShadowMapRenderer::Initialize()
{
	RENDERTARGET_DESC desc{};
	desc.enableColorBuffer = false;
	desc.enableDepthSRV = true;
	desc.width = m_GameContext.windowWidth;
	desc.height = m_GameContext.windowHeight;

	m_pShadowRenderTarget = new RenderTarget(m_GameContext.d3dContext);
	m_pShadowRenderTarget->Create(desc);

	m_pShadowMapGenerator = MaterialManager::Get()->CreateMaterial<ShadowMapMaterial>();
	m_GeneratorTechniqueContexts[int(ShadowGeneratorType::Static)] = m_pShadowMapGenerator->GetTechniqueContext(int(ShadowGeneratorType::Static));
	m_GeneratorTechniqueContexts[int(ShadowGeneratorType::Skinned)] = m_pShadowMapGenerator->GetTechniqueContext(int(ShadowGeneratorType::Skinned));
}

void ShadowMapRenderer::UpdateMeshFilter(const SceneContext& sceneContext, MeshFilter* pMeshFilter) const
{
	ShadowGeneratorType type{ (pMeshFilter->HasAnimations()) ? ShadowGeneratorType::Skinned : ShadowGeneratorType::Static };
	MaterialTechniqueContext technique{ m_GeneratorTechniqueContexts[int(type)] };
	pMeshFilter->BuildVertexBuffer(sceneContext, technique.inputLayoutID, technique.inputLayoutSize, technique.pInputLayoutDescriptions);
}

void ShadowMapRenderer::Begin(const SceneContext& sceneContext)
{
	//This function is called once right before we start the Shadow Pass (= generating the ShadowMap)
	//This function is responsible for setting the pipeline into the correct state, meaning
	//	- Making sure the ShadowMap is unbound from the pipeline as a ShaderResource (SRV), so we can bind it as a RenderTarget (RTV)
	//	- Calculating the Light ViewProjection, and updating the relevant Shader variables
	//	- Binding the ShadowMap RenderTarget as Main Game RenderTarget (= Everything we render is rendered to this rendertarget)
	//	- Clear the current (which should be the ShadowMap RT) rendertarget

	//1. Making sure that the ShadowMap is unbound from the pipeline as ShaderResourceView (SRV) is important, because we cannot use the same resource as a ShaderResourceView (texture resource inside a shader) and a RenderTargetView (target everything is rendered too) at the same time. In case this happens, you'll see an error in the output of visual studio - warning you that a resource is still bound as a SRV and cannot be used as an RTV.
	constexpr ID3D11ShaderResourceView* const pSRV[] = { nullptr };
	sceneContext.d3dContext.pDeviceContext->PSSetShaderResources(1, 1, pSRV);

	float viewHeight{ 100.f };
	float viewWidth{ viewHeight * (float(m_GameContext.windowWidth) / m_GameContext.windowHeight) };
	float nearZ{ 0.1f };
	float farZ{ 500.0f };

	//2. Calculate the Light ViewProjection and store in m_LightVP
	XMMATRIX projMatrix = XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);

	XMVECTOR eyePos{ XMLoadFloat4(&sceneContext.pLights->GetDirectionalLight().position) };
	XMVECTOR focalPoint{ eyePos + XMLoadFloat4(&sceneContext.pLights->GetDirectionalLight().direction) };
	XMFLOAT4 up{ 0,1,0,0 };
	
	XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePos, focalPoint, XMLoadFloat4(&up));
	XMStoreFloat4x4(&m_LightVP, viewMatrix * projMatrix);

	//3. Update this matrix (m_LightVP) on the ShadowMapMaterial effect
	m_pShadowMapGenerator->SetVariable_Matrix(L"gLightViewProj", &m_LightVP._11);

	//4. Set the Main Game RenderTarget to m_pShadowRenderTarget (OverlordGame::SetRenderTarget)
	m_GameContext.pGame->SetRenderTarget(m_pShadowRenderTarget);

	//5. Clear the ShadowMap rendertarget (RenderTarget::Clear)
	m_pShadowRenderTarget->Clear();
}

void ShadowMapRenderer::DrawMesh(const SceneContext& sceneContext, MeshFilter* pMeshFilter, const XMFLOAT4X4& meshWorld, const std::vector<XMFLOAT4X4>& meshBones)
{
	//This function is called for every mesh that needs to be rendered on the shadowmap (= cast shadows)

	//1. Figure out the correct ShadowGeneratorType (Static or Skinned)
	ShadowGeneratorType type{ (pMeshFilter->HasAnimations()) ? ShadowGeneratorType::Skinned : ShadowGeneratorType::Static };

	//2. Retrieve the correct TechniqueContext for m_GeneratorTechniqueContexts
	MaterialTechniqueContext technique{ m_GeneratorTechniqueContexts[int(type)] };
	//3. Set the relevant variables on the ShadowMapMaterial
	//		- world of the mesh
	//		- if animated, the boneTransforms
	m_pShadowMapGenerator->SetVariable_Matrix(L"gWorld", &meshWorld._11);
	if (type == ShadowGeneratorType::Skinned) {
		m_pShadowMapGenerator->SetVariable_MatrixArray(L"gBones", &meshBones[0]._11, UINT(meshBones.size()));
	}

	//4. Setup Pipeline for Drawing (Similar to ModelComponent::Draw, but for our ShadowMapMaterial)
	const auto pDeviceContext = sceneContext.d3dContext.pDeviceContext;

	//Set Inputlayout
	pDeviceContext->IASetInputLayout(technique.pInputLayout);

	//Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& subMesh : pMeshFilter->GetMeshes())
	{
		//Set Vertex Buffer
		const UINT offset = 0;
		const auto& vertexBufferData = pMeshFilter->GetVertexBufferData(technique.inputLayoutID, subMesh.id);
		pDeviceContext->IASetVertexBuffers(0, 1, &vertexBufferData.pVertexBuffer, &vertexBufferData.VertexStride,
			&offset);

		//Set Index Buffer
		pDeviceContext->IASetIndexBuffer(subMesh.buffers.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//DRAW
		auto tech = technique.pTechnique;
		D3DX11_TECHNIQUE_DESC techDesc{};

		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			tech->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(subMesh.indexCount, 0, 0);
		}
	}
}

void ShadowMapRenderer::End(const SceneContext&) const
{
	m_GameContext.pGame->SetRenderTarget(nullptr);
}

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowMap() const
{
	return m_pShadowRenderTarget->GetDepthShaderResourceView();
}

void ShadowMapRenderer::Debug_DrawDepthSRV(const XMFLOAT2& position, const XMFLOAT2& scale, const XMFLOAT2& pivot) const
{
	if (m_pShadowRenderTarget->HasDepthSRV())
	{
		SpriteRenderer::Get()->DrawImmediate(m_GameContext.d3dContext, m_pShadowRenderTarget->GetDepthShaderResourceView(), position, XMFLOAT4{ Colors::White }, pivot, scale);

		//Remove from Pipeline
		constexpr ID3D11ShaderResourceView* const pSRV[] = { nullptr };
		m_GameContext.d3dContext.pDeviceContext->PSSetShaderResources(0, 1, pSRV);
	}
}
