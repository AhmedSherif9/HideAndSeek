#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 4800;

void setupCamera();
void setupLights();
void checkForEnvironment2();

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 65.0f, float eyeY = 2500.0f, float eyeZ = 105.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;

class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_palmtree;
Model_3DS model_chair;
Model_3DS model_apple;
Model_3DS model_table;
Model_3DS model_wardrobe;
Model_3DS model_coin1;
Model_3DS model_coin2;
Model_3DS model_coin3;
Model_3DS model_coin4;
Model_3DS model_door;
Model_3DS model_wall;
Model_3DS model_zombie;
Model_3DS model_character;
Model_3DS model_lamp;

// Textures
GLTexture tex_ground;

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-2500, 0, -2500);
	glTexCoord2f(5, 0);
	glVertex3f(2500, 0, -2500);
	glTexCoord2f(5, 5);
	glVertex3f(2500, 0, 2500);
	glTexCoord2f(0, 5);
	glVertex3f(-2500, 0, 2500);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

//=======================================================================
// Display Function
//=======================================================================

void drawWall(double thickness) {
	glPushMatrix();
	glTranslated(0.5, 0.5 * thickness, 0.5);
	glScaled(80.0, thickness, 80.0);
	glutSolidCube(1);
	glPopMatrix();
}

void checkforCoins() {
	if (model_character.pos.x == -700 && model_character.pos.z == -200) {
		model_coin3.pos.x = 120000000;
	}
	if (model_character.pos.x == 0 && model_character.pos.z == 600) {
		model_coin1.pos.x = 120000000;
	}
	if (model_character.pos.x == 600 && model_character.pos.z == -100) {
		model_coin4.pos.x = 120000000;
	}
	if (model_character.pos.x == 400 && model_character.pos.z == 1300) {
		model_coin2.pos.x = 120000000;
	}
	glutPostRedisplay();
}

void checkForEnvironment2() {
	if (model_coin3.pos.x == 120000000 && model_coin2.pos.x == 120000000
		&& model_coin1.pos.x == 120000000 && model_coin4.pos.x == 120000000
		&& model_character.pos.z == 600 && model_character.pos.x == -700) {
		model_character.pos.x -= 200;
	}
	glutPostRedisplay();
}

