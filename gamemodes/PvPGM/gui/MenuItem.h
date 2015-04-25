/*
 * MenuItem.h
 *
 *  Created on: Mar 23, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include "../../../Urho3DPlayer.h"

class MenuItem
{
public:
	virtual ~MenuItem();
	virtual String GetMenuImage() = 0;
	virtual void Execute() = 0;
	virtual void LogOut() = 0;
};
