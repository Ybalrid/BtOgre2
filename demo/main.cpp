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

//C++ standard library
#include <thread>

//Ogre includes
#include <Ogre.h>
#include <OgreMesh.h>
#include <OgreMesh2.h>
#include <OgreMeshManager.h>
#include <OgreMeshManager2.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <Compositor/OgreCompositorManager2.h>

//BtOgre includes
#include <BtOgre.hpp>
#include <BtOgreGP.h>

//SDL library, for windowing and input management
#include <SDL.h>
#include <SDL_syswm.h>

using namespace Ogre;

class BtOgreTestApplication
{
protected:
	//For the sake of simplicity, we are using "naked" pointer. I would recommend using std::unique_ptr<> for theses and not worrying about deleting them
	//Bullet intialization
	btDynamicsWorld* phyWorld;
	btBroadphaseInterface* mBroadphase;
	btDefaultCollisionConfiguration* mCollisionConfig;
	btCollisionDispatcher* mDispatcher;
	btSequentialImpulseConstraintSolver* mSolver;

	//BtOgre debug drawer object
	BtOgre::DebugDrawer* mDebugDrawer;

	//A physics object that is put on the scene
	SceneNode* mNinjaNode;
	Item* mNinjaItem;
	btRigidBody* mNinjaBody;
	btCollisionShape* mNinjaShape;

	//The "landscape" of the scene
	Item* mGroundItem;
	btRigidBody* mGroundBody;
	btCollisionShape* mGroundShape;

	//Ogre important objects
	Root* mRoot;
	SceneManager* mSceneMgr;
	Camera* mCamera;

	//Copile time constants for Ogre's static configuration here
	static constexpr const size_t SMGR_WORKERS{ 4 };

	//Window management objects
	bool running = true;
	RenderWindow* mWindow;
	SDL_Window* mSDLWindow;
	SDL_Event mSDLEvent;

