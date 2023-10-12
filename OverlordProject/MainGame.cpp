#include "stdafx.h"
#include "MainGame.h"

#include "Scenes/PortalTestScene.h"
#include "Scenes/MainMenu.h"
#include "Scenes/ControlsScene.h"

//Game is preparing
void MainGame::OnGamePreparing(GameContext& gameContext)
{
	gameContext.windowTitle = L"GP2 - Exam Project (2023) | (2DAE07) Mahieu Robbe";
}

void MainGame::Initialize()
{
	SceneManager::Get()->AddGameScene(new MainMenu());
	SceneManager::Get()->AddGameScene(new PortalTestScene());
	SceneManager::Get()->AddGameScene(new ControlsScene());
}

LRESULT MainGame::WindowProcedureHook(HWND /*hWnd*/, UINT message, WPARAM wParam, LPARAM lParam)
{

	if(message == WM_KEYUP)
	{
		if ((lParam & 0x80000000) != 0x80000000)
			return -1;

		//[F1] Toggle Scene Info Overlay
		if(wParam == VK_F1)
		{
			const auto pScene = SceneManager::Get()->GetActiveScene();
			pScene->GetSceneSettings().Toggle_ShowInfoOverlay();
		}

		//[F2] Toggle Debug Renderer (Global)
		if (wParam == VK_F2)
		{
			DebugRenderer::ToggleDebugRenderer();
			return 0;

		}

		//[F5] If PhysX Framestepping is enables > Next Frame	
		if (wParam == VK_F6)
		{
			const auto pScene = SceneManager::Get()->GetActiveScene();
			pScene->GetPhysxProxy()->NextPhysXFrame();
		}
	}
	

	return -1;
}
