// Fill out your copyright notice in the Description page of Project Settings.


#include "Debugging/DebugHelpers.h"

UDebugHelpers::UDebugHelpers()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
void UDebugHelpers::DrawGridCreated(const FVector& CellCenter, float CellSize, float Duration)
{
	if (!bShowGridCreated) return;
	DrawCellBox(CellCenter, CellSize, FColor::Green, Duration);
}

void UDebugHelpers::DrawUnoccupiedCell(const FVector& CellCenter, float CellSize, float Duration)
{
	if (!bShowUnoccupiedCells) return;
	DrawCellBox(CellCenter, CellSize, FColor::Blue, Duration);
}

void UDebugHelpers::DrawOccupiedCell(const FVector& CellCenter, float CellSize, float Duration)
{
	if (!bShowOccupiedCells) return;
	DrawCellBox(CellCenter, CellSize, FColor::Red, Duration);
}

void UDebugHelpers::DrawCellBox(const FVector& CellCenter, float CellSize, const FColor& Color, float Duration)
{
	if (!GetWorld()) return;

	const float HalfSize = CellSize * 0.5f;
	const FVector Extent(HalfSize, HalfSize, 1.0f); // Flat box for floor grid

	DrawDebugBox(
		GetWorld(),
		CellCenter,
		Extent,
		Color,
		false,      // bPersistent
		Duration,
		0,          // DepthPriority
		2.0f        // Thickness
	);
}
#endif