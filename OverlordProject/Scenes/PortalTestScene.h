#pragma once
class PortalMaterial;
class Character;
class PostVignette;
class UIButtonComponent;

class PortalTestScene : public GameScene
{
	public:
		PortalTestScene() : GameScene(L"Game") {}
		virtual ~PortalTestScene() = default;
		PortalTestScene(const PortalTestScene& other) = delete;
		PortalTestScene(PortalTestScene&& other) noexcept = delete;
		PortalTestScene& operator=(const PortalTestScene& other) = delete;
		PortalTestScene& operator=(PortalTestScene&& other) noexcept = delete;

	protected:
		void Initialize() override;
		void Draw() override;
		void PostDraw() override;
		void Update() override;
		void OnSceneActivated() override;

		void PauseGame(bool paused);

	private:
		std::vector<GameObject*> m_pPortals;

		enum InputIds
		{
			CharacterMoveLeft,
			CharacterMoveRight,
			CharacterMoveForward,
			CharacterMoveBackward,
			CharacterJump,

			PortalGunLeft,
			PortalGunRight,
			Pause,
			MenuSelect
		};

		enum CollisionLayer {
			LevelStatic = CollisionGroup::Group0,
			LevelDynamic = CollisionGroup::Group1,
			LevelNonPortal = CollisionGroup::Group4,
			Player = CollisionGroup::Group2,
			Portal = CollisionGroup::Group3,
		};

		Character* m_pCharacter{};
		GameObject* m_pCube;
		GameObject* m_pHUD;

		bool m_IsPaused = false;
		GameObject* m_pPauseScreen;
		UIButtonComponent* m_pMenuButton;
		UIButtonComponent* m_pRetryButton;
		UIButtonComponent* m_pExitButton;

		PostVignette* m_pVignette;

		void ResetGame();
};

