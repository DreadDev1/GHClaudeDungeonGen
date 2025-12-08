// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/GridTypes.h"
#include "Types/RoomShapeTypes.h"
#include "RoomData.generated.h"

// Forward declarations
class UFloorData;
class UWallData;
class UDoorData;
class UCeilingData;

/**
 * Complete room definition data asset
 * Defines room identity, grid configuration, shape, dimensions, and compositional references
 */
UCLASS(BlueprintType)
class GHCLAUDEDUNGEONGEN_API URoomData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ========== Room Identity ==========
	
	/** Unique name identifier for this room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Identity")
	FName RoomName;

	/** Descriptive text for this room (used in UI, tooltips, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Identity")
	FText RoomDescription;

	// ========== Grid Configuration ==========
	
	/** Grid configuration settings for this room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Configuration")
	FGridConfiguration GridConfig;

	// ========== Shape Configuration ==========
	
	/** Array of allowed shapes for this room type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Configuration")
	TArray<FRoomShapeDefinition> AllowedShapes;

	/** Minimum dimensions for this room (X, Y in cells) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Configuration", meta = (ClampMin = "1", ClampMax = "50"))
	FIntPoint MinDimensions;

	/** Maximum dimensions for this room (X, Y in cells) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Configuration", meta = (ClampMin = "1", ClampMax = "50"))
	FIntPoint MaxDimensions;

	// ========== Compositional References ==========
	
	/** Reference to floor data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositional References")
	TSoftObjectPtr<UFloorData> FloorData;

	/** Reference to wall data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositional References")
	TSoftObjectPtr<UWallData> WallData;

	/** Reference to door data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositional References")
	TSoftObjectPtr<UDoorData> DoorData;

	/** Reference to ceiling data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compositional References")
	TSoftObjectPtr<UCeilingData> CeilingData;

	// ========== Generation Settings ==========
	
	/** Minimum number of doorways this room must have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings", meta = (ClampMin = "0", ClampMax = "10"))
	int32 MinDoorways;

	/** Maximum number of doorways this room can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxDoorways;

	/** Weight for random room selection (higher = more likely to be chosen) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RoomSelectionWeight;

	// ========== Room Flags ==========
	
	/** If true, this room can be used as an entry room for the dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Flags")
	bool bCanBeEntryRoom;

	/** If true, this room can be used as an exit room for the dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Flags")
	bool bCanBeExitRoom;

	URoomData()
		: RoomName(NAME_None)
		, RoomDescription(FText::FromString("Default Room"))
		, GridConfig()
		, AllowedShapes()
		, MinDimensions(3, 3)
		, MaxDimensions(10, 10)
		, FloorData(nullptr)
		, WallData(nullptr)
		, DoorData(nullptr)
		, CeilingData(nullptr)
		, MinDoorways(1)
		, MaxDoorways(4)
		, RoomSelectionWeight(1.0f)
		, bCanBeEntryRoom(true)
		, bCanBeExitRoom(true)
	{
	}
};
