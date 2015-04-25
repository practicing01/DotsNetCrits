/*
 * PlayerSB.cpp
 *
 *  Created on: Mar 19, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Resource/XMLFile.h>

#include "PlayerSB.h"
#include "../logicComponents/GameMechanic.h"
#include "../logicComponents/SceneObjectMoveToWithCollision.h"
#include "../logicComponents/SceneObjectMoveTo.h"
#include "../logicComponents/SceneObjectRotateTo.h"
#include "../logicComponents/Transform.h"
#include "../logicComponents/ThirdPersonCamera.h"
#include "../logicComponents/TopDownCamera.h"
#include "../logicComponents/CameraToggle.h"
#include "../logicComponents/Armor.h"
#include "../logicComponents/Gravity.h"
#include "../logicComponents/Health.h"
#include "../logicComponents/RotSpeed.h"
#include "../logicComponents/Speed.h"

#include "../logicComponents/Teleport.h"
#include "../logicComponents/Melee.h"
#include "../logicComponents/DPSHeal.h"
#include "../logicComponents/AOE.h"
#include "../logicComponents/AOEHeal.h"
#include "../logicComponents/Blind.h"
#include "../logicComponents/Cleanse.h"
#include "../logicComponents/Crit.h"
#include "../logicComponents/DOT.h"
#include "../logicComponents/DOTHeal.h"
#include "../logicComponents/Silence.h"
#include "../logicComponents/Shield.h"
#include "../logicComponents/Sprint.h"
#include "../logicComponents/Snare.h"
#include "../logicComponents/Cloak.h"
#include "../logicComponents/Knockback.h"
#include "../logicComponents/SoundPlayer.h"

#include "../PvPGM.h"
#include "../../../network/networkconstants/NetworkConstants.h"

PlayerSB::PlayerSB(Context* context, Urho3DPlayer* main, GameMode* gamemode) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;
	gamemode_ = gamemode;
	scene_ = gamemode->GetScene();
	mechanicID_ = "PlayerSB";

	context->RegisterFactory<SceneObjectMoveToWithCollision>();
	context->RegisterFactory<SceneObjectMoveTo>();
	context->RegisterFactory<SceneObjectRotateTo>();
	context->RegisterFactory<Transform>();
	context->RegisterFactory<ThirdPersonCamera>();
	context->RegisterFactory<TopDownCamera>();
	context->RegisterFactory<CameraToggle>();
	context->RegisterFactory<Armor>();
	context->RegisterFactory<Gravity>();
	context->RegisterFactory<Health>();
	context->RegisterFactory<RotSpeed>();
	context->RegisterFactory<Speed>();

	context->RegisterFactory<Teleport>();
	context->RegisterFactory<Melee>();
	context->RegisterFactory<DPSHeal>();
	context->RegisterFactory<AOE>();
	context->RegisterFactory<AOEHeal>();
	context->RegisterFactory<Blind>();
	context->RegisterFactory<Cleanse>();
	context->RegisterFactory<Crit>();
	context->RegisterFactory<DOT>();
	context->RegisterFactory<DOTHeal>();
	context->RegisterFactory<Silence>();
	context->RegisterFactory<Shield>();
	context->RegisterFactory<Sprint>();
	context->RegisterFactory<Snare>();
	context->RegisterFactory<Cloak>();
	context->RegisterFactory<Knockback>();

	context->RegisterFactory<SoundPlayer>();
}

PlayerSB::~PlayerSB()
{
	RemovePlayer();
}

void PlayerSB::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	//float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("client loop");
	//elapsedTime_ += timeStep;
}

void PlayerSB::RecursiveSetAnimation(Node* noed, String ani, bool loop, unsigned char layer)//todo check if animation exists, if not set default.
{
	if (noed->HasComponent<AnimationController>())
	{
		noed->GetComponent<AnimationController>()->PlayExclusive("Models/Witch3/" + ani + ".ani", layer, loop, 0.25f);//reduce archive size by reusing animations
		//noed->GetComponent<AnimationController>()->PlayExclusive("Models/" + noed->GetName() + "/" + ani + ".ani", layer, loop, 0.25f);//proper way
		if (!loop)
		{
			noed->GetComponent<AnimationController>()->SetAutoFade(ani, 0.25f);
		}
	}

	for (int x = 0; x < noed->GetNumChildren(); x++)
	{
		RecursiveSetAnimation(noed->GetChild(x), ani, loop, layer);
	}
}

void PlayerSB::LoadDefaultPlayer()
{
	LoadPlayer("Witch3");//todo endless loop possibilities :d
}

void PlayerSB::LoadPlayer(String modelFilename)
{
	if (!main_->cache_->Exists("Models/" + modelFilename + "/" + modelFilename + ".xml"))
	{
		LoadDefaultPlayer();
		return;
	}
	RemovePlayer();
    XMLFile* xmlFile = main_->cache_->GetResource<XMLFile>("Models/" + modelFilename + "/" + modelFilename + ".xml");
	player_ = scene_->InstantiateXML(xmlFile->GetRoot(), Vector3::ZERO, Quaternion(), LOCAL);

	RecursiveSetAnimation(player_, "idle1", true, 0);
	modelFilename_ = modelFilename;

	gameMechanics_.Push( (GameMechanic*)this );
	clientIDSelf_ = clientData_->clientID_;

	Transform* _Transform = new Transform(context_);
	player_->AddComponent(_Transform, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Transform );
	((GameMechanic*)_Transform)->gameMode_ = gamemode_;
	((GameMechanic*)_Transform)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Transform)->SetGameMode();

	Armor* _Armor = new Armor(context_);
	player_->AddComponent(_Armor, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Armor );
	((GameMechanic*)_Armor)->gameMode_ = gamemode_;
	((GameMechanic*)_Armor)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Armor)->SetGameMode();

	Gravity* _Gravity = new Gravity(context_);
	player_->AddComponent(_Gravity, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Gravity );
	((GameMechanic*)_Gravity)->gameMode_ = gamemode_;
	((GameMechanic*)_Gravity)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Gravity)->SetGameMode();

	Health* _Health = new Health(context_);
	player_->AddComponent(_Health, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Health );
	((GameMechanic*)_Health)->gameMode_ = gamemode_;
	((GameMechanic*)_Health)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Health)->SetGameMode();

	RotSpeed* _RotSpeed = new RotSpeed(context_);
	player_->AddComponent(_RotSpeed, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_RotSpeed );
	((GameMechanic*)_RotSpeed)->gameMode_ = gamemode_;
	((GameMechanic*)_RotSpeed)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_RotSpeed)->SetGameMode();

	Speed* _Speed = new Speed(context_);
	player_->AddComponent(_Speed, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Speed );
	((GameMechanic*)_Speed)->gameMode_ = gamemode_;
	((GameMechanic*)_Speed)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Speed)->SetGameMode();

	ThirdPersonCamera* _ThirdPersonCamera = new ThirdPersonCamera(context_);
	player_->AddComponent(_ThirdPersonCamera, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_ThirdPersonCamera );
	((GameMechanic*)_ThirdPersonCamera)->gameMode_ = gamemode_;
	((GameMechanic*)_ThirdPersonCamera)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_ThirdPersonCamera)->SetGameMode();

	TopDownCamera* _TopDownCamera = new TopDownCamera(context_);
	_TopDownCamera->SetEnabled(false);
	player_->AddComponent(_TopDownCamera, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_TopDownCamera );
	((GameMechanic*)_TopDownCamera)->gameMode_ = gamemode_;
	((GameMechanic*)_TopDownCamera)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_TopDownCamera)->SetGameMode();

	CameraToggle* _CameraToggle = new CameraToggle(context_);
	player_->AddComponent(_CameraToggle, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_CameraToggle );
	((GameMechanic*)_CameraToggle)->gameMode_ = gamemode_;
	((GameMechanic*)_CameraToggle)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_CameraToggle)->SetGameMode();

	SceneObjectMoveToWithCollision* _SceneObjectMoveToWithCollision = new SceneObjectMoveToWithCollision(context_);//todo check if RemoveComponents() deletes this pointer.
	player_->AddComponent(_SceneObjectMoveToWithCollision, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_SceneObjectMoveToWithCollision );
	((GameMechanic*)_SceneObjectMoveToWithCollision)->gameMode_ = gamemode_;
	((GameMechanic*)_SceneObjectMoveToWithCollision)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_SceneObjectMoveToWithCollision)->SetGameMode();
	_SceneObjectMoveToWithCollision->gravity_ = gravity_;

	SceneObjectMoveTo* _SceneObjectMoveTo = new SceneObjectMoveTo(context_);
	player_->AddComponent(_SceneObjectMoveTo, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_SceneObjectMoveTo );
	((GameMechanic*)_SceneObjectMoveTo)->gameMode_ = gamemode_;
	((GameMechanic*)_SceneObjectMoveTo)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_SceneObjectMoveTo)->SetGameMode();

	SceneObjectRotateTo* _SceneObjectRotateTo = new SceneObjectRotateTo(context_);
	player_->AddComponent(_SceneObjectRotateTo, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_SceneObjectRotateTo );
	((GameMechanic*)_SceneObjectRotateTo)->gameMode_ = gamemode_;
	((GameMechanic*)_SceneObjectRotateTo)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_SceneObjectRotateTo)->SetGameMode();

	Teleport* _Teleport = new Teleport(context_);
	player_->AddComponent(_Teleport, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Teleport );
	((GameMechanic*)_Teleport)->gameMode_ = gamemode_;
	((GameMechanic*)_Teleport)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Teleport)->SetGameMode();

	Melee* _Melee = new Melee(context_);
	player_->AddComponent(_Melee, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Melee );
	((GameMechanic*)_Melee)->gameMode_ = gamemode_;
	((GameMechanic*)_Melee)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Melee)->SetGameMode();

	DPSHeal* _DPSHeal = new DPSHeal(context_);
	player_->AddComponent(_DPSHeal, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_DPSHeal );
	((GameMechanic*)_DPSHeal)->gameMode_ = gamemode_;
	((GameMechanic*)_DPSHeal)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_DPSHeal)->SetGameMode();

	AOE* _AOE = new AOE(context_);
	player_->AddComponent(_AOE, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_AOE );
	((GameMechanic*)_AOE)->gameMode_ = gamemode_;
	((GameMechanic*)_AOE)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_AOE)->SetGameMode();

	AOEHeal* _AOEHeal = new AOEHeal(context_);
	player_->AddComponent(_AOEHeal, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_AOEHeal );
	((GameMechanic*)_AOEHeal)->gameMode_ = gamemode_;
	((GameMechanic*)_AOEHeal)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_AOEHeal)->SetGameMode();

	Blind* _Blind = new Blind(context_);
	player_->AddComponent(_Blind, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Blind );
	((GameMechanic*)_Blind)->gameMode_ = gamemode_;
	((GameMechanic*)_Blind)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Blind)->SetGameMode();

	Cleanse* _Cleanse = new Cleanse(context_);
	player_->AddComponent(_Cleanse, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Cleanse );
	((GameMechanic*)_Cleanse)->gameMode_ = gamemode_;
	((GameMechanic*)_Cleanse)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Cleanse)->SetGameMode();

	Crit* _Crit = new Crit(context_);
	player_->AddComponent(_Crit, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Crit );
	((GameMechanic*)_Crit)->gameMode_ = gamemode_;
	((GameMechanic*)_Crit)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Crit)->SetGameMode();

	DOT* _DOT = new DOT(context_);
	player_->AddComponent(_DOT, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_DOT );
	((GameMechanic*)_DOT)->gameMode_ = gamemode_;
	((GameMechanic*)_DOT)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_DOT)->SetGameMode();

	DOTHeal* _DOTHeal = new DOTHeal(context_);
	player_->AddComponent(_DOTHeal, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_DOTHeal );
	((GameMechanic*)_DOTHeal)->gameMode_ = gamemode_;
	((GameMechanic*)_DOTHeal)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_DOTHeal)->SetGameMode();

	Silence* _Silence = new Silence(context_);
	player_->AddComponent(_Silence, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Silence );
	((GameMechanic*)_Silence)->gameMode_ = gamemode_;
	((GameMechanic*)_Silence)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Silence)->SetGameMode();

	Shield* _Shield = new Shield(context_);
	player_->AddComponent(_Shield, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Shield );
	((GameMechanic*)_Shield)->gameMode_ = gamemode_;
	((GameMechanic*)_Shield)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Shield)->SetGameMode();

	Sprint* _Sprint = new Sprint(context_);
	player_->AddComponent(_Sprint, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Sprint );
	((GameMechanic*)_Sprint)->gameMode_ = gamemode_;
	((GameMechanic*)_Sprint)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Sprint)->SetGameMode();

	Snare* _Snare = new Snare(context_);
	player_->AddComponent(_Snare, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Snare );
	((GameMechanic*)_Snare)->gameMode_ = gamemode_;
	((GameMechanic*)_Snare)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Snare)->SetGameMode();

	Cloak* _Cloak = new Cloak(context_);
	player_->AddComponent(_Cloak, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Cloak );
	((GameMechanic*)_Cloak)->gameMode_ = gamemode_;
	((GameMechanic*)_Cloak)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Cloak)->SetGameMode();

	Knockback* _Knockback = new Knockback(context_);
	player_->AddComponent(_Knockback, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_Knockback );
	((GameMechanic*)_Knockback)->gameMode_ = gamemode_;
	((GameMechanic*)_Knockback)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_Knockback)->SetGameMode();

	SoundPlayer* _SoundPlayer = new SoundPlayer(context_);
	player_->AddComponent(_SoundPlayer, 0, LOCAL);
	gameMechanics_.Push( (GameMechanic*)_SoundPlayer );
	((GameMechanic*)_SoundPlayer)->gameMode_ = gamemode_;
	((GameMechanic*)_SoundPlayer)->clientIDSelf_ = clientData_->clientID_;
	((GameMechanic*)_SoundPlayer)->SetGameMode();
}

void PlayerSB::RemovePlayer()
{
	if (player_)
	{
		player_->RemoveAllChildren();
		player_->RemoveAllComponents();
		player_->Remove();
	}
}

void PlayerSB::ServerSync()
{
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_SynchronizeComponent_);
	msg_.WriteInt(clientData_->clientID_);
	msg_.WriteString(mechanicID_);
	msg_.WriteInt(health_);
	msg_.WriteInt(armor_);
	msg_.WriteFloat(gravity_);
	msg_.WriteFloat(speed_);

	receiver_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);

	for (int x = 0; x < gameMechanics_.Size(); x++)
	{
		if (gameMechanics_[x] != this)
		{
			gameMechanics_[x]->receiver_ = receiver_;
			gameMechanics_[x]->clientID_ = clientID_;
			gameMechanics_[x]->ServerSync();
		}
	}
}

void PlayerSB::ClientSync()
{
	health_ = message_->ReadInt();
	armor_ = message_->ReadInt();
	gravity_ = message_->ReadFloat();
	speed_ = message_->ReadFloat();
}

void PlayerSB::SetSynchronization()
{
	String mechanicID = message_->ReadString();

	for (int x = 0; x < gameMechanics_.Size(); x++)
	{
		if (gameMechanics_[x]->mechanicID_ == mechanicID)
		{
			gameMechanics_[x]->message_ = message_;
			gameMechanics_[x]->clientID_ = clientID_;
			gameMechanics_[x]->lagTime_ = lagTime_;
			gameMechanics_[x]->ClientSync();
			break;
		}
	}
}

void PlayerSB::RequestMechanicExecute()
{
	String mechanicID = message_->ReadString();

	for (int x = 0; x < gameMechanics_.Size(); x++)
	{
		if (gameMechanics_[x]->mechanicID_ == mechanicID)
		{
			gameMechanics_[x]->message_ = message_;
			gameMechanics_[x]->clientID_ = clientID_;
			gameMechanics_[x]->lagTime_ = lagTime_;
			gameMechanics_[x]->ServerExecute();
			break;
		}
	}
}

void PlayerSB::MechanicExecute()
{
	String mechanicID = message_->ReadString();

	for (int x = 0; x < gameMechanics_.Size(); x++)
	{
		if (gameMechanics_[x]->mechanicID_ == mechanicID)
		{
			gameMechanics_[x]->message_ = message_;
			gameMechanics_[x]->clientID_ = clientID_;
			gameMechanics_[x]->lagTime_ = lagTime_;
			gameMechanics_[x]->ClientExecute();
			break;
		}
	}
}

void PlayerSB::ClientRequestExecute()
{
}

void PlayerSB::ServerExecute()
{
}

void PlayerSB::ClientExecute()
{
}

void PlayerSB::SetGameMode()
{
}
