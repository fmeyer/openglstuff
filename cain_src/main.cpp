/*
 * 3D Face Screensaver
 * Copyright (C) 2005 - 2007  Philippe Choquette
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <windows.h>
#include <scrnsave.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <time.h>
#include "resource.h"
#include "wireface.h"
#include "normals.h"

#define TIMER 1


void InitGL(HWND hWnd, HDC &hDC, HGLRC &hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void SetupAnimation(int Width, int Height);
void OnTimer(HDC hDC);
void updtNorms(void);


GLfloat dhauteur = 0.0;
sens dirMov = UP;
boolean move = FALSE;

GLint delaiExpr = 300;
boolean changeExpr = FALSE;
sentiment expression = NEUTRE;
sentiment exprDesiree = NEUTRE;

GLint delaiYeux = 200;
boolean yeuxBougent = FALSE;
dirYeux yeuxActuel = CENTRE;
dirYeux yeuxDesire = CENTRE;
GLfloat angleYeux = 0.0;

boolean loin = TRUE;
GLint nbOui = 3;
GLfloat angle1 = 0.0; /* rotation autour de l'axe x */
GLfloat angle2 = 0.0; /* rotation autour de l'axe y */
boolean angle1aug = TRUE;
GLint angle1delai = 0;
boolean angle2aug = TRUE;

etatProche proche = SETZERO;
GLfloat proche_x = 0.0;
GLfloat proche_y = 0.0;
GLfloat proche_z = 0.0;
GLfloat proche_dx = 120.0 / ETAPES_PROCHE;
GLfloat proche_dy = 12.459 / ETAPES_PROCHE;
GLfloat proche_dz = -10.538 / ETAPES_PROCHE;
GLint attendProche;
GLfloat vientGauche_dy = 2*12.459 / ETAPES_PROCHE;

GLfloat* vert2 = NULL;
short stepTrans = 0;
GLfloat increm[VERTICES_BOX01*3];
int toInc[VERTICES_BOX01];
int nInc = 0;

GLint pupil[24];

GLfloat vertices[VERTICES_BOX01*3];
GLfloat norm[VERTICES_BOX01*3];

int Width, Height; //globals for size of screen


BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    return TRUE;
}

