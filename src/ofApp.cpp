
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name: Sair Abbas
//  Date: 12/06/2021


#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;

	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();
	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	moon.loadModel("geo/moon-houdini.obj");
	moon.setScaleNormalization(false);

	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	bHide = true;

	//  Create Octree for testing.
	//
	octree.create(moon.getMesh(0), 20);

	// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	// load the shader
	//
	#ifdef TARGET_OPENGLES
		shader.load("shaders_gles/shader");
	#else
		shader.load("shaders/shader");
	#endif

	//Create lander
	loadLander("geo/lander.obj");

	//Initialize landing area
	landingArea = Box(Vector3(-25, 0, -25), Vector3(25, 1, 25));

	//Initialize background image
	backgroundImage.load("images/space.jpg");

	//Initialize sounds
	thrustSound.load("sounds/thrust.mp3");
	explosionSound.load("sounds/explosion.mp3");
	backgroundSound.load("sounds/background.mp3");
	backgroundSound.setVolume(0.1);
	backgroundSound.setLoop(true);
	backgroundSound.play();

	//Set up cameras
	theCam = &mainCam;
	mainCam.disableMouseInput();

	mainCam.setPosition(-170, 20, -170);
	mainCam.setNearClip(.1);
	mainCam.setFov(65.5);
	mainCam.lookAt(lander.getPosition());

	trackCam.setPosition(-90, 15, -30);
	trackCam.setNearClip(.1);
	trackCam.setFov(75.5);

	onBoardCam.lookAt(glm::vec3(0, -1, 0));
	onBoardCam.setNearClip(.1);
	onBoardCam.setFov(65.5);

	legCam.lookAt(glm::vec3(-1, 0, 1));
	legCam.setNearClip(.1);
	legCam.setFov(65.5);

	//Set up lander 
	turbForceLander = new TurbulenceForce(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
	gravityForceLander = new GravityForce(glm::vec3(0, -1.64, 0));
	thrustForceLander = new ThrustForce(glm::vec3(0, 0, 0));
	impulseForceLander = new ImpulseForce(glm::vec3(0, 0, 0));
	landerEmitter.sys->addForce(turbForceLander);
	landerEmitter.sys->addForce(gravityForceLander);
	landerEmitter.sys->addForce(thrustForceLander);
	landerEmitter.sys->addForce(impulseForceLander);
	landerEmitter.setPosition(lander.getPosition());
	landerEmitter.setVelocity(glm::vec3(0, 0, 0));
	landerEmitter.setGroupSize(1);
	landerEmitter.setLifespan(-1);
	landerEmitter.setOneShot(true);
	landerEmitter.setEmitterType(RadialEmitter);

	//Set up thruster 
	turbForceThrust = new TurbulenceForce(glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10));
	gravityForceThrust = new GravityForce(glm::vec3(0, 0, 0));
	thrustForceThrust = new ThrustForce(glm::vec3(0, -500, 0));
	thrustEmitter.sys->addForce(turbForceThrust);
	thrustEmitter.sys->addForce(gravityForceThrust);
	thrustEmitter.sys->addForce(thrustForceThrust);
	thrustEmitter.setVelocity(glm::vec3(0, 0, 0));
	thrustEmitter.setLifespan(0.2);
	thrustEmitter.setGroupSize(100);
	thrustEmitter.setOneShot(true);
	thrustEmitter.setEmitterType(DiscEmitter);

	//Set up crash explosion
	radialForce = new ImpulseRadialForce(5000);
	radialForce->setHeight(0.2);
	explosionEmitter.sys->addForce(turbForceThrust);
	explosionEmitter.sys->addForce(gravityForceLander);
	explosionEmitter.sys->addForce(radialForce);
	explosionEmitter.setVelocity(glm::vec3(0, 0, 0));
	explosionEmitter.setLifespan(4);
	explosionEmitter.setGroupSize(5000);
	explosionEmitter.setOneShot(true);
	explosionEmitter.setEmitterType(RadialEmitter);
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	
	//Check if game has started
	if (landerEmitter.sys->particles.size() > 0)
	{
		//Check for any collisions
		checkCollisions();
		//Update lander position with particle
		lander.setPosition(landerEmitter.sys->particles[0].position.x, 
			landerEmitter.sys->particles[0].position.y, 
			landerEmitter.sys->particles[0].position.z);
	}
	landerEmitter.update();

	//Update AGL value
	AGL = calculateAGL();

	//Update thrust position and sound player
	thrustEmitter.setPosition(lander.getPosition() + glm::vec3(0, 0.6, 0));
	thrustEffect();
	thrustEmitter.update();

	//Update explosion position
	explosionEmitter.setPosition(lander.getPosition() + glm::vec3(0, 3, 0));
	explosionEmitter.update();

	//Update cameras
	trackCam.lookAt(lander.getPosition());
	onBoardCam.setPosition(lander.getPosition() + glm::vec3(2, 1, 0));
	legCam.setPosition(lander.getPosition() + glm::vec3(4, 0.5, -4));

	//Check fuel time left
	if (time <= 0) 
	{
		score = "Out of Fuel";
		gameOver = true;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	//Load in VBO
	loadVbo();
	ofBackground(ofColor::black);

	//Draw GUI
	ofDisableDepthTest();
	backgroundImage.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	if (!bHide) gui.draw();
	ofEnableDepthTest();

	theCam->begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		moon.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		moon.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}

			if (bLanderSelected) {

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}

	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		moon.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}
	
	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
    }
	if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - mainCam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}
	ofPopMatrix();

	ofSetColor(255, 100, 90);
	glDepthMask(GL_FALSE);
	// this makes everything look glowy :)
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();
	// begin drawing in the camera
	//
	shader.begin();
	// draw particle emitter here..
	//
	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)thrustEmitter.sys->particles.size() + (int)explosionEmitter.sys->particles.size());
	particleTex.unbind();
	//  end drawing in the camera
	shader.end();
	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();
	// set back the depth mask
	//
	glDepthMask(GL_TRUE);

	theCam->end();

	ofDisableDepthTest();
	ofSetColor(ofColor::blue);
	//Game Start Captioning
	if (startedGame == false)
	{
		font.load("fonts/font.ttf", 70);
		font.drawString("Lunar Lander", 150, ofGetWindowHeight() / 3);
		font.load("fonts/font.ttf", 30);
		font.drawString("press Spacebar to continue", 200, ofGetWindowHeight() / 2 + 100);
	}
	//Game Over Captioning
	else if (gameOver == true)
	{
		font.load("fonts/font.ttf", 70);
		font.drawString("Game Over", 250, ofGetWindowHeight() / 3);
		font.load("fonts/font.ttf", 40);
		font.drawString(score, 300, ofGetWindowHeight() / 2.5 + 100);
	}
	ofEnableDepthTest();

	//Display fuel time left
	ofSetColor(ofColor::white);
	int roundedTime = time;
	ofDrawBitmapString("Fuel Left: " + to_string(roundedTime) + " seconds", ofGetWindowWidth() - 200, 30);

	//Display AGL value
	if(displayAGL)
		ofDrawBitmapString("Altitude (AGL): " + to_string(AGL) + " meters", ofGetWindowWidth() - 220, 50);
}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
	
	if (thrustEmitter.sys->particles.size() > 0)
	{
		vector<ofVec3f> sizes;
		vector<ofVec3f> points;
		//Add thrust emitter particles
		for (int i = 0; i < thrustEmitter.sys->particles.size(); i++) {
			points.push_back(thrustEmitter.sys->particles[i].position);
			sizes.push_back(ofVec3f(5));
		}
		// upload the data to the vbo
		//
		int total = (int)points.size();
		vbo.clear();
		vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
		vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
	}
	if (explosionEmitter.sys->particles.size() > 0)
	{
		vector<ofVec3f> sizes;
		vector<ofVec3f> points;
		//Add explosion emitter particles
		for (int i = 0; i < explosionEmitter.sys->particles.size(); i++) {
			points.push_back(explosionEmitter.sys->particles[i].position);
			sizes.push_back(ofVec3f(5));
		}
		// upload the data to the vbo
		//
		int total = (int)points.size();
		vbo.clear();
		vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
		vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
	}
}

// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);
	
	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}

void ofApp::keyPressed(int key) {

	switch (key) {
	case OF_KEY_UP:
		landerEmitter.sys->reset();
		turbForceLander->set(ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1)); 
		if (bShftKeyDown || bCtrlKeyDown) 
			thrustForceLander->set(ofVec3f(0, 0, -10));
		else {
			thrustForceLander->set(ofVec3f(0, 10, 0));
			thrustEmitter.sys->reset();
			thrustEmitter.start();
			thruster = true;
		}
		impulseForceLander->set(ofVec3f(0, 0, 0));
		time = time - 0.2;
		break;
	case OF_KEY_DOWN:
		landerEmitter.sys->reset();
		turbForceLander->set(ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1));
		if (bShftKeyDown || bCtrlKeyDown)
			thrustForceLander->set(ofVec3f(0, 0, 10));
		else
			thrustForceLander->set(ofVec3f(0, -10, 0));
		impulseForceLander->set(ofVec3f(0, 0, 0));
		break;
	case OF_KEY_LEFT:
		landerEmitter.sys->reset();
		turbForceLander->set(ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1));
		thrustForceLander->set(ofVec3f(-10, 0, 0));
		impulseForceLander->set(ofVec3f(0, 0, 0));
		break;
	case OF_KEY_RIGHT:
		landerEmitter.sys->reset();
		turbForceLander->set(ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1));
		thrustForceLander->set(ofVec3f(10, 0, 0));
		impulseForceLander->set(ofVec3f(0, 0, 0));
		break;
	case ' ':
		if (startedGame == false)
		{
			landerEmitter.sys->reset();
			landerEmitter.start();
			startedGame = true;
		}
		break;
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'D':
	case 'd':
		displayAGL = !displayAGL;
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		bHide = !bHide;
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'r':
		mainCam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &trackCam;
		break;
	case OF_KEY_F3:
		theCam = &onBoardCam;
		break;
	case OF_KEY_F4:
		theCam = &legCam;
		break;
	case OF_KEY_ALT:
		mainCam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		bShftKeyDown = true;
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {
	
	switch (key) {
	case OF_KEY_UP:
		thruster = false;
		thrustForceLander->set(ofVec3f(0, 0, 0));
		break;
	case OF_KEY_DOWN:
	case OF_KEY_LEFT:
	case OF_KEY_RIGHT:
		thrustForceLander->set(ofVec3f(0, 0, 0));
		break;
	case OF_KEY_ALT:
		mainCam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		bShftKeyDown = false;
		break;
	default:
		break;

	}
}

