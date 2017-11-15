/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  BtOgre test application, main file.
 *
 *        Version:  1.1
 *        Created:  01/14/2009 05:48:31 PM
 *
 *         Author:  Nikhilesh (nikki), Arthur Brainville (Ybalrid)
 *
 * =====================================================================================
 */

#include <Ogre.h>
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <OgreMesh.h>
#include <OgreMesh2.h>
#include <OgreMeshManager.h>
#include <OgreMeshManager2.h>
#include <Compositor/OgreCompositorManager2.h>

#include <BtOgre.hpp>
#include <BtOgreGP.h>
#include <thread>

#include <SDL.h>
#include <SDL_syswm.h>

using namespace Ogre;


#include <fstream>

class BtOgreTestApplication
{
protected:

	btDynamicsWorld* phyWorld;
	BtOgre::DebugDrawer* dbgdraw;
	btAxisSweep3 *mBroadphase;
	btDefaultCollisionConfiguration *mCollisionConfig;
	btCollisionDispatcher *mDispatcher;
	btSequentialImpulseConstraintSolver *mSolver;

	SceneNode *mNinjaNode;
	Item *mNinjaItem;
	btRigidBody *mNinjaBody;
	btCollisionShape *mNinjaShape;

	Item *mGroundItem;
	btRigidBody *mGroundBody;
	btBvhTriangleMeshShape *mGroundShape;

	Root* mRoot;
	SceneManager* mSceneMgr;

	Camera* mCamera;

	static constexpr const char* const SL{ "GLSL" };
	static constexpr const char* const GL3PLUS_RENDERSYSTEM{ "OpenGL 3+ Rendering Subsystem" };
	static constexpr const size_t SMGR_WORKERS{ 4 };

	volatile bool running = true;
    RenderWindow* mWindow;
    SDL_Window* mSDLWindow;
    SDL_Event mSDLEvent;

	unsigned long milliNow, milliLast;

public:
	BtOgreTestApplication() : dbgdraw(nullptr),
		mNinjaNode(nullptr),
		mNinjaItem(nullptr),
		mNinjaBody(nullptr),
		mNinjaShape(nullptr),
		mGroundItem(nullptr),
		mGroundBody(nullptr),
		mGroundShape(nullptr),
		mRoot(nullptr),
		mSceneMgr(nullptr),
		mCamera(nullptr),
		mWindow(nullptr),
		milliNow{ 0 },
		milliLast{ 0 }
	{

        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) throw std::runtime_error("Failed SDL_Init()");

		//Bullet initialisation.
		mBroadphase = new btAxisSweep3(btVector3(-10000, -10000, -10000), btVector3(10000, 10000, 10000), 1024);
		mCollisionConfig = new btDefaultCollisionConfiguration();
		mDispatcher = new btCollisionDispatcher(mCollisionConfig);
		mSolver = new btSequentialImpulseConstraintSolver();

		phyWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
		phyWorld->setGravity(btVector3(0, -9.8, 0));
	}

	~BtOgreTestApplication()
	{
		//Free rigid bodies
		phyWorld->removeRigidBody(mNinjaBody);
		delete mNinjaBody->getMotionState();
		delete mNinjaBody;
		delete mNinjaShape;

		phyWorld->removeRigidBody(mGroundBody);
		delete mGroundBody->getMotionState();
		delete mGroundBody;
		delete mGroundShape->getMeshInterface();
		delete mGroundShape;

		//Free Bullet stuff.
		delete dbgdraw;
		delete phyWorld;

		delete mSolver;
		delete mDispatcher;
		delete mCollisionConfig;
		delete mBroadphase;

        delete mRoot;

        SDL_DestroyWindow(mSDLWindow);
        SDL_Quit();
	}

