#include <iostream>
using std::cerr;
using std::endl;
#include <limits>
#include <random>
#include <string>
using std::string;
#include <vector>

#include "common/loaders/ObjectFile.h"
#include "common/loaders/VolumeFile.h"

#include "common/importer/Importer.h"

#include <ospray_cpp/Data.h>

#include "OSPRayFixture.h"

using namespace ospcommon;

bool OSPRayFixture::customView = false;
vec3f OSPRayFixture::pos;
vec3f OSPRayFixture::at;
vec3f OSPRayFixture::up;

string OSPRayFixture::renderer_type;
string OSPRayFixture::imageOutputFile;

std::vector<string> OSPRayFixture::benchmarkModelFiles;

int OSPRayFixture::width  = 1024;
int OSPRayFixture::height = 1024;

float OSPRayFixture::samplingRate = 0.125f;

float OSPRayFixture::tf_scale = 1.f;
std::vector<vec4f> OSPRayFixture::tf_colors;
std::vector<float> OSPRayFixture::isosurfaces;

vec2f OSPRayFixture::volume_data_range = {
  std::numeric_limits<float>::infinity(),
  std::numeric_limits<float>::infinity()
};

vec3f OSPRayFixture::bg_color = {1.f, 1.f, 1.f};

// helper function to write the rendered image as PPM file
static void writePPM(const string &fileName, const int sizeX, const int sizeY,
                     const uint32_t *pixel)
{
  FILE *file = fopen(fileName.c_str(), "wb");
  fprintf(file, "P6\n%i %i\n255\n", sizeX, sizeY);
  unsigned char *out = (unsigned char *)alloca(3*sizeX);
  for (int y = 0; y < sizeY; y++) {
    const unsigned char *in = (const unsigned char *)&pixel[(sizeY-1-y)*sizeX];
    for (int x = 0; x < sizeX; x++) {
      out[3*x + 0] = in[4*x + 0];
      out[3*x + 1] = in[4*x + 1];
      out[3*x + 2] = in[4*x + 2];
    }
    fwrite(out, 3*sizeX, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}

static OSPTexture2D createTexture2D(ospray::miniSG::Texture2D *msgTex)
{
  if(msgTex == NULL)
  {
    static int numWarnings = 0;
    if (++numWarnings < 10)
    {
      cerr << "WARNING: material does not have Textures"
           << " (only warning for the first 10 times)!" << endl;
    }
    return NULL;
  }

  static std::map<ospray::miniSG::Texture2D*,
      OSPTexture2D> alreadyCreatedTextures;
  if (alreadyCreatedTextures.find(msgTex) != alreadyCreatedTextures.end())
    return alreadyCreatedTextures[msgTex];

  //TODO: We need to come up with a better way to handle different possible pixel layouts
  OSPTextureFormat type = OSP_TEXTURE_R8;

  if (msgTex->depth == 1) {
    if( msgTex->channels == 1 ) type = OSP_TEXTURE_R8;
    if( msgTex->channels == 3 )
      type = msgTex->prefereLinear ? OSP_TEXTURE_RGB8 : OSP_TEXTURE_SRGB;
    if( msgTex->channels == 4 )
      type = msgTex->prefereLinear ? OSP_TEXTURE_RGBA8 : OSP_TEXTURE_SRGBA;
  } else if (msgTex->depth == 4) {
    if( msgTex->channels == 1 ) type = OSP_TEXTURE_R32F;
    if( msgTex->channels == 3 ) type = OSP_TEXTURE_RGB32F;
    if( msgTex->channels == 4 ) type = OSP_TEXTURE_RGBA32F;
  }

  vec2i texSize(msgTex->width, msgTex->height);
  OSPTexture2D ospTex = ospNewTexture2D((osp::vec2i&)texSize,
                                        type,
                                        msgTex->data);

  alreadyCreatedTextures[msgTex] = ospTex;

  ospCommit(ospTex);
  return ospTex;
}

static OSPMaterial createDefaultMaterial(OSPRenderer renderer)
{
  static OSPMaterial ospMat = NULL;

  if (ospMat) return ospMat;

  ospMat = ospNewMaterial(renderer, "OBJMaterial");

  if (!ospMat)
  {
    std::string msg = "could not create default material 'OBJMaterial'";
    throw std::runtime_error(msg);
  }

  ospSet3f(ospMat, "Kd", .8f, 0.f, 0.f);
  ospCommit(ospMat);
  return ospMat;
}

static OSPMaterial createMaterial(OSPRenderer renderer,
                                  ospray::miniSG::Material *mat)
{
  if (mat == NULL)
  {
    static int numWarnings = 0;
    if (++numWarnings < 10)
    {
      cerr << "WARNING: model does not have materials! "
           << "(assigning default)" << endl;
    }
    return createDefaultMaterial(renderer);
  }

  static std::map<ospray::miniSG::Material*, OSPMaterial> createdMaterials;

  if (createdMaterials.find(mat) != createdMaterials.end()) {
    return createdMaterials[mat];
  }

  const char *type = mat->getParam("type", "OBJMaterial");
  assert(type);
  OSPMaterial ospMat = createdMaterials[mat]
      = ospNewMaterial(renderer, type);
  if (!ospMat) {
    return createDefaultMaterial(renderer);
  }

  const bool isOBJMaterial = !strcmp(type, "OBJMaterial");

  for (auto it =  mat->params.begin(); it !=  mat->params.end(); ++it) {
    const char *name = it->first.c_str();
    const auto *p = it->second.ptr;

    switch(p->type) {
    case ospray::miniSG::Material::Param::INT:
      ospSet1i(ospMat,name,p->i[0]);
      break;
    case ospray::miniSG::Material::Param::FLOAT: {
      float f = p->f[0];
      /* many mtl materials of obj models wrongly store the phong exponent
         'Ns' in range [0..1], whereas OSPRay's material implementations
         correctly interpret it to be in [0..inf), thus we map ranges here */
      if (isOBJMaterial &&
          (!strcmp(name, "Ns") || !strcmp(name, "ns")) &&
          f < 1.f) {
        f = 1.f/(1.f - f) - 1.f;
      }
      ospSet1f(ospMat, name, f);
    } break;
    case ospray::miniSG::Material::Param::FLOAT_3:
      ospSet3fv(ospMat,name,p->f);
      break;
    case ospray::miniSG::Material::Param::STRING:
      ospSetString(ospMat,name,p->s);
      break;
    case ospray::miniSG::Material::Param::TEXTURE:
    {
      auto *tex = (ospray::miniSG::Texture2D*)p->ptr;
      if (tex) {
        OSPTexture2D ospTex = createTexture2D(tex);
        assert(ospTex);
        ospCommit(ospTex);
        ospSetObject(ospMat, name, ospTex);
      }
      break;
    }
    default:
      throw std::runtime_error("unknown material parameter type");
    };
  }

  ospCommit(ospMat);
  return ospMat;
}

static void importObjectsFromFile(const std::string &filename,
                                  OSPRayFixture *f)
{
  auto &model = f->model;

  // Load OSPRay objects from a file.
  ospray::importer::Group *imported = ospray::importer::import(filename);

  // Iterate over geometries
  for (size_t i = 0; i < imported->geometry.size(); i++) {
    auto geometry = ospray::cpp::Geometry(imported->geometry[i]->handle);
    geometry.commit();
    model.addGeometry(geometry);
  }

  // Iterate over volumes
  for (size_t i=0 ; i < imported->volume.size() ; i++) {
    ospray::importer::Volume *vol = imported->volume[i];
    auto volume = ospray::cpp::Volume(vol->handle);

    // For now we set the same transfer function on all volumes.
    volume.set("transferFunction", f->tf);
    volume.set("samplingRate", f->samplingRate);
    volume.commit();

    // Add the loaded volume(s) to the model.
    model.addVolume(volume);

    // Set the minimum and maximum values in the domain for both color and
    // opacity components of the transfer function.
    f->tf.set("valueRange", vol->voxelRange.x, vol->voxelRange.y);
    f->tf.commit();

    auto volumeMesh = new ospray::miniSG::Mesh;
    volumeMesh->bounds = vol->bounds;
    f->sgModel.mesh.push_back(volumeMesh);

    // Create any specified isosurfaces
    if (!f->isosurfaces.empty()) {
      auto isoValueData = ospray::cpp::Data(f->isosurfaces.size(), OSP_FLOAT,
                                            f->isosurfaces.data());
      auto isoGeometry = ospray::cpp::Geometry("isosurfaces");

      isoGeometry.set("isovalues", isoValueData);
      isoGeometry.set("volume", volume);
      isoGeometry.commit();

      model.addGeometry(isoGeometry);
    }
  }

  model.commit();
}

static void loadModelFromFile(OSPRayFixture *f)
{
  for (auto &file : f->benchmarkModelFiles) {
    FileName fn = file;
    if (fn.ext() == "stl") {
      ospray::miniSG::importSTL(f->sgModel,fn);
    } else if (fn.ext() == "msg") {
      ospray::miniSG::importMSG(f->sgModel,fn);
    } else if (fn.ext() == "tri") {
      ospray::miniSG::importTRI(f->sgModel,fn);
    } else if (fn.ext() == "xml") {
      ospray::miniSG::importRIVL(f->sgModel,fn);
    } else if (fn.ext() == "obj") {
      ospray::miniSG::importOBJ(f->sgModel,fn);
    } else if (fn.ext() == "hbp") {
      ospray::miniSG::importHBP(f->sgModel,fn);
    } else if (fn.ext() == "x3d") {
      ospray::miniSG::importX3D(f->sgModel,fn);
    } else if (fn.ext() == "osp") {
      importObjectsFromFile(fn, f);
    } else {
      throw std::runtime_error("could not open file: " + file);
    }
  }
}

static void addMeshToModel(OSPRayFixture *f)
{
  for (size_t i = 0; i < f->sgModel.mesh.size(); i++) {
    Ref<ospray::miniSG::Mesh> msgMesh = f->sgModel.mesh[i];

    if (msgMesh->triangle.empty()) {
      continue;
    }

    // create ospray mesh
    auto ospMesh = ospray::cpp::Geometry("trianglemesh");

    // check if we have to transform the vertices:
    if (f->sgModel.instance[i] != ospray::miniSG::Instance(i)) {
      for (size_t vID = 0; vID < msgMesh->position.size(); vID++) {
        msgMesh->position[vID] = xfmPoint(f->sgModel.instance[i].xfm,
                                          msgMesh->position[vID]);
      }
    }

    // add position array to mesh
    auto position = ospray::cpp::Data(msgMesh->position.size(),
                                      OSP_FLOAT3A,
                                      &msgMesh->position[0],
                                      OSP_DATA_SHARED_BUFFER);
    ospMesh.set("position", position);

    // add triangle index array to mesh
    if (!msgMesh->triangleMaterialId.empty()) {
      auto primMatID = ospray::cpp::Data(msgMesh->triangleMaterialId.size(),
                                         OSP_INT,
                                         &msgMesh->triangleMaterialId[0],
                                         OSP_DATA_SHARED_BUFFER);
      ospMesh.set("prim.materialID", primMatID);
    }

    // add triangle index array to mesh
    auto index = ospray::cpp::Data(msgMesh->triangle.size(),
                                   OSP_INT3,
                                   &msgMesh->triangle[0],
                                   OSP_DATA_SHARED_BUFFER);
    assert(msgMesh->triangle.size() > 0);
    ospMesh.set("index", index);

    // add normal array to mesh
    if (!msgMesh->normal.empty()) {
      auto normal = ospray::cpp::Data(msgMesh->normal.size(),
                                      OSP_FLOAT3A,
                                      &msgMesh->normal[0],
                                      OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->normal.size() > 0);
      ospMesh.set("vertex.normal", normal);
    }

    // add color array to mesh
    if (!msgMesh->color.empty()) {
      auto color = ospray::cpp::Data(msgMesh->color.size(),
                                     OSP_FLOAT3A,
                                     &msgMesh->color[0],
                                     OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->color.size() > 0);
      ospMesh.set("vertex.color", color);
    }

    // add texcoord array to mesh
    if (!msgMesh->texcoord.empty()) {
      auto texcoord = ospray::cpp::Data(msgMesh->texcoord.size(),
                                        OSP_FLOAT2,
                                        &msgMesh->texcoord[0],
                                        OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->texcoord.size() > 0);
      ospMesh.set("vertex.texcoord", texcoord);
    }

    ospMesh.set("alpha_type", 0);
    ospMesh.set("alpha_component", 4);

    // add triangle material id array to mesh
    if (msgMesh->materialList.empty()) {
      // we have a single material for this mesh...
      OSPMaterial singleMaterial =
          createMaterial((OSPRenderer)f->renderer.handle(),
                         msgMesh->material.ptr);
      ospMesh.setMaterial(singleMaterial);
    } else {
      // we have an entire material list, assign that list
      std::vector<OSPMaterial > materialList;
      std::vector<OSPTexture2D > alphaMaps;
      std::vector<float> alphas;
      for (int i=0;i<msgMesh->materialList.size();i++) {
        materialList.push_back(
              createMaterial((OSPRenderer)f->renderer.handle(),
                             msgMesh->materialList[i].ptr)
              );

        for (auto it = msgMesh->materialList[i]->params.begin();
             it != msgMesh->materialList[i]->params.end(); it++) {
          const char *name = it->first.c_str();
          const ospray::miniSG::Material::Param *p = it->second.ptr;
          if(p->type == ospray::miniSG::Material::Param::TEXTURE) {
            if(!strcmp(name, "map_kd") || !strcmp(name, "map_Kd")) {
              auto *tex = (ospray::miniSG::Texture2D*)p->ptr;
              OSPTexture2D ospTex = createTexture2D(tex);
              ospCommit(ospTex);
              alphaMaps.push_back(ospTex);
            }
          } else if(p->type == ospray::miniSG::Material::Param::FLOAT) {
            if(!strcmp(name, "d")) alphas.push_back(p->f[0]);
          }
        }

        while(materialList.size() > alphaMaps.size()) {
          alphaMaps.push_back(NULL);
        }
        while(materialList.size() > alphas.size()) {
          alphas.push_back(0.f);
        }
      }
      auto ospMaterialList = ospray::cpp::Data(materialList.size(),
                                               OSP_OBJECT,
                                               &materialList[0]);
      ospMesh.set("materialList",ospMaterialList.handle());
    }

    ospMesh.commit();
    f->model.addGeometry(ospMesh);
  }

  f->model.commit();
}

static void createOSPCamera(OSPRayFixture *f)
{
  const box3f worldBounds(f->sgModel.getBBox());
  vec3f center = worldBounds.center();
  vec3f diag   = worldBounds.size();
  diag         = max(diag, vec3f(0.3f*length(diag)));

  vec3f pos, dir, up;

  if (OSPRayFixture::customView) {
    pos = OSPRayFixture::pos;
    up  = OSPRayFixture::up;
    dir = OSPRayFixture::at - OSPRayFixture::pos;
  } else {
    pos = center - 0.95f*vec3f(-.6*diag.x, -1.2*diag.y, .8*diag.z);
    up  = vec3f(0.f, 1.f, 0.f);
    dir = center - pos;
  }

  f->camera.set("pos", pos);
  f->camera.set("dir", dir);
  f->camera.set("up",  up );

  f->camera.set("aspect", f->width/float(f->height));
  f->camera.commit();
}

static void createDefaultTransferFunction(OSPRayFixture *f)
{
  // Add colors
  std::vector<vec4f> colors;
  if (f->tf_colors.empty()) {
    colors.emplace_back(0.f, 0.f, 0.f, 0.f);
    colors.emplace_back(0.9f, 0.9f, 0.9f, 1.f);
  } else {
    colors = f->tf_colors;
  }
  std::vector<vec3f> colorsAsVec3;
  for (auto &c : colors) colorsAsVec3.emplace_back(c.x, c.y, c.z);
  auto colorsData = ospray::cpp::Data(colors.size(), OSP_FLOAT3,
                                      colorsAsVec3.data());
  f->tf.set("colors", colorsData);

  // Add opacities
  std::vector<float> opacityValues;

  const int N_OPACITIES = 64;//NOTE(jda) - This affects image quality and
                             //            performance!
  const int N_INTERVALS = colors.size() - 1;
  const float OPACITIES_PER_INTERVAL = N_OPACITIES / float(N_INTERVALS);
  for (int i = 0; i < N_OPACITIES; ++i) {
    int lcolor = static_cast<int>(i/OPACITIES_PER_INTERVAL);
    int hcolor = lcolor + 1;

    float v0 = colors[lcolor].w;
    float v1 = colors[hcolor].w;
    float t = (i / OPACITIES_PER_INTERVAL) - lcolor;

    float opacity = (1-t)*v0 + t*v1;
    if (opacity > 1.f) opacity = 1.f;
    opacityValues.push_back(f->tf_scale*opacity);
  }
  auto opacityValuesData = ospray::cpp::Data(opacityValues.size(),
                                             OSP_FLOAT,
                                             opacityValues.data());
  f->tf.set("opacities", opacityValuesData);

  // Commit transfer function
  f->tf.commit();
}

static void createOSPRenderer(OSPRayFixture *f)
{
  auto &r = f->renderer_type;
  if (!r.empty()) {
    f->renderer = ospray::cpp::Renderer(r);
  }
  f->renderer.set("bgColor", f->bg_color.x, f->bg_color.y, f->bg_color.z);
}

static void createFramebuffer(OSPRayFixture *f)
{
  f->fb = ospray::cpp::FrameBuffer(osp::vec2i{f->width, f->height},
                                   OSP_FB_SRGBA,
                                   OSP_FB_COLOR|OSP_FB_ACCUM);
  f->fb.clear(OSP_FB_ACCUM | OSP_FB_COLOR);
}

OSPRayFixture::OSPRayFixture() :
  renderer("ao1"),
  camera("perspective"),
  tf("piecewise_linear")
{
}

void OSPRayFixture::SetUp()
{
  createDefaultTransferFunction(this);

  loadModelFromFile(this);
  createOSPRenderer(this);
  addMeshToModel(this);
  createOSPCamera(this);
  createFramebuffer(this);

  renderer.set("world",  model);
  renderer.set("model",  model);
  renderer.set("camera", camera);
  renderer.set("spp", 1);

  renderer.commit();
}

void OSPRayFixture::TearDown()
{
  if (!imageOutputFile.empty()) {
    auto *lfb = (uint32_t*)fb.map(OSP_FB_COLOR);
    writePPM(imageOutputFile + ".ppm", width, height, lfb);
    fb.unmap(lfb);
  }
}
