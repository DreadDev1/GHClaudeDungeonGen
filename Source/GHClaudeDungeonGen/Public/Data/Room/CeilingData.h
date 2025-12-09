// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/GridTypes.h"
#include "CeilingData.generated.h"

// Forward declaration
class UMaterialInterface;

/**
 * Data asset for ceiling tile configurations
 * Contains mesh placement data and material information for dungeon ceilings
 */
UCLASS(BlueprintType)
class GHCLAUDEDUNGEONGEN_API UCeilingData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Identifying name for this asset pack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ceiling Data")
	FName AssetPackName;

	/** Array of ceiling tile meshes with placement data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ceiling Data")
	TArray<FMeshPlacementData> CeilingTiles;

	/** Height offset from floor level to ceiling (default 300.0 = 3 meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ceiling Data", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float CeilingHeightOffset;

	/** Default material to apply to ceiling tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ceiling Data|Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultMaterial;

	UCeilingData()
		: AssetPackName(NAME_None)
		, CeilingTiles()
		, CeilingHeightOffset(300.0f)
		, DefaultMaterial(nullptr)
	{
	}
};
