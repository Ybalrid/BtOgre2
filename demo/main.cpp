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
#include <OgreHlmsPbs.h>
#include <OgreWindow.h>
#include <Compositor/OgreCompositorManager2.h>

//BtOgre includes
#include <BtOgre.hpp>
#include <BtOgreGP.h>

//SDL library, for windowing and input management
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_keycode.h>

using namespace Ogre;

class BtOgreTestApplication
{
protected:
	//For the sake of simplicity, we are using "naked" pointer. I would recommend using std::unique_ptr<> for theses and not worrying about deleting them
	//Bullet intialization
	btDynamicsWorld * phyWorld;
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
	Ogre::Window* mWindow;
	SDL_Window* mSDLWindow;
	SDL_Event mSDLEvent;
	int physicsObjectCount = 0;

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
		mSDLWindow(nullptr),
		milliNow{ 0 },
		milliLast{ 0 }
	{
		//Initialise the SDL library
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) throw std::runtime_error("Failed SDL_Init()");
		SDL_SetRelativeMouseMode(SDL_TRUE);

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
		if (mNinjaBody) {
			phyWorld->removeRigidBody(mNinjaBody);
			delete mNinjaBody->getMotionState();
			delete mNinjaBody;
			delete mNinjaShape;
		}
		if (mGroundBody) {
			phyWorld->removeRigidBody(mGroundBody);
			delete mGroundBody->getMotionState();
			delete mGroundBody;
			//Here's a quirk of the cleanup of a "triangle" based collision shape: You need to delete the mesh interface
			delete reinterpret_cast<btTriangleMeshShape*>(mGroundShape)->getMeshInterface();
			delete mGroundShape;
		}

		//Free Bullet stuff.
		//delete mDebugDrawer;
		delete phyWorld;

		delete mSolver;
		delete mDispatcher;
		delete mCollisionConfig;
		delete mBroadphase;

		//Stop Ogre
		//delete mRoot;

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
                        
