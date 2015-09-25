#include "CommandLineSceneBuilder.h"

using std::cerr;
using std::cout;
using std::endl;

// Static local helper functions //////////////////////////////////////////////

static void error(const std::string &msg)
{
  cerr << "#ospModelViewer fatal error : " << msg << endl;
  cerr << endl;
  cerr << "Proper usage: " << endl;
  cerr << "  ./ospModelViewer"
       << " [-bench <warmpup>x<numFrames>] [-model] <inFileName>" << endl;
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

// CommandLineSceneBuilder definitions ////////////////////////////////////////

namespace ospray {

CommandLineSceneBuilder::CommandLineSceneBuilder(int ac, const char **&av) :
  m_model(NULL),
  m_renderer(NULL),
  m_camera(NULL),
  m_msgModel(NULL),
  m_defaultDirLight_direction(.3, -1, -.2),
  m_alpha(false),
  m_createDefaultMaterial(true),
  m_spp(1),
  m_naos(0),
  m_aorl(-1.f),
  m_maxObjectsToConsider((uint32)-1),
  m_forceInstancing(false),
  m_frameBufferMode(glut3D::Glut3DWidget::FRAMEBUFFER_UCHAR),
  m_rendererType("ao1"),
  m_cameraType("perspective")
{
  m_msgModel = new miniSG::Model;

  parseCommandLine(ac, av);

  if (m_config.verboseOutput) {
    reportParsedData();
  }

  createScene();

  m_camera = ospNewCamera(m_cameraType.c_str());
  Assert(m_camera != NULL && "could not create camera");
  ospSet3f(m_camera, "pos", -1,  1, -1);
  ospSet3f(m_camera, "dir",  1, -1,  1);
  ospCommit(m_camera);

  ospSetObject(m_renderer, "world", m_model);
  ospSetObject(m_renderer, "model", m_model);
  ospSetObject(m_renderer, "camera",m_camera);
  ospSet1i(m_renderer, "spp", m_spp);
  if (m_naos > 0)   ospSet1i(m_renderer, "aoSamples", m_naos);
  if (m_aorl > 0.f) ospSet1f(m_renderer, "aoOcclusionDistance", m_aorl);
  ospCommit(m_camera);
  ospCommit(m_renderer);

  if (m_config.verboseOutput) {
    cout << "#ospModelViewer: done creating window. Press 'Q' to quit." << endl;
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
    cout << "#ospModelViewer: starting to process cmdline arguments" << endl;
  }

  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "--renderer") {
      assert(i+1 < ac);
      m_rendererType = av[++i];
    } else if (arg == "-v") {
      m_config.verboseOutput = true;
    } else if (arg == "-o") {
      m_config.outFileName = strdup(av[++i]);
    } else if (arg == "-o:nacc") {
      m_config.numAccumsFrameInFileOutput = atoi(av[++i]);
    } else if (arg == "-o:spp") {
      m_config.numSPPinFileOutput = atoi(av[++i]);
    } else if (arg == "--max-objects") {
      m_maxObjectsToConsider = atoi(av[++i]);
    } else if (arg == "--spp" || arg == "-spp") {
      m_spp = atoi(av[++i]);
    } else if (arg == "--nacc" || arg == "-nacc") {
      m_config.maxAccum = atoi(av[++i]);
    } else if (arg == "--aos" || arg == "-aos") {
      m_naos = atoi(av[++i]);
    } else if (arg == "--aod" || arg == "-aod") {
      m_aorl = atof(av[++i]);
    } else if (arg == "--camera" || arg == "-c") {
      m_cameraType = std::string(av[++i]);
    } else if (arg == "--force-instancing") {
      m_forceInstancing = true;
    } else if (arg == "--pt") {
      m_config.maxAccum = 1024;
      m_rendererType = "pathtracer";
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
    } else if (arg == "-bench") {
      if (++i < ac)
      {
        std::string arg2(av[i]);
        size_t pos = arg2.find("x");
        if (pos != std::string::npos)
        {
          arg2.replace(pos, 1, " ");
          std::stringstream ss(arg2);
          ss >> m_config.benchWarmup >> m_config.benchFrames;
        }
      }
    } else if (arg == "--no-default-material") {
      m_createDefaultMaterial = false;
    } else if (av[i][0] == '-') {
      error("unknown commandline argument '"+arg+"'");
    } else {
      embree::FileName fn = arg;
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
  cout << "#ospModelViewer: done parsing. found model with" << endl;
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

void CommandLineSceneBuilder::createScene()
{
  // -------------------------------------------------------
  // create ospray model
  // -------------------------------------------------------
  m_model = ospNewModel();

  m_renderer = ospNewRenderer(m_rendererType.c_str());
  if (!m_renderer) {
    throw std::runtime_error("could not create m_renderer '" +
                             m_rendererType + "'");
  }
  Assert(m_renderer != NULL && "could not create m_renderer");
  ospCommit(m_renderer);

  // code does not yet do instancing ... check that the model doesn't
  // contain instances
  bool doesInstancing = 0;

  if (m_forceInstancing) {
    if (m_config.verboseOutput) {
      cout << "#ospModelViewer: forced instancing - instances on." << endl;
    }
    doesInstancing = true;
  } else if (m_msgModel->instance.size() > m_msgModel->mesh.size()) {
    if (m_config.verboseOutput) {
      cout << "#ospModelViewer: found more object instances than meshes "
           << "- turning on instancing" << endl;
    }
    doesInstancing = true;
  } else {
    if (m_config.verboseOutput) {
      cout << "#ospModelViewer: number of instances matches number of "
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
    cout << "#ospModelViewer: adding parsed geometries to ospray model" << endl;
  }
  std::vector<OSPModel> instanceModels;

  for (size_t i=0;i<m_msgModel->mesh.size();i++) {
    Ref<miniSG::Mesh> msgMesh = m_msgModel->mesh[i];

    // create ospray mesh
    OSPGeometry ospMesh = m_alpha ?
          ospNewGeometry("alpha_aware_triangle_mesh") :
          ospNewTriangleMesh();

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
    ospSetData(ospMesh,"position",position);

    // add triangle index array to mesh
    if (!msgMesh->triangleMaterialId.empty()) {
      OSPData primMatID = ospNewData(msgMesh->triangleMaterialId.size(),
                                     OSP_INT,
                                     &msgMesh->triangleMaterialId[0],
          OSP_DATA_SHARED_BUFFER);
      ospSetData(ospMesh,"prim.materialID",primMatID);
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
      OSPMaterial singleMaterial = createMaterial(m_renderer,
                                                  msgMesh->material.ptr);
      ospSetMaterial(ospMesh,singleMaterial);
    } else {
      // we have an entire material list, assign that list
      std::vector<OSPMaterial > materialList;
      std::vector<OSPTexture2D > alphaMaps;
      std::vector<float> alphas;
      for (int i=0;i<msgMesh->materialList.size();i++) {
        materialList.push_back(
              createMaterial(m_renderer, msgMesh->materialList[i].ptr)
              );

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

      // only set these if alpha aware mode enabled
      // this currently doesn't work on the MICs!
      if(m_alpha) {
        OSPData ospAlphaMapList = ospNewData(alphaMaps.size(),
                                             OSP_OBJECT,
                                             &alphaMaps[0]);
        ospSetData(ospMesh, "alpha_maps", ospAlphaMapList);

        OSPData ospAlphaList = ospNewData(alphas.size(),
                                          OSP_OBJECT,
                                          &alphas[0]);
        ospSetData(ospMesh, "alphas", ospAlphaList);
      }
    }

    ospCommit(ospMesh);

    if (doesInstancing) {
      OSPModel model_i = ospNewModel();
      ospAddGeometry(model_i,ospMesh);
      ospCommit(model_i);
      instanceModels.push_back(model_i);
    } else
      ospAddGeometry(m_model,ospMesh);

  }

  if (doesInstancing) {
    for (int i=0;i<m_msgModel->instance.size();i++) {
      OSPGeometry inst =
          ospNewInstance(instanceModels[m_msgModel->instance[i].meshID],
          m_msgModel->instance[i].xfm);
      ospAddGeometry(m_model,inst);
    }
  }

  if (m_config.verboseOutput) {
    cout << "#m_modelViewer: committing model" << endl;
  }

  ospCommit(m_model);

  if (m_config.verboseOutput) {
    cout << "#m_modelViewer: done creating ospray model." << endl;
  }

  //TODO: Need to figure out where we're going to read lighting data from
  //begin light test
  std::vector<OSPLight> lights;
  if (m_defaultDirLight_direction != vec3f(0.f)) {
    if (m_config.verboseOutput) {
      cout << "#m_modelViewer: Adding a hard coded directional "
           << "light as the sun." << endl;
    }
    OSPLight ospLight = ospNewLight(m_renderer, "DirectionalLight");
    ospSetString(ospLight, "name", "sun" );
    ospSet3f(ospLight, "color", 1, 1, 1);
    ospSet3fv(ospLight, "direction", &m_defaultDirLight_direction.x);
    ospSet1f(ospLight, "angularDiameter", 0.53f);
    ospCommit(ospLight);
    lights.push_back(ospLight);
  }

  OSPData lightArray = ospNewData(lights.size(), OSP_OBJECT, &lights[0], 0);
  ospSetData(m_renderer, "lights", lightArray);
  //end light test
  ospCommit(m_renderer);
}

OSPMaterial
ospray::CommandLineSceneBuilder::createDefaultMaterial(OSPRenderer renderer)
{
  if(!m_createDefaultMaterial) return NULL;

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

OSPMaterial CommandLineSceneBuilder::createMaterial(OSPRenderer renderer,
                                                    miniSG::Material *mat)
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
  static std::map<miniSG::Material *,OSPMaterial> alreadyCreatedMaterials;

  if (alreadyCreatedMaterials.find(mat) != alreadyCreatedMaterials.end())
    return alreadyCreatedMaterials[mat];

  const char *type = mat->getParam("type","OBJMaterial");
  assert(type);
  OSPMaterial ospMat = alreadyCreatedMaterials[mat]
      = ospNewMaterial(renderer,type);
  if (!ospMat)
  {
    warnMaterial(type);
    return createDefaultMaterial(renderer);
  }

  const bool isOBJMaterial = !strcmp(type, "OBJMaterial");

  for (miniSG::Material::ParamMap::const_iterator it =  mat->params.begin();
       it !=  mat->params.end(); ++it)
  {
    const char *name = it->first.c_str();
    const miniSG::Material::Param *p = it->second.ptr;

    switch(p->type) {
    case miniSG::Material::Param::INT:
      ospSet1i(ospMat,name,p->i[0]);
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
      ospSet1f(ospMat,name,f);
    } break;
    case miniSG::Material::Param::FLOAT_3:
      ospSet3fv(ospMat,name,p->f);
      break;
    case miniSG::Material::Param::STRING:
      ospSetString(ospMat,name,p->s);
      break;
    case miniSG::Material::Param::TEXTURE:
    {
      miniSG::Texture2D *tex = (miniSG::Texture2D*)p->ptr;
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

}// namespace ospray
