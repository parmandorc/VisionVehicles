// Fill out your copyright notice in the Description page of Project Settings.

#include "VisionVehicles.h"
#include "VisionVehiclesAIController.h"

bool start_point_found;
bool end_point_found;

FVector2D start_point;
FVector2D end_point;

FVector2D AVisionVehiclesAIController::FindTrackEnd(TArray<bool> cameraFeed, int size_y, int size_x)
{
     if (cameraFeed.Num() != size_y*size_x)
          return FVector2D(-1, -1);
     start_point_found = false;
     end_point_found = false;

     //go along the left border: (0, Y) -> (0, 0)
     for (int y = size_y - 1; y >= 0; y--)
     {
          if (cameraFeed[y*size_x] == true && !start_point_found)
          {
               start_point_found = true;
               start_point = FVector2D(0.f, y);
          }
          if (cameraFeed[y*size_x] == false && !end_point_found && start_point_found)
          {
               end_point_found = true;
               end_point = FVector2D(0.f, y - 1);
          }
     }

     //go along the top border: (0, 0) -> (X, 0)
     if (!end_point_found)
     {
          for (int x = 0; x < size_x; x++)
          {
               if (cameraFeed[x] == true && !start_point_found)
               {
                    start_point_found = true;
                    start_point = FVector2D(x, 0.f);
               }
               if (cameraFeed[x] == false && !end_point_found && start_point_found)
               {
                    end_point_found = true;
                    end_point = FVector2D(x - 1, 0.f);
               }
          }

          //go along the right border: (X, 0) -> (X, Y)
          if (!end_point_found)
          {
               for (int y = 0; y < size_y; y++)
               {
                    if (cameraFeed[(y + 1)*size_x - 1] == true && !start_point_found)
                    {
                         start_point_found = true;
                         start_point = FVector2D(size_x - 1, y);
                    }
                    if (cameraFeed[(y + 1)*size_x - 1] == false && !end_point_found && start_point_found)
                    {
                         end_point_found = true;
                         end_point = FVector2D(size_x - 1, y - 1);
                    }
               }
          }
     }

     if (end_point_found)
     {
          return FVector2D(
               int((start_point.X + end_point.X) / 2),
               int((start_point.Y + end_point.Y) / 2));
     }
     return FVector2D(-1, -1);
}
