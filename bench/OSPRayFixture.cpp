#include <iostream>
using std::cerr;
using std::endl;
#include <random>
#include <string>
using std::string;
#include <vector>

#include "OSPRayFixture.h"

using ospray::box3f;
using ospray::vec2i;
using ospray::vec3f;

#ifndef BENCH_MODEL
# define BENCH_MODEL "/Users/jdamstut/data/city/city.obj"
#endif

// helper function to write the rendered image as PPM file
static void writePPM(const string &fileName, const int sizeX, const int sizeY,
                     const uint32 *pixel)
{
  FILE *file = fopen(fileName.c_str(), "wb");
  fprintf(file, "P6\n%i %i\n255\n", sizeX, sizeY);
  unsigned char *out = (unsigned char *)alloca(3*sizeX);
  for (int y = 0; y < sizeY; y++) {
    const unsigned char *in = (const unsigned char *)&pixel[(sizeY-1-y)*sizeX];
    for (int x = 0; x < sizeX; x++) {
      out[3*x + 0] = in[4*x + 0];
      out[3*x + 1] = in[4*x + 1];
      out[3*x + 2] = in[4*x +2 ];
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

  //TODO: We need to come up with a better way to handle different possible
  //      pixel layouts
  OSPDataType type = OSP_VOID_PTR;

  if (msgTex->depth == 1) {
    if( msgTex->channels == 3 ) type = OSP_UCHAR3;
    if( msgTex->channels == 4 ) type = OSP_UCHAR4;
  } else if (msgTex->depth == 4) {
    if( msgTex->channels == 3 ) type = OSP_FLOAT3;
    if( msgTex->channels == 4 ) type = OSP_FLOAT3A;
  }

  OSPTexture2D ospTex = ospNewTexture2D(msgTex->width,
                                        msgTex->height,
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

  static std::map<ospray::miniSG::Material*, OSPMaterial> alreadyCreatedMaterials;

  if (alreadyCreatedMaterials.find(mat) != alreadyCreatedMaterials.end())
    return alreadyCreatedMaterials[mat];

  const char *type = mat->getParam("type","OBJMaterial");
  assert(type);
  OSPMaterial ospMat = alreadyCreatedMaterials[mat]
      = ospNewMaterial(renderer,type);
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
      ospSet1f(ospMat,name,f);
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

static void loadModel(OSPRayFixture *f)
{
  embree::FileName fn = BENCH_MODEL;
  ospray::miniSG::importOBJ(f->sgModel, fn);
}

static void createOSPModel(OSPRayFixture *f)
{
  f->model = ospNewModel();

  for (size_t i = 0; i < f->sgModel.mesh.size(); i++) {
    ospray::Ref<ospray::miniSG::Mesh> msgMesh = f->sgModel.mesh[i];

    // create ospray mesh
    OSPGeometry ospMesh = ospNewGeometry("trianglemesh");

    // check if we have to transform the vertices:
    if (f->sgModel.instance[i] != ospray::miniSG::Instance(i)) {
      for (size_t vID = 0; vID < msgMesh->position.size(); vID++) {
        msgMesh->position[vID] = xfmPoint(f->sgModel.instance[i].xfm,
                                          msgMesh->position[vID]);
      }
    }

    // add position array to mesh
    OSPData position = ospNewData(msgMesh->position.size(),
                                  OSP_FLOAT3A,
                                  &msgMesh->position[0],
                                  OSP_DATA_SHARED_BUFFER);
    ospSetData(ospMesh,"position",position);

    // add triangle index array to mesh
    if (!msgMesh->triangleMaterialId.empty()) {
      OSPData primMatID = ospNewData(msgMesh->triangleMaterialId.size(),
                                     OSP_INT,
                                     &msgMesh->triangleMaterialId[0],
                                     OSP_DATA_SHARED_BUFFER);
      ospSetData(ospMesh, "prim.materialID", primMatID);
    }

    // add triangle index array to mesh
    OSPData index = ospNewData(msgMesh->triangle.size(),
                               OSP_INT3,
                               &msgMesh->triangle[0],
                               OSP_DATA_SHARED_BUFFER);
    assert(msgMesh->triangle.size() > 0);
    ospSetData(ospMesh,"index",index);

    // add normal array to mesh
    if (!msgMesh->normal.empty()) {
      OSPData normal = ospNewData(msgMesh->normal.size(),
                                  OSP_FLOAT3A,
                                  &msgMesh->normal[0],
                                  OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->normal.size() > 0);
      ospSetData(ospMesh,"vertex.normal",normal);
    }

    // add color array to mesh
    if (!msgMesh->color.empty()) {
      OSPData color = ospNewData(msgMesh->color.size(),
                                 OSP_FLOAT3A,
                                 &msgMesh->color[0],
                                 OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->color.size() > 0);
      ospSetData(ospMesh,"vertex.color",color);
    }

    // add texcoord array to mesh
    if (!msgMesh->texcoord.empty()) {
      OSPData texcoord = ospNewData(msgMesh->texcoord.size(),
                                    OSP_FLOAT2,
                                    &msgMesh->texcoord[0],
                                    OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->texcoord.size() > 0);
      ospSetData(ospMesh,"vertex.texcoord",texcoord);
    }

    ospSet1i(ospMesh, "alpha_type", 0);
    ospSet1i(ospMesh, "alpha_component", 4);

    // add triangle material id array to mesh
    if (msgMesh->materialList.empty()) {
      // we have a single material for this mesh...
      OSPMaterial singleMaterial = createMaterial(f->renderer,
                                                  msgMesh->material.ptr);
      ospSetMaterial(ospMesh,singleMaterial);
    } else {
      // we have an entire material list, assign that list
      std::vector<OSPMaterial > materialList;
      std::vector<OSPTexture2D > alphaMaps;
      std::vector<float> alphas;
      for (int i=0;i<msgMesh->materialList.size();i++) {
        materialList.push_back(
              createMaterial(f->renderer, msgMesh->materialList[i].ptr)
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
      OSPData ospMaterialList = ospNewData(materialList.size(),
                                           OSP_OBJECT,
                                           &materialList[0]);
      ospSetData(ospMesh,"materialList",ospMaterialList);
    }

    ospCommit(ospMesh);
    ospAddGeometry(f->model,ospMesh);
  }

  ospCommit(f->model);
}

static void createOSPCamera(OSPRayFixture *f)
{
  f->camera = ospNewCamera("perspective");

  const box3f worldBounds(f->sgModel.getBBox());
  vec3f center = embree::center(worldBounds);
  vec3f diag   = worldBounds.size();
  diag         = max(diag,vec3f(0.3f*length(diag)));
  vec3f from   = center - .75f*vec3f(-.6*diag.x,-1.2*diag.y,.8*diag.z);
  vec3f dir    = center - from;
  vec3f up     = vec3f(0.f, 1.f, 0.f);

#if 0
  ospSetVec3f(f->camera, "pos", from);
  ospSetVec3f(f->camera, "dir", dir);
  ospSetVec3f(f->camera, "up",  up);
#else
  // city model view
  ospSetVec3f(f->camera, "pos", vec3f(33398.523438, 20320.738281, -35626.101562));
  ospSetVec3f(f->camera, "dir", vec3f(14588.197266, 4275.844727, -10825.982422));
  ospSetVec3f(f->camera, "up",  vec3f(0, 1, 0));
#endif

  ospSetf(f->camera, "aspect", 1.333f);
  ospCommit(f->camera);
}

static void createOSPRenderer(OSPRayFixture *f)
{
  f->renderer = ospNewRenderer("ao1");
  ospSetVec3f(f->renderer, "bgcolor", vec3f(1.f, 1.f, 1.f));
}

static void createFramebuffer(OSPRayFixture *f)
{
  f->fb = ospNewFrameBuffer(vec2i(1024, 1024), OSP_RGBA_I8,
                            OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
  ospSet1f(f->fb, "gamma", 2.2f);
  ospCommit(f->fb);
  ospFrameBufferClear(f->fb, OSP_FB_ACCUM);
}

void OSPRayFixture::SetUp()
{
  const char *av[] = {"ospBenchmark"};
  int ac = 1;
  ospInit(&ac, av);

  loadModel(this);
  createOSPRenderer(this);
  createOSPModel(this);
  createOSPCamera(this);
  createFramebuffer(this);

  ospSetObject(renderer, "world",  model);
  ospSetObject(renderer, "model",  model);
  ospSetObject(renderer, "camera", camera);
  ospSet1i(renderer, "spp", 1);

  ospCommit(renderer);

#if 1
  ospRenderFrame(fb, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);
  auto *lfb = (uint32*)ospMapFrameBuffer(fb, OSP_FB_COLOR);
# if 0
  writePPM("test.ppm", 1024, 1024, lfb);
# else
  bool hasColor = false;
  for (int i = 0; i < 1024*1024; ++i) {
    if ((lfb[i] & 0xFFFFFF00) > 0) {
      hasColor = true;
      std::cerr << "got color: " << lfb[i] << std::endl;
      break;
    }
  }

  if (hasColor) {
    std::cerr << "IMAGE HAS COLOR!!!" << std::endl;
  } else {
    std::cerr << "IMAGE HAS NO COLOR..." << std::endl;
  }
# endif
  ospUnmapFrameBuffer(lfb, fb);
#endif
}

void OSPRayFixture::TearDown()
{
  ospFreeFrameBuffer(fb);
}