                          LogManager::getSingleton().logMessage("hlms " + dataFolder + libraryFolderPath);
		}
                LogManager::getSingleton().logMessage("hlms 1");
		//Create and register the unlit Hlms
		HlmsUnlit* hlmsUnlit = OGRE_NEW HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
                LogManager::getSingleton().logMessage("hlms 101");
		Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);
                LogManager::getSingleton().logMessage("hlms 102");
		hlmsUnlit->setDebugOutputPath(false, false);
                LogManager::getSingleton().logMessage("hlms 11");
		//Do the same for HlmsPbs:
		HlmsPbs::getDefaultPaths(dataFolderPath, libraryFoldersPaths);
		Archive* archivePbs = ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);
                LogManager::getSingleton().logMessage("hlms 12");
		//Get the library archive(s)
		ArchiveVec archivePbsLibraryFolders;
		for (const auto& libraryFolderPath : libraryFoldersPaths)
		{
			Archive* archiveLibrary = ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
			archivePbsLibraryFolders.push_back(archiveLibrary);
		}

		//Create and register
                LogManager::getSingleton().logMessage("hlms 2");
		HlmsPbs* hlmsPbs = OGRE_NEW HlmsPbs(archivePbs, &archivePbsLibraryFolders);
                LogManager::getSingleton().logMessage("hlms 3");
		Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
		hlmsPbs->setDebugOutputPath(false, false);
	}

	void createPhysicsObject()
	{
		//Generate a unique name to each created node by incrementing the counter.
		std::string physicsNodeName = "mNinjaNode_" + std::to_string(physicsObjectCount);
		auto pos = Vector3{ 0, 10, 0 };
		auto rot = Quaternion::IDENTITY;

		auto ninjaMesh = asV2mesh("Player.mesh", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, std::to_string(physicsObjectCount));
		mNinjaItem = mSceneMgr->createItem(ninjaMesh);

		mNinjaItem->setName(physicsNodeName);
		mNinjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(SCENE_DYNAMIC, pos, rot);
		mNinjaNode->setName(physicsNodeName);
		physicsObjectCount += 1;
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
	}

	void createScene()
	{
		//Set some ambiant lighting
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2), ColourValue(0.2, 0.2, 0.2), mSceneMgr->getAmbientLightHemisphereDir());

		////Set the camera position
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

		//Create a dynamic object
		createPhysicsObject();

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

		Ogre::LogManager::getSingleton().setLogDetail(Ogre::LoggingLevel::LL_BOREME);
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
		SDL_SysWMinfo* wmInfo = new SDL_SysWMinfo;
		SDL_VERSION(&wmInfo->version);
		SDL_GetWindowWMInfo(mSDLWindow, wmInfo);

		//Retreive window handle
		std::string winHandle;
		switch (wmInfo->subsystem)
		{
#ifdef _WIN32
		case SDL_SYSWM_WINDOWS:
			//winHandle = StringConverter::toString(reinterpret_cast<uintptr_t>(wmInfo.info.win.window));
			winHandle = Ogre::StringConverter::toString((uintptr_t)wmInfo->info.win.window);
			break;
#else
		case SDL_SYSWM_X11:
			winHandle = Ogre::StringConverter::toString(reinterpret_cast<uintptr_t>(wmInfo->info.x11.window));
			break;
			//TODO handle wayland (and maybe mir?)
#endif
		default:
			LogManager::getSingleton().logMessage("Unimplemented window handle retreival for subsystem " + std::to_string(wmInfo->subsystem));
			throw std::runtime_error("Unimplemented window handle retreival code for current operating system! subsystem " + std::to_string(wmInfo->subsystem));
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
		resourceGroupManager->addResourceLocation("../../data/OgreCore.zip", "Zip");
		resourceGroupManager->addResourceLocation("../../data/Meshes", "FileSystem");
		resourceGroupManager->addResourceLocation("../../data/Textures", "FileSystem");

		//Init the HLMS
                LogManager::getSingleton().logMessage("hlms init start");
		declareHlmsLibrary("../Data");

                LogManager::getSingleton().logMessage("hlms init done");
		//All resources initialized
		resourceGroupManager->initialiseAllResourceGroups(false);

		//Create a scene manager
		mSceneMgr = mRoot->createSceneManager(ST_GENERIC, SMGR_WORKERS, "MAIN_SMGR");

		//Create the camera
		mCamera = mSceneMgr->createCamera("MyCamera");
		mCamera->setAutoAspectRatio(true);

		mCamera->setPosition(15, 15, 15);
		mCamera->lookAt(0, 1, 0);

		//Setup the objects in the scene
		createScene();

		//Create a basic compositor
		auto compositorManager = mRoot->getCompositorManager2();
		IdString mainWorkspace{ "MainWorkspace" };
		if (!compositorManager->hasWorkspaceDefinition(mainWorkspace))
			compositorManager->createBasicWorkspaceDef("MainWorkspace", ColourValue(0.05, 0.4, 0.8, 1));
		compositorManager->addWorkspace(mSceneMgr, mWindow->getTexture(), mCamera, mainWorkspace, true);
	}

	///Render a frame
	void frame()
	{
		auto movement = Vector3::ZERO;
		float cameraTransformSpeedFactor = 20;
		float cameraRotateSpeedFactor = 2000;
		const auto state = SDL_GetKeyboardState(nullptr);

		if (state[SDL_SCANCODE_W])
			movement.z = -1;
		if (state[SDL_SCANCODE_A])
			movement.x = -1;
		if (state[SDL_SCANCODE_S])
			movement.z = 1;
		if (state[SDL_SCANCODE_D])
			movement.x = 1;

		float yawAngle = 0;
		float pitchAngle = 0;

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
				//mWindow->resize(mSDLEvent.window.data1, mSDLEvent.window.data2);
#endif
				mWindow->windowMovedOrResized();
				break;
			default:
				break;
			}
			break;

		case SDL_MOUSEMOTION:

			yawAngle = mSDLEvent.motion.xrel;
			pitchAngle = mSDLEvent.motion.yrel;
			break;

		case SDL_KEYDOWN:
			switch (mSDLEvent.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				running = false;
				break;
			case SDLK_SPACE:
				createPhysicsObject();
				break;
			default:
				break;
			}
		default:
			break;
		}

		movement.normalise();
		if (movement != Vector3::ZERO)
		{
			//Move the camera in the local axis.
			mCamera->moveRelative(movement / cameraTransformSpeedFactor);
		}

		//Set the new camera orientation
		mCamera->yaw(-Radian(yawAngle) / cameraRotateSpeedFactor);
		mCamera->pitch(Radian(-pitchAngle / cameraRotateSpeedFactor));

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
		auto root = std::make_unique<Ogre::Root>();
		mRoot = root->getSingletonPtr();
		if (!root->showConfigDialog()) return;
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
