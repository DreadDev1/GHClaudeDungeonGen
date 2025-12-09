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
 * Master room actor that handles room generation and management
 * Supports both editor-time and runtime generation with forced placement workflow
 */
UCLASS()
class GHCLAUDEDUNGEONGEN_API AMasterRoom : public AActor
{
	GENERATED_BODY()

public:
	AMasterRoom();

	// ========== Configuration Properties ==========
	
	/** Reference to the room data asset defining this room's configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Configuration")
	TSoftObjectPtr<URoomData> RoomData;

	/** Seed for random generation of this room */
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
};
