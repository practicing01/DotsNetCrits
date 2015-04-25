/*
 * CameraToggle.cpp
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

#include "CameraToggle.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"
#include "ThirdPersonCamera.h"
#include "TopDownCamera.h"

CameraToggle::CameraToggle(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "CameraToggle";
}

CameraToggle::~CameraToggle()
{
}

void CameraToggle::Start()
{
	cameraType_ = 0;
}

void CameraToggle::Update(float timeStep)
{
}

void CameraToggle::ServerSync()
{
}

void CameraToggle::ClientSync()
{
}

void CameraToggle::ClientRequestExecute()
{
	if (cameraType_ == 0)
	{
		cameraType_ = 1;
		node_->GetComponent<ThirdPersonCamera>()->SetEnabled(false);
		node_->GetComponent<TopDownCamera>()->SetEnabled(true);
	}
	else if (cameraType_ == 1)
	{
		cameraType_ = 0;
		node_->GetComponent<TopDownCamera>()->SetEnabled(false);
		node_->GetComponent<ThirdPersonCamera>()->SetEnabled(true);
	}
}

void CameraToggle::ServerExecute()
{
}

void CameraToggle::ClientExecute()
{
}

void CameraToggle::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}
