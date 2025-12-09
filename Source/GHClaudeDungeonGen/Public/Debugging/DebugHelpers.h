// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/GridTypes.h"
#include "DebugHelpers.generated.h"

/**
 * Debug helper component for visualizing room generation
 * Provides grid visualization, cell state display, and doorway indicators
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GHCLAUDEDUNGEONGEN_API UDebugHelpers : public UActorComponent
{
	GENERATED_BODY()

public:
	UDebugHelpers();

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
};
