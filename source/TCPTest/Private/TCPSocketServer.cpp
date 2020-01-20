// Fill out your copyright notice in the Description page of Project Settings.


#include "TCPSocketServer.h"

FSocket* SocketServer;
FSocket* SocketClient;
FIPv4Address ip;

bool UTCPSocketServer::createSoc()
{
	SocketServer = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	if (!SocketServer) {
		return false;
	}
	// SocketServer.SetNonBlocking(false);
	//SocketServer->SetNonBlocking(false);
	return true;
}

void UTCPSocketServer::closeSoc()
{
	if (SocketServer)
	{
		SocketServer->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SocketServer);
	}
	if (SocketClient)
	{
		SocketClient->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SocketClient);
	}
}

bool UTCPSocketServer::bindSoc(const FString& TheIP, const int32 ThePort)
{
	FIPv4Address::Parse(TheIP, ip);
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(ThePort);
	bool bBind = SocketServer->Bind(*addr);
	return bBind;
}

bool UTCPSocketServer::listenSoc(const int32 MaxBacklog)
{
	bool bListen = SocketServer->Listen(MaxBacklog);
	return bListen;
}

bool UTCPSocketServer::acceptSoc(FString & TheIP, int32 & ThePort)
{
	TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FIPv4Address::Parse(TheIP, ip);
	uint32 ipd;
	//targetAddr->SetIp(ip.Value);
	//targetAddr->SetPort(ThePort);
	SocketClient = SocketServer->Accept(*targetAddr, "aaa");
	ThePort = targetAddr->GetPort();
	targetAddr->GetIp(ipd);



	char strTemp[20];
	sprintf(strTemp, "%d.%d.%d.%d",
		(ipd & 0xff000000) >> 24,
		(ipd & 0x00ff0000) >> 16,
		(ipd & 0x0000ff00) >> 8,
		(ipd & 0x000000ff));


	TheIP = FString(strTemp);


	if (!SocketClient)
	{
		return false;
	}
	return true;
}

FString UTCPSocketServer::recvSoc()
{
	TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	TArray<uint8> ReceivedData;
	uint32 Size;
	if (SocketClient->HasPendingData(Size))
	{
		uint8 *Recv = new uint8[Size];
		int32 BytesRead = 0;
		ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
		SocketClient->RecvFrom(ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *targetAddr);
		if (ReceivedData.Num() > 0)
		{
			FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
			return ReceivedUE4String;
		}
	}
	return FString();
}

FString UTCPSocketServer::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(BinaryArray.GetData())));
}