/*
 * Client.h
 *
 *  Created on: Mar 18, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../../Urho3DPlayer.h"

#include "../netpulse/NetPulse.h"
#include "../clientdata/ClientData.h"
#include "../../gamemodes/GameMode.h"

using namespace Urho3D;

class Client : public Object
{
	OBJECT(Client);
public:
	Client(Context* context, Urho3DPlayer* main, String ipAddress);
	~Client();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandleServerConnect(StringHash eventType, VariantMap& eventData);
	void HandleServerDisconnect(StringHash eventType, VariantMap& eventData);
	void HandleConnectFailed(StringHash eventType, VariantMap& eventData);
    void HandleNetworkMessage(StringHash eventType, VariantMap& eventData);
    void LogOut();

	Urho3DPlayer* main_;
	NetPulse* netpulse_;
	float elapsedTime_;

	GameMode* gamemode_;
	String gamemodeName_;

	VectorBuffer msg_;

	Vector<ClientData*> clientData_;

};
