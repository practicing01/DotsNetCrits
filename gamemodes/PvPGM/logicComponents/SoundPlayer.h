/*
 * SoundPlayer.h
 *
 *  Created on: Apr 22, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "GameMechanic.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

EVENT(E_SOUNDREQUEST, SoundRequest)
{
   PARAM(P_NODE, Node);
   PARAM(P_SOUNDTYPE, SoundType);
}

class SoundPlayer: public LogicComponent, GameMechanic
{
	OBJECT(SoundPlayer);
public:
	SoundPlayer(Context* context);
	~SoundPlayer();
	void HandleSoundRequest(StringHash eventType, VariantMap& eventData);
	virtual void Update(float timeStep);
	virtual void Start();
	void ServerSync();
	void ClientSync();
	void ClientRequestExecute();
	void ServerExecute();
	void ClientExecute();
	void SetGameMode();

	VectorBuffer msg_;

	Vector<String> castSounds_;
	Vector<String> meleeSounds_;
	Vector<String> hurtSounds_;

	static const int SOUNDTYPE_CAST_ = 0;
	static const int SOUNDTYPE_MELEE_ = 1;
	static const int SOUNDTYPE_HURT_ = 2;
};
