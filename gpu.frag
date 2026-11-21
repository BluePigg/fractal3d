#version 330

#define min_er 0.000001f
#define sponge_unit 100.0f
#define fog_ratio 0.4f
int sponge_count = 10;
float er_unit = 0.0001f;
float error_range = 0.001f;

struct Ray {
  vec3 pos;
  vec3 direction;
  float flight_len;
  int len;
};

out vec4 finalColor;

uniform vec2 resolution;

uniform int mode;

uniform vec3 cam_pos;
uniform vec3 cam_forward;
uniform vec3 cam_right;
uniform vec3 cam_up;

int max_ray_length;

vec4 COLOR_HIT = vec4(0.0f,0.5f,0.5f,1.0f);
vec4 COLOR_MISS = vec4(0.8f,0.8f,0.8f,1.0f);

void process_ray(inout Ray ray, float process_unit) {
  ray.pos += ray.direction*process_unit;
  ray.flight_len += process_unit;
  ray.len++;
}

float distance_function_template_(vec3 pos) {
  return length(pos) - 1.0f;
}

float get_cross(vec3 p) {
  float pillar_1 = max(abs(p.x),abs(p.z)) - 1.0f / 3.0f;
  float pillar_2 = max(abs(p.x),abs(p.y)) - 1.0f / 3.0f;
  float pillar_3 = max(abs(p.y),abs(p.z)) - 1.0f / 3.0f;
  return max(max(-pillar_1,-pillar_2),-pillar_3);
}

float distance_function(vec3 pos) {
  vec3 p = pos/sponge_unit;
  float cube = max(max(abs(p.x),abs(p.y)),abs(p.z)) - 1.0f;
  float d = max(cube,get_cross(p));
  float scale = 1.0f;
  for (int i=0; i<sponge_count-1; i++) {
    p*=3.0f;
    scale *= 3.0f;
    p.x = mod(p.x+1.0f,2.0f)-1.0f;
    p.y = mod(p.y+1.0f,2.0f)-1.0f;
    p.z = mod(p.z+1.0f,2.0f)-1.0f;
    d = max(d,get_cross(p)/scale);
    if (sponge_unit/(3.0f*scale) <= error_range) {break;}
  }
  return d*sponge_unit;
}

void render(Ray ray) {
  float d;
  while (  (d=distance_function(ray.pos)) > error_range) {
      if (ray.len>=max_ray_length) {
        break;
      }
      process_ray(ray,d);
    error_range = max(er_unit * ray.flight_len,min_er);
  }
  bool hit = ray.len<max_ray_length;
  if (ray.len<=0) ray.len = 1;
  vec3 hitcolor = COLOR_HIT.xyz/(1.0 + ray.len*0.05);
  vec3 misscolor = COLOR_MISS.xyz/(1.0 + distance(cam_pos,ray.pos)*0.05);
  vec3 fcolor = mix(hitcolor,misscolor,fog_ratio);
  finalColor = hit ? vec4(fcolor.x,fcolor.y,fcolor.z,1.0) : COLOR_MISS;
}

void center(Ray ray) {
  while (length(ray.pos)-0.01f>error_range) {
    if (ray.len>=max_ray_length) {
      break;
    }
    process_ray(ray,length(ray.pos)-0.01f);
  }
  if (ray.len<max_ray_length) {
    finalColor = vec4(1.0f,0.0f,0.0f,1.0f);
  }
}

void main() {
  vec2 coord = (gl_FragCoord.xy-0.5*resolution) / (resolution.y*0.5);
  Ray ray;
  ray.pos = cam_pos;
  ray.direction = normalize(cam_forward + coord.x*cam_right + coord.y*cam_up);
  ray.len = 0;
  ray.flight_len = 0.0f;

  finalColor = vec4(1.0f,1.0f,1.0f,1.0f);


  float cd = distance_function(cam_pos);
  if (mode==0) {
    uint bits = floatBitsToUint(cd);
    float r = float((bits >> 24) & 255u) / 255.0;
    float g = float((bits >> 16) & 255u) / 255.0;
    float b = float((bits >> 8) & 255u) / 255.0;
    float a = float(bits & 255u) / 255.0;
    finalColor = vec4(r,g,b,a);
  } else {
    max_ray_length = 100+int(max(0.0f,-log2(max(cd,min_er)))*20);
    render(ray);
  }
}
