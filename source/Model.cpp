#include "Model.h"
#include <3ds.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "stb_image.h"

GLuint load_image(const char *fn) {
  int x, y, n;
  u8 *image_data = stbi_load(fn, &x, &y, &n, 4);
  if (image_data) {
    GLuint nname;
    glGenTextures(1, &nname);
    glBindTexture(GL_TEXTURE_2D, nname);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(image_data);
    return nname;
  } else {
    printf("Error loading file %s\n", fn);
    return 0;
  }

}

void model_load_obj(const char *filename, Model &model) {
  auto slurp_file = [](const char *filename) {
    std::ifstream t(filename);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());
    t.close();
    return str;
  };

  std::string obj = slurp_file(filename);
  std::string line;
  std::stringstream ss(obj);

  while (getline(ss, line)) {
    auto split = [](const std::string &s, char delim) {
      auto split_sub = [](const std::string &s, char delim,
                          std::vector<std::string> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
          elems.push_back(item);
        }
        return elems;
      };
      std::vector<std::string> elems;
      split_sub(s, delim, elems);
      return elems;
    };

    if (line.compare("") == 0)
      continue;

    std::vector<std::string> parts = split(line, ' ');

    if (parts.at(0).compare("v") == 0) {
      model.vertices.push_back(atof(parts.at(1).c_str())); // x, y, z
      model.vertices.push_back(atof(parts.at(2).c_str()));
      model.vertices.push_back(atof(parts.at(3).c_str()));
    } else if (parts.at(0).compare("vt") == 0) {
      model.texcoords.push_back(atof(parts.at(1).c_str())); // x, y
      model.texcoords.push_back(atof(parts.at(2).c_str()));
    } else if (parts.at(0).compare("vn") == 0) {
      model.normals.push_back(atof(parts.at(1).c_str())); // x, y, z
      model.normals.push_back(atof(parts.at(2).c_str()));
      model.normals.push_back(atof(parts.at(3).c_str()));
    } else if (parts.at(0).compare("usemtl") == 0) {
      // TODO
    } else if (parts.at(0).compare("mtllib") == 0) {
      auto load_mtl = [&model, &split, &slurp_file](const char *filename) {

        std::string mtl = slurp_file(filename);
        std::stringstream mss(mtl);
        std::string line;
        Model::Material *material = NULL;
        while (std::getline(mss, line)) {
          auto parts = split(line, ' ');
          if (parts.size() < 1)
            continue;
          if (parts.at(0).compare("newmtl") == 0) {
            if (material) {
              model.materials.push_back(material);
            }

            material = new Model::Material();
            material->name = parts.at(1);
          }

          if (material != NULL) {
            if (parts.at(0).compare("Ns") == 0) {
              material->ns = atof(parts.at(1).c_str());
            } else if (parts.at(0).compare("Ka") == 0) {
              material->ka[0] = atof(parts.at(1).c_str());
              material->ka[1] = atof(parts.at(2).c_str());
              material->ka[2] = atof(parts.at(3).c_str());
            } else if (parts.at(0).compare("Kd") == 0) {
              material->kd[0] = atof(parts.at(1).c_str());
              material->kd[1] = atof(parts.at(2).c_str());
              material->kd[2] = atof(parts.at(3).c_str());
            } else if (parts.at(0).compare("Ks") == 0) {
              material->ks[0] = atof(parts.at(1).c_str());
              material->ks[1] = atof(parts.at(2).c_str());
              material->ks[2] = atof(parts.at(3).c_str());
            } else if (parts.at(0).compare("Ni") == 0) {
              material->ni = atof(parts.at(1).c_str());
            } else if (parts.at(0).compare("d") == 0) {
              material->d = atof(parts.at(1).c_str());
            } else if (parts.at(0).compare("illum") == 0) {
              material->illum = atol(parts.at(1).c_str());
            } else if (parts.at(0).compare("map_Kd") == 0) {
              // expects path to be relative to MTL file.
              std::string mtl_path = filename;
              std::size_t last = mtl_path.find_last_of('/');
              std::string dir = "";
              if (last != std::string::npos) {
                dir = mtl_path.substr(0, last + 1);
              }

              std::string path = dir + parts.at(1);
              printf("Loading image:%s\n", path.c_str());

              material->map_kd = load_image(path.c_str());

              model.textures = true;
            }
          }
        }

        if (material) {
          model.materials.push_back(material);
          // delete material;
        }

      };
      std::string mtl_path = filename;
      std::size_t last = mtl_path.find_last_of('/');
      std::string dir = "";
      if (last != std::string::npos) {
        dir = mtl_path.substr(0, last + 1);
      }

      std::string path = dir + parts.at(1);
      load_mtl(path.c_str());
    } else if (parts.at(0).compare("o") == 0) {
      // TODO
    } else if (parts.at(0).compare("s") == 0) {
      // TODO
    } else if (parts.at(0).compare("f") == 0) {
      auto add_index = [&split, &model](std::string ver) {
        std::vector<std::string> part = split(ver, '/');
        model.indices.push_back(atol(part.at(0).c_str()) - 1);
        model.indices.push_back(atol(part.at(1).c_str()) - 1);
        model.indices.push_back(atol(part.at(2).c_str()) - 1);
      };

      add_index(parts.at(1));
      add_index(parts.at(2));
      add_index(parts.at(3));
    }
  }

  model.indices_size = model.indices.size();
}

void render_model(Model &model) {
  glDisable(GL_TEXTURE_2D);
  if (!model.listId) {
    model.listId = glGenLists(1);



    glNewList(model.listId, GL_COMPILE);
    if (model.textures) {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, model.materials.at(0)->map_kd);
    }
    glBegin(GL_TRIANGLES);
    glColor4f(1.0, 1.0, 1.0, 1.0f);
    for (uint16_t i = 0; i <= model.indices_size + 1; i += 3) {
      uint16_t index = model.indices[i];
      uint16_t tex = model.indices[i + 1];
      uint16_t norm = model.indices[i + 2];

      if (model.textures) {
        glTexCoord2f(model.texcoords[(tex * 2)], 1.0 - model.texcoords[(tex * 2) + 1]);
      }
      glNormal3f(model.normals[(norm * 3)], model.normals[(norm * 3) + 1],
                 model.normals[(norm * 3) + 2]);
      glVertex3f(model.vertices[(index * 3)], model.vertices[(index * 3) + 1],
                 model.vertices[(index * 3) + 2]);
    }
    glEnd();
    glEndList();
  }
  glCallList(model.listId);
}
