#pragma once

class UIButtonComponent : public BaseComponent
{
public:
	UIButtonComponent(const std::wstring& text, const XMFLOAT4& rect, SpriteFont* font);
	virtual ~UIButtonComponent() = default;
	UIButtonComponent(const UIButtonComponent& other) = delete;
	UIButtonComponent(UIButtonComponent&& other) noexcept = delete;
	UIButtonComponent& operator=(const UIButtonComponent& other) = delete;
	UIButtonComponent& operator=(UIButtonComponent&& other) noexcept = delete;

	virtual void Initialize(const SceneContext& /*sceneContext*/) override {}
	virtual void Update(const SceneContext& /*sceneContext*/) override;
	virtual void Draw(const SceneContext& /*sceneContext*/) override;

	bool IsSelected() const;

private:
	std::wstring m_Text;
	XMFLOAT4 m_Rect;
	SpriteFont* m_pFont;
	XMFLOAT4 m_Color;
	XMFLOAT4 m_SelectedColor;
	bool m_Selected;
};

