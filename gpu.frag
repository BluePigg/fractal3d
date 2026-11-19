#version 330

#define error_range 0.01

struct Ray {
  vec3 pos;
  vec3 direction;
  int len;
};

out vec4 finalColor;

uniform vec2 resolution;

uniform int mode;

uniform vec3 cam_pos;
uniform vec3 cam_forward;
uniform vec3 cam_right;
uniform vec3 cam_up;

uniform int max_ray_length;

vec4 COLOR_HIT = vec4(0.0f,0.5f,0.5f,1.0f);
vec4 COLOR_MISS = vec4(1.0f,1.0f,1.0f,1.0f);

void process_ray(inout Ray ray, float process_unit) {
  ray.pos += ray.direction*process_unit;
  ray.len++;
}

// void checkboard(Ray ray) {
//   bool black = false;
//   while (ray.pos.y>error_range) {
//     if (ray.len>=max_ray_length) {
//       break;
//     }
//     process_ray(ray,ray.pos.y);
//   }
//   bool hit = ray.len<max_ray_length;
//   if (hit) {
//     black = int(floor(ray.pos.x)+floor(ray.pos.z))%2 == 0;
//   }
//
//   finalColor = hit ? (black ? vec4(0.0f,0.0f,0.0f,1.0f) : vec4(0.0f,0.0f,0.5f,1.0f)) : finalColor;
// }
//
// void sphere(Ray ray) {
//   while (length(ray.pos)-1.0f>error_range) {
//     if (ray.len>=max_ray_length) {
//       break;
//     }
//     process_ray(ray,length(ray.pos)-1.0f);
//   }
//   bool hit = ray.len<max_ray_length;
//   finalColor = hit ? vec4(1.0f,0.0f,0.0f,1.0f) : finalColor;
// }

float distance_function_template_(vec3 pos) {
  return length(pos) - 1.0f;
}

float distance_function_template(vec3 pos) {
  return max(abs(pos.x),abs(pos.z))-2;
}

void template_(Ray ray) {
  while ( distance_function_template(ray.pos) > error_range) {
      if (ray.len>=max_ray_length) {
        break;
      }
      process_ray(ray,distance_function_template(ray.pos));
  }
  bool hit = ray.len<max_ray_length;
  if (ray.len<=0) ray.len = 1;
  vec3 hitcolor = COLOR_HIT.xyz/(1.0 + ray.len*0.05);
  finalColor = hit ? vec4(hitcolor.x,hitcolor.y,hitcolor.z,1.0) : (COLOR_MISS);
}

void main() {
  vec2 coord = (gl_FragCoord.xy-0.5*resolution) / (resolution.y*0.5);
  Ray ray;
  ray.pos = cam_pos;
  ray.direction = normalize(cam_forward + coord.x*cam_right + coord.y*cam_up);
  ray.len = 0;

  finalColor = vec4(1.0f,1.0f,1.0f,1.0f);

  if (mode==0) {
    float d = distance_function_template(cam_pos);
    uint bits = floatBitsToUint(d);
    float r = float((bits >> 24) & 255u) / 255.0;
    float g = float((bits >> 16) & 255u) / 255.0;
    float b = float((bits >> 8) & 255u) / 255.0;
    float a = float(bits & 255u) / 255.0;
    finalColor = vec4(r,g,b,a);
  } else {
    template_(ray);
  }
}
