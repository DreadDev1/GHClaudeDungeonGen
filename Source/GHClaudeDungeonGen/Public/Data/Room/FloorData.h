// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/GridTypes.h"
#include "FloorData.generated.h"

// Forward declaration
class UMaterialInterface;

/**
 * Data asset for floor tile configurations
 * Contains mesh placement data and material information for dungeon floors
 */
UCLASS(BlueprintType)
class GHCLAUDEDUNGEONGEN_API UFloorData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Identifying name for this asset pack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Data")
	FName AssetPackName;

	/** Array of floor tile meshes with placement data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Data")
	TArray<FMeshPlacementData> FloorTiles;

	/** Default material to apply to floor tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Data|Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultMaterial;

	/** Array of material variations for randomization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Data|Materials")
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

	/** If true, randomly applies different materials from MaterialVariations array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Data|Materials")
	bool bRandomizeMaterials;

	UFloorData()
		: AssetPackName(NAME_None)
		, FloorTiles()
		, DefaultMaterial(nullptr)
		, MaterialVariations()
		, bRandomizeMaterials(false)
	{
	}
};
