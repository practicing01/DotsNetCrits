/*
 * Gravity.cpp
 *
 *  Created on: Apr 11, 2015
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

#include "Gravity.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

Gravity::Gravity(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "Gravity";
	gravity_ = -9.0f;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	//SetUpdateEventMask(USE_UPDATE);
}

Gravity::~Gravity()
{
}

void Gravity::Start()
{
}

void Gravity::Update(float timeStep)
{
}

void Gravity::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteFloat(gravity_);
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Gravity::ClientSync()
{
	gravity_ = message_->ReadFloat();
}

void Gravity::ClientRequestExecute()
{
}

void Gravity::ServerExecute()
{
	float amount = message_->ReadFloat();
	char operation = message_->ReadByte();

	if (operation == -1)//subtract
	{
		gravity_ -= amount;
	}
	else if (operation == 0)//set
	{
		gravity_ = amount;
	}
	else if (operation == 1)//add
	{
		gravity_ += amount;
	}

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteFloat(amount);
	msg_.WriteByte(operation);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Gravity::ClientExecute()
{
	float amount = message_->ReadFloat();
	char operation = message_->ReadByte();

	if (operation == -1)//subtract
	{
		gravity_ -= amount;
	}
	else if (operation == 0)//set
	{
		gravity_ = amount;
	}
	else if (operation == 1)//add
	{
		gravity_ += amount;
	}
}

void Gravity::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
