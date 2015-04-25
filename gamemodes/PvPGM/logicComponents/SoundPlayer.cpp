/*
 * SoundPlayer.cpp
 *
 *  Created on: Apr 22, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
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
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "SoundPlayer.h"
#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

SoundPlayer::SoundPlayer(Context* context) :
		LogicComponent(context)
{
	mechanicID_ = "SoundPlayer";

	castSounds_.Push("Sounds/witch3/cast/goblin-1.ogg");
	castSounds_.Push("Sounds/witch3/cast/goblin-5.ogg");
	castSounds_.Push("Sounds/witch3/cast/goblin-7.ogg");
	castSounds_.Push("Sounds/witch3/cast/goblin-10.ogg");
	castSounds_.Push("Sounds/witch3/cast/goblin-13.ogg");

	meleeSounds_.Push("Sounds/witch3/melee/goblin-2.ogg");
	meleeSounds_.Push("Sounds/witch3/melee/goblin-6.ogg");
	meleeSounds_.Push("Sounds/witch3/melee/goblin-8.ogg");
	meleeSounds_.Push("Sounds/witch3/melee/goblin-11.ogg");
	meleeSounds_.Push("Sounds/witch3/melee/goblin-14.ogg");

	hurtSounds_.Push("Sounds/witch3/hurt/goblin-3.ogg");
	hurtSounds_.Push("Sounds/witch3/hurt/goblin-4.ogg");
	hurtSounds_.Push("Sounds/witch3/hurt/goblin-9.ogg");
	hurtSounds_.Push("Sounds/witch3/hurt/goblin-12.ogg");
	hurtSounds_.Push("Sounds/witch3/hurt/goblin-15.ogg");
}

SoundPlayer::~SoundPlayer()
{
}

void SoundPlayer::Start()
{
	SubscribeToEvent(E_SOUNDREQUEST, HANDLER(SoundPlayer, HandleSoundRequest));
}

void SoundPlayer::Update(float timeStep)
{
}

void SoundPlayer::ServerSync()
{
}

void SoundPlayer::ClientSync()
{
}

void SoundPlayer::ClientRequestExecute()
{
}

void SoundPlayer::ServerExecute()
{
}

void SoundPlayer::ClientExecute()
{
}

void SoundPlayer::SetGameMode()
{
	main_ = gameMode_->GetMain();
	scene_ = gameMode_->GetScene();
	cameraNode_ = node_->GetChild("camera");
}

void SoundPlayer::HandleSoundRequest(StringHash eventType, VariantMap& eventData)
{
	SharedPtr<Node> node = SharedPtr<Node>(static_cast<Node*>(eventData[SoundRequest::P_NODE].GetPtr()));
	int soundType = static_cast<int>( eventData[SoundRequest::P_SOUNDTYPE].GetInt() );

	if (soundType == SOUNDTYPE_CAST_)
	{
		node->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>( castSounds_[ Random( 0,castSounds_.Size() ) ] ) , 0, 0.05f);
	}
	else if (soundType == SOUNDTYPE_MELEE_)
	{
		node->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>( meleeSounds_[ Random( 0,meleeSounds_.Size() ) ] )  , 0, 0.05f);
	}
	else if (soundType == SOUNDTYPE_HURT_)
	{
		node->GetComponent<SoundSource3D>()->Play(main_->cache_->GetResource<Sound>( hurtSounds_[ Random( 0,hurtSounds_.Size() ) ] )  , 0, 0.05f);
	}
}
