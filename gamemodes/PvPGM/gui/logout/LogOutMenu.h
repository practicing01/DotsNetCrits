/*
 * LogOutMenu.h
 *
 *  Created on: Mar 24, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include "../../../../Urho3DPlayer.h"
#include "../../../GameMode.h"

#include "../MenuItem.h"

class LogOutMenu : public Object, MenuItem
{
	OBJECT(LogOutMenu);
public:
	LogOutMenu(Context* context, Urho3DPlayer* main, GameMode* gameMode);
	~LogOutMenu();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	String GetMenuImage();
	void Execute();
	void LogOut();

	Urho3DPlayer* main_;
	GameMode* gameMode_;
	float elapsedTime_;

	String image_;

};
