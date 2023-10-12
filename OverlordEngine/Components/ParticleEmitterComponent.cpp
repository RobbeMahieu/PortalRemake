#include "stdafx.h"
#include "ParticleEmitterComponent.h"
#include "Misc/ParticleMaterial.h"

ParticleMaterial* ParticleEmitterComponent::m_pParticleMaterial{};

ParticleEmitterComponent::ParticleEmitterComponent(const std::wstring& assetFile, const ParticleEmitterSettings& emitterSettings, UINT particleCount):
	m_ParticlesArray(new Particle[particleCount]),
	m_ParticleCount(particleCount), //How big is our particle buffer?
	m_MaxParticles(particleCount), //How many particles to draw (max == particleCount)
	m_AssetFile(assetFile),
	m_EmitterSettings(emitterSettings)
{
	m_enablePostDraw = true; //This enables the PostDraw function for the component
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	delete[] m_ParticlesArray;
	m_pVertexBuffer->Release();
}

void ParticleEmitterComponent::Initialize(const SceneContext& sceneContext)
{
	if (!m_pParticleMaterial) {
		m_pParticleMaterial = MaterialManager::Get()->CreateMaterial<ParticleMaterial>();
	}

	CreateVertexBuffer(sceneContext);

	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);
}

void ParticleEmitterComponent::CreateVertexBuffer(const SceneContext& sceneContext)
{
	if (m_pVertexBuffer) {
		m_pVertexBuffer->Release();
	}

	D3D11_BUFFER_DESC buffDesc{};
	buffDesc.Usage = D3D11_USAGE_DYNAMIC;
	buffDesc.ByteWidth = UINT(sizeof(VertexParticle) * m_ParticleCount);
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffDesc.MiscFlags = 0;
	
	HANDLE_ERROR(sceneContext.d3dContext.pDevice->CreateBuffer(&buffDesc, nullptr, &m_pVertexBuffer));
}

void ParticleEmitterComponent::Update(const SceneContext& sceneContext)
{
	float elapsedTime = sceneContext.pGameTime->GetElapsed();
	float particleInterval = (m_EmitterSettings.maxEnergy) / m_ParticleCount;
	m_LastParticleSpawn += elapsedTime;

	m_ActiveParticles = 0;
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	HANDLE_ERROR(sceneContext.d3dContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	VertexParticle* pBuffer = reinterpret_cast<VertexParticle*>(mappedResource.pData);

	for (UINT i{ 0 }; i < m_ParticleCount; ++i) {
		
		// Update active particles
		if (m_ParticlesArray[i].isActive) {
			UpdateParticle(m_ParticlesArray[i], elapsedTime);
		}

		// Spawn new particles
		if (!m_ParticlesArray[i].isActive && m_LastParticleSpawn >= particleInterval) {
			//m_LastParticleSpawn = 0;
			m_LastParticleSpawn -= particleInterval;
			SpawnParticle(m_ParticlesArray[i]);
		}

		// Add active particles to vertex buffer
		if (m_ParticlesArray[i].isActive) {
			pBuffer[m_ActiveParticles] = m_ParticlesArray[i].vertexInfo;
			++m_ActiveParticles;
		}
	}

	sceneContext.d3dContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::UpdateParticle(Particle& p, float elapsedTime) const
{
	if (!p.isActive) {
		return;
	}

	// Update lifetime
	p.currentEnergy -= elapsedTime;
	if (p.currentEnergy <= 0) {
		p.isActive = false;
		return;
	}

	// Update position
	XMStoreFloat3(&p.vertexInfo.Position, XMLoadFloat3(&p.vertexInfo.Position) + (XMLoadFloat3(&m_EmitterSettings.velocity) * elapsedTime));

	float lifePercent = p.currentEnergy / p.totalEnergy;

	// Update color
	p.vertexInfo.Color = m_EmitterSettings.color;
	p.vertexInfo.Color.w *= lifePercent;

	// Update size (lerp)
	p.vertexInfo.Size = p.initialSize + (1 - lifePercent) * (p.sizeChange - 1) * p.initialSize;
}

void ParticleEmitterComponent::SpawnParticle(Particle& p)
{
	p.isActive = true;

	p.totalEnergy = MathHelper::randF(m_EmitterSettings.minEnergy, m_EmitterSettings.maxEnergy);
	p.currentEnergy = p.totalEnergy;

	// Random position
	XMVECTOR randomDirection = XMVECTOR{ 1,0,0 };
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(MathHelper::randF(-XM_PI, XM_PI), MathHelper::randF(-XM_PI, XM_PI), MathHelper::randF(-XM_PI, XM_PI));
	randomDirection = XMVector3TransformNormal(randomDirection, rotationMatrix);

	float distance = MathHelper::randF(m_EmitterSettings.minEmitterRadius, m_EmitterSettings.maxEmitterRadius);
	XMStoreFloat3(&p.vertexInfo.Position, randomDirection * distance + XMLoadFloat3(&m_pGameObject->GetTransform()->GetPosition()));

	// Random size
	p.initialSize = MathHelper::randF(m_EmitterSettings.minSize, m_EmitterSettings.maxSize);
	p.vertexInfo.Size = p.initialSize;	
	p.sizeChange = MathHelper::randF(m_EmitterSettings.minScale, m_EmitterSettings.maxScale);

	// Random rotation
	p.vertexInfo.Rotation = MathHelper::randF(-XM_PI, XM_PI);

	// Color
	p.vertexInfo.Color = m_EmitterSettings.color;

}

void ParticleEmitterComponent::PostDraw(const SceneContext& sceneContext)
{
	m_pParticleMaterial->SetVariable_Matrix(L"gWorldViewProj", &sceneContext.pCamera->GetViewProjection()._11);
	m_pParticleMaterial->SetVariable_Matrix(L"gViewInverse", &sceneContext.pCamera->GetViewInverse()._11);
	m_pParticleMaterial->SetVariable_Texture(L"gParticleTexture", m_pParticleTexture);

	MaterialTechniqueContext techniqueContext = m_pParticleMaterial->GetTechniqueContext();

	sceneContext.d3dContext.pDeviceContext->IASetInputLayout(techniqueContext.pInputLayout);

	//Set Primitive Topology
	sceneContext.d3dContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Vertex buffer
	unsigned int offset = 0;
	unsigned int stride = sizeof(VertexParticle);
	sceneContext.d3dContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//DRAW
	auto tech = techniqueContext.pTechnique;
	D3DX11_TECHNIQUE_DESC techDesc{};

	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, sceneContext.d3dContext.pDeviceContext);
		sceneContext.d3dContext.pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}

void ParticleEmitterComponent::DrawImGui()
{
	if(ImGui::CollapsingHeader("Particle System"))
	{
		ImGui::SliderUInt("Count", &m_ParticleCount, 0, m_MaxParticles);
		ImGui::InputFloatRange("Energy Bounds", &m_EmitterSettings.minEnergy, &m_EmitterSettings.maxEnergy);
		ImGui::InputFloatRange("Size Bounds", &m_EmitterSettings.minSize, &m_EmitterSettings.maxSize);
		ImGui::InputFloatRange("Scale Bounds", &m_EmitterSettings.minScale, &m_EmitterSettings.maxScale);
		ImGui::InputFloatRange("Radius Bounds", &m_EmitterSettings.minEmitterRadius, &m_EmitterSettings.maxEmitterRadius);
		ImGui::InputFloat3("Velocity", &m_EmitterSettings.velocity.x);
		ImGui::ColorEdit4("Color", &m_EmitterSettings.color.x, ImGuiColorEditFlags_NoInputs);
	}
}