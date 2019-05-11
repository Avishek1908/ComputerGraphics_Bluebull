/*
 *
 * Demonstrates how to load and display an Wavefront OBJ file.
 * Using triangles and normals as static object. No texture mapping.
 *
 * OBJ files must be triangulated!!!
 * Non triangulated objects wont work!
 * You can use Blender to triangulate
 *
 */

#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#define KEY_ESCAPE 27
#define POINTS_PER_VERTEX 3
#define TOTAL_FLOATS_IN_TRIANGLE 9
using namespace std;
using namespace std;

/************************************************************************
  Window
 ************************************************************************/

typedef struct {
	int width;
	int height;
	char* title;

	float field_of_view_angle;
	float z_near;
	float z_far;
} glutWindow;



/***************************************************************************
  OBJ Loading
 ***************************************************************************/

class Model_OBJ
{
public:

	float* normals;							// Stores the normals
	float* Faces_Triangles;					// Stores the triangles
	float* vertexBuffer;					// Stores the points which make the object
	long TotalConnectedPoints;				// Stores the total number of connected verteces
	long TotalConnectedTriangles;			// Stores the total number of connected triangles






	Model_OBJ()
	{
		this->TotalConnectedTriangles = 0;
		this->TotalConnectedPoints = 0;
	}

	float* calculateNormal(float *coord1, float *coord2, float *coord3)
	{
		/* calculate Vector1 and Vector2 */
		float va[3], vb[3], vr[3], val;
		va[0] = coord1[0] - coord2[0];
		va[1] = coord1[1] - coord2[1];
		va[2] = coord1[2] - coord2[2];

		vb[0] = coord1[0] - coord3[0];
		vb[1] = coord1[1] - coord3[1];
		vb[2] = coord1[2] - coord3[2];

		/* cross product */
		vr[0] = va[1] * vb[2] - vb[1] * va[2];
		vr[1] = vb[0] * va[2] - va[0] * vb[2];
		vr[2] = va[0] * vb[1] - vb[0] * va[1];

		/* normalization factor */
		val = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);

		float norm[3];
		norm[0] = vr[0] / val;
		norm[1] = vr[1] / val;
		norm[2] = vr[2] / val;

