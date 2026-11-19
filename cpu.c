#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#define PREVENT_ZERO 1e-44f

typedef struct camera {
  Vector3 pos;
  Vector3 forward;
  Vector3 up;
  Vector3 right;
} camera;

int W = 800;
int H = 800;

int changeSpeed = 0;
float arrow_sens_rev = 20.0f;
float sensitivity = 1.5f;
float speed = 0.05f;

int max_ray_length = 100;
float ray_process_unit = 1.0f;

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

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(W, H, "Fractal.");

  Shader shader = LoadShader(0, "gpu.frag");

  int cam_pos_loc = GetShaderLocation(shader, "cam_pos");
  int cam_forward_loc = GetShaderLocation(shader, "cam_forward");
  int cam_right_loc = GetShaderLocation(shader, "cam_right");
  int cam_up_loc = GetShaderLocation(shader, "cam_up");
  int max_ray_length_loc = GetShaderLocation(shader, "max_ray_length");
  int resolution_loc = GetShaderLocation(shader, "resolution");

  DisableCursor();
  while (!WindowShouldClose()) {
    W = GetScreenWidth();
    H = GetScreenHeight();

    Vector2 mdelta = GetMouseDelta();
    rotateCamera(cam, mdelta);

    Vector3 case_forward = Vector3Scale(
        cam->forward,
        speed * (changeSpeed ? (Vector3Length(cam->pos) + PREVENT_ZERO) : 1));
    Vector3 case_right = Vector3Scale(
        cam->right,
        speed * (changeSpeed ? (Vector3Length(cam->pos) + PREVENT_ZERO) : 1));
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

    Vector2 resolution = {(float)W, (float)H};
    SetShaderValue(shader, cam_pos_loc, &cam->pos, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, cam_forward_loc, &cam->forward, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, cam_right_loc, &cam->right, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, cam_up_loc, &cam->up, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, max_ray_length_loc, &max_ray_length,
                   SHADER_UNIFORM_INT);
    SetShaderValue(shader, resolution_loc, &resolution, SHADER_UNIFORM_VEC2);

    BeginShaderMode(shader);

    DrawRectangle(0, 0, W, H, GRAY);

    EndShaderMode();

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
