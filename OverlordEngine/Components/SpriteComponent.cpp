#include "stdafx.h"
#include "SpriteComponent.h"

SpriteComponent::SpriteComponent(const std::wstring& spriteAsset, const XMFLOAT2& pivot, const XMFLOAT4& color):
	m_SpriteAsset(spriteAsset),
	m_Pivot(pivot),
	m_Color(color)
{}

void SpriteComponent::Initialize(const SceneContext& /*sceneContext*/)
{
	m_pTexture = ContentManager::Load<TextureData>(m_SpriteAsset);
}

void SpriteComponent::SetTexture(const std::wstring& spriteAsset)
{
	m_SpriteAsset = spriteAsset;
	m_pTexture = ContentManager::Load<TextureData>(m_SpriteAsset);
}

void SpriteComponent::Draw(const SceneContext& sceneContext)
{
	if (!m_pTexture)
		return;

	TransformComponent* transform{ m_pGameObject->GetTransform() };
	XMFLOAT2 pos{ transform->GetWorldPosition().x, transform->GetWorldPosition().y };
	XMFLOAT2 scale{ transform->GetScale().x, transform->GetScale().y };
	float rot{ MathHelper::QuaternionToEuler(transform->GetRotation()).z};

	SpriteRenderer::Get()->AppendSprite(m_pTexture, pos, m_Color, m_Pivot, scale, rot, transform->GetPosition().z);
	SpriteRenderer::Get()->Draw(sceneContext);
}