BOOL WINAPI ScreenSaverConfigureDialog
 (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hIDOK;

    switch (msg) {
        case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch LOWORD(wParam)
		{
		case IDOK:
			EndDialog(hDlg, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		return FALSE;
    }
    return FALSE;
}

LONG WINAPI ScreenSaverProc
 (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static HDC hDC;
  static HGLRC hRC;
  static RECT rect;

  switch ( msg ) {

  case WM_CREATE: 
    // get window dimensions
    GetClientRect( hWnd, &rect );
    Width = rect.right;
    Height = rect.bottom;

	// setup OpenGL, then animation
    InitGL( hWnd, hDC, hRC );
    SetupAnimation(Width, Height);

	//set timer to tick every 10 ms
    SetTimer( hWnd, TIMER, 10, NULL );
    return 0;

  case WM_DESTROY:
    KillTimer( hWnd, TIMER );
    CloseGL( hWnd, hDC, hRC );
    return 0;

  case WM_TIMER:
    OnTimer(hDC);	//animate!	
    return 0;
  }

	return DefScreenSaverProc(hWnd, msg, wParam, lParam);
}


// Initialize OpenGL
static void InitGL(HWND hWnd, HDC &hDC, HGLRC &hRC)
{
   static PIXELFORMATDESCRIPTOR pfd =   //static ensures data is stored between calls
   {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,                                //nVersion should be set to 1
      PFD_DRAW_TO_WINDOW |              //buffer can draw to window
      PFD_SUPPORT_OPENGL |              //buffer supports OpenGL drawing
      PFD_DOUBLEBUFFER,                 //buffer is double buffered
      PFD_TYPE_RGBA,                    //rgba pixels
      32,                               //32 bits color depth
      8, 0, 8, 0, 8, 0,                 //look up rest at msdn, color bits ignored
      0,                                //no alpha buffer
      0,                                //shift bit ignored
      0,                                //no accumulation buffer
      0, 0, 0, 0,                       //accumulation bits
      32,                               //32 bits z buffer
      0,                                //no stencil buffer
      0,                                //no auxiliary buffer
      PFD_MAIN_PLANE,                   //main drawing layer
      0,                                //reserved
      0, 0, 0                           //layer mask ignored
   };

   hDC = GetDC(hWnd);                              //retrieves a handle to a display device context
   int i = ChoosePixelFormat(hDC, &pfd);           //try and match a pixel format supported by DC
   SetPixelFormat(hDC, i, &pfd);
   hRC = wglCreateContext(hDC);                    //create a new OpenGL rendering context
   wglMakeCurrent(hDC, hRC);                       //makes OpenGL hRC the calling threads current RC

   short j;

   GLfloat mat_shininess[] = { 10.0 };
   GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat red_light[] = { 0.78, 0.2753, 0.2753, 1.0 };
   GLfloat blue_light[] = { 0.2753, 0.2753, 0.78, 1.0 };

   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_SMOOTH);
   glEnableClientState (GL_VERTEX_ARRAY);
 
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

   glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 22.5);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
   glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 22.5);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, red_light);
   glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 22.5);
   glLightfv(GL_LIGHT2, GL_DIFFUSE, blue_light);
   glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 22.5);
   glLightfv(GL_LIGHT3, GL_DIFFUSE, white_light);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHT1);
   glEnable(GL_LIGHT2);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   for(j = 0; j < 24; j++) pupil[j] = j + 24;
   
   //CopyMemory(vertices, verticesJoie, VERTICES_BOX01*3*sizeof(GLfloat));
   //updtNorms();
   CopyMemory(vertices, verticesNeutre, VERTICES_BOX01*3*sizeof(GLfloat));
   CopyMemory(norm, normNeutre, VERTICES_BOX01*3*sizeof(GLfloat));

   srand( (unsigned)time( NULL ) );
}

/* calcul de la matrice d'incrémentation des sommets */
void setInc(void)
{
   GLfloat dx, dy, dz;
   int i;
   
   switch(exprDesiree)
   {
      case NEUTRE:
         vert2 = verticesNeutre;
         break;
      case JOIE:
         vert2 = verticesJoie;
         break;
      case NEUTREBF:
         vert2 = verticesNeutreBF;
         break;
      case COLERE:
         vert2 = verticesColere;
         break;
   }
   
   nInc = 0;

   for (i = 0; i < VERTICES_BOX01; i++)
   {
      dx = vert2[3*i] - vertices[3*i];
      dy = vert2[3*i + 1] - vertices[3*i + 1];
      dz = vert2[3*i + 2] - vertices[3*i + 2];

      if (dx != 0.0 || dy != 0.0 || dz != 0.0)
      {
         nInc++;
         
         increm[3*(nInc-1)] = dx / ETAPES_TRANS;
         increm[3*(nInc-1) + 1] = dy / ETAPES_TRANS;
         increm[3*(nInc-1) + 2] = dz / ETAPES_TRANS;
         
         toInc[nInc-1] = i;
      }
   }
}


/* mise à jour des sommets */
void incVert(void)
{
   int i;
   
   for (i = 0; i < nInc; i++)
   {
      vertices[3*toInc[i]] += increm[3*i];
      vertices[3*toInc[i] + 1] += increm[3*i + 1];
      vertices[3*toInc[i] + 2] += increm[3*i + 2];
   }
}


