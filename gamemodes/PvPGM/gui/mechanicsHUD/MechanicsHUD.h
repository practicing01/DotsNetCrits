/*
 * MechanicsHUD.h
 *
 *  Created on: Apr 8, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/UI/Button.h>

#include <Urho3D/Core/Object.h>
#include "../../../../Urho3DPlayer.h"
#include "../../../GameMode.h"
#include "../../player/PlayerSB.h"

class MechanicsHUD : public Object
{
	OBJECT(MechanicsHUD);
public:
	MechanicsHUD(Context* context, Urho3DPlayer* main, GameMode* gameMode, PlayerSB* playerSelf);
	~MechanicsHUD();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandleRelease(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	GameMode* gameMode_;
	float elapsedTime_;

	SharedPtr<UIElement> mechanicsHUD_;

	PlayerSB* playerSelf_;
};
