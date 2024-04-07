#include "raylib.h"
#include <array>
#include <cmath>
#include <iostream>
#include <utility>

constexpr int cell_size = 10;
constexpr int row_count = 50;
constexpr int collum_count = 100;
constexpr Color cell_color = Color{64, 224, 208, 255};
bool game_paused = true;
std::array<std::array<bool, collum_count>, row_count> cells;

int CountNeighbours(int row_idx, int col_idx) {
  static std::array<std::pair<int, int>, 8> neighbour_offs = {
      std::make_pair(-1, -1), std::make_pair(-1, 0), std::make_pair(-1, 1),
      std::make_pair(0, 1),   std::make_pair(1, 1),  std::make_pair(1, 0),
      std::make_pair(1, -1),  std::make_pair(0, -1)};

  int n_count = 0;

  for (const auto &[row_off, col_off] : neighbour_offs) {
    int new_row_idx = row_idx + row_off;
    int new_col_idx = col_idx + col_off;
    if (new_row_idx < 0 || new_row_idx >= cells.size())
      continue;
    if (new_col_idx < 0 || new_col_idx >= cells.at(0).size())
      continue;

    if (cells.at(new_row_idx).at(new_col_idx))
      n_count++;
  }

  return n_count;
}

void RenderFrame() {
  BeginDrawing();
  ClearBackground(BLACK);

  int row_idx = 0;
  for (const auto &row : cells) {
    int cell_idx = 0;
    for (const auto &cell : row) {
      if (cell)
        DrawRectangle(cell_idx * cell_size, row_idx * cell_size, cell_size,
                      cell_size, cell_color);
      cell_idx++;
    }
    row_idx++;
  }

  for (int curr_row_idx = 0; curr_row_idx < row_count; curr_row_idx++) {
    for (int curr_col_idx = 0; curr_col_idx < collum_count; curr_col_idx++) {
      DrawLine(curr_col_idx * cell_size, curr_row_idx * cell_size,
               curr_col_idx * cell_size, row_count * cell_size, WHITE);
    }
    DrawLine(0, curr_row_idx * cell_size, collum_count * cell_size,
             curr_row_idx * cell_size, WHITE);
  }
  EndDrawing();
}

void Update() {
  if (IsKeyPressed(KEY_SPACE)) {
    game_paused = !game_paused;
  }

  if (game_paused) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        !IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
      return;

    int row_idx = floor(GetMouseY() / cell_size);
    int col_idx = floor(GetMouseX() / cell_size);

    cells.at(row_idx).at(col_idx) =
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ? true : false;

    return;
  }

  auto updated_cells = cells;

  for (int curr_row_idx = 0; curr_row_idx < row_count; curr_row_idx++) {
    for (int curr_col_idx = 0; curr_col_idx < collum_count; curr_col_idx++) {
      int n_count = CountNeighbours(curr_row_idx, curr_col_idx);
      bool is_alive = cells.at(curr_row_idx).at(curr_col_idx);

      if (is_alive && n_count < 2)
        updated_cells.at(curr_row_idx).at(curr_col_idx) = false;
      if (is_alive && (n_count == 2 || n_count == 3))
        updated_cells.at(curr_row_idx).at(curr_col_idx) = true;
      if (is_alive && n_count > 3)
        updated_cells.at(curr_row_idx).at(curr_col_idx) = false;
      if (!is_alive && n_count == 3)
        updated_cells.at(curr_row_idx).at(curr_col_idx) = true;
    }
  }

  cells = std::move(updated_cells);
}

int main(void) {
  InitWindow(collum_count * cell_size, row_count * cell_size, "Game of Life");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    Update();
    RenderFrame();
  }

  CloseWindow();

  return 0;
}
