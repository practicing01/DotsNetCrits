/*
 * Shield.h
 *
 *  Created on: Apr 20, 2015
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

class Shield: public LogicComponent, GameMechanic
{
	OBJECT(Shield);
public:
	Shield(Context* context);
	~Shield();
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleCleanseStatus(StringHash eventType, VariantMap& eventData);
	virtual void Start();
	void TouchUp(StringHash eventType, VariantMap& eventData);
	void ServerSync();
	void ClientSync();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();

	VectorBuffer msg_;

	/*ParticleEmitter* emitterStartFX_;
	SharedPtr<Node> particleStartNode_;*/
	ParticleEmitter* emitterEndFX_;
	SharedPtr<Node> particleEndNode_;
	BoundingBox beeBox_;
	float elapsedTime_;
	float ShieldDuration_;
	float ShieldElapsedTime_;
	bool shielded_;
	int shield_;
};
