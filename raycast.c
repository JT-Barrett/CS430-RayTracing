/*********************************************************************
| Title: raycast.c
| Author: JT Barrett
| Assignment: CS 430 - Computer Graphics: Project 2
| Date Completed: 10/4/2016
\*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include "raycast.h"

int main(int argc, char* argv[]){
  //Uncomment debugging prints for more information

  if (argc != EXPECTED_ARGS){
    fprintf(stderr, "Error: The program was not passed the correct number of arguments.\n");
    return 1;
  }

  //Create the scene of objects and parse the json file into it
  Object *scene = malloc(sizeof(Object)*128);
  int num_objects = read_scene(argv[3], scene);

  //Take the scene and do the raycasting to generate a pixel buffer
  
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);
  //if (width > 300 || height > 300){
   // fprintf(stderr, "Error: This program cannot handle images that large.\n");
   // return 1;
  //}
  Pixel pixbuffer[width*height];
  raycast(pixbuffer, scene, num_objects, width, height);

  //Take the pixel buffer and write it as a valid ppm file.
  ppm_output(pixbuffer, argv[4], sizeof(Pixel)*(width*height), 255, width, height);

    //printf("\nNumber of Objects: %d\n", num_objects);

  /*for (int i = 0; i < PIXELS_M*PIXELS_N*3; i++)
    printf("%d ", pixbuffer[i]);
  */

  //printf("\nCamera Info: %d\n %lf %lf", scene[0].kind, scene[0].camera.width, scene[0].camera.height);

  return 0;
}


/**********************
|Main Methods
\*********************/
int read_scene(char* json_input, Scene scene){
  int c;
  FILE* json = fopen(json_input, "r");
  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", json);
    exit(1);
  }

  skip_ws(json);

  // Find the beginning of the list
  expect_c(json, '[');

  skip_ws(json);

  // Find the objects

  int obj_num = 0;
  while (1) {
    int c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
      fclose(json);
      return -1;
    }
    if (c == '{') {
      skip_ws(json);

      // Parse the object
      char* key = next_string(json);
      if (strcmp(key, "type") != 0) {
        fprintf(stderr, "Error: Expected \"type\" key\n");
        exit(1);
      }

      skip_ws(json);

      expect_c(json, ':');

      skip_ws(json);

      char* value = next_string(json);

      //extract the object type
      if (strcmp(value, "camera") == 0) {
        scene[obj_num].kind = 3;
        //printf("||Camera Recognized||\n");
      }
      else if (strcmp(value, "sphere") == 0) {
        scene[obj_num].kind = 1;
        //printf("||Sphere Recognized||\n");
      }
      else if (strcmp(value, "plane") == 0) {
        scene[obj_num].kind = 2;
        //printf("||Plane Recognized||\n");
      }
      else if (strcmp(value, "light") == 0) {
        scene[obj_num].kind = 4;
        //printf("||Plane Recognized||\n");
      }
      else {
        fprintf(stderr, "Error: Unknown type, \"%s\"\n", value);
        exit(1);
      }

      skip_ws(json);

      //extract any other fields
      while (1) {
        // , }
        c = next_c(json);
        if (c == '}') {
          // stop parsing this object
          break;
        }
        else if (c == ',') {
          // read another field
          skip_ws(json);
          char* key = next_string(json);
          skip_ws(json);
          expect_c(json, ':');
          skip_ws(json);

          //check the object type, and insert into the appropriate fields
          if (scene[obj_num].kind == 3){
            if (strcmp(key, "width") == 0){
              double value = next_number(json);              
              memcpy(&scene[obj_num].camera.width, &value, sizeof(double));  
            }else if (strcmp(key, "height") == 0){
              double value = next_number(json);
              scene[obj_num].camera.height = value;
              memcpy(&scene[obj_num].camera.height, &value, sizeof(double));
            }else{
              fprintf(stderr, "Error: Unrecognized field \"%s\" for 'camera'.\n.", key);
              exit(1);
            }
          }
          else if (scene[obj_num].kind == 1){
            if (strcmp(key, "diffuse_color") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].diffuse_color, value, sizeof(double)*3);
            }
            else if (strcmp(key, "specular_color") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].specular_color, value, sizeof(double)*3);
            }            
            else if (strcmp(key, "radius") == 0)
              scene[obj_num].sphere.radius = next_number(json);
            else if (strcmp(key, "position") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].sphere.center, value, sizeof(double)*3);
            }
            else{
              fprintf(stderr, "Error: Unrecognized field \"%s\" for 'sphere'.\n.", key);
              exit(1);
            }
          }
          else if (scene[obj_num].kind == 2){
            if (strcmp(key, "width") == 0)
              scene[obj_num].plane.width = next_number(json);
            else if (strcmp(key, "height") == 0)
              scene[obj_num].plane.height = next_number(json);
            else if (strcmp(key, "diffuse_color") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].diffuse_color, value, sizeof(double)*3);
            }
            else if (strcmp(key, "specular_color") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].specular_color, value, sizeof(double)*3);
            }
            else if (strcmp(key, "position") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].plane.center, value, sizeof(double)*3);
            }
            else if (strcmp(key, "normal") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].plane.normal, value, sizeof(double)*3);
            }
            else{
              fprintf(stderr, "Error: Unrecognized field \"%s\" for 'plane'.\n.", key);
              exit(1);
            }
          }
          else if (scene[obj_num].kind == 4){
            if (strcmp(key, "color") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].light.color, value, sizeof(double)*3);
            }
            else if (strcmp(key, "position") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].light.center, value, sizeof(double)*3);
            }
            else if (strcmp(key, "direction") == 0){
              double* value = next_vector(json);
              memcpy(&scene[obj_num].light.direction, value, sizeof(double)*3);
            }
            else if (strcmp(key, "theta") == 0)
              scene[obj_num].light.theta = next_number(json);            
            else if (strcmp(key, "radial-a2") == 0)
              scene[obj_num].light.radial_a2 = next_number(json);
            else if (strcmp(key, "radial-a1") == 0)
              scene[obj_num].light.radial_a1 = next_number(json);
            else if (strcmp(key, "radial-a0") == 0)
              scene[obj_num].light.radial_a0 = next_number(json);
            else if (strcmp(key, "angular-a0") == 0)
              scene[obj_num].light.radial_a0 = next_number(json);                                                 
            else{
              fprintf(stderr, "Error: Unrecognized field \"%s\" for 'light'.\n.", key);
              exit(1);
            }
          }          
        }
        skip_ws(json);
      }

      //printf("--New Object Loaded--\n");
      obj_num++;
      /*else {
        fprintf(stderr, "Error: Unexpected value\n");
        exit(1);
      }*/
    }
    skip_ws(json);
    c = next_c(json);
    //check if end of object or end of json file
    if (c == ',') {
      // noop
      skip_ws(json);
    }
    else if (c == ']') {
      fclose(json);
      return obj_num;
    }
    else {
      fprintf(stderr, "Error: Expecting ',' or ']'.\n");
      exit(1);
    }
  }

  //return how many objects were parsed
  return obj_num;
}

