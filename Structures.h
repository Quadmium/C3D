typedef struct
{
   double *points;
   int numPoints;
} Mesh;

typedef struct
{
    double x, y, z;
} Transform;

typedef struct
{
    Mesh mesh;
    Transform transform;
} GameObject;

typedef struct
{
    GameObject *gameObjects;
    int numGameObjects;
} World;