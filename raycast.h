#define SCENE_FILE "example.json"
#define OUTPUT_FILE "output.ppm"
#define ORIGIN 0
#define MAX_DEPTH 255
#define EXPECTED_ARGS 5

//Needed Structs and Types

typedef struct Object{
  int kind; // 0 = cylinder, 1 = sphere, 2 = plane, 3 = camera
  double color[3];
  union {
    struct {
      double center[3];
      double radius;
    } cylinder;
    struct {
      double center[3];
      double radius;
    } sphere;
    struct {
      double center[3];
      double width;
      double height;
      double normal[3];
    } plane;
    struct {
      double width;
      double height;
    } camera;
  };
} Object;

typedef struct Pixel{
  int r,g,b;
} Pixel;

typedef Object Scene[128];
//Main method prototypes
int read_scene(char* json, Scene scene);
int raycast (int *buffer, Scene scene, int num_objects, int width, int height);
int ppm_output(int  *buffer, char *output_file_name, int size, int depth, int width, int height);

//Parser prototypes
double* next_vector(FILE* json);
double next_number(FILE* json);
char* next_string(FILE* json);
void skip_ws(FILE* json);
void expect_c(FILE* json, int d);
int next_c(FILE* json);

//Intersect prototypes and methods
double sphere_intersection(double* Ro, double* Rd, double* C, double r);
double plane_intersection(double* Ro, double* Rd, double* c, double *n);
static inline double sqr(double v) {
  return v*v;
}
static inline void normalize(double* v) {
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}
static inline int float_color_to_int(double d){
}
