// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/GridTypes.h"
#include "DoorData.generated.h"

// Forward declarations
class UMaterialInterface;
class AActor;

/**
 * Data asset for door and doorway configurations
 * Contains mesh placement data for doorways and interactive door actors
 */
UCLASS(BlueprintType)
class GHCLAUDEDUNGEONGEN_API UDoorData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Identifying name for this asset pack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data")
	FName AssetPackName;

	/** Array of doorway opening meshes (archways, frames without doors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data|Meshes")
	TArray<FMeshPlacementData> DoorwayMeshes;

	/** Array of door meshes (actual door objects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data|Meshes")
	TArray<FMeshPlacementData> DoorMeshes;

	/** Actor class for interactive doorways (future use with ADungeonDoorway) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data|Interaction")
	TSubclassOf<AActor> DoorwayActorClass;

	/** Extent of the interaction volume for door triggers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data|Interaction")
	FVector InteractionVolumeExtent;

	/** Default material to apply to door meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data|Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultMaterial;

	UDoorData()
		: AssetPackName(NAME_None)
		, DoorwayMeshes()
		, DoorMeshes()
		, DoorwayActorClass(nullptr)
		, InteractionVolumeExtent(FVector(100.0f, 100.0f, 250.0f))
		, DefaultMaterial(nullptr)
	{
	}
};
