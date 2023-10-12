#include "stdafx.h"
#include "Character.h"

Character::Character(const CharacterDesc& characterDesc) :
	m_CharacterDesc{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime),
	m_CanMove{ true }
{}

void Character::Initialize(const SceneContext& /*sceneContext*/)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CharacterDesc.controller));

	//Camera
	const auto pCamera = AddChild(new FixedCamera());
	m_pCameraComponent = pCamera->GetComponent<CameraComponent>();
	m_pCameraComponent->SetActive(true);

	pCamera->GetTransform()->Translate(0.f, m_CharacterDesc.controller.height * .5f, 0.f);
}

void Character::Update(const SceneContext& sceneContext)
{
	if (m_pCameraComponent->IsActive() && m_CanMove)
	{
		//constexpr float epsilon{ 0.01f }; //Constant that can be used to compare if a float is near zero
		float elapsedSec{ sceneContext.pGameTime->GetElapsed() };

		//***************
		//HANDLE INPUT

		//## Input Gathering (move)
		XMFLOAT2 move{ 0,0 };
		//move.y should contain a 1 (Forward) or -1 (Backward) based on the active input (check corresponding actionId in m_CharacterDesc)
		move.x = sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveBackward) ? -1.0f : 0.0f;
		move.x += sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveForward) ? 1.0f : 0.0f;
		//Optional: if move.y is near zero (abs(move.y) < epsilon), you could use the ThumbStickPosition.y for movement

		//move.x should contain a 1 (Right) or -1 (Left) based on the active input (check corresponding actionId in m_CharacterDesc)
		move.y = sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveLeft) ? -1.0f : 0.0f;
		move.y += sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_MoveRight) ? 1.0f : 0.0f;
		//Optional: if move.x is near zero (abs(move.x) < epsilon), you could use the Left ThumbStickPosition.x for movement

		//## Input Gathering (look)
		XMFLOAT2 look{ 0.f, 0.f };
		//Only if the Left Mouse Button is Down >
		look = XMFLOAT2(float(sceneContext.pInput->GetMouseMovement().x), float(sceneContext.pInput->GetMouseMovement().y));

		//Optional: in case look.x AND look.y are near zero, you could use the Right ThumbStickPosition for look

		//************************
		//GATHERING TRANSFORM INFO

		//Retrieve the TransformComponent
		TransformComponent* transform{ GetTransform() };
		//Retrieve the forward & right vector (as XMVECTOR) from the TransformComponent
		XMVECTOR forward{ XMLoadFloat3(&transform->GetForward()) };
		XMVECTOR right{ XMLoadFloat3(&transform->GetRight()) };

		//***************
		//CAMERA ROTATION

		//Adjust the TotalYaw (m_TotalYaw) & TotalPitch (m_TotalPitch) based on the local 'look' variable
		XMFLOAT3 worldUp{ 0,1,0 };

		float yawDegrees{ look.x * m_CharacterDesc.rotationSpeed * elapsedSec };
		float pitchDegrees{ look.y * m_CharacterDesc.rotationSpeed * elapsedSec };
		XMVECTOR pitch = XMQuaternionRotationNormal(right, XMConvertToRadians(pitchDegrees));
		XMVECTOR yaw = XMQuaternionRotationNormal(XMLoadFloat3(&worldUp), XMConvertToRadians(yawDegrees));
		XMVECTOR rotation = XMQuaternionMultiply(XMLoadFloat4(&transform->GetRotation()), XMQuaternionMultiply(pitch, yaw));

		//Rotate this character based on the TotalPitch (X) and TotalYaw (Y)
		transform->Rotate(rotation);

		// Make sure there's no roll



		//********
		//MOVEMENT

		//## Horizontal Velocity (Forward/Backward/Right/Left)
		float currentAcceleration{ m_MoveAcceleration * elapsedSec };
		//If the character is moving (= input is pressed)
		if (move.x != 0 || move.y != 0) {
			//Calculate & Store the current direction (m_CurrentDirection) >> based on the forward/right vectors and the pressed input
			XMStoreFloat3(&m_CurrentDirection, move.x * forward + move.y * right);
			//Increase the current MoveSpeed with the current Acceleration (m_MoveSpeed)
			m_MoveSpeed += currentAcceleration;
			//Make sure the current MoveSpeed stays below the maximum MoveSpeed (CharacterDesc::maxMoveSpeed)
			m_MoveSpeed = (m_MoveSpeed > m_CharacterDesc.maxMoveSpeed) ? m_CharacterDesc.maxMoveSpeed : m_MoveSpeed;
		}
		//Else (character is not moving, or stopped moving)
		else {
			//Decrease the current MoveSpeed with the current Acceleration (m_MoveSpeed)
			m_MoveSpeed -= currentAcceleration;
			//Make sure the current MoveSpeed doesn't get smaller than zero
			m_MoveSpeed = (m_MoveSpeed < 0) ? 0 : m_MoveSpeed;
		}

		//Now we can calculate the Horizontal Velocity which should be stored in m_TotalVelocity.xz
		m_TotalVelocity.x = m_CurrentDirection.x * m_MoveSpeed;
		m_TotalVelocity.z = m_CurrentDirection.z * m_MoveSpeed;

		//## Vertical Movement (Jump/Fall)
		//If the Controller Component is NOT grounded (= freefall)
		if (!(m_pControllerComponent->GetCollisionFlags() & PxControllerCollisionFlag::eCOLLISION_DOWN)) {
			//Decrease the y component of m_TotalVelocity with a fraction (ElapsedTime) of the Fall Acceleration (m_FallAcceleration)
			m_TotalVelocity.y -= m_FallAcceleration * sceneContext.pGameTime->GetElapsed();
			//Make sure that the minimum speed stays above -CharacterDesc::maxFallSpeed (negative!)
			m_TotalVelocity.y = (m_TotalVelocity.y < -m_CharacterDesc.maxFallSpeed) ? -m_CharacterDesc.maxFallSpeed : m_TotalVelocity.y;
		}
		//Else If the jump action is triggered
		else if (sceneContext.pInput->IsActionTriggered(m_CharacterDesc.actionId_Jump)) {
			//Set m_TotalVelocity.y equal to CharacterDesc::JumpSpeed
			m_TotalVelocity.y = m_CharacterDesc.JumpSpeed;
		}
		//Else (=Character is grounded, no input pressed)
		else {
			//m_TotalVelocity.y is zero
			m_TotalVelocity.y = -0.1f;
		}

		//************
		//DISPLACEMENT

		//The displacement required to move the Character Controller (ControllerComponent::Move) can be calculated using our TotalVelocity (m/s)
		//Calculate the displacement (m) for the current frame and move the ControllerComponent
		XMFLOAT3 displacement{ m_TotalVelocity.x * elapsedSec, m_TotalVelocity.y * elapsedSec , m_TotalVelocity.z * elapsedSec };
		m_pControllerComponent->Move(displacement);

	}
}

