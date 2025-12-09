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
class URoomData;
class UDebugHelpers;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Master room actor that handles room generation and management
 * Supports both editor-time and runtime generation with forced placement workflow
 */
 * AMasterRoom - Core room actor for dungeon generation
 * Handles both editor-time preview and runtime generation of rooms
 * Supports forced placement workflow for testing and design iteration
 */
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
	// ========== Configuration Properties ==========
	
	/** Reference to the room data asset defining this room's configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration")
	TSoftObjectPtr<URoomData> RoomData;

	/** Seed for random generation of this room */
	// ========== Room Data Configuration ==========
	
	/** Reference to the room data asset that defines this room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration", meta = (AllowedClasses = "/Script/GHClaudeDungeonGen.RoomData"))
	TSoftObjectPtr<URoomData> RoomData;

	/** Seed for this room's generation (used if bUseRandomSeed is false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration|Generation", meta = (EditCondition = "!bUseRandomSeed"))
	int32 GenerationSeed;

	/** If true, uses a random seed each time instead of GenerationSeed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration|Generation")
	bool bUseRandomSeed;

	/** If true, overrides the shape from RoomData with ShapeOverride */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration|Shape")
	bool bOverrideShape;

	/** Shape override (only used if bOverrideShape is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration|Shape", meta = (EditCondition = "bOverrideShape"))
	FRoomShapeDefinition ShapeOverride;

	// ========== Runtime Generation State ==========
	
	/** Grid cells for this room (populated during generation) */
	UPROPERTY(BlueprintReadOnly, Category = "Room State")
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

	// ========== Runtime State ==========
	
	/** Grid cells representing the room's layout */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room State")
	TArray<FGridCell> Grid;

	/** Random stream for reproducible generation */
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

	/** Whether this room has been generated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room State")
	bool bIsGenerated;

	// ========== Doorway Management ==========
	
	/** World space positions of doorway snap points for connecting to other rooms */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Doorways")
	TArray<FVector> DoorwaySnapPoints;

	/** Directions of doorways (North, South, East, West) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Doorways")
	TArray<EWallDirection> DoorwayDirections;

	// ========== Component Hierarchy ==========
	
	/** Root scene component for the entire room */
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

	/** Debug helper component for visualization */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDebugHelpers> DebugHelper;

	// ========== Forced Placement System ==========
	
	/** Map of forced prop placements (cell coordinates -> actor class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placement")
	TMap<FIntPoint, TSubclassOf<AActor>> ForcedPropPlacements;

	/** Map of forced furniture placements (cell coordinates -> actor class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placement")
	TMap<FIntPoint, TSubclassOf<AActor>> ForcedFurniturePlacements;

	/** Map of forced enemy spawn placements (cell coordinates -> actor class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placement")
	TMap<FIntPoint, TSubclassOf<AActor>> ForcedEnemyPlacements;

	// ========== Generation Methods ==========
	
	/** Generate the room using RoomData configuration */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void GenerateRoom();

	/** Clear all generated room content */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void ClearRoom();

	/** Regenerate room with new seed */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void RegenerateRoom(int32 NewSeed);

	/** Apply forced placements to the room */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void ApplyForcedPlacements();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** Initialize component hierarchy */
	void InitializeComponents();

	/** Generate grid based on shape definition */
	void GenerateGrid(const FRoomShapeDefinition& Shape);

	/** Spawn floor meshes */
	void SpawnFloorMeshes();

	/** Spawn wall meshes */
	void SpawnWallMeshes();

	/** Spawn ceiling meshes */
	void SpawnCeilingMeshes();

	/** Spawn doorway meshes */
	void SpawnDoorwayMeshes();

	/** Calculate doorway snap points from grid */
	void CalculateDoorwaySnapPoints();
	UPROPERTY(BlueprintReadOnly, Category = "Room State")
	/** Whether the room has been generated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room State")
	bool bIsGenerated;

	// ========== Doorway Snap Points ==========
	
	/** World-space locations for doorway snap points (used for room-to-room connections) */
	UPROPERTY(BlueprintReadOnly, Category = "Room State|Doorways")
	TArray<FVector> DoorwaySnapPoints;

	/** Directions for each doorway snap point (North, East, South, West) */
	UPROPERTY(BlueprintReadOnly, Category = "Room State|Doorways")
	TArray<EWallDirection> DoorwayDirections;

	// ========== Scene Components ==========
	
	/** Root component for the room */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Components")
	TObjectPtr<USceneComponent> RoomRoot;

	/** Container for all floor mesh components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Components")
	TObjectPtr<USceneComponent> FloorContainer;

	/** Container for all wall mesh components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Components")
	TObjectPtr<USceneComponent> WallContainer;

	/** Container for all ceiling mesh components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Components")
	TObjectPtr<USceneComponent> CeilingContainer;

	/** Container for all doorway mesh components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Components")
	TObjectPtr<USceneComponent> DoorwayContainer;

	/** Debug helper component for visualization */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Components")
	TObjectPtr<UDebugHelpers> DebugHelper;

	// ========== Forced Placement Maps ==========
	
	/** Map of grid coordinates to forced floor mesh indices (for designer control) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placement", AdvancedDisplay)
	TMap<FIntPoint, int32> ForcedFloorPlacements;

	/** Map of grid coordinates to forced wall mesh indices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placement", AdvancedDisplay)
	TMap<FIntPoint, int32> ForcedWallPlacements;

	/** Map of grid coordinates to forced ceiling mesh indices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placement", AdvancedDisplay)
	TMap<FIntPoint, int32> ForcedCeilingPlacements;

	/** Map of grid coordinates to forced doorway mesh indices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Placement", AdvancedDisplay)
	TMap<FIntPoint, int32> ForcedDoorwayPlacements;

	// ========== Generation Methods ==========
	
	/** Generate or regenerate the room based on current settings */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void GenerateRoom();

	/** Clear all generated geometry and reset state */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void ClearRoom();

	/** Regenerate room with a new random seed */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	void RegenerateWithNewSeed();

	/** Get the cell size from the RoomData (or default if not set) */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	float GetCellSize() const;

	/** Get the shape definition to use (either from RoomData or override) */
	UFUNCTION(BlueprintCallable, Category = "Room Generation")
	FRoomShapeDefinition GetActiveShapeDefinition() const;

	// ========== Editor Support ==========
	
#if WITH_EDITOR
	/** Called when properties change in editor (for live preview) */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	/** Called when actor is constructed in editor (for preview) */
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

private:
	// ========== Internal Generation Methods ==========
	
	/** Initialize the grid based on shape definition */
	void InitializeGrid();

	/** Generate floor meshes for all cells */
	void GenerateFloorMeshes();

	/** Generate wall meshes for all cells */
	void GenerateWallMeshes();

	/** Generate ceiling meshes for all cells */
	void GenerateCeilingMeshes();

	/** Generate doorway meshes and snap points */
	void GenerateDoorways();

	/** Spawn a static mesh component in the specified container */
	UStaticMeshComponent* SpawnMeshComponent(USceneComponent* Parent, UStaticMesh* Mesh, const FTransform& Transform);

	/** Calculate world position for a grid coordinate */
	FVector GridToWorldPosition(const FIntPoint& GridCoord) const;

	/** Get the effective seed to use for generation */
	int32 GetEffectiveSeed() const;
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
