#pragma once

class ShootComponent : public BaseComponent
{
public:
	ShootComponent(GameObject* pLeftPortal, GameObject* pRightPortal, TransformComponent* playerTransfrom);
	virtual ~ShootComponent() = default;
	ShootComponent(const ShootComponent& other) = delete;
	ShootComponent(ShootComponent&& other) noexcept = delete;
	ShootComponent& operator=(const ShootComponent& other) = delete;
	ShootComponent& operator=(ShootComponent&& other) noexcept = delete;

	virtual void Initialize(const SceneContext& sceneContext) override;
	virtual void Update(const SceneContext& /*sceneContext*/) override;
	void EnableShooting(bool canShoot);
	void Reset();

private:
	enum Actions {
		ShootLeft = 100,
		ShootRight = 101
	};

	struct Shot {
		GameObject* particles;
		XMFLOAT3 direction;
		GameObject* portal;
		bool destroy = false;
	};

	GameObject* m_pLeftPortal;
	GameObject* m_pRightPortal;
	TransformComponent* m_pPlayerTransform;
	ParticleEmitterSettings m_ParticleSettings;
	std::vector<Shot> m_Shots;
	bool m_CanShoot;

	void ShootPortal(GameObject* portal);
	void CheckHit(Shot& shot);

	// SFX
	FMOD::Sound* m_pShootSFX;

};

