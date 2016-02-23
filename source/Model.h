#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <stdint.h>
#include <string>
#include <GL/gl.h>

typedef struct {

  typedef struct {
    std::string name;
    float ns;
    float ka[3];
    float kd[3];
    float ks[3];
    float ni;
    float d;
    int illum;
    GLuint map_kd;
  } Material;

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> texcoords;
  std::vector<uint16_t> indices;
  uint32_t indices_size;
  bool textures = false;
  std::vector<Material *> materials;
  GLuint listId = 0;

} Model;

void model_load_obj(const char *fn, Model &mod);
void render_model(Model &model);
GLuint load_image(const char *fn);

#endif
