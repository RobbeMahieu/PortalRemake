#include "stdafx.h"
#include "PortalTestScene.h"
#include "Prefabs/SpherePrefab.h"
#include "Prefabs/CubePrefab.h"
#include "Materials/PortalMaterial.h"
#include "Materials/Post/PostVignette.h"
#include "Components/PortalComponent.h"
#include "Materials/DiffuseMaterial.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow_Skinned.h"
#include "Prefabs/Character.h"
#include "Components/ChellAnimator.h"
#include "Components/ShootComponent.h"
#include "Components/IgnorePortalDraw.h"
#include "Components/UIButtonComponent.h"
#include "Components/PickUpComponent.h"

void PortalTestScene::Initialize() {
	m_SceneContext.settings.enableOnGUI = true;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.showInfoOverlay = false;

	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	// Physics material
	const auto pDefaultPxMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.3f);

	// -- Player --
	CharacterDesc characterDesc{ pDefaultPxMaterial };
	characterDesc.actionId_MoveForward = CharacterMoveForward;
	characterDesc.actionId_MoveBackward = CharacterMoveBackward;
	characterDesc.actionId_MoveLeft = CharacterMoveLeft;
	characterDesc.actionId_MoveRight = CharacterMoveRight;
	characterDesc.actionId_Jump = CharacterJump;
	characterDesc.controller.height = 1.70f;
	characterDesc.rotationSpeed = 20.0f;
	characterDesc.maxMoveSpeed = 10.0f;
	characterDesc.controller.stepOffset = 0.2f;

	m_pCharacter = AddChild(new Character(characterDesc));

	// Mesh + Materials
	auto m_pCharacterVisuals = m_pCharacter->AddChild(new GameObject);
	m_pCharacterVisuals->GetTransform()->Scale(0.035f, 0.035f, 0.035f);
	m_pCharacterVisuals->GetTransform()->Rotate(0, 180, 0);
	m_pCharacterVisuals->GetTransform()->Translate(0, -characterDesc.controller.radius-characterDesc.controller.height/2.0f, -0.5f);
	
	const auto pChellEyeL = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	pChellEyeL->SetDiffuseTexture(L"Textures/chell/eyeball_l.png");

	const auto pChellEyeR = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	pChellEyeR->SetDiffuseTexture(L"Textures/chell/eyeball_r.png");

	const auto pChellBody = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	pChellBody->SetDiffuseTexture(L"Textures/chell/chell_body.png");

	const auto pChellFace = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	pChellFace->SetDiffuseTexture(L"Textures/chell/chell_face.png");

	const auto pChellHair = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	pChellHair->SetDiffuseTexture(L"Textures/chell/chell_hair.png");

	const auto pChellModel = m_pCharacterVisuals->AddComponent(new ModelComponent(L"Meshes/chell.ovm"));
	pChellModel->SetMaterial(pChellEyeL,0);
	pChellModel->SetMaterial(pChellEyeR,1);
	pChellModel->SetMaterial(pChellHair,2);
	pChellModel->SetMaterial(pChellFace,3);
	pChellModel->SetMaterial(pChellBody,4);

	// Animation
	m_pCharacter->AddComponent(new ChellAnimator());

	// Collision
	m_pCharacter->GetComponent<ControllerComponent>()->SetCollisionGroup(CollisionGroup(CollisionLayer::Player));

	// -- Portal Gun --
	const auto pGun1 = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pGun1->SetDiffuseTexture(L"Textures/portalGun/portalGun.png");

	const auto pGun2 = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pGun2->SetDiffuseTexture(L"Textures/portalGun/portalGun2.png");

	auto gun = m_pCharacter->AddChild(new GameObject);
	auto gunModel = gun->AddComponent(new ModelComponent(L"Meshes/portalGun.ovm"));
	gunModel->SetMaterial(pGun1, 0);
	gunModel->SetMaterial(pGun2, 1);
	gun->GetTransform()->Scale(0.035f, 0.035f, 0.035f);
	gun->GetTransform()->Rotate(10, 175, 0);
	gun->GetTransform()->Translate(0.12f, 0.3f, -0.15f);

	// Gun for camera
	auto gunForCamera = m_pCharacter->AddChild(new GameObject);
	gunForCamera->AddComponent(new IgnorePortalDraw());
	gunModel = gunForCamera->AddComponent(new ModelComponent(L"Meshes/portalGun.ovm"));
	gunModel->SetMaterial(pGun1, 0);
	gunModel->SetMaterial(pGun2, 1);
	gunForCamera->GetTransform()->Scale(0.035f, 0.035f, 0.035f);
	gunForCamera->GetTransform()->Rotate(10, 175, 0);
	gunForCamera->GetTransform()->Translate(0.22f, 0.75f, 0.50f);

	// -- Test Cube --
	const auto pMetalCubeMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pMetalCubeMaterial->SetDiffuseTexture(L"Textures/metal_box.png");

	m_pCube = AddChild(new GameObject);
	const auto pCubeModel = new ModelComponent(L"Meshes/MetalCube.ovm");
	pCubeModel->SetMaterial(pMetalCubeMaterial);
	m_pCube->AddComponent(pCubeModel);
	m_pCube->GetTransform()->Scale(0.025f, 0.025f, 0.025f);

	// Collision
	const auto pCubeActor = m_pCube->AddComponent(new RigidBodyComponent());
	auto pPxConvexMesh = ContentManager::Load<PxConvexMesh>(L"Meshes/MetalCube.ovpc");
	pCubeActor->AddCollider(PxConvexMeshGeometry(pPxConvexMesh, PxMeshScale(0.025f)), *pDefaultPxMaterial);
	pCubeActor->SetCollisionGroup(CollisionGroup(CollisionLayer::LevelDynamic));

	// -- Level --
	const auto pLevelObj = new GameObject();

	const auto pLevelMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pLevelMaterial->SetDiffuseTexture(L"Textures/ground.png");

	const auto pLevelNonPortalObj = new GameObject();
	const auto pLevelModel = new ModelComponent(L"Meshes/Level/levelSurface.ovm");
	pLevelModel->SetMaterial(pLevelMaterial);

	pLevelNonPortalObj->AddComponent(pLevelModel);
	pLevelNonPortalObj->GetTransform()->Scale(0.2f, 0.2f, 0.2f);

	const auto pLevelWallsMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pLevelWallsMaterial->SetDiffuseTexture(L"Textures/portalWalls.png");

	const auto pLevelPortalObj = new GameObject();
	const auto pLevelWallsModel = new ModelComponent(L"Meshes/Level/portalSurface.ovm");
	pLevelWallsModel->SetMaterial(pLevelWallsMaterial);

	pLevelPortalObj->AddComponent(pLevelWallsModel);
	pLevelPortalObj->GetTransform()->Scale(0.2f, 0.2f, 0.2f);

	// Collision
	const auto pLevelNonPortalActor = pLevelNonPortalObj->AddComponent(new RigidBodyComponent(true));
	const auto pPxLevelTriangleMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/Level/levelSurface.ovpt");
	pLevelNonPortalActor->AddCollider(PxTriangleMeshGeometry(pPxLevelTriangleMesh, PxMeshScale(0.2f)), *pDefaultPxMaterial);
	pLevelNonPortalActor->SetCollisionGroup(CollisionGroup(CollisionLayer::LevelNonPortal));

	const auto pLevelPortalActor = pLevelPortalObj->AddComponent(new RigidBodyComponent(true));
	const auto pPxLevelWallsTriangleMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/Level/portalSurface.ovpt");
	pLevelPortalActor->AddCollider(PxTriangleMeshGeometry(pPxLevelWallsTriangleMesh, PxMeshScale(0.2f)), *pDefaultPxMaterial);
	pLevelPortalActor->SetCollisionGroup(CollisionGroup(CollisionLayer::LevelStatic));

	pLevelObj->AddChild(pLevelNonPortalObj);
	pLevelObj->AddChild(pLevelPortalObj);
	AddChild(pLevelObj);

	// -- HUD -- 
	m_pHUD = AddChild(new GameObject);
	m_pHUD->AddComponent(new IgnorePortalDraw);
	GameObject* crosshairObject = m_pHUD->AddChild(new GameObject);
	auto crosshair = crosshairObject->AddComponent(new SpriteComponent(L"Textures/crosshair.png"));
	crosshair->SetPivot(XMFLOAT2{ 0.5f,0.5f });
	crosshairObject->GetTransform()->Translate(m_SceneContext.windowWidth / 2, m_SceneContext.windowHeight / 2, 0);
	crosshairObject->GetTransform()->Scale(0.3f, 0.3f, 0.3f);

	GameObject* orangeObject = m_pHUD->AddChild(new GameObject);
	auto orange = orangeObject->AddComponent(new SpriteComponent(L"Textures/crossOrange.png"));
	orange->SetPivot(XMFLOAT2{ 0.5f,0.5f });
	orangeObject->GetTransform()->Translate(m_SceneContext.windowWidth / 2, m_SceneContext.windowHeight / 2, 0);
	orangeObject->GetTransform()->Scale(0.3f, 0.3f, 0.3f);
	orangeObject->SetActive(false);

	GameObject* blueObject = m_pHUD->AddChild(new GameObject);
	auto blue = blueObject->AddComponent(new SpriteComponent(L"Textures/crossBlue.png"));
	blue->SetPivot(XMFLOAT2{ 0.5f,0.5f });
	blueObject->GetTransform()->Translate(m_SceneContext.windowWidth / 2, m_SceneContext.windowHeight / 2, 0);
	blueObject->GetTransform()->Scale(0.3f, 0.3f, 0.3f);
	blueObject->SetActive(false);

	// -- Portal 1 --
	GameObject* portal1 = new GameObject();
	portal1->AddComponent(new PortalComponent(blueObject, true));
	AddChild(portal1);
	portal1->GetTransform()->Translate(0, -100, 0);
	m_pPortals.push_back(portal1);
	GameObject* p1Camera = AddChild(new FixedCamera());

	// -- Portal 2 --
	GameObject* portal2 = new GameObject();
	portal2->AddComponent(new PortalComponent(orangeObject, false));
	AddChild(portal2);
	portal2->GetTransform()->Translate(0, -100, 0);
	m_pPortals.push_back(portal2);
	GameObject* p2Camera = AddChild(new FixedCamera());

	portal1->GetComponent<PortalComponent>()->LinkPortal(portal2->GetTransform(), m_pCharacter->GetCamera(), p1Camera->GetComponent<CameraComponent>());
	portal2->GetComponent<PortalComponent>()->LinkPortal(portal1->GetTransform(), m_pCharacter->GetCamera(), p2Camera->GetComponent<CameraComponent>());

	// -- Pause Screen --
	m_pPauseScreen = AddChild(new GameObject());
	m_pPauseScreen->SetActive(false);

	GameObject* background = m_pPauseScreen->AddChild(new GameObject);
	auto image = background->AddComponent(new SpriteComponent(L"Textures/Menu/pauseScreen.png"));
	image->SetPivot(XMFLOAT2{ 0.5f,0.5f });
	background->GetTransform()->Translate(m_SceneContext.windowWidth / 2, m_SceneContext.windowHeight / 2, 0);

	SpriteFont* font = ContentManager::Load<SpriteFont>(L"SpriteFonts/Consolas_32.fnt");

	// Play button 
	GameObject* menuButton = m_pPauseScreen->AddChild(new GameObject);
	m_pMenuButton = menuButton->AddComponent(new UIButtonComponent(L"MAIN MENU", XMFLOAT4{ 100,400,200,30 }, font));

	// Retry button 
	GameObject* retryButton = m_pPauseScreen->AddChild(new GameObject);
	m_pRetryButton = retryButton->AddComponent(new UIButtonComponent(L"RETRY", XMFLOAT4{ 100,450,200,30 }, font));

	// Exit button 
	GameObject* exitButton = m_pPauseScreen->AddChild(new GameObject);
	m_pExitButton = exitButton->AddComponent(new UIButtonComponent(L"EXIT", XMFLOAT4{ 100,500,200,30 }, font));

	// -- Input -- 
	auto inputAction = InputAction(CharacterMoveLeft, InputState::down, 'A');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveRight, InputState::down, 'D');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveForward, InputState::down, 'W');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterMoveBackward, InputState::down, 'S');
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(CharacterJump, InputState::pressed, VK_SPACE, -1, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(Pause, InputState::pressed, VK_ESCAPE);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(MenuSelect, InputState::pressed, -1, VK_LBUTTON, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);

	// Shoot behaviour
	m_pCharacter->AddComponent(new ShootComponent(portal1, portal2, m_pCharacter->GetTransform()));
	GameObject* pickupSocket = m_pCharacter->AddChild(new GameObject);
	pickupSocket->AddComponent(new PickUpComponent());
	pickupSocket->GetTransform()->Translate(0.22f, 0.75f, 2.50f);

	// Post processing
	m_pVignette = MaterialManager::Get()->CreateMaterial<PostVignette>();
}

