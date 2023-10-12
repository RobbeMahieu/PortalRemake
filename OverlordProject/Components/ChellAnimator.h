#pragma once

class ChellAnimator : public BaseComponent
{
	public:
		ChellAnimator();
		virtual ~ChellAnimator() = default;
		ChellAnimator(const ChellAnimator& other) = delete;
		ChellAnimator(ChellAnimator&& other) noexcept = delete;
		ChellAnimator& operator=(const ChellAnimator& other) = delete;
		ChellAnimator& operator=(ChellAnimator&& other) noexcept = delete;

		virtual void Initialize(const SceneContext& sceneContext) override;
		virtual void Update(const SceneContext& /*sceneContext*/) override;

	private:

		enum State {
			 Moving = 0,
			 Idle = 1,
		};

		ModelAnimator* m_pAnimator = nullptr;
		State m_CurrentState = State::Moving;
		XMFLOAT3 m_PreviousPosition{};

		void SetState(State newState);

		// Sounds
		FMOD::Sound* m_pFootstepsSFX;
		FMOD::Channel* m_pFootstepsChannel;
};

