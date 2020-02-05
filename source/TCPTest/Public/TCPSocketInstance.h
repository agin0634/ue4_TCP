// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Networking/Public/Networking.h"

#include "Core/Public/Async/AsyncWork.h"
#include "TCPSocketInstance.generated.h"

/**
 * 
 */

class FAcceptTask;

UCLASS()
class TCPTEST_API UTCPSocketInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	friend class FAcceptTask;

	UFUNCTION(BlueprintCallable, Category = "TCPSocket")
	bool createSoc(const FString& TheIP, const int32 ThePort, const int32 MaxBacklog);

	UFUNCTION(BlueprintCallable, Category = "TCPSocket")
	void acceptSoc();

	UFUNCTION(BlueprintCallable, Category = "TCPSocket")
	void closeSoc();

	UFUNCTION(BlueprintCallable, Category = "TCPSocket")
	FString recvSoc();

	static FString StringFromBinaryArray(TArray<uint8> BinaryArray);
	
public:
	FSocket* SocketClient;
	FSocket* SocketServer;
	FIPv4Address ip;
	FString ReceivedUE4String = FString();
	bool bIsAccepting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAccept = false;

private:
	FAutoDeleteAsyncTask<FAcceptTask>* AcceptTask = nullptr;

};
