// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/GridTypes.h"
#include "DebugHelpers.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GHCLAUDEDUNGEONGEN_API UDebugHelpers : public UActorComponent
{
	GENERATED_BODY()

public:
	UDebugHelpers();

	/** Enable/disable debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bShowDebugVisualization;

	/** Draw grid lines in green */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawGrid;

	/** Draw occupied cells in red */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawOccupiedCells;

	/** Draw unoccupied cells in blue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawUnoccupiedCells;

	/** Draw forced placements with special markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	bool bDrawForcedPlacements;

	/** Thickness of debug lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DebugLineThickness;

	/** Height offset for debug visualization (to avoid z-fighting) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	float DebugVisualizationZOffset;

	/**
	 * Draw the grid lines in the editor
	 * @param Grid - Array of grid cells to visualize
	 * @param CellSize - Size of each cell in world units
	 * @param OwnerLocation - World location of the room actor
	 * @param World - World context for drawing
	 */
	void DrawGrid(const TArray<FGridCell>& Grid, float CellSize, const FVector& OwnerLocation, UWorld* World);

	/**
	 * Draw occupied cells in red
	 * @param Grid - Array of grid cells to visualize
	 * @param CellSize - Size of each cell in world units
	 * @param OwnerLocation - World location of the room actor
	 * @param World - World context for drawing
	 */
	void DrawOccupiedCells(const TArray<FGridCell>& Grid, float CellSize, const FVector& OwnerLocation, UWorld* World);

	/**
	 * Draw unoccupied cells in blue
	 * @param Grid - Array of grid cells to visualize
	 * @param CellSize - Size of each cell in world units
	 * @param OwnerLocation - World location of the room actor
	 * @param World - World context for drawing
	 */
	void DrawUnoccupiedCells(const TArray<FGridCell>& Grid, float CellSize, const FVector& OwnerLocation, UWorld* World);

	/**
	 * Draw forced placement markers
	 * @param ForcedPlacements - Map of forced placements to visualize
	 * @param CellSize - Size of each cell in world units
	 * @param OwnerLocation - World location of the room actor
	 * @param World - World context for drawing
	 */
	void DrawForcedPlacements(const TMap<FIntPoint, struct FMeshPlacementData>& ForcedPlacements, float CellSize, const FVector& OwnerLocation, UWorld* World);

protected:
	/** Helper function to draw a cell rectangle */
	void DrawCellRect(const FVector& CellWorldPosition, float CellSize, const FLinearColor& Color, UWorld* World);
};
