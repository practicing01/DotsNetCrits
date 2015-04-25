/*
 * Health.cpp
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

#include "Health.h"
#include "Shield.h"
#include "SoundPlayer.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

Health::Health(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "Health";
	health_ = 100;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	//SetUpdateEventMask(USE_UPDATE);
}

Health::~Health()
{
}

void Health::Start()
{
}

void Health::Update(float timeStep)
{
}

void Health::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(health_);
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Health::ClientSync()
{
	health_ = message_->ReadInt();
}

void Health::ClientRequestExecute()
{
}

void Health::ServerExecute()
{
	int amount = message_->ReadInt();
	char operation = message_->ReadByte();

	if (node_->GetComponent<Shield>()->shielded_ && operation == -1)
	{
		amount -= node_->GetComponent<Shield>()->shield_;
		if (amount < 0){amount = 0;}
	}

	if (operation == -1)//subtract
	{
		health_ -= amount;
	}
	else if (operation == 0)//set
	{
		health_ = amount;
	}
	else if (operation == 1)//add
	{
		health_ += amount;
	}

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(amount);
	msg_.WriteByte(operation);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);

	VariantMap vm;
	vm[HealthStatus::P_NODE] = node_;
	vm[HealthStatus::P_HEALTH] = health_;
	SendEvent(E_HEALTHSTATUS,vm);
}

void Health::ClientExecute()
{
	int amount = message_->ReadInt();
	char operation = message_->ReadByte();

	if (operation == -1)//subtract
	{
		health_ -= amount;

		VariantMap vm;
		vm[SoundRequest::P_NODE] = node_;
		vm[SoundRequest::P_SOUNDTYPE] = SoundPlayer::SOUNDTYPE_HURT_;
		SendEvent(E_SOUNDREQUEST,vm);

	}
	else if (operation == 0)//set
	{
		health_ = amount;
	}
	else if (operation == 1)//add
	{
		health_ += amount;
	}
}

void Health::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
