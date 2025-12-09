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
class UStaticMesh;
class UInstancedStaticMeshComponent;

UCLASS()
class GHCLAUDEDUNGEONGEN_API AMasterRoom : public AActor
{
	GENERATED_BODY()

public:
	AMasterRoom();

	// ========== Configuration Properties ==========
	
	/** Reference to the room data asset that defines this room's configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration")
	TSoftObjectPtr<URoomData> RoomData;

	/** Seed value for room generation randomness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration", meta = (EditCondition = "!bUseRandomSeed"))
	int32 GenerationSeed;

	/** If true, uses a random seed instead of the specified GenerationSeed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration")
	bool bUseRandomSeed;

	/** If true, overrides the shape from RoomData with ShapeOverride */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration")
	bool bOverrideShape;

	/** Shape override to use when bOverrideShape is true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration", meta = (EditCondition = "bOverrideShape"))
	FRoomShapeDefinition ShapeOverride;

	// ========== Forced Placement Configuration ==========
	
	/** Forced floor placements: key is bottom-left cell coordinate, value is mesh placement data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placements")
	TMap<FIntPoint, FMeshPlacementData> ForcedFloorPlacements;

	/** Forced wall placements: key is position and direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placements")
	TMap<FIntPoint, FMeshPlacementData> ForcedWallPlacements;

	/** Forced ceiling placements: key is bottom-left cell coordinate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placements")
	TMap<FIntPoint, FMeshPlacementData> ForcedCeilingPlacements;

	/** Forced doorway placements: key is position and direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placements")
	TMap<FIntPoint, FMeshPlacementData> ForcedDoorwayPlacements;

	// ========== CallInEditor Functions ==========
	
	/** Generate the room using current configuration */
	UFUNCTION(CallInEditor, Category = "Room Generation")
	void GenerateRoom();

	/** Clear all generated content from the room */
	UFUNCTION(CallInEditor, Category = "Room Generation")
	void ClearRoom();

	/** Regenerate the room (clear then generate) */
	UFUNCTION(CallInEditor, Category = "Room Generation")
	void RegenerateRoom();

	/** Update debug visualization (call after generation or when debug settings change) */
	UFUNCTION(CallInEditor, Category = "Room Generation")
	void UpdateDebugVisualization();

	// ========== Runtime State ==========
	
	/** Grid cells representing the room layout */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room State")
	TArray<FGridCell> Grid;

	/** Random stream for deterministic generation */
	UPROPERTY()
	FRandomStream RandomStream;

	/** Whether the room has been generated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room State")
	bool bIsGenerated;

	// ========== Doorway Snap Points ==========
	
	/** World positions of doorway snap points for connecting to other rooms */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Doorways")
	TArray<FVector> DoorwaySnapPoints;

	/** Directions corresponding to each doorway snap point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Doorways")
	TArray<EWallDirection> DoorwayDirections;

	// ========== Components ==========
	
	/** Root component for the entire room hierarchy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RoomRoot;

	/** Container for all floor meshes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> FloorContainer;

	/** Container for all wall meshes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> WallContainer;

	/** Container for all ceiling meshes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> CeilingContainer;

	/** Container for all doorway meshes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DoorwayContainer;

	/** Debug visualization component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDebugHelpers> DebugHelper;

protected:
	// ========== Internal Generation Methods ==========
	
	/** Initialize the grid based on room shape */
	void InitializeGrid();

	/** Apply forced placements and mark occupied cells */
	void ApplyForcedPlacements();

	/** Generate floor tiles using weighted multi-cell placement */
	void GenerateFloorTiles();

	/** Generate wall meshes based on grid configuration */
	void GenerateWalls();

	/** Generate ceiling tiles */
	void GenerateCeiling();

	/** Generate doorway meshes and populate snap points */
	void GenerateDoorways();

	/** Check if a multi-cell placement at the given position fits within available cells */
	bool CanPlaceMeshAt(const FIntPoint& BottomLeft, int32 CellsX, int32 CellsY) const;

	/** Mark cells as occupied for a multi-cell placement */
	void MarkCellsOccupied(const FIntPoint& BottomLeft, int32 CellsX, int32 CellsY);

	/** Spawn a static mesh at the given world position */
	UStaticMeshComponent* SpawnMeshAtPosition(const FVector& WorldPosition, UStaticMesh* Mesh, const FMeshPlacementData& PlacementData, USceneComponent* ParentComponent);

	/** Calculate world position for a cell coordinate considering pivot type */
	FVector CalculateWorldPositionForCell(const FIntPoint& CellCoord, const FMeshPlacementData& PlacementData) const;

	/** Get the active room shape (from override or RoomData) */
	FRoomShapeDefinition GetActiveRoomShape() const;

	/** Load and validate RoomData asset */
	bool LoadRoomData();
};
