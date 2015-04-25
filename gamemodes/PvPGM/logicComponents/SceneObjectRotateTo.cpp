/*
 * SceneObjectRotateTo.cpp
 *
 *  Created on: Jan 3, 2015
 *      Author: practicing01, dahlia, hd_, others from the internet
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>
#include "math.h"

#include "SceneObjectRotateTo.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

SceneObjectRotateTo::SceneObjectRotateTo(Context* context) :
LogicComponent(context)
{
	mechanicID_ = "SceneObjectRotateTo";
	isRotating_ = false;
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void SceneObjectRotateTo::OnRotateToComplete()
{
	VariantMap vm;
	vm[SceneObjectRotateToComplete::P_NODE] = node_;
	SendEvent(E_SCENEOBJECTROTATETOCOMPLETE,vm);
}

void SceneObjectRotateTo::RotateTo(Quaternion dest, float speed, bool stopOnCompletion)
{
	rotateToDest_ = dest;
	rotateToLoc_ = node_->GetRotation();
	rotateToSpeed_ = speed;

	rotateToTravelTime_ = 1.0f / rotateToSpeed_;

	inderp_ = 0.0f;
	rotateToElapsedTime_ = 0.0f;
	rotateToStopOnTime_ = stopOnCompletion;
	isRotating_ = true;
}

void SceneObjectRotateTo::Update(float timeStep)
{
	if (isRotating_ == true)
	{
		inderp_ += rotateToSpeed_*timeStep;
		node_->SetRotation(rotateToLoc_.Slerp(rotateToDest_, inderp_));
		rotateToElapsedTime_ += timeStep;
		if (rotateToElapsedTime_ >= rotateToTravelTime_)
		{
			isRotating_ = false;
			if (rotateToStopOnTime_ == true)
			{
			}
			OnRotateToComplete();
		}
	}
}

void SceneObjectRotateTo::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteQuaternion(node_->GetRotation());
	msg_.WriteBool(isRotating_);
	if (isRotating_)
	{
		msg_.WriteQuaternion(rotateToDest_);
		msg_.WriteFloat(rotateToSpeed_);//todo add speed ramp
		msg_.WriteBool(rotateToStopOnTime_);
	}
	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void SceneObjectRotateTo::ClientSync()
{
	Quaternion rot = message_->ReadQuaternion();
	node_->SetRotation(rot);
	bool isRotating = message_->ReadBool();
	if (isRotating)
	{
		Quaternion dest = message_->ReadQuaternion();
		float speed = message_->ReadFloat();
		bool stop = message_->ReadBool();
		RotateTo(dest, speed, stop);
	}
}

void SceneObjectRotateTo::ClientRequestExecute()
{
}

void SceneObjectRotateTo::ServerExecute()
{
	Quaternion dest = message_->ReadQuaternion();
	float speed = message_->ReadFloat();
	bool stopOnCompletion = message_->ReadBool();
	//float speedRamp = speed + lagTime_;//todo

	RotateTo(dest, speed, stopOnCompletion);

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_MechanicExecute_);
	msg_.WriteInt(clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteQuaternion(dest);
	msg_.WriteFloat(speed);
	msg_.WriteBool(stopOnCompletion);
	msg_.WriteFloat(lagTime_);
	main_->network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void SceneObjectRotateTo::ClientExecute()
{
	Quaternion dest = message_->ReadQuaternion();
	float speed = message_->ReadFloat();
	bool stopOnCompletion = message_->ReadBool();
	float lagTime = message_->ReadFloat();
	//float spedRamp = speed + lagTime;
	//speedRamp += lagTime_; todo

	RotateTo(dest, speed, stopOnCompletion);
}

void SceneObjectRotateTo::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
