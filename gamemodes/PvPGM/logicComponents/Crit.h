/*
 * Crit.h
 *
 *  Created on: Apr 19, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include "GameMechanic.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class Crit: public LogicComponent, GameMechanic
{
	OBJECT(Crit);
public:
	Crit(Context* context);
	~Crit();
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	virtual void Start();
	void TouchUp(StringHash eventType, VariantMap& eventData);
	void ServerSync();
	void ClientSync();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();

	VectorBuffer msg_;

	ParticleEmitter* emitterStartFX_;
	SharedPtr<Node> particleStartNode_;
	ParticleEmitter* emitterEndFX_;
	SharedPtr<Node> particleEndNode_;
	BoundingBox beeBox_;
	Ray rae_;
	PODVector<PhysicsRaycastResult> raeResult_;
	PODVector<RigidBody*> rigidBodies_;
	float radius_;
	float elapsedTime_;
};