	//To do some time keeping and step the physics simulation
	unsigned long milliNow, milliLast;

public:
	BtOgreTestApplication() :
		mDebugDrawer(nullptr),
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
		//Initialise the SDL library
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) throw std::runtime_error("Failed SDL_Init()");

		//Initialize the Bullet Physics engine
		mBroadphase = new btDbvtBroadphase;
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

		//Here's a quirk of the cleanup of a "triangle" based collision shape: You need to delete the mesh interface
		delete reinterpret_cast<btTriangleMeshShape*>(mGroundShape)->getMeshInterface();
		delete mGroundShape;

		//Free Bullet stuff.
		delete mDebugDrawer;
		delete phyWorld;

		delete mSolver;
		delete mDispatcher;
		delete mCollisionConfig;
		delete mBroadphase;

		//Stop Ogre
		delete mRoot;

		//Stop and cleanup SDL
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
		//Make sure the string we got is a valid path
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
		hlmsUnlit->setDebugOutputPath(false, false);

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
		hlmsPbs->setDebugOutputPath(false, false);
	}

	void createScene()
	{
		//Set some ambiant lighting
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2), ColourValue(0.2, 0.2, 0.2), mSceneMgr->getAmbientLightHemisphereDir());

		//Set the camera position
		mCamera->setPosition(Vector3(10, 10, 10));
		mCamera->lookAt(Vector3::ZERO);
		mCamera->setNearClipDistance(0.05);

		//Add a diretctional light
		auto light = mSceneMgr->createLight();
		auto lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		lightNode->attachObject(light);
		light->setType(Light::LT_DIRECTIONAL);
		light->setDirection({ 0, -1, 0.25f });

		//Set the debug drawer
		mDebugDrawer = new BtOgre::DebugDrawer{ mSceneMgr->getRootSceneNode(), phyWorld, mSceneMgr };
		phyWorld->setDebugDrawer(mDebugDrawer);

		//creat the main object
		auto pos = Vector3{ 0, 10, 0 };
		auto rot = Quaternion::IDENTITY;
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

		//Create the ground
		const auto groundMesh = asV2mesh("TestLevel_b0.mesh");
		mGroundItem = mSceneMgr->createItem(groundMesh);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mGroundItem);
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
		//Initialize Ogre
		mRoot = new Root("plugins.cfg", "ogre.cfg", "Ogre.log");
		LogManager::getSingleton().setLogDetail(LL_BOREME);
		mRoot->showConfigDialog();
		
		//Do not create a window with ogre yet. We're using the SDL to handle window and events:
		mRoot->initialise(false);

		//Fetch configuration for the window
		auto width = 1024, height = 768;
		auto cfgOpts = mRoot->getRenderSystem()->getConfigOptions();
		auto opt = cfgOpts.find("Video Mode");
		if (opt != cfgOpts.end())
		{
			//Ignore leading space
			const auto start = opt->second.currentValue.find_first_of("012356789");
			//Get the width and height
			auto widthEnd = opt->second.currentValue.find(' ', start);
			// we know that the height starts 3 characters after the width and goes until the next space
			auto heightEnd = opt->second.currentValue.find(' ', widthEnd + 3);
			// Now we can parse out the values
			width = StringConverter::parseInt(opt->second.currentValue.substr(0, widthEnd));
			height = StringConverter::parseInt(opt->second.currentValue.substr(
				widthEnd + 3, heightEnd));
		}
		auto fullscreen = StringConverter::parseBool(cfgOpts["Full Screen"].currentValue);

		//Create an SDL window
		
		mSDLWindow = SDL_CreateWindow("BtOgre21 Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));

		//Get access to native (os/window manager dependent) window information
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(mSDLWindow, &wmInfo);

		//Retreive window handle
		std::string winHandle;
		switch (wmInfo.subsystem)
		{
#ifdef _WIN32
		case SDL_SYSWM_WINDOWS:
			winHandle = StringConverter::toString(reinterpret_cast<uintptr_t>(wmInfo.info.win.window));
			break;
#else
		case SDL_SYSWM_X11:
			winHandle = Ogre::StringConverter::toString(reinterpret_cast<uintptr_t>(wmInfo.info.x11.window));
			break;
		//TODO handle wayland (and maybe mir?)
#endif
		default:
			LogManager::getSingleton().logMessage("Unimplemented window handle retreival");
			throw std::runtime_error("Unimplemented window handel retreival code for current operating system!");
			break;
		}

		//Create the configuration parameters for initializing the Ogre::RenderWindow object
		NameValuePairList params;
#ifdef _WIN32
		params.insert(make_pair("externalWindowHandle", winHandle));
#else
		params.insert(std::make_pair("parentWindowHandle", winHandle));
#endif
		params.insert(std::make_pair("title", "BtOgre21 Demo"));
		params.insert(make_pair("FSAA", cfgOpts["FSAA"].currentValue));
		params.insert(make_pair("vsync", cfgOpts["vsync"].currentValue));

		//Create the render window
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

		//Create the camera
		mCamera = mSceneMgr->createCamera("MyCamera");
		mCamera->setAutoAspectRatio(true);

		//Setup the objects in the scene
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
		//Get and process events
		WindowEventUtilities::messagePump();
		while (SDL_PollEvent(&mSDLEvent)) switch (mSDLEvent.type)
		{
		case SDL_WINDOWEVENT:
			switch (mSDLEvent.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				running = false;
				break;
			case SDL_WINDOWEVENT_RESIZED:
#ifndef _WIN32
				mWindow->resize(mSDLEvent.window.data1, mSDLEvent.window.data2);
#endif
				mWindow->windowMovedOrResized();

				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (mSDLEvent.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				running = false;
				break;
			}
			break;
		}

		//Get the timing for stepping physics
		milliLast = milliNow;
		milliNow = mRoot->getTimer()->getMilliseconds();
		
		//Step the simulation and the debug drawer
		phyWorld->stepSimulation(float(milliNow - milliLast) / 1000.0f);
		mDebugDrawer->step();

		//Render the frame
		mRoot->renderOneFrame();

		//Sleep for a millisec to prevent 100% time usage
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
