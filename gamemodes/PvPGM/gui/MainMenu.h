/*
 * MainMenu.h
 *
 *  Created on: Mar 23, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/UI/Button.h>

#include <Urho3D/Core/Object.h>
#include "../../../Urho3DPlayer.h"
#include "../../GameMode.h"

#include "MenuItem.h"

class MainMenu : public Object
{
	OBJECT(MainMenu);
public:
	MainMenu(Context* context, Urho3DPlayer* main, GameMode* gameMode);
	~MainMenu();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandleMenuToggle(StringHash eventType, VariantMap& eventData);
	void HandleMenuItemExec(StringHash eventType, VariantMap& eventData);
	void SetMenuItemImage();

	Urho3DPlayer* main_;
	GameMode* gameMode_;
	float elapsedTime_;

	SharedPtr<UIElement> mainMenuButt_;
	SharedPtr<UIElement> mainMenu_;

	Vector<MenuItem*> menuItems_;

	int menuItemIndex_;
};
