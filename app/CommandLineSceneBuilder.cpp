#include "CommandLineSceneBuilder.h"

using std::cout;
using std::endl;

// Static local helper functions //////////////////////////////////////////////

static void error(const std::string &msg)
{
  cout << "#ospModelViewer fatal error : " << msg << endl;
  cout << endl;
  cout << "Proper usage: " << endl;
  cout << "  ./ospModelViewer"
       << " [-bench <warmpup>x<numFrames>] [-model] <inFileName>" << endl;
  cout << endl;
  exit(1);
}

static void warnMaterial(const std::string &type)
{
  static std::map<std::string,int> numOccurances;
  if (numOccurances[type] == 0)
  {
    cout << "could not create material type '"<<  type <<
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
      cout << "WARNING: material does not have Textures"
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
    msgModel(NULL),
    defaultDirLight_direction(.3, -1, -.2),
    g_alpha(false),
    g_createDefaultMaterial(true),
    spp(1),
    maxObjectsToConsider((uint32)-1),
    forceInstancing(false),
    g_frameBufferMode(glut3D::Glut3DWidget::FRAMEBUFFER_UCHAR),
    rendererType("obj")
{
    msgModel = new miniSG::Model;

    parseCommandLine(ac, av);

    reportParsedData();
    createScene();

    m_camera = ospNewCamera("perspective");
    Assert(m_camera != NULL && "could not create camera");
    ospSet3f(m_camera, "pos", -1,  1, -1);
    ospSet3f(m_camera, "dir",  1, -1,  1);
    ospCommit(m_camera);

    ospSetObject(m_renderer, "world", m_model);
    ospSetObject(m_renderer, "model", m_model);
    ospSetObject(m_renderer, "camera",m_camera);
    ospSet1i(m_renderer, "spp", spp);
    ospCommit(m_camera);
    ospCommit(m_renderer);

    printf("#ospModelViewer: done creating window. Press 'Q' to quit.\n");
}

void CommandLineSceneBuilder::parseCommandLine(int ac, const char **&av)
{
    cout << "#ospModelViewer: starting to process cmdline arguments" << endl;
    for (int i=1;i<ac;i++) {
        const std::string arg = av[i];
        if (arg == "--renderer") {
            assert(i+1 < ac);
            rendererType = av[++i];
        } else if (arg == "--always-redraw" || arg == "-fps") {
            m_config.alwaysRedraw = true;
        } else if (arg == "-o") {
            m_config.outFileName = strdup(av[++i]);
        } else if (arg == "-o:nacc") {
            m_config.numAccumsFrameInFileOutput = atoi(av[++i]);
        } else if (arg == "-o:spp") {
            m_config.numSPPinFileOutput = atoi(av[++i]);
        } else if (arg == "--max-objects") {
            maxObjectsToConsider = atoi(av[++i]);
        } else if (arg == "--spp" || arg == "-spp") {
            spp = atoi(av[++i]);
        } else if (arg == "--force-instancing") {
            forceInstancing = true;
        } else if (arg == "--pt") {
            // shortcut for '--renderer pathtracer'
            m_config.maxAccum = 1024;
            rendererType = "pathtracer";
        } else if (arg == "--sun-dir") {
            if (!strcmp(av[i+1],"none")) {
                defaultDirLight_direction = vec3f(0.f);
            } else {
                defaultDirLight_direction.x = atof(av[++i]);
                defaultDirLight_direction.y = atof(av[++i]);
                defaultDirLight_direction.z = atof(av[++i]);
            }
        } else if (arg == "--module" || arg == "--plugin") {
            assert(i+1 < ac);
            const char *moduleName = av[++i];
            cout << "loading ospray module '" << moduleName << "'" << endl;
            ospLoadModule(moduleName);
        } else if (arg == "--alpha") {
            g_alpha = true;
        } else if (arg == "-bench") {
            if (++i < ac)
            {
                std::string arg2(av[i]);
                size_t pos = arg2.find("x");
                if (pos != std::string::npos)
                {
                    arg2.replace(pos, 1, " ");
                    std::stringstream ss(arg2);
                    ss >> m_config.g_benchWarmup >> m_config.g_benchFrames;
                }
            }
        } else if (arg == "--no-default-material") {
            g_createDefaultMaterial = false;
        } else if (av[i][0] == '-') {
            error("unknown commandline argument '"+arg+"'");
        } else {
            embree::FileName fn = arg;
            if (fn.ext() == "stl") {
                miniSG::importSTL(*msgModel,fn);
            } else if (fn.ext() == "msg") {
                miniSG::importMSG(*msgModel,fn);
            } else if (fn.ext() == "tri") {
                miniSG::importTRI(*msgModel,fn);
            } else if (fn.ext() == "xml") {
                miniSG::importRIVL(*msgModel,fn);
            } else if (fn.ext() == "obj") {
                miniSG::importOBJ(*msgModel,fn);
            } else if (fn.ext() == "hbp") {
                miniSG::importHBP(*msgModel,fn);
            } else if (fn.ext() == "x3d") {
                miniSG::importX3D(*msgModel,fn);
            } else if (fn.ext() == "astl") {
                miniSG::importSTL(msgAnimation,fn);
            }
        }
    }
}

void CommandLineSceneBuilder::reportParsedData()
{
    // -------------------------------------------------------
    // done parsing
    // -------------------------------------------------------]
    cout << "#ospModelViewer: done parsing. found model with" << endl;
    // cout << "  - num materials: " << msgModel->material.size() << endl;
    cout << "  - num meshes   : " << msgModel->mesh.size() << " ";
    size_t numUniqueTris = 0;
    size_t numInstancedTris = 0;
    for (size_t  i=0;i<msgModel->mesh.size();i++) {
        if (i < 10)
            cout << "[" << msgModel->mesh[i]->size() << "]";
        else
            if (i == 10) cout << "...";
        numUniqueTris += msgModel->mesh[i]->size();
    }
    cout << endl;
    cout << "  - num instances: " << msgModel->instance.size() << " ";
    for (size_t  i=0;i<msgModel->instance.size();i++) {
        if (i < 10)
            cout << "[" << msgModel->mesh[msgModel->instance[i].meshID]->size() << "]";
        else
            if (i == 10) cout << "...";
        numInstancedTris += msgModel->mesh[msgModel->instance[i].meshID]->size();
    }
    cout << endl;
    cout << "  - num unique triangles   : " << numUniqueTris << endl;
    cout << "  - num instanced triangles: " << numInstancedTris << endl;

    if (numInstancedTris == 0 && msgAnimation.empty())
        error("no (valid) input files specified - model contains no triangles");
}

void CommandLineSceneBuilder::createScene()
{
    // -------------------------------------------------------
    // create ospray model
    // -------------------------------------------------------
    m_model = ospNewModel();

    m_renderer = ospNewRenderer(rendererType.c_str());
    if (!m_renderer)
        throw std::runtime_error("could not create m_renderer '"+rendererType+"'");
    Assert(m_renderer != NULL && "could not create m_renderer");
    ospCommit(m_renderer);

    // code does not yet do instancing ... check that the model doesn't contain instances
    bool doesInstancing = 0;

    if (forceInstancing) {
        std::cout << "#ospModelViewer: forced instancing - instances on." << std::endl;
        doesInstancing = true;
    } else if (msgModel->instance.size() > msgModel->mesh.size()) {
        std::cout << "#ospModelViewer: found more object instances than meshes - turning on instancing" << std::endl;
        doesInstancing = true;
    } else {
        std::cout << "#ospModelViewer: number of instances matches number of meshes, creating single model that contains all meshes" << std::endl;
        doesInstancing = false;
    }
    if (doesInstancing) {
        if (msgModel->instance.size() > maxObjectsToConsider) {
            cout << "cutting down on the number of meshes as requested on cmdline..." << endl;
            msgModel->instance.resize(maxObjectsToConsider);
        }
    } else {
        if (msgModel->instance.size() > maxObjectsToConsider) {
            cout << "cutting down on the number of meshes as requested on cmdline..." << endl;
            msgModel->instance.resize(maxObjectsToConsider);
            msgModel->mesh.resize(maxObjectsToConsider);
        }
    }


    cout << "#ospModelViewer: adding parsed geometries to ospray model" << endl;
    std::vector<OSPModel> instanceModels;

    for (size_t i=0;i<msgModel->mesh.size();i++) {
        //      printf("Mesh %i/%li\n",i,msgModel->mesh.size());
        Ref<miniSG::Mesh> msgMesh = msgModel->mesh[i];

        // create ospray mesh
        OSPGeometry ospMesh = g_alpha ? ospNewGeometry("alpha_aware_triangle_mesh") : ospNewTriangleMesh();

        // check if we have to transform the vertices:
        if (doesInstancing == false && msgModel->instance[i] != miniSG::Instance(i)) {
            // cout << "Transforming vertex array ..." << endl;
            for (size_t vID=0;vID<msgMesh->position.size();vID++) {
                msgMesh->position[vID] = xfmPoint(msgModel->instance[i].xfm,
                                                  msgMesh->position[vID]);
            }
        }
        // add position array to mesh
        OSPData position = ospNewData(msgMesh->position.size(),OSP_FLOAT3A,
                                      &msgMesh->position[0],OSP_DATA_SHARED_BUFFER);
        ospSetData(ospMesh,"position",position);

        // add triangle index array to mesh
        if (!msgMesh->triangleMaterialId.empty()) {
            OSPData primMatID = ospNewData(msgMesh->triangleMaterialId.size(),OSP_INT,
                                           &msgMesh->triangleMaterialId[0],OSP_DATA_SHARED_BUFFER);
            ospSetData(ospMesh,"prim.materialID",primMatID);
        }

        // cout << "INDEX" << endl;
        // add triangle index array to mesh
        OSPData index = ospNewData(msgMesh->triangle.size(),OSP_INT3,
                                   &msgMesh->triangle[0],OSP_DATA_SHARED_BUFFER);
        assert(msgMesh->triangle.size() > 0);
        ospSetData(ospMesh,"index",index);

        // add normal array to mesh
        if (!msgMesh->normal.empty()) {
            OSPData normal = ospNewData(msgMesh->normal.size(),OSP_FLOAT3A,
                                        &msgMesh->normal[0],OSP_DATA_SHARED_BUFFER);
            assert(msgMesh->normal.size() > 0);
            ospSetData(ospMesh,"vertex.normal",normal);
        } else {
            // cout << "no vertex normals!" << endl;
        }

        // add color array to mesh
        if (!msgMesh->color.empty()) {
            OSPData color = ospNewData(msgMesh->color.size(),OSP_FLOAT3A,
                                       &msgMesh->color[0],OSP_DATA_SHARED_BUFFER);
            assert(msgMesh->color.size() > 0);
            ospSetData(ospMesh,"vertex.color",color);
        } else {
            // cout << "no vertex colors!" << endl;
        }

        // add texcoord array to mesh
        if (!msgMesh->texcoord.empty()) {
            OSPData texcoord = ospNewData(msgMesh->texcoord.size(), OSP_FLOAT2,
                                          &msgMesh->texcoord[0], OSP_DATA_SHARED_BUFFER);
            assert(msgMesh->texcoord.size() > 0);
            ospSetData(ospMesh,"vertex.texcoord",texcoord);
        }

        ospSet1i(ospMesh, "alpha_type", 0);
        ospSet1i(ospMesh, "alpha_component", 4);

        // add triangle material id array to mesh
        if (msgMesh->materialList.empty()) {
            // we have a single material for this mesh...
            OSPMaterial singleMaterial = createMaterial(m_renderer, msgMesh->material.ptr);
            ospSetMaterial(ospMesh,singleMaterial);
        } else {
            // we have an entire material list, assign that list
            std::vector<OSPMaterial > materialList;
            std::vector<OSPTexture2D > alphaMaps;
            std::vector<float> alphas;
            for (int i=0;i<msgMesh->materialList.size();i++) {
                materialList.push_back(createMaterial(m_renderer, msgMesh->materialList[i].ptr));

                for (miniSG::Material::ParamMap::const_iterator it =  msgMesh->materialList[i]->params.begin();
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
            OSPData ospMaterialList = ospNewData(materialList.size(), OSP_OBJECT, &materialList[0], 0);
            ospSetData(ospMesh,"materialList",ospMaterialList);

            // only set these if alpha aware mode enabled
            // this currently doesn't work on the MICs!
            if(g_alpha) {
                OSPData ospAlphaMapList = ospNewData(alphaMaps.size(), OSP_OBJECT, &alphaMaps[0], 0);
                ospSetData(ospMesh, "alpha_maps", ospAlphaMapList);

                OSPData ospAlphaList = ospNewData(alphas.size(), OSP_OBJECT, &alphas[0], 0);
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
        for (int i=0;i<msgModel->instance.size();i++) {
            OSPGeometry inst = ospNewInstance(instanceModels[msgModel->instance[i].meshID],
                    msgModel->instance[i].xfm);
            ospAddGeometry(m_model,inst);
        }
    }
    cout << "#m_modelViewer: committing model" << endl;
    ospCommit(m_model);
    cout << "#m_modelViewer: done creating ospray model." << endl;

    //TODO: Need to figure out where we're going to read lighting data from
    //begin light test
    std::vector<OSPLight> lights;
    if (defaultDirLight_direction != vec3f(0.f)) {
        cout << "#m_modelViewer: Adding a hard coded directional light as the sun." << endl;
        OSPLight ospLight = ospNewLight(m_renderer, "DirectionalLight");
        ospSetString(ospLight, "name", "sun" );
        ospSet3f(ospLight, "color", 1, 1, 1);
        ospSet3fv(ospLight, "direction", &defaultDirLight_direction.x);
        ospSet1f(ospLight, "angularDiameter", 0.53f);
        ospCommit(ospLight);
        lights.push_back(ospLight);
    }
#if 0
    //spot light
    cout << "#m_modelViewer: Adding a hard coded spotlight for test." << endl;
    OSPLight ospSpot = ospNewLight(m_renderer, "SpotLight");
    ospSetString(ospSpot, "name", "spot_test");
    ospSet3f(ospSpot, "position", 0.f, 2.f, 0.f);
    ospSet3f(ospSpot, "direction", 0.f, -1.f, 0.7f);
    ospSet3f(ospSpot, "color", 1.f, 1.f, .5f);
    ospSet1f(ospSpot, "intensity", 17.f);
    ospSet1f(ospSpot, "openingAngle", 50.f);
    ospSet1f(ospSpot, "penumbraAngle", 2.f);
    ospCommit(ospSpot);
    lights.push_back(ospSpot);
    //point light
    cout << "#m_modelViewer: Adding a hard coded pointlight for test." << endl;
    OSPLight ospPoint = ospNewLight(m_renderer, "PointLight");
    ospSetString(ospPoint, "name", "point_test");
    ospSet3f(ospPoint, "position", -5.f, 20.f, 10.f);
    ospSet3f(ospPoint, "color", .5f, 1.f, 1.f);
    ospSet1f(ospPoint, "intensity", 200.f);
    ospSet1f(ospPoint, "radius", 4.f);
    ospCommit(ospPoint);
    lights.push_back(ospPoint);
    //ambient light
    cout << "#m_modelViewer: Adding a hard coded ambientlight for test." << endl;
    OSPLight ospAmbient = ospNewLight(m_renderer, "AmbientLight");
    ospSetString(ospAmbient, "name", "ambient_test");
    ospSet1f(ospAmbient, "intensity", 0.2f);
    ospCommit(ospAmbient);
    lights.push_back(ospAmbient);
    //quad light
    cout << "#m_modelViewer: Adding a hard coded quadlight for test." << endl;
    OSPLight ospQuad = ospNewLight(m_renderer, "QuadLight");
    ospSetString(ospQuad, "name", "quad_test");
    ospSet3f(ospQuad, "position", 1.f, 3.5f, 0.f);
    ospSet3f(ospQuad, "edge1", 0.f, 0.f, 0.3f);
    ospSet3f(ospQuad, "edge2", 2.f, 0.f, 0.f);
    ospSet3f(ospQuad, "color", .5f, 1.f, .5f);
    ospSet1f(ospQuad, "intensity", 45.f);
    ospCommit(ospQuad);
    lights.push_back(ospQuad);
#endif
    OSPData lightArray = ospNewData(lights.size(), OSP_OBJECT, &lights[0], 0);
    ospSetData(m_renderer, "lights", lightArray);
    //end light test
    ospCommit(m_renderer);
}

OSPMaterial ospray::CommandLineSceneBuilder::createDefaultMaterial(OSPRenderer renderer)
{
    if(!g_createDefaultMaterial) return NULL;
    static OSPMaterial ospMat = NULL;
    if (ospMat) return ospMat;

    ospMat = ospNewMaterial(renderer, "OBJMaterial");
    if (!ospMat)
    {
        throw std::runtime_error("could not create default material 'OBJMaterial'");
        //cout << "given renderer does not know material type 'OBJMaterial'"
        //     << endl;
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
            cout << "WARNING: model does not have materials! "
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
            if (isOBJMaterial && (!strcmp(name, "Ns") || !strcmp(name, "ns")) && f < 1.f)
                f = 1.f/(1.f - f) - 1.f;
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
