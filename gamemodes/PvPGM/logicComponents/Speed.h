/*
 * Speed.h
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

class Speed: public LogicComponent, GameMechanic
{
	OBJECT(Speed);
public:
	Speed(Context* context);
	~Speed();
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

	float speed_;
};
