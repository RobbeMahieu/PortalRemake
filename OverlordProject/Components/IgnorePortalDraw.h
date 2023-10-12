#pragma once

class IgnorePortalDraw : public BaseComponent
{
public:
	IgnorePortalDraw() = default;
	virtual ~IgnorePortalDraw() = default;
	IgnorePortalDraw(const IgnorePortalDraw& other) = delete;
	IgnorePortalDraw(IgnorePortalDraw&& other) noexcept = delete;
	IgnorePortalDraw& operator=(const IgnorePortalDraw& other) = delete;
	IgnorePortalDraw& operator=(IgnorePortalDraw&& other) noexcept = delete;

	virtual void Initialize(const SceneContext&) override {};
};