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
			UE_LOG(LogTemp, Warning, TEXT("instance is Invalid"));
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
		bIsAccepting = false;
		AcceptTask = nullptr;
	}
}

FString UTCPSocketInstance::recvSoc() 
{
	TArray<uint8> ReceivedData;
	uint32 Size;
	if (SocketClient->HasPendingData(Size))
	{
		uint8 *Recv = new uint8[Size];
		int32 BytesRead = 0;
		ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
		SocketClient->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);
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
	BinaryArray.Add(0);// Add 0 termination. Even if the string is already 0-terminated, it doesn't change the results.
	// Create a string from a byte array. The string is expected to be 0 terminated (i.e. a byte set to 0).
	// Use UTF8_TO_TCHAR if needed.
	// If you happen to know the data is UTF-16 (USC2) formatted, you do not need any conversion to begin with.
	// Otherwise you might have to write your own conversion algorithm to convert between multilingual UTF-16 planes.
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(BinaryArray.GetData())));
}
