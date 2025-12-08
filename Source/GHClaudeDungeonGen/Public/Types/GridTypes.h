// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GridTypes.generated.h"

// Forward declarations
class UStaticMesh;
class AActor;

/**
 * Enum defining pivot point locations for mesh placement
 * Used to ensure proper alignment of floor tiles, walls, and other dungeon components
 */
UENUM(BlueprintType)
enum class EMeshPivotType : uint8
{
	/** Centered on X and Y axes, typically used for floor and ceiling tiles */
	CenterXY UMETA(DisplayName = "Center XY"),
	
	/** Centered on X, back on Y, bottom on Z - used for wall segments and doorway frames */
	BottomBackCenter UMETA(DisplayName = "Bottom Back Center"),
	
	/** Centered on X and Y, bottom on Z */
	BottomCenter UMETA(DisplayName = "Bottom Center"),
	
	/** Custom pivot point defined by user offset */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Enum representing the state of a cell in the dungeon grid
 */
UENUM(BlueprintType)
enum class ECellState : uint8
{
	/** Cell is empty and available for placement */
	Unoccupied UMETA(DisplayName = "Unoccupied"),
	
	/** Cell is occupied by a room or structure */
	Occupied UMETA(DisplayName = "Occupied"),
	
	/** Cell is reserved for future use (e.g., hallway planning) */
	Reserved UMETA(DisplayName = "Reserved"),
	
	/** Cell is excluded from generation (e.g., obstacles, predefined areas) */
	Excluded UMETA(DisplayName = "Excluded")
};

/**
 * Enum defining cardinal directions for walls
 * North = +Y, East = +X, South = -Y, West = -X (Unreal Engine coordinate system)
 */
UENUM(BlueprintType)
enum class EWallDirection : uint8
{
	/** Positive Y direction */
	North UMETA(DisplayName = "North (+Y)"),
	
	/** Positive X direction */
	East UMETA(DisplayName = "East (+X)"),
	
	/** Negative Y direction */
	South UMETA(DisplayName = "South (-Y)"),
	
	/** Negative X direction */
	West UMETA(DisplayName = "West (-X)")
};

/**
 * Struct representing a single cell in the dungeon grid
 * Contains all information about the cell's state, position, walls, and occupancy
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FGridCell
{
	GENERATED_BODY()

	/** Grid coordinates (X, Y) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell")
	FIntPoint GridCoordinates;

	/** Current state of this cell */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell")
	ECellState CellState;

	/** World space position of this cell's center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell")
	FVector WorldPosition;

	/** Whether this cell has a wall on the north edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Walls")
	bool bHasNorthWall;

	/** Whether this cell has a wall on the east edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Walls")
	bool bHasEastWall;

	/** Whether this cell has a wall on the south edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Walls")
	bool bHasSouthWall;

	/** Whether this cell has a wall on the west edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Walls")
	bool bHasWestWall;

	/** Whether this cell has a doorway on the north edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Doorways")
	bool bHasNorthDoorway;

	/** Whether this cell has a doorway on the east edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Doorways")
	bool bHasEastDoorway;

	/** Whether this cell has a doorway on the south edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Doorways")
	bool bHasSouthDoorway;

	/** Whether this cell has a doorway on the west edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Cell|Doorways")
	bool bHasWestDoorway;

	/** Weak pointer to the actor currently occupying this cell */
	UPROPERTY()
	TWeakObjectPtr<AActor> OccupyingActor;

	FGridCell()
		: GridCoordinates(0, 0)
		, CellState(ECellState::Unoccupied)
		, WorldPosition(FVector::ZeroVector)
		, bHasNorthWall(false)
		, bHasEastWall(false)
		, bHasSouthWall(false)
		, bHasWestWall(false)
		, bHasNorthDoorway(false)
		, bHasEastDoorway(false)
		, bHasSouthDoorway(false)
		, bHasWestDoorway(false)
		, OccupyingActor(nullptr)
	{
	}
};

/**
 * Struct containing configuration data for the dungeon grid
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FGridConfiguration
{
	GENERATED_BODY()

	/** Size of each cell in world units (default 100.0 = 1 meter in UE) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Configuration", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float CellSize;

	/** Reference to shape definition for this grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Configuration")
	FName ShapeDefinitionName;

	/** Seed for random generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Configuration")
	int32 GenerationSeed;

	/** If true, uses a random seed instead of the specified GenerationSeed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Configuration")
	bool bUseRandomSeed;

	FGridConfiguration()
		: CellSize(100.0f)
		, ShapeDefinitionName(NAME_None)
		, GenerationSeed(0)
		, bUseRandomSeed(true)
	{
	}
};

/**
 * Struct defining placement data for static mesh assets
 * Used for floor tiles, wall segments, ceiling tiles, etc.
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FMeshPlacementData
{
	GENERATED_BODY()

	/** Soft reference to the static mesh asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Pivot point type for this mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement")
	EMeshPivotType PivotType;

	/** Custom pivot offset (only used when PivotType is Custom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement", meta = (EditCondition = "PivotType == EMeshPivotType::Custom"))
	FVector CustomPivotOffset;

	/** Number of cells this mesh occupies in the X direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement", meta = (ClampMin = "1", ClampMax = "10"))
	int32 CellsX;

	/** Number of cells this mesh occupies in the Y direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement", meta = (ClampMin = "1", ClampMax = "10"))
	int32 CellsY;

	/** Weight for random selection (higher = more likely to be chosen) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float SelectionWeight;

	/** If true, allows this mesh to be rotated in 90-degree increments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement")
	bool bAllowRotation;

	/** If true and rotation is allowed, allows 180-degree rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Placement", meta = (EditCondition = "bAllowRotation"))
	bool bAllow180Rotation;

	FMeshPlacementData()
		: Mesh(nullptr)
		, PivotType(EMeshPivotType::CenterXY)
		, CustomPivotOffset(FVector::ZeroVector)
		, CellsX(1)
		, CellsY(1)
		, SelectionWeight(1.0f)
		, bAllowRotation(true)
		, bAllow180Rotation(true)
	{
	}
};
