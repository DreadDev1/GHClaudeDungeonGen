// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/GridTypes.h"
#include "DebugHelpers.generated.h"

/**
 * UDebugHelpers - Component for debugging and visualization of dungeon generation
 * Provides methods for drawing grids, cells, walls, and doorways in editor and runtime
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GHCLAUDEDUNGEONGEN_API UDebugHelpers : public UActorComponent
{
	GENERATED_BODY()

public:
	UDebugHelpers();

	// ========== Debug Visualization Settings ==========
	
	/** Whether to draw the grid in editor/runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawGrid;

	/** Whether to draw cell coordinates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawCellCoordinates;

	/** Whether to draw wall indicators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawWalls;

	/** Whether to draw doorway indicators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawDoorways;

	/** Whether to draw cell state colors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawCellStates;

	/** Color for grid lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor GridColor;

	/** Color for wall lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor WallColor;

	/** Color for doorway indicators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor DoorwayColor;

	/** Color for unoccupied cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor UnoccupiedCellColor;

	/** Color for occupied cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor OccupiedCellColor;

	/** Thickness of debug lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	float LineThickness;

	/** Height offset for drawing lines above floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	float DebugDrawHeight;

	// ========== Debug Drawing Methods ==========
	
	/** Draw a single grid cell */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawGridCell(const FGridCell& Cell, float CellSize, const FVector& Offset = FVector::ZeroVector);

	/** Draw multiple grid cells */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawGridCells(const TArray<FGridCell>& Cells, float CellSize, const FVector& Offset = FVector::ZeroVector);

	/** Draw wall indicators for a cell */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawCellWalls(const FGridCell& Cell, float CellSize, const FVector& Offset = FVector::ZeroVector);

	/** Draw doorway indicators for a cell */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawCellDoorways(const FGridCell& Cell, float CellSize, const FVector& Offset = FVector::ZeroVector);

	/** Draw a coordinate label at a cell position */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawCellCoordinate(const FGridCell& Cell, const FVector& Offset = FVector::ZeroVector);

	/** Draw doorway snap points */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawDoorwaySnapPoints(const TArray<FVector>& SnapPoints, float Radius = 20.0f);

	/** Clear all debug drawings */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void ClearDebugDrawings();

protected:
	/** Called every frame (if tick is enabled) */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Get the world for drawing debug shapes */
	UWorld* GetDebugWorld() const;
};
