// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MasterRoom.generated.h"

class UDebugHelpers;
UCLASS()
class GHCLAUDEDUNGEONGEN_API AMasterRoom : public AActor
{
	GENERATED_BODY()

public:
	AMasterRoom();
	
	// Grid Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridSizeX = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridSizeY = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 100.0f;

	// Grid cell states: true = occupied, false = unoccupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	TArray<bool> GridCells;

	// Grid functions
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void InitializeGrid();

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector GetCellWorldPosition(int32 CellIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsValidCellIndex(int32 CellIndex) const;

protected:
	// Debug Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	TObjectPtr<UDebugHelpers> DebugHelpers;

};
