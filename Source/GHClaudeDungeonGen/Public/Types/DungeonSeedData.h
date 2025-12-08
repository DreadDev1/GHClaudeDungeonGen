// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonSeedData.generated.h"

/**
 * Struct containing seed data for a single room
 * Used for saving and loading room configurations
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FRoomSeedData
{
	GENERATED_BODY()

	/** Unique seed for this room's generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Seed Data")
	int32 RoomSeed;

	/** Grid location of the room (X, Y coordinates) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Seed Data")
	FIntPoint Location;

	/** Rotation of the room in degrees (0, 90, 180, 270) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Seed Data")
	int32 Rotation;

	/** Name/path of the room data asset used for this room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Seed Data")
	FName RoomDataAssetName;

	FRoomSeedData()
		: RoomSeed(0)
		, Location(0, 0)
		, Rotation(0)
		, RoomDataAssetName(NAME_None)
	{
	}
};

/**
 * Struct containing seed data for a single floor/level
 * Contains all room, hallway, and doorway configurations for the floor
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FFloorSeedData
{
	GENERATED_BODY()

	/** Index of this floor (0 = ground floor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Seed Data")
	int32 FloorIndex;

	/** Unique seed for this floor's generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Seed Data")
	int32 FloorSeed;

	/** Array of room seed data for all rooms on this floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Seed Data")
	TArray<FRoomSeedData> RoomSeeds;

	/** Array of hallway seed data for this floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Seed Data")
	TArray<FRoomSeedData> HallwaySeeds;

	/** Array of doorway positions and configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Seed Data")
	TArray<FIntPoint> DoorwayPositions;

	FFloorSeedData()
		: FloorIndex(0)
		, FloorSeed(0)
		, RoomSeeds()
		, HallwaySeeds()
		, DoorwayPositions()
	{
	}
};

/**
 * Struct containing complete dungeon generation seed data
 * Used for saving and loading entire dungeon configurations
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FDungeonSeedData
{
	GENERATED_BODY()

	/** Master seed for the entire dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Seed Data")
	int32 MasterSeed;

	/** Array of floor seed data for all floors in the dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Seed Data")
	TArray<FFloorSeedData> FloorSeeds;

	/** Timestamp when this seed data was generated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Seed Data")
	FDateTime GenerationTimestamp;

	/** Version number for save compatibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Seed Data")
	int32 SaveVersion;

	FDungeonSeedData()
		: MasterSeed(0)
		, FloorSeeds()
		, GenerationTimestamp(FDateTime::Now())
		, SaveVersion(1)
	{
	}
};
