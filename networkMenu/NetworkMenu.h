/*
 * NetworkMenu.h
 *
 *  Created on: Feb 24, 2015
 *      Author: practicing01 and https://github.com/sw17ch/udp_broadcast_example
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../Urho3DPlayer.h"

using namespace Urho3D;

class NetworkMenu : public Object
{
	OBJECT(NetworkMenu);
public:
	NetworkMenu(Context* context, Urho3DPlayer* main);
	~NetworkMenu();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void TouchDown(StringHash eventType, VariantMap& eventData);
	void HandleTextFinished(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	float elapsedTime_;

	String ipAddress_;
};
