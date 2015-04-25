/*
 * Cleanse.cpp
 *
 *  Created on: Apr 19, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
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
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "Cleanse.h"
#include "Silence.h"
#include "SoundPlayer.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

Cleanse::Cleanse(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "Cleanse";
	clientExecuting_ = false;
	cooldown_ = 5.0f;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	//SetUpdateEventMask(USE_UPDATE);
}

Cleanse::~Cleanse()
{
}

void Cleanse::Start()
{
	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
}

void Cleanse::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

	elapsedTime_ += timeStep;
	if (elapsedTime_ >= cooldown_)
	{
		clientExecuting_ = false;
		UnsubscribeFromEvent(E_UPDATE);
	}
}

void Cleanse::ServerSync()
{return;
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Cleanse::ClientSync()
{
}

void Cleanse::ClientRequestExecute()
{
	if (!clientExecuting_ && !node_->GetComponent<Silence>()->silenced_)
	{
		clientExecuting_ = true;
		elapsedTime_ = 0.0f;
		gameMode_->touchSubscriberCount_++;
		SubscribeToEvent(E_TOUCHEND, HANDLER(Cleanse, TouchUp));
	}
}

void Cleanse::ServerExecute()
{
	int clientID = message_->ReadInt();

	Node* noed = gameMode_->GetClientIDNode(clientID);

	clientExecuting_ = true;
	elapsedTime_ = 0.0f;
	SubscribeToEvent(E_UPDATE, HANDLER(Cleanse, HandleUpdate));

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(clientID);
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);

	VariantMap vm;
	vm[CleanseStatus::P_NODE] = noed;
	SendEvent(E_CLEANSESTATUS,vm);
}

void Cleanse::ClientExecute()
{
	int clientID = message_->ReadInt();
	lagTime_ += message_->ReadFloat();

	Node* noed = gameMode_->GetClientIDNode(clientID);

	victoria_ = noed->GetPosition();
	victoria_.y_ += noed->GetComponent<Cleanse>()->beeBox_.HalfSize().y_;
	noed->GetComponent<Cleanse>()->particleStartNode_->SetWorldPosition(victoria_);
	noed->GetComponent<Cleanse>()->particleStartNode_->SetRotation(noed->GetRotation());
	noed->GetComponent<Cleanse>()->emitterStartFX_->Reset();
	noed->GetComponent<Cleanse>()->emitterStartFX_->SetEmitting(true);
	noed->GetComponent<Cleanse>()->particleStartNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/cleanse/cleanse.ogg"), 0.0f, 0.25f);

	VariantMap vm;
	vm[CleanseStatus::P_NODE] = noed;
	SendEvent(E_CLEANSESTATUS,vm);

	gameMode_->RequestAnimation(clientID_,"attack", false, 1);

	VariantMap vm2;//todo find out if events are processed immediately so i can just clear() vm
	vm2[SoundRequest::P_NODE] = node_;
	vm2[SoundRequest::P_SOUNDTYPE] = SoundPlayer::SOUNDTYPE_CAST_;
	SendEvent(E_SOUNDREQUEST,vm2);

	clientExecuting_ = true;
	elapsedTime_ = lagTime_;
	SubscribeToEvent(E_UPDATE, HANDLER(Cleanse, HandleUpdate));
}

void Cleanse::TouchUp(StringHash eventType, VariantMap& eventData)
{
	if (main_->ui_->GetFocusElement())
	{
		return;
	}

	gameMode_->touchSubscriberCount_--;
	UnsubscribeFromEvent(E_TOUCHEND);

	using namespace TouchEnd;

	Ray cameraRay = cameraNode_->GetComponent<Camera>()->GetScreenRay(
			(float) eventData[P_X].GetInt() / main_->graphics_->GetWidth(),
			(float) eventData[P_Y].GetInt() / main_->graphics_->GetHeight());

	PhysicsRaycastResult raeResult_;

	scene_->GetComponent<PhysicsWorld>()->RaycastSingle(raeResult_, cameraRay, 1000.0f, 2);//todo define masks.

	int clientID = 0;

	if (raeResult_.body_)
	{
		clientID = gameMode_->GetNodeClientID(raeResult_.body_->GetNode());
	}
	else
	{
		clientExecuting_ = false;
		return;
	}

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_RequestMechanicExecute_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(clientID);
	main_->network_->GetServerConnection()->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);

//todo clientside prediction
}

void Cleanse::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");

	particleStartNode_ = scene_->CreateChild(0,LOCAL);
	emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/cleanse.xml"));
	particleStartNode_->SetWorldScale(Vector3::ONE * 2.0f);
	emitterStartFX_->SetEmitting(false);
	emitterStartFX_->SetViewMask(1);

	particleStartNode_->CreateComponent<SoundSource3D>(LOCAL);
}
