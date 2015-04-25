/*
 * PlayerSB.h
 *
 *  Created on: Mar 19, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../../../Urho3DPlayer.h"
#include "../../../network/client/Client.h"
#include "../../GameMode.h"
#include "../logicComponents/GameMechanic.h"

class PlayerSB : public Object, GameMechanic//todo convert this class to a logic component
{
	OBJECT(PlayerSB);
public:
	PlayerSB(Context* context, Urho3DPlayer* main, GameMode* gamemode);
	~PlayerSB();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	//void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
	void LoadDefaultPlayer();
	void LoadPlayer(String modelFilename);
	void RemovePlayer();
	void RecursiveSetAnimation(Node* noed, String ani, bool loop, unsigned char layer);
	void ServerSync();
	void ClientSync();
	void SetSynchronization();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();
	void RequestMechanicExecute();
	void MechanicExecute();

	Urho3DPlayer* main_;
	float elapsedTime_;
	GameMode* gamemode_;

	SharedPtr<Scene> scene_;
    SharedPtr<Node> player_;
    String modelFilename_;

    ClientData* clientData_;
    VectorBuffer msg_;

    Vector<GameMechanic*> gameMechanics_;

    float speed_;
    float rotSpeed_;
    int health_;
    int armor_;
    float gravity_;
    float rayDistance_;
    float inderp_;
    float remainingDist_;
    Vector3 victoria_;
    Vector3 vectoria_;
    Vector3 camOrigin_;
	BoundingBox beeBox_;
	Ray cameraRay_;
};
