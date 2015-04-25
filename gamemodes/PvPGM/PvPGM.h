/*
 * PvPGM.h
 *
 *  Created on: Mar 18, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../../Urho3DPlayer.h"
#include "../../network/server/Server.h"
#include "../../network/client/Client.h"
#include "../GameMode.h"
#include "player/PlayerSB.h"
#include "gui/MainMenu.h"
#include "gui/mechanicsHUD/MechanicsHUD.h"

class PvPGM : public Object, GameMode
{
	OBJECT(PvPGM);
public:
	PvPGM(Context* context, Urho3DPlayer* main, bool hosting, Server* server, Client* client);
	~PvPGM();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void ServerHandleNetworkMessage(StringHash eventType, VariantMap& eventData);
    void ClientHandleNetworkMessage(StringHash eventType, VariantMap& eventData);
    void ServerHandleMoveToComplete(StringHash eventType, VariantMap& eventData);
    void ServerHandleHealthStatus(StringHash eventType, VariantMap& eventData);
	void TouchDown(StringHash eventType, VariantMap& eventData);
	void TouchUp(StringHash eventType, VariantMap& eventData);
	void TouchDrag(StringHash eventType, VariantMap& eventData);
	void ServerSynchronizeScene(Connection* conn);
	PlayerSB* NewPlayer(ClientData* clientData, String modelFilename, Vector3 position, Quaternion rotation);
	void ClientDisco(int clientID);
	void PlayerPosition(int clientID, Vector3 destination);
	void LogOut();
	Urho3DPlayer* GetMain();
	Scene* GetScene();
	Node* GetCamera();
	bool GetIsHost();
	int GetSenderClientID(Connection* sender);
	int GetNodeClientID(Node* noed);
	Node* GetClientIDNode(int clientID);
	PlayerSB* GetPlayerSelf();
	float ServerGetSenderLagTime(Connection* sender);
	float ClientGetSenderLagTime(Connection* sender);
	void RequestAnimation(int clientID, String ani, bool loop, unsigned char layer);
	void RequestMechanicExecution(Node* receiver, VectorBuffer& mechanicParams);

	Urho3DPlayer* main_;
	float elapsedTime_;

	bool hosting_;

	Server* server_;
	Client* client_;

	SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
	Vector3 vectoria_;
	Vector3 victoria_;
	Quaternion quarterOnion_;
	Quaternion quarterPounder_;

	VectorBuffer msg_;
	Network* network_;

	String sceneFilename_;

	Vector<PlayerSB*> players_;

	Vector< SharedPtr<Node> > spawnPoints_;

	MainMenu* mainMenu_;
	MechanicsHUD* mechanicsHUD_;

	static const int MSG_LOADSCENE_ = 0;
	static const int MSG_SCENELOADED_ = 1;
	static const int MSG_NEWPLAYER_ = 2;
	static const int MSG_PLAYERDISCO_ = 3;
	static const int MSG_TOUCHDOWN_ = 4;
	static const int MSG_TOUCHUP_ = 5;
	static const int MSG_TOUCHDRAG_ = 6;
	//static const int MSG_MOVE_ = 7;
	static const int MSG_Position_ = 8;//todo change to use Transform logic component
	static const int MSG_SynchronizeComponent_ = 9;
	static const int MSG_RequestMechanicExecute_ = 10;
	static const int MSG_MechanicExecute_ = 11;

};
