/*
 * Client.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/IO/Log.h>

#include "Client.h"
#include "../networkconstants/NetworkConstants.h"
#include "../../gamemodes/PvPGM/PvPGM.h"
#include "../../networkMenu/NetworkMenu.h"

Client::Client(Context* context, Urho3DPlayer* main, String ipAddress) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;

    SubscribeToEvent(E_SERVERCONNECTED, HANDLER(Client, HandleServerConnect));
    SubscribeToEvent(E_SERVERDISCONNECTED, HANDLER(Client, HandleServerDisconnect));
    SubscribeToEvent(E_CONNECTFAILED, HANDLER(Client, HandleConnectFailed));
	SubscribeToEvent(E_NETWORKMESSAGE, HANDLER(Client, HandleNetworkMessage));

	netpulse_ = new NetPulse(context_, main_);
	main_->logicStates_.Push(netpulse_);

	main_->network_->Connect(ipAddress, 9001, 0);
}

Client::~Client()
{
	main_->logicStates_.Push(new NetworkMenu(context_, main_));
	main_->logicStates_.Remove(this);
}

void Client::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("client loop");
	elapsedTime_ += timeStep;
}

void Client::HandleServerConnect(StringHash eventType, VariantMap& eventData)
{
	//LOGERRORF("client connected to server");
}

void Client::HandleServerDisconnect(StringHash eventType, VariantMap& eventData)
{
	//LOGERRORF("client diconnected from server");
	netpulse_->ClearConnections();
	gamemode_->LogOut();
}

void Client::HandleConnectFailed(StringHash eventType, VariantMap& eventData)
{
	//LOGERRORF("client connection failed");
	LogOut();
}

void Client::HandleNetworkMessage(StringHash eventType, VariantMap& eventData)
{
	//LOGERRORF("network message");

	using namespace NetworkMessage;

	int msgID = eventData[P_MESSAGEID].GetInt();

	if (msgID == MSG_NEWCLIENT)
	{
		const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
		MemoryBuffer msg(data);
		int clientID = msg.ReadInt();
		bool self = msg.ReadBool();

		ClientData* newClient = new ClientData(clientID, NULL, self);//set null sender because the server will always be the sender to the client.
		clientData_.Push(newClient);
	}
    else if (msgID == MSG_CLIENTDISCO)
    {
    	const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
    	MemoryBuffer msg(data);
    	int clientID = msg.ReadInt();

    	for (int x = 0; x < clientData_.Size(); x++)
    	{
    		if (clientData_[x]->clientID_ == clientID)
    		{
    			ClientData* clientData = clientData_[x];
    			clientData_.Remove(clientData_[x]);
    			delete clientData_[x];
    			break;
    		}
    	}
    }
    else if (msgID == MSG_LOADGAMEMODE)
    {
    	const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
    	MemoryBuffer msg(data);
    	String gamemodeName = msg.ReadString();

    	gamemodeName_ = gamemodeName;

    	if (gamemodeName == "PvPGM")
    	{
    		gamemode_ = (GameMode*)(new PvPGM(context_, main_, false, NULL, this));
    		main_->logicStates_.Push((Object*)gamemode_);
    	}
    }
}

void Client::LogOut()
{
	delete netpulse_;
	main_->network_->Disconnect();
	delete this;
}
