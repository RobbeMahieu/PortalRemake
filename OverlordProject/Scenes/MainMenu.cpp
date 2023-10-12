#include "stdafx.h"
#include "MainMenu.h"
#include "Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "Components/UIButtonComponent.h"

void MainMenu::Initialize() {

	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.drawUserDebug = false;
	m_SceneContext.settings.showInfoOverlay = false;

	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	// -- Menu -- 
	GameObject* Menu = AddChild(new GameObject);

	// -- Background
	GameObject* background = Menu->AddChild(new GameObject);
	auto image = background->AddComponent(new SpriteComponent(L"Textures/Menu/background.png"));
	image->SetPivot(XMFLOAT2{ 0.5f,0.5f });
	background->GetTransform()->Translate(m_SceneContext.windowWidth / 2, m_SceneContext.windowHeight / 2, 0);

	// -- Spinning Cube --
	const auto pMetalCubeMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	pMetalCubeMaterial->SetDiffuseTexture(L"Textures/metal_box.png");

	m_pCube =  new GameObject();
	const auto pCubeModel = new ModelComponent(L"Meshes/MetalCube.ovm");
	pCubeModel->SetMaterial(pMetalCubeMaterial);

	m_pCube->AddComponent(pCubeModel);
	AddChild(m_pCube);
	m_pCube->GetTransform()->Translate(20, 0, 60);
	m_pCube->GetTransform()->Rotate(45, 0, -45);
	m_pCube->GetTransform()->Scale(0.5f, 0.5f, 0.5f);

	SpriteFont* font = ContentManager::Load<SpriteFont>(L"SpriteFonts/Consolas_32.fnt");

	// -- Play button --
	GameObject* playButton = Menu->AddChild(new GameObject);
	m_pPlayButton = playButton->AddComponent(new UIButtonComponent(L"PLAY", XMFLOAT4{ 100,400,200,30 }, font));

	// -- Exit button --
	GameObject* exitButton = Menu->AddChild(new GameObject);
	m_pExitButton = exitButton->AddComponent(new UIButtonComponent(L"EXIT", XMFLOAT4{ 100,450,200,30 }, font));

	GameObject* camera = new GameObject();
	AddChild(camera);
	SetActiveCamera(camera->AddComponent(new CameraComponent));

	// -- Inputs --
	auto inputAction = InputAction(Select, InputState::pressed, -1, VK_LBUTTON, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);
}

void MainMenu::Update() {
	m_pCube->GetTransform()->Rotate(45, m_RotateSpeed*m_SceneContext.pGameTime->GetTotal(), -45);

	if (m_SceneContext.pInput->IsActionTriggered(Select)) {

		if (m_pPlayButton->IsSelected()) {
			SceneManager::Get()->SetActiveGameScene(L"Controls");
		}

		if (m_pExitButton->IsSelected()) {
			PostQuitMessage(0);
		}
		
	}
}