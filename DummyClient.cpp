#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"

char sendData[] = "HyeongSeokServer";

class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{
		cout << "~ServerSession 소멸자" << endl;
	}

	virtual void OnConnected() override
	{
		Protocol::CS_LOGIN loginPk;
		auto sendBuffer = ServerPacketHandler::CreateSendBuffer(loginPk);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		shared_ptr<PacketSession> session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		ServerPacketHandler::PacketHandler(session, buffer, len);
	}

	virtual void OnSend(int32 len) override
	{
		cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		cout << "Disconnected !!!! " << endl;
	}
};

int main()
{
	ServerPacketHandler::InitBinding();

	this_thread::sleep_for(1s);

	shared_ptr<ClientService> service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 30001),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>,
		500);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	Protocol::CS_CHAT chatPk;
	chatPk.set_msg(u8"Start!");
	auto sendBuffer = ServerPacketHandler::CreateSendBuffer(chatPk);

	while (true)
	{
		service->Broadcast(sendBuffer);
		this_thread::sleep_for(1s);
	}

	GThreadManager->Join();
}