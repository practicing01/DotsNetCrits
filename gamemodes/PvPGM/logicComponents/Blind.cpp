/*
 * Blind.cpp
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

#include "Blind.h"
#include "Cleanse.h"
#include "Silence.h"
#include "SoundPlayer.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

Blind::Blind(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "Blind";
	clientExecuting_ = false;
	cooldown_ = 10.0f;
	blinded_ = false;
	blindDuration_ = 5.0f;
}

Blind::~Blind()
{
}

void Blind::Start()
{
	SubscribeToEvent(E_UPDATE, HANDLER(Blind, HandleUpdate));
	SubscribeToEvent(E_CLEANSESTATUS, HANDLER(Blind, HandleCleanseStatus));
}

void Blind::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

	if (clientExecuting_)
	{
		elapsedTime_ += timeStep;
		if (elapsedTime_ >= cooldown_)
		{
			clientExecuting_ = false;
		}
	}

	if (blinded_)
	{
		blindElapsedTime_ += timeStep;
		if (blindElapsedTime_ >= blindDuration_)
		{
			blinded_ = false;
			node_->RemoveChild(particleEndNode_);
			emitterEndFX_->SetEmitting(false);
		}
	}

	/*if (!clientExecuting_ && !blinded_)
	{
		UnsubscribeFromEvent(E_UPDATE);
	}*/
}

void Blind::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteBool(blinded_);
	if (blinded_)
	{
		msg_.WriteFloat(blindElapsedTime_);
	}
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void Blind::ClientSync()
{
	blinded_ = message_->ReadBool();
	if (blinded_)
	{
		blindElapsedTime_ = message_->ReadFloat();
		blindElapsedTime_ += lagTime_;

		node_->AddChild(particleEndNode_);
		victoria_ = node_->GetPosition();
		victoria_.y_ += beeBox_.Size().y_;
		particleEndNode_->SetWorldPosition(victoria_);
		emitterEndFX_->SetEmitting(true);
		//node_->GetComponent<Blind>()->SubscribeToEvent(E_UPDATE, HANDLER(Blind, HandleUpdate));
	}
}

void Blind::ClientRequestExecute()
{
	if (!clientExecuting_ && !node_->GetComponent<Silence>()->silenced_)
	{
		clientExecuting_ = true;
		elapsedTime_ = 0.0f;
		gameMode_->touchSubscriberCount_++;
		SubscribeToEvent(E_TOUCHEND, HANDLER(Blind, TouchUp));
	}
}

void Blind::ServerExecute()
{//todo casting times  (based on lag)
	int clientID = message_->ReadInt();

	Node* noed = gameMode_->GetClientIDNode(clientID);
	/*noed->AddChild(noed->GetComponent<Blind>()->particleEndNode_);
	victoria_ = noed->GetPosition();
	victoria_.y_ += noed->GetComponent<Blind>()->beeBox_.Size().y_;
	noed->GetComponent<Blind>()->particleEndNode_->SetWorldPosition(victoria_);
	noed->GetComponent<Blind>()->emitterEndFX_->SetEmitting(true);//server doesn't need to display gfx*/
	noed->GetComponent<Blind>()->blinded_ = true;
	noed->GetComponent<Blind>()->blindElapsedTime_ = 0.0f;
	//noed->GetComponent<Blind>()->SubscribeToEvent(E_UPDATE, HANDLER(Blind, HandleUpdate));

	clientExecuting_ = true;
	elapsedTime_ = 0.0f;
	//SubscribeToEvent(E_UPDATE, HANDLER(Blind, HandleUpdate));

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(clientID);
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
	//gameMode_->RequestAnimation(clientID_,"attack", false, 1);
}

void Blind::ClientExecute()
{//todo casting times (based on lag)
	/*particleStartNode_->SetPosition(node_->GetPosition());
	emitterStartFX_->SetEmitting(true);*/

	int clientID = message_->ReadInt();
	lagTime_ += message_->ReadFloat();

	Node* noed = gameMode_->GetClientIDNode(clientID);
	noed->AddChild(noed->GetComponent<Blind>()->particleEndNode_);
	victoria_ = noed->GetPosition();
	victoria_.y_ += noed->GetComponent<Blind>()->beeBox_.Size().y_;
	noed->GetComponent<Blind>()->particleEndNode_->SetWorldPosition(victoria_);
	noed->GetComponent<Blind>()->emitterEndFX_->SetEmitting(true);
	noed->GetComponent<Blind>()->blinded_ = true;
	noed->GetComponent<Blind>()->blindElapsedTime_ = lagTime_;
	//noed->GetComponent<Blind>()->SubscribeToEvent(E_UPDATE, HANDLER(Blind, HandleUpdate));
	noed->GetComponent<Blind>()->particleEndNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/blind/blind.ogg"), 0.0f, 0.5f);

	gameMode_->RequestAnimation(clientID_,"attack", false, 1);

	VariantMap vm;
	vm[SoundRequest::P_NODE] = node_;
	vm[SoundRequest::P_SOUNDTYPE] = SoundPlayer::SOUNDTYPE_CAST_;
	SendEvent(E_SOUNDREQUEST,vm);

	clientExecuting_ = true;
	elapsedTime_ = lagTime_;
	//SubscribeToEvent(E_UPDATE, HANDLER(Blind, HandleUpdate));
}

void Blind::TouchUp(StringHash eventType, VariantMap& eventData)
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

void Blind::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");

	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();

	/*particleStartNode_ = scene_->CreateChild(0,LOCAL);
	emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/blind.xml"));
	particleStartNode_->SetWorldScale(Vector3::ONE);
	emitterStartFX_->SetEmitting(false);
	emitterStartFX_->SetViewMask(1);*/

	particleEndNode_ = scene_->CreateChild(0,LOCAL);
	emitterEndFX_ = particleEndNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterEndFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/blind.xml"));
	particleEndNode_->SetWorldScale(Vector3::ONE * 500.0f);
	emitterEndFX_->SetEmitting(false);
	emitterEndFX_->SetViewMask(1);

	particleEndNode_->CreateComponent<SoundSource3D>(LOCAL);
}

void Blind::HandleCleanseStatus(StringHash eventType, VariantMap& eventData)
{
	using namespace CleanseStatus;

	SharedPtr<Node> node = SharedPtr<Node>(static_cast<Node*>(eventData[P_NODE].GetPtr()));

	if (node == node_)
	{
		blinded_ = false;
		node_->RemoveChild(particleEndNode_);
		emitterEndFX_->SetEmitting(false);
	}
}
