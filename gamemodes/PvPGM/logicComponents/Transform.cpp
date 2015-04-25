/*
 * Transform.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>

#include "Transform.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

Transform::Transform(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "Transform";
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	//SetUpdateEventMask(USE_UPDATE);
}

Transform::~Transform()
{
}

void Transform::Start()
{
}

void Transform::Update(float timeStep)
{
}

void Transform::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteVector3(node_->GetPosition());
	msg_.WriteQuaternion(node_->GetRotation());
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Transform::ClientSync()
{
	node_->SetPosition(message_->ReadVector3());
	node_->SetRotation(message_->ReadQuaternion());
}

void Transform::ClientRequestExecute()
{
}

void Transform::ServerExecute()
{
	Vector3 pos = message_->ReadVector3();
	Quaternion rot = message_->ReadQuaternion();

	node_->SetPosition(pos);
	node_->SetRotation(rot);

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteVector3(pos);
	msg_.WriteQuaternion(rot);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Transform::ClientExecute()
{
	node_->SetPosition(message_->ReadVector3());
	node_->SetRotation(message_->ReadQuaternion());
}

void Transform::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
