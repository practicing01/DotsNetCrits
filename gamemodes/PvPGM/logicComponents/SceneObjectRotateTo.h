/*
 * SceneObjectRotateTo.h
 *
 *  Created on: Jan 3, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Math/Quaternion.h>

#include "GameMechanic.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

EVENT(E_SCENEOBJECTROTATETOCOMPLETE, SceneObjectRotateToComplete)
{
   PARAM(P_NODE, Node);  //node
}

class SceneObjectRotateTo: public LogicComponent, GameMechanic
{
	OBJECT(SceneObjectRotateTo);
public:
	SceneObjectRotateTo(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
	void OnRotateToComplete();
	void RotateTo(Quaternion dest, float speed, bool stopOnCompletion);
	void ServerSync();
	void ClientSync();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();

    VectorBuffer msg_;

	Quaternion rotateToDest_;
	Quaternion rotateToLoc_;
	float rotateToSpeed_;
	float rotateToTravelTime_;
	float rotateToElapsedTime_;
	float inderp_;
	float remainingDist_;
	bool rotateToStopOnTime_;
	bool isRotating_;
};
