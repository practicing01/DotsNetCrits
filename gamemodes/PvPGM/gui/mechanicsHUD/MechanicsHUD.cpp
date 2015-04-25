/*
 * MechanicsHUD.cpp
 *
 *  Created on: Apr 8, 2015
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

#include "MechanicsHUD.h"

MechanicsHUD::MechanicsHUD(Context* context, Urho3DPlayer* main, GameMode* gameMode, PlayerSB* playerSelf) :
    Object(context)
{
	elapsedTime_ = 0.0f;
	main_ = main;
	gameMode_ = gameMode;
	playerSelf_ = playerSelf;

	mechanicsHUD_ = main_->ui_->LoadLayout(main_->cache_->GetResource<XMLFile>("UI/mechanicsHUD.xml"));
    main_->ui_->GetRoot()->AddChild(mechanicsHUD_);

    for (int x = 0; x < mechanicsHUD_->GetNumChildren(); x++)
    {
    	SubscribeToEvent(mechanicsHUD_->GetChild(x), E_RELEASED, HANDLER(MechanicsHUD, HandleRelease));
    }

	//SubscribeToEvent(E_UPDATE, HANDLER(MainMenu, HandleUpdate));
}

MechanicsHUD::~MechanicsHUD()
{
	main_->ui_->GetRoot()->RemoveChild(mechanicsHUD_);
}

void MechanicsHUD::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("main menu loop");
	elapsedTime_ += timeStep;
}

void MechanicsHUD::HandleRelease(StringHash eventType, VariantMap& eventData)
{
	using namespace Released;

	UIElement* ele = static_cast<UIElement*>(eventData[Released::P_ELEMENT].GetPtr());

	String mechanicID = ele->GetVar("mechanicID").GetString();

	for (int x = 0; x < playerSelf_->gameMechanics_.Size(); x++)
	{
		if (playerSelf_->gameMechanics_[x]->mechanicID_ == mechanicID)
		{
			playerSelf_->gameMechanics_[x]->ClientRequestExecute();
			break;
		}
	}
}
