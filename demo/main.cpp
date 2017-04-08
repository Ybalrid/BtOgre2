/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  BtOgre test application, main file.
 *
 *        Version:  1.0
 *        Created:  01/14/2009 05:48:31 PM
 *
 *         Author:  Nikhilesh (nikki)
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

#define USEV1

using namespace Ogre;

/*
 * =====================================================================================
 *    Namespace:  Globals
 *  Description:  A dirty 'globals' hack.
 * =====================================================================================
 */

namespace Globals
{
	btDynamicsWorld *phyWorld;
	BtOgre::DebugDrawer *dbgdraw;
}

/*
 * =====================================================================================
 *        Class:  BtOgreTestApplication
 *  Description:  Derives from ExampleApplication and overrides stuff.
 * =====================================================================================
 */

#include <fstream>

void sanityCheck(BtOgre::StaticMeshToShapeConverter& cvt1, BtOgre::StaticMeshToShapeConverter& cvt2)
{
	std::ofstream out("cvt1.txt");
	if (!out.is_open()) abort();

	out << "Cvt1:\n";
	out << "IndexBuffer Count : " << cvt1.getIndexCount() << '\n';
	out << "Triangle count : " << cvt1.getTriangleCount() << '\n';
	out << "IndexBuffer listing (1 per line) :\n";
	for (size_t i = 0; i < cvt1.getIndexCount(); i++)
		out << cvt1.getIndices()[i] << '\n';
	out << "VertexBufferCount : " << cvt1.getVertexCount() << '\n';
	out << "VertexBuffer listing (1 per line) :\n";
	for (size_t i = 0; i < cvt1.getVertexCount(); i++)
		out << cvt1.getVertices()[i] << '\n';
	out << "end of cvt1" << std::endl;

	out.close();

	out.open("cvt2.txt");
	if (!out.is_open()) abort();

	out << "Cvt2:\n";
	out << "IndexBuffer Count : " << cvt2.getIndexCount() << '\n';
	out << "Triangle count : " << cvt2.getTriangleCount() << '\n';
	out << "IndexBuffer listing (1 per line) :\n";
	for (size_t i = 0; i < cvt2.getIndexCount(); i++)
		out << cvt2.getIndices()[i] << '\n';
	out << "VertexBufferCount : " << cvt2.getVertexCount() << '\n';
	out << "VertexBuffer listing (1 per line) :\n";
	for (size_t i = 0; i < cvt2.getVertexCount(); i++)
		out << cvt2.getVertices()[i] << '\n';
	out << "end of cvt2" << std::endl;

	auto log = [](const std::string& message) {Ogre::LogManager::getSingleton().logMessage(message); };

	if (cvt1.getIndexCount() != cvt2.getIndexCount())
	{
		log("Index count doesn't match");
	}

	if (cvt1.getVertexCount() != cvt2.getVertexCount())
	{
		log("Vertex count doesn't match");
	}

	if (cvt1.getTriangleCount() != cvt2.getTriangleCount())
	{
		log("Triangle count doesn't match");
	}

	auto minIndex = std::min(cvt1.getIndexCount(), cvt2.getIndexCount());
	auto minVertex = std::min(cvt1.getVertexCount(), cvt2.getVertexCount());

	for (size_t i = 0; i < minIndex; i++)
	{
		if (cvt1.getIndices()[i] != cvt2.getIndices()[i])
			log("Index at position " + std::to_string(i) + "doesn't match between the 2 converters");
	}

	for (size_t i = 0; i < minVertex; i++)
	{
		if (cvt1.getVertices()[i] != cvt2.getVertices()[i])
			log("Vertex at position " + std::to_string(i) + "doesn't match between the 2 converters");
	}
}

class BtOgreTestApplication
{
protected:
	btAxisSweep3 *mBroadphase;
	btDefaultCollisionConfiguration *mCollisionConfig;
	btCollisionDispatcher *mDispatcher;
	btSequentialImpulseConstraintSolver *mSolver;

	Ogre::SceneNode *mNinjaNode;
	Ogre::Item *mNinjaItem;
	btRigidBody *mNinjaBody;
	btCollisionShape *mNinjaShape;

	Ogre::Item *mGroundItem;
	btRigidBody *mGroundBody;
	btBvhTriangleMeshShape *mGroundShape;

