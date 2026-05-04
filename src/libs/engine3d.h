#ifndef ENGINE3D_H
#define ENGINE3D_H

typedef struct scene scene;
typedef struct mesh mesh;
typedef struct material material;
typedef struct camera camera;
typedef struct light light;
typedef struct transform transform;

scene* createscene(void);
void destroyscene(scene* s);
mesh* loadmesh(char* path);
void freemesh(mesh* m);
material* creatematerial(float r, float g, float b, float shininess);
camera* createcamera(float fov, float aspect, float near, float far);
light* createlight(int type, float x, float y, float z, float r, float g, float b, float intensity);
transform* createtransform(void);
void setposition(transform* t, float x, float y, float z);
void setrotation(transform* t, float x, float y, float z);
void setscale(transform* t, float x, float y, float z);
void addmesh(scene* s, mesh* m, transform* t, material* mat);
void addlight(scene* s, light* l);
void setcamera(scene* s, camera* cam);
void render(scene* s);
void init3d(int w, int h);
void run3d(scene* s, void (*update)(void*), void (*render)(void*), void* data);

#endif
