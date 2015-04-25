/*
 * PvPGM.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include "PvPGM.h"
#include "../../network/networkconstants/NetworkConstants.h"
#include "logicComponents/GameMechanic.h"
#include "logicComponents/SceneObjectMoveToWithCollision.h"
#include "logicComponents/SceneObjectMoveTo.h"
#include "logicComponents/SceneObjectRotateTo.h"
#include "logicComponents/Transform.h"
#include "logicComponents/ThirdPersonCamera.h"
#include "logicComponents/TopDownCamera.h"
#include "logicComponents/Armor.h"
#include "logicComponents/Gravity.h"
#include "logicComponents/Health.h"
#include "logicComponents/RotSpeed.h"
#include "logicComponents/Speed.h"

#include "logicComponents/Teleport.h"
#include "logicComponents/Melee.h"
#include "logicComponents/DPSHeal.h"
#include "logicComponents/AOE.h"
#include "logicComponents/AOEHeal.h"
#include "logicComponents/Blind.h"
#include "logicComponents/Cleanse.h"
#include "logicComponents/Crit.h"
#include "logicComponents/DOT.h"
#include "logicComponents/DOTHeal.h"
#include "logicComponents/Silence.h"
#include "logicComponents/Shield.h"
#include "logicComponents/Sprint.h"
#include "logicComponents/Snare.h"
#include "logicComponents/Cloak.h"
#include "logicComponents/Knockback.h"
#include "logicComponents/SoundPlayer.h"

PvPGM::PvPGM(Context* context, Urho3DPlayer* main, bool hosting, Server* server, Client* client) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;

	hosting_ = hosting;
	server_ = server;
	client_ = client;
	mechanicsHUD_ = NULL;
	mainMenu_ = NULL;

	network_ = main_->network_;

	scene_ = new Scene(context_);
	cameraNode_ = new Node(context_);

	touchSubscriberCount_ = 0;

	if (hosting_)
	{
		File loadFile(context_,main_->filesystem_->GetProgramDir()
				+ "Data/Scenes/TehDungeon.xml", FILE_READ);
		scene_->LoadXML(loadFile);

		sceneFilename_ = scene_->GetFileName().Substring( scene_->GetFileName().FindLast("/") + 1 );

		Vector< SharedPtr<Node> > children = scene_->GetChildren();

		for (int x = 0; x < children.Size(); x++)
		{
			if (children[x]->GetName() == "spawn")
			{
				spawnPoints_.Push(children[x]);
			}
			else if (children[x]->GetName() == "bgm")
			{
				children[x]->GetComponent<SoundSource>()->Stop();
			}
		}

		cameraNode_ = scene_->GetChild("camera");

		SubscribeToEvent(E_NETWORKMESSAGE, HANDLER(PvPGM, ServerHandleNetworkMessage));
		SubscribeToEvent(E_SCENEOBJECTMOVETOCOMPLETE, HANDLER(PvPGM, ServerHandleMoveToComplete));
		SubscribeToEvent(E_HEALTHSTATUS, HANDLER(PvPGM, ServerHandleHealthStatus));
	}
	else
	{
		if (GetPlatform() == "Android")
		{
			//main_->renderer_->SetReuseShadowMaps(false);
			//main_->renderer_->SetShadowQuality(SHADOWQUALITY_LOW_16BIT);
			//main_->renderer_->SetMobileShadowBiasMul(2.0f);
			main_->renderer_->SetMobileShadowBiasAdd(0.001);
		}

		main_->viewport_->SetScene(scene_);
		main_->viewport_->SetCamera(cameraNode_->GetComponent<Camera>());
		//SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(PvPGM, HandlePostRenderUpdate));

		mainMenu_ = new MainMenu(context_, main_, this);

		SubscribeToEvent(E_NETWORKMESSAGE, HANDLER(PvPGM, ClientHandleNetworkMessage));
		//SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(PvPGM, HandlePostRenderUpdate));
		SubscribeToEvent(E_UPDATE, HANDLER(PvPGM, HandleUpdate));

		msg_.Clear();
		main_->network_->GetServerConnection()->SendMessage(MSG_GAMEMODELOADED, true, true, msg_);
	}

}

PvPGM::~PvPGM()
{
	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->clientData_->self_)
		{
			PlayerSB* player = players_[x];
			players_.Remove(players_[x]);
			delete player;
			break;
		}
	}

	if (mainMenu_)
	{
		delete mainMenu_;
	}

	if (mechanicsHUD_)
	{
		delete mechanicsHUD_;
	}

	if (cameraNode_)
	{
		cameraNode_->RemoveAllChildren();
		cameraNode_->RemoveAllComponents();
		cameraNode_->Remove();
	}

	if (scene_)
	{
		scene_->RemoveAllChildren();
		scene_->RemoveAllComponents();
	}

	main_->logicStates_.Remove(this);

	if (!hosting_)
	{
		client_->LogOut();
	}
}

void PvPGM::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("client loop");
	elapsedTime_ += timeStep;

	if (main_->input_->GetKeyDown(SDLK_ESCAPE))
	{
		main_->GetSubsystem<Engine>()->Exit();
	}
}

void PvPGM::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	GetSubsystem<Renderer>()->DrawDebugGeometry(true);
	if (scene_)
	{
		if (scene_->GetComponent<PhysicsWorld>())
		{
			scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
		}
	}
}

void PvPGM::ServerSynchronizeScene(Connection* conn)
{
	//todo Send scene nodes recursively.
	//For now though, just tell the client to load the static scene.
	msg_.Clear();
	msg_.WriteInt(MSG_LOADSCENE_);
	msg_.WriteString(sceneFilename_.CString());
	conn->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

PlayerSB* PvPGM::NewPlayer(ClientData* clientData, String modelFilename, Vector3 position, Quaternion rotation)
{
	PlayerSB* player = new PlayerSB(context_, main_, this);
	players_.Push(player);
	player->clientData_ = clientData;
	player->speed_ = 10.0f;
	player->health_ = 100;
	player->armor_ = 0;
	player->rotSpeed_ = 4.0f;
	player->gravity_ = -9.0f;
	player->LoadPlayer(modelFilename);
	player->player_->SetPosition(position);
	player->player_->SetRotation(rotation);

	if (clientData->self_)
	{
		mechanicsHUD_ = new MechanicsHUD(context_, main_, this, player);

		cameraNode_ = player->player_->GetChild("camera");
		if (!hosting_)
		{
			main_->viewport_->SetCamera(cameraNode_->GetComponent<Camera>());

			main_->audio_->SetListener(player->player_->GetChild("soundListener")->GetComponent<SoundListener>());
		}
		SubscribeToEvent(E_TOUCHBEGIN, HANDLER(PvPGM, TouchDown));
		SubscribeToEvent(E_TOUCHMOVE, HANDLER(PvPGM, TouchDrag));
		SubscribeToEvent(E_TOUCHEND, HANDLER(PvPGM, TouchUp));
	}

	return player;
}

void PvPGM::ServerHandleNetworkMessage(StringHash eventType, VariantMap& eventData)
{
	using namespace NetworkMessage;

	int msgID = eventData[P_MESSAGEID].GetInt();

	Connection* sender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	if (msgID == MSG_GAMEMODELOADED)
	{
		ServerSynchronizeScene(sender);
	}
	else if (msgID == MSG_GAMEMODEMSG)
	{
		const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
		MemoryBuffer msg(data);
		int messageID = msg.ReadInt();
		if (messageID == MSG_SCENELOADED_)
		{
			PlayerSB* player;

			//Create player on server.
			for (int x = 0; x < server_->clientData_.Size(); x++)
			{
				if (server_->clientData_[x]->connection_ == sender)
				{
					int index = Random( 0, spawnPoints_.Size() );
					victoria_ = spawnPoints_[index]->GetPosition();
					quarterOnion_ = spawnPoints_[index]->GetRotation();
					player = NewPlayer(server_->clientData_[x], "Witch3", victoria_, quarterOnion_);//Witch3 is the default model.
					break;
				}
			}

			for (int x = 0; x < players_.Size(); x++)
			{
				//Send client the existing players.
				msg_.Clear();
				msg_.WriteInt(MSG_NEWPLAYER_);
				msg_.WriteInt(players_[x]->clientData_->clientID_);
				msg_.WriteString(players_[x]->modelFilename_);
				msg_.WriteVector3(players_[x]->player_->GetPosition());
				msg_.WriteQuaternion(players_[x]->player_->GetRotation());

				sender->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);

				if (sender != players_[x]->clientData_->connection_)
				{
					((GameMechanic*)(players_[x]))->receiver_ = sender;
					((GameMechanic*)(players_[x]))->clientID_ = players_[x]->clientData_->clientID_;
					players_[x]->ServerSync();

					//Send the existing players the client.
					msg_.Clear();
					msg_.WriteInt(MSG_NEWPLAYER_);
					msg_.WriteInt(player->clientData_->clientID_);
					msg_.WriteString(player->modelFilename_);
					msg_.WriteVector3(player->player_->GetPosition());
					msg_.WriteQuaternion(player->player_->GetRotation());

					players_[x]->clientData_->connection_->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
				}
			}
		}
		else if (messageID == MSG_RequestMechanicExecute_)
		{
			for (int x = 0; x < players_.Size(); x++)
			{
				if (players_[x]->clientData_->connection_ == sender)
				{
					float lagTime = ServerGetSenderLagTime(sender);
					lagTime += ServerGetSenderLagTime(sender);
					((GameMechanic*)(players_[x]))->message_ = &msg;
					((GameMechanic*)(players_[x]))->clientID_ = players_[x]->clientData_->clientID_;
					((GameMechanic*)(players_[x]))->lagTime_ = lagTime;
					players_[x]->RequestMechanicExecute();
					break;
				}
			}
		}
	}
}

void PvPGM::ClientHandleNetworkMessage(StringHash eventType, VariantMap& eventData)//todo completely scrap this concept and use the urho event system
{
	using namespace NetworkMessage;

	int msgID = eventData[P_MESSAGEID].GetInt();

	Connection* sender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	if (msgID == MSG_GAMEMODEMSG)
	{
		const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
		MemoryBuffer msg(data);
		int messageID = msg.ReadInt();
		if (messageID == MSG_LOADSCENE_)
		{
			sceneFilename_ = msg.ReadString();

			File loadFile(context_,main_->filesystem_->GetProgramDir()
					+ "Data/Scenes/" + sceneFilename_, FILE_READ);
			scene_->LoadXML(loadFile);

			cameraNode_ = scene_->GetChild("camera");
			main_->viewport_->SetScene(scene_);
			main_->viewport_->SetCamera(cameraNode_->GetComponent<Camera>());

			//Let serverside gamemode know we've loaded the scene.
			msg_.Clear();
			msg_.WriteInt(MSG_SCENELOADED_);
			main_->network_->GetServerConnection()->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
		}
		else if (messageID == MSG_NEWPLAYER_)
		{
			int clientID = msg.ReadInt();
			String modelFilename = msg.ReadString();
			victoria_ = msg.ReadVector3();
			quarterOnion_ = msg.ReadQuaternion();

			for (int x = 0; x < client_->clientData_.Size(); x++)
			{
				if (client_->clientData_[x]->clientID_ == clientID)
				{
					NewPlayer(client_->clientData_[x], modelFilename, victoria_, quarterOnion_);
					break;
				}
			}
		}
		else if (messageID == MSG_PLAYERDISCO_)
		{
			int clientID = msg.ReadInt();

			for (int x = 0; x < players_.Size(); x++)
			{
				if (players_[x]->clientData_->clientID_ == clientID)
				{
					PlayerSB* player = players_[x];
					players_.Remove(players_[x]);
					delete player;
					break;
				}
			}
		}
		else if (messageID == MSG_Position_)
		{
			int clientID = msg.ReadInt();
			victoria_ = msg.ReadVector3();

			PlayerPosition(clientID, victoria_);
		}
		else if (messageID == MSG_SynchronizeComponent_)
		{
			int clientID = msg.ReadInt();

			float lagTime = 0.0f;

			//Get the senders lag.
			lagTime += ClientGetSenderLagTime(sender);

			for (int x = 0; x < players_.Size(); x++)
			{
				if (players_[x]->clientData_->clientID_ == clientID)
				{
					((GameMechanic*)(players_[x]))->message_ = &msg;
					((GameMechanic*)(players_[x]))->lagTime_ = lagTime;
					players_[x]->SetSynchronization();
					break;
				}
			}
		}
		else if (messageID == MSG_MechanicExecute_)
		{
			int clientID = msg.ReadInt();

			for (int x = 0; x < players_.Size(); x++)
			{
				if (players_[x]->clientData_->clientID_ == clientID)
				{
					float lagTime = 0.0f;
					lagTime += ClientGetSenderLagTime(sender);
					((GameMechanic*)(players_[x]))->message_ = &msg;
					((GameMechanic*)(players_[x]))->clientID_ = clientID;
					((GameMechanic*)(players_[x]))->lagTime_ = lagTime;
					players_[x]->MechanicExecute();
					break;
				}
			}
		}
	}
}

void PvPGM::ClientDisco(int clientID)
{
	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->clientData_->clientID_ == clientID)
		{
			msg_.Clear();
			msg_.WriteInt(MSG_PLAYERDISCO_);
			msg_.WriteInt(players_[x]->clientData_->clientID_);
			network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
			PlayerSB* player = players_[x];
			players_.Remove(players_[x]);
			delete player;
			break;
		}
	}
}

void PvPGM::TouchDown(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchBegin;
}

void PvPGM::TouchUp(StringHash eventType, VariantMap& eventData)
{
	if (main_->ui_->GetFocusElement() || touchSubscriberCount_)
	{
		return;
	}

	using namespace TouchEnd;

	Ray cameraRay = cameraNode_->GetComponent<Camera>()->GetScreenRay(
			(float) eventData[P_X].GetInt() / main_->graphics_->GetWidth(),
			(float) eventData[P_Y].GetInt() / main_->graphics_->GetHeight());

	//PODVector<RayQueryResult> results;
	PhysicsRaycastResult raeResult_;

	//RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f,
		//	DRAWABLE_GEOMETRY);

	scene_->GetComponent<PhysicsWorld>()->RaycastSingle(raeResult_, cameraRay, 10000.0f, 1);//todo define masks.

	//scene_->GetComponent<Octree>()->Raycast(query);
	//scene_->GetComponent<Octree>()->RaycastSingle(query);

	if (raeResult_.body_)//results.Size())
	{
		victoria_ = raeResult_.position_;
		//victoria_ = results[0].position_;
		/*
		for (int x = 0; x < results.Size(); x++)
		{
			if (results[x].node_->GetName() == "floor")
			{
				victoria_ = results[x].position_;
			}
		}*/
	}
	else
	{
		return;
	}

	PlayerSB* self = GetPlayerSelf();
	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_RequestMechanicExecute_);
	msg_.WriteString("SceneObjectMoveToWithCollision");
	msg_.WriteVector3(victoria_);
	int finalSpeed = self->player_->GetComponent<Speed>()->speed_;
	if (self->player_->GetComponent<Sprint>()->Sprinted_)
	{
		finalSpeed += self->player_->GetComponent<Sprint>()->sprint_;
	}
	if (self->player_->GetComponent<Snare>()->Snared_)
	{
		finalSpeed -= self->player_->GetComponent<Snare>()->snare_;
	}
	msg_.WriteFloat(finalSpeed);
	msg_.WriteFloat(self->player_->GetComponent<Gravity>()->gravity_);
	msg_.WriteBool(true);
	main_->network_->GetServerConnection()->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);

	vectoria_ = victoria_;
	vectoria_.y_ = self->player_->GetPosition().y_;//Same height so it doesn't rotate up/down, just left/right
	quarterOnion_ = self->player_->GetRotation();//Temp storage.
	self->player_->LookAt(vectoria_);//Hack rotation calculation :d
	quarterPounder_ = self->player_->GetRotation();
	self->player_->SetRotation(quarterOnion_);
	//self->RecursiveSetAnimation(self->player_, "run", true, 0);

	msg_.Clear();
	msg_.WriteInt(PvPGM::MSG_RequestMechanicExecute_);
	msg_.WriteString("SceneObjectRotateTo");
	msg_.WriteQuaternion(quarterPounder_);
	msg_.WriteFloat(self->player_->GetComponent<RotSpeed>()->rotSpeed_);
	msg_.WriteBool(true);
	main_->network_->GetServerConnection()->SendMessage(MSG_GAMEMODEMSG, true, true, msg_);
}