	Ogre::Root* mRoot;
	Ogre::SceneManager* mSceneMgr;

	Ogre::Camera* mCamera;

	static constexpr const char* const SL{ "GLSL" };
	static constexpr const char* const GL3PLUS_RENDERSYSTEM{ "OpenGL 3+ Rendering Subsystem" };
	static constexpr const size_t SMGR_WORKERS{ 4 };

	volatile bool running = true;
	Ogre::RenderWindow* mWindow;

	unsigned long milliNow, milliLast;

public:
	BtOgreTestApplication()
	{
		//Bullet initialisation.
		mBroadphase = new btAxisSweep3(btVector3(-10000, -10000, -10000), btVector3(10000, 10000, 10000), 1024);
		mCollisionConfig = new btDefaultCollisionConfiguration();
		mDispatcher = new btCollisionDispatcher(mCollisionConfig);
		mSolver = new btSequentialImpulseConstraintSolver();

		Globals::phyWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
		Globals::phyWorld->setGravity(btVector3(0, -9.8, 0));
	}

	~BtOgreTestApplication()
	{
		//Free rigid bodies
		Globals::phyWorld->removeRigidBody(mNinjaBody);
		delete mNinjaBody->getMotionState();
		delete mNinjaBody;
		delete mNinjaShape;

		Globals::phyWorld->removeRigidBody(mGroundBody);
		delete mGroundBody->getMotionState();
		delete mGroundBody;
		delete mGroundShape->getMeshInterface();
		delete mGroundShape;

		//Free Bullet stuff.
		delete Globals::dbgdraw;
		delete Globals::phyWorld;

		delete mSolver;
		delete mDispatcher;
		delete mCollisionConfig;
		delete mBroadphase;

		delete mRoot;
	}

protected:

	decltype(auto) loadV1mesh(Ogre::String meshName)
	{
		return Ogre::v1::MeshManager::getSingleton().load(meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);
	}

	decltype(auto) asV2mesh(Ogre::String meshName, Ogre::String ResourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::String sufix = " V2",
		bool halfPos = true, bool halfTextCoords = true, bool qTangents = true)
	{
		//Get the V1 mesh
		auto v1mesh = loadV1mesh(meshName);

		//Convert it as a V2 mesh
		auto mesh = Ogre::MeshManager::getSingletonPtr()->createManual(meshName + sufix, ResourceGroup);
		mesh->importV1(v1mesh.get(), halfPos, halfTextCoords, qTangents);

		//Unload the useless V1 mesh
		v1mesh->unload();
		v1mesh.setNull();

		//Return the shared pointer to the new mesh
		return mesh;
	}

	void declareHlmsLibrary(const Ogre::String&& path)
	{
#ifdef _DEBUG
		if (string(SL) != "GLSL" || string(Ogre::Root::getSingleton().getRenderSystem()->getName()) != "OpenGL 3+ Rendering Subsystem")
			throw std::runtime_error("This function is OpenGL only. Please use the RenderSytem_GL3+ in the Ogre configuration!");
#endif
		Ogre::String hlmsFolder = path;

		//The hlmsFolder can come from a configuration file where it could be "empty" or set to "." or lacking the trailing "/"
		if (hlmsFolder.empty()) hlmsFolder = "./";
		else if (hlmsFolder[hlmsFolder.size() - 1] != '/') hlmsFolder += "/";

		//Get the hlmsManager (not a singleton by itself, but accessible via Root)
		auto hlmsManager = Ogre::Root::getSingleton().getHlmsManager();

		//Define the shader library to use for HLMS
		auto library = Ogre::ArchiveVec();
		auto archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(hlmsFolder + "Hlms/Common/" + SL, "FileSystem", true);
		library.push_back(archiveLibrary);

		//Define "unlit" and "PBS" (physics based shader) HLMS
		auto archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(hlmsFolder + "Hlms/Unlit/" + SL, "FileSystem", true);
		auto archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(hlmsFolder + "Hlms/Pbs/" + SL, "FileSystem", true);
		auto hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(archiveUnlit, &library);
		auto hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &library);
		hlmsManager->registerHlms(hlmsUnlit);
		hlmsManager->registerHlms(hlmsPbs);
	}

	void createScene(void)
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

		Globals::dbgdraw = new BtOgre::DebugDrawer(mSceneMgr->getRootSceneNode(), Globals::phyWorld);
		Globals::phyWorld->setDebugDrawer(Globals::dbgdraw);

		//----------------------------------------------------------
		// Ninja!
		//----------------------------------------------------------

		Vector3 pos = Vector3(0, 10, 0);
		Quaternion rot = Quaternion::IDENTITY;

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
		BtOgre::RigidBodyState *ninjaState = new BtOgre::RigidBodyState(mNinjaNode);

		//Create the Body.
		mNinjaBody = new btRigidBody(mass, ninjaState, mNinjaShape, inertia);
		Globals::phyWorld->addRigidBody(mNinjaBody);

		//----------------------------------------------------------
		// Ground!
		//----------------------------------------------------------

		//Create Ogre stuff.
		//MeshManager::getSingleton().createPlane("groundPlane", "General", Plane(Vector3::UNIT_Y, 0), 100, 100,
		//10, 10, true, 1, 5, 5, Vector3::UNIT_Z);

		auto groundMesh = asV2mesh("TestLevel_b0.mesh");

		mGroundItem = mSceneMgr->createItem(groundMesh);
		//mGroundEntity->setMaterialName("Examples/Rockwall");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mGroundItem);

		//Create the ground shape.
