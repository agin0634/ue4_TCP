// Fill out your copyright notice in the Description page of Project Settings.


#include "TCPSocketInstance.h"

class FAcceptTask : public FNonAbandonableTask
{
private:
	UTCPSocketInstance* instance;
	FCriticalSection CriticalSection;

public:

	FAcceptTask(UTCPSocketInstance* SocketInstance)
	{
		instance = SocketInstance;
	}

	~FAcceptTask()
	{
		instance = nullptr;

		UE_LOG(LogTemp, Warning, TEXT("AcceptTask Stop"));

	}

	void DoWork() 
	{
		if (!instance || instance->bIsAccepting == false) 
		{
			UE_LOG(LogTemp, Warning, TEXT("instance is invalid"));
			return;
		}

		while (instance && instance->bIsAccept != true)
		{
			if (!instance || instance->bIsAccept == true)
			{
				break;
			}

			if (instance->bIsAccepting == false)
			{
				break;
			}

			CriticalSection.Lock();
			acceptSocket();
			CriticalSection.Unlock();
		}
	}

	void acceptSocket()
	{
		// accept socket
		TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		uint32 ipd;
		bool Pending;

		if (instance->SocketServer->HasPendingConnection(Pending) && Pending)
		{
			instance->SocketClient = instance->SocketServer->Accept(*targetAddr, "received socket Connection");
		}
		
		if (instance->SocketClient != NULL)
		{
			targetAddr->GetIp(ipd);

			char strTemp[20];
			sprintf(strTemp, "%d.%d.%d.%d",
				(ipd & 0xff000000) >> 24,
				(ipd & 0x00ff0000) >> 16,
				(ipd & 0x0000ff00) >> 8,
				(ipd & 0x000000ff));

			UE_LOG(LogTemp, Warning, TEXT("IP:%s , Port:%d"), *FString(strTemp), targetAddr->GetPort());
			instance->bIsAccept = true;
		}
		else
		{
			instance->bIsAccept = false;
		}
	}


	/*
	Need this function
	example: https://wiki.unrealengine.com/Using_AsyncTasks
	 */
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAcceptTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

bool UTCPSocketInstance::createSoc(const FString& TheIP, const int32 ThePort, const int32 MaxBacklog)
{
	// create socket server
	SocketServer = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	if (!SocketServer) {
		UE_LOG(LogTemp, Warning, TEXT("socket create ERROR"));
		return false;
	}

	// bind socket
	FIPv4Address::Parse(TheIP, ip);
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(ThePort);

	if (!SocketServer->Bind(*addr))
	{
		UE_LOG(LogTemp, Warning, TEXT("socket bind ERROR"));
		return false;
	}

	// listen socket
	if (!SocketServer->Listen(MaxBacklog))
	{
		UE_LOG(LogTemp, Warning, TEXT("socket listen ERROR"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("socket create succeed"));
	return true;
}

void UTCPSocketInstance::acceptSoc()
{
	AcceptTask = (new FAutoDeleteAsyncTask<FAcceptTask>(this));
	AcceptTask->StartBackgroundTask();
	bIsAccepting = true;
}

void UTCPSocketInstance::closeSoc()
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
	if (AcceptTask) 
	{
		AcceptTask = nullptr;
		bIsAccepting = false;
	}
}

FString UTCPSocketInstance::recvSoc() 
{
	//TODO recvUE4data is dirty
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

FString UTCPSocketInstance::StringFromBinaryArray(TArray<uint8> BinaryArray)
{
	return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(BinaryArray.GetData())));
}



/*
// accept socket
uint32 ipd;

SocketClient = SocketServer->Accept(*addr, "aaa");
addr->GetIp(ipd);

char strTemp[20];
sprintf(strTemp, "%d.%d.%d.%d",
	(ipd & 0xff000000) >> 24,
	(ipd & 0x00ff0000) >> 16,
	(ipd & 0x0000ff00) >> 8,
	(ipd & 0x000000ff));

if (!SocketClient)
{
	UE_LOG(LogTemp, Warning, TEXT("accept ERROR"));
	return false;
}
else
{
	UE_LOG(LogTemp, Warning, TEXT("IP:%s , Port:%d"), *FString(strTemp), addr->GetPort());
	return true;
}
*/