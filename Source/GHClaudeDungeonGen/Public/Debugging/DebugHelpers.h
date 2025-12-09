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
 * Debug helper component for visualizing room generation
 * Provides grid visualization, cell state display, and doorway indicators
 * UDebugHelpers - Component for debugging and visualization of dungeon generation
 * Provides methods for drawing grids, cells, walls, and doorways in editor and runtime
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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

private:
	/** Helper to get world location for debug drawing */
	FVector GetWorldLocationForCell(const FIntPoint& GridCoord, float CellSize) const;

	/** Helper to get cell color based on state */
	FColor GetColorForCellState(ECellState State) const;
	// ========== Debug Display Options ==========
	
	/** Whether to draw the grid overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display")
	bool bShowGrid;

	/** Whether to draw cell coordinates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display")
	bool bShowCellCoordinates;

	/** Whether to draw cell states (color-coded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display")
	bool bShowCellStates;

	/** Whether to draw wall indicators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display")
	bool bShowWalls;

	/** Whether to draw doorway indicators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display")
	bool bShowDoorways;

	/** Whether to draw snap point locations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display")
	bool bShowSnapPoints;

	/** Duration for debug draw persistence (0 = one frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float DebugDrawDuration;

	/** Thickness of debug lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Display", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DebugLineThickness;

	// ========== Debug Drawing Methods ==========
	
	/** Draw grid overlay for the specified cells */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawGrid(const TArray<FGridCell>& GridCells, float CellSize);

	/** Draw cell coordinates as text */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawCellCoordinates(const TArray<FGridCell>& GridCells);

	/** Draw cell states with color coding */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawCellStates(const TArray<FGridCell>& GridCells, float CellSize);

	/** Draw wall indicators */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawWalls(const TArray<FGridCell>& GridCells, float CellSize);

	/** Draw doorway indicators */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawDoorways(const TArray<FGridCell>& GridCells, float CellSize);

	/** Draw snap points */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawSnapPoints(const TArray<FVector>& SnapPoints);

	/** Draw all debug visualizations for a room */
	UFUNCTION(BlueprintCallable, Category = "Debug Helpers")
	void DrawRoomDebug(const TArray<FGridCell>& GridCells, const TArray<FVector>& SnapPoints, float CellSize);
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

	/** Get color for cell state */
	UFUNCTION(BlueprintPure, Category = "Debug Helpers")
	FColor GetCellStateColor(ECellState State) const;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Cache of grid cells for continuous drawing */
	TArray<FGridCell> CachedGridCells;
	
	/** Cache of snap points for continuous drawing */
	TArray<FVector> CachedSnapPoints;
	
	/** Cached cell size */
	float CachedCellSize;

	/** Helper method to check if any debug flags are enabled */
	bool IsAnyDebugEnabled() const;
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
