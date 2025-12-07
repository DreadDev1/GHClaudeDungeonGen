// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugHelpers.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GHCLAUDEDUNGEONGEN_API UDebugHelpers : public UActorComponent
{
	GENERATED_BODY()

public:
	UDebugHelpers();

#if WITH_EDITORONLY_DATA
	// Toggle bools for debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Grid")
	bool bShowGridCreated = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Grid")
	bool bShowUnoccupiedCells = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Grid")
	bool bShowOccupiedCells = true;

	// Draw grid outline when grid is created (Green)
	UFUNCTION(BlueprintCallable, Category = "Debug|Grid")
	void DrawGridCreated(const FVector& CellCenter, float CellSize, float Duration = 2.0f);

	// Draw unoccupied cell outline (Blue)
	UFUNCTION(BlueprintCallable, Category = "Debug|Grid")
	void DrawUnoccupiedCell(const FVector& CellCenter, float CellSize, float Duration = 2.0f);

	// Draw occupied cell outline (Red)
	UFUNCTION(BlueprintCallable, Category = "Debug|Grid")
	void DrawOccupiedCell(const FVector& CellCenter, float CellSize, float Duration = 2.0f);

private:
	void DrawCellBox(const FVector& CellCenter, float CellSize, const FColor& Color, float Duration);
#endif
};
