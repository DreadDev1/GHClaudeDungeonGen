// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/GridTypes.h"
#include "DebugHelpers.generated.h"

/**
 * UDebugHelpers - Component for visualizing dungeon generation data
 * Provides debug drawing for grids, cell states, walls, doorways, and forced placements
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class GHCLAUDEDUNGEONGEN_API UDebugHelpers : public UActorComponent
{
	GENERATED_BODY()

public:
	UDebugHelpers();

	// ========== Debug Visualization Properties ==========
	
	/** Enable debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bEnableDebugDraw;

	/** Draw grid cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawGrid;

	/** Draw cell states (color-coded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawCellStates;

	/** Draw walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawWalls;

	/** Draw doorways */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawDoorways;

	/** Draw forced placements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawForcedPlacements;

	/** Debug line thickness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	float DebugLineThickness;

	/** Debug draw duration (-1 for persistent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	float DebugDrawDuration;

	// ========== Debug Colors ==========
	
	/** Color for unoccupied cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor UnoccupiedCellColor;

	/** Color for occupied cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor OccupiedCellColor;

	/** Color for reserved cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor ReservedCellColor;

	/** Color for excluded cells */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor ExcludedCellColor;

	/** Color for walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor WallColor;

	/** Color for doorways */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor DoorwayColor;

	/** Color for forced placements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization|Colors")
	FColor ForcedPlacementColor;

	// ========== API Methods ==========
	
	/** Draws the complete grid */
	UFUNCTION(BlueprintCallable, Category = "Debug Visualization")
	void DrawDebugGrid(const TMap<FIntPoint, FGridCell>& Grid, float CellSize);

	/** Draws cell states with color coding */
	UFUNCTION(BlueprintCallable, Category = "Debug Visualization")
	void DrawDebugCellStates(const TMap<FIntPoint, FGridCell>& Grid, float CellSize);

	/** Draws walls */
	UFUNCTION(BlueprintCallable, Category = "Debug Visualization")
	void DrawDebugWalls(const TMap<FIntPoint, FGridCell>& Grid, float CellSize);

	/** Draws doorways */
	UFUNCTION(BlueprintCallable, Category = "Debug Visualization")
	void DrawDebugDoorways(const TMap<FIntPoint, FGridCell>& Grid, float CellSize);

	/** Draws forced placement highlights */
	UFUNCTION(BlueprintCallable, Category = "Debug Visualization")
	void DrawDebugForcedPlacements(const TMap<FIntPoint, FMeshPlacementData>& ForcedPlacements, float CellSize);

	/** Toggles debug drawing on/off */
	UFUNCTION(BlueprintCallable, Category = "Debug Visualization")
	void ToggleDebugDraw();

	/** Draws all debug visualizations if enabled */
	UFUNCTION(BlueprintCallable, Category = "Debug Visualization")
	void UpdateDebugVisualization(const TMap<FIntPoint, FGridCell>& Grid, float CellSize);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Helper to get world location for debug drawing */
	FVector GetWorldLocationForCell(const FIntPoint& GridCoord, float CellSize) const;

	/** Helper to get cell color based on state */
	FColor GetColorForCellState(ECellState State) const;
};
