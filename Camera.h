struct cameraProperties
{
    double 
    rotX, rotY,
    ar,
    fov,
    near,
    far,
    scale,
    offsetX, offsetY;
    Transform transform;
};

extern struct cameraProperties camProps;

void camera_init();
void camera_draw(World *w);