// This method creates the viewplane and loops through each object checking for colisions for each ray
// Each ray is then converted to a pixel on the screen and placed into a pixel array for PPM output.
int raycast(Pixel *buffer, Scene scene, int num_objects, int width, int height){
  printf("raycast entered\n");
  //camera starts at 0, load in camera dimensions
  double cx = 0;
  double cy = 0;
  double h = scene[0].camera.height;
  double w = scene[0].camera.width;

  //set the image dimensions
  int M = width;
  int N = height;

  double pixheight = h / M;
  double pixwidth = w / N;
  printf("\nM:%d N:%d h:%d w:%d pH:%lf pW:%lf\n", M, N, h, w, pixheight, pixwidth);

  int current_pixel = 0;
  int testpixel = 0;
  for (int y = 0; y < N; y += 1) {
    for (int x = 0; x < M; x += 1) {
      double Ro[3] = { 0, 0, 0 };
      // Rd = normalize(P - Ro)
      double Rd[3] = {
        cx - (w / 2) + pixwidth * (x + 0.5),
        cy - (h / 2) + pixheight * (y + 0.5),
        1
      };
      v3_normalize(Rd);

      //initialize variables for closest colision, then loop over objects looking for colisions
      double best_t = INFINITY;
      int best_obj = 0;
      for (int i = 1; i < num_objects; i += 1) {
        double t = 0;

        //check object type and send to appropriate colision check
        switch (scene[i].kind) {
        case 1:
          t = sphere_intersection(Ro, Rd, scene[i].sphere.center, scene[i].sphere.radius);
          break;
        case 2:
          t = plane_intersection(Ro, Rd, scene[i].plane.center, scene[i].plane.normal);
          break;
        case 4:
          //it's a light do nothing
          break;
        default:
          fprintf(stderr, "Error: Unrecognized object in scene\n");
          exit(1);
        }

        //set closest colision variables
        if (t > 0 && t < best_t){
          best_t = t;
          best_obj = i;
        }
      }


      //Initialize a pixel and convert the scene's 0.0-1.0 color to 0-255 color
      Pixel pix;
      if (best_t > 0 && best_t != INFINITY) {
        double color[3];

        //initialize color as black
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
        
        //loop over lights adding lighting elements to the color
        for (int i = 1; i < num_objects; i += 1) {
          if(scene[i].kind != 4) 
            continue;
          double Ron[3];
          double Rdn[3];
          double Ron_temp[3];

          //cast a ray from intersection to light, if there is no object in the way add specular color
          v3_scale(Rd, best_t, Ron_temp);
          v3_add(Ron_temp, Ro, Ron);
          v3_sub(scene[i].light.center, Ron, Rdn);

          double B[3];
          v3_sub(scene[i].light.center, Ron, B);
          double dist = v3_mag(B);



          double best_t_shadow = INFINITY;
          double best_obj_shadow = 0;
          for (int k=1; k < num_objects ; k += 1){
            //carful, origin not at 0
            if (k != best_obj){
              double t_shadow = 0;
              switch(scene[k].kind){
              case 1:
                t_shadow = sphere_intersection(Ron, Rdn, scene[k].sphere.center, scene[k].sphere.radius);
                break;
              case 2:
                t_shadow = plane_intersection(Ron, Rdn, scene[k].plane.center, scene[k].plane.normal);
                break;
              case 4:
                break;                
              default:
                fprintf(stderr, "Error: Unrecognized object in scene when looking for shadow\n");
                exit(1);
              }

              if (t_shadow > 0 && t_shadow < best_t_shadow){
                best_t_shadow = t_shadow;
                best_obj_shadow = k;
              }

            }
          }

          //make sure to check that light obstruction is in front of the light
          if (best_t_shadow > 0 && best_t_shadow != INFINITY && best_t_shadow < dist ) {
            //Do nothing!
          }
          else{
            // no object was in the way
            // N, L, R, V
            // Set up needed vectors
            double N[3], L[3], R[3], V[3];

            if(scene[best_obj].kind == 2)
              memcpy(N, scene[best_obj].plane.normal, sizeof(double)*3); // if plane
            else
              v3_sub(Ron, scene[best_obj].sphere.center, N);
            v3_normalize(N);

            v3_sub(scene[i].light.center, Ron, L);
            v3_normalize(L); //light_position - Ron;

            //r=L−2(L DOT n)n
            double q = 2*v3_dot(L, N);
            double rscale[3];
            v3_scale(N, q, rscale);
            v3_sub(L, rscale, R);
            v3_normalize(R);

            memcpy(V, Rd, sizeof(double)*3);
            v3_scale(V, -1.0, V);
            v3_normalize(V);

            //Populate the diffuse component
            double diff_nl = v3_dot(N, L);
            double diff[3];
              diff[0] = scene[best_obj].diffuse_color[0]*scene[i].light.color[0];
              diff[1] = scene[best_obj].diffuse_color[1]*scene[i].light.color[1];
              diff[2] = scene[best_obj].diffuse_color[2]*scene[i].light.color[2];
              v3_scale(diff, diff_nl, diff);  

            //Populate the specular component
            double spec_vr = -1*v3_dot(V, R);
            double spec[3];
            if(spec_vr > 0){
              spec[0] = (scene[best_obj].specular_color[0]*scene[i].light.color[0]*pow(spec_vr, 20));
              spec[1] = (scene[best_obj].specular_color[1]*scene[i].light.color[1]*pow(spec_vr, 20));
              spec[2] = (scene[best_obj].specular_color[2]*scene[i].light.color[2]*pow(spec_vr, 20));
            }
            else {
              spec[0] = 0;
              spec[1] = 0;
              spec[2] = 0;
            }

            //account for angular and radial attinuation and add diffuse + specular to finish color processing
            double rad_a = frad(scene[i].light.radial_a0, scene[i].light.radial_a1, scene[i].light.radial_a2, dist);
            double ang_a;
              if(scene[i].light.theta != 0){
                ang_a = fang(Rd, scene[i].light.direction, scene[i].light.theta * M_PI / 180.0, scene[i].light.angular_a0);
              }
              else
                ang_a = 1;
            color[0] += rad_a * ang_a * (diff[0] + spec[0]);
            color[1] += rad_a * ang_a * (diff[1] + spec[1]); //*fang * frad
            color[2] += rad_a * ang_a * (diff[2] + spec[2]);
          }
        }

        //Assign and clamp color values
        pix.r = (unsigned char)clamp(color[0]*255.0);
        pix.g = (unsigned char)clamp(color[1]*255.0);
        pix.b = (unsigned char)clamp(color[2]*255.0);

      }
      else {
        //eigengrau
        pix.r = (unsigned char)clamp(0.086);
        pix.g = (unsigned char)clamp(0.086);
        pix.b = (unsigned char)clamp(0.114);
        //printf(".");
      }

      //load the pixel data into the pixel buffer
      buffer[current_pixel++] = pix;

    }
  }

  return 0;
}

