/*
 * Armor.h
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

class Armor: public LogicComponent, GameMechanic
{
	OBJECT(Armor);
public:
	Armor(Context* context);
	~Armor();
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

	int armor_;
};
