// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Engine.h"
#include "TCPSocketServer.generated.h"

/**
 * 
 */
UCLASS()
class TCPTEST_API UTCPSocketServer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MySocket")
	static bool createSoc();

	UFUNCTION(BlueprintCallable, Category = "MySocket")
	static void closeSoc();

	UFUNCTION(BlueprintCallable, Category = "MySocket")
	static bool bindSoc(const FString& TheIP, const int32 ThePort);

	UFUNCTION(BlueprintCallable, Category = "MySocket")
	static bool listenSoc(const int32 MaxBacklog);

	UFUNCTION(BlueprintCallable, Category = "MySocket")
	static bool acceptSoc(FString& TheIP, int32 & ThePort);

	UFUNCTION(BlueprintCallable, Category = "MySocket")
	static FString recvSoc();

	static FString StringFromBinaryArray(const TArray<uint8>& BinaryArray);
};
