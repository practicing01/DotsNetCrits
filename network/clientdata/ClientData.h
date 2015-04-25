/*
 * ClientData.h
 *
 *  Created on: Mar 18, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Core/Object.h>

using namespace Urho3D;

class ClientData
{
public:
	ClientData(int clientID, Connection* connection, bool self);
	int clientID_;
	Connection* connection_;
	bool self_;
};
