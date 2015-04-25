/*
 * AOEHealHeal.cpp
 *
 *  Created on: Apr 18, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
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

#include "AOEHeal.h"
#include "Silence.h"
#include "SoundPlayer.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

AOEHeal::AOEHeal(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "AOEHeal";
	clientExecuting_ = false;
	cooldown_ = 10.0f;
	pulseRate_ = 1.0f;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	//SetUpdateEventMask(USE_UPDATE);
}

AOEHeal::~AOEHeal()
{
}

void AOEHeal::Start()
{
}

void AOEHeal::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

	if (gameMode_->GetIsHost())
	{
		pulseElapsedTime_ += timeStep;
		if (pulseElapsedTime_ >= pulseRate_)
		{
			pulseElapsedTime_ = 0.0f;

			node_->GetScene()->GetComponent<PhysicsWorld>()->GetRigidBodies(rigidBodies_, Sphere(particleEndNode_->GetPosition(), radius_), 2);

			bool hit = false;

			if (rigidBodies_.Size())
			{
				data_.Clear();
				data_.WriteString("Health");//mechanicID
				data_.WriteInt(10);//amount
				data_.WriteByte(1);//add operation

				for (int x = 0; x < rigidBodies_.Size(); x++)
				{
					Node* noed = rigidBodies_[x]->GetNode();
					//if (noed != node_)
					{
						hit = true;
						gameMode_->RequestMechanicExecution(noed, data_);
						/*victoria_ = noed->GetPosition();
						//todo particles for each hit
						BoundingBox beeBox = noed->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
						victoria_.y_ += beeBox.Size().y_;*/
					}
				}
			}
		}
	}

	elapsedTime_ += timeStep;
	if (elapsedTime_ >= cooldown_)
	{
		emitterEndFX_->SetEmitting(false);
		clientExecuting_ = false;
		UnsubscribeFromEvent(E_UPDATE);
	}
}

void AOEHeal::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteBool(clientExecuting_);
	if (clientExecuting_)
	{
		msg_.WriteVector3(particleEndNode_->GetPosition());
		msg_.WriteFloat(elapsedTime_);
	}
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void AOEHeal::ClientSync()
{
	clientExecuting_ = message_->ReadBool();
	if (clientExecuting_)
	{
		particleEndNode_->SetPosition(message_->ReadVector3());
		emitterEndFX_->SetEmitting(true);
		elapsedTime_ = message_->ReadFloat();
		elapsedTime_ += lagTime_;
		SubscribeToEvent(E_UPDATE, HANDLER(AOEHeal, HandleUpdate));
	}
}

void AOEHeal::ClientRequestExecute()
{
	if (!clientExecuting_ && !node_->GetComponent<Silence>()->silenced_)
	{
		clientExecuting_ = true;
		elapsedTime_ = 0.0f;
		gameMode_->touchSubscriberCount_++;
		SubscribeToEvent(E_TOUCHEND, HANDLER(AOEHeal, TouchUp));
	}
}

void AOEHeal::ServerExecute()
{//todo casting times  (based on lag)
	victoria_ = message_->ReadVector3();
	particleEndNode_->SetPosition(victoria_);

	clientExecuting_ = true;
	elapsedTime_ = 0.0f;
	pulseElapsedTime_ = 0.0f;
	SubscribeToEvent(E_UPDATE, HANDLER(AOEHeal, HandleUpdate));

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteVector3(victoria_);
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
	//gameMode_->RequestAnimation(clientID_,"attack", false, 1);
}

void AOEHeal::ClientExecute()
{//todo casting times (based on lag)
	/*particleStartNode_->SetPosition(node_->GetPosition());
	emitterStartFX_->SetEmitting(true);*/
	victoria_ = message_->ReadVector3();
	particleEndNode_->SetPosition(victoria_);
	lagTime_ += message_->ReadFloat();
	gameMode_->RequestAnimation(clientID_,"attack", false, 1);
	particleEndNode_->SetPosition(victoria_);
	emitterEndFX_->SetEmitting(true);
	particleEndNode_->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>("Sounds/aoeheal/aoeheal.ogg"));

	VariantMap vm;
	vm[SoundRequest::P_NODE] = node_;
	vm[SoundRequest::P_SOUNDTYPE] = SoundPlayer::SOUNDTYPE_CAST_;
	SendEvent(E_SOUNDREQUEST,vm);

	clientExecuting_ = true;
	elapsedTime_ = lagTime_;
	SubscribeToEvent(E_UPDATE, HANDLER(AOEHeal, HandleUpdate));
}

void AOEHeal::TouchUp(StringHash eventType, VariantMap& eventData)
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

	scene_->GetComponent<PhysicsWorld>()->RaycastSingle(raeResult_, cameraRay, 1000.0f, 1);//todo define masks.

	if (raeResult_.body_)
	{
		victoria_ = raeResult_.position_;
		vectoria_ -= -node_->GetDirection() * (radius_ * 1.01f);
	}
	else
	{
		clientExecuting_ = false;
		return;
	}

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_RequestMechanicExecute_);
	msg_.WriteString(mechanicID_);
	msg_.WriteVector3(victoria_);
	main_->network_->GetServerConnection()->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);

//todo clientside prediction
}

void AOEHeal::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");

	/*particleStartNode_ = scene_->CreateChild(0,LOCAL);
	emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/aoeheal.xml"));
	particleStartNode_->SetWorldScale(Vector3::ONE);
	emitterStartFX_->SetEmitting(false);
	emitterStartFX_->SetViewMask(1);*/

	particleEndNode_ = scene_->CreateChild(0,LOCAL);
	emitterEndFX_ = particleEndNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterEndFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/aoeheal.xml"));
	particleEndNode_->SetWorldScale(Vector3::ONE * 5.0f);
	emitterEndFX_->SetEmitting(false);
	emitterEndFX_->SetViewMask(1);
	radius_ = particleEndNode_->GetWorldScale().x_ * 2.0f;

	particleEndNode_->CreateComponent<SoundSource3D>(LOCAL);
}
