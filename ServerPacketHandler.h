#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(shared_ptr<PacketSession>&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketBiding[UINT16_MAX];

enum : uint16
{
	//Lobby 100~
	CS_LOGIN = 100,
	SC_LOGIN = 101,
	CS_CREATE_PLAYER = 102,
	SC_CREATE_PLAYER = 103,
	//InGame 2000~
	CS_ENTER_GAME = 2002,
	SC_ENTER_GAME = 2003,
	CS_CHAT = 2004,
	SC_CHAT = 2005,


};

bool Handle_INVALID(shared_ptr<PacketSession>& session, BYTE* buffer, int32 len);
bool Handle_SC_LOGIN(shared_ptr<PacketSession>& session, Protocol::SC_LOGIN& pkt);
bool Handle_SC_CREATE_PLAYER(shared_ptr<PacketSession>& session, Protocol::SC_CREATE_PLAYER& pkt);
bool Handle_SC_ENTER_GAME(shared_ptr<PacketSession>& session, Protocol::SC_ENTER_GAME& pkt);
bool Handle_SC_CHAT(shared_ptr<PacketSession>& session, Protocol::SC_CHAT& pkt);


class ServerPacketHandler
{
public:
	static void InitBinding()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketBiding[i] = Handle_INVALID;
		GPacketBiding[SC_LOGIN] = 
			[](shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
		{ return PacketHandler<Protocol::SC_LOGIN>(Handle_SC_LOGIN, session, buffer, len); };
		GPacketBiding[SC_ENTER_GAME] =
			[](shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) 
		{ return PacketHandler<Protocol::SC_ENTER_GAME>(Handle_SC_ENTER_GAME, session, buffer, len); };
		GPacketBiding[SC_CHAT] =
			[](shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) 
		{ return PacketHandler<Protocol::SC_CHAT>(Handle_SC_CHAT, session, buffer, len); };
		GPacketBiding[SC_CREATE_PLAYER] =
			[](shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
		{ return PacketHandler<Protocol::SC_CREATE_PLAYER>(Handle_SC_CREATE_PLAYER, session, buffer, len); };

	}

	static bool PacketHandler(shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketBiding[header->id](session, buffer, len);
	}
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::CS_LOGIN& packet) { return CreateSendBuffer(packet, CS_LOGIN); }
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::CS_ENTER_GAME& packet) { return CreateSendBuffer(packet, CS_ENTER_GAME); }
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::CS_CHAT& packet) { return CreateSendBuffer(packet, CS_CHAT); }
	static shared_ptr<SendBuffer> CreateSendBuffer(Protocol::CS_CREATE_PLAYER& packet) { return CreateSendBuffer(packet, CS_CREATE_PLAYER); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool PacketHandler(ProcessFunc func, shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static shared_ptr<SendBuffer> CreateSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		shared_ptr<SendBuffer> sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};