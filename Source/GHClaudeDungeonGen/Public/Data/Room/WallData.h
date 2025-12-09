// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/GridTypes.h"
#include "WallData.generated.h"

// Forward declaration
class UMaterialInterface;

/**
 * Data asset for wall segment configurations
 * Contains mesh placement data for walls, corners, and doorway frames
 */
UCLASS(BlueprintType)
class GHCLAUDEDUNGEONGEN_API UWallData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Identifying name for this asset pack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Data")
	FName AssetPackName;

	/** Array of wall segment meshes (straight wall sections) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Data|Segments")
	TArray<FMeshPlacementData> WallSegments;

	/** Array of inner corner meshes (concave corners) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Data|Corners")
	TArray<FMeshPlacementData> InnerCorners;

	/** Array of outer corner meshes (convex corners) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Data|Corners")
	TArray<FMeshPlacementData> OuterCorners;

	/** Array of doorway frame meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Data|Doorways")
	TArray<FMeshPlacementData> DoorwayFrames;

	/** Default material to apply to wall meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Data|Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultMaterial;

	UWallData()
		: AssetPackName(NAME_None)
		, WallSegments()
		, InnerCorners()
		, OuterCorners()
		, DoorwayFrames()
		, DefaultMaterial(nullptr)
	{
	}
};