//Caculate distance between lander and ground level
int ofApp::calculateAGL() {
	//Create ray in the downwards direction
	Ray ray = Ray(Vector3(lander.getPosition().x, lander.getPosition().y, lander.getPosition().z), Vector3(0, -1, 0));
	TreeNode nodeRtn;
	//Check if ray intersects ground level
	if (octree.intersect(ray, octree.root, nodeRtn))
		//Return distance value between points
		return glm::length(octree.mesh.getVertex(nodeRtn.points[0]) - 
			glm::vec3(lander.getPosition().x, 
				lander.getPosition().y, 
				lander.getPosition().z));
}

//Check collisions between objects 
void ofApp::checkCollisions() {

	//Check if lander box collides with octree leaf node boxes
	ofVec3f min = lander.getSceneMin() + lander.getPosition();
	ofVec3f max = lander.getSceneMax() + lander.getPosition();
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	colBoxList.clear();
	if (octree.intersect(bounds, octree.root, colBoxList))
		collision = true;
	else
		collision = false;

	// apply impulse function
	//
	ofVec3f vel = landerEmitter.sys->particles[0].velocity; // velocity of particle
	ofVec3f norm = ofVec3f(0, 1, 0);  // just use vertical for normal for now
	ofVec3f f = (restitution + 1.0)*((-vel.dot(norm))*norm);
	// only bother to check for descending particles.
	//
	if (vel.y < 0 && collision == true)
	{
		//Check if correct landing
		if (f.y >= 0 && f.y <= 3 && gameOver == false)
			score = "Correct Landing";
		//Check if hard landing
		if (f.y > 3 && f.y <= 6 && gameOver == false)
			score = "Hard Landing";
		//Check if crash landing
		if (f.y > 6 && gameOver == false)
		{
			score = "Crash Landing";
			//Activate explosion
			explosionEmitter.sys->reset();
			explosionEmitter.start();
			explosionSound.play();
			//Bounce lander high up
			f.y = 50;
			gameOver = true;
		}
		impulseForceLander->apply(f);
		//Check if lander collides with landing area box
		if (bounds.overlap(landingArea))
			gameOver = true;
	}
}

//Thrust effect sound player
void ofApp::thrustEffect() {
		//Check if thruster is toggled on
		if (thruster) 
		{
			//Check if thruster is not playing
			if (!thrustSound.isPlaying()) 
			{
				//Play sound
				thrustSound.play();
			}
		}
		//If thruster is toggled off
		else 
		{
			//Stop sound
			thrustSound.stop();
		}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (mainCam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (mainCam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = mainCam.getPosition();
		glm::vec3 mouseWorld = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), mainCam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = mainCam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - mainCam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (mainCam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, mainCam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		TreeNode nodeRtn;
		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);

	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {
	ofVec3f point;
	raySelectWithOctree(point);
	mainCam.setPosition(point + glm::vec3(0, 10, 0));
	mainCam.lookAt(lander.getPosition());
}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//Load in lander
void ofApp::loadLander(string file) {

	if (lander.loadModel(file)) {
		lander.setScaleNormalization(false);
		lander.setScale(1, 1, 1);
		lander.setPosition(-150, 20, -150);
		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}
	}
	else cout << "Error: Can't load model " << file << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - mainCam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = mainCam.getPosition();
		glm::vec3 camAxis = mainCam.getZAxis();
		glm::vec3 mouseWorld = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = mainCam.getPosition();
	glm::vec3 camAxis = mainCam.getZAxis();
	glm::vec3 mouseWorld = mainCam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}