		return norm;
	}


	int Load(char* filename)
	{
		string line;
		ifstream objFile(filename);
		if (objFile.is_open())													// If obj file is open, continue
		{
			objFile.seekg(0, ios::end);										// Go to end of the file, 
			long fileSize = objFile.tellg();									// get file size
			objFile.seekg(0, ios::beg);										// we'll use this to register memory for our 3d model

			vertexBuffer = (float*)malloc(fileSize);							// Allocate memory for the verteces
			Faces_Triangles = (float*)malloc(fileSize * sizeof(float));			// Allocate memory for the triangles
			normals = (float*)malloc(fileSize * sizeof(float));					// Allocate memory for the normals

			int triangle_index = 0;												// Set triangle index to zero
			int normal_index = 0;												// Set normal index to zero

			while (!objFile.eof())											// Start reading file data
			{
				getline(objFile, line);											// Get line from file

				if (line.c_str()[0] == 'v')										// The first character is a v: on this line is a vertex stored.
				{
					line[0] = ' ';												// Set first character to 0. This will allow us to use sscanf

					sscanf(line.c_str(), "%f %f %f ",							// Read floats from the line: v X Y Z
						&vertexBuffer[TotalConnectedPoints],
						&vertexBuffer[TotalConnectedPoints + 1],
						&vertexBuffer[TotalConnectedPoints + 2]);

					TotalConnectedPoints += POINTS_PER_VERTEX;					// Add 3 to the total connected points
				}
				if (line.c_str()[0] == 'f')										// The first character is an 'f': on this line is a point stored
				{
					line[0] = ' ';												// Set first character to 0. This will allow us to use sscanf

					int vertexNumber[4] = { 0, 0, 0 };
					sscanf(line.c_str(), "%i%i%i",								// Read integers from the line:  f 1 2 3
						&vertexNumber[0],										// First point of our triangle. This is an 
						&vertexNumber[1],										// pointer to our vertexBuffer list
						&vertexNumber[2]);										// each point represents an X,Y,Z.

					vertexNumber[0] -= 1;										// OBJ file starts counting from 1
					vertexNumber[1] -= 1;										// OBJ file starts counting from 1
					vertexNumber[2] -= 1;										// OBJ file starts counting from 1


					/********************************************************************
					 * Create triangles (f 1 2 3) from points: (v X Y Z) (v X Y Z) (v X Y Z).
					 * The vertexBuffer contains all verteces
					 * The triangles will be created using the verteces we read previously
					 */

					int tCounter = 0;
					for (int i = 0; i < POINTS_PER_VERTEX; i++)
					{
						Faces_Triangles[triangle_index + tCounter] = vertexBuffer[3 * vertexNumber[i]];
						Faces_Triangles[triangle_index + tCounter + 1] = vertexBuffer[3 * vertexNumber[i] + 1];
						Faces_Triangles[triangle_index + tCounter + 2] = vertexBuffer[3 * vertexNumber[i] + 2];
						tCounter += POINTS_PER_VERTEX;
					}

					/*********************************************************************
					 * Calculate all normals, used for lighting
					 */
					float coord1[3] = { Faces_Triangles[triangle_index], Faces_Triangles[triangle_index + 1],Faces_Triangles[triangle_index + 2] };
					float coord2[3] = { Faces_Triangles[triangle_index + 3],Faces_Triangles[triangle_index + 4],Faces_Triangles[triangle_index + 5] };
					float coord3[3] = { Faces_Triangles[triangle_index + 6],Faces_Triangles[triangle_index + 7],Faces_Triangles[triangle_index + 8] };
					float *norm = this->calculateNormal(coord1, coord2, coord3);

					tCounter = 0;
					for (int i = 0; i < POINTS_PER_VERTEX; i++)
					{
						normals[normal_index + tCounter] = norm[0];
						normals[normal_index + tCounter + 1] = norm[1];
						normals[normal_index + tCounter + 2] = norm[2];
						tCounter += POINTS_PER_VERTEX;
					}

					triangle_index += TOTAL_FLOATS_IN_TRIANGLE;
					normal_index += TOTAL_FLOATS_IN_TRIANGLE;
					TotalConnectedTriangles += TOTAL_FLOATS_IN_TRIANGLE;
				}
			}
			objFile.close();														// Close OBJ file
		}
		else
		{
			cout << "Unable to open file";
		}
		return 0;
	}

	void Release()
	{
		free(this->Faces_Triangles);
		free(this->normals);
		free(this->vertexBuffer);
	}

	void Draw()
	{
		glEnableClientState(GL_VERTEX_ARRAY);						// Enable vertex arrays
		glEnableClientState(GL_NORMAL_ARRAY);						// Enable normal arrays
		glVertexPointer(3, GL_FLOAT, 0, Faces_Triangles);				// Vertex Pointer to triangle array
		glNormalPointer(GL_FLOAT, 0, normals);						// Normal pointer to normal array
		glDrawArrays(GL_TRIANGLES, 0, TotalConnectedTriangles);		// Draw the triangles
		glDisableClientState(GL_VERTEX_ARRAY);						// Disable vertex arrays
		glDisableClientState(GL_NORMAL_ARRAY);						// Disable normal arrays
	}
};
/***************************************************************************
 * Program code
 ***************************************************************************/

Model_OBJ obj, obj1, lefthand, righthand, leftleg, rightleg, hammer, tanhammer,righthandhamless, tree, rambody, ramlefthand, ramrighthand, ramleftleg, ramrightleg , bluebull, ramhandwithbull, thanos, grave;
float g_rotation;
glutWindow win;
int hand_rot = 0;
int a = 1;
float thortr[3] = { 0.0, 0.0, 0.0 };
float thorrh[3] = { 0.0, 0.0, 0.0 };
float thorrhwh[3] = { 1.0, 1.0, 1.0 };
float hamm[3] = { 0.0, 0.0, 0.0 };
float hammhand[3] = { 0.0, 0.0, 0.0 };
float bodyrot = 0;
int stopthor = 1;
float throwhammer[3] = { -1.2,1.5,-6.2 };
GLfloat xRotated, yRotated, zRotated;
GLdouble radius = 1;
int thor = 1;
float ram[3] = { 0.0, 0.0, 0.0 };
float ramhandrot = 0.0;
float bluemot[3] = { 4.5, 30.0, -10.0 };
float ramhwb[3] = { 0.0,0.0,0.0 };
float ramhwob[3] = { 0.0,0.0,0.0 };
float bulls[3] = { 0.4, 0.4, 0.4 };
float rambullrot = 0;
float bulltrans = 1.4;
int ultabull = 0;
int handrot = 0;
float throwrot = 0.0;

