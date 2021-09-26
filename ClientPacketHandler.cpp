#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Room.h"
#include "GameSession.h"
using namespace std;
PacketHandlerFunc GPacketBiding[UINT16_MAX];

bool Handle_INVALID(shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Handle_SC_LOGIN(shared_ptr<PacketSession>& session, Protocol::CS_LOGIN& pk)
{
	shared_ptr<GameSession> gSession = static_pointer_cast<GameSession>(session);

	Protocol::SC_LOGIN loginPk;
	loginPk.set_success(true);
	static Atomic<uint64> genId = 1;

	{
		auto player = loginPk.add_players();
		player->set_name(u8"Test1");
		player->set_playertype(Protocol::TEST_PLAYER);

		shared_ptr<Player> refPlayer = MakeShared<Player>();
		refPlayer->playerId = genId++;
		refPlayer->name = player->name();
		refPlayer->type = player->playertype();
		refPlayer->ownerSession = gSession;
		
		gSession->_players.push_back(refPlayer);
	}

	{
		auto player = loginPkt.add_players();
		player->set_name(u8"Test2");
		player->set_playertype(Protocol::PLAYER_TYPE_MAGE);

		shared_ptr<Player> playerPtr = MakeShared<Player>();
		playerPtr->playerId = genId++;
		playerPtr->name = player->name();
		playerPtr->type = player->playertype();
		playerPtr->ownerSession = gSession;

		gSession->_players.push_back(playerPtr);
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_CS_ENTER_GAME(shared_ptr<PacketSession>& session, Protocol::CS_ENTER_GAME& pk)
{
	shared_ptr<GameSession> gameSession = static_pointer_cast<GameSession>(session);

	uint64 index = pk.playerindex();
	gameSession->_currentPlayer = gameSession->_players[index];
	gameSession->_room = roomPtr;

	roomPtr->DoActionByAsync(&Room::Enter, gameSession->_currentPlayer);

	Protocol::SC_ENTER_GAME enterPk;
	enterPk.set_success(true);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterPk);
	gameSession->_currentPlayer->ownerSession->Send(sendBuffer);

	return true;
}

bool Handle_CS_CHAT(shared_ptr<PacketSession>& session, Protocol::CS_CHAT& pk)
{
	cout << pk.msg() << endl;

	Protocol::SC_CHAT chatPk;
	chatPk.set_msg(pkt.msg());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPk);

	roomPtr->DoActionByAsync(&Room::Broadcast, sendBuffer);

	return true;
}
