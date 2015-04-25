/*
 * GameMechanic.h
 *
 *  Created on: Apr 7, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include "../../../Urho3DPlayer.h"
#include "../../GameMode.h"

class GameMechanic
{
public:
	virtual ~GameMechanic();
	virtual void ServerSync() = 0;
	virtual void ClientSync() = 0;
	virtual void ClientRequestExecute() = 0;
	virtual void ServerExecute() = 0;
	virtual void ClientExecute() = 0;
	virtual void SetGameMode() = 0;
	String mechanicID_;
	Connection* receiver_;
	int clientID_;
	int clientIDSelf_;
	MemoryBuffer* message_;
	VectorBuffer data_;
	float lagTime_;
	GameMode* gameMode_;
	Urho3DPlayer* main_;
	SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    Vector3 victoria_;
	Vector3 vectoria_;
	bool clientExecuting_;
	float cooldown_;
};
