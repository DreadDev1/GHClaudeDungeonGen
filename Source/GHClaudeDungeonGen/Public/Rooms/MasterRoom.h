// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/GridTypes.h"
#include "Types/RoomShapeTypes.h"
#include "MasterRoom.generated.h"

// Forward declarations
class UDebugHelpers;
class URoomData;
class USceneComponent;

/**
 * AMasterRoom - Runtime room generator actor
 * Handles procedural generation of dungeon rooms from RoomData assets
 * Supports both editor-time and runtime generation with forced placement workflow
 */
UCLASS(Blueprintable, BlueprintType)
class GHCLAUDEDUNGEONGEN_API AMasterRoom : public AActor
{
	GENERATED_BODY()

public:
	AMasterRoom();

	// ========== Core Properties ==========
	
	/** Reference to the RoomData asset that defines this room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation")
	TSoftObjectPtr<URoomData> RoomData;

	/** Generation seed for this room instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation", meta = (EditCondition = "!bUseRandomSeed"))
	int32 GenerationSeed;

	/** If true, generates a random seed each time; otherwise uses GenerationSeed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation")
	bool bUseRandomSeed;

	/** Optional shape override - if set, uses this instead of RoomData's AllowedShapes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation")
	FRoomShapeDefinition ShapeOverride;

	/** Whether to use the ShapeOverride instead of RoomData shapes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation")
	bool bUseShapeOverride;

	/** Flag indicating whether this room has been generated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation")
	bool bIsGenerated;

	// ========== Runtime Grid ==========
	
	/** Runtime grid storing all cell data (key = grid coordinates) */
	UPROPERTY()
	TMap<FIntPoint, FGridCell> RuntimeGrid;

	/** Random stream for consistent random generation */
	UPROPERTY()
	FRandomStream RandomStream;

	// ========== Doorway Snap Points ==========
	
	/** Doorway snap points on the north edge (in grid coordinates) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Doorways")
	TArray<FIntPoint> NorthDoorwaySnapPoints;

	/** Doorway snap points on the east edge (in grid coordinates) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Doorways")
	TArray<FIntPoint> EastDoorwaySnapPoints;

	/** Doorway snap points on the south edge (in grid coordinates) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Doorways")
	TArray<FIntPoint> SouthDoorwaySnapPoints;

	/** Doorway snap points on the west edge (in grid coordinates) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Doorways")
	TArray<FIntPoint> WestDoorwaySnapPoints;

	// ========== Scene Component Containers ==========
	
	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Components")
	USceneComponent* RootSceneComponent;

	/** Container for all floor mesh instances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Components")
	USceneComponent* FloorContainer;

	/** Container for all wall mesh instances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Components")
	USceneComponent* WallContainer;

	/** Container for all door mesh instances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Components")
	USceneComponent* DoorContainer;

	/** Container for all ceiling mesh instances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Components")
	USceneComponent* CeilingContainer;

	/** Debug helper component for visualization */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Generation|Components")
	UDebugHelpers* DebugHelpers;

	// ========== Forced Placements ==========
	
	/** Forced floor placements (key = bottom-left grid coordinate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation|Forced Placements")
	TMap<FIntPoint, FMeshPlacementData> ForcedFloorPlacements;

	/** Forced wall placements (key = bottom-left grid coordinate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation|Forced Placements")
	TMap<FIntPoint, FMeshPlacementData> ForcedWallPlacements;

	/** Forced ceiling placements (key = bottom-left grid coordinate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Generation|Forced Placements")
	TMap<FIntPoint, FMeshPlacementData> ForcedCeilingPlacements;

	// ========== API Methods ==========
	
	/** Main generation entry point - generates complete room from RoomData */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void GenerateRoom();

	/** Clears all generated meshes and resets the room */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void CleanupRoom();

	/** Refreshes debug visualization (useful when changing debug settings) */
	UFUNCTION(BlueprintCallable, Category = "Room Generation|Debug")
	void RefreshDebugVisualization();

	/** Returns true if the room has been successfully generated */
	UFUNCTION(BlueprintPure, Category = "Room Generation")
	bool IsRoomGenerated() const { return bIsGenerated; }

	/** Returns the current generation seed being used */
	UFUNCTION(BlueprintPure, Category = "Room Generation")
	int32 GetCurrentSeed() const { return GenerationSeed; }

	/** Returns the number of cells in the runtime grid */
	UFUNCTION(BlueprintPure, Category = "Room Generation")
	int32 GetGridCellCount() const { return RuntimeGrid.Num(); }

protected:
	/** Generates floor tiles with forced placement support */
	void GenerateFloor();

	/** Generates wall segments with forced placement support */
	void GenerateWalls();

	/** Generates ceiling tiles with forced placement support */
	void GenerateCeiling();

	/** Applies forced placements and validates no overlaps */
	bool ApplyForcedPlacements();

	/** Converts grid coordinates to world position */
	FVector GetWorldPositionForCell(const FIntPoint& GridCoord, float ZOffset = 0.0f) const;

	/** Checks if a grid position is valid within the room bounds */
	bool IsValidGridPosition(const FIntPoint& GridCoord) const;

	/** Calculates pivot offset based on EMeshPivotType and cell size */
	FVector CalculatePivotOffset(const FMeshPlacementData& PlacementData) const;

	/** Attempts to place a multi-cell mesh at the specified location */
	bool TryPlaceMultiCellMesh(const FIntPoint& BottomLeftCell, const FMeshPlacementData& PlacementData, USceneComponent* ParentContainer);

	/** Reserves cells for a multi-cell mesh footprint */
	bool ReserveCellsForFootprint(const FIntPoint& BottomLeftCell, int32 FootprintX, int32 FootprintY);

	/** Checks if a footprint would overlap occupied or reserved cells */
	bool CheckFootprintOverlap(const FIntPoint& BottomLeftCell, int32 FootprintX, int32 FootprintY) const;

	/** Initializes the runtime grid based on shape definition */
	void InitializeGrid(const FRoomShapeDefinition& ShapeDefinition);

	/** Gets the current cell size from RoomData's GridConfig */
	float GetCellSize() const;
};
