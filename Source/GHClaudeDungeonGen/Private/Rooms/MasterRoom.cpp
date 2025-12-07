// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/MasterRoom.h"

#include "Debugging/DebugHelpers.h"

AMasterRoom::AMasterRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	DebugHelpers = CreateDefaultSubobject<UDebugHelpers>(TEXT("DebugHelpers"));
}

void AMasterRoom::InitializeGrid()
{
	const int32 TotalCells = GridSizeX * GridSizeY;
	GridCells.SetNum(TotalCells);

	for (int32 i = 0; i < TotalCells; i++)
	{
		GridCells[i] = false; // All cells start unoccupied

#if WITH_EDITORONLY_DATA
		if (DebugHelpers)
		{
			DebugHelpers->DrawGridCreated(GetCellWorldPosition(i), CellSize);
		}
#endif
	}
}

FVector AMasterRoom::GetCellWorldPosition(int32 CellIndex) const
{
	if (!IsValidCellIndex(CellIndex)) return FVector::ZeroVector;

	const int32 X = CellIndex % GridSizeX;
	const int32 Y = CellIndex / GridSizeX;

	const FVector LocalOffset(
		(X * CellSize) + (CellSize * 0.5f),
		(Y * CellSize) + (CellSize * 0.5f),
		0.0f
	);

	return GetActorLocation() + LocalOffset;
}

bool AMasterRoom::IsValidCellIndex(int32 CellIndex) const
{
	return CellIndex >= 0 && CellIndex < GridCells.Num();
}