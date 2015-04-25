/*
 * SceneObjectMoveTo.cpp
 *
 *  Created on: Dec 9, 2014
 *      Author: practicing01 and others from the internet.
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>

#include "SceneObjectMoveTo.h"
#include "SceneObjectMoveToWithCollision.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

SceneObjectMoveToWithCollision::SceneObjectMoveToWithCollision(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "SceneObjectMoveToWithCollision";
	isMoving_ = false;
	//SetUpdateEventMask(USE_UPDATE);
	SetUpdateEventMask(USE_FIXEDUPDATE);
}

void SceneObjectMoveToWithCollision::Start()
{
    // Component has been inserted into its scene node. Subscribe to events now
	radius_ = node_->GetComponent<CollisionShape>()->GetSize().x_;
	radius_ *= node_->GetWorldScale().x_;
	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
    SubscribeToEvent(GetNode(), E_NODECOLLISION, HANDLER(SceneObjectMoveToWithCollision, HandleNodeCollision));
}

void SceneObjectMoveToWithCollision::OnMoveToComplete()
{
	VariantMap vm;
	vm[SceneObjectMoveToComplete::P_NODE] = node_;
	SendEvent(E_SCENEOBJECTMOVETOCOMPLETE,vm);
	gameMode_->RequestAnimation(clientID_,"idle1", true, 0);
	//todo implement stopOnCompletion = false
	//calculate direction based on previous location and keep going
}

void SceneObjectMoveToWithCollision::MoveTo(Vector3 dest, float speed, float speedRamp, float gravity, bool stopOnCompletion)
{
	moveToSpeed_ = speed;
	speedRamp_ = speedRamp;
	gravity_ = gravity;
	moveToLoc_ = node_->GetPosition();
	moveToDest_ = dest;
	moveToDest_.y_ = moveToLoc_.y_;
	moveToDir_ = dest - moveToLoc_;
	moveToDir_.Normalize();
	moveToTravelTime_ = (moveToDest_ - moveToLoc_).Length() / speed;
	moveToElapsedTime_ = 0;
	moveToStopOnTime_ = stopOnCompletion;
	radius_ = node_->GetComponent<CollisionShape>()->GetSize().x_;
	radius_ *= node_->GetWorldScale().x_;
	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
	isMoving_ = true;
}

void SceneObjectMoveToWithCollision::FixedUpdate(float timeStep)
{
	if (isMoving_ == true)
	{
		if (speedRamp_ > moveToSpeed_)
		{
			speedRamp_ -= timeStep;
			if (speedRamp_ < moveToSpeed_)
			{
				speedRamp_ = moveToSpeed_;
			}
		}

		inderp_ = speedRamp_ * timeStep;
		moveToLoc_ = node_->GetPosition();
		moveToDest_.y_ = moveToLoc_.y_;
		remainingDist_ = (moveToLoc_ - moveToDest_).Length();

		victoria_ = moveToLoc_.Lerp(moveToDest_, inderp_ / remainingDist_);

		node_->SetPosition(victoria_);

		if (remainingDist_ <= 0.1f)
		{
			isMoving_ = false;
			OnMoveToComplete();
		}

		//Can't calculate time because the ground isn't flat.
		/*moveToElapsedTime_ += timeStep;
		if (moveToElapsedTime_ >= moveToTravelTime_)
		{
			isMoving_ = false;
			if (moveToStopOnTime_ == true)
			{
			}
			OnMoveToComplete();
		}*/
	}
