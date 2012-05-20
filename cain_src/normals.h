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

#ifndef NORMALS_H
#define NORMALS_H

#include <math.h>
#include <stdlib.h>
#define  WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>


void surfaceNormal(GLfloat * pt0, GLfloat * pt1, GLfloat * pt2, 
GLfloat * outVect)
/* Donne la normale d'une surface.
 * Les points passés en paramètres doivent l'être dans le même ordre que le
 * polygone est défini (antihoraire face à l'extérieur).
 */
{
   GLfloat centre0[3], centre1[3], norme;
   
   centre0[0] = pt0[0]-pt1[0];
   centre0[1] = pt0[1]-pt1[1];
   centre0[2] = pt0[2]-pt1[2];

   centre1[0] = pt2[0]-pt1[0];
   centre1[1] = pt2[1]-pt1[1];
   centre1[2] = pt2[2]-pt1[2];
   
   if (outVect == NULL) exit(1);

   /* produit vectoriel */
   outVect[0] = centre1[1]*centre0[2] - centre0[1]*centre1[2];
   outVect[1] = centre0[0]*centre1[2] - centre1[0]*centre0[2];
   outVect[2] = centre1[0]*centre0[1] - centre0[0]*centre1[1];
   
   /* normalisation */
   norme = sqrt(outVect[0]*outVect[0] + outVect[1]*outVect[1] + outVect[2]
      *outVect[2]);
   outVect[0] /= norme;
   outVect[1] /= norme;
   outVect[2] /= norme;
}


void vertexNormal(GLfloat ** normFaces, int nbFaces, GLfloat * vNorm)
/* Donne le vecteur normal d'un sommet. 
 * Les vecteurs normaux
 */
{
   int i; 
   GLfloat norme;

   if (vNorm == NULL) exit(1);

   vNorm[0] = vNorm[1] = vNorm[2] = 0.0;
   for(i = 0; i<nbFaces ;i++)
   {
      vNorm[0] += normFaces[i][0];
      vNorm[1] += normFaces[i][1];
      vNorm[2] += normFaces[i][2];
   }

   /* normalisation */
   norme = sqrt(vNorm[0]*vNorm[0] + vNorm[1]*vNorm[1] + vNorm[2]*vNorm[2]);
   vNorm[0] /= norme;
   vNorm[1] /= norme;
   vNorm[2] /= norme;
}


#endif
