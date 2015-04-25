/*
 * TopDownCamera.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Resource/XMLFile.h>

#include "TopDownCamera.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

TopDownCamera::TopDownCamera(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "TopDownCamera";
	distance_ = 10.0f;
}

TopDownCamera::~TopDownCamera()
{
}

void TopDownCamera::Start()
{
	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
}

void TopDownCamera::OnSetEnabled()
{
	if (IsEnabled())
	{
		camOrigin_ = node_->GetPosition();
		camOrigin_.y_ += beeBox_.Size().y_ * distance_;
		node_->GetChild("camera")->SetWorldPosition(camOrigin_);
		node_->GetChild("camera")->SetRotation(Quaternion(90.0f, 0.0f, 0.0f));
		SubscribeToEvent(E_POSTUPDATE, HANDLER(TopDownCamera, HandlePostUpdate));
	}
	else
	{
		UnsubscribeFromEvent(E_POSTUPDATE);
	}
}

void TopDownCamera::Update(float timeStep)
{
}

void TopDownCamera::ServerSync()
{
}

void TopDownCamera::ClientSync()
{
}

void TopDownCamera::ClientRequestExecute()
{
}

void TopDownCamera::ServerExecute()
{
}

void TopDownCamera::ClientExecute()
{
}

void TopDownCamera::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}

void TopDownCamera::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	if (!node_){return;}

	float timeStep = eventData[PostUpdate::P_TIMESTEP].GetFloat();

	node_->GetChild("camera")->SetWorldRotation(Quaternion(90.0f, 0.0f, 0.0f));
}
