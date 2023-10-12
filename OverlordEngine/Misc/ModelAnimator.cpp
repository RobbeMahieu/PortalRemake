#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
	m_pMeshFilter{pMeshFilter}
{
	SetAnimation(0);
}

void ModelAnimator::Update(const SceneContext& sceneContext)
{
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		auto passedTicks = sceneContext.pGameTime->GetElapsed() * m_CurrentClip.ticksPerSecond * m_AnimationSpeed;
		passedTicks = fmod(passedTicks, m_CurrentClip.duration);

		if (m_Reversed) {
			m_TickCount -= passedTicks;
			if (m_TickCount < 0) {
				m_TickCount += m_CurrentClip.duration;
			}
		}
		else {
			m_TickCount += passedTicks;
			if (m_TickCount > m_CurrentClip.duration) {
				m_TickCount -= m_CurrentClip.duration;
			}
		}

		AnimationKey keyA, keyB;
		keyA = *std::find_if(m_CurrentClip.keys.rbegin(), m_CurrentClip.keys.rend(), [=](auto key) {
			return key.tick <= m_TickCount;
		});

		keyB = *std::find_if(m_CurrentClip.keys.begin(), m_CurrentClip.keys.end(), [=](auto key) {
			return key.tick > m_TickCount;
		});

		float blendFactor{ 1.0f - ((keyB.tick - m_TickCount) / (keyB.tick - keyA.tick)) };
		m_Transforms.clear();

		for (int i{ 0 }; i < m_pMeshFilter->m_BoneCount; ++i) {

			// Get bone transforms
			auto transformA{ XMLoadFloat4x4(&keyA.boneTransforms[i]) };
			auto transformB{ XMLoadFloat4x4(&keyB.boneTransforms[i]) };

			XMVECTOR translateA, translateB;
			XMVECTOR rotateA, rotateB;
			XMVECTOR scaleA, scaleB;
			XMMatrixDecompose(&scaleA, &rotateA, &translateA, transformA);
			XMMatrixDecompose(&scaleB, &rotateB, &translateB, transformB);

			// Interpolate between bones
			XMMATRIX blendedTranslate = XMMatrixTranslationFromVector(XMVectorLerp(translateA, translateB, blendFactor));
			XMMATRIX blendedRotate = XMMatrixRotationQuaternion(XMQuaternionSlerp(rotateA, rotateB, blendFactor));
			XMMATRIX blendedScale = XMMatrixScalingFromVector(XMVectorLerp(scaleA, scaleB, blendFactor));

			// Recombine transform matrix and add it to the vector
			XMFLOAT4X4 transform{};
			XMMATRIX blendedTransform = XMMatrixMultiply(XMMatrixMultiply(blendedScale, blendedRotate), blendedTranslate);
			XMStoreFloat4x4(&transform, blendedTransform);

			m_Transforms.push_back(transform);
		}
	}
}

void ModelAnimator::SetAnimation(const std::wstring& clipName)
{
	m_ClipSet = false;

	auto founClip = std::find_if(m_pMeshFilter->m_AnimationClips.begin(), m_pMeshFilter->m_AnimationClips.end(), [&](auto clip) {
		return clip.name == clipName;
	});
	
	if (founClip != m_pMeshFilter->m_AnimationClips.end()) {
		SetAnimation(*founClip);
	}
	else {
		Reset();
		Logger::LogWarning(L"No animation clip found with name: " + clipName);
	}

}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	m_ClipSet = false;

	if (clipNumber >= m_pMeshFilter->m_AnimationClips.size()) {
		Reset();
		Logger::LogWarning(L"No animation clip found with index: " + clipNumber);
		return;
	}
	else {
		SetAnimation(m_pMeshFilter->m_AnimationClips[clipNumber]);
	}
}

void ModelAnimator::SetAnimation(const AnimationClip& clip)
{
	m_ClipSet = true;
	m_CurrentClip = clip;
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	m_IsPlaying = !pause;
	m_TickCount = 0;
	m_AnimationSpeed = 1.0f;

	if (m_ClipSet) {
		std::vector<XMFLOAT4X4> boneTransforms{ m_CurrentClip.keys[0].boneTransforms };
		m_Transforms.assign(boneTransforms.begin(), boneTransforms.end());
	}
	else {
		XMFLOAT4X4 identityMatrix{};
		XMStoreFloat4x4(&identityMatrix,XMMatrixIdentity());

		m_Transforms.assign(m_pMeshFilter->m_BoneCount, identityMatrix);
	}
}
