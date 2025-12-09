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

private:
	/** Get the world for drawing debug shapes */
	UWorld* GetDebugWorld() const;
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
