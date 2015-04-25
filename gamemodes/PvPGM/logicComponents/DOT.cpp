/*
 * DOT.cpp
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

#include "DOT.h"
#include "Cleanse.h"
#include "Silence.h"
#include "SoundPlayer.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

DOT::DOT(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "DOT";
	clientExecuting_ = false;
	cooldown_ = 10.0f;
	doted_ = false;
	DOTDuration_ = 5.0f;
	DOTInterval_ = 1.0f;
}

DOT::~DOT()
{
}

void DOT::Start()
{
	SubscribeToEvent(E_UPDATE, HANDLER(DOT, HandleUpdate));
	SubscribeToEvent(E_CLEANSESTATUS, HANDLER(DOT, HandleCleanseStatus));
}

void DOT::HandleUpdate(StringHash eventType, VariantMap& eventData)
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

	if (doted_)
	{
		DOTElapsedTime_ += timeStep;
		if (DOTElapsedTime_ >= DOTDuration_)
		{
			doted_ = false;
			node_->RemoveChild(particleEndNode_);
			emitterEndFX_->SetEmitting(false);
		}

		DOTIntervalElapsedTime_ += timeStep;
		if (DOTIntervalElapsedTime_ >= DOTInterval_)
		{
			DOTIntervalElapsedTime_ = 0.0f;
			if (gameMode_->GetIsHost())
			{
				data_.Clear();
				data_.WriteString("Health");//mechanicID
				data_.WriteInt(10);//amount
				data_.WriteByte(-1);//subtract operation
				gameMode_->RequestMechanicExecution(node_, data_);
			}
		}
	}

	/*if (!clientExecuting_ && !doted_)
	{
		UnsubscribeFromEvent(E_UPDATE);
	}*/
}

void DOT::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteBool(doted_);
	if (doted_)
	{
		msg_.WriteFloat(DOTElapsedTime_);
	}
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void DOT::ClientSync()
{
	doted_ = message_->ReadBool();
	if (doted_)
	{
		DOTElapsedTime_ = message_->ReadFloat();
		DOTElapsedTime_ += lagTime_;

		node_->AddChild(particleEndNode_);
		victoria_ = node_->GetPosition();
		victoria_.y_ += beeBox_.Size().y_;
		particleEndNode_->SetWorldPosition(victoria_);
		emitterEndFX_->SetEmitting(true);
		//node_->GetComponent<DOT>()->SubscribeToEvent(E_UPDATE, HANDLER(DOT, HandleUpdate));
	}
}

void DOT::ClientRequestExecute()
{
	if (!clientExecuting_ && !node_->GetComponent<Silence>()->silenced_)
	{
		clientExecuting_ = true;
		elapsedTime_ = 0.0f;
		gameMode_->touchSubscriberCount_++;
		SubscribeToEvent(E_TOUCHEND, HANDLER(DOT, TouchUp));
	}
}

void DOT::ServerExecute()
{//todo casting times  (based on lag)
	int clientID = message_->ReadInt();

	Node* noed = gameMode_->GetClientIDNode(clientID);
	/*noed->AddChild(noed->GetComponent<DOT>()->particleEndNode_);
	victoria_ = noed->GetPosition();
	victoria_.y_ += noed->GetComponent<DOT>()->beeBox_.Size().y_;
	noed->GetComponent<DOT>()->particleEndNode_->SetWorldPosition(victoria_);
	noed->GetComponent<DOT>()->emitterEndFX_->SetEmitting(true);//server doesn't need to display gfx*/
	noed->GetComponent<DOT>()->doted_ = true;
	noed->GetComponent<DOT>()->DOTElapsedTime_ = 0.0f;
	noed->GetComponent<DOT>()->DOTIntervalElapsedTime_ = 0.0f;
	//noed->GetComponent<DOT>()->SubscribeToEvent(E_UPDATE, HANDLER(DOT, HandleUpdate));

	clientExecuting_ = true;
	elapsedTime_ = 0.0f;
	//SubscribeToEvent(E_UPDATE, HANDLER(DOT, HandleUpdate));

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(clientID);
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
	//gameMode_->RequestAnimation(clientID_,"attack", false, 1);
}

void DOT::ClientExecute()
{//todo casting times (based on lag)
	/*particleStartNode_->SetPosition(node_->GetPosition());
	emitterStartFX_->SetEmitting(true);*/

	int clientID = message_->ReadInt();
	lagTime_ += message_->ReadFloat();

	Node* noed = gameMode_->GetClientIDNode(clientID);
	noed->AddChild(noed->GetComponent<DOT>()->particleEndNode_);
	victoria_ = noed->GetPosition();
	victoria_.y_ += noed->GetComponent<DOT>()->beeBox_.Size().y_;
	noed->GetComponent<DOT>()->particleEndNode_->SetWorldPosition(victoria_);
	noed->GetComponent<DOT>()->emitterEndFX_->SetEmitting(true);
	noed->GetComponent<DOT>()->doted_ = true;
	noed->GetComponent<DOT>()->DOTElapsedTime_ = lagTime_;
	//noed->GetComponent<DOT>()->SubscribeToEvent(E_UPDATE, HANDLER(DOT, HandleUpdate));
	noed->GetComponent<DOT>()->particleEndNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/dot/dot.ogg"), 0.0f, 0.5f);

	gameMode_->RequestAnimation(clientID_,"attack", false, 1);

	VariantMap vm;
	vm[SoundRequest::P_NODE] = node_;
	vm[SoundRequest::P_SOUNDTYPE] = SoundPlayer::SOUNDTYPE_CAST_;
	SendEvent(E_SOUNDREQUEST,vm);

	clientExecuting_ = true;
	elapsedTime_ = lagTime_;
	//SubscribeToEvent(E_UPDATE, HANDLER(DOT, HandleUpdate));
}

void DOT::TouchUp(StringHash eventType, VariantMap& eventData)
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

void DOT::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");

	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();

	/*particleStartNode_ = scene_->CreateChild(0,LOCAL);
	emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dot.xml"));
	particleStartNode_->SetWorldScale(Vector3::ONE);
	emitterStartFX_->SetEmitting(false);
	emitterStartFX_->SetViewMask(1);*/

	particleEndNode_ = scene_->CreateChild(0,LOCAL);
	emitterEndFX_ = particleEndNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterEndFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/dot.xml"));
	particleEndNode_->SetWorldScale(Vector3::ONE * 500.0f);
	emitterEndFX_->SetEmitting(false);
	emitterEndFX_->SetViewMask(1);

	particleEndNode_->CreateComponent<SoundSource3D>(LOCAL);
}

void DOT::HandleCleanseStatus(StringHash eventType, VariantMap& eventData)
{
	using namespace CleanseStatus;

	SharedPtr<Node> node = SharedPtr<Node>(static_cast<Node*>(eventData[P_NODE].GetPtr()));

	if (node == node_)
	{
		doted_ = false;
		node_->RemoveChild(particleEndNode_);
		emitterEndFX_->SetEmitting(false);
	}
}
