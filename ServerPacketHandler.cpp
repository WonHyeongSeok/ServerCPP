#include "pch.h"
#include "ServerPacketHandler.h"

PacketHandlerFunc GPacketBiding[UINT16_MAX];

bool Handle_INVALID(shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	
	return false;
}

bool Handle_SC_LOGIN(shared_ptr<PacketSession>& session, Protocol::SC_LOGIN& pkt)
{
	if (pkt.success() == false)
		return true;

	if (pkt.players().size() == 0)
	{
		Protocol::SC_CreatePlayer createPk;
		auto createSendBuffer = ServerPacketHandler::CreateSendBuffer(createPk);
		session->Send(createSendBuffer);
		return false;
	}

	Protocol::CS_ENTER_GAME enterPk;
	enterPk.set_playerindex(0);
	//일단 캐릭터 하나만 나중에 파주자..
	auto sendBuffer = ServerPacketHandler::CreateSendBuffer(enterPk);
	session->Send(sendBuffer);

	return true;
}

bool Handle_SC_ENTER_GAME(shared_ptr<PacketSession>& session, Protocol::SC_ENTER_GAME& pkt)
{
	return true;
}

bool Handle_SC_CHAT(shared_ptr<PacketSession>& session, Protocol::SC_CHAT& pkt)
{
	std::cout << pkt.msg() << endl;
	return true;
}
