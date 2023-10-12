#include "stdafx.h"
#include "PortalComponent.h"
#include <functional>
using namespace std::placeholders;

#include "Materials/PortalMaterial.h"
#include "Materials/DiffuseMaterial.h"
#include "Prefabs/Character.h"

PortalComponent::PortalComponent(GameObject* HUD, bool isBlue) 
	: BaseComponent() 
	, m_pHUD{ HUD }
	, m_IsBlue{ isBlue }
{
	SoundManager::Get()->GetSystem()->createSound("Resources/Sounds/teleport.mp3", FMOD_LOOP_OFF, 0, &m_pTeleportSFX);
}

PortalComponent::~PortalComponent() {
	SafeDelete(m_pRenderTarget);
	SafeDelete(m_pShaderTarget);
}

void PortalComponent::Initialize(const SceneContext& sceneContext) {
	
	// Portal render target
	RENDERTARGET_DESC desc{};
	desc.enableColorBuffer = true;
	desc.enableColorSRV = true;
	desc.width = int(sceneContext.windowWidth);
	desc.height = int(sceneContext.windowHeight);

	m_pRenderTarget = new RenderTarget(sceneContext.d3dContext);
	m_pRenderTarget->Create(desc);

	m_pShaderTarget = new RenderTarget(sceneContext.d3dContext);
	m_pShaderTarget->Create(desc);

	// Materials
	m_pPortalMaterial = MaterialManager::Get()->CreateMaterial<PortalMaterial>();
	auto pTrimMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial>();
	std::wstring path{ m_IsBlue ? L"Textures/portalTrimBlue.png" : L"Textures/portalTrimOrange.png" };
	pTrimMaterial->SetDiffuseTexture(path);

	// Portal Mesh
	m_pPortalPlane = m_pGameObject->AddChild(new GameObject);
	const auto pModel = m_pPortalPlane->AddComponent(new ModelComponent(L"Meshes/portal.ovm"));
	pModel->SetMaterial(m_pPortalMaterial);

	const auto pTrimModel = m_pPortalPlane->AddComponent(new ModelComponent(L"Meshes/portalTrim.ovm"));
	pTrimModel->SetMaterial(pTrimMaterial);

	m_pPortalPlane->GetTransform()->Rotate(90, 0, 0);

	// Portal plane trigger
	const auto pDefaultPxMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);
	m_pPortalTrigger = m_pGameObject->AddComponent(new RigidBodyComponent(true));
	const auto pPxTriggerCollider = ContentManager::Load<PxConvexMesh>(L"Meshes/portal.ovpc");
	m_pPortalTrigger->AddCollider(PxConvexMeshGeometry(pPxTriggerCollider), *pDefaultPxMaterial, true, PxTransform(0,0,0, PhysxHelper::ToPxQuat(m_pPortalPlane->GetTransform()->GetRotation())));
	m_pPortalTrigger->SetCollisionIgnoreGroups(CollisionGroup::Group0 | CollisionGroup::Group3);
	
	m_pGameObject->SetOnTriggerCallBack(std::bind(&PortalComponent::OnTrigger, this, _1, _2, _3));

	// Portal Collision
	const auto pPxColliderMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/portalCollision.ovpt");
	m_pPortalTrigger->AddCollider(PxTriangleMeshGeometry(pPxColliderMesh), *pDefaultPxMaterial, false, PxTransform(0, 0, 0, PhysxHelper::ToPxQuat(m_pPortalPlane->GetTransform()->GetRotation())));
	m_pPortalTrigger->SetCollisionGroup(CollisionGroup::Group3);
	m_pPortalTrigger->SetCollisionIgnoreGroups(CollisionGroup::Group0 | CollisionGroup::Group3);

	ActivatePortal(false);
}

void PortalComponent::LinkPortal(TransformComponent* otherPortal, CameraComponent* playerCamera, CameraComponent* portalCamera) {
	
	m_pOtherPortal = otherPortal;
	m_pPlayerCamera = playerCamera;
	m_pPortalCamera = portalCamera;
}

void PortalComponent::BeginPortalRender(const SceneContext& sceneContext, OverlordGame* game) {

	// Disable other portal
	GetScene()->RemoveChild(m_pOtherPortal->GetGameObject());

	// Make sure the view target is unbound from pixel shader
	constexpr ID3D11ShaderResourceView* const pSRV[] = { nullptr };
	sceneContext.d3dContext.pDeviceContext->PSSetShaderResources(0, 1, pSRV);

	// Set view target as main render target
	game->SetRenderTarget(m_pRenderTarget);
	m_pRenderTarget->Clear();

	// Set the camera to the correct position
	m_pSceneCamera = sceneContext.pCamera;
	GetScene()->SetActiveCamera(m_pPortalCamera);

	XMVECTOR position{};
	XMVECTOR rotation{};
	GetShadowLocation(m_pPlayerCamera->GetGameObject(), position, rotation);
	m_pPortalCamera->GetTransform()->Translate(position);
	m_pPortalCamera->GetTransform()->Rotate(rotation);
	
}

