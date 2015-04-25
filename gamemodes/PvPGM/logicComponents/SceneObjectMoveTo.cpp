/*
 * SceneObjectMoveTo.cpp
 *
 *  Created on: Dec 9, 2014
 *      Author: practicing01 and others from the internet.
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>

#include "SceneObjectMoveTo.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

SceneObjectMoveTo::SceneObjectMoveTo(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "SceneObjectMoveTo";
	isMoving_ = false;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void SceneObjectMoveTo::OnMoveToComplete()
{
	VariantMap vm;
	vm[SceneObjectMoveToComplete::P_NODE] = node_;
	SendEvent(E_SCENEOBJECTMOVETOCOMPLETE,vm);
}

void SceneObjectMoveTo::MoveTo(Vector3 dest, float speed, float speedRamp, bool stopOnCompletion)
{
	moveToSpeed_ = speed;
	speedRamp_ = speedRamp;
	moveToDest_ = dest;
	moveToLoc_ = node_->GetPosition();
	moveToDir_ = dest - moveToLoc_;
	moveToDir_.Normalize();
	moveToTravelTime_ = (moveToDest_ - moveToLoc_).Length() / speed;
	moveToElapsedTime_ = 0;
	moveToStopOnTime_ = stopOnCompletion;
	isMoving_ = true;
}

void SceneObjectMoveTo::Update(float timeStep)
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
		remainingDist_ = (node_->GetPosition() - moveToDest_).Length();
		node_->SetPosition(node_->GetPosition().Lerp(moveToDest_, inderp_ / remainingDist_));
		moveToElapsedTime_ += timeStep;
		if (moveToElapsedTime_ >= moveToTravelTime_)
		{
			isMoving_ = false;
			if (moveToStopOnTime_ == true)
			{
			}
			OnMoveToComplete();
		}
	}
}

void SceneObjectMoveTo::ServerSync()
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
		msg_.WriteBool(moveToStopOnTime_);
	}
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void SceneObjectMoveTo::ClientSync()
{
	Vector3 pos = message_->ReadVector3();
	node_->SetPosition(pos);
	bool isMoving = message_->ReadBool();
	if (isMoving)
	{
		Vector3 dest = message_->ReadVector3();
		float speed = message_->ReadFloat();
		float speedRamp = message_->ReadFloat();
		bool stop = message_->ReadBool();
		MoveTo(dest, speed, speedRamp + lagTime_, stop);
	}
}

void SceneObjectMoveTo::ClientRequestExecute()
{
}

void SceneObjectMoveTo::ServerExecute()
{
	Vector3 dest = message_->ReadVector3();
	float speed = message_->ReadFloat();
	bool stopOnCompletion = message_->ReadBool();
	float speedRamp = speed + lagTime_;

	MoveTo(dest, speed, speedRamp, stopOnCompletion);

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteVector3(dest);
	msg_.WriteFloat(speed);
	msg_.WriteBool(stopOnCompletion);
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void SceneObjectMoveTo::ClientExecute()
{
	Vector3 dest = message_->ReadVector3();
	float speed = message_->ReadFloat();
	bool stopOnCompletion = message_->ReadBool();
	float lagTime = message_->ReadFloat();
	float speedRamp = speed + lagTime;
	speedRamp += lagTime_;

	MoveTo(dest, speed, speedRamp, stopOnCompletion);
}

void SceneObjectMoveTo::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