float tans[3] = {0.0, 0.0, 0.0};
float tanr[3] = {0.0, 0.0, 0.0};
float hams[3] = {0.0,0.0,0.0};
float hamt[3] = {-10.0,8.0,-6.5};
float hamr = 0;
float gavs[3] = { 0.0,0.0,0.0 };

void display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0, 3, -10, 0, 0, 0, 0, 1, 0);

	glPushMatrix();
	glTranslatef(80.0, -127.0, 75.0);
	glColor3f(0, 1, 0);
	glRotatef(xRotated, 1.0, 0.0, 0.0);
	glRotatef(yRotated, 0.0, 1.0, 0.0);
	glRotatef(zRotated, 0.0, 0.0, 1.0);
	glScalef(1.0, 1.0, 1.0);
	glutSolidSphere(140, 20, 20);
	glFlush();

	glPopMatrix();
	glPushMatrix();
	glTranslatef(-60.0, -131.0, 75.0);
	glColor3f(0, 1, 0);
	glRotatef(xRotated, 1.0, 0.0, 0.0);
	glRotatef(yRotated, 0.0, 1.0, 0.0);
	glRotatef(zRotated, 0.0, 0.0, 1.0);
	glScalef(1.0, 1.0, 1.0);
	glutSolidSphere(140, 20, 20);
	glFlush();
	glPopMatrix();

	
	//Mountains

	glPushMatrix();
	glTranslatef(15.0, 1.5, 7.5);
	glColor3f(0.52, 0.37, 0.26);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glRotatef(0.0, 0.0, 1.0, 0.0);
	glRotatef(0.0, 0.0, 0.0, 1.0); 
	glScalef(1.0, 1.0, 1.0);
	glutSolidCone(9, 9.5, 50, 50);
	glFlush();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 1.5, 11.5);
	glColor3f(0.52, 0.37, 0.26);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glRotatef(0.0, 0.0, 1.0, 0.0);
	glRotatef(0.0, 0.0, 0.0, 1.0);
	glScalef(1.0, 1.0, 1.0);
	glutSolidCone(11, 11.5, 50, 50);
	glFlush();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-15.0, 1.5, 7.5);
	glColor3f(0.52, 0.37, 0.26);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glRotatef(0.0, 0.0, 1.0, 0.0);
	glRotatef(0.0, 0.0, 0.0, 1.0);
	glScalef(1.0, 1.0, 1.0);
	glutSolidCone(9, 9.5, 50, 50);
	glFlush();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.1f, 0.26f, 0);
	glRotatef(2.0, 1.0, 0.0, 0.0);
	glTranslatef(-9.0, 1.2, -0.3);
	tree.Draw();
	glPopMatrix();
	//THANOS ------------------------------------------------



	glPushMatrix();
	glRotatef(180, 0.0, 1.0, 0.0);
	glScalef(tans[0], tans[1], tans[2]);
	glTranslatef(tanr[1], tanr[1], tanr[2]);
	thanos.Draw();
	glPopMatrix();

	glPushMatrix();
	glScalef(gavs[0], gavs[1], gavs[2]);
	glRotatef(120, 0.0, 1.0, 0.0);
	glTranslatef(5.0, 0.5, -5.3);
	grave.Draw();
	glPopMatrix();

	glPushMatrix();
	glScalef(hams[0], hams[1], hams[2]);
	glTranslatef(hamt[0], hamt[1], hamt[2]);
	glRotatef(0, 1.0, 0.0, 0.0);
	glRotatef(hamr, 0.0, 0.0, 1.0);
	tanhammer.Draw();
	glPopMatrix();


	//---------------------------RAMDEV-----------------------------------

	glPushMatrix();
	glColor3f(0.0,0.0,1.0);
	glScalef(bulls[0], bulls[1], bulls[2]);
	glTranslatef(bluemot[0], bluemot[1], bluemot[2]);
	bluebull.Draw();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.26f, 0);
	glScalef(ram[0], ram[1], ram[2]);
	glRotatef(180.0, 0.0, 0.0, 1.0);
	glTranslatef(0.0, -1.9, -3.0);
	rambody.Draw();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.26f, 0);
	glScalef(ramhwob[0], ramhwob[1], ramhwob[2]);
	glTranslatef(1.01, 4.3, -3.0);
	glRotatef(throwrot, 0.0, 0.0, 1.0);
	glRotatef(ramhandrot, 1.0, 0.0, 0.0);
	lefthand.Draw();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.26f, 0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glScalef(ramhwb[0], ramhwb[1], ramhwb[2]);
	glTranslatef(1.9, 2.9, bulltrans);
	glRotatef(rambullrot, 1.0, 0.0, 0.0);
	ramhandwithbull.Draw();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.26f, 0);
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glScalef(ram[0], ram[1], ram[2]);
	glTranslatef(-0.2, 2.2, 3.6);
	leftleg.Draw();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.26f, 0);
	glScalef(ram[0], ram[1], ram[2]);
	glTranslatef(-1.0, 4.3, -3.0);
	glRotatef(ramhandrot, 1.0, 0.0, 0.0);
	righthandhamless.Draw();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.26f, 0);
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glScalef(ram[0], ram[1], ram[2]);
	glTranslatef(0.3, 2.2, 3.6);
	rightleg.Draw();
	glPopMatrix();







	glColor3f(0.95f, 0.95f, 0.95f);

	glPushMatrix();
	glScalef(hamm[0], hamm[1], hamm[2]);
	glTranslatef(throwhammer[0], throwhammer[1], throwhammer[2]);
	glRotatef(0, 1.0, 0.0, 0.0);
	hammer.Draw();
	glPopMatrix();

	glRotatef(-30, 0.0, 1.0, 0.0);
	glRotatef(bodyrot, 0.0, 1.0, 0.0);
	glTranslatef(thortr[0], thortr[1], thortr[2]);

	glPushMatrix();
	//glRotatef(g_rotation, 0, 1, 0);
	//glRotatef(90, 0, 1, 0);
	//g_rotation++;
	glTranslatef(0.0, 0.0, -1.0);
	obj1.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 3.0, 0.0);
	obj.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.5, 4.5, -0.4);
	glRotatef(hand_rot, 1.0, 0.0, 0.0);
	lefthand.Draw();
	glPopMatrix();
	
	glPushMatrix();
	glScalef(thorrhwh[0], thorrhwh[1], thorrhwh[2]);
	glTranslatef(-0.8, 4.5, -0.4);
	glRotatef(-hand_rot, 1.0, 0.0, 0.0);
	righthand.Draw();
	glPopMatrix();

	glPushMatrix();
	glScalef( thorrh[0], thorrh[1], thorrh[2] );
	glTranslatef(-0.8, 4.5, -0.4);
	glRotatef(-hand_rot, 1.0, 0.0, 0.0);
	righthandhamless.Draw();
	glPopMatrix();

	glPushMatrix();
	glScalef(hammhand[0], hammhand[1], hammhand[2]);
	glTranslatef(-1.8, throwhammer[1], -2.2);
	glRotatef(-hand_rot, 1.0, 0.0, 0.0);
	hammer.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.0, 3.0, -0.4);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(hand_rot, 1.0, 0.0, 0.0);
	leftleg.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.27, 3.0, -0.4);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(-hand_rot, 1.0, 0.0, 0.0);
	rightleg.Draw();
	glPopMatrix();

	//RAMDEV-------------------------------------------------------------------------------------

	glutSwapBuffers();
}