void PvPGM::TouchDrag(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchMove;
}

void PvPGM::PlayerPosition(int clientID, Vector3 destination)
{
	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->clientData_->clientID_ == clientID)
		{
			players_[x]->player_->GetComponent<SceneObjectMoveToWithCollision>()->isMoving_ = false;
			players_[x]->player_->SetPosition(destination);
			players_[x]->RecursiveSetAnimation(players_[x]->player_, "idle1", true, 0);
			break;
		}
	}
}

void PvPGM::ServerHandleMoveToComplete(StringHash eventType, VariantMap& eventData)
{
	using namespace SceneObjectMoveToComplete;

	SharedPtr<Node> player = SharedPtr<Node>(static_cast<Node*>(eventData[P_NODE].GetPtr()));

	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->player_ == player)
		{
			msg_.Clear();
			msg_.WriteInt(MSG_Position_);
			msg_.WriteInt(players_[x]->clientData_->clientID_);
			msg_.WriteVector3(player->GetPosition());
			network_->BroadcastMessage(MSG_GAMEMODEMSG, true, true, msg_);
			//players_[x]->RecursiveSetAnimation(players_[x]->player_, "idle1", true, 0);
			break;
		}
	}
}

void PvPGM::ServerHandleHealthStatus(StringHash eventType, VariantMap& eventData)
{
	using namespace HealthStatus;

	SharedPtr<Node> node = SharedPtr<Node>(static_cast<Node*>(eventData[P_NODE].GetPtr()));
	int health = static_cast<int>( eventData[P_HEALTH].GetInt() );

	if (health <= 0)//todo health cap at 100 perhaps
	{
		for (int x = 0; x < players_.Size(); x++)
		{
			if (players_[x]->player_ == node)
			{
				msg_.Clear();
				msg_.WriteString("Health");
				msg_.WriteInt(100);
				msg_.WriteByte(0);
				const PODVector<unsigned char>& data = msg_.GetBuffer();
				MemoryBuffer msg(data);
				((GameMechanic*)(players_[x]))->message_ = &msg;
				((GameMechanic*)(players_[x]))->clientID_ = players_[x]->clientData_->clientID_;
				((GameMechanic*)(players_[x]))->lagTime_ = 0.0f;
				players_[x]->RequestMechanicExecute();

				int index = Random( 0, spawnPoints_.Size() );
				victoria_ = spawnPoints_[index]->GetPosition();
				quarterOnion_ = spawnPoints_[index]->GetRotation();

				msg_.Clear();
				msg_.WriteString("Transform");
				msg_.WriteVector3(victoria_);
				msg_.WriteQuaternion(quarterOnion_);
				const PODVector<unsigned char>& data2 = msg_.GetBuffer();
				MemoryBuffer msg2(data);
				((GameMechanic*)(players_[x]))->message_ = &msg2;
				((GameMechanic*)(players_[x]))->clientID_ = players_[x]->clientData_->clientID_;
				((GameMechanic*)(players_[x]))->lagTime_ = 0.0f;
				players_[x]->RequestMechanicExecute();

				break;
			}
		}
	}
}

