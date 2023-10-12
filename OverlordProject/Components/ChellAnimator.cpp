#include "stdafx.h"
#include "ChellAnimator.h"

ChellAnimator::ChellAnimator() : BaseComponent()
{
	SoundManager::Get()->GetSystem()->createSound("Resources/Sounds/footsteps.mp3", FMOD_LOOP_NORMAL, 0, &m_pFootstepsSFX);
	SoundManager::Get()->GetSystem()->playSound(m_pFootstepsSFX, 0, true, &m_pFootstepsChannel);
	m_pFootstepsChannel->setPaused(true);
}

void ChellAnimator::Initialize(const SceneContext& /*sceneContext*/ ) {
	m_pAnimator = m_pGameObject->GetComponent<ModelComponent>(true)->GetAnimator();
	SetState(State::Idle);
	m_PreviousPosition = GetTransform()->GetWorldPosition();
}

void ChellAnimator::Update(const SceneContext& /*sceneContext*/) {
	
	XMFLOAT3 currentPos{ GetTransform()->GetWorldPosition() };

	bool didNotMove{ currentPos.x == m_PreviousPosition.x && currentPos.y == m_PreviousPosition.y };

	SetState(didNotMove ? State::Idle : State::Moving);

	m_PreviousPosition = currentPos;
}

void ChellAnimator::SetState(State newState) {
	if (m_CurrentState == newState) {
		return;
	}

	m_CurrentState = newState;
	m_pAnimator->SetAnimation(m_CurrentState);

	if (!m_pAnimator->IsPlaying()) {
		m_pAnimator->Play();
	}

	// Footsteps SFX
	if (m_CurrentState == State::Moving) {
		m_pFootstepsChannel->setPaused(false);
	}
	else {		
		m_pFootstepsChannel->setPaused(true);
	}
}