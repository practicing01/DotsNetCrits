/*
 * GameMode.h
 *
 *  Created on: Mar 19, 2015
 *      Author: practicing01
 */
#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include "../Urho3DPlayer.h"

class GameMode
{
public:
	virtual ~GameMode();
	virtual void ClientDisco(int clientID) = 0;
	virtual void LogOut() = 0;
	virtual Urho3DPlayer* GetMain() = 0;
	virtual Scene* GetScene() = 0;
	virtual Node* GetCamera() = 0;
	virtual bool GetIsHost() = 0;
	virtual void RequestAnimation(int clientID, String ani, bool loop, unsigned char layer) = 0;
	virtual void RequestMechanicExecution(Node* receiver, VectorBuffer& mechanicParams) = 0;
	virtual int GetNodeClientID(Node* noed) = 0;
	virtual Node* GetClientIDNode(int clientID) = 0;

	int touchSubscriberCount_;
};
