// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomShapeTypes.generated.h"

/**
 * Enum defining standard room shapes for procedural generation
 */
UENUM(BlueprintType)
enum class ERoomShape : uint8
{
	/** Standard rectangular room */
	Rectangle UMETA(DisplayName = "Rectangle"),
	
	/** L-shaped room with one extension */
	LShape UMETA(DisplayName = "L-Shape"),
	
	/** T-shaped room with two extensions */
	TShape UMETA(DisplayName = "T-Shape"),
	
	/** U-shaped room with three extensions */
	UShape UMETA(DisplayName = "U-Shape"),
	
	/** Custom shape defined by cell array */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Struct defining a room shape configuration
 * Can represent standard shapes (Rectangle, L, T, U) or custom layouts
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FRoomShapeDefinition
{
	GENERATED_BODY()

	/** The type of room shape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Shape")
	ERoomShape ShapeType;

	/** Width of the rectangular base (used for Rectangle, L, T, U shapes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Shape", meta = (ClampMin = "1", ClampMax = "50"))
	int32 RectWidth;

	/** Height of the rectangular base (used for Rectangle, L, T, U shapes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Shape", meta = (ClampMin = "1", ClampMax = "50"))
	int32 RectHeight;

	/** Custom cell layout array (1 = occupied, 0 = empty). Used only when ShapeType is Custom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Shape|Custom", meta = (EditCondition = "ShapeType == ERoomShape::Custom"))
	TArray<int32> CustomCellLayout;

	/** Width of the custom layout grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Shape|Custom", meta = (ClampMin = "1", ClampMax = "50", EditCondition = "ShapeType == ERoomShape::Custom"))
	int32 CustomLayoutWidth;

	/** Height of the custom layout grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Shape|Custom", meta = (ClampMin = "1", ClampMax = "50", EditCondition = "ShapeType == ERoomShape::Custom"))
	int32 CustomLayoutHeight;

	FRoomShapeDefinition()
		: ShapeType(ERoomShape::Rectangle)
		, RectWidth(5)
		, RectHeight(5)
		, CustomCellLayout()
		, CustomLayoutWidth(5)
		, CustomLayoutHeight(5)
	{
	}
};

/**
 * Struct defining a template for complex room shapes (L, T, U)
 * Defines main section dimensions and extension properties
 */
USTRUCT(BlueprintType)
struct GHCLAUDEDUNGEONGEN_API FShapeTemplate
{
	GENERATED_BODY()

	/** Width of the main section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Template", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MainSectionWidth;

	/** Height of the main section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Template", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MainSectionHeight;

	/** Width of the extension section(s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Template", meta = (ClampMin = "1", ClampMax = "50"))
	int32 ExtensionWidth;

	/** Height of the extension section(s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Template", meta = (ClampMin = "1", ClampMax = "50"))
	int32 ExtensionHeight;

	/** 
	 * Extension attach point: 0 = start, 1 = middle, 2 = end
	 * Determines where the extension connects to the main section
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape Template", meta = (ClampMin = "0", ClampMax = "2"))
	int32 ExtensionAttachPoint;

	FShapeTemplate()
		: MainSectionWidth(5)
		, MainSectionHeight(5)
		, ExtensionWidth(3)
		, ExtensionHeight(3)
		, ExtensionAttachPoint(1)
	{
	}
};
