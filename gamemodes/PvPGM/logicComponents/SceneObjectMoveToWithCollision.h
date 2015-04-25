/*
 * SceneObjectMoveTo.h
 *
 *  Created on: Dec 9, 2014
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include "SceneObjectMoveTo.h"
#include "GameMechanic.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class SceneObjectMoveToWithCollision: public LogicComponent, GameMechanic
{
	OBJECT(SceneObjectMoveToWithCollision);
public:
	SceneObjectMoveToWithCollision(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	//virtual void Update(float timeStep);
	virtual void FixedUpdate(float timeStep);
	void OnMoveToComplete();
	void MoveTo(Vector3 dest, float speed, float speedRamp, float gravity, bool stopOnCompletion);
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
	/// Handle startup. Called by LogicComponent base class.
	virtual void Start();
	void ServerSync();
	void ClientSync();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();

    VectorBuffer msg_;

	bool moveToStopOnTime_;
	bool isMoving_;
	float moveToSpeed_;
	float speedRamp_;
	float gravity_;
	float moveToTravelTime_;
	float moveToElapsedTime_;
	float inderp_;
	float remainingDist_;
	float radius_;
	Vector3 moveToDest_;
	Vector3 moveToLoc_;
	Vector3 moveToDir_;
	Ray rae_;
	PhysicsRaycastResult raeResult_;
	BoundingBox beeBox_;
};