void myDisplay(void)
{
	setupCamera();
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	//walls
	//ground
	glColor3f(0.5, 0.35, 0.05);
	glPushMatrix();
	glRotated(-45, 0.0, 1.0, 0.0);
	glScaled(20,2,20);
	drawWall(0.02);
	glPopMatrix();

	//front
	glColor3f(0.4, 0.2, 0.0);
	glPushMatrix();
	glTranslated(575, 0, 575);
	glRotated(-45, 0.0, 1.0, 0.0);
	glRotated(90, 0.0, 0.0, 1.0);
	glScaled(20, 2, 20);
	drawWall(0.02);
	glPopMatrix();

	//back
	glPushMatrix();
	glTranslated(-555, 0, -555);
	glRotated(-45, 0.0, 1.0, 0.0);
	glRotated(90, 0.0, 0.0, 1.0);
	glScaled(20, 2, 20);
	drawWall(0.02);
	glPopMatrix();

	//right
	glPushMatrix();
	glTranslated(555, 0, -555);
	glRotated(45, 0.0, 1.0, 0.0);
	glRotated(90, 0.0, 0.0, 1.0);
	glScaled(20, 2, 20);
	drawWall(0.02);
	glPopMatrix();

	//left
	glPushMatrix();
	glTranslated(-575, 0, 575);
	glRotated(45, 0.0, 1.0, 0.0);
	glRotated(90, 0.0, 0.0, 1.0);
	glScaled(20, 2, 20);
	drawWall(0.02);
	glPopMatrix();

	// Draw Ground
	RenderGround();

	// Draw apple Model
	glPushMatrix();
	glTranslatef(0, 0, 1700);
	glScalef(1.0, 1.0, 1.0);
	model_apple.Draw();
	glPopMatrix();
	// Draw Tree Model
	glPushMatrix();
	glTranslatef(100, 0, 1700);
	glScalef(100.0, 100.0, 100.0);
	model_tree.Draw();
	glPopMatrix();
	// Draw apple Model
	glPushMatrix();
	glTranslatef(900, 0, 1700);
	glScalef(1.0, 1.0, 1.0);
	model_apple.Draw();
	glPopMatrix();

	// Draw apple Model
	glPushMatrix();
	glTranslatef(1800, 0, 600);
	glScalef(1.0, 1.0, 1.0);
	model_apple.Draw();
	glPopMatrix();
	// Draw Tree Model
	glPushMatrix();
	glTranslatef(1800, 0, 100);
	glScalef(100.0, 100.0, 100.0);
	model_tree.Draw();
	glPopMatrix();
	// Draw apple Model
	glPushMatrix();
	glTranslatef(1800, 0, 0);
	glScalef(1.0, 1.0, 1.0);
	model_apple.Draw();
	glPopMatrix();

	// Draw apple Model
	glPushMatrix();
	glTranslatef(-1670, 0, -1900);
	glScalef(1.0, 1.0, 1.0);
	model_apple.Draw();
	glPopMatrix();
	// Draw Tree Model
	glPushMatrix();
	glTranslatef(-1270, 0, -1700);
	glScalef(100.0, 100.0, 100.0);
	model_tree.Draw();
	glPopMatrix();

	// Draw apple Model
	glPushMatrix();
	glTranslatef(1500, 0, -1900);
	glScalef(1.0, 1.0, 1.0);
	model_apple.Draw();
	glPopMatrix();
	// Draw Tree Model
	glPushMatrix();
	glTranslatef(1000, 0, -1600);
	glScalef(100.0, 100.0, 100.0);
	model_tree.Draw();
	glPopMatrix();

	// Draw palm Tree Model
	glPushMatrix();
	glTranslatef(-1600, 0, 1000);
	glScalef(100.0, 100.0, 100.0);
	model_palmtree.Draw();
	glPopMatrix();
	// Draw apple Model
	glPushMatrix();
	glTranslatef(-2100, 0, -500);
	glScalef(1.0, 1.0, 1.0);
	model_apple.Draw();
	glPopMatrix();

	// Draw Table Model
	glPushMatrix();
	glTranslated(800, 0, 50);
	glRotated(45, 0.0, 1.0, 0.0);
	glScalef(3.0, 3.0, 3.0);
	model_table.Draw();
	glPopMatrix();

	// Draw Table Model
	glPushMatrix();
	glTranslated(50, 0, 700);
	glScalef(3.0, 3.0, 3.0);
	model_table.Draw();
	glPopMatrix();

	// Draw wardrobe Model
	glPushMatrix();
	glTranslated(-800, 0, 0);
	glRotated(-135, 0.0, 1.0, 0.0);
	glScalef(200.0, 200.0, 200.0);
	model_wardrobe.Draw();
	glPopMatrix();

	// Draw Table Model
	glPushMatrix();
	glTranslated(-400, 0, -280);
	glRotated(135, 0.0, 1.0, 0.0);
	glScalef(3.0, 3.0, 3.0);
	model_table.Draw();
	glPopMatrix();

	// Draw Chair Model
	glPushMatrix();
	glTranslated(50, 0, 650);
	glScalef(1.8, 1.8, 1.8);
	model_chair.Draw();
	glPopMatrix();

	// Draw Chair Model
	glPushMatrix();
	glTranslated(-400, 0, -180);
	glRotated(135, 0.0, 1.0, 0.0);
	glScalef(1.8, 1.8, 1.8);
	model_chair.Draw();
	glPopMatrix();
	// Draw Chair Model
	glPushMatrix();
	glTranslated(-500, 0, -280);
	glRotated(135, 0.0, 1.0, 0.0);
	glScalef(1.8, 1.8, 1.8);
	model_chair.Draw();
	glPopMatrix();
	// Draw Chair Model
	glPushMatrix();
	glTranslated(-300, 0, -280);
	glRotated(315, 0.0, 1.0, 0.0);
	glScalef(1.8, 1.8, 1.8);
	model_chair.Draw();
	glPopMatrix();
	// Draw Chair Model
	glPushMatrix();
	glTranslated(-300, 0, -480);
	glRotated(315, 0.0, 1.0, 0.0);
	glScalef(1.8, 1.8, 1.8);
	model_chair.Draw();
	glPopMatrix();

	// Draw Coin Model
	glPushMatrix();
	glTranslated(0, 100, 0);
	glScalef(1.0, 1.0, 1.0);
	model_coin1.Draw();
	glPopMatrix();
	// Draw Coin Model
	glPushMatrix();
	glTranslated(-800, 100, -180);
	glScalef(1.0, 1.0, 1.0);
	model_coin2.Draw();
	glPopMatrix();
	// Draw Coin Model
	glPushMatrix();
	glTranslated(1000, 100, 30);
	glScalef(1.0, 1.0, 1.0);
	model_coin3.Draw();
	glPopMatrix();
	// Draw Coin Model
	glPushMatrix();
	glTranslated(0, 100, 900);
	glScalef(1.0, 1.0, 1.0);
	model_coin4.Draw();
	glPopMatrix();

	// Draw Door Model
	glPushMatrix();
	glTranslated(550, 0,-550);
	glRotated(-45, 0.0, 1.0, 0.0);
	glScalef(1.0, 1.0, 1.0);
	model_door.Draw();
	glPopMatrix();

	// Draw monster Model
	glPushMatrix();
	glTranslated(100, 110, 100);
	glRotated(90, 1.0, 0.0, 0.0);
	glScalef(100.0, 100.0, 100.0);
	model_zombie.Draw();
	glPopMatrix();
	// Draw monster Model
	glPushMatrix();
	glTranslated(-1300, 110, 1000);
	glRotated(45, 0.0, 1.0, 0.0);
	glRotated(90, 1.0, 0.0, 0.0);
	glScalef(100.0, 100.0, 100.0);
	model_zombie.Draw();
	glPopMatrix();

	// Draw character Model
	glPushMatrix();
	glTranslated(400, 1, 400);
	glRotated(225, 0.0, 1.0, 0.0);
	glScalef(1.0, 1.0, 1.0);
	model_character.Draw();
	glPopMatrix();

	// Draw lamp Model
	glPushMatrix();
	glTranslated(0, 0, -800);
	//glRotated(225, 0.0, 1.0, 0.0);
	glScalef(0.25, 0.25, 0.25);
	model_lamp.Draw();
	glPopMatrix();

	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	glScalef(30, 30, 30);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


	glPopMatrix();

	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char key, int x, int y)
{
	float d = 20.0;

	switch (key) {
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 't':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 27:
		exit(0);
		break;
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	case 'i':
		model_character.rot.y = 0.0f;
		if (model_character.pos.z == 1300 &&
			(model_character.pos.x == 800 ||
				model_character.pos.x == 700 || model_character.pos.x == 600 || model_character.pos.x == 500 || model_character.pos.x == 400 ||
				model_character.pos.x == 300 || model_character.pos.x == 200 || model_character.pos.x == 100 ||
				model_character.pos.x == 0 || model_character.pos.x == -100 ||
				model_character.pos.x == -200 || model_character.pos.x == -300 ||
				model_character.pos.x == -400 || model_character.pos.x == -500 ||
				model_character.pos.x == -600 || model_character.pos.x == -700)) {
			//sound for walls
		}
		else if (model_character.pos.z == -300 &&
			(model_character.pos.x == 800 ||
				model_character.pos.x == 700 || model_character.pos.x == 600 || model_character.pos.x == 500 || model_character.pos.x == 400 ||
				model_character.pos.x == 300 || model_character.pos.x == 200 || model_character.pos.x == 100 ||
				model_character.pos.x == 0 || model_character.pos.x == -100 ||
				model_character.pos.x == -200 || model_character.pos.x == -300 ||
				model_character.pos.x == -400 || model_character.pos.x == -500 ||
				model_character.pos.x == -600 || model_character.pos.x == -700
				|| model_character.pos.x == -800)) {
			//sound for walls
		}
		else if ((model_character.pos.x == -400 && model_character.pos.z == -100) ||
			(model_character.pos.x == -500 && model_character.pos.z == -100) ||
			(model_character.pos.x == -600 && model_character.pos.z == -100) ||
			(model_character.pos.x == -700 && model_character.pos.z == -100) ||
			(model_character.pos.x == 300 && model_character.pos.z == -100) ||
			(model_character.pos.x == 300 && model_character.pos.z == -200) ||
			(model_character.pos.x == 400 && model_character.pos.z == -200) ||
			(model_character.pos.x == 500 && model_character.pos.z == -100) ||
			(model_character.pos.x == 600 && model_character.pos.z == 0) ||
			(model_character.pos.x == 100 && model_character.pos.z == 800)
			) {
			//sound for tables
		}
		else if ((model_character.pos.x == 500 && model_character.pos.z == 1000) ||
			(model_character.pos.x == 600 && model_character.pos.z == 1000)) {
			//sound for wardrobe
		}
		else if ((model_character.pos.x == -500 && model_character.pos.z == 1100) ||
			(model_character.pos.x == -600 && model_character.pos.z == 1100)) {
			//sound for bulb
		}
		else if ((model_character.pos.x == 1800 && model_character.pos.z == 900) ||
			(model_character.pos.x == 1900 && model_character.pos.z == 900) ||
			(model_character.pos.x == 1200 && model_character.pos.z == -800) || 
			(model_character.pos.x == 1100 && model_character.pos.z == -800) || 
			(model_character.pos.x == -1200 && model_character.pos.z == -900) || 
			(model_character.pos.x == -1800 && model_character.pos.z == 900) || 
			(model_character.pos.x == -1900 && model_character.pos.z == 900) || 
			(model_character.pos.x == -300 && model_character.pos.z == 2600) ) {
			//sound for trees
		}
		else if ((model_character.pos.x == 0 && model_character.pos.z == 900) ||
			(model_character.pos.x == 200 && model_character.pos.z == 900) ||
			(model_character.pos.x == 200 && model_character.pos.z == 1000) || 
			(model_character.pos.x == -100 && model_character.pos.z == 1000) ||
			(model_character.pos.x == -200 && model_character.pos.z == 1000)) {
			// sound for chair
		}
		else {
			model_character.pos.z += 100.0f;
		}
		std::cout << " z is: " << model_character.pos.z;
		break;
	case 'j':
		model_character.rot.y = 90.0f;
		if (model_character.pos.x == 800 &&
			(model_character.pos.z == -200 || model_character.pos.z == -100 || model_character.pos.z == 0 || model_character.pos.z == 100 ||
				model_character.pos.z == 200 || model_character.pos.z == 300 || model_character.pos.z == 400 ||
				model_character.pos.z == 500 || model_character.pos.z == 600 ||
				model_character.pos.z == 700 || model_character.pos.z == 800 ||
				model_character.pos.z == 900 || model_character.pos.z == 1000 ||
				model_character.pos.z == 1100 || model_character.pos.z == 1200 ||
				model_character.pos.z == 1300)) {
			//sound for walls
		}
		else if ((model_character.pos.x == 0 && model_character.pos.z == 900) ||
			(model_character.pos.x == 0 && model_character.pos.z == 1000) ||
			(model_character.pos.x == 0 && model_character.pos.z == 1100) ||
			(model_character.pos.x == 0 && model_character.pos.z == 1200) ||
			(model_character.pos.x == 200 && model_character.pos.z == -100) ||
			(model_character.pos.x == 300 && model_character.pos.z == 0) ||
			(model_character.pos.x == 400 && model_character.pos.z == 100) ||
			(model_character.pos.x == 500 && model_character.pos.z == 200)) {
			// sound for tables
		}
		else if ((model_character.pos.x == 400 && model_character.pos.z == 1000) ||
			(model_character.pos.x == 400 && model_character.pos.z == 1100) ||
			(model_character.pos.x == 400 && model_character.pos.z == 1200)) {
			// sound for wardrobe
		}
		else if ((model_character.pos.x == -700 && model_character.pos.z == 1100)) {
			// sound for bulb
		}
		else if ((model_character.pos.x == -100 && model_character.pos.z == 1000) ||
			(model_character.pos.x == -200 && model_character.pos.z == 1100) ||
			(model_character.pos.x == 300 && model_character.pos.z == 100)) {
			// sound for chair
		}
		else {
			model_character.pos.x += 100.0f;
		}
		std::cout << " x is: " << model_character.pos.x;
		break;
	case 'k':
		model_character.rot.y = 180.0f;
		if (model_character.pos.z == -200 &&
			(model_character.pos.x == 800 ||
				model_character.pos.x == 700 || model_character.pos.x == 600 || model_character.pos.x == 500 || model_character.pos.x == 400 ||
				model_character.pos.x == 300 || model_character.pos.x == 200 || model_character.pos.x == 100 ||
				model_character.pos.x == 0 || model_character.pos.x == -100 ||
				model_character.pos.x == -200 || model_character.pos.x == -300 ||
				model_character.pos.x == -400 || model_character.pos.x == -500 ||
				model_character.pos.x == -600 || model_character.pos.x == -700)) {
			// sound for walls
		}
		else if ((model_character.pos.x == 100 && model_character.pos.z == 1300) ||
			(model_character.pos.x == -700 && model_character.pos.z == 100) || 
			(model_character.pos.x == -600 && model_character.pos.z == 100) || 
			(model_character.pos.x == -500 && model_character.pos.z == 100) || 
			(model_character.pos.x == -400 && model_character.pos.z == 100) || 
			(model_character.pos.x == 600 && model_character.pos.z == 200) ||
			(model_character.pos.x == 500 && model_character.pos.z == 200) || 
			(model_character.pos.x == 400 && model_character.pos.z == 100) || 
			(model_character.pos.x == 300 && model_character.pos.z == 0)) {
			// sound for tables
		}
		else if ((model_character.pos.x == 600 && model_character.pos.z == 1300) ||
			(model_character.pos.x == 500 && model_character.pos.z == 1300)) {
			// sound for wardrobe
		}
		else if ((model_character.pos.x == -600 && model_character.pos.z == 1200) ||
			(model_character.pos.x == -500 && model_character.pos.z == 1200)) {
			// sound for bulb
		}
		else if ((model_character.pos.x == -100 && model_character.pos.z == 1200) ||
			(model_character.pos.x == -200 && model_character.pos.z == 1200) ||
			(model_character.pos.x == 0 && model_character.pos.z == 1100) ||
			(model_character.pos.x == 200 && model_character.pos.z == 1100) ||
			(model_character.pos.x == 200 && model_character.pos.z == 1200)
			|| (model_character.pos.x == 400 && model_character.pos.z == 200)) {
			// sound for chair
		}
		else {
			model_character.pos.z -= 100.0f;
		}
		std::cout << " z is: " << model_character.pos.z;
		break;
	case 'l':
		model_character.rot.y = -90.0f;
		if (model_character.pos.x == -700 &&
			(model_character.pos.z == -200 || model_character.pos.z == -100 || model_character.pos.z == 0 || model_character.pos.z == 100 ||
				model_character.pos.z == 200|| model_character.pos.z == 300 || model_character.pos.z == 400 ||
				model_character.pos.z == 500 || model_character.pos.z == 600 ||
				model_character.pos.z == 700 || model_character.pos.z == 800 || 
				model_character.pos.z == 900 || model_character.pos.z == 1000 || 
				model_character.pos.z == 1100 || model_character.pos.z == 1200 || 
				model_character.pos.z == 1300)) {
			//sound for walls
		}
		else if ((model_character.pos.x == 200 && model_character.pos.z == 1200) ||
			(model_character.pos.x == 200 && model_character.pos.z == 1100) ||
			(model_character.pos.x == 200 && model_character.pos.z == 1000) ||
			(model_character.pos.x == 200 && model_character.pos.z == 900) ||
			(model_character.pos.x == -300 && model_character.pos.z == 0) ||
			(model_character.pos.x == -300 && model_character.pos.z == -100) ||
			(model_character.pos.x == 700 && model_character.pos.z == 200) ||
			(model_character.pos.x == 700 && model_character.pos.z == 100) ||
			(model_character.pos.x == 600 && model_character.pos.z == 0) ||
			(model_character.pos.x == 500 && model_character.pos.z == -100)) {
			// sound for tables
		}
		else if ((model_character.pos.x == 700 && model_character.pos.z == 1000) ||
			(model_character.pos.x == 700 && model_character.pos.z == 1100) ||
			(model_character.pos.x == 700 && model_character.pos.z == 1200)) {
			// sound for wardrobe
		}
		else if ((model_character.pos.x == -500 && model_character.pos.z == 1100)) {
			// sound for bulb
		}
		else if ((model_character.pos.x == 300 && model_character.pos.z == 1000) ||
			(model_character.pos.x == 300 && model_character.pos.z == 1100)
			|| (model_character.pos.x == 0 && model_character.pos.z == 1100)) {
			// sound for chair
		}
		else {
			model_character.pos.x -= 100.0f;
		}
		std::cout << " x is: " << model_character.pos.x;
		break;
	default:
		break;
	}

	checkforCoins();
	checkForEnvironment2();
	glutPostRedisplay();
}
void Special(int key, int x, int y) {
	float a = 1.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	//model_wall.Load("Models/wall/wall.3ds");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_palmtree.Load("models/Tree3/Tree3.3ds");
	model_table.Load("Models/odesd2_B2_3ds/odesd2_B2_3ds.3ds");
	model_apple.Load("models/apple/apple.3ds");
	model_chair.Load("Models/odesd2_C4_3ds/odesd2_C4_3ds.3ds");
	model_wardrobe.Load("Models/Wardobe_3ds/MRWardobe.3ds");
	model_coin1.Load("models/3ds-coin/rc-coin.3ds");
	model_coin2.Load("models/3ds-coin/rc-coin.3ds");
	model_coin3.Load("models/3ds-coin/rc-coin.3ds");
	model_coin4.Load("models/3ds-coin/rc-coin.3ds");
	model_door.Load("models/Door_3DS/Door_Standart.3ds");
	model_character.Load("models/Terrorist/FatTerrorist.3ds");
	model_zombie.Load("models/Zombie/ZOMBIE.3ds");
	model_lamp.Load("models/lamp3ds/lamp.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

//................................................................................................

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy,aspectRatio,zNear,zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera.look();
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);

	glutSpecialFunc(Special);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}