void initialize()
{
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, win.width, win.height);
	GLfloat aspect = (GLfloat)win.width / win.height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.1f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	GLfloat amb_light[] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
	GLfloat specular[] = { 0.7, 0.7, 0.3, 1 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}


void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case KEY_ESCAPE:
		exit(0);
		break;
	default:
		break;
	}
}


void handleAnim()
{

	if (thor == 1)
	{
		if (thortr[0] > 2)
		{
			if (thortr[2] < -4)
			{
				if (throwhammer[1] < -1.5)
				{
					if (bodyrot > 140)
					{

						if (thortr[2] < -7.5)
						{
							stopthor = 0;
							thor = 0;
							ram[0] = 1.0;
							ram[1] = 1.0;
							ram[2] = 1.0;
						}
						else
						{
							printf("%f \n", thortr[2]);
							thortr[2] -= 0.01;
						}

					}
					else
					{
						hamm[0] = 1.0;
						hamm[1] = 1.0;
						hamm[2] = 1.0;
						hammhand[0] = 0.0;
						hammhand[1] = 0.0;
						hammhand[2] = 0.0;
						stopthor = 1;
						bodyrot += 0.06;

					}
				}
				else
				{
					stopthor = 0;
					thorrh[0] = 1.0;
					thorrh[1] = 1.0;
					thorrh[2] = 1.0;
					hammhand[0] = 1.0;
					hammhand[1] = 1.0;
					hammhand[2] = 1.0;
					thorrhwh[0] = 0.0;
					thorrhwh[1] = 0.0;
					thorrhwh[2] = 0.0;
					throwhammer[1] -= 0.01;
					printf("%f \n", throwhammer);
				}

			}
			else
			{
				bodyrot = 40;
				thortr[2] -= 0.008;
				printf("%f \n", thortr[2]);
			}
		}
		else
		{
			thortr[0] += 0.008;
			thortr[2] -= 0.008;
			printf("%f  \n", thortr[0]);
		}

	}
	else if(thor == 0)
	{
		ram[0] = 1.0;
		ram[1] = 1.0;
		ram[2] = 1.0;

		ramhwob[0] = 1.0;
		ramhwob[1] = 1.0;
		ramhwob[2] = 1.0;
		if (handrot)
		{
			if (bluemot[1] < 14.0)
			{
				if ( ultabull)
				{

					if (rambullrot < 0)
					{
						if (ramhandrot < 3)
						{
							if (throwhammer[1] > 1.5)
							{
								if (throwrot > 60)
								{
									if (throwhammer[0] > 13)
									{
										thor = 2;
									}
									else
									{
										throwhammer[0] += 0.01;
									}

								}
								else
								{

									throwrot += 1;
								}
							}
							else
							{
								throwhammer[1] += 0.01;
								throwhammer[0] += 0.01;
								throwhammer[2] += 0.01;
							}
						}

						else
						{
							bulltrans = 1.9;


							ramhwb[0] = 0.0;
							ramhwb[1] = 0.0;
							ramhwb[2] = 0.0;

							ramhwob[0] = 1.0;
							ramhwob[1] = 1.0;
							ramhwob[2] = 1.0;

							ramhandrot -= 0.2;
						}

					}
					else
					{

						bulltrans = 0.8;

						rambullrot -= 0.2;
					}

				}
				else
				{
					ramhwb[0] = 1.0;
					ramhwb[1] = 1.0;
					ramhwb[2] = 1.0;

					ramhwob[0] = 0.0;
					ramhwob[1] = 0.0;
					ramhwob[2] = 0.0;

					bulls[0] = 0.0;
					bulls[1] = 0.0;
					bulls[2] = 0.0;

					rambullrot += 0.2;
					if(rambullrot > 40 )
					ultabull = 1.0;

				}

			}
			else
			{
				bluemot[1] -= 0.05;
			}
		}
		else
		{
			ramhandrot += 0.7;
			if (ramhandrot > 145)
				handrot = 1.0;
		}
	}

	else
	{
	ram[0] = 0;
	ram[1] = 0;
	ram[2] = 0;
	ramhwob[0] = 0;
	ramhwob[1] = 0;
	ramhwob[2] = 0;
	tans[0] = 1.0;
	tans[1] = 1.0;
	tans[2] = 1.0;

	if (tanr[2] > 4)
	{

		if (hamt[0] > 0)
		{
			hams[0] = 0.0;
			hams[1] = 0.0;
			hams[2] = 0.0;
			tans[0] = 0.0;
			tans[1] = 0.0;
			tans[2] = 0.0;
			gavs[0] = 1.0;
			gavs[1] = 1.0;
			gavs[2] = 1.0;
		}
		else
		{

			hams[0] = 0.7;
			hams[1] = 0.7;
			hams[2] = 0.7;
			hamt[0] += 0.1;
			hamr += 3;
			printf("%f\n", hamt[0]);
		}
	}
	else
	{
		tanr[2] = tanr[2] + 0.01;

	}
		
	
	}


	if (hand_rot == -60)
		a = 2;
	if (hand_rot == 60)
		a = 1;

	//Sleep(2);
	switch (a)
	{
	case 1: if(stopthor)hand_rot -= 1;
		break;
	case 2: if (stopthor)hand_rot += 1;
		break;

	}
	glutPostRedisplay();
}


