/*
 * CameraToggle.h
 *
 *  Created on: Apr 15, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "GameMechanic.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class CameraToggle: public LogicComponent, GameMechanic
{
	OBJECT(CameraToggle);
public:
	CameraToggle(Context* context);
	~CameraToggle();
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

	char cameraType_;//0 = thirdperson, 1 = topdown todo enumerations
};