void PortalTestScene::OnSceneActivated() {
	ResetGame();
}

void PortalTestScene::Draw() {

	for (GameObject* portal : m_pPortals) {
		portal->GetComponent<PortalComponent>()->ClearRenderTargets();
	}

	// Disable items that don't need to be drawn on portals
	for (const auto pChild : m_pChildren)
	{
		std::vector<IgnorePortalDraw*> comps = pChild->GetComponents<IgnorePortalDraw>(true);
		for (auto& comp : comps) {
			comp->GetGameObject()->m_IsActive = false;
		}
	}

	int portalRecursions{ 5 };
	for(int i{0}; i < portalRecursions; ++i){

		for (GameObject* portal : m_pPortals) {

			if (!portal->GetComponent<PortalComponent>()->IsActive()) { continue; }
			portal->GetComponent<PortalComponent>()->BeginPortalRender(m_SceneContext, m_pGame);

			//Object-Scene Draw
			for (const auto pChild : m_pChildren)
			{
				pChild->RootDraw(m_SceneContext);
			}

			portal->GetComponent<PortalComponent>()->EndPortalRender(m_pGame);
		}
	}

	// Re-enable objects that don't need to be drawn on portals
	for (const auto pChild : m_pChildren)
	{
		std::vector<IgnorePortalDraw*> comps = pChild->GetComponents<IgnorePortalDraw>(true);
		for (auto& comp : comps) {
			comp->GetGameObject()->m_IsActive = true;
		}
	}
}

