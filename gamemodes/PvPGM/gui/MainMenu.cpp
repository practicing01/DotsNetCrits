/*
 * MainMenu.cpp
 *
 *  Created on: Mar 23, 2015
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

#include "MainMenu.h"
#include "logout/LogOutMenu.h"

MainMenu::MainMenu(Context* context, Urho3DPlayer* main, GameMode* gameMode) :
    Object(context)
{
	elapsedTime_ = 0.0f;
	main_ = main;
	gameMode_ = gameMode;

	previousExtents_ = IntVector2(800, 480);

	SubscribeToEvent(E_RESIZED, HANDLER(MainMenu, HandleElementResize));

	LogOutMenu* logoutmMenu = new LogOutMenu(context_, main_, gameMode_);

	menuItems_.Push((MenuItem*)logoutmMenu);

	menuItemIndex_ = 0;

	mainMenuButt_ = main_->ui_->LoadLayout(main_->cache_->GetResource<XMLFile>("UI/mainMenuButt.xml"));
    main_->ui_->GetRoot()->AddChild(mainMenuButt_);

    mainMenu_ = main_->ui_->LoadLayout(main_->cache_->GetResource<XMLFile>("UI/mainMenu.xml"));
    main_->ui_->GetRoot()->AddChild(mainMenu_);
    mainMenu_->SetEnabled(false);
    mainMenu_->SetVisible(false);

    SubscribeToEvent(mainMenuButt_, E_RELEASED, HANDLER(MainMenu, HandleMenuToggle));
    SubscribeToEvent(mainMenu_->GetChild("selectButt", true), E_RELEASED, HANDLER(MainMenu, HandleMenuItemExec));
	//SubscribeToEvent(E_UPDATE, HANDLER(MainMenu, HandleUpdate));
}

MainMenu::~MainMenu()
{
	for (int x = 0; x < menuItems_.Size(); x++)
	{
		menuItems_[x]->LogOut();
	}
	menuItems_.Clear();
	main_->ui_->GetRoot()->RemoveChild(mainMenuButt_);
	main_->ui_->GetRoot()->RemoveChild(mainMenu_);
}

void MainMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("main menu loop");
	elapsedTime_ += timeStep;
}

void MainMenu::HandleMenuToggle(StringHash eventType, VariantMap& eventData)
{
	using namespace Released;

	//UIElement* ele = static_cast<UIElement*>(eventData[Released::P_ELEMENT].GetPtr());

	mainMenu_->SetEnabled(!mainMenu_->IsEnabled());
    mainMenu_->SetVisible(!mainMenu_->IsVisible());

    if (mainMenu_->IsVisible())
    {
    	SetMenuItemImage();
    }
}

void MainMenu::SetMenuItemImage()
{
	int width = main_->cache_->GetResource<Texture2D>("Textures/" +
			menuItems_[menuItemIndex_]->GetMenuImage())->GetWidth();

	int height = main_->cache_->GetResource<Texture2D>("Textures/" +
			menuItems_[menuItemIndex_]->GetMenuImage())->GetHeight();

	( (Sprite*)( mainMenu_->GetChild("selectButt", true)->GetChild(0) ) )->SetTexture(main_->cache_->GetResource<Texture2D>("Textures/" +
			menuItems_[menuItemIndex_]->GetMenuImage()));

	( (Sprite*)( mainMenu_->GetChild("selectButt", true)->GetChild(0) ) )->SetImageRect(IntRect(0, 0, width, height));//todo scale to window scale

}

void MainMenu::HandleMenuItemExec(StringHash eventType, VariantMap& eventData)
{
	using namespace Released;

	//UIElement* ele = static_cast<UIElement*>(eventData[Released::P_ELEMENT].GetPtr());

	menuItems_[menuItemIndex_]->Execute();
}

void MainMenu::HandleElementResize(StringHash eventType, VariantMap& eventData)
{
	using namespace Resized;

	UIElement* ele = static_cast<UIElement*>(eventData[ElementAdded::P_ELEMENT].GetPtr());

	if (ele->GetName() != "" && ele->GetName() != "mechanicButt")
	{
		IntVector2 rootExtent = main_->ui_->GetRoot()->GetSize();

		IntVector2 scaledExtent;

		scaledExtent.x_ = ( ele->GetWidth() *  rootExtent.x_ ) / previousExtents_.x_;
		scaledExtent.y_ = ( ele->GetHeight() *  rootExtent.y_ ) / previousExtents_.y_;

		ele->SetSize(scaledExtent);

		IntVector2 scaledPosition = IntVector2(
				( ele->GetPosition().x_ *  rootExtent.x_ ) / previousExtents_.x_,
				( ele->GetPosition().y_ *  rootExtent.y_ ) / previousExtents_.y_);

		ele->SetPosition(scaledPosition);
	}
}
