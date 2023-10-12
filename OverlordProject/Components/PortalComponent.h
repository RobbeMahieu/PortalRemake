#pragma once
#include <unordered_map>
#include <unordered_set>

class PortalMaterial;

class PortalComponent final : public BaseComponent
{
	public:
		PortalComponent(GameObject* HUD, bool isBlue);
		virtual ~PortalComponent();
		PortalComponent(const PortalComponent& other) = delete;
		PortalComponent(PortalComponent&& other) noexcept = delete;
		PortalComponent& operator=(const PortalComponent& other) = delete;
		PortalComponent& operator=(PortalComponent&& other) noexcept = delete;

		virtual void Initialize(const SceneContext& sceneContext) override;
		virtual void Update(const SceneContext& /*sceneContext*/) override;

		void LinkPortal(TransformComponent* otherPortal, CameraComponent* playerCamera, CameraComponent* portalCamera);

		void BeginPortalRender(const SceneContext& sceneContext, OverlordGame* game);
		void EndPortalRender(OverlordGame* game);
		void OnTrigger(GameObject* pTriggerObject, GameObject* pOtherObject, PxTriggerAction action);
		void DrawImGui(int index);
		void ClearRenderTargets();
		void ActivatePortal(bool active);
		bool IsActive() const { return m_PortalActive; }
		bool IsBlue() const { return m_IsBlue; }

	private:

		void GetShadowLocation(const GameObject* object, XMVECTOR& position, XMVECTOR& Rotation) const;

		TransformComponent* m_pOtherPortal = nullptr;
		CameraComponent* m_pPortalCamera = nullptr;
		CameraComponent* m_pPlayerCamera = nullptr;

		RenderTarget* m_pRenderTarget = nullptr;
		RenderTarget* m_pShaderTarget = nullptr;

		PortalMaterial* m_pPortalMaterial = nullptr;

		RigidBodyComponent* m_pPortalTrigger = nullptr;

		CameraComponent* m_pSceneCamera = nullptr;

		GameObject* m_pPortalPlane = nullptr;
		GameObject* m_pHUD = nullptr;

		std::unordered_map<GameObject*, float> m_CollidingObjects;
		std::unordered_set<GameObject*> m_NotColliding{};
		bool m_PortalActive = false;
		const bool m_IsBlue;

		// SFX
		FMOD::Sound* m_pTeleportSFX;
};

