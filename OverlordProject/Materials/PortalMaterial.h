#pragma once

class PortalMaterial : public Material<PortalMaterial>
{
	public:
		PortalMaterial();
		~PortalMaterial() override = default;

		PortalMaterial(const PortalMaterial& other) = delete;
		PortalMaterial(PortalMaterial&& other) noexcept = delete;
		PortalMaterial& operator=(const PortalMaterial& other) = delete;
		PortalMaterial& operator=(PortalMaterial&& other) noexcept = delete;

		void UpdatePortalSurface(ID3D11ShaderResourceView* pSRV);
		void SetPlayerCameraVariable(XMMATRIX CameraWVP);

	protected:
		void InitializeEffectVariables() override;
};

