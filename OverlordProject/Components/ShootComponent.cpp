#include "stdafx.h"
#include "ShootComponent.h"
#include "PortalComponent.h"

ShootComponent::ShootComponent(GameObject* pLeft, GameObject* pRight, TransformComponent* playerTransform) 
	: BaseComponent()
	, m_pLeftPortal{ pLeft }
	, m_pRightPortal{ pRight }
	, m_pPlayerTransform{ playerTransform }
	, m_CanShoot{ true }
{
	SoundManager::Get()->GetSystem()->createSound("Resources/Sounds/shoot.mp3", FMOD_LOOP_OFF, 0, &m_pShootSFX);
}

void ShootComponent::Initialize(const SceneContext& sceneContext) {

	auto inputAction = InputAction(ShootLeft, InputState::pressed, -1, VK_LBUTTON);
	sceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(ShootRight, InputState::pressed, -1, VK_RBUTTON);
	sceneContext.pInput->AddInputAction(inputAction);

	m_ParticleSettings.maxEnergy = 1.0f;
	m_ParticleSettings.minEnergy = 0.2f;
	m_ParticleSettings.maxEmitterRadius = 0.1f;
	m_ParticleSettings.minEmitterRadius = 0.0f;
	m_ParticleSettings.maxScale = 0.3f;
	m_ParticleSettings.minScale = 0.1f;
	m_ParticleSettings.minSize = 0.3f;
	m_ParticleSettings.maxSize = 0.7f;
}

void ShootComponent::Update(const SceneContext& sceneContext) {

	if (!m_CanShoot) { return; }

	if (sceneContext.pInput->IsActionTriggered(Actions::ShootLeft)) {
		ShootPortal(m_pLeftPortal);
	}

	if (sceneContext.pInput->IsActionTriggered(Actions::ShootRight)) {
		ShootPortal(m_pRightPortal);
	}

	// Update Particles
	for (auto& shot : m_Shots) {

		float speed{ 70.0f * sceneContext.pGameTime->GetElapsed() };
		XMFLOAT3 newPos{ PhysxHelper::ToXMFLOAT3(PhysxHelper::ToPxVec3(shot.direction) * speed + PhysxHelper::ToPxVec3(shot.particles->GetTransform()->GetPosition())) };
		shot.particles->GetTransform()->Translate(newPos);
		CheckHit(shot);
	}

	// Remove hit shots
	m_Shots.erase(std::remove_if(m_Shots.begin(), m_Shots.end(), [](Shot shot) { return shot.destroy; }), m_Shots.end());
}

void ShootComponent::ShootPortal(GameObject* portal) {

	portal->GetComponent<PortalComponent>()->ActivatePortal(false);
	portal->GetTransform()->Translate(0, -1000, 0);

	// Spawn Shot
	auto particles = GetScene()->SpawnChild(new GameObject);
	particles->GetTransform()->Translate(GetScene()->GetActiveCamera()->GetTransform()->GetWorldPosition());

	m_ParticleSettings.color = (portal->GetComponent<PortalComponent>()->IsBlue()) ? XMFLOAT4(0.0f, 0.4f, 1.0f, 1.0f) : XMFLOAT4(1.0f, 0.6f, 0.2f, 1.0f);

	particles->AddComponent(new ParticleEmitterComponent(L"Textures/gunParticle.png", m_ParticleSettings, 250));
	Shot newShot{};
	newShot.direction = m_pPlayerTransform->GetForward();
	newShot.particles = particles;
	newShot.portal = portal;
	m_Shots.emplace_back(newShot);

	// Sound
	SoundManager::Get()->GetSystem()->playSound(m_pShootSFX, 0, false, 0);
}

void ShootComponent::CheckHit(Shot& shot) {

	PxVec3 direction{ PhysxHelper::ToPxVec3(shot.direction) };
	PxVec3 start{ PhysxHelper::ToPxVec3(shot.particles->GetTransform()->GetPosition()) };

	PxRaycastBuffer  hitResult{};
	PxRaycastBuffer  hitResult2{};
	PxQueryFilterData hitGroups{};
	hitGroups.data.word0 = static_cast<UINT32>(CollisionGroup::Group0);
	GetScene()->GetPhysxProxy()->Raycast(start, direction, 5.0f, hitResult, PxHitFlag::eDEFAULT, hitGroups);
	hitGroups.data.word0 = static_cast<UINT32>(~CollisionGroup::Group2);
	GetScene()->GetPhysxProxy()->Raycast(start, direction, 5.0f, hitResult2, PxHitFlag::eDEFAULT, hitGroups);

	if (!hitResult.hasAnyHits() && !hitResult2.hasAnyHits()) { return; }
	if (hitResult.block.distance > hitResult2.block.distance) { 

		// Despawn particles and shot
		shot.destroy = true;
		GetScene()->DestroyChild(shot.particles);
		return;
	}

	XMFLOAT3 hitPos = PhysxHelper::ToXMFLOAT3(hitResult.block.position + hitResult.block.normal * 0.1f);
	XMFLOAT3 hitNormal = PhysxHelper::ToXMFLOAT3(hitResult.block.normal);

	XMFLOAT3 worldForward{ 0,0,1 };
	XMFLOAT3 worldRight{ 1,0,0 };
	XMVECTOR rotationAxis{ XMVector3Cross(XMLoadFloat3(&worldForward), XMLoadFloat3(&hitNormal)) };
	float vectorLength{};
	XMStoreFloat(&vectorLength, XMVector3Length(rotationAxis));
	if (vectorLength != 1.0f) {
		rotationAxis = XMVector3Cross(XMLoadFloat3(&worldRight), XMLoadFloat3(&hitNormal));
	}

	float angle{};
	XMStoreFloat(&angle, XMVector3AngleBetweenNormals(XMLoadFloat3(&worldForward), XMLoadFloat3(&hitNormal)));
	XMVECTOR rotation{ XMQuaternionRotationNormal(rotationAxis, angle) };

	// Set portal
	shot.portal->GetTransform()->Translate(hitPos);
	shot.portal->GetTransform()->Rotate(rotation);
	shot.portal->GetComponent<PortalComponent>()->ActivatePortal(true);

	// Despawn particles and shot
	shot.destroy = true;
	GetScene()->DestroyChild(shot.particles);
}

void ShootComponent::EnableShooting(bool canShoot) {
	m_CanShoot = canShoot;
}

void ShootComponent::Reset() {
	m_pLeftPortal->GetComponent<PortalComponent>()->ActivatePortal(false);
	m_pLeftPortal->GetTransform()->Translate(0, -1000, 0);

	m_pRightPortal->GetComponent<PortalComponent>()->ActivatePortal(false);
	m_pRightPortal->GetTransform()->Translate(0, -1000, 0);
}