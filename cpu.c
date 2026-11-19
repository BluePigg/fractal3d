#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <string.h>

typedef struct camera {
  Vector3 pos;
  Vector3 forward;
  Vector3 up;
  Vector3 right;
} camera;

int W = 800;
int H = 800;

int changeSpeed = 1;
float arrow_sens_rev = 20.0f;
float sensitivity = 1.5f;
float speed = 0.05f;
float min_dist = 0.1f;

int max_ray_length = 100;
float ray_process_unit = 1.0f;

float maxf(float a, float b) { return (a > b) ? a : b; }

camera *InitCamera() {
  camera *cam = (camera *)malloc(sizeof(camera));

  cam->pos = (Vector3){1.0f, 1.0f, 1.0f};
  cam->forward = (Vector3){1.0f, 0.0f, 0.0f};
  cam->right = (Vector3){0.0f, 0.0f, 1.0f};
  cam->up = (Vector3){0.0f, 1.0f, 0.0f};

  return cam;
}

void rotateCamera(camera *cam, Vector2 delta) {
  cam->forward = Vector3Normalize(Vector3Add(
      cam->forward,
      Vector3Add(
          Vector3Scale(cam->up, (-1 * delta.y / (float)H) * sensitivity),
          Vector3Scale(cam->right, (delta.x / (float)W) * sensitivity))));
  if (cam->forward.y >= 0.99f) {
    cam->forward.y = 0.99f;
  }
  if (cam->forward.y <= -0.99f) {
    cam->forward.y = -0.99f;
  }
  cam->right = Vector3Normalize(
      Vector3CrossProduct(cam->forward, (Vector3){0.0f, 1.0f, 0.0f}));
  cam->up = Vector3CrossProduct(cam->right, cam->forward);
}

int main(int argc, char *argv[]) {
  SetTargetFPS(60);
  camera *cam = InitCamera();

  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(W, H, "Fractal.");

  Shader shader = LoadShader(0, "gpu.frag");
  RenderTexture2D rt = LoadRenderTexture(1, 1);

  int cam_pos_loc = GetShaderLocation(shader, "cam_pos");
  int cam_forward_loc = GetShaderLocation(shader, "cam_forward");
  int cam_right_loc = GetShaderLocation(shader, "cam_right");
  int cam_up_loc = GetShaderLocation(shader, "cam_up");
  int max_ray_length_loc = GetShaderLocation(shader, "max_ray_length");
  int resolution_loc = GetShaderLocation(shader, "resolution");
  int mode_loc = GetShaderLocation(shader, "mode");

  DisableCursor();
  while (!WindowShouldClose()) {
    W = GetScreenWidth();
    H = GetScreenHeight();

    Vector2 mdelta = GetMouseDelta();
    rotateCamera(cam, mdelta);

    int mode = 0;
    rlDisableColorBlend();
    SetShaderValue(shader, mode_loc, &mode, SHADER_UNIFORM_INT);
    SetShaderValue(shader, cam_pos_loc, &(cam->pos), SHADER_UNIFORM_VEC3);
    BeginTextureMode(rt);
    BeginShaderMode(shader);
    DrawRectangle(0, 0, 1, 1, WHITE);
    EndShaderMode();
    EndTextureMode();
    Image image = LoadImageFromTexture(rt.texture);
    Color *colors = LoadImageColors(image);
    unsigned int raw =
        (unsigned int)(colors->r) << 24 | (unsigned int)(colors->g) << 16 |
        (unsigned int)(colors->b) << 8 | (unsigned int)(colors->a);
    float dist;
    memcpy(&dist, &raw, sizeof(float));
    rlEnableColorBlend();

    Vector3 case_forward = Vector3Scale(
        cam->forward, speed * (changeSpeed ? maxf(dist, min_dist) : 1));
    Vector3 case_right = Vector3Scale(
        cam->right, speed * (changeSpeed ? maxf(dist, min_dist) : 1));
    UnloadImage(image);
    UnloadImageColors(colors);
    if (IsKeyDown(KEY_W)) {
      cam->pos = Vector3Add(cam->pos, case_forward);
    }
    if (IsKeyDown(KEY_S)) {
      cam->pos = Vector3Add(cam->pos, Vector3Scale(case_forward, -1));
    }
    if (IsKeyDown(KEY_D)) {
      cam->pos = Vector3Add(cam->pos, case_right);
    }
    if (IsKeyDown(KEY_A)) {
      cam->pos = Vector3Add(cam->pos, Vector3Scale(case_right, -1));
    }

    float right = (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) *
                  ((float)W / arrow_sens_rev);
    float up =
        (IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * ((float)H / arrow_sens_rev);
    rotateCamera(cam, (Vector2){right, up});

    if (IsKeyPressed(KEY_F11)) {
      ToggleFullscreen();
    }

    BeginDrawing();

    ClearBackground(WHITE);

    mode = 1;
    Vector2 resolution = {(float)W, (float)H};
    SetShaderValue(shader, cam_pos_loc, &cam->pos, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, cam_forward_loc, &cam->forward, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, cam_right_loc, &cam->right, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, cam_up_loc, &cam->up, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, max_ray_length_loc, &max_ray_length,
                   SHADER_UNIFORM_INT);
    SetShaderValue(shader, resolution_loc, &resolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, mode_loc, &mode, SHADER_UNIFORM_INT);

    BeginShaderMode(shader);

    DrawRectangle(0, 0, W, H, GRAY);

    EndShaderMode();

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