protected:

	v1::MeshPtr loadV1mesh(const String& meshName)
	{
		return v1::MeshManager::getSingleton().load(meshName, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC);
	}

	MeshPtr asV2mesh(const String& meshName, const String& ResourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, const String& sufix = " V2",
		const bool& halfPos = true, const bool& halfTextCoords = true, const bool& qTangents = true)
	{
		//Get the V1 mesh
		auto v1mesh = loadV1mesh(meshName);

		//Convert it as a V2 mesh
		const auto mesh = MeshManager::getSingletonPtr()->createManual(meshName + sufix, ResourceGroup);
		mesh->importV1(v1mesh.get(), halfPos, halfTextCoords, qTangents);

		//Unload the useless V1 mesh
		v1mesh->unload();
		v1mesh.setNull();

		//Return the shared pointer to the new mesh
		return mesh;
	}

	void declareHlmsLibrary(const String&& path)
	{
#ifdef _DEBUG
		if (std::string(SL) != "GLSL" || std::string(Ogre::Root::getSingleton().getRenderSystem()->getName()) != "OpenGL 3+ Rendering Subsystem")
			throw std::runtime_error("This function is OpenGL only. Please use the RenderSytem_GL3+ in the Ogre configuration!");
#endif
		auto dataFolder = path;

		if (dataFolder.empty()) dataFolder = "./";
		else if (dataFolder[dataFolder.size() - 1] != '/') dataFolder += '/';

		//For retrieval of the paths to the different folders needed
		String dataFolderPath;
		StringVector libraryFoldersPaths;

		//Get the path to all the subdirectories used by HlmsUnlit
		HlmsUnlit::getDefaultPaths(dataFolderPath, libraryFoldersPaths);

		//Create the Ogre::Archive objects needed
		Archive* archiveUnlit = ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);
		ArchiveVec archiveUnlitLibraryFolders;
		for (const auto& libraryFolderPath : libraryFoldersPaths)
		{
			Archive* archiveLibrary = ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
			archiveUnlitLibraryFolders.push_back(archiveLibrary);
		}

		//Create and register the unlit Hlms
		HlmsUnlit* hlmsUnlit = OGRE_NEW HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
		Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);

		//Do the same for HlmsPbs:
		HlmsPbs::getDefaultPaths(dataFolderPath, libraryFoldersPaths);
		Archive* archivePbs = ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);

		//Get the library archive(s)
		ArchiveVec archivePbsLibraryFolders;
		for (const auto& libraryFolderPath : libraryFoldersPaths)
		{
			Archive* archiveLibrary = ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
			archivePbsLibraryFolders.push_back(archiveLibrary);
		}

		//Create and register
		HlmsPbs* hlmsPbs = OGRE_NEW HlmsPbs(archivePbs, &archivePbsLibraryFolders);
		Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	}

	void createScene()
	{
		//Some normal stuff.
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2), ColourValue(0.2, 0.2, 0.2), mSceneMgr->getAmbientLightHemisphereDir());
		mCamera->setPosition(Vector3(10, 10, 10));
		mCamera->lookAt(Vector3::ZERO);
		mCamera->setNearClipDistance(0.05);
		LogManager::getSingleton().setLogDetail(LL_BOREME);

		auto light = mSceneMgr->createLight();
		auto lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		lightNode->attachObject(light);
		light->setType(Light::LT_DIRECTIONAL);
		light->setDirection({ 0, -1, 0.25f });

		//----------------------------------------------------------
		// Debug drawing!
		//----------------------------------------------------------

		dbgdraw = new BtOgre::DebugDrawer{ mSceneMgr->getRootSceneNode(), phyWorld, mSceneMgr };
		phyWorld->setDebugDrawer(dbgdraw);

		//----------------------------------------------------------
		// Ninja!
		//----------------------------------------------------------

		auto pos = Vector3{ 0, 10, 0 };
		auto rot = Quaternion::IDENTITY;

		//Create Ogre stuff.

		//mNinjaEntity = mSceneMgr->createItem("ninjaEntity", "Player.mesh");

		auto ninjaMesh = asV2mesh("Player.mesh");
		mNinjaItem = mSceneMgr->createItem(ninjaMesh);

		mNinjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(SCENE_DYNAMIC, pos, rot);
		mNinjaNode->attachObject(mNinjaItem);

		//Create shape.
		BtOgre::StaticMeshToShapeConverter converter(mNinjaItem);
		mNinjaShape = converter.createSphere();

		//Calculate inertia.
		btScalar mass = 5;
		btVector3 inertia;
		mNinjaShape->calculateLocalInertia(mass, inertia);

		//Create BtOgre MotionState (connects Ogre and Bullet).
		auto ninjaState = new BtOgre::RigidBodyState(mNinjaNode);

		//Create the Body.
		mNinjaBody = new btRigidBody(mass, ninjaState, mNinjaShape, inertia);
		phyWorld->addRigidBody(mNinjaBody);

		//----------------------------------------------------------
		// Ground!
		//----------------------------------------------------------

		//Create Ogre stuff.
		//MeshManager::getSingleton().createPlane("groundPlane", "General", Plane(Vector3::UNIT_Y, 0), 100, 100,
		//10, 10, true, 1, 5, 5, Vector3::UNIT_Z);

		const auto groundMesh = asV2mesh("TestLevel_b0.mesh");

		mGroundItem = mSceneMgr->createItem(groundMesh);
		//mGroundEntity->setMaterialName("Examples/Rockwall");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mGroundItem);

		//Create the ground shape.
		BtOgre::StaticMeshToShapeConverter converter2(mGroundItem);

		mGroundShape = converter2.createTrimesh();

		//Create MotionState (no need for BtOgre here, you can use it if you want to though).
		const auto groundState = new btDefaultMotionState(
			btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

		//Create the Body.
		mGroundBody = new btRigidBody(0, groundState, mGroundShape, btVector3(0, 0, 0));
		phyWorld->addRigidBody(mGroundBody);
	}

	void setup()
	{
		mRoot = new Root("plugins.cfg", "ogre.cfg", "Ogre.log");
		LogManager::getSingleton().setLogDetail(LL_BOREME);

		mRoot->showConfigDialog();

		//mWindow = mRoot->initialise(true, "BtOgre21 Demo");
        mRoot->initialise(false);

        int width = 1024, height = 768;
        auto cfgOpts = mRoot->getRenderSystem()->getConfigOptions();
        Ogre::ConfigOptionMap::iterator opt = cfgOpts.find( "Video Mode" );
        if( opt != cfgOpts.end() )
        {
            //Ignore leading space
            const Ogre::String::size_type start = opt->second.currentValue.find_first_of("012356789");
            //Get the width and height
            Ogre::String::size_type widthEnd = opt->second.currentValue.find(' ', start);
            // we know that the height starts 3 characters after the width and goes until the next space
            Ogre::String::size_type heightEnd = opt->second.currentValue.find(' ', widthEnd+3);
            // Now we can parse out the values
            width   = Ogre::StringConverter::parseInt( opt->second.currentValue.substr( 0, widthEnd ) );
            height  = Ogre::StringConverter::parseInt( opt->second.currentValue.substr(
                                                           widthEnd+3, heightEnd ) );
        }
        bool fullscreen = Ogre::StringConverter::parseBool( cfgOpts["Full Screen"].currentValue );

        mSDLWindow = SDL_CreateWindow("BtOgre21 Demo", 0, 0, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);

        SDL_GetWindowWMInfo(mSDLWindow, &wmInfo);
        std::string winHandle;

        switch(wmInfo.subsystem)
        {
#ifdef _WIN32
            case SDL_SYSWM_WINDOWS:
                winHandle = Ogre::StringConverter::toString((uintptr_t)wmInfo.info.win.window);
            break;
#else
            case SDL_SYSWM_X11:
                winHandle = Ogre::StringConverter::toString((uintptr_t)wmInfo.info.x11.window);
            break;
#endif
        default:
                Ogre::LogManager::getSingleton().logMessage ("Unimplemented window handle retreival");
                throw std::runtime_error("Unimplemented window handel retreival code for current operating system!");
            break;
        }

        Ogre::NameValuePairList params;
#ifdef _WIN32
        params.insert(std::make_pair("externalWindowHandle", winHandle));
#else
        params.insert(std::make_pair("parentWindowHandle", winHandle));
#endif
        params.insert(std::make_pair("title", "BtOgre21 Demo"));
        params.insert(std::make_pair("FSAA",cfgOpts["FSAA"].currentValue));
        params.insert(std::make_pair("vsync", cfgOpts["vsync"].currentValue));

        mWindow = mRoot->createRenderWindow("BtOgre21 Demo", width, height, fullscreen, &params);

		//Declare some resources
		auto resourceGroupManager = ResourceGroupManager::getSingletonPtr();
		resourceGroupManager->addResourceLocation("./data/OgreCore.zip", "Zip");
		resourceGroupManager->addResourceLocation("./data/Meshes", "FileSystem");
		resourceGroupManager->addResourceLocation("./data/Textures", "FileSystem");

		//Init the HLMS
		declareHlmsLibrary("HLMS");

		//All resources initialized
		resourceGroupManager->initialiseAllResourceGroups(false);

		//Create a scene manager
		mSceneMgr = mRoot->createSceneManager(ST_GENERIC, SMGR_WORKERS, INSTANCING_CULLING_THREADED, "MAIN_SMGR");

		mCamera = mSceneMgr->createCamera("MyCamera");
		mCamera->setAutoAspectRatio(true);

		createScene();

		//Create a basic compositor
		auto compositorManager = mRoot->getCompositorManager2();
		IdString mainWorkspace{ "MainWorkspace" };
		if (!compositorManager->hasWorkspaceDefinition(mainWorkspace))
			compositorManager->createBasicWorkspaceDef("MainWorkspace", ColourValue(1, 1, 0, 1));
		compositorManager->addWorkspace(mSceneMgr, mWindow, mCamera, mainWorkspace, true);
	}

	///Render a frame
	void frame()
	{
        WindowEventUtilities::messagePump();
        while(SDL_PollEvent(&mSDLEvent)) switch(mSDLEvent.type)
        {
            case SDL_WINDOWEVENT:
            switch(mSDLEvent.window.event)
            {
                case SDL_WINDOWEVENT_CLOSE:
                    running = false;
                break;
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_MOVED:
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_MAXIMIZED:
                mWindow->resize(mSDLEvent.window.data1, mSDLEvent.window.data2);
                mWindow->windowMovedOrResized();
                break;
            }
            break;
        case SDL_KEYDOWN:
            switch(mSDLEvent.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                running = false;
                break;
            }
            break;
        }

		milliLast = milliNow;
		milliNow = mRoot->getTimer()->getMilliseconds();

		phyWorld->stepSimulation(float(milliNow - milliLast) / 1000.0f);
		dbgdraw->step();

		mRoot->renderOneFrame();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

public:
	void go()
	{
		setup();
		while (running)
		{
			frame();
		}
	}
};

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main() function. Need say more?
 * =====================================================================================
 */

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char **argv)
#endif
{
	BtOgreTestApplication app;
	app.go();

	return 0;
}