int main(int argc, char **argv)
{

	win.width = 640;
	win.height = 480;
	win.field_of_view_angle = 100;
	win.z_near = 1.0f;
	win.z_far = 500.0f;
	// initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);  // Display Mode
	glutInitWindowSize(1000, 720);
	glutCreateWindow("color cuce");							// create Window
	xRotated = yRotated = zRotated = 30.0;
	xRotated = 43;
	yRotated = 50;
	glutDisplayFunc(display);									// register Display Function
	glutIdleFunc(handleAnim);									// register Idle Function
	glutKeyboardFunc(keyboard);								// register Keyboard Handler
	initialize();
	char ni[] = "E:\\Bhargav\\Projects\\OpenGL\\thor_face.obj";
	char ni1[] = "E:\\Bhargav\\Projects\\OpenGL\\thor_body.obj";
	char ni2[] = "E:\\Bhargav\\Projects\\OpenGL\\thor_left_hand.obj";
	char ni3[] = "E:\\Bhargav\\Projects\\OpenGL\\thor_right_hand_hammer.obj";
	char ni4[] = "E:\\Bhargav\\Projects\\OpenGL\\thor_left_leg.obj";
	char ni5[] = "E:\\Bhargav\\Projects\\OpenGL\\thor_right_leg.obj";
	char ham[] = "E:\\Bhargav\\Projects\\OpenGL\\hammer.obj";
	char rhand[] = "E:\\Bhargav\\Projects\\OpenGL\\thor_right_hand.obj";
	char treeobj[] = "E:\\Bhargav\\Projects\\OpenGL\\thorTree.obj";

	char ramchest[] = "E:\\Bhargav\\Projects\\OpenGL\\ramdev.obj";
	char ramlh[] = "E:\\Bhargav\\Projects\\OpenGL\\ramleftHand.obj";
	char ramll[] = "E:\\Bhargav\\Projects\\OpenGL\\ramleftLeg.obj";
	char ramrh[] = "E:\\Bhargav\\Projects\\OpenGL\\ramrightHand.obj";
	char ramrl[] = "E:\\Bhargav\\Projects\\OpenGL\\ramrightleg.obj";
	char bb[] = "E:\\Bhargav\\Projects\\OpenGL\\bluebull.obj";
	char rhwb[] = "E:\\Bhargav\\Projects\\OpenGL\\ramhandwithbull.obj";


	char tan1[] = "E:\\Bhargav\\Projects\\OpenGL\\THAN1.OBJ";
	char gav1[] = "E:\\Bhargav\\Projects\\OpenGL\\grave1.OBJ";


	obj.Load(ni);
	obj1.Load(ni1);
	lefthand.Load(ni2);
	righthand.Load(ni3);
	leftleg.Load(ni4);
	rightleg.Load(ni5);
	hammer.Load(ham);
	righthandhamless.Load(rhand);
	tree.Load(treeobj);
	bluebull.Load(bb);

	rambody.Load(ramchest);
	ramlefthand.Load(ramlh);
	ramleftleg.Load(ramll);
	ramrighthand.Load(ramrh);
	ramrightleg.Load(ramrl);
	ramhandwithbull.Load(rhwb);

	tanhammer.Load(ham);


	thanos.Load(tan1);
	grave.Load(gav1);

	glutMainLoop();												// run GLUT mainloop
	return 0;
}