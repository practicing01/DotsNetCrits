/*
 * Splash.h
 *
 *  Created on: Dec 24, 2014
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../Urho3DPlayer.h"

using namespace Urho3D;

class Splash : public Object
{
	OBJECT(Splash);
public:
	Splash(Context* context, Urho3DPlayer* main);
	~Splash();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void SplashScreen();

	Urho3DPlayer* main_;
	float elapsedTime_;

};
