#pragma once
class ControlsScene : public GameScene
{
public:
	ControlsScene() : GameScene(L"Controls") {}
	virtual ~ControlsScene() = default;
	ControlsScene(const ControlsScene& other) = delete;
	ControlsScene(ControlsScene&& other) noexcept = delete;
	ControlsScene& operator=(const ControlsScene& other) = delete;
	ControlsScene& operator=(ControlsScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void OnSceneActivated() override;
	void Update() override;

	float m_WaitTime{ 3.0f };
	float m_Timer{ 0.0f };
};