void Character::DrawImGui()
{
	if (ImGui::CollapsingHeader("Character"))
	{
		ImGui::Text(std::format("Move Speed: {:0.1f} m/s", m_MoveSpeed).c_str());
		ImGui::Text(std::format("Fall Speed: {:0.1f} m/s", m_TotalVelocity.y).c_str());

		ImGui::Text(std::format("Move Acceleration: {:0.1f} m/s2", m_MoveAcceleration).c_str());
		ImGui::Text(std::format("Fall Acceleration: {:0.1f} m/s2", m_FallAcceleration).c_str());

		const float jumpMaxTime = m_CharacterDesc.JumpSpeed / m_FallAcceleration;
		const float jumpMaxHeight = (m_CharacterDesc.JumpSpeed * jumpMaxTime) - (0.5f * (m_FallAcceleration * powf(jumpMaxTime, 2)));
		ImGui::Text(std::format("Jump Height: {:0.1f} m", jumpMaxHeight).c_str());

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Move Speed (m/s)", &m_CharacterDesc.maxMoveSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Move Acceleration Time (s)", &m_CharacterDesc.moveAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_MoveAcceleration = m_CharacterDesc.maxMoveSpeed / m_CharacterDesc.moveAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Fall Speed (m/s)", &m_CharacterDesc.maxFallSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Fall Acceleration Time (s)", &m_CharacterDesc.fallAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_FallAcceleration = m_CharacterDesc.maxFallSpeed / m_CharacterDesc.fallAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		ImGui::DragFloat("Jump Speed", &m_CharacterDesc.JumpSpeed, 0.1f, 0.f, 0.f, "%.1f");
		ImGui::DragFloat("Rotation Speed (deg/s)", &m_CharacterDesc.rotationSpeed, 0.1f, 0.f, 0.f, "%.1f");

		bool isActive = m_pCameraComponent->IsActive();
		if(ImGui::Checkbox("Character Camera", &isActive))
		{
			m_pCameraComponent->SetActive(isActive);
		}
	}
}

CameraComponent* Character::GetCamera() const {
	return m_pCameraComponent;
}

void Character::EnableMovement(bool canMove) {
	m_CanMove = canMove;
}