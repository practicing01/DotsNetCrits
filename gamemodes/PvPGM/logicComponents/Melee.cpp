/*
 * Melee.cpp
 *
 *  Created on: Apr 10, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
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

#include "Melee.h"
#include "Blind.h"
#include "SoundPlayer.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

Melee::Melee(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "Melee";
	clientExecuting_ = false;
	cooldown_ = 1.0f;
}

Melee::~Melee()
{
}

void Melee::Start()
{
	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
	radius_ = node_->GetComponent<CollisionShape>()->GetSize().x_;
	radius_ *= node_->GetWorldScale().x_;
}

void Melee::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

	elapsedTime_ += timeStep;
	if (elapsedTime_ >= cooldown_)
	{
		clientExecuting_ = false;
		UnsubscribeFromEvent(E_UPDATE);
	}
}

void Melee::ServerSync()
{
}

void Melee::ClientSync()
{
}

void Melee::ClientRequestExecute()
{
	if (!clientExecuting_)
	{
		clientExecuting_ = true;
		elapsedTime_ = 0.0f;
		SubscribeToEvent(E_UPDATE, HANDLER(Melee, HandleUpdate));

		msg_.Clear();
		msg_.WriteInt(PvPGM::MSG_RequestMechanicExecute_);
		msg_.WriteString(mechanicID_);
		main_->network_->GetServerConnection()->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
	}
	//todo clientside prediction
}

void Melee::ServerExecute()
{//todo casting times  (based on lag)

	/*raeResult_.Clear();
	rae_.origin_ = node_->GetPosition();
	rae_.origin_.y_ += beeBox_.Size().y_ * 0.5f;
	rae_.direction_ = node_->GetDirection();

	node_->GetScene()->GetComponent<PhysicsWorld>()->Raycast(raeResult_, rae_, radius_ * 2.0f, 2);//todo define masks.*/
	node_->GetScene()->GetComponent<PhysicsWorld>()->GetRigidBodies(rigidBodies_, Sphere(node_->GetPosition(), radius_), 2);

	bool hit = false;

	if (!node_->GetComponent<Blind>()->blinded_)
	{
		if (rigidBodies_.Size())
		{
			data_.Clear();
			data_.WriteString("Health");//mechanicID
			data_.WriteInt(10);//amount
			data_.WriteByte(-1);//subtract operation

			for (int x = 0; x < rigidBodies_.Size(); x++)
			{
				Node* noed = rigidBodies_[x]->GetNode();
				if (noed != node_)
				{
					hit = true;
					gameMode_->RequestMechanicExecution(noed, data_);
					victoria_ = noed->GetPosition();
					//todo particles for each hit
					BoundingBox beeBox = noed->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
					victoria_.y_ += beeBox.Size().y_;
				}
			}
		}
	}

	/*if (raeResult_.Size())
	{
		int index = -1;

		for (int x = 0; x < raeResult_.Size(); x++)
		{
			if (raeResult_[x].body_->GetNode() != node_)
			{
				if (index > -1)
				{
					if (raeResult_[x].distance_ < raeResult_[index].distance_)
					{
						index = x;
					}
				}
				else
				{
					index = x;
				}
			}
		}

		if (index != -1)
		{
			victoria_ = raeResult_[index].body_->GetNode()->GetPosition();
			victoria_.y_ += raeResult_[index].body_->GetNode()->GetComponent<AnimatedModel>()->GetWorldBoundingBox().Size().y_;
		}
	}*/

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteBool(hit);
	if (hit)
	{
		msg_.WriteVector3(victoria_);
	}
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
	//gameMode_->RequestAnimation(clientID_,"skill", false, 1);
}

void Melee::ClientExecute()
{//todo casting times (based on lag)
	bool hit = message_->ReadBool();
	if (hit)
	{
		vectoria_ = message_->ReadVector3();
	}
	lagTime_ += message_->ReadFloat();
	gameMode_->RequestAnimation(clientID_,"skill", false, 1);
	victoria_ = node_->GetPosition();
	victoria_.y_ += beeBox_.Size().y_;
	particleStartNode_->SetPosition(victoria_);
	emitterStartFX_->SetEmitting(true);
	if (hit)
	{
		particleEndNode_->SetPosition(vectoria_);
		emitterEndFX_->SetEmitting(true);
	}

	VariantMap vm;
	vm[SoundRequest::P_NODE] = node_;
	vm[SoundRequest::P_SOUNDTYPE] = SoundPlayer::SOUNDTYPE_MELEE_;
	SendEvent(E_SOUNDREQUEST,vm);

}

void Melee::TouchUp(StringHash eventType, VariantMap& eventData)
{

}

void Melee::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");

	particleStartNode_ = scene_->CreateChild(0,LOCAL);
	emitterStartFX_ = particleStartNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterStartFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/sweat.xml"));
	particleStartNode_->SetWorldScale(Vector3::ONE);
	emitterStartFX_->SetEmitting(false);
	emitterStartFX_->SetViewMask(1);

	particleEndNode_ = scene_->CreateChild(0,LOCAL);
	emitterEndFX_ = particleEndNode_->CreateComponent<ParticleEmitter>(LOCAL);
	emitterEndFX_->SetEffect(main_->cache_->GetResource<ParticleEffect>("Particle/blood.xml"));
	particleEndNode_->SetWorldScale(Vector3::ONE);
	emitterEndFX_->SetEmitting(false);
	emitterEndFX_->SetViewMask(1);
}