//This method takes an array of color values as integers and writes them to a PPM P3 file.
int ppm_output(Pixel *buffer, char *output_file_name, int size, int depth, int width, int height){
  printf("\nppm_output entered\n");
  // Attempt to open outfile
  FILE *output_file;
  output_file = fopen(output_file_name, "w");
  if (!output_file)
  {
    fprintf(stderr, "\nError: Could not open file for write.");
    fclose(output_file);
    exit(1);
  }

  else {
    //Write the PPM P3 header
    fprintf(output_file, "P3\n%d %d\n%d\n", width, height, depth);
    //Append pixel data onto the rest of the file BACKWARDS, the image is upside down!
      for (int i = width-1; i >=0; i--)
      {
        for (int j = 0; j<height; j++)
        {
          fprintf(output_file, "%d ", buffer[(i*width)+j].r);
          fprintf(output_file, "%d ", buffer[(i*width)+j].g);
          fprintf(output_file, "%d ", buffer[(i*width)+j].b);
        }
        // Add newline after each line
        fprintf(output_file, "\n");
      }
  }

  //close up the output file
  fclose(output_file);
  return 0;
}


/**********************
|Parser Methods
\*********************/
double* next_vector(FILE* json){
  double* v = malloc(3 * sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  v[0] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[1] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[2] = next_number(json);
  skip_ws(json);
  expect_c(json, ']');
  return v;
}
double next_number(FILE* json){
  double value;
  fscanf(json, "%lf", &value);
  // Error check this..
  return value;
}
char* next_string(FILE* json){
  char buffer[129];
  int c = next_c(json);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string");
    exit(1);
  }
  c = next_c(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = next_c(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}
void skip_ws(FILE* json){
  int c = next_c(json);
  while (isspace(c)) {
    c = next_c(json);
  }
  ungetc(c, json);
}
void expect_c(FILE* json, int d){
  int c = next_c(json);
  if (c == d) return;
  fprintf(stderr, "Error: Expected '%c'");
  exit(1);
}
int next_c(FILE* json){
  int c = fgetc(json);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file");
    exit(1);
  }
  return c;
}


/**********************
|Intersect Methods
\*********************/
double sphere_intersection(double* Ro, double* Rd, double* C, double r){
  double a = (sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]));
  double b = (2 * (Ro[0] * Rd[0] - Rd[0] * C[0] + Ro[1] * Rd[1] - Rd[1] * C[1] + Ro[2] * Rd[2] - Rd[2] * C[2]));
  double c = sqr(Ro[0]) - 2 * Ro[0] * C[0] + sqr(C[0]) + sqr(Ro[1]) - 2 * Ro[1] * C[1] + sqr(C[1]) + sqr(Ro[2]) - 2 * Ro[2] * C[2] + sqr(C[2]) - sqr(r);

  double det = sqr(b) - 4 * a * c;
  if (det <= 0) return -1;

  det = sqrt(det);

  double t0 = (-b - det) / (2 * a);
  if (t0 > 0) return t0;

  double t1 = (-b + det) / (2 * a);
  if (t1 > 0) return t1;

  return -1;
}

double plane_intersection(double* Ro, double* Rd, double* c, double *n){
  v3_normalize(n);
  v3_normalize(Rd);
  double d = -(v3_dot(c, n));
  double e = v3_dot(Ro, n);
  double f = v3_dot(Rd, n);

  double t = -(e + d)/f;
  if (t > 0){
    return t;
  }
  return -1;
}

