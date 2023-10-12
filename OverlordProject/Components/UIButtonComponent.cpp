#include "stdafx.h"
#include "UIButtonComponent.h"

UIButtonComponent::UIButtonComponent(const std::wstring& text, const XMFLOAT4& rect, SpriteFont* font)
	: BaseComponent()
	, m_Text{ text }
	, m_Rect{ rect }
	, m_pFont{ font }
	, m_Color { Colors::White }
	, m_SelectedColor { Colors::Orange }
	, m_Selected{ false }
{
}

void UIButtonComponent::Update(const SceneContext& /*sceneContext*/) {

	// Hover detection
	POINT mousePos{ GetScene()->GetSceneContext().pInput->GetMousePosition() };
	m_Selected = mousePos.x > m_Rect.x && mousePos.x <= m_Rect.x + m_Rect.z
			  && mousePos.y > m_Rect.y && mousePos.y <= m_Rect.y + m_Rect.w;
}

void UIButtonComponent::Draw(const SceneContext& /*sceneContext*/) {
	XMFLOAT4 color = (m_Selected) ? m_SelectedColor : m_Color;
	TextRenderer::Get()->DrawText(m_pFont, m_Text, XMFLOAT2{ m_Rect.x, m_Rect.y }, color);
}

bool UIButtonComponent::IsSelected() const {

	return m_Selected;
}