// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/GridTypes.h"
#include "Types/RoomShapeTypes.h"
#include "MasterRoom.generated.h"

// Forward declarations
class URoomData;
class UDebugHelpers;
class USceneComponent;
class UStaticMeshComponent;

/**
 * AMasterRoom - Core room actor for dungeon generation
 * Handles both editor-time preview and runtime generation of rooms
 * Supports forced placement workflow for testing and design iteration
 */
UCLASS()
class GHCLAUDEDUNGEONGEN_API AMasterRoom : public AActor
{
	GENERATED_BODY()

public:
	AMasterRoom();

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
	TArray<FGridCell> Grid;

	/** Random stream for deterministic generation */
	UPROPERTY()
	FRandomStream RandomStream;

	/** Whether this room has been generated */
	UPROPERTY(BlueprintReadOnly, Category = "Room State")
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
};
