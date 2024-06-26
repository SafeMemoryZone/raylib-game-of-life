#include "raylib.h"
#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <utility>

constexpr int kCellSize = 10;
constexpr int kRowCount = 50;
constexpr int kColCount = 100;
constexpr Color kCellCol = Color{64, 224, 208, 255};

Camera2D camera{};
bool game_paused;
std::array<std::array<bool, kColCount>, kRowCount> cells{};
float curr_time;
float tick_delay;
std::string curr_status;

std::string FloatToStr(float value) {
  std::ostringstream oss;
  oss << std::fixed << value;
  std::string str = oss.str();

  str.erase(str.find_last_not_of('0') + 1, std::string::npos);

  if (str.back() == '.') {
    str.pop_back();
  }

  return str;
}

int CountNeighbours(int row_idx, int col_idx) {
  static std::array<std::pair<int, int>, 8> neighbour_offs = {
      std::make_pair(-1, -1), std::make_pair(-1, 0), std::make_pair(-1, 1),
      std::make_pair(0, 1),   std::make_pair(1, 1),  std::make_pair(1, 0),
      std::make_pair(1, -1),  std::make_pair(0, -1)};

  int n_count = 0;

  for (const auto &[row_off, col_off] : neighbour_offs) {
    int new_row_idx = row_idx + row_off;
    int new_col_idx = col_idx + col_off;
    if (new_row_idx < 0 || new_row_idx >= kRowCount)
      continue;
    if (new_col_idx < 0 || new_col_idx >= kColCount)
      continue;

    if (cells.at(new_row_idx).at(new_col_idx))
      n_count++;
  }

  return n_count;
}

void RenderFrame() {
  BeginDrawing();
  BeginMode2D(camera);
  ClearBackground(BLACK);

  int row_idx = 0;
  for (const auto &row : cells) {
    int cell_idx = 0;
    for (const auto &cell : row) {
      if (cell)
        DrawRectangle(cell_idx * kCellSize, row_idx * kCellSize, kCellSize,
                      kCellSize, kCellCol);
      cell_idx++;
    }
    row_idx++;
  }

  for (int curr_row_idx = 0; curr_row_idx < kRowCount; curr_row_idx++) {
    for (int curr_col_idx = 0; curr_col_idx < kColCount; curr_col_idx++) {
      DrawLine(curr_col_idx * kCellSize, curr_row_idx * kCellSize,
               curr_col_idx * kCellSize, kRowCount * kCellSize, WHITE);
    }
    DrawLine(0, curr_row_idx * kCellSize, kColCount * kCellSize,
             curr_row_idx * kCellSize, WHITE);
  }

  EndMode2D();
  int text_size = MeasureText(curr_status.c_str(), 80);
  DrawText(curr_status.c_str(), kColCount * kCellSize - text_size - 10,
           kRowCount * kCellSize - 80, 80, GREEN);
  EndDrawing();
}

void Update() {
  if (IsKeyPressed(KEY_SPACE)) {
    game_paused = !game_paused;
  }

  curr_status = game_paused ? "PAUSED" : "x" + FloatToStr(0.5 / tick_delay);

  if (IsKeyPressed(KEY_C)) {
    for (auto &row : cells) {
      row.fill(false);
    }
    game_paused = true;
  }

  if (IsKeyDown(KEY_W)) {
    camera.target.y -= 3;
  }

  if (IsKeyDown(KEY_A)) {
    camera.target.x -= 3;
  }

  if (IsKeyDown(KEY_S)) {
    camera.target.y += 3;
  }

  if (IsKeyDown(KEY_D)) {
    camera.target.x += 3;
  }

  if (auto scroll = GetMouseWheelMove()) {
    float new_zoom = camera.zoom + scroll * 0.1;
    if (new_zoom > 1.0f && new_zoom < 10.0f) {
      Vector2 mouse_pos = GetMousePosition();
      Vector2 world_pos_before = GetScreenToWorld2D(mouse_pos, camera);

      camera.zoom = new_zoom;

      Vector2 world_pos_after = GetScreenToWorld2D(mouse_pos, camera);
      camera.target.x += (world_pos_before.x - world_pos_after.x);
      camera.target.y += (world_pos_before.y - world_pos_after.y);
    }
  }

  if (game_paused) {
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        !IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
      return;

    Vector2 pos = GetScreenToWorld2D(GetMousePosition(), camera);
    int row_idx = floor(pos.y / kCellSize);
    int col_idx = floor(pos.x / kCellSize);

    if (row_idx >= kRowCount || col_idx >= kColCount)
      return;

    cells.at(row_idx).at(col_idx) =
        IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? true : false;

    return;
  }

  if (IsKeyPressed(KEY_RIGHT)) {
    tick_delay /= 2;
  }

  else if (IsKeyPressed(KEY_LEFT)) {
    tick_delay *= 2;
  }

  if (curr_time < tick_delay) {
    curr_time += GetFrameTime();
    return;
  }

  curr_time -= tick_delay;
  auto updated_cells = cells;

  for (int curr_row_idx = 0; curr_row_idx < kRowCount; curr_row_idx++) {
    for (int curr_col_idx = 0; curr_col_idx < kColCount; curr_col_idx++) {
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

void InitGame() {
  game_paused = true;
  curr_time = 0;
  tick_delay = 0.5;
  curr_status = "PAUSED";
  camera.zoom = 1.0;
}

int main(void) {
  InitWindow(kColCount * kCellSize, kRowCount * kCellSize, "Game of Life");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
  InitGame();

  while (!WindowShouldClose()) {
    Update();
    RenderFrame();
  }

  CloseWindow();

  return 0;
}
