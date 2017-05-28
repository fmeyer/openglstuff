#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "wireface.h"
#include "normals.h"

void updtNorms(void);

GLfloat dhauteur = 0.0;
sens dirMov = UP;
bool move = false;

GLint delaiExpr = 300;
bool changeExpr = false;
sentiment expression = NEUTRE;
sentiment exprDesiree = NEUTRE;

GLint delaiYeux = 200;
bool yeuxBougent = false;
dirYeux yeuxActuel = CENTRE;
dirYeux yeuxDesire = CENTRE;
GLfloat angleYeux = 0.0;

bool loin = true;
GLint nbOui = 3;
GLfloat angle1 = 0.0; /* rotation autour de l'axe x */
GLfloat angle2 = 0.0; /* rotation autour de l'axe y */
bool angle1aug = true;
GLint angle1delai = 0;
bool angle2aug = true;

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
GLfloat* increm = NULL;
int* toInc = NULL;
int nInc = 0;

GLint pupil[24];

GLfloat vertices[VERTICES_BOX01*3];
GLfloat norm[VERTICES_BOX01*3];


void init(void)
{
   short i;

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

   for(i = 0; i < 24; i++) pupil[i] = i + 24;

   //CopyMemory(vertices, verticesJoie, VERTICES_BOX01*3*sizeof(GLfloat));
   //updtNorms();
   memcpy(vertices, verticesNeutre, VERTICES_BOX01*3*sizeof(GLfloat));
   memcpy(norm, normNeutre, VERTICES_BOX01*3*sizeof(GLfloat));

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
   free(increm);
   increm = NULL;
   free(toInc);
   toInc = NULL;

   for (i = 0; i < VERTICES_BOX01; i++)
   {
      dx = vert2[3*i] - vertices[3*i];
      dy = vert2[3*i + 1] - vertices[3*i + 1];
      dz = vert2[3*i + 2] - vertices[3*i + 2];

      if (dx != 0.0 || dy != 0.0 || dz != 0.0)
      {
         nInc++;
         increm = realloc(increm, 3*nInc*sizeof(GLfloat));
         toInc = realloc(toInc, nInc*sizeof(GLfloat));

         if (toInc == NULL || increm == NULL)
         {
            printf("erreur de reallocation dans setInc");
            exit(1);
         }

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

   memset(currSize, 0, VERTICES_BOX01*sizeof(GLint));
   memset(vrtxFacesNorm, 0, VERTICES_BOX01*sizeof(GLfloat**));

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
            // AnsiToOem(errMsg, errMsg);
            printf(errMsg);
            exit(1);
         }
         vrtxFacesNorm[faces[i][j]][currSize[faces[i][j]]-1] =
            &faceNormals[i][0];
      }
   }

   /* calcul des normales des sommets */
   for(i = 0; i < VERTICES_BOX01; i++)
      vertexNormal(vrtxFacesNorm[i], currSize[i], &norm[i*3]);
}


void anim(void)
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
                  angle1aug = false;
                  angle1delai = 50;
               }
            }
            else
            {
               angle1 -= 0.05;
               if (angle1 <= -5.0)
               {
                  angle1 = -5.0;
                  angle1aug = true;
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
               angle2aug = false;
               nbOui--;
            }
         }
         else
         {
            angle2 -= 0.05;
            if (angle2 <= -20.0)
            {
               angle2 = -20.0;
               angle2aug = true;
            }
         }
      } // if (nbOui > 0)
      else
      {
         loin = false;
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
               loin = true;
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
         changeExpr = false;
         expression = exprDesiree;
         delaiExpr = 300;
      }
   }
   else
   {
      delaiExpr--;
      if (delaiExpr == 0)
      {
         changeExpr = true;
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

         exprDesiree = choixExpr[rand() % 3];
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
               yeuxBougent = false;
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
                  yeuxBougent = false;
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
                  yeuxBougent = false;
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
               yeuxBougent = false;
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
      if (delaiYeux == 0)
      {
         yeuxBougent = true;
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

   glutPostRedisplay();
}


void display(void)
{
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

   glutSwapBuffers();
}


void reshape (int w, int h)
{
   /* fenêtre large */
   if (h/w < 0.75)
   {
      glViewport ((GLsizei) (w/2 - 2*h/3), 0, (GLsizei) (4*h/3), (GLsizei) h);
   }
   /* fenêtre haute */
   else
   {
      glViewport (0, (GLsizei) (h/2 - 3*w/8), (GLsizei) w, (GLsizei) (3*w/4));
   }
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(45.0, 4.0/3.0, 1.5, 200.0);
   glMatrixMode (GL_MODELVIEW);
}


int main(int argc, char** argv)
{
   //int i;

   /* traitement des paramètres
   for(i = 0; i < argc; i++)
   {
      if (strcmp(argv[i],"-l") == 0) enlight = true;
   }  */

   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (640, 480);
   glutInitWindowPosition (200, 200);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutIdleFunc(anim);
   glutMainLoop();

   return 0;
}
