/*
 * LogOutMenu.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Resource/XMLFile.h>

#include "LogOutMenu.h"

LogOutMenu::LogOutMenu(Context* context, Urho3DPlayer* main, GameMode* gameMode) :
    Object(context)
{
	elapsedTime_ = 0.0f;
	main_ = main;
	gameMode_ = gameMode;

	image_ = "logout.png";

	//SubscribeToEvent(E_UPDATE, HANDLER(LogOutMenu, HandleUpdate));
}

LogOutMenu::~LogOutMenu()
{
}

void LogOutMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	elapsedTime_ += timeStep;
}

String LogOutMenu::GetMenuImage()
{
	return image_;
}

void LogOutMenu::Execute()
{
	gameMode_->LogOut();
}

void LogOutMenu::LogOut()
{
	delete this;
}