int PvPGM::GetSenderClientID(Connection* sender)
{
	for (int x = 0; x < server_->clientData_.Size(); x++)
	{
		if (server_->clientData_[x]->connection_ == sender)
		{
			return server_->clientData_[x]->clientID_;
		}
	}
	return -1;
}

int PvPGM::GetNodeClientID(Node* noed)
{
	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->player_ == noed)
		{
			return players_[x]->clientData_->clientID_;
		}
	}
	return -1;
}

Node* PvPGM::GetClientIDNode(int clientID)
{
	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->clientData_->clientID_ == clientID)
		{
			return players_[x]->player_;
		}
	}
	return NULL;
}

float PvPGM::ServerGetSenderLagTime(Connection* sender)
{
	for (int y = 0; y < server_->netpulse_->connections_.Size(); y++)
	{
		if (server_->netpulse_->connections_[y]->connection_ == sender)
		{
			return server_->netpulse_->connections_[y]->lagTime_;
		}
	}
	return -1;
}

float PvPGM::ClientGetSenderLagTime(Connection* sender)
{
	for (int y = 0; y < client_->netpulse_->connections_.Size(); y++)
	{
		if (client_->netpulse_->connections_[y]->connection_ == sender)
		{
			return client_->netpulse_->connections_[y]->lagTime_;
		}
	}
	return -1;
}

