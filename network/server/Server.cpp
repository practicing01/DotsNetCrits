/*
 * Server.cpp
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

#include "Server.h"
#include "../networkconstants/NetworkConstants.h"
#include "../../gamemodes/PvPGM/PvPGM.h"

Server::Server(Context* context, Urho3DPlayer* main) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;

	netpulse_ = new NetPulse(context_, main_);
	main_->logicStates_.Push(netpulse_);

	main_->network_->StartServer(9001);

	network_ = GetSubsystem<Network>();
	clientIDCount_ = 0;

	gamemodeName_ = "PvPGM";
	gamemode_ = (GameMode*)(new PvPGM(context_, main_, true, this, NULL));
	main_->logicStates_.Push((Object*)gamemode_);

	SubscribeToEvent(E_CLIENTDISCONNECTED, HANDLER(Server, HandleClientDisconnect));
	SubscribeToEvent(E_CLIENTCONNECTED, HANDLER(Server, HandleClientConnect));
	SubscribeToEvent(E_NETWORKMESSAGE, HANDLER(Server, HandleNetworkMessage));
}

Server::~Server()
{
	delete netpulse_;
	main_->logicStates_.Remove(this);
}

void Server::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("server loop");
	elapsedTime_ += timeStep;
}

void Server::HandleClientConnect(StringHash eventType, VariantMap& eventData)
{
	//LOGERRORF("client connected to server");

	using namespace NetworkMessage;

	Connection* sender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	ClientData* newClient = new ClientData(clientIDCount_, sender, false);

	//Synchronize client data.
	for (int x = 0; x < clientData_.Size(); x++)
	{
		//Send the new client an old clients data
		msg_.Clear();
		msg_.WriteInt(clientData_[x]->clientID_);
		msg_.WriteBool(false);
		sender->SendMessage(MSG_NEWCLIENT, true, true, msg_);

		//Send the old client the new clients data.
		msg_.Clear();
		msg_.WriteInt(clientIDCount_);
		msg_.WriteBool(false);
		clientData_[x]->connection_->SendMessage(MSG_NEWCLIENT, true, true, msg_);

	}

	//Send the new client its data.
	clientData_.Push(newClient);
	msg_.Clear();
	msg_.WriteInt(clientIDCount_);
	msg_.WriteBool(true);
	sender->SendMessage(MSG_NEWCLIENT, true, true, msg_);

	//Send the new client the gamemode to load.
	msg_.Clear();
	msg_.WriteString(gamemodeName_.CString());
	sender->SendMessage(MSG_LOADGAMEMODE, true, true, msg_);

	clientIDCount_++;
}

void Server::HandleClientDisconnect(StringHash eventType, VariantMap& eventData)
{
	//LOGERRORF("client diconnected from server");

	using namespace NetworkMessage;

	Connection* sender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	netpulse_->RemoveConnection(sender);

	msg_.Clear();

	for (int x = 0; x < clientData_.Size(); x++)
	{
		if (clientData_[x]->connection_ == sender)
		{
			msg_.WriteInt(clientData_[x]->clientID_);
			if (gamemodeName_ == "PvPGM")//todo figure out a way to upcast or not have to cast at all so this class doesn't have to hardcode gamemodes
			{
				((PvPGM*)gamemode_)->ClientDisco(clientData_[x]->clientID_);
			}
			ClientData* clientData = clientData_[x];
			clientData_.Remove(clientData_[x]);
			delete clientData;
			break;
		}
	}

	network_->BroadcastMessage(MSG_CLIENTDISCO, true, true, msg_);

	if (sender->GetAddress() == "127.0.0.1")
	{
		gamemode_->LogOut();
		main_->network_->StopServer();
		delete this;
	}
}

void Server::HandleNetworkMessage(StringHash eventType, VariantMap& eventData)
{
	//LOGERRORF("network message");
}
