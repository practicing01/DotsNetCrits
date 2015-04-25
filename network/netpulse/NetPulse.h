/*
 * NetPulse.h
 *
 *  Created on: Mar 7, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>

#include "../../Urho3DPlayer.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

const int MSG_PULSE = 8008135;

class NetPulse: public Object
{
	OBJECT(NetPulse);
public:
	NetPulse(Context* context, Urho3DPlayer* main);
	~NetPulse();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleNetworkMessage(StringHash eventType, VariantMap& eventData);
    void ClearConnections();
    void RemoveConnection(Connection* conn);

    Urho3DPlayer* main_;
	float elapsedTime_;
	float pulseInterval_;

	typedef struct
	{
		Connection* connection_;
		float lastPulseTime_;
		float lagTime_;
	}PulseConnections;

	Vector<PulseConnections*> connections_;

	Network* network_;
	VectorBuffer msg_;
};
