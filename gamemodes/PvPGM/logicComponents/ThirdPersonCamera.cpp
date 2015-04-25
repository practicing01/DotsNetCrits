/*
 * ThirdPersonCamera.cpp
 *
 *  Created on: Apr 11, 2015
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

#include "ThirdPersonCamera.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

ThirdPersonCamera::ThirdPersonCamera(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "ThirdPersonCamera";
}

ThirdPersonCamera::~ThirdPersonCamera()
{
}

void ThirdPersonCamera::Start()
{
	camOrigin_ = node_->GetChild("camera")->GetPosition();
	rayDistance_ = (node_->GetChild("camera")->GetWorldPosition() - node_->GetPosition()).Length();
	beeBox_ = node_->GetComponent<AnimatedModel>()->GetWorldBoundingBox();
	SubscribeToEvent(E_POSTUPDATE, HANDLER(ThirdPersonCamera, HandlePostUpdate));
}

void ThirdPersonCamera::OnSetEnabled()
{
	if (IsEnabled())
	{
		node_->GetChild("camera")->SetPosition(camOrigin_);
		SubscribeToEvent(E_POSTUPDATE, HANDLER(ThirdPersonCamera, HandlePostUpdate));
	}
	else
	{
		UnsubscribeFromEvent(E_POSTUPDATE);
	}
}

void ThirdPersonCamera::Update(float timeStep)
{
}

void ThirdPersonCamera::ServerSync()
{
}

void ThirdPersonCamera::ClientSync()
{
}

void ThirdPersonCamera::ClientRequestExecute()
{
}

void ThirdPersonCamera::ServerExecute()
{
}

void ThirdPersonCamera::ClientExecute()
{
}

void ThirdPersonCamera::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}

void ThirdPersonCamera::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	if (!scene_)
	{
		return;
	}
	else if (!scene_->GetComponent<Octree>())
	{
		return;
	}

	float timeStep = eventData[PostUpdate::P_TIMESTEP].GetFloat();

	vectoria_ = beeBox_.Size();

	cameraRay_.origin_ = node_->GetPosition();

	cameraRay_.origin_.y_ += vectoria_.y_;

	cameraRay_.direction_ = (cameraNode_->GetWorldPosition() - cameraRay_.origin_).Normalized();

	victoria_ = cameraNode_->GetPosition();
	remainingDist_ = (victoria_ - camOrigin_).Length();
	inderp_ = (remainingDist_ * timeStep);

	if (remainingDist_ > 1.0f)
	{
		cameraNode_->SetPosition(victoria_.Lerp(camOrigin_, inderp_ / remainingDist_));
		if ( (cameraNode_->GetPosition() - camOrigin_).Length() < 1.0f)
		{
			cameraNode_->SetPosition(camOrigin_);
		}
	}

	PODVector<RayQueryResult> results;

	RayOctreeQuery query(results, cameraRay_, RAY_TRIANGLE, rayDistance_,
			DRAWABLE_GEOMETRY);

	scene_->GetComponent<Octree>()->Raycast(query);

	if (results.Size())
	{
		int index = -1;
		for (int x = 0; x < results.Size(); x++)
		{
			if (results[x].drawable_->GetViewMask()!=1)//todo define viewmasks
			{
				if (index > -1)
				{
					if (results[x].distance_ < results[index].distance_)
					{
						index = x;
					}
				}
				else
				{
					index = x;
				}
			}
		}

		if (index > -1)
		{
			if (results[index].position_.y_ < cameraRay_.origin_.y_)
			{
				victoria_ = results[index].position_;
				victoria_.y_ += vectoria_.y_;
				cameraNode_->SetWorldPosition(victoria_);
			}
			else
			{
				cameraNode_->SetWorldPosition(results[index].position_);
			}
		}
	}

	cameraNode_->LookAt(cameraRay_.origin_);
}
