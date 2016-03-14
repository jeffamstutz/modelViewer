#include "CommandLineSceneBuilder.h"

#include <ospray_cpp/Data.h>

#include <random>

using std::cerr;
using std::cout;
using std::endl;

using namespace ospcommon;

// Static local helper functions //////////////////////////////////////////////

static void error(const std::string &msg)
{
  cerr << "#ospDebugViewer fatal error : " << msg << endl;
  cerr << endl;
  cerr << "Proper usage: " << endl;
  cerr << "  ./ospDebugViewer" << " <inFileName> [options]" << endl;
  cerr << endl;
  exit(1);
}

static void warnMaterial(const std::string &type)
{
  static std::map<std::string,int> numOccurances;
  if (numOccurances[type] == 0)
  {
    cerr << "could not create material type '"<<  type <<
            "'. Replacing with default material." << endl;
  }
  numOccurances[type]++;
}

static OSPTexture2D createTexture2D(ospray::miniSG::Texture2D *msgTex)
{
  if(msgTex == nullptr)
  {
    static int numWarnings = 0;
    if (++numWarnings < 10)
    {
      cerr << "WARNING: material does not have Textures"
           << " (only warning for the first 10 times)!" << endl;
    }
    return nullptr;
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

// CommandLineSceneBuilder definitions ////////////////////////////////////////

namespace ospray {

CommandLineSceneBuilder::CommandLineSceneBuilder(int ac, const char **&av) :
  m_renderer(nullptr),
  m_camera(nullptr),
  m_defaultDirLight_direction(.3, -1, -.2),
  m_alpha(false),
  m_createDefaultMaterial(true),
  m_spp(1),
  m_maxObjectsToConsider((uint32_t)-1),
  m_forceInstancing(false),
  m_rendererType("ao1"),
  m_cameraType("perspective")
{
  m_msgModel = new miniSG::Model;

  parseCommandLine(ac, av);

  if (m_config.verboseOutput) {
    reportParsedData();
  }

  createRenderer();
#if 1
  createScene();
#elif 0
  createSpheres();
#else
  createCylinders();
#endif
  createSunLight();

  m_camera = cpp::Camera(m_cameraType.c_str());
  Assert(m_camera.handle() != nullptr && "could not create camera");
  m_camera.set("pos", -1,  1, -1);
  m_camera.set("dir",  1, -1,  1);
  m_camera.commit();

  m_renderer.set("world",  m_model);
  m_renderer.set("model",  m_model);
  m_renderer.set("camera", m_camera);
  m_renderer.set("spp", m_spp);
  m_renderer.commit();

  if (m_config.verboseOutput) {
    cout << "#ospDebugViewer: done creating window. Press 'Q' to quit." << endl;
  }
}

void CommandLineSceneBuilder::parseCommandLine(int ac, const char **&av)
{
  // Check if verbose output is turned on //

  for (int i = 1; i < ac; ++i)
  {
    const std::string arg = av[i];
    if (arg == "-v") {
      m_config.verboseOutput = true;
      break;
    }
  }

  // Parse the rest of the arguments

  if (m_config.verboseOutput) {
    cout << "#ospDebugViewer: starting to process cmdline arguments" << endl;
  }

  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "--renderer") {
      assert(i+1 < ac);
      m_rendererType = av[++i];
    } else if (arg == "-v") {
      m_config.verboseOutput = true;
    } else if (arg == "-s") {
      m_config.scriptFileName = av[++i];
    } else if (arg == "--max-objects") {
      m_maxObjectsToConsider = atoi(av[++i]);
    } else if (arg == "--camera" || arg == "-c") {
      m_cameraType = av[++i];
    } else if (arg == "--force-instancing") {
      m_forceInstancing = true;
    } else if (arg == "--sun-dir") {
      if (!strcmp(av[i+1],"none")) {
        m_defaultDirLight_direction = vec3f(0.f);
      } else {
        m_defaultDirLight_direction.x = atof(av[++i]);
        m_defaultDirLight_direction.y = atof(av[++i]);
        m_defaultDirLight_direction.z = atof(av[++i]);
      }
    } else if (arg == "--module" || arg == "--plugin") {
      assert(i+1 < ac);
      const char *moduleName = av[++i];
      if (m_config.verboseOutput) {
        cout << "loading ospray module '" << moduleName << "'" << endl;
      }
      ospLoadModule(moduleName);
    } else if (arg == "--alpha") {
      m_alpha = true;
    } else if (arg == "--no-default-material") {
      m_createDefaultMaterial = false;
    } else if (av[i][0] == '-') {
      error("unknown commandline argument '" + arg + "'");
    } else {
      FileName fn = arg;
      if (fn.ext() == "stl") {
        miniSG::importSTL(*m_msgModel,fn);
      } else if (fn.ext() == "msg") {
        miniSG::importMSG(*m_msgModel,fn);
      } else if (fn.ext() == "tri") {
        miniSG::importTRI(*m_msgModel,fn);
      } else if (fn.ext() == "xml") {
        miniSG::importRIVL(*m_msgModel,fn);
      } else if (fn.ext() == "obj") {
        miniSG::importOBJ(*m_msgModel,fn);
      } else if (fn.ext() == "hbp") {
        miniSG::importHBP(*m_msgModel,fn);
      } else if (fn.ext() == "x3d") {
        miniSG::importX3D(*m_msgModel,fn);
      } else if (fn.ext() == "astl") {
        miniSG::importSTL(m_msgAnimation,fn);
      }
    }
  }
}

void CommandLineSceneBuilder::reportParsedData()
{
  cout << "#ospDebugViewer: done parsing. found model with" << endl;
  cout << "  - num meshes   : " << m_msgModel->mesh.size() << " ";
  size_t numUniqueTris = 0;
  size_t numInstancedTris = 0;
  for (size_t  i=0;i<m_msgModel->mesh.size();i++) {
    if (i < 10)
      cout << "[" << m_msgModel->mesh[i]->size() << "]";
    else
      if (i == 10) cout << "...";
    numUniqueTris += m_msgModel->mesh[i]->size();
  }
  cout << endl;
  cout << "  - num instances: " << m_msgModel->instance.size() << " ";
  for (size_t i = 0; i < m_msgModel->instance.size(); i++) {
    if (i < 10) {
      cout << "["
           << m_msgModel->mesh[m_msgModel->instance[i].meshID]->size()
           << "]";
    } else {
      if (i == 10)
        cout << "...";
    }
    numInstancedTris +=
        m_msgModel->mesh[m_msgModel->instance[i].meshID]->size();
  }
  cout << endl;
  cout << "  - num unique triangles   : " << numUniqueTris << endl;
  cout << "  - num instanced triangles: " << numInstancedTris << endl;

  if (numInstancedTris == 0 && m_msgAnimation.empty())
    error("no (valid) input files specified - model contains no triangles");
}

void CommandLineSceneBuilder::createRenderer()
{
  m_renderer = cpp::Renderer(m_rendererType.c_str());
  m_renderer.commit();
}

void CommandLineSceneBuilder::createScene()
{
  // code does not yet do instancing ... check that the model doesn't
  // contain instances
  bool doesInstancing = 0;

  if (m_forceInstancing) {
    if (m_config.verboseOutput) {
      cout << "#ospDebugViewer: forced instancing - instances on." << endl;
    }
    doesInstancing = true;
  } else if (m_msgModel->instance.size() > m_msgModel->mesh.size()) {
    if (m_config.verboseOutput) {
      cout << "#ospDebugViewer: found more object instances than meshes "
           << "- turning on instancing" << endl;
    }
    doesInstancing = true;
  } else {
    if (m_config.verboseOutput) {
      cout << "#ospDebugViewer: number of instances matches number of "
           << "meshes, creating single model that contains all meshes" << endl;
    }
    doesInstancing = false;
  }

  if (m_msgModel->instance.size() > m_maxObjectsToConsider) {
    if (m_config.verboseOutput) {
      cout << "cutting down on the number of meshes as requested "
           << "on cmdline..." << endl;
    }
    m_msgModel->instance.resize(m_maxObjectsToConsider);
  }

  if (doesInstancing) {
    m_msgModel->mesh.resize(m_maxObjectsToConsider);
  }


  if (m_config.verboseOutput) {
    cout << "#ospDebugViewer: adding parsed geometries to ospray model" << endl;
  }
  std::vector<OSPModel> instanceModels;

  for (size_t i=0;i<m_msgModel->mesh.size();i++) {
    Ref<miniSG::Mesh> msgMesh = m_msgModel->mesh[i];

    // create ospray mesh
    auto ospMesh = m_alpha ? cpp::Geometry("alpha_aware_triangle_mesh") :
                             cpp::Geometry("triangles");

    // check if we have to transform the vertices:
    if (doesInstancing == false &&
        m_msgModel->instance[i] != miniSG::Instance(i)) {
      for (size_t vID=0;vID<msgMesh->position.size();vID++) {
        msgMesh->position[vID] = xfmPoint(m_msgModel->instance[i].xfm,
                                          msgMesh->position[vID]);
      }
    }

    // add position array to mesh
    OSPData position = ospNewData(msgMesh->position.size(),
                                  OSP_FLOAT3A,
                                  &msgMesh->position[0],
        OSP_DATA_SHARED_BUFFER);
    ospMesh.set("position", position);

    // add triangle index array to mesh
    if (!msgMesh->triangleMaterialId.empty()) {
      OSPData primMatID = ospNewData(msgMesh->triangleMaterialId.size(),
                                     OSP_INT,
                                     &msgMesh->triangleMaterialId[0],
          OSP_DATA_SHARED_BUFFER);
      ospMesh.set("prim.materialID", primMatID);
    }

    // add triangle index array to mesh
    OSPData index = ospNewData(msgMesh->triangle.size(),
                               OSP_INT3,
                               &msgMesh->triangle[0],
        OSP_DATA_SHARED_BUFFER);
    assert(msgMesh->triangle.size() > 0);
    ospMesh.set("index", index);

    // add normal array to mesh
    if (!msgMesh->normal.empty()) {
      OSPData normal = ospNewData(msgMesh->normal.size(),
                                  OSP_FLOAT3A,
                                  &msgMesh->normal[0],
          OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->normal.size() > 0);
      ospMesh.set("vertex.normal", normal);
    }

    // add color array to mesh
    if (!msgMesh->color.empty()) {
      OSPData color = ospNewData(msgMesh->color.size(),
                                 OSP_FLOAT3A,
                                 &msgMesh->color[0],
          OSP_DATA_SHARED_BUFFER);
      assert(msgMesh->color.size() > 0);
      ospMesh.set("vertex.color", color);
    }
    // add texcoord array to mesh
    if (!msgMesh->texcoord.empty()) {
      OSPData texcoord = ospNewData(msgMesh->texcoord.size(),
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
      auto singleMaterial = createMaterial(m_renderer, msgMesh->material.ptr);
      ospMesh.setMaterial(singleMaterial);
    } else {
      // we have an entire material list, assign that list
      std::vector<OSPMaterial> materialList;
      std::vector<OSPTexture2D> alphaMaps;
      std::vector<float> alphas;
      for (int i=0;i<msgMesh->materialList.size();i++) {
        auto m = (OSPMaterial)createMaterial(m_renderer,
                                        msgMesh->materialList[i].ptr).handle();
        materialList.push_back(m);

        for (miniSG::Material::ParamMap::const_iterator it =
             msgMesh->materialList[i]->params.begin();
             it != msgMesh->materialList[i]->params.end(); it++) {
          const char *name = it->first.c_str();
          const miniSG::Material::Param *p = it->second.ptr;
          if(p->type == miniSG::Material::Param::TEXTURE) {
            if(!strcmp(name, "map_kd") || !strcmp(name, "map_Kd")) {
              miniSG::Texture2D *tex = (miniSG::Texture2D*)p->ptr;
              OSPTexture2D ospTex = createTexture2D(tex);
              ospCommit(ospTex);
              alphaMaps.push_back(ospTex);
            }
          } else if(p->type == miniSG::Material::Param::FLOAT) {
            if(!strcmp(name, "d")) alphas.push_back(p->f[0]);
          }
        }

        while(materialList.size() > alphaMaps.size()) {
          alphaMaps.push_back(nullptr);
        }
        while(materialList.size() > alphas.size()) {
          alphas.push_back(0.f);
        }
      }
      auto ospMaterialList = cpp::Data(materialList.size(),
                                       OSP_OBJECT,
                                       &materialList[0]);
      ospMesh.set("materialList", ospMaterialList);

      // only set these if alpha aware mode enabled
      // this currently doesn't work on the MICs!
      if(m_alpha) {
        auto ospAlphaMapList = cpp::Data(alphaMaps.size(),
                                         OSP_OBJECT,
                                         &alphaMaps[0]);
        ospMesh.set("alpha_maps", ospAlphaMapList);

        auto ospAlphaList = cpp::Data(alphas.size(),
                                      OSP_OBJECT,
                                      &alphas[0]);
        ospMesh.set("alphas", ospAlphaList);
      }
    }

    ospMesh.commit();

    if (doesInstancing) {
      cpp::Model model_i;
      model_i.addGeometry(ospMesh);
      model_i.commit();
      instanceModels.push_back((OSPModel)model_i.handle());
    } else {
      m_model.addGeometry(ospMesh);
    }
  }

  if (doesInstancing) {
    for (int i=0;i<m_msgModel->instance.size();i++) {
      OSPGeometry inst =
          ospNewInstance(instanceModels[m_msgModel->instance[i].meshID],
          reinterpret_cast<osp::affine3f&>(m_msgModel->instance[i].xfm));
      m_model.addGeometry(inst);
    }
  }

  if (m_config.verboseOutput) {
    cout << "#m_modelViewer: committing model" << endl;
  }

  m_model.commit();

  if (m_config.verboseOutput) {
    cout << "#m_modelViewer: done creating ospray model." << endl;
  }
}

void CommandLineSceneBuilder::createSpheres()
{
  struct Sphere {
    vec3f center;
    uint  colorID;
  };

  std::vector<Sphere> spheres;
  std::vector<vec4f>  colors;

#define NUM_SPHERES 100000
#define NUM_COLORS 10

  spheres.resize(NUM_SPHERES);
  colors.resize(NUM_SPHERES);

  std::default_random_engine rng;
  std::uniform_real_distribution<float> vdist(-1000.0f, 1000.0f);
  std::uniform_real_distribution<float> cdist(0.0f, 1.0f);
  std::uniform_int_distribution<uint>   ciddist(0, NUM_COLORS-1);

  for (int i = 0; i < NUM_SPHERES; i++) {
    spheres[i].center.x = vdist(rng);
    spheres[i].center.y = vdist(rng);
    spheres[i].center.z = vdist(rng);
    spheres[i].colorID  = ciddist(rng);
  }

  for (int i = 0; i < NUM_COLORS; i++) {
    colors[i].x = cdist(rng);
    colors[i].y = cdist(rng);
    colors[i].z = cdist(rng);
    colors[i].w = 1.0f;
  }

  auto sphereData = cpp::Data(sizeof(Sphere)*NUM_SPHERES, OSP_CHAR,
                              spheres.data());
  auto colorData  = cpp::Data(NUM_COLORS, OSP_FLOAT4, colors.data());

  sphereData.commit();
  colorData.commit();

  auto geometry = cpp::Geometry("spheres");
  geometry.set("spheres", sphereData);
  geometry.set("color",   colorData);
  geometry.set("radius", 10.f);
  geometry.set("bytes_per_sphere", int(sizeof(Sphere)));
  geometry.set("offset_colorID", int(sizeof(vec3f)));
  geometry.commit();

  m_model.addGeometry(geometry);
  m_model.commit();
}

void CommandLineSceneBuilder::createCylinders()
{
  struct Cylinder {
    vec3f v0;
    vec3f v1;
    uint  colorID;
  };

  std::vector<Cylinder> cylinders;
  std::vector<vec4f>  colors;

#define NUM_CYLINDERS 100
#define NUM_COLORS 10

  cylinders.resize(NUM_CYLINDERS);
  colors.resize(NUM_CYLINDERS);

  std::default_random_engine rng;
  std::uniform_real_distribution<float> vdist(-1000.0f, 1000.0f);
  std::uniform_real_distribution<float> cdist(0.0f, 1.0f);
  std::uniform_int_distribution<uint>   ciddist(0, NUM_COLORS-1);

  for (int i = 0; i < NUM_CYLINDERS; i++) {
    cylinders[i].v0.x = vdist(rng);
    cylinders[i].v0.y = vdist(rng);
    cylinders[i].v0.z = vdist(rng);
    cylinders[i].v1.x = vdist(rng);
    cylinders[i].v1.y = vdist(rng);
    cylinders[i].v1.z = vdist(rng);
    cylinders[i].colorID  = ciddist(rng);
  }

  for (int i = 0; i < NUM_COLORS; i++) {
    colors[i].x = cdist(rng);
    colors[i].y = cdist(rng);
    colors[i].z = cdist(rng);
    colors[i].w = 1.0f;
  }

  auto cylinderData = cpp::Data(sizeof(Cylinder)*NUM_CYLINDERS, OSP_CHAR,
                                cylinders.data());
  auto colorData  = cpp::Data(NUM_COLORS, OSP_FLOAT4, colors.data());

  cylinderData.commit();
  colorData.commit();

  auto geometry = cpp::Geometry("cylinders");
  geometry.set("cylinders", cylinderData);
  geometry.set("color",   colorData);
  geometry.set("radius", 10.f);
  geometry.set("bytes_per_cylinder", int(sizeof(Cylinder)));
  geometry.set("offset_colorID", int(2*sizeof(vec3f)));
  geometry.commit();

  m_model.addGeometry(geometry);
  m_model.commit();
}

void CommandLineSceneBuilder::createSunLight()
{
  //TODO: Need to figure out where we're going to read lighting data from
  std::vector<OSPLight> lights;
  if (m_defaultDirLight_direction != vec3f(0.f)) {
    if (m_config.verboseOutput) {
      cout << "#m_modelViewer: Adding a hard coded directional "
           << "light as the sun." << endl;
    }
    auto ospLight = m_renderer.newLight("DirectionalLight");
    if (ospLight.handle() == nullptr) {
      throw std::runtime_error("Failed to create a 'DirectionalLight'!");
    }
    ospLight.set("name", "sun");
    ospLight.set("color", 1.f, 1.f, 1.f);
    ospLight.set("direction", m_defaultDirLight_direction);
    ospLight.set("angularDiameter", 0.53f);
    ospLight.commit();
    lights.push_back((OSPLight)ospLight.handle());
  }

  auto lightArray = cpp::Data(lights.size(), OSP_OBJECT, lights.data());
  //lightArray.commit();
  m_renderer.set("lights", lightArray);
}

cpp::Material
CommandLineSceneBuilder::createDefaultMaterial(cpp::Renderer renderer)
{
  if(!m_createDefaultMaterial) return nullptr;

  static auto ospMat = cpp::Material(nullptr);

  if (ospMat.handle()) return ospMat;

  ospMat = renderer.newMaterial("OBJMaterial");

  ospMat.set("Kd", .8f, 0.f, 0.f);
  ospMat.commit();
  return ospMat;
}

cpp::Material CommandLineSceneBuilder::createMaterial(cpp::Renderer renderer,
                                                      miniSG::Material *mat)
{
  if (mat == nullptr)
  {
    static int numWarnings = 0;
    if (++numWarnings < 10)
    {
      cerr << "WARNING: model does not have materials! "
           << "(assigning default)" << endl;
    }
    return createDefaultMaterial(renderer);
  }
  static std::map<miniSG::Material *, cpp::Material> alreadyCreatedMaterials;

  if (alreadyCreatedMaterials.find(mat) != alreadyCreatedMaterials.end()) {
    return alreadyCreatedMaterials[mat];
  }

  const char *type = mat->getParam("type", "OBJMaterial");
  assert(type);

  cpp::Material ospMat;
  try {
    ospMat = alreadyCreatedMaterials[mat] = renderer.newMaterial(type);
  } catch (const std::runtime_error &/*e*/) {
    warnMaterial(type);
    return createDefaultMaterial(renderer);
  }

  const bool isOBJMaterial = !strcmp(type, "OBJMaterial");

  for (auto it =  mat->params.begin(); it !=  mat->params.end(); ++it) {
    const char *name = it->first.c_str();
    const miniSG::Material::Param *p = it->second.ptr;

    switch(p->type) {
    case miniSG::Material::Param::INT:
      ospMat.set(name, p->i[0]);
      break;
    case miniSG::Material::Param::FLOAT: {
      float f = p->f[0];
      /* many mtl materials of obj models wrongly store the phong exponent
         'Ns' in range [0..1], whereas OSPRay's material implementations
         correctly interpret it to be in [0..inf), thus we map ranges here */
      if (isOBJMaterial &&
          (!strcmp(name, "Ns") || !strcmp(name, "ns")) &&
          f < 1.f) {
        f = 1.f/(1.f - f) - 1.f;
      }
      ospMat.set(name, f);
    } break;
    case miniSG::Material::Param::FLOAT_3:
     ospMat.set(name, p->f[0], p->f[1], p->f[2]);
      break;
    case miniSG::Material::Param::STRING:
      ospMat.set(name, p->s);
      break;
    case miniSG::Material::Param::TEXTURE:
    {
      miniSG::Texture2D *tex = (miniSG::Texture2D*)p->ptr;
      if (tex) {
        OSPTexture2D ospTex = createTexture2D(tex);
        assert(ospTex);
        ospCommit(ospTex);
        ospMat.set(name, ospTex);
      }
      break;
    }
    default:
      throw std::runtime_error("unknown material parameter type");
    };
  }

  ospMat.commit();
  return ospMat;
}

}// namespace ospray