#ifndef USEV1
		BtOgre::StaticMeshToShapeConverter converter2(mGroundItem);
#else
		auto v1Ground = loadV1mesh("TestLevel_b0.mesh").get();
		BtOgre::StaticMeshToShapeConverter converter2(mGroundItem);
#endif

		//sanityCheck(converter2v1, converter2);

		mGroundShape = converter2.createTrimesh();

		//Create MotionState (no need for BtOgre here, you can use it if you want to though).
		btDefaultMotionState* groundState = new btDefaultMotionState(
			btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

		//Create the Body.
		mGroundBody = new btRigidBody(0, groundState, mGroundShape, btVector3(0, 0, 0));
		Globals::phyWorld->addRigidBody(mGroundBody);
	}

	void setup()
	{
		mRoot = new Ogre::Root("plugins.cfg", "ogre.cfg", "Ogre.log");
		Ogre::LogManager::getSingleton().setLogDetail(Ogre::LoggingLevel::LL_BOREME);

		mRoot->showConfigDialog();

		mWindow = mRoot->initialise(true, "BtOgre21 Demo");

		//Declare some resources
		auto resourceGroupManager = Ogre::ResourceGroupManager::getSingletonPtr();
		resourceGroupManager->addResourceLocation("./data/Meshes", "FileSystem");
		resourceGroupManager->addResourceLocation("./data/Textures", "FileSystem");
		resourceGroupManager->addResourceLocation("./data/OgreCore.zip", "Zip");

		//Init the HLMS
		declareHlmsLibrary("HLMS");

		//All resources initialized
		resourceGroupManager->initialiseAllResourceGroups();

		mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, SMGR_WORKERS, Ogre::INSTANCING_CULLING_THREADED, "MAIN_SMGR");

		mCamera = mSceneMgr->createCamera("MyCamera");

		createScene();

		auto compositorManager = mRoot->getCompositorManager2();
		IdString mainWorkspace{ "MainWorkspace" };
		if (!compositorManager->hasWorkspaceDefinition(mainWorkspace))
			compositorManager->createBasicWorkspaceDef("MainWorkspace", Ogre::ColourValue(1, 1, 0, 1));
		compositorManager->addWorkspace(mSceneMgr, mWindow, mCamera, mainWorkspace, true);
	}

	void frame()
	{
		Ogre::WindowEventUtilities::messagePump();

		if (mWindow->isClosed())
			running = false;
		else
		{
			milliLast = milliNow;
			milliNow = mRoot->getTimer()->getMilliseconds();

			Globals::phyWorld->stepSimulation(float(milliNow - milliLast) / 1000.0f);
			Globals::dbgdraw->step();

			mRoot->renderOneFrame();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
public:
	void go()
	{
		setup();
		while (running)
		{
			frame();
		}

		Ogre::LogManager::getSingleton().logMessage("Not rendering anymore!");
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