/* recalcul des normales */
void updtNorms(void)
{
   int i,j;
   int ptrFlSize = sizeof(GLfloat*);
   char errMsg[] = "Erreur après realloc.";

   /* normales des faces dans lesquelles les sommets sont compris */
   GLfloat** vrtxFacesNorm[VERTICES_BOX01];
   GLfloat faceNormals[FACES_BOX01][3];

   GLint currSize[VERTICES_BOX01];

   for(i = 0; i < FACES_BOX01; i++)
      surfaceNormal(
         &vertices[3*faces[i][0]], 
         &vertices[3*faces[i][1]],
         &vertices[3*faces[i][2]], 
         &faceNormals[i][0]);

   ZeroMemory(currSize, VERTICES_BOX01*sizeof(GLint));
   ZeroMemory(vrtxFacesNorm, VERTICES_BOX01*sizeof(GLfloat**));

   /* initialisaition de vrtxFacesNorm */
   for(i = 0; i < FACES_BOX01; i++)
   {
      for(j = 0; j < 3; j++)
      {
         currSize[faces[i][j]]++;
         vrtxFacesNorm[faces[i][j]] =
            (GLfloat**)realloc(vrtxFacesNorm[faces[i][j]], 
            currSize[faces[i][j]]*ptrFlSize);
         if (vrtxFacesNorm[faces[i][j]] == NULL)
         {
            AnsiToOem(errMsg, errMsg);
            printf(errMsg);
            exit(1);
         }
         vrtxFacesNorm[faces[i][j]][currSize[faces[i][j]]-1] =
            &faceNormals[i][0];
      }
   }

   /* calcul des normales des sommets */
   for(i = 0; i < VERTICES_BOX01; i++)
   {
      vertexNormal(vrtxFacesNorm[i], currSize[i], &norm[i*3]);
      free(vrtxFacesNorm[i]);
   }
}


// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );

  ReleaseDC( hWnd, hDC );
}