void PvPGM::RequestAnimation(int clientID, String ani, bool loop, unsigned char layer)
{
	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->clientData_->clientID_ == clientID)
		{
			players_[x]->RecursiveSetAnimation(players_[x]->player_, ani, loop, layer);
			break;
		}
	}
}

void PvPGM::RequestMechanicExecution(Node* receiver, VectorBuffer& mechanicParams)
{
	const PODVector<unsigned char>& data = mechanicParams.GetBuffer();
	MemoryBuffer msg(data);

	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->player_ == receiver)
		{
			((GameMechanic*)(players_[x]))->message_ = &msg;
			((GameMechanic*)(players_[x]))->clientID_ = ((GameMechanic*)(players_[x]))->clientIDSelf_;
			((GameMechanic*)(players_[x]))->lagTime_ = 0.0f;
			players_[x]->RequestMechanicExecute();
			break;
		}
	}
}

PlayerSB* PvPGM::GetPlayerSelf()
{
	for (int x = 0; x < players_.Size(); x++)
	{
		if (players_[x]->clientData_->self_)
		{
			return players_[x];
		}
	}

	return NULL;
}

bool PvPGM::GetIsHost()
{
	if (hosting_)
	{
		return true;
	}
	return false;
}

void PvPGM::LogOut()
{
	delete this;
}

Urho3DPlayer* PvPGM::GetMain()
{
	return main_;
}

Scene* PvPGM::GetScene()
{
	return scene_;
}

Node* PvPGM::GetCamera()
{
	return cameraNode_;
}