/*//The following code works but will cause bouncing on slopes because the collision shape is a sphere.
	moveToLoc_ = node_->GetPosition();

	raeResult_.body_ = NULL;

	rae_.origin_ = moveToLoc_;
	rae_.direction_ = Vector3::DOWN;//todo change for games that might behave differently.

	node_->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(raeResult_, rae_, Abs(gravity_ * timeStep), 1);//todo define masks.

	if (!raeResult_.body_)
	{
		node_->SetPosition(moveToLoc_ + (rae_.direction_ * Abs(gravity_ * timeStep) ) );
	}

	return;*/
	//todo fix gravity unsynch from server (need gravity ramp)
	moveToLoc_ = node_->GetPosition();

	raeResult_.body_ = NULL;
	rae_.origin_ = node_->LocalToWorld(node_->GetComponent<CollisionShape>()->GetPosition());
	rae_.direction_ = Vector3::DOWN;//todo change for games that might behave differently.

	node_->GetScene()->GetComponent<PhysicsWorld>()->SphereCast(raeResult_, rae_, radius_, Abs(gravity_ * timeStep), 1);//todo define masks.

	if (!raeResult_.body_)
	{
		node_->SetPosition(moveToLoc_ + (rae_.direction_ * Abs(gravity_ * timeStep) ) );
	}

}

void SceneObjectMoveToWithCollision::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollision;

    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
    SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData[P_OTHERNODE].GetPtr()));

    while (!contacts.IsEof())
    {
        Vector3 contactPosition = contacts.ReadVector3();
        Vector3 contactNormal = contacts.ReadVector3();
        float contactDistance = contacts.ReadFloat();
        float contactImpulse = contacts.ReadFloat();

        vectoria_ = node_->GetPosition();

        float level = Abs(contactNormal.y_);

        if (level > 0.1f)
        {//todo check if above height, if so it's a ceiling, don't raise
        	if (vectoria_.y_ < contactPosition.y_)
        	{
        		vectoria_.y_ = contactPosition.y_;
        		node_->SetPosition(vectoria_);
        	}
        }
        else//wall
        {
        	vectoria_ -= -node_->GetDirection() * (contactDistance * 1.01f);
        	node_->SetPosition(vectoria_);
        	isMoving_ = false;
        	OnMoveToComplete();
        }
    }
}

void SceneObjectMoveToWithCollision::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteVector3(node_->GetPosition());
	msg_.WriteBool(isMoving_);
	if (isMoving_)
	{
		msg_.WriteVector3(moveToDest_);
		msg_.WriteFloat(moveToSpeed_);
		msg_.WriteFloat(speedRamp_);
		msg_.WriteFloat(gravity_);//todo add gravity ramp
		msg_.WriteBool(moveToStopOnTime_);
	}
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void SceneObjectMoveToWithCollision::ClientSync()
{
	Vector3 pos = message_->ReadVector3();
	node_->SetPosition(pos);
	bool isMoving = message_->ReadBool();
	if (isMoving)
	{
		Vector3 dest = message_->ReadVector3();
		float speed = message_->ReadFloat();
		float speedRamp = message_->ReadFloat();
		float gravity = message_->ReadFloat();
		bool stop = message_->ReadBool();
		MoveTo(dest, speed, speedRamp + lagTime_, gravity, stop);
	}
}

void SceneObjectMoveToWithCollision::ClientRequestExecute()
{
}

void SceneObjectMoveToWithCollision::ServerExecute()
{
	Vector3 dest = message_->ReadVector3();
	float speed = message_->ReadFloat();
	float gravity = message_->ReadFloat();//todo gravity speed ramp
	bool stopOnCompletion = message_->ReadBool();
	float speedRamp = speed + lagTime_;

	MoveTo(dest, speed, speedRamp, gravity, stopOnCompletion);
	gameMode_->RequestAnimation(clientID_,"run", true, 0);

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteVector3(dest);
	msg_.WriteFloat(speed);
	msg_.WriteFloat(gravity);
	msg_.WriteBool(stopOnCompletion);
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void SceneObjectMoveToWithCollision::ClientExecute()
{
	Vector3 dest = message_->ReadVector3();
	float speed = message_->ReadFloat();
	float gravity = message_->ReadFloat();
	bool stopOnCompletion = message_->ReadBool();
	float lagTime = message_->ReadFloat();
	float speedRamp = speed + lagTime;
	speedRamp += lagTime_;

	MoveTo(dest, speed, speedRamp, gravity, stopOnCompletion);
	gameMode_->RequestAnimation(clientID_,"run", true, 0);
}

void SceneObjectMoveToWithCollision::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
