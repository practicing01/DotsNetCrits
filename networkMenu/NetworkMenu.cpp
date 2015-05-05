/*
 * NetworkMenu.cpp
 *
 *  Created on: Feb 24, 2015
 *      Author: practicing01 and https://github.com/sw17ch/udp_broadcast_example
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

#include <Urho3D/DebugNew.h>

#include "NetworkMenu.h"

#include "../network/server/Server.h"
#include "../network/client/Client.h"

NetworkMenu::NetworkMenu(Context* context, Urho3DPlayer* main) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;

	previousExtents_ = IntVector2(800, 480);

	SubscribeToEvent(E_RESIZED, HANDLER(NetworkMenu, HandleElementResize));

	main_->cameraNode_->RemoveAllChildren();
	main_->cameraNode_->RemoveAllComponents();
	main_->cameraNode_->Remove();

	File loadFile(context_,main_->filesystem_->GetProgramDir()
			+ "Data/Scenes/networkMenu.xml", FILE_READ);
	main_->scene_->LoadXML(loadFile);

	main_->cameraNode_ = main_->scene_->GetChild("camera");
	main_->viewport_->SetScene(main_->scene_);
	main_->viewport_->SetCamera(main_->cameraNode_->GetComponent<Camera>());

	UI* ui = GetSubsystem<UI>();

	XMLFile* style = main_->cache_->GetResource<XMLFile>("UI/DefaultStyle.xml");
	ui->GetRoot()->SetDefaultStyle(style);

    SharedPtr<UIElement> layoutRoot = ui->LoadLayout(main_->cache_->GetResource<XMLFile>("UI/ipAddress.xml"));
    ui->GetRoot()->AddChild(layoutRoot);

    ipAddress_ = "127.0.0.1";

	SubscribeToEvent(E_UPDATE, HANDLER(NetworkMenu, HandleUpdate));
	SubscribeToEvent(E_TOUCHBEGIN, HANDLER(NetworkMenu, TouchDown));
	SubscribeToEvent(E_TEXTFINISHED, HANDLER(NetworkMenu, HandleTextFinished));
}

NetworkMenu::~NetworkMenu()
{
}

void NetworkMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	//float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("loop");
	//elapsedTime_ += timeStep;

	if (main_->input_->GetKeyDown(SDLK_ESCAPE))
	{
		main_->GetSubsystem<Engine>()->Exit();
	}
}

void NetworkMenu::TouchDown(StringHash eventType, VariantMap& eventData)
{
	if (!main_->scene_){return;}
	using namespace TouchBegin;

	Ray cameraRay = main_->cameraNode_->GetComponent<Camera>()->GetScreenRay(
			(float) eventData[P_X].GetInt() / main_->graphics_->GetWidth(),
			(float) eventData[P_Y].GetInt() / main_->graphics_->GetHeight());

	PODVector<RayQueryResult> results;

	RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f,
			DRAWABLE_GEOMETRY);

	main_->scene_->GetComponent<Octree>()->Raycast(query);

	if (results.Size())
	{
		for (int x = 0; x < results.Size(); x++)
		{
			//LOGERRORF(results[x].node_->GetName().CString());
			if (results[x].node_->GetName() == "hostButt")
			{
				//do stuffs with butt
				main_->cameraNode_->RemoveAllChildren();
				main_->cameraNode_->RemoveAllComponents();
				main_->cameraNode_->Remove();
				main_->scene_->RemoveAllChildren();
				main_->scene_->RemoveAllComponents();
				main_->GetSubsystem<UI>()->GetRoot()->RemoveAllChildren();
				main_->logicStates_.Remove(this);
				main_->logicStates_.Push(new Server(context_, main_));
				main_->logicStates_.Push(new Client(context_, main_, ipAddress_));
				delete this;
				return;
			}
			else if (results[x].node_->GetName() == "joinButt")
			{
				//do stuffs with butt
				main_->cameraNode_->RemoveAllChildren();
				main_->cameraNode_->RemoveAllComponents();
				main_->cameraNode_->Remove();
				main_->scene_->RemoveAllChildren();
				main_->scene_->RemoveAllComponents();
				main_->GetSubsystem<UI>()->GetRoot()->RemoveAllChildren();
				main_->logicStates_.Remove(this);
				main_->logicStates_.Push(new Client(context_, main_, ipAddress_));
				delete this;
				return;
			}
		}
	}
}

void NetworkMenu::HandleTextFinished(StringHash eventType, VariantMap& eventData)
{
	using namespace TextFinished;

	ipAddress_ = eventData[P_TEXT].GetString();

}

void NetworkMenu::HandleElementResize(StringHash eventType, VariantMap& eventData)
{
	using namespace Resized;

	UIElement* ele = static_cast<UIElement*>(eventData[ElementAdded::P_ELEMENT].GetPtr());

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