void SetupAnimation(int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   //glOrtho(-68.0, 68.0, -51.0, 51.0, -1.0, 1.0);
   gluPerspective(45.0, 4.0/3.0, 1.5, 200.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


void OnTimer(HDC hDC) //increment and display
{
   GLint hasard;
   GLint choixExpr[3];
   
   /* position de la tête */
   if (loin)
   {
      if (nbOui > 0)
      {
         if (angle1delai == 0)
         {
            if (angle1aug)
            {
               angle1 += 0.05;
               if (angle1 >= 5.0)
               {
                  angle1 = 5.0;
                  angle1aug = FALSE;
                  angle1delai = 50;
               }
            }
            else
            {
               angle1 -= 0.05;
               if (angle1 <= -5.0)
               {
                  angle1 = -5.0;
                  angle1aug = TRUE;
                  angle1delai = 50;
               }
            }
         }
         else
         {
            angle1delai--;
         }

         if (angle2aug)
         {
            angle2 += 0.05;
            if (angle2 >= 20.0)
            {
               angle2 = 20.0;
               angle2aug = FALSE;
               nbOui--;
            }
         }
         else
         {
            angle2 -= 0.05;
            if (angle2 <= -20.0)
            {
               angle2 = -20.0;
               angle2aug = TRUE;
            }
         }
      } // if (nbOui > 0)
      else
      {
         loin = FALSE;
      }
   }
   else
   {
      switch(proche)
      {
         case SETZERO:
            if (angle1 != 0.0 || angle2 != 0.0)
            {
               if (angle1 > 0.0)
               {
                  angle1 -= 0.05;
                  if (angle1 < 0.0) angle1 = 0.0;
               }
               else if (angle1 < 0.0)
               {
                  angle1 += 0.05;
                  if (angle1 > 0.0) angle1 = 0.0;
               }

               if (angle2 > 0.0)
               {
                  angle2 -= 0.05;
                  if (angle2 < 0.0) angle2 = 0.0;
               }
               else if (angle2 < 0.0)
               {
                  angle2 += 0.05;
                  if (angle2 > 0.0) angle2 = 0.0;
               }
            }
            else
            {
               proche = VIENTDROIT;
            }
         break;
         case VIENTDROIT:
            if (proche_x < 120.0 && proche_y < 12.459 && proche_z > -10.538)
            {
               proche_x += proche_dx;
               proche_y += proche_dy;
               proche_z += proche_dz;
            }
            else
            {
               proche = ATTENDDROIT;
               attendProche = 200;
            }
         break;
         case ATTENDDROIT:
            attendProche--;
            if (attendProche == 0)
            {
               proche = VIENTGAUCHE;
            }
         break;
         case VIENTGAUCHE:
            if (proche_y > -12.459)
            {
               proche_y -= vientGauche_dy;
            }
            else
            {
               proche = ATTENDGAUCHE;
               attendProche = 200;
            }
         break;
         case ATTENDGAUCHE:
            attendProche--;
            if (attendProche == 0)
            {
               proche = QUITTEGAUCHE;
            }
         break;
         case QUITTEGAUCHE:
            if (proche_x > 0.0 && proche_y < 0.0 && proche_z < 0.0)
            {
               proche_x -= proche_dx;
               proche_y += proche_dy;
               proche_z -= proche_dz;
            }
            else
            {
               proche = SETZERO;
               loin = TRUE;
               proche_x = proche_y = proche_z = 0.0;
               nbOui = 3;
            }
         break;
      }
   }

   /* expression du visage */
   if (changeExpr)
   {
      stepTrans++;

      if (stepTrans == 1) setInc();

      incVert();

      updtNorms();

      if (stepTrans == ETAPES_TRANS)
      {
         stepTrans = 0;
         changeExpr = FALSE;
         expression = exprDesiree;
         delaiExpr = 300;
      }
   }
   else
   {
      delaiExpr--;
      if (delaiExpr == 0)
      {
         changeExpr = TRUE;
         // choix de la prochaine expression
         switch (expression)
         {
            case NEUTRE:
               choixExpr[0] = JOIE;
               choixExpr[1] = NEUTREBF;
               choixExpr[2] = COLERE;
            break;
            case JOIE:
               choixExpr[0] = NEUTRE;
               choixExpr[1] = NEUTREBF;
               choixExpr[2] = COLERE;
            break;
            case NEUTREBF:
               choixExpr[0] = NEUTRE;
               choixExpr[1] = JOIE;
               choixExpr[2] = COLERE;
            break;
            case COLERE:
               choixExpr[0] = NEUTRE;
               choixExpr[1] = JOIE;
               choixExpr[2] = NEUTREBF;
            break;
         }

         exprDesiree = (sentiment)choixExpr[rand() % 3];
      }
   }

   /* orientation des yeux */
   if (yeuxBougent)
   {
      switch (yeuxActuel)
      {
         case GAUCHE:
            angleYeux -= 1.0;
            if (angleYeux <= 0.0)
            {
               yeuxBougent = FALSE;
               delaiYeux = rand() % 200;
               yeuxActuel = yeuxDesire;
               angleYeux = 0.0;
            }
         break;
         case CENTRE:
            if (yeuxDesire == GAUCHE)
            {
               angleYeux += 1.0;
               if (angleYeux >= 20.0)
               {
                  yeuxBougent = FALSE;
                  delaiYeux = rand() % 200;
                  yeuxActuel = yeuxDesire;
                  angleYeux = 20.0;
               }
            }
            else
            {
               angleYeux -= 1.0;
               if (angleYeux <= -20.0)
               {
                  yeuxBougent = FALSE;
                  delaiYeux = rand() % 200;
                  yeuxActuel = yeuxDesire;
                  angleYeux = -20.0;
               }
            }
         break;
         case DROITE:
            angleYeux += 1.0;
            if (angleYeux >= 0.0)
            {
               yeuxBougent = FALSE;
               delaiYeux = rand() % 200;
               yeuxActuel = yeuxDesire;
               angleYeux = 0.0;
           }
         break;
      }
   }
   else
   {
      delaiYeux--;
      if (delaiYeux <= 0)
      {
         yeuxBougent = TRUE;
         // choix de la prochaine position
         switch (yeuxActuel)
         {
            case GAUCHE:
               yeuxDesire = CENTRE;
            break;
            case CENTRE:
               hasard = rand() % 2;
               if (hasard) yeuxDesire = GAUCHE;
               else yeuxDesire = DROITE;
            break;
            case DROITE:
               yeuxDesire = CENTRE;
            break;
         }
      }
   }
   

   // Display

   GLfloat white_light_pos[] = {32.876, 0.0, -81.376, 1.0};
   GLfloat white_spot_dir[] = {-0.457936, 0.0, 0.888985};
   GLfloat red_light_pos[] = {-32.517, -58.405, -102.872, 1.0};
   GLfloat red_spot_dir[] = {0.0, 0.513511, 0.858083};
   GLfloat blue_light_pos[] = {-32.517, 58.405, -102.872, 1.0};
   GLfloat blue_spot_dir[] = {0.0, -0.513511, 0.858083};
   GLfloat white_light_pos2[] = {32.876, 0.0, 59.874, 1.0};
   GLfloat white_spot_dir2[] = {-0.457936, 0.0, -0.888985};
   GLfloat mat_white[] = {1.0, 1.0, 1.0, 1.0};
   GLfloat mat_brown[] = {0.745, 0.416, 0.17, 1.0};
   GLfloat mat_black[] = {0.0, 0.0, 0.0, 1.0};
   GLfloat emission_yeux[] = {0.3, 0.3, 0.3, 0.0};
   GLfloat emission_def[] = {0.0, 0.0, 0.0, 1.0};

   GLint i;
   
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glPushMatrix();
   
   gluLookAt(150.0, 0.0, 10.0, 0.0, 0.0, 10.0, 0.0, 0.0, 1.0);

   glRotatef(angle1, 1.0, 0.0, 0.0);
   glTranslatef(-20.0, 0.0, 0.0);
   glRotatef(angle2, 0.0, 1.0, 0.0);
   glTranslatef(20.0, 0.0, 0.0);

   glTranslatef(proche_x, proche_y, proche_z);

   glLightfv(GL_LIGHT0, GL_POSITION, white_light_pos);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, white_spot_dir);
   glLightfv(GL_LIGHT1, GL_POSITION, red_light_pos);
   glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, red_spot_dir);
   glLightfv(GL_LIGHT2, GL_POSITION, blue_light_pos);
   glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, blue_spot_dir);
   glLightfv(GL_LIGHT3, GL_POSITION, white_light_pos2);
   glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, white_spot_dir2);


   /* affichage du visage */

   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_white);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glEnableClientState(GL_NORMAL_ARRAY);
   glNormalPointer(GL_FLOAT, 0, norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_BOX01, GL_UNSIGNED_INT, faces);


   /* affichage des blancs d'yeux */

   glMaterialfv(GL_FRONT, GL_EMISSION, emission_yeux);

   glVertexPointer(3, GL_FLOAT, 0, eye_vertices);
   glNormalPointer(GL_FLOAT, 0, eye_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_GEO1, GL_UNSIGNED_INT, eye_faces);
   
   glPushMatrix();
   glTranslatef(0.0, 24.918, 0.0);
   glDrawElements(GL_TRIANGLES, 3*FACES_GEO1, GL_UNSIGNED_INT, eye_faces);
   glPopMatrix();

   glMaterialfv(GL_FRONT, GL_EMISSION, emission_def);


   /* affichage des iris */

   glPushMatrix();
   glTranslatef(-8.372, -12.459, 20.538);
   glRotatef(angleYeux, 0.0, 0.0, 1.0);
   glTranslatef(8.372, 12.459, -20.538);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_brown);
   glVertexPointer(3, GL_FLOAT, 0, iris_vertices);
   glDisableClientState (GL_NORMAL_ARRAY);
   for (i=0; i<FACES_CONE02; i++)
   {
      glNormal3fv(&iris_norm[3*i]);
      glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, &iris_faces[i][0]);
   }
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.0, 24.918, 0.0);
   glTranslatef(-8.372, -12.459, 20.538);
   glRotatef(angleYeux, 0.0, 0.0, 1.0);
   glTranslatef(8.372, 12.459, -20.538);
   glVertexPointer(3, GL_FLOAT, 0, iris_vertices);
   for (i=0; i<FACES_CONE02; i++)
   {
      glNormal3fv(&iris_norm[3*i]);
      glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, &iris_faces[i][0]);
   }
   glPopMatrix();
   
   
   /* affichage des pupilles */

   glPushMatrix();
   glTranslatef(-8.372, -12.459, 20.538);
   glRotatef(angleYeux, 0.0, 0.0, 1.0);
   glTranslatef(8.372, 12.459, -20.538);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_black);
   glDrawElements(GL_POLYGON, 24, GL_UNSIGNED_INT, pupil);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.0, 24.918, 0.0);
   glTranslatef(-8.372, -12.459, 20.538);
   glRotatef(angleYeux, 0.0, 0.0, 1.0);
   glTranslatef(8.372, 12.459, -20.538);
   glDrawElements(GL_POLYGON, 24, GL_UNSIGNED_INT, pupil);
   glPopMatrix();


   /* affichage des dents du bas */

   glEnable(GL_LIGHT3);

   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_white);
   glVertexPointer(3, GL_FLOAT, 0, dent01_vertices);
   glEnableClientState (GL_NORMAL_ARRAY);
   glNormalPointer(GL_FLOAT, 0, dent01_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT01, GL_UNSIGNED_INT, dent01_faces);
   glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, dent01_faces4);

   glVertexPointer(3, GL_FLOAT, 0, dent02_vertices);
   glNormalPointer(GL_FLOAT, 0, dent02_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT01, GL_UNSIGNED_INT, dent01_faces);
   glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, dent01_faces4);

   glVertexPointer(3, GL_FLOAT, 0, dent03_vertices);
   glNormalPointer(GL_FLOAT, 0, dent03_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT01, GL_UNSIGNED_INT, dent01_faces);
   glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, dent01_faces4);

   glVertexPointer(3, GL_FLOAT, 0, dent04_vertices);
   glNormalPointer(GL_FLOAT, 0, dent04_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT04, GL_UNSIGNED_INT, dent04_faces);

   glVertexPointer(3, GL_FLOAT, 0, dent05_vertices);
   glNormalPointer(GL_FLOAT, 0, dent05_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT04, GL_UNSIGNED_INT, dent04_faces);

   glVertexPointer(3, GL_FLOAT, 0, dent06_vertices);
   glNormalPointer(GL_FLOAT, 0, dent06_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT06, GL_UNSIGNED_INT, dent06_faces);

   glVertexPointer(3, GL_FLOAT, 0, dent07_vertices);
   glNormalPointer(GL_FLOAT, 0, dent07_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT06, GL_UNSIGNED_INT, dent06_faces);

   glDisable(GL_LIGHT3);


   /* affichage des dents du haut */

   glVertexPointer(3, GL_FLOAT, 0, dent08_vertices);
   glNormalPointer(GL_FLOAT, 0, dent08_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT08, GL_UNSIGNED_INT, dent08_faces);
   glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, dent08_faces4);

   glVertexPointer(3, GL_FLOAT, 0, dent09_vertices);
   glNormalPointer(GL_FLOAT, 0, dent09_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT08, GL_UNSIGNED_INT, dent08_faces);
   glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, dent08_faces4);

   glVertexPointer(3, GL_FLOAT, 0, dent10_vertices);
   glNormalPointer(GL_FLOAT, 0, dent10_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT10, GL_UNSIGNED_INT, dent10_faces);
   glDrawElements(GL_QUADS, 8, GL_UNSIGNED_INT, dent10_faces4);

   glVertexPointer(3, GL_FLOAT, 0, dent11_vertices);
   glNormalPointer(GL_FLOAT, 0, dent11_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT11, GL_UNSIGNED_INT, dent11_faces);

   glVertexPointer(3, GL_FLOAT, 0, dent12_vertices);
   glNormalPointer(GL_FLOAT, 0, dent12_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT11, GL_UNSIGNED_INT, dent11_faces);

   glVertexPointer(3, GL_FLOAT, 0, dent13_vertices);
   glNormalPointer(GL_FLOAT, 0, dent13_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT13, GL_UNSIGNED_INT, dent13_faces);

   glVertexPointer(3, GL_FLOAT, 0, dent14_vertices);
   glNormalPointer(GL_FLOAT, 0, dent14_norm);
   glDrawElements(GL_TRIANGLES, 3*FACES_DENT13, GL_UNSIGNED_INT, dent13_faces);


   glPopMatrix();

   SwapBuffers(hDC);
}