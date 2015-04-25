/*
 * ThirdPersonCamera.h
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

class ThirdPersonCamera: public LogicComponent, GameMechanic
{
	OBJECT(ThirdPersonCamera);
public:
	ThirdPersonCamera(Context* context);
	~ThirdPersonCamera();
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
	void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
	virtual void Start();
	virtual void OnSetEnabled();
	void ServerSync();
	void ClientSync();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();

	VectorBuffer msg_;

    float rayDistance_;
    float inderp_;
    float remainingDist_;
	Vector3 victoria_;
	Vector3 vectoria_;
	Vector3 camOrigin_;
	BoundingBox beeBox_;
	Ray cameraRay_;
};
