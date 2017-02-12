#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <time.h>
#include <math.h>
#include "main.h"
#include "Structures.h"
#include "Camera.h"
#include "include/SDL2_gfxPrimitives.h"

struct cameraProperties camProps = {0, 0, 1, 70 * M_PI/180, 0.01, 100, 100, 0, 0, {0, 0, 0}};

static double rotMat[3*3];

static void perspectiveProjection(double *points, int numPoints, double *outPoints);
static void multMat(int n, int m, int p, double *mat1, double *mat2, double *out);
static void genRotZ(double t, double *out);
static void genRotY(double t, double *out);
static void genRotX(double t, double *out);

void camera_init()
{
    camProps.offsetX = screenWidth / 2;
    camProps.offsetY = screenHeight / 2;
    camProps.scale = screenWidth / 2;
    camProps.transform.y = 1;
    camProps.transform.z = -3;
    camProps.ar = screenWidth / screenHeight;
}

void camera_draw(World *w)
{
    //camera_transform.z += 0.1*deltaTime;
    //camProps.transform.y += deltaTime;
    //camProps.rotX = -25 * M_PI / 180;
    double rotXM[3*3], rotYM[3*3];
    genRotX(camProps.rotX, rotXM);
    genRotY(camProps.rotY, rotYM);
    multMat(3, 3, 3, rotXM, rotYM, rotMat);
    
    for(int g=0; g < w->numGameObjects; g++)
    {
        GameObject curG = w->gameObjects[g];
        int numPoints = curG.mesh.numPoints;
        double points[3 * numPoints];
        for(int p=0; p<numPoints; p++)
        {
            points[p] = curG.mesh.points[p] + curG.transform.x;
            points[p+numPoints] = curG.mesh.points[p+numPoints] + curG.transform.y;
            points[p+2*numPoints] = curG.mesh.points[p+2*numPoints] + curG.transform.z;
        }
        
        perspectiveProjection(points, numPoints, points);
        
        for(int i=0; i<numPoints; i++)
            for(int j=i+1; j<numPoints; j++)
            {
                /*printf("Draw %f, %f, %f, %f \n", points[i], 
                        points[i+numPoints],
                        points[j],
                        points[j+numPoints]);*/
                if(abs(points[i+2*numPoints] > 1 || points[j+2*numPoints] > 1)) continue;
                
                //fix linergba not drawing correctly for some inputs (probably too close to 0 distance = error)
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawLine(renderer,
                        (int)(camProps.offsetX + camProps.scale * points[i]), 
                        (int)(camProps.offsetY - camProps.scale * points[i+numPoints]),
                        (int)(camProps.offsetX + camProps.scale * points[j]),
                        (int)(camProps.offsetY - camProps.scale * points[j+numPoints]));//, 0, 0, 0, 255);
            }
    }
}

static void perspectiveProjection(double *points, int numPoints, double *outPoints)
{
    double localPoints[3 * numPoints];
    for(int i=0; i<numPoints; i++)
    {
        // Transform to local camera position
        localPoints[i] = points[i] - camProps.transform.x;
        localPoints[i+numPoints] = points[i+numPoints] - camProps.transform.y;
        localPoints[i+2*numPoints] = points[i+2*numPoints] - camProps.transform.z;
    }
    
    multMat(3, 3, numPoints, rotMat, localPoints, outPoints);
    for(int i=0; i<numPoints; i++)
    {
        // Transform to local camera position
        double z = outPoints[i+2*numPoints];
        double near = camProps.near;
        double far = camProps.far;
        outPoints[i] = outPoints[i] / (z * tan(camProps.fov / 2));
        outPoints[i+numPoints] = outPoints[i+numPoints] / (z * tan(camProps.fov / 2));
        outPoints[i+2*numPoints] = (-near-far)/(near-far) + 2*near*far/(near-far)/z;
    }
}

// mat1: n x m, mat2: m x p
static void multMat(int n, int m, int p, double *mat1, double *mat2, double *out)
{
    for(int j=0; j<p; j++)
    for(int i=0; i<n; i++)
    {
        double sum = 0;
        for(int k=0; k<m; k++)
            sum += mat1[i * m + k] * mat2[k * p + j];
        out[i * p + j] = sum;
    }
}

static void genRotZ(double t, double *out)
{
    out[0] = cos(t); out[1] = -sin(t); out[2] = 0;
    out[3] = sin(t); out[4] = cos(t);  out[5] = 0;
    out[6] = 0;      out[7] = 0;       out[8] = 1;
}

static void genRotY(double t, double *out)
{
    out[0] = cos(t);  out[1] = 0; out[2] = sin(t);
    out[3] = 0;       out[4] = 1; out[5] = 0;
    out[6] = -sin(t); out[7] = 0; out[8] = cos(t);
}

static void genRotX(double t, double *out)
{
    out[0] = 1; out[1] = 0;      out[2] = 0;
    out[3] = 0; out[4] = cos(t); out[5] = -sin(t);
    out[6] = 0; out[7] = sin(t); out[8] = cos(t);
}