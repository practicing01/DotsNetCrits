/*
 * ClientData.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: practicing01
 */

#include "ClientData.h"

ClientData::ClientData(int clientID, Connection* connection, bool self)
{
	clientID_ = clientID;
	connection_ = connection;
	self_ = self;
}
