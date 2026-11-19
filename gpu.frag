#version 330

#define error_range 0.01

struct Ray {
  vec3 pos;
  vec3 direction;
  int len;
};

out vec4 finalColor;

uniform vec2 resolution;

uniform vec3 cam_pos;
uniform vec3 cam_forward;
uniform vec3 cam_right;
uniform vec3 cam_up;

uniform int max_ray_length;


void process_ray(inout Ray ray, float process_unit) {
  ray.pos += ray.direction*process_unit;
  ray.len++;
}

void checkboard(Ray ray) {
  bool black = false;
  while (ray.pos.y>error_range) {
    if (ray.len>=max_ray_length) {
      break;
    }
    process_ray(ray,ray.pos.y);
  }
  bool hit = ray.len<max_ray_length;
  if (hit) {
    black = int(floor(ray.pos.x)+floor(ray.pos.z))%2 == 0;
  }

  finalColor = hit ? (black ? vec4(0.0f,0.0f,0.0f,1.0f) : vec4(0.0f,0.0f,0.5f,1.0f)) : finalColor;
}

void sphere(Ray ray) {
  while (length(ray.pos)-1.0f>error_range) {
    if (ray.len>=max_ray_length) {
      break;
    }
    process_ray(ray,length(ray.pos)-1.0f);
  }
  bool hit = ray.len<max_ray_length;
  finalColor = hit ? vec4(1.0f,0.0f,0.0f,1.0f) : finalColor;
}

void main() {
  vec2 coord = (gl_FragCoord.xy-0.5*resolution) / (resolution.y*0.5);
  Ray ray;
  ray.pos = cam_pos;
  ray.direction = normalize(cam_forward + coord.x*cam_right + coord.y*cam_up);
  ray.len = 0;

  finalColor = vec4(1.0f,1.0f,1.0f,1.0f);

  checkboard(ray);
}
