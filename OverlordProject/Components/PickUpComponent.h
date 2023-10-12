#pragma once

class PickUpComponent : public BaseComponent
{
	public:
		PickUpComponent();
		virtual ~PickUpComponent() = default;
		PickUpComponent(const PickUpComponent& other) = delete;
		PickUpComponent(PickUpComponent&& other) noexcept = delete;
		PickUpComponent& operator=(const PickUpComponent& other) = delete;
		PickUpComponent& operator=(PickUpComponent&& other) noexcept = delete;

		virtual void Initialize(const SceneContext& /*sceneContext*/) override;
		virtual void Update(const SceneContext& /*sceneContext*/) override;
		void EnablePickUp(bool canPickup);
		void Reset();

	private:
		enum Actions {
			PickUpObject = 200,
		};

		GameObject* m_pPickedUpObject = nullptr;
		bool m_CanPickup = true;
		float m_Range{ 10.0f };

		void Pickup();
		void Drop();
	};

