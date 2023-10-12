#include "stdafx.h"
#include "BoneObject.h"

BoneObject::BoneObject(BaseMaterial* pMaterial, float length)
	: m_pMaterial{pMaterial}
	, m_Length{length}
{
}

void BoneObject::Initialize(const SceneContext&) {
	GameObject* pEmpty = new GameObject();
	AddChild(pEmpty);
	ModelComponent* pModel = pEmpty->AddComponent(new ModelComponent(L"Meshes/bone.ovm"));
	pModel->SetMaterial(m_pMaterial);
	pEmpty->GetTransform()->Rotate(0, -90, 0);
	pEmpty->GetTransform()->Scale(m_Length);
}

void BoneObject::AddBone(BoneObject* pBone) {

	// Translate it to 0 on the x-axis
	XMFLOAT3 bonePos = pBone->GetTransform()->GetPosition();
	XMFLOAT3 parentPos = GetTransform()->GetPosition();
	pBone->GetTransform()->Translate((m_Length + parentPos.x) - bonePos.x, 0, 0);

	AddChild(pBone);
}

void BoneObject::CalculateBindPose() {

	XMMATRIX worldMatrix = XMLoadFloat4x4(&(GetTransform()->GetWorld()));
	XMMATRIX invMatrix = XMMatrixInverse(nullptr, worldMatrix);
	XMStoreFloat4x4(&m_BindPose, invMatrix);

	std::vector<BoneObject*> childBones = GetChildren<BoneObject>();
	for (BoneObject* childBone : childBones) {
		childBone->CalculateBindPose();
	}
}