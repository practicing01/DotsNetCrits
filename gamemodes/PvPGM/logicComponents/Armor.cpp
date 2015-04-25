/*
 * Armor.cpp
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

#include "Armor.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

Armor::Armor(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "Armor";
	armor_ = 0;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	//SetUpdateEventMask(USE_UPDATE);
}

Armor::~Armor()
{
}

void Armor::Start()
{
}

void Armor::Update(float timeStep)
{
}

void Armor::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(armor_);
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Armor::ClientSync()
{
	armor_ = message_->ReadInt();
}

void Armor::ClientRequestExecute()
{
}

void Armor::ServerExecute()
{
	int amount = message_->ReadInt();
	char operation = message_->ReadByte();

	if (operation == -1)//subtract
	{
		armor_ -= amount;
	}
	else if (operation == 0)//set
	{
		armor_ = amount;
	}
	else if (operation == 1)//add
	{
		armor_ += amount;
	}

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(amount);
	msg_.WriteByte(operation);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Armor::ClientExecute()
{
	int amount = message_->ReadInt();
	char operation = message_->ReadByte();

	if (operation == -1)//subtract
	{
		armor_ -= amount;
	}
	else if (operation == 0)//set
	{
		armor_ = amount;
	}
	else if (operation == 1)//add
	{
		armor_ += amount;
	}
}

void Armor::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
