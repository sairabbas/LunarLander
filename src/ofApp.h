//Sair Abbas

#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "glm/gtx/intersect.hpp"
#include "Octree.h"
#include "ParticleEmitter.h"
#include <dos.h>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void loadLander(string file);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		void checkCollisions();
		void thrustEffect();
		void loadVbo();
		int calculateAGL();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p , glm::vec3 n);

		//Cameras
		ofEasyCam mainCam;
		ofCamera trackCam;
		ofCamera onBoardCam;
		ofCamera legCam;
		ofCamera *theCam;

		ofxAssimpModelLoader moon, lander;
		Box boundingBox, landerBounds, landingArea;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;

		int AGL;
		string score = "No Landing";
		float time = 120;
		float restitution = 0.9;
	
		//Lander emitter
		ParticleEmitter landerEmitter;
		TurbulenceForce *turbForceLander;
		GravityForce *gravityForceLander;
		ThrustForce *thrustForceLander;
		ImpulseForce *impulseForceLander;

		//Thrust emitter
		ParticleEmitter thrustEmitter;
		TurbulenceForce *turbForceThrust;
		GravityForce *gravityForceThrust;
		ThrustForce *thrustForceThrust;

		//Explosion
		ParticleEmitter explosionEmitter;
		ImpulseRadialForce *radialForce;

		//Particle Rendering
		ofVbo vbo;
		ofShader shader;
		ofTexture  particleTex;

		ofxIntSlider numLevels;
		ofxPanel gui;
	
		bool thruster;
		bool landed = false;
		bool explode = false;
		bool collision;
		bool bShftKeyDown;
		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool gameOver = false;
		bool displayAGL = true;
		bool startedGame = false;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		bool bLanderLoaded;
		bool bTerrainSelected;
	
		ofImage backgroundImage;
		ofSoundPlayer thrustSound;
		ofSoundPlayer backgroundSound;
		ofSoundPlayer explosionSound;
		ofTrueTypeFont font;

		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;
};
