/*
 * Server.h
 *
 *  Created on: Mar 18, 2015
 *      Author: practicing01 and https://github.com/sw17ch/udp_broadcast_example
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../../Urho3DPlayer.h"
#include <Urho3D/Network/Network.h>

#include "../netpulse/NetPulse.h"
#include "../clientdata/ClientData.h"
#include "../../gamemodes/GameMode.h"

using namespace Urho3D;

class Server : public Object
{
	OBJECT(Server);
public:
	Server(Context* context, Urho3DPlayer* main);
	~Server();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandleClientDisconnect(StringHash eventType, VariantMap& eventData);
	void HandleClientConnect(StringHash eventType, VariantMap& eventData);
    void HandleNetworkMessage(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	NetPulse* netpulse_;
	float elapsedTime_;

	VectorBuffer msg_;
	Network* network_;
	int clientIDCount_;

	GameMode* gamemode_;
	String gamemodeName_;

	Vector<ClientData*> clientData_;
};
