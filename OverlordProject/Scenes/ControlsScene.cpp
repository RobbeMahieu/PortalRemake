#include "stdafx.h"
#include "ControlsScene.h"

void ControlsScene::Initialize() {

	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.drawUserDebug = false;
	m_SceneContext.settings.showInfoOverlay = false;

	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	// -- Menu -- 
	GameObject* Menu = AddChild(new GameObject);
	GameObject* background = Menu->AddChild(new GameObject);
	auto image = background->AddComponent(new SpriteComponent(L"Textures/Menu/controls.png"));
	image->SetPivot(XMFLOAT2{ 0.5f,0.5f });
	background->GetTransform()->Translate(m_SceneContext.windowWidth / 2, m_SceneContext.windowHeight / 2, 0);

	GameObject* camera = new GameObject();
	AddChild(camera);
	SetActiveCamera(camera->AddComponent(new CameraComponent));
}

void ControlsScene::OnSceneActivated() {
	m_Timer = 0.0f;
}

void ControlsScene::Update() {
	
	m_Timer += m_SceneContext.pGameTime->GetElapsed();
	if (m_Timer >= m_WaitTime) {
		SceneManager::Get()->SetActiveGameScene(L"Game");
	}		
}