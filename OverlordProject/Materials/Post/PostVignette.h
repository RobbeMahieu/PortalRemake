#pragma once

class PostVignette : public PostProcessingMaterial
{
public:
	PostVignette();
	~PostVignette() override = default;
	PostVignette(const PostVignette& other) = delete;
	PostVignette(PostVignette&& other) noexcept = delete;
	PostVignette& operator=(const PostVignette& other) = delete;
	PostVignette& operator=(PostVignette&& other) noexcept = delete;

protected:
	void Initialize(const GameContext& /*gameContext*/) override {}

};


