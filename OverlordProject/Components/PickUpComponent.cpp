#include "stdafx.h"
#include "PickUpComponent.h"

PickUpComponent::PickUpComponent()
	: BaseComponent()
{}

void PickUpComponent::Initialize(const SceneContext& sceneContext) {

	auto inputAction = InputAction(PickUpObject, InputState::pressed, 'E');
	sceneContext.pInput->AddInputAction(inputAction);
}

void PickUpComponent::Update(const SceneContext& sceneContext) {
	
	// Update picked up object
	if (m_pPickedUpObject) {
		m_pPickedUpObject->GetTransform()->Translate(GetTransform()->GetWorldPosition());
	}

	if (!m_CanPickup) { return; }

	if (sceneContext.pInput->IsActionTriggered(Actions::PickUpObject)) {
		if (m_pPickedUpObject) {
			Drop();
		}
		else {
			Pickup();
		}
	}
}

void PickUpComponent::EnablePickUp(bool canPickup) {
	m_CanPickup = canPickup;
}

void PickUpComponent::Reset() {
	if (m_pPickedUpObject) {
		Drop();
	}
}

void PickUpComponent::Drop() {
	m_pPickedUpObject = nullptr;
}

void PickUpComponent::Pickup() {
	
	PxVec3 direction{ PhysxHelper::ToPxVec3(GetScene()->GetActiveCamera()->GetTransform()->GetForward())};
	PxVec3 start{ PhysxHelper::ToPxVec3(GetScene()->GetActiveCamera()->GetTransform()->GetWorldPosition()) };

	PxRaycastBuffer  hitResult{};
	PxQueryFilterData hitGroups{};
	hitGroups.data.word0 = static_cast<UINT32>(CollisionGroup::Group1);
	GetScene()->GetPhysxProxy()->Raycast(start, direction, 5.0f, hitResult, PxHitFlag::eDEFAULT, hitGroups);

	if (!hitResult.hasAnyHits()) { return; }

	RigidBodyComponent* rigidbodyComponent{ reinterpret_cast<RigidBodyComponent*>(hitResult.block.actor->userData) };
	if (rigidbodyComponent) {
		m_pPickedUpObject = rigidbodyComponent->GetGameObject();
	}
}