/*
 * Health.h
 *
 *  Created on: Apr 11, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "GameMechanic.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

EVENT(E_HEALTHSTATUS, HealthStatus)
{
   PARAM(P_NODE, Node);
   PARAM(P_HEALTH, Health);
}

class Health: public LogicComponent, GameMechanic
{
	OBJECT(Health);
public:
	Health(Context* context);
	~Health();
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
	virtual void Start();
	void ServerSync();
	void ClientSync();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();

	VectorBuffer msg_;

	int health_;
};
