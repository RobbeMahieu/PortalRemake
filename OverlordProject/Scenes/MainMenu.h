#pragma once

class UIButtonComponent;

class MainMenu : public GameScene
{
public:
	MainMenu() : GameScene(L"Menu") {}
	virtual ~MainMenu() = default;
	MainMenu(const MainMenu& other) = delete;
	MainMenu(MainMenu&& other) noexcept = delete;
	MainMenu& operator=(const MainMenu& other) = delete;
	MainMenu& operator=(MainMenu&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;

	enum MenuActions {
		Select,
	};

	GameObject* m_pCube = nullptr;
	UIButtonComponent* m_pPlayButton = nullptr;
	UIButtonComponent* m_pExitButton = nullptr;
	float m_RotateSpeed = 20.0f;
};