void PortalTestScene::PostDraw() {
	
	// Make sure the HUD is drawn last
	m_pHUD->RootDraw(m_SceneContext);

	// Draw pause screen on top of it
	if (m_IsPaused) {
		m_pPauseScreen->RootDraw(m_SceneContext);
	}
}

void PortalTestScene::Update() {

	if (m_SceneContext.pInput->IsActionTriggered(Pause)) {
		PauseGame(!m_IsPaused);
	}

	m_SceneContext.pInput->ForceMouseToCenter(!m_IsPaused);

	if (m_IsPaused) {
		if (m_SceneContext.pInput->IsActionTriggered(MenuSelect)) {

			if (m_pMenuButton->IsSelected()) {
				SceneManager::Get()->SetActiveGameScene(L"Menu");
			}

			if (m_pRetryButton->IsSelected()) {
				SceneManager::Get()->SetActiveGameScene(L"Controls");
			}

			if (m_pExitButton->IsSelected()) {
				PostQuitMessage(0);
			}
		}
	}
}

void PortalTestScene::PauseGame(bool paused) {
	m_IsPaused = paused;
	m_SceneContext.pInput->CursorVisible(paused);
	m_pCharacter->EnableMovement(!paused);
	m_pCharacter->GetComponent<ShootComponent>()->EnableShooting(!paused);
	m_pCharacter->GetComponent<PickUpComponent>(true)->EnablePickUp(!paused);
	m_pPauseScreen->SetActive(paused);

	if (paused) {
		RemovePostProcessingEffect(m_pVignette);
	}
	else {
		AddPostProcessingEffect(m_pVignette);
	}
}

void PortalTestScene::ResetGame() {
	
	// Make sure game is not paused
	PauseGame(false);

	// Reset positions
	m_pCharacter->GetTransform()->Translate(0, 5, 0);
	m_pCharacter->GetTransform()->Rotate(0, 0, 0);
	m_pCube->GetTransform()->Translate(5, 7, 15);
	m_pCube->GetTransform()->Rotate(0, 0, 0);

	// Reset portals
	m_pCharacter->GetComponent<ShootComponent>()->Reset();
	m_pCharacter->GetComponent<PickUpComponent>(true)->Reset();
}