void PortalComponent::EndPortalRender(OverlordGame* game) {
	
	// Reset the camera
	GetScene()->SetActiveCamera(m_pSceneCamera);

	// Return to default render target
	game->SetRenderTarget(nullptr);

	// Update shader render target
	std::swap(m_pRenderTarget, m_pShaderTarget);

	// Update the Portal Material
	m_pPortalMaterial->UpdatePortalSurface(m_pShaderTarget->GetColorShaderResourceView());

	auto world = XMLoadFloat4x4(&m_pPortalPlane->GetTransform()->GetWorld());
	auto viewProj = XMLoadFloat4x4(&m_pSceneCamera->GetViewProjection());
	auto wvp = world * viewProj;
	m_pPortalMaterial->SetPlayerCameraVariable(wvp);

	// Enable other portal
	GetScene()->AddChild(m_pOtherPortal->GetGameObject());
}

void PortalComponent::Update(const SceneContext& /*sceneContext*/) {

	// Remove not collding object (since leave does not work and groups can't be changed in a callback)
	for (auto& object : m_NotColliding) {
		m_CollidingObjects.erase(object);
		
		// Player
		ControllerComponent* controller = object->GetComponent<ControllerComponent>();
		if (controller) {
			controller->SetCollisionIgnoreGroup(CollisionGroup::None);
		}

		//Objects
		RigidBodyComponent* rigidbody = object->GetComponent<RigidBodyComponent>();
		if (rigidbody) {
			rigidbody->SetCollisionIgnoreGroups(CollisionGroup::None);
		}
	}
	m_NotColliding.clear();

	for (auto& object : m_CollidingObjects) {
		// Player
		ControllerComponent* controller = object.first->GetComponent<ControllerComponent>();
		if (controller) {
			controller->SetCollisionIgnoreGroup(CollisionGroup::Group0 | CollisionGroup::Group4);
		}

		//Objects
		RigidBodyComponent* rigidbody = object.first->GetComponent<RigidBodyComponent>();
		if (rigidbody) {
			rigidbody->SetCollisionIgnoreGroups(CollisionGroup::Group0 | CollisionGroup::Group4);
		}

		m_NotColliding.insert(object.first);
	}

}

void PortalComponent::OnTrigger(GameObject* pTriggerObject, GameObject* pOtherObject , PxTriggerAction action)
{
	if (pTriggerObject == m_pGameObject) {
		
		if (action == PxTriggerAction::ENTER) {
			
			float previousValue = m_CollidingObjects[pOtherObject];
			XMStoreFloat(&m_CollidingObjects[pOtherObject], XMVector3Dot((XMLoadFloat3(&pOtherObject->GetTransform()->GetPosition()) - XMLoadFloat3(&GetTransform()->GetPosition())), XMLoadFloat3(&GetTransform()->GetForward())));

			if (previousValue * m_CollidingObjects[pOtherObject] < 0) {
				// TELEPORT!
				XMVECTOR position{};
				XMVECTOR rotation{};
				GetShadowLocation(pOtherObject, position, rotation);
				pOtherObject->GetTransform()->Translate(position);
				pOtherObject->GetTransform()->Rotate(rotation);

				// Sound
				SoundManager::Get()->GetSystem()->playSound(m_pTeleportSFX, 0, false, 0);

			}
			else {
				m_NotColliding.erase(pOtherObject);
			}
		}
	}
}

void PortalComponent::GetShadowLocation(const GameObject* object, XMVECTOR& position, XMVECTOR& rotation) const {
	if (m_pOtherPortal) {

		XMVECTOR objectRotation{ XMLoadFloat4(&object->GetTransform()->GetWorldRotation()) };
		XMVECTOR objectPosition{ XMLoadFloat3(&object->GetTransform()->GetWorldPosition()) };

		XMVECTOR relativeRotation{ XMQuaternionMultiply(objectRotation, XMQuaternionInverse(XMLoadFloat4(&GetTransform()->GetWorldRotation()))) };
		XMVECTOR relativePosition{ objectPosition - XMLoadFloat3(&GetTransform()->GetWorldPosition()) };

		//  Rotate object to correct orientation
		rotation = XMQuaternionMultiply(XMQuaternionMultiply(relativeRotation, XMLoadFloat4(&m_pOtherPortal->GetTransform()->GetWorldRotation())), XMQuaternionRotationNormal(XMLoadFloat3(&m_pOtherPortal->GetUp()), PxPi));

		//  Translate object to correct position
		XMVECTOR relativePortalRotation{ XMQuaternionMultiply(XMQuaternionMultiply(XMLoadFloat4(&m_pOtherPortal->GetTransform()->GetWorldRotation()), XMQuaternionInverse(XMLoadFloat4(&GetTransform()->GetWorldRotation()))), XMQuaternionRotationNormal(XMLoadFloat3(&m_pOtherPortal->GetUp()), PxPi)) };
		position =  XMLoadFloat3(&m_pOtherPortal->GetTransform()->GetWorldPosition()) + XMVector3Rotate(relativePosition, relativePortalRotation);
	}
}

void PortalComponent::DrawImGui(int index) {
	
	std::string tag{ "Portal " + std::to_string(index) + " Camera" };

	bool isActive = m_pPortalCamera->IsActive();
	if (ImGui::Checkbox(tag.c_str(), &isActive))
	{
		m_pPortalCamera->SetActive(isActive);
	}
	
}

void PortalComponent::ClearRenderTargets() {
	m_pRenderTarget->Clear();
	m_pShaderTarget->Clear();
}

void PortalComponent::ActivatePortal(bool active){
	m_PortalActive = active;
	m_pHUD->SetActive(active);
}