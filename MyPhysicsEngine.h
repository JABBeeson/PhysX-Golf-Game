#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>
#include <chrono>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = {PxVec3(130.f/255.f,30.f/255.f,0.f/255.f),
		PxVec3(1.f/255.f,255.f/255.f,1.f/255.f),		
		PxVec3(255.f/255.f,255.f/255.f,255.f/255.f),	//white
		PxVec3(247.f/255.f,240.f/255.f,9.f/255.f),	//sand
		PxVec3(0.f/255.f,255.f/255.f,255.f/255.f)}; //ICE

	struct FilterGroup
	{
		enum Enum
		{
			BALL = (1 << 0),
			GRASS = (1 << 1),
			SAND = (1 << 2),
			ICE = (1 << 3),
			CLUB = (1 << 4)
			//add more if you need
		};
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;
		bool potted;

		MySimulationEventCallback() : potted(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) 
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() == PxGeometryType::eSPHERE )
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						//cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;
						pairs[i].otherActor->setGlobalPose(PxTransform(PxVec3(0.0f, 3.0f, 5.0f), PxQuat(PxIdentity)));
						//cout << "end" << endl;
						potted = true;
						
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						//cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
						cout << "_!_!_!_!_!_!__!_!_!_!_!_!_!_!_!_!_!__!__-----  END  -----__!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!!_!_!_!_!_!__!_!_!_!_!_!_!_" << endl;
						//potted = false;
					}
				}
				//if (pairs[i].otherShape->getGeometryType() != PxGeometryType::eSPHERE)
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs) 
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				switch (pairs[i].shapes[0]->getSimulationFilterData().word0)
				{
				case FilterGroup::BALL:					
					cout << "Contact with ball!" << endl;
					break;

				case FilterGroup::GRASS:
					cout << "Contact with Grass!" << endl;
					break;

				case FilterGroup::SAND:
					cout << "Contact with Sand!" << endl;
					break;
				case FilterGroup::ICE:
					cout << "Contact with Ice!" << endl;
					break;

				default:
					break;
				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		pairFlags = PxPairFlag::eSOLVE_CONTACT;			
		pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;	
		
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{
		int par;
		PxTransform* lastBallPosition;
		Plane* plane, * outOfBoundsPlane;
		Cloth* flag;
		Box* box, * wall, *pole,* floorPannel, * hinge, * outOfBoundsTrigger;
		CorridorWall* corridor;
		WindMillBuilding* windMillBuilding;
		WindMillBlades* windMillBlades;
		Club* club;
		Sphere* ball;
		PxRigidDynamic* ballRigid;
		RevoluteJoint* windMillJoint;
		Trampoline* tramp;
		Joint* joint;
		PxTransform lastLocation;
		float timer;
		std::chrono::high_resolution_clock clock;
		//Pyramid* building;

		PxMaterial* ballMat = GetPhysics()->createMaterial(.1f, .1f, .4f);		//GoldBall
		PxMaterial* grassMat = GetPhysics()->createMaterial(.3f, .2f, .2f);		//Grass
		PxMaterial* woodMat = GetPhysics()->createMaterial(.3f, .1f, .5f);		//Wood
		PxMaterial* sandMat = GetPhysics()->createMaterial(1.f, 1.f, .1f);	//Sand
		PxMaterial* iceMat = GetPhysics()->createMaterial(.08f, .05f, .4f);	//Ice

		MySimulationEventCallback* my_callback;
		
	public:
		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene(CustomFilterShader) {};	// make it access the custom collision shader

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

			//cloth visualisation
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_HORIZONTAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_VERTICAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_BENDING, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_SHEARING, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit() 
		{ 
			par = 0;
			SetVisualisation();		
			//GetMaterial()->setDynamicFriction(.2f);

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);			

			//plane, catches ball when out of course;
			plane = new Plane(PxVec3(0.0f, 0.1f, 0.0f));
			plane->Color(PxVec3(210.f / 255.f, 210.f / 255.f, 210.f / 255.f));
			Add(plane);
			outOfBoundsPlane = new Plane();
			outOfBoundsPlane->Color(PxVec3(210.f/255.f,210.f/255.f,210.f/255.f));
			outOfBoundsPlane->SetTrigger(true);
			outOfBoundsPlane->Material(grassMat);
			Add(outOfBoundsPlane);

			float height = 1.0f;
			float _length = 10.0f;
			float _width = 11.f;

			club = new Club(PxTransform(PxVec3(0.0f, 10.0f, 6.0f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))), PxVec3(1.0f, 1.0f, 1.f));
			club->Color(color_palette[4]);
			club->SetupFiltering(FilterGroup::CLUB, FilterGroup::BALL);
			Add(club);

			ball = new Sphere(PxTransform(PxVec3(0.0f, height + 2.1f, 5.0f), PxQuat(PxIdentity)), 0.3f);
			ball->Color(color_palette[2]);
			ball->SetupFiltering(FilterGroup::BALL, FilterGroup::GRASS | FilterGroup::SAND | FilterGroup::ICE);
			ball->Name("ball");
			((PxRigidDynamic*)ball->Get())->setMass(1);
			((PxRigidDynamic*)ball->Get())->setAngularDamping(.1f);
			((PxRigidDynamic*)ball->Get())->setMassSpaceInertiaTensor(PxVec3(0.f));
		//	((PxRigidDynamic*)ball->Get())->setLinearVelocity(PxVec3(-140.0f, 0.0f, 0.0f));
			Add(ball);


			wall = new Box(PxTransform(PxVec3(0.0f, height + 1.1f, 5.0f)));
			wall->SetKinematic(true);
			Add(wall);

			hinge = new Box(PxTransform(PxVec3(0.f, 12.7f,5.5f)));
			hinge->Color(color_palette[0]);
			hinge->SetKinematic(true);
			Add(hinge);

			joint = new RevoluteJoint(hinge, PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPi / 2, PxVec3(1.f, 0.f, 0.f))), club, PxTransform(PxVec3(0.f, 10.f, 0.f)));
			//((RevoluteJoint*)club->Get())->DriveVelocity(2);

			//start back wall
			wall = new Box(PxTransform(PxVec3(.0f, (height*3) +.5f, 10.0f)), PxVec3(10.0f, 1.0f, 1.0f));
			wall->Color(color_palette[0]);
			wall->Name("startBackWall");
			wall->Material(woodMat);
			wall->SetKinematic(true);
			//Add(wall);
			//wall section 
			corridor = new CorridorWall(PxTransform(PxVec3(-10.0f, height * 3, -65.0f)), PxVec3(1.0f, 1.0f, 8.5f));
			corridor->Color(color_palette[0]);
			corridor->SetKinematic(true);
			corridor->Material(woodMat);
			Add(corridor);
			//end back wall
			wall = new Box(PxTransform(PxVec3(.0f, (height * 3) +.5f, -150.0f)), PxVec3(10.0f, 1.0f, 1.0f));
			wall->Color(color_palette[0]);
			wall->Name("endBackWall");
			corridor->Material(woodMat);
			wall->SetKinematic(true);

			Add(wall);

			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, 10.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[1]);
			floorPannel->SetKinematic(true);
			floorPannel->Material(grassMat);
			floorPannel->SetupFiltering(FilterGroup::GRASS, FilterGroup::BALL);
			Add(floorPannel);
			//trampoline section
			tramp = new Trampoline(PxTransform(PxVec3(0.0f, height - 1.1f, -5.0f)), PxVec3(8.7f, 2.8f, 4.7f), 200.0f, 0.0f);
			tramp->AddToScene(this);

			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, -20.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[1]);
			floorPannel->SetKinematic(true);
			floorPannel->Material(grassMat);
			floorPannel->SetupFiltering(FilterGroup::GRASS, FilterGroup::BALL);
			Add(floorPannel);

			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, -40.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[1]);
			floorPannel->SetKinematic(true);
			//Sand
			floorPannel->Material(grassMat);
			floorPannel->SetupFiltering(FilterGroup::SAND, FilterGroup::BALL);
			Add(floorPannel);
			
			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, -60.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[3]);
			floorPannel->SetKinematic(true);
			//Sand
			floorPannel->Material(sandMat);
			floorPannel->SetupFiltering(FilterGroup::SAND, FilterGroup::BALL);
			Add(floorPannel);
			
			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, -80.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[4]);
			floorPannel->SetKinematic(true);
			//ICE
			floorPannel->Material(iceMat);
			floorPannel->SetupFiltering(FilterGroup::ICE, FilterGroup::BALL);
			Add(floorPannel);

			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, -100.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[4]);
			floorPannel->SetKinematic(true);
			//ICE
			floorPannel->Material(iceMat);
			floorPannel->SetupFiltering(FilterGroup::ICE, FilterGroup::BALL);
			Add(floorPannel);
			
			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, -120.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[1]);
			floorPannel->SetKinematic(true);
			floorPannel->Material(grassMat);
			floorPannel->SetupFiltering(FilterGroup::GRASS, FilterGroup::BALL);
			Add(floorPannel);
			//floor pannel
			floorPannel = new Box(PxTransform(PxVec3(.0f, height + .5f, -140.0f)), PxVec3(_width, height, _length));
			floorPannel->Color(color_palette[1]);
			floorPannel->SetKinematic(true);
			floorPannel->Material(sandMat);
			floorPannel->SetupFiltering(FilterGroup::GRASS, FilterGroup::BALL);
			Add(floorPannel);
			
			//TODO Make This work.
			windMillBuilding = new WindMillBuilding(PxTransform(PxVec3(0.0f, height * 3, -30.0f)), PxVec3(3.0f, 3.f, 3.0f));
			windMillBuilding->SetKinematic(true);
			//building = new Pyramid(PxTransform(PxVec3(0, -3, 0)));
			Add(windMillBuilding);

			for (int i = 0; i < 1; i++)
			{
				//WindMill blades
				windMillBlades = new WindMillBlades(PxTransform(PxVec3(0.0f, (height * 3) + 10.f, -30.0f), PxQuat(PxPi / 4, PxVec3(1.f, 0.f, 0.f))), PxVec3(1.0f, 1.f, 1.0f));
				((PxRigidDynamic*)windMillBlades->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				//windMillBlades->GetShape(0)->setLocalPose()
				Add(windMillBlades);

				//windMill join.
				box = new Box(PxTransform(PxVec3(i * 20.f, (height * 3) + 7.f, -30.f)), PxVec3(2.f, 2.f, 2.f));
				box->SetKinematic(true);
				box->Color(color_palette[3]);
				Add(box);

				windMillJoint = new RevoluteJoint(box, PxTransform(PxVec3(0.f, 0.f, 4.5f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))), windMillBlades, PxTransform(PxVec3(0.f, 0.f, 0.f)));
				windMillJoint->DriveVelocity(2);
			}

			//flag pole
			pole = new Box(PxTransform(PxVec3(0.f, (height * 3) + 7.f, -130.f)), PxVec3(0.5f, 20.f, 0.5f));
			pole->SetKinematic(true);
			pole->Color(color_palette[3]);
			Add(pole);

			outOfBoundsTrigger = new Box(PxTransform(PxVec3(0.f, (height * 3), -130.f)), PxVec3(2.0f, 1.0f, 2.0f));
			//outOfBoundsTrigger->Color(color_palette[5]);
			outOfBoundsTrigger->SetTrigger(true);
			outOfBoundsTrigger->SetKinematic(true);
			Add(outOfBoundsTrigger);

			flag = new Cloth(PxTransform(PxVec3(0.f, (height * 3) + 19.f, -130.f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))), PxVec2(8.f, 8.f), 40,40, true);
			((PxCloth*)flag->Get())->setExternalAcceleration(PxVec3(5.0f, 8.0f, 5.0f));
			//((PxCloth*)cloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig());
			flag->Color(color_palette[3]);
			Add(flag);
						
			
			vector<PxRigidDynamic*> actors(px_scene->getNbActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC));
			if (actors.size() && (px_scene->getActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC, (PxActor**)&actors.front(), (PxU32)actors.size())))
			{
				//actors[1]->setMassSpaceInertiaTensor(PxVec3(0.f));
			}

			//setting custom cloth parameters
			//((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(1.f));
		}

		//Custom udpate function
		virtual void CustomUpdate() 
		{

			if (my_callback->potted)
			{
				flag->Color(color_palette[5]);
			}
			if (!((PxRigidDynamic*)ball->Get())->isSleeping())
				((PxRigidDynamic*)club->Get())->putToSleep();
			if (((PxRigidDynamic*)ball->Get())->getLinearVelocity().normalize() < 0.2f && !((PxRigidDynamic*)ball->Get())->isSleeping()) {
				((PxRigidDynamic*)ball->Get())->putToSleep();
				//cout << "sleep" << endl;
			}
			if (((PxRigidDynamic*)ball->Get())->isSleeping() && !(lastLocation == ((PxRigidDynamic*)ball->Get())->getGlobalPose()))
			{
				cout << "location update" << endl;
				lastLocation = ((PxRigidDynamic*)ball->Get())->getGlobalPose();
				PxTransform newPos = lastLocation;
				newPos.p.y += 9.9f;
				newPos.p.z += .8f;
				((PxRigidDynamic*)hinge->Get())->setGlobalPose(newPos);
				//((PxRigidDynamic*)hinge->Get())->setAngularVelocity(PxVec3(0.f));
				//hinge->GetShape()->setGlobalPose(newPos); 
			}
			if (((PxRigidDynamic*)ball->Get())->getGlobalPose().p.y < 1)	//ball to lower than the course, therefor out of play.
			{
				((PxRigidDynamic*)ball->Get())->setLinearVelocity(PxVec3(0.f));
				((PxRigidDynamic*)ball->Get())->setGlobalPose(lastLocation);
			}

		}

		/// An example use of key release handling
		void ExampleKeyReleaseHandler(int choice)
		{
			cerr << "Change cloth target" << endl;
			if(choice == 1)
				((PxCloth*)flag->Get())->setTargetPose(PxTransform(PxVec3(1.0f)));
			if(choice == 2)
				((PxCloth*)flag->Get())->setGlobalPose(PxTransform(PxVec3(5.0f)));
		}

		/// An example use of key presse handling
		void ExampleKeyPressHandler()
		{
			cerr << "I am pressed!" << endl;
		}
	};
}
