//modified by: Nick Polach
//
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
//#include <unistd.h>
//#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/glu.h>
#include "fonts.h"
#include <iostream>
#include "ppm.h"

typedef float Flt;
typedef Flt Vec[3];
typedef Flt Matrix[4][4];
//some macros
#define MakeVector(x, y, z, v) (v)[0]=(x);(v)[1]=(y);(v)[2]=(z)
#define VecCross(a,b,c) (c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1];\
			       (c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2];\
(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]
#define VecSub(a,b,c) (c)[0] = (a)[0] - (b)[0]; \
			       (c)[1] = (a)[1] - (b)[1]; \
(c)[2] = (a)[2] - (b)[2]
#define rnd() (Flt)rand() / (Flt)RAND_MAX
//some constants
const Flt PI = 3.141592653589793;
const Vec upv = {0.0, 1.0, 0.0};

Ppmimage * bruteImage;
GLuint bruteTexture;
GLuint bruteSilhouette;

Ppmimage * flierImage;
GLuint flierTexture;
GLuint flierSilhouette;

Ppmimage * portalImage;
GLuint portalTexture;
GLuint portalSilhouette;

Ppmimage * fireballImage;
GLuint fireballTexture;
GLuint fireballSilhouette;

Ppmimage * gunImage;
GLuint gunTexture;
GLuint gunSilhouette;

Ppmimage * crosshairImage;
GLuint crosshairTexture;
GLuint crosshairSilhouette;

Ppmimage * floor1Image;
GLuint floor1Texture;

Ppmimage * wall1Image;
GLuint wall1Texture;

// TODO
// Explosion when bullet collides with enemy

class Player {
    public:
	Flt health;
	int score;

	Vec pos;
	Vec view;
	Flt angleH;
	Flt angleV;
	Vec upv;
	Flt lookSpeed;
	Flt moveSpeed;
	Flt strafeSpeed;

	Player() {
	    health = 100.0;
	    score = 0;

	    MakeVector(0.0, 0.0, 5.0, pos);
	    MakeVector(0.0, 0.0, 0.0, view);
	    angleH = 0.0f;
	    angleV = 0.0f;
	    MakeVector(0.0, 1.0, 0.0, upv);
	    lookSpeed = 0.0080f;
	    moveSpeed = .1f;
	    strafeSpeed = .10f;
	}

	bool wallCollide(Flt x, Flt z) {
	    bool collides = false;
	    // Left/Right wall collision
	    if (x < -22.4f) {
		collides = true;
	    } else if (x > 22.4f) {
		collides = true;
	    }

	    // Back/Front wall collision
	    if (z < -37.4f) {
		collides = true;
	    } else if (z > 7.4f) {
		collides = true;
	    }
	    return collides;
	}


	void moveForward() 
	{

	    float viewX = view[0] - pos[0];
	    float viewZ = view[2] - pos[2];

	    Flt newX = pos[0] + viewX * moveSpeed;
	    Flt newZ = pos[2] + viewZ * moveSpeed;

	    if (!wallCollide(newX, newZ)) {
		pos[0] = newX;
		pos[2] = newZ;

		view[0] += viewX * moveSpeed;
		view[2] += viewZ * moveSpeed;
	    }

	}

	void moveBackward() 
	{
	    float viewX = view[0] - pos[0];
	    float viewZ = view[2] - pos[2];

	    Flt newX = pos[0] - viewX * moveSpeed;
	    Flt newZ = pos[2] - viewZ * moveSpeed;

	    if (!wallCollide(newX, newZ)) {
		pos[0] = newX;
		pos[2] = newZ;

		view[0] -= viewX * moveSpeed;
		view[2] -= viewZ * moveSpeed;
	    }
	}

	void moveLeft() 
	{

	    float viewX = view[0] - pos[0];
	    float viewY = view[1] - pos[1];
	    float viewZ = view[2] - pos[2];

	    float x = ((viewY * upv[2]) - (viewZ * upv[1]));
	    float y = ((viewZ * upv[0]) - (viewX * upv[2]));
	    float z = ((viewX * upv[1]) - (viewY * upv[0]));

	    float magnitude = sqrt( (x * x) + (y * y) + (z * z) );

	    x /= magnitude;
	    y /= magnitude;
	    z /= magnitude;

	    Flt newX = pos[0] - x*strafeSpeed;
	    Flt newZ = pos[2] - z*strafeSpeed;

	    if (!wallCollide(newX, newZ)) {
		pos[0] = newX;
		pos[2] = newZ;

		view[0] -= x*strafeSpeed;
		view[2] -= z*strafeSpeed;
	    }
	}

	void moveRight() 
	{
	    float viewX = view[0] - pos[0];
	    float viewY = view[1] - pos[1];
	    float viewZ = view[2] - pos[2];

	    float x = ((viewY * upv[2]) - (viewZ * upv[1]));
	    float y = ((viewZ * upv[0]) - (viewX * upv[2]));
	    float z = ((viewX * upv[1]) - (viewY * upv[0]));

	    float magnitude = sqrt( (x * x) + (y * y) + (z * z) );

	    x /= magnitude;
	    y /= magnitude;
	    z /= magnitude;

	    Flt newX = pos[0] + x*strafeSpeed;
	    Flt newZ = pos[2] + z*strafeSpeed;

	    if (!wallCollide(newX, newZ)) {

		pos[0] = newX;
		pos[2] = newZ;

		view[0] += x*strafeSpeed;
		view[2] += z*strafeSpeed;
	    }
	}

	void lookUp() 
	{
	    angleV -= lookSpeed;
	    if (angleV < -1.0)
		angleV = -1.0;

	    view[1] = sin(angleV);
	}

	void lookDown() 
	{
	    angleV += lookSpeed;
	    if (angleV > 1.0)
		angleV = 1.0;

	    view[1] = sin(angleV);
	}

	void lookLeft() 
	{
	    angleH += lookSpeed;
	    view[0] = pos[0] + sin(angleH);
	    view[2] = pos[2] + -cos(angleH);
	}

	void lookRight() 
	{
	    angleH -= lookSpeed;
	    view[0] = pos[0] + sin(angleH);
	    view[2] = pos[2] + -cos(angleH);
	}
};

class Timers {
    public:
	double physicsRate;
	double oobillion;
	struct timespec timeStart, timeEnd, timeCurrent;
	struct timespec animTime; // Animation time
	Timers() {
	    physicsRate = 1.0 / 60.0;
	    oobillion = 1.0 / 1e9;
	    recordTime(&animTime);
	}
	double timeDiff(struct timespec *start, struct timespec *end) {
	    return (double)(end->tv_sec - start->tv_sec ) +
		(double)(end->tv_nsec - start->tv_nsec) * oobillion;
	}
	void timeCopy(struct timespec *dest, struct timespec *source) {
	    memcpy(dest, source, sizeof(struct timespec));
	}
	void recordTime(struct timespec *t) {
	    clock_gettime(CLOCK_REALTIME, t);
	}
};

class Brute {
    public:
	Vec pos;
	Vec vel;
	Flt health;
	Timers timer;
	int FRAMECOUNT;
	int spriteFrame;
	struct timespec lastHit;
	double delay;
	Brute() {
	    health = 200.0;
	    spriteFrame = 0;
	    delay = 0.35;
	    FRAMECOUNT = 2;
	    timer.recordTime(&lastHit);
	}
};


class Flier {
    public:
	Vec pos;
	Vec vel;
	Flt health;
	Timers timer;
	int FRAMECOUNT;
	int spriteFrame;
	struct timespec lastShot;
	Flt nextShotDelay;
	double delay;
	Flier() {
	    health = 100.0;
	    spriteFrame = 0;
	    delay = 0.125;
	    FRAMECOUNT = 4;
	    nextShotDelay = rnd() * 1;
	    timer.recordTime(&lastShot);
	}
};

class Fireball {
    public:
	Vec pos;
	Vec dir; // direction
	Timers timer;
	int FRAMECOUNT;
	int spriteFrame;
	double delay;
	Fireball() {
	    spriteFrame = 0;
	    delay = 0.125;
	    FRAMECOUNT = 5;
	}

};

class Bullet {
    public:
	Vec pos;
	Vec dir; // direction
	Timers timer;
	int FRAMECOUNT;
	int spriteFrame;
	double delay;
	Bullet() {
	    spriteFrame = 0;
	    delay = 0.125;
	    FRAMECOUNT = 5;
	}

};


class Portal {
    public:
	Vec pos;
	Timers timer;
	struct timespec lastSpawn;
	Flt nextSpawnDelay;
	int FRAMECOUNT;
	int spriteFrame;
	double delay;
	Portal() {
	    spriteFrame = 0;
	    delay = 0.1;
	    FRAMECOUNT = 5;
	    timer.recordTime(&lastSpawn);
	    nextSpawnDelay = rnd() * 5;
	}
};

class Explosion {
    public:
	Vec pos;
	Timers timer;
	Flt lifeLength;
	int FRAMECOUNT;
	int spriteFrame;
	double delay;
	Explosion() {
	    spriteFrame = 0;
	    delay = 0.125;
	    FRAMECOUNT = 5;
	    lifeLength=0.25;
	}
};

class Global {
    public:
	int xres, yres;
	int fps;
	int vsync;
	int shotReset;
	Flt aspectRatio;
	Player player;
	Matrix cameraMatrix;
	unsigned char key_states;
	unsigned char w_mask;
	unsigned char a_mask;
	unsigned char s_mask;
	unsigned char d_mask;
	GLfloat lightPosition[4];
	Brute * brutes;
	int nbrutes;
	Flier * fliers;
	int nfliers;
	Fireball * fireballs;
	int nfireballs;
	Bullet * bullets;
	int nbullets;
	Explosion * explosions;
	int nexplosions;


	Portal * portals;
	int nportals;

	Global() {
	    shotReset = 0;
	    xres = 0.0;
	    yres = 0.0;
	    fps = 0;
	    vsync = 1;


	    aspectRatio = (GLfloat)xres / (GLfloat)yres;
	    Player player;
	    // Masks for byte
	    // #7 = 0x80 = 1000 0000
	    // #6 = 0x40 = 0100 0000
	    // #5 = 0x20 = 0010 0000
	    // #4 = 0x10 = 0001 0000
	    // 
	    // d = 0x08 = 0000 1000
	    // s = 0x04 = 0000 0100
	    // a = 0x02 = 0000 0010
	    // w = 0x01 = 0000 0001
	    // Using w, a, s, d, shift
	    key_states = 0x00;
	    w_mask = 0x01;
	    a_mask = 0x02;
	    s_mask = 0x04;
	    d_mask = 0x08;

	    brutes = new Brute[100];
	    nbrutes = 0;
	    fliers = new Flier[100];
	    nfliers = 0;
	    fireballs = new Fireball[100];
	    nfireballs = 0;
	    bullets = new Bullet[100];
	    nbullets = 0;
	    explosions = new Explosion[100];
	    nexplosions = 0;


	    portals = new Portal[10];
	    nportals = 0;
	}
} g;

//X Windows wrapper class
class X11_wrapper {
    private:
	//Display *dpy;
	Window win;
	GLXContext glc;
    public:
	Display *dpy;
	X11_wrapper() {
	    //Look here for information on XVisualInfo parameters.
	    //http://www.talisman.org/opengl-1.1/Reference/glXChooseVisual.html
	    //
	    // Go to fullscreen

	    //g.xres = 1024;
	    //g.yres = 768;
	    g.xres = 0.0;
	    g.yres = 0.0;


	    GLint att[] = { GLX_RGBA,
		GLX_STENCIL_SIZE, 2,
		GLX_DEPTH_SIZE, 24,
		GLX_DOUBLEBUFFER, None };
	    XSetWindowAttributes swa;
	    setup_screen_res(g.xres, g.yres);
	    dpy = XOpenDisplay(NULL);

	    Window root = DefaultRootWindow(dpy);

	    int fullscreen = 0;
	    if (!g.xres && !g.yres) {
		XWindowAttributes getWinAttr;
		XGetWindowAttributes(dpy, root, &getWinAttr);
		g.xres = getWinAttr.width;
		g.yres = getWinAttr.height;
		setup_screen_res(g.xres, g.yres);
		XGrabKeyboard(dpy, root, False,
			GrabModeAsync, GrabModeAsync, CurrentTime);
		fullscreen = 1;
	    }

	    if (dpy == NULL) {
		printf("\ncannot connect to X server\n\n");
		exit(EXIT_FAILURE);
	    }
	    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	    if (vi == NULL) {
		printf("\nno appropriate visual found\n\n");
		exit(EXIT_FAILURE);
	    }
	    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	    swa.colormap = cmap;
	    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		StructureNotifyMask | SubstructureNotifyMask | PointerMotionMask | ButtonPressMask;

	    unsigned int winops = CWBorderPixel|CWColormap|CWEventMask;
	    if (fullscreen) {
		winops |= CWOverrideRedirect;
		swa.override_redirect = True;
	    }

	    win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
		    vi->depth, InputOutput, vi->visual,
		    winops, &swa);
	    //set_title("4490 OpenGL Lab-1");
	    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	    glXMakeCurrent(dpy, win, glc);
	    set_title();
	    show_mouse_cursor(0);
	}
	~X11_wrapper() {
	    XDestroyWindow(dpy, win);
	    XCloseDisplay(dpy);
	}
	void set_title() {
	    //Set the window title bar.
	    XMapWindow(dpy, win);
	}
	void set_title(const char *str) {
	    //Set the window title bar.
	    XMapWindow(dpy, win);
	    XStoreName(dpy, win, str);
	}
	void setup_screen_res(const int w, const int h) {
	    g.xres = w;
	    g.yres = h;
	    g.aspectRatio = (GLfloat)g.xres / (GLfloat)g.yres;
	}
	void reshape_window(int width, int height) {
	    //window has been resized.
	    setup_screen_res(width, height);
	    glViewport(0, 0, (GLint)width, (GLint)height);
	}
	void check_resize(XEvent *e) {
	    //The ConfigureNotify is sent by the server if the window is resized.
	    if (e->type != ConfigureNotify)
		return;
	    XConfigureEvent xce = e->xconfigure;
	    if (xce.width != g.xres || xce.height != g.yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	    }
	}
	bool getXPending() {
	    return XPending(dpy);
	}
	void swapBuffers() {
	    glXSwapBuffers(dpy, win);
	}
	XEvent getXNextEvent() {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    return e;
	}
	void set_mouse_position(int x, int y) {
	    XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
	}
	void show_mouse_cursor(const int onoff) {
	    if (onoff) {
		//this removes our own blank cursor.
		XUndefineCursor(dpy, win);
		return;
	    }
	    //vars to make blank cursor
	    Pixmap blank;
	    XColor dummy;
	    char data[1] = {0};
	    Cursor cursor;
	    //make a blank cursor
	    blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
	    if (blank == None)
		std::cout << "error: out of memory." << std::endl;
	    cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
	    XFreePixmap(dpy, blank);
	    //this makes you the cursor. then set it using this function
	    XDefineCursor(dpy, win, cursor);
	    //after you do not need the cursor anymore use this function.
	    //it will undo the last change done by XDefineCursor
	    //(thus do only use ONCE XDefineCursor and then XUndefineCursor):
	}
} x11;

void imageConvert()
{
    //    //clean up all images in master folder
    remove("./images/floor1.ppm");
    remove("./images/wall1.ppm");

    // Spritesheets
    remove("./images/brutesheet.ppm");
    remove("./images/fliersheet.ppm");
    remove("./images/portalsheet.ppm");
    remove("./images/fireballsheet.ppm");
    remove("./images/fireballsheet2.ppm");
    remove("./images/gunsheet.ppm");
    remove("./images/crosshairsheet.ppm");


    //    //convert images to ppm
    system("mogrify -format ppm ./images/floor1.jpg");
    system("mogrify -format ppm ./images/wall1.jpg");

    // Spritesheets
    system("mogrify -format ppm ./images/brutesheet.png");
    system("mogrify -format ppm ./images/fliersheet.jpg");
    system("mogrify -format ppm ./images/portalsheet.jpg");
    system("mogrify -format ppm ./images/fireballsheet.jpg");
    system("mogrify -format ppm ./images/fireballsheet2.jpg");
    system("mogrify -format ppm ./images/gunsheet.png");
    system("mogrify -format ppm ./images/crosshairsheet.png");
}

void imageClean()
{
    //clean up all images in master folder
    remove("./images/floor1.ppm");
    remove("./images/wall1.ppm");

    // Spritesheets
    remove("./images/brutesheet.ppm");
    remove("./images/fliersheet.ppm");
    remove("./images/portalsheet.ppm");
    remove("./images/fireballsheet.ppm");
    remove("./images/fireballsheet2.ppm");
    remove("./images/gunsheet.ppm");
    remove("./images/crosshairsheet.ppm");
}

void init_opengl();
void init_portals();

void spawnEnemies();
void shootBullet();

void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

void unitTest();

void init_image(char * imagePath, Ppmimage * image, GLuint * texture);
unsigned char *buildAlphaData(Ppmimage *img);
void init_alpha_image(char * imagePath, Ppmimage * image,
	GLuint * texture, GLuint * silhouette);


int main()
{
    srand(time(0));
    //unitTest();

    imageConvert();
    init_opengl();
    init_portals();
    int done=0;
    int frameCount=0;
    double physicsCountdown=0.0;
    double timeSpan;
    Timers frameTimer;
    Timers physicsTimer;
    frameTimer.recordTime(&frameTimer.timeStart);
    physicsTimer.recordTime(&physicsTimer.timeStart);
    while (!done) {
	while (x11.getXPending()) {
	    XEvent e = x11.getXNextEvent();
	    x11.check_resize(&e);
	    check_mouse(&e);
	    done = check_keys(&e);
	}
	if (g.player.health > 0) {

	    // Physics Timers
	    physicsTimer.recordTime(&physicsTimer.timeCurrent);
	    timeSpan = physicsTimer.timeDiff(&physicsTimer.timeStart, &physicsTimer.timeCurrent);
	    physicsTimer.timeCopy(&physicsTimer.timeStart, &physicsTimer.timeCurrent);
	    physicsCountdown += timeSpan;
	    while(physicsCountdown >= physicsTimer.physicsRate) {
		physics();
		physicsCountdown -= physicsTimer.physicsRate;
	    }

	    render();
	    x11.swapBuffers();

	    // FPS Counter
	    frameCount++;
	    frameTimer.recordTime(&frameTimer.timeCurrent);
	    timeSpan = frameTimer.timeDiff(&frameTimer.timeStart, &frameTimer.timeCurrent);
	    if (timeSpan >= 1.0f) {
		g.fps = frameCount;
		frameCount = 0;
		frameTimer.recordTime(&frameTimer.timeStart);
	    }
	    spawnEnemies();
	}
    }
    cleanup_fonts();
    imageClean();
    return 0;
}

void init_portals()
{
    MakeVector(12.5, 0.0, -2.5, g.portals[0].pos);
    MakeVector(12.5, 0.0, -27.5, g.portals[1].pos);
    MakeVector(-12.5, 0.0, -2.5, g.portals[2].pos);
    MakeVector(-12.5, 0.0, -27.5, g.portals[3].pos);
    g.nportals = 4;
}



void init_opengl()
{
    //OpenGL initialization
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, g.aspectRatio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    //Enable this so material colors are the same as vert colors.
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    //Ambient light
    GLfloat ambientColor[] = {0.3f, 0.3f, 0.3f, 1.0f}; //Color(0.2, 0.2, 0.2)
    //GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f}; //Color(0.2, 0.2, 0.2)
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);


    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();

    // Initialize brute image
    init_alpha_image((char *)"./images/brutesheet.ppm",
	    bruteImage, &bruteTexture, &bruteSilhouette);

    // Initialize flier image
    init_alpha_image((char *)"./images/fliersheet.ppm",
	    flierImage, &flierTexture, &flierSilhouette);

    // Initialize fireball image
    init_alpha_image((char *)"./images/fireballsheet2.ppm",
	    fireballImage, &fireballTexture, &fireballSilhouette);

    // Initialize gun image
    // 79x100
    init_alpha_image((char *)"./images/gunsheet.ppm",
	    gunImage, &gunTexture, &gunSilhouette);

    // Initialize crosshair image
    // 79x100
    init_alpha_image((char *)"./images/crosshairsheet.ppm",
	    crosshairImage, &crosshairTexture, &crosshairSilhouette);

    // Initialize portal image
    init_alpha_image((char *)"./images/portalsheet.ppm",
	    portalImage, &portalTexture, &portalSilhouette);

    // Initialize floor images
    init_image((char *)"./images/floor1.ppm",
	    floor1Image, &floor1Texture);

    // Initialize wall images
    init_image((char *)"./images/wall1.ppm",
	    wall1Image, &wall1Texture);

    //
    //Test the stencil buffer on this computer.
    //
    // https://www.opengl.org/discussion_boards/showthread.php/
    // 138452-stencil-buffer-works-on-one-machine-but-not-on-another
    //
    // Before you try using stencil buffer try this:
    // Code :
    // GLint stencilBits = 0;
    // glGetIntegerv(GL_STENCIL_BITS, &amp;stencilBits);
    // if (stencilBits < 1)
    //    MessageBox(NULL,"no stencil buffer.\n","Stencil test", MB_OK);
    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    if (stencilBits < 1) {
	printf("No stencil buffer on this computer.\n");
	printf("Exiting program.\n");
	exit(0);
    }

    // https://github.com/godotengine/godot/blob/master/platform/x11/context_gl_x11.cpp
    // Disable vsync
    //static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
    //glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalEXT");
    //GLXDrawable drawable = glXGetCurrentDrawable();
    //glXSwapIntervalEXT(x11.dpy, drawable, 0);
}

void check_mouse(XEvent *e)
{
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    if (g.shotReset == 0) {
		shootBullet();
		g.player.moveBackward();
		g.shotReset = 15;
	    }
	}
    }

    static int savex = 0;
    static int savey = 0;
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	//Mouse moved
	int xdiff = savex - e->xbutton.x;
	int ydiff = savey - e->xbutton.y;
	if (xdiff < 0) {
	    g.player.lookLeft();

	}
	else if (xdiff > 0) {
	    g.player.lookRight();

	}
	if (ydiff < 0) {
	    g.player.lookUp();

	}
	if (ydiff > 0) {
	    g.player.lookDown();

	}
	savex = g.xres/2;
	savey = g.yres/2;
	x11.set_mouse_position(g.xres/2,g.yres/2);
    }
}

int check_keys(XEvent *e)
{
    GLXDrawable drawable = glXGetCurrentDrawable();
    //Was there input from the keyboard?
    if (e->type != KeyPress && e->type != KeyRelease)
	return 0;

    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyRelease) {
	switch(key) {
	    // Camera angle
	    case XK_w:
		g.key_states = g.key_states&~(g.w_mask);
		break;
	    case XK_a:
		g.key_states = g.key_states&~(g.a_mask);
		break;
	    case XK_s:
		g.key_states = g.key_states&~(g.s_mask);
		break;
	    case XK_d:
		g.key_states = g.key_states&~(g.d_mask);
		break;
	}
    }

    if (e->type == KeyPress) {
	//Was there input from the keyboard?
	switch(key) {
	    // Camera angle
	    case XK_1:
		break;
	    case XK_w:
		g.key_states = g.key_states|g.w_mask;
		break;
	    case XK_a:
		g.key_states = g.key_states|g.a_mask;
		break;
	    case XK_s:
		g.key_states = g.key_states|g.s_mask;
		break;
	    case XK_d:
		g.key_states = g.key_states|g.d_mask;
		break;
	    case XK_v:
		g.vsync ^= 1;
		// https://github.com/godotengine/godot/blob/master/platform/x11/context_gl_x11.cpp
		// Disable vsync
		static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
		glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalEXT");
		if (g.vsync) 
		    glXSwapIntervalEXT(x11.dpy, drawable, 1);
		else
		    glXSwapIntervalEXT(x11.dpy, drawable, 0);
		break;
	    case XK_Escape:
		return 1;
	}
    }
    return 0;
}

void drawFloor()
{
    Flt w = 2.5;
    Flt d = 2.5;
    Flt h = -1.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, floor1Texture);
    glBegin(GL_QUADS);

    // Bottom floor
    for (int i = 0; i < 45; i+=5) {
	for (int j = 0; j <45; j+=5) {
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f( w-20+j, h,-d+5-i);

	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(-w-20+j, h,-d+5-i);

	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(-w-20+j, h, d+5-i);

	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f( w-20+j, h, d+5-i);
	}
    }

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);

    // Top floor (ceiling)
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(0, 10, 0);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, wall1Texture);
    glBegin(GL_QUADS);

    for (int i = 0; i < 45; i+=5) {
	for (int j = 0; j <45; j+=5) {
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f( w-20+j, h,-d+5-i);

	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(-w-20+j, h,-d+5-i);

	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(-w-20+j, h, d+5-i);

	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f( w-20+j, h, d+5-i);
	}
    }

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);

}

void drawWall()
{
    Flt w = 2.5;
    Flt d = 7.5;
    Flt h = -1.0;

    //Front&Back Walls
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(1.5, 1.5, 5);
    glRotatef(90, 1, 0, 0);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, wall1Texture);
    glBegin(GL_QUADS);
    for (int i = 0; i < 45; i+=5) {

	// Back Walls
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( w-i+18.5, h+3.5, -d);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w-i+18.5, h+3.5, -d);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w-i+18.5, h+3.5, d);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( w-i+18.5, h+3.5, d);

	// Front Walls
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( w-i+18.5, h-41.5, -d);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w-i+18.5, h-41.5, -d);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w-i+18.5, h-41.5, d);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( w-i+18.5, h-41.5, d);
    }
    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);

    //Left&Right Walls
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(1.5, 1.5, 5);
    glRotatef(90, 1, 0, 0);
    glRotatef(90, 0, 0, 1);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, wall1Texture);
    glBegin(GL_QUADS);

    for (int i = 0; i < 45; i+=5) {
	// Left Walls
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( w-i, h+25 ,-d);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w-i, h+25, -d);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w-i, h+25, d);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( w-i, h+25, d);

	// Right Walls
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( w-i, h-20 ,-d);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w-i, h-20, -d);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w-i, h-20, d);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( w-i, h-20, d);
    }

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void make_view_matrix(Vec p1, Vec p2, Matrix m)
{
    //Line between p1 and p2 form a LOS Line-of-sight.
    //	//A rotation matrix is built to transform objects to this LOS.
    //		//Diana Gruber  http://www.makegames.com/3Drotation/
    m[0][0]=m[1][1]=m[2][2]=1.0f;
    m[0][1]=m[0][2]=m[1][0]=m[1][2]=m[2][0]=m[2][1]=0.0f;
    Vec out = { p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2] };
    //
    Flt l1, len = out[0]*out[0] + out[1]*out[1] + out[2]*out[2];
    if (len == 0.0f) {
	MakeVector(0.0f,0.0f,1.0f,out);
    } else {
	l1 = 1.0f / sqrtf(len);
	out[0] *= l1;
	//out[1] *= l1;
	out[2] *= l1;
    }
    m[2][0] = out[0];
    //m[2][1] = out[1];
    m[2][2] = out[2];
    Vec up = { -out[1] * out[0], upv[1] - out[1] * out[1], -out[1] * out[2] };
    //
    len = up[0]*up[0] + up[1]*up[1] + up[2]*up[2];
    if (len == 0.0f) {
	MakeVector(0.0f,0.0f,1.0f,up);
    }
    else {
	l1 = 1.0f / sqrtf(len);
	up[0] *= l1;
	up[1] *= l1;
	up[2] *= l1;
    }

    //make left vector.
    VecCross(up, out, m[0]);
}


void vecScale(Vec v, Flt s)
{
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}

void spawnEnemies()
{

    double timeSpan;
    int enemyType;
    for (int i = 0; i < g.nportals; i++) {
	g.portals[i].timer.recordTime(&g.portals[i].timer.timeCurrent);
	timeSpan = g.portals[i].timer.timeDiff(&g.portals[i].lastSpawn, &g.portals[i].timer.timeCurrent);
	if (timeSpan > 5+g.portals[i].nextSpawnDelay) {
	    enemyType = rand()%2;
	    switch (enemyType) {
		case 0:
		    MakeVector(g.portals[i].pos[0], 0.0, g.portals[i].pos[2], g.brutes[g.nbrutes].pos);
		    MakeVector(0.0, 0.0, 0.0, g.brutes[g.nbrutes].vel);
		    g.nbrutes += 1;
		    g.portals[i].timer.timeCopy(&g.portals[i].lastSpawn, &g.portals[i].timer.timeCurrent);
		    break;
		case 1:
		    MakeVector(g.portals[i].pos[0], 1.2, g.portals[i].pos[2], g.fliers[g.nfliers].pos);
		    MakeVector(0.0, 0.0, 0.0, g.fliers[g.nfliers].vel);
		    g.nfliers += 1;
		    g.portals[i].timer.timeCopy(&g.portals[i].lastSpawn, &g.portals[i].timer.timeCurrent);
		    break;
	    }
	}
    }
}

void drawBrutes()
{
    Flt w = 0.5;
    Flt d = 0.75;
    Flt h = 0.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    for (int i = 0; i < g.nbrutes; i++) {

	glPushMatrix();
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, bruteSilhouette);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0f); //Alpha

	glTranslated(g.brutes[i].pos[0], g.brutes[i].pos[1]-.25, g.brutes[i].pos[2]);
	//glTranslated(g.brutes[i].pos[0], g.brutes[i].pos[1]-.5, g.brutes[i].pos[2]);
	///// Billboarding
	//Setup camera rotation matrix
	//
	Vec v;
	VecSub(g.brutes[i].pos, g.player.pos, v);
	Vec z = {0.0f, 0.0f, 0.0f};
	make_view_matrix(z, v, g.cameraMatrix);
	//
	//Billboard_to_camera();
	//
	float mat[16];
	mat[ 0] = g.cameraMatrix[0][0];
	mat[ 1] = g.cameraMatrix[0][1];
	mat[ 2] = g.cameraMatrix[0][2];
	mat[ 4] = g.cameraMatrix[1][0];
	mat[ 5] = g.cameraMatrix[1][1];
	mat[ 6] = g.cameraMatrix[1][2];
	mat[ 8] = g.cameraMatrix[2][0];
	mat[ 9] = g.cameraMatrix[2][1];
	mat[10] = g.cameraMatrix[2][2];
	mat[ 3] = mat[ 7] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0f;
	mat[15] = 1.0f;
	glMultMatrixf(mat);
	//
	///// End Billboarding

	float tx = g.brutes[i].spriteFrame * .5;

	glRotatef(90, 1, 0, 0);
	glBegin(GL_QUADS);

	glTexCoord2f(tx, 0.0f);
	glVertex3f( w, h,-d);

	glTexCoord2f(tx+.5, 0.0f);
	glVertex3f(-w, h,-d);

	glTexCoord2f(tx+.5, 1.0f);
	glVertex3f(-w, h, d);

	glTexCoord2f(tx, 1.0f);
	glVertex3f( w, h, d);

	glEnd();
	glPopMatrix();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}

void drawFliers()
{
    Flt w = 0.35;
    Flt d = 0.35;
    Flt h = 0.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    for (int i = 0; i < g.nfliers; i++) {

	glPushMatrix();
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, flierSilhouette);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0f); //Alpha

	glTranslated(g.fliers[i].pos[0], g.fliers[i].pos[1]-.5, g.fliers[i].pos[2]);
	///// Billboarding
	//Setup camera rotation matrix
	//
	Vec v;
	VecSub(g.fliers[i].pos, g.player.pos, v);
	Vec z = {0.0f, 0.0f, 0.0f};
	make_view_matrix(z, v, g.cameraMatrix);
	//
	//Billboard_to_camera();
	//
	float mat[16];
	mat[ 0] = g.cameraMatrix[0][0];
	mat[ 1] = g.cameraMatrix[0][1];
	mat[ 2] = g.cameraMatrix[0][2];
	mat[ 4] = g.cameraMatrix[1][0];
	mat[ 5] = g.cameraMatrix[1][1];
	mat[ 6] = g.cameraMatrix[1][2];
	mat[ 8] = g.cameraMatrix[2][0];
	mat[ 9] = g.cameraMatrix[2][1];
	mat[10] = g.cameraMatrix[2][2];
	mat[ 3] = mat[ 7] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0f;
	mat[15] = 1.0f;
	glMultMatrixf(mat);
	//
	///// End Billboarding

	float tx = g.fliers[i].spriteFrame * .25;

	glRotatef(90, 1, 0, 0);
	glBegin(GL_QUADS);

	glTexCoord2f(tx, 0.0f);
	glVertex3f( w, h,-d);

	glTexCoord2f(tx+.25, 0.0f);
	glVertex3f(-w, h,-d);

	glTexCoord2f(tx+.25, 1.0f);
	glVertex3f(-w, h, d);

	glTexCoord2f(tx, 1.0f);
	glVertex3f( w, h, d);

	glEnd();
	glPopMatrix();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}

void drawFireballs()
{
    Flt w = 0.15;
    Flt d = 0.15;
    Flt h = 0.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    for (int i = 0; i < g.nfireballs; i++) {

	glPushMatrix();
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, fireballSilhouette);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0f); //Alpha

	glTranslated(g.fireballs[i].pos[0], g.fireballs[i].pos[1]-.5, g.fireballs[i].pos[2]);
	///// Billboarding
	//Setup camera rotation matrix
	//
	Vec v;
	VecSub(g.fireballs[i].pos, g.player.pos, v);
	Vec z = {0.0f, 0.0f, 0.0f};
	make_view_matrix(z, v, g.cameraMatrix);
	//
	//Billboard_to_camera();
	//
	float mat[16];
	mat[ 0] = g.cameraMatrix[0][0];
	mat[ 1] = g.cameraMatrix[0][1];
	mat[ 2] = g.cameraMatrix[0][2];
	mat[ 4] = g.cameraMatrix[1][0];
	mat[ 5] = g.cameraMatrix[1][1];
	mat[ 6] = g.cameraMatrix[1][2];
	mat[ 8] = g.cameraMatrix[2][0];
	mat[ 9] = g.cameraMatrix[2][1];
	mat[10] = g.cameraMatrix[2][2];
	mat[ 3] = mat[ 7] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0f;
	mat[15] = 1.0f;
	glMultMatrixf(mat);
	//
	///// End Billboarding

	float tx = g.fireballs[i].spriteFrame * .20;

	glRotatef(90, 1, 0, 0);
	glBegin(GL_QUADS);

	glTexCoord2f(tx, 0.0f);
	glVertex3f( w, h,-d);

	glTexCoord2f(tx+.20, 0.0f);
	glVertex3f(-w, h,-d);

	glTexCoord2f(tx+.20, 1.0f);
	glVertex3f(-w, h, d);

	glTexCoord2f(tx, 1.0f);
	glVertex3f( w, h, d);

	glEnd();
	glPopMatrix();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}

void drawExplosions()
{
    Flt w = 0.35;
    Flt d = 0.35;
    Flt h = 0.0;

    double timeSpan;
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    for (int i = 0; i < g.nexplosions; i++) {
	g.explosions[i].timer.recordTime(&g.explosions[i].timer.timeCurrent);
	timeSpan = g.explosions[i].timer.timeDiff(&g.explosions[i].timer.timeStart, &g.explosions[i].timer.timeCurrent);

	//printf("timeSpan: %f\n", timeSpan);
	if (timeSpan < g.explosions[i].lifeLength) {
	    w =  timeSpan / g.explosions[i].lifeLength;
	    d =  timeSpan / g.explosions[i].lifeLength;
	    glPushMatrix();
	    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	    glBindTexture(GL_TEXTURE_2D, fireballSilhouette);
	    glEnable(GL_ALPHA_TEST);
	    glAlphaFunc(GL_GREATER, 0.0f); //Alpha

	    glTranslated(g.explosions[i].pos[0], g.explosions[i].pos[1]-.5, g.explosions[i].pos[2]);
	    ///// Billboarding
	    //Setup camera rotation matrix
	    //
	    Vec v;
	    VecSub(g.explosions[i].pos, g.player.pos, v);
	    Vec z = {0.0f, 0.0f, 0.0f};
	    make_view_matrix(z, v, g.cameraMatrix);
	    //
	    //Billboard_to_camera();
	    //
	    float mat[16];
	    mat[ 0] = g.cameraMatrix[0][0];
	    mat[ 1] = g.cameraMatrix[0][1];
	    mat[ 2] = g.cameraMatrix[0][2];
	    mat[ 4] = g.cameraMatrix[1][0];
	    mat[ 5] = g.cameraMatrix[1][1];
	    mat[ 6] = g.cameraMatrix[1][2];
	    mat[ 8] = g.cameraMatrix[2][0];
	    mat[ 9] = g.cameraMatrix[2][1];
	    mat[10] = g.cameraMatrix[2][2];
	    mat[ 3] = mat[ 7] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0f;
	    mat[15] = 1.0f;
	    glMultMatrixf(mat);
	    //
	    ///// End Billboarding

	    float tx = g.explosions[i].spriteFrame * .20;

	    glRotatef(90, 1, 0, 0);
	    glBegin(GL_QUADS);

	    glTexCoord2f(tx, 0.0f);
	    glVertex3f( w, h,-d);

	    glTexCoord2f(tx+.20, 0.0f);
	    glVertex3f(-w, h,-d);

	    glTexCoord2f(tx+.20, 1.0f);
	    glVertex3f(-w, h, d);

	    glTexCoord2f(tx, 1.0f);
	    glVertex3f( w, h, d);

	    glEnd();
	    glPopMatrix();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_ALPHA_TEST);
    }
}


void drawBullets()
{
    Flt w = 0.05;
    Flt d = 0.05;
    Flt h = 0.0;

    glColor4f(0.0, 0.2, 0.2, 1.0); // reset gl color
    for (int i = 0; i < g.nbullets; i++) {

	glPushMatrix();
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, fireballSilhouette);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0f); //Alpha

	glTranslated(g.bullets[i].pos[0], g.bullets[i].pos[1]-.5, g.bullets[i].pos[2]);
	///// Billboarding
	//Setup camera rotation matrix
	//
	Vec v;
	VecSub(g.bullets[i].pos, g.player.pos, v);
	Vec z = {0.0f, 0.0f, 0.0f};
	make_view_matrix(z, v, g.cameraMatrix);
	//
	//Billboard_to_camera();
	//
	float mat[16];
	mat[ 0] = g.cameraMatrix[0][0];
	mat[ 1] = g.cameraMatrix[0][1];
	mat[ 2] = g.cameraMatrix[0][2];
	mat[ 4] = g.cameraMatrix[1][0];
	mat[ 5] = g.cameraMatrix[1][1];
	mat[ 6] = g.cameraMatrix[1][2];
	mat[ 8] = g.cameraMatrix[2][0];
	mat[ 9] = g.cameraMatrix[2][1];
	mat[10] = g.cameraMatrix[2][2];
	mat[ 3] = mat[ 7] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0f;
	mat[15] = 1.0f;
	glMultMatrixf(mat);
	//
	///// End Billboarding

	float tx = g.bullets[i].spriteFrame * .20;

	glRotatef(90, 1, 0, 0);
	glBegin(GL_QUADS);

	glTexCoord2f(tx, 0.0f);
	glVertex3f( w, h,-d);

	glTexCoord2f(tx+.20, 0.0f);
	glVertex3f(-w, h,-d);

	glTexCoord2f(tx+.20, 1.0f);
	glVertex3f(-w, h, d);

	glTexCoord2f(tx, 1.0f);
	glVertex3f( w, h, d);

	glEnd();
	glPopMatrix();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}


void drawPortals()
{
    Flt w = 1.5;
    Flt d = 1.5;
    Flt h = 0.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    for (int i = 0; i < g.nportals; i++) {

	glPushMatrix();
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, portalSilhouette);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0f); //Alpha

	glTranslated(g.portals[i].pos[0], g.portals[i].pos[1]+.4, g.portals[i].pos[2]);
	///// Billboarding
	//Setup camera rotation matrix
	//
	Vec v;
	VecSub(g.portals[i].pos, g.player.pos, v);
	Vec z = {0.0f, 0.0f, 0.0f};
	make_view_matrix(z, v, g.cameraMatrix);
	//
	//Billboard_to_camera();
	//
	float mat[16];
	mat[ 0] = g.cameraMatrix[0][0];
	mat[ 1] = g.cameraMatrix[0][1];
	mat[ 2] = g.cameraMatrix[0][2];
	mat[ 4] = g.cameraMatrix[1][0];
	mat[ 5] = g.cameraMatrix[1][1];
	mat[ 6] = g.cameraMatrix[1][2];
	mat[ 8] = g.cameraMatrix[2][0];
	mat[ 9] = g.cameraMatrix[2][1];
	mat[10] = g.cameraMatrix[2][2];
	mat[ 3] = mat[ 7] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0f;
	mat[15] = 1.0f;
	glMultMatrixf(mat);
	//
	///// End Billboarding

	float tx = g.portals[i].spriteFrame * .20;

	glRotatef(90, 1, 0, 0);
	glBegin(GL_QUADS);

	glTexCoord2f(tx, 0.0f);
	glVertex3f( w, h,-d);

	glTexCoord2f(tx+.20, 0.0f);
	glVertex3f(-w, h,-d);

	glTexCoord2f(tx+.20, 1.0f);
	glVertex3f(-w, h, d);

	glTexCoord2f(tx, 1.0f);
	glVertex3f( w, h, d);

	glEnd();
	glPopMatrix();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}

void drawGun() {

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, crosshairSilhouette);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f); //Alpha
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(g.xres/2-20, (g.yres/2));
    glTexCoord2f(0.0f, 0.0f); glVertex2i(g.xres/2-20, (g.yres/2+60)-100); 
    glTexCoord2f(1.0f, 0.0f); glVertex2i(g.xres/2+20, (g.yres/2+60)-100);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(g.xres/2+20, (g.yres/2));
    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);


    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, gunSilhouette);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f); //Alpha
    glBegin(GL_QUADS);
    if (g.shotReset == 0) {
	glTexCoord2f(0.0f, 1.0f); glVertex2i(g.xres/2-2.35*(g.xres/9), 0);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(g.xres/2-2.35*(g.xres/9), g.yres/2.0); 
	glTexCoord2f(0.5f, 0.0f); glVertex2i(g.xres/2+(g.xres/9),   g.yres/2.0);
	glTexCoord2f(0.5f, 1.0f); glVertex2i(g.xres/2+(g.xres/9),   0);
    } else {
	glTexCoord2f(0.5f, 1.0f); glVertex2i(g.xres/2-2.35*(g.xres/9), 0);
	glTexCoord2f(0.5f, 0.0f); glVertex2i(g.xres/2-2.35*(g.xres/9), g.yres/2.0); 
	glTexCoord2f(1.0f, 0.0f); glVertex2i(g.xres/2+(g.xres/9),   g.yres/2.0);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(g.xres/2+(g.xres/9),   0);
	g.shotReset -= 1;
    }
}

void vecNormalize(Vec v)
{
    Flt len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (len == 0.0)
	return;
    len = 1.0 / sqrt(len);
    v[0] *= len;
    v[1] *= len;
    v[2] *= len;
}

void shootFireball(Flt x, Flt y, Flt z) {

    Flt speed = .25;
    //Flt speed = .125;

    // Camera center - brute center
    Vec v;
    v[0] = g.player.pos[0] - x;
    v[1] = g.player.pos[1] - y;
    v[2] = g.player.pos[2] - z;

    // Normalize vector
    Vec norm = {v[0], v[1], v[2]};
    vecNormalize(norm);

    MakeVector(x, y, z, g.fireballs[g.nfireballs].pos);
    MakeVector(norm[0]*speed, norm[1]*speed, norm[2]*speed, g.fireballs[g.nfireballs].dir);
    g.nfireballs++;
}

void shootBullet() {

    Flt speed = .90;

    // Camera center - brute center
    Vec v;
    v[0] = g.player.pos[0] - g.player.view[0];
    v[1] = g.player.pos[1] - g.player.view[1];
    v[2] = g.player.pos[2] - g.player.view[2];

    // Normalize vector
    Vec norm = {v[0], v[1], v[2]};
    vecNormalize(norm);

    MakeVector(g.player.pos[0], g.player.pos[1]+.4, g.player.pos[2], g.bullets[g.nbullets].pos);
    MakeVector(-norm[0]*speed, -norm[1]*speed, -norm[2]*speed, g.bullets[g.nbullets].dir);
    g.nbullets++;
}

void makeExplosion(Flt x, Flt y, Flt z) {

    printf("Added explosion at %f, %f, %f\n", x, y, z);
    g.explosions[g.nexplosions].timer.recordTime(&g.explosions[g.nexplosions].timer.timeStart);
    MakeVector(x, y, z, g.explosions[g.nexplosions].pos);
    g.nexplosions++;
}


void physics()
{
    // Player movement
    if (g.key_states & g.w_mask) {
	if (g.player.pos[2] > -37.4f) {
	    g.player.moveForward();
	}
    }
    if (g.key_states & g.a_mask) {
	if (g.player.pos[0] > -22.4f) {
	    g.player.moveLeft();
	}
    }
    if (g.key_states & g.s_mask) {
	if (g.player.pos[2] < 7.4f) {
	    g.player.moveBackward();
	}
    }
    if (g.key_states & g.d_mask) {
	if (g.player.pos[0] < 22.4f) {
	    g.player.moveRight();
	}
    }

    // Brute physics
    for (int i = 0; i < g.nbrutes; i++) {

	// Sprite Animation
	g.brutes[i].timer.recordTime(&g.brutes[i].timer.timeCurrent);
	double timeSpan = g.brutes[i].timer.timeDiff(&g.brutes[i].timer.animTime, &g.brutes[i].timer.timeCurrent);
	if (timeSpan > g.brutes[i].delay) {
	    ++g.brutes[i].spriteFrame;
	    if(g.brutes[i].spriteFrame >= g.brutes[i].FRAMECOUNT)
		g.brutes[i].spriteFrame = 0;
	    g.brutes[i].timer.recordTime(&g.brutes[i].timer.animTime);
	}

	// Camera center - brute center
	Vec v;
	v[0] = g.player.pos[0] - g.brutes[i].pos[0];
	v[1] = g.player.pos[1] - g.brutes[i].pos[1];
	v[2] = g.player.pos[2] - g.brutes[i].pos[2];

	// Normalize vector
	Vec norm = {v[0], v[1], v[2]};
	vecNormalize(norm);

	Flt len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	if (len > 1.5) {
	    g.brutes[i].vel[0] = norm[0]*.02;
	    g.brutes[i].vel[1] = norm[1]*.02;
	    g.brutes[i].vel[2] = norm[2]*.02;

	    g.brutes[i].pos[0] += g.brutes[i].vel[0];
	    g.brutes[i].pos[2] += g.brutes[i].vel[2];
	}

	// Left/Right wall collision
	if (g.brutes[i].pos[0] < -22.0f) {
	    g.brutes[i].pos[0] = -22.0f;
	    g.brutes[i].vel[0] = 0;
	} else if (g.brutes[i].pos[0] > 22.0f) {
	    g.brutes[i].pos[0] = 22;
	    g.brutes[i].vel[0] = 0;
	}

	// Back/Front wall collision
	if (g.brutes[i].pos[2] < -37.0f) {
	    g.brutes[i].pos[2] = -37.0f;
	    g.brutes[i].vel[2] = 0;
	} else if (g.brutes[i].pos[2] > 7.0f) {
	    g.brutes[i].pos[2] = 7.0f;
	    g.brutes[i].vel[2] = 0;
	}

	if (g.brutes[i].pos[0] < g.player.pos[0] + 2 && // right of player
		g.brutes[i].pos[0] > g.player.pos[0] - 2 && // left of player
		g.brutes[i].pos[2] < g.player.pos[2] + 2 && // front of player
		g.brutes[i].pos[2] > g.player.pos[2] - 2)   // back of player
	{
	    g.brutes[i].timer.recordTime(&g.brutes[i].timer.timeCurrent);
	    double timeSpan = g.brutes[i].timer.timeDiff(&g.brutes[i].lastHit, &g.brutes[i].timer.timeCurrent);
	    if (timeSpan > 0.5f) {
		g.brutes[i].timer.timeCopy(&g.brutes[i].lastHit, &g.brutes[i].timer.timeCurrent);
		g.player.health -= 5;
	    }
	}


    }

    // Flier physics
    for (int i = 0; i < g.nfliers; i++) {

	// Sprite Animation
	g.fliers[i].timer.recordTime(&g.fliers[i].timer.timeCurrent);
	double timeSpan = g.fliers[i].timer.timeDiff(&g.fliers[i].lastShot, &g.fliers[i].timer.timeCurrent);
	if (timeSpan > 2.5 + g.fliers[i].nextShotDelay) {
	    shootFireball(g.fliers[i].pos[0], g.fliers[i].pos[1], g.fliers[i].pos[2]);
	    g.fliers[i].timer.recordTime(&g.fliers[i].lastShot);
	    g.fliers[i].nextShotDelay = rnd() * 5;
	}
	timeSpan = g.fliers[i].timer.timeDiff(&g.fliers[i].timer.animTime, &g.fliers[i].timer.timeCurrent);
	if (timeSpan > g.fliers[i].delay) {
	    ++g.fliers[i].spriteFrame;
	    if(g.fliers[i].spriteFrame >= g.fliers[i].FRAMECOUNT)
		g.fliers[i].spriteFrame = 0;
	    g.fliers[i].timer.recordTime(&g.fliers[i].timer.animTime);
	}

	// Camera center - fliers center
	Vec v;
	v[0] = g.player.pos[0] - g.fliers[i].pos[0];
	v[1] = g.player.pos[1] - g.fliers[i].pos[1];
	v[2] = g.player.pos[2] - g.fliers[i].pos[2];

	// Normalize vector
	Vec norm = {v[0], v[1], v[2]};
	vecNormalize(norm);

	g.fliers[i].vel[0] = norm[0]*.04;
	g.fliers[i].vel[1] = norm[1]*.04;
	g.fliers[i].vel[2] = norm[2]*.04;

	Flt len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	if (len > 10) {
	    g.fliers[i].pos[0] += g.fliers[i].vel[0];
	    g.fliers[i].pos[2] += g.fliers[i].vel[2];
	} else {
	    g.fliers[i].pos[0] -= g.fliers[i].vel[0];
	    g.fliers[i].pos[2] -= g.fliers[i].vel[2];
	}

	// Left/Right wall collision
	if (g.fliers[i].pos[0] < -22.0f) {
	    g.fliers[i].pos[0] = -22.0f;
	    g.fliers[i].vel[0] = 0;
	} else if (g.fliers[i].pos[0] > 22.0f) {
	    g.fliers[i].pos[0] = 22;
	    g.fliers[i].vel[0] = 0;
	}

	// Back/Front wall collision
	if (g.fliers[i].pos[2] < -37.0f) {
	    g.fliers[i].pos[2] = -37.0f;
	    g.fliers[i].vel[2] = 0;
	} else if (g.fliers[i].pos[2] > 7.0f) {
	    g.fliers[i].pos[2] = 7.0f;
	    g.fliers[i].vel[2] = 0;
	}
    }

    // Fireball physics
    for (int i = 0; i < g.nfireballs; i++) {

	// Constant direction
	g.fireballs[i].pos[0] += g.fireballs[i].dir[0];
	g.fireballs[i].pos[1] += g.fireballs[i].dir[1];
	g.fireballs[i].pos[2] += g.fireballs[i].dir[2];

	// collision with walls
	if (g.fireballs[i].pos[0] < -22.4f || //
		g.fireballs[i].pos[0] > 22.4f  || //
		g.fireballs[i].pos[1] < 0	   || // floor
		g.fireballs[i].pos[2] < -37.4f || //
		g.fireballs[i].pos[2] > 7.4f)     //
	{

	    // Copy last fireball to current possition and decrement nfireballs
	    if (g.nfireballs > 1) {
		g.fireballs[i].pos[0] = g.fireballs[g.nfireballs-1].pos[0];
		g.fireballs[i].pos[1] = g.fireballs[g.nfireballs-1].pos[1];
		g.fireballs[i].pos[2] = g.fireballs[g.nfireballs-1].pos[2];
		g.fireballs[i].dir[0] = g.fireballs[g.nfireballs-1].dir[0];
		g.fireballs[i].dir[1] = g.fireballs[g.nfireballs-1].dir[1];
		g.fireballs[i].dir[2] = g.fireballs[g.nfireballs-1].dir[2];
	    }
	    --g.nfireballs;

	    // Move change current fireball if there are still fireballs
	    if (g.nfireballs > 0) {
		g.fireballs[i].pos[0] += g.fireballs[i].dir[0];
		g.fireballs[i].pos[1] += g.fireballs[i].dir[1];
		g.fireballs[i].pos[2] += g.fireballs[i].dir[2];
	    }
	}

	// collision with player
	if (g.fireballs[i].pos[0] < g.player.pos[0] + 1 && // right of player
		g.fireballs[i].pos[0] > g.player.pos[0] - 1 && // left of player
		g.fireballs[i].pos[1] < g.player.pos[1] + 3 && // top of player
		g.fireballs[i].pos[1] > g.player.pos[1] - 3 && // bottom of player
		g.fireballs[i].pos[2] < g.player.pos[2] + 1 && // front of player
		g.fireballs[i].pos[2] > g.player.pos[2] - 1)   // back of player
	{
	    g.player.health -= 10;

	    // Copy last fireball to current possition and decrement nfireballs
	    if (g.nfireballs > 1) {
		g.fireballs[i].pos[0] = g.fireballs[g.nfireballs-1].pos[0];
		g.fireballs[i].pos[1] = g.fireballs[g.nfireballs-1].pos[1];
		g.fireballs[i].pos[2] = g.fireballs[g.nfireballs-1].pos[2];
		g.fireballs[i].dir[0] = g.fireballs[g.nfireballs-1].dir[0];
		g.fireballs[i].dir[1] = g.fireballs[g.nfireballs-1].dir[1];
		g.fireballs[i].dir[2] = g.fireballs[g.nfireballs-1].dir[2];
	    }
	    --g.nfireballs;

	    // Move change current fireball if there are still fireballs
	    if (g.nfireballs > 0) {
		g.fireballs[i].pos[0] += g.fireballs[i].dir[0];
		g.fireballs[i].pos[1] += g.fireballs[i].dir[1];
		g.fireballs[i].pos[2] += g.fireballs[i].dir[2];
	    }
	}

	if (g.nfireballs > 0) {

	    // Sprite Animation
	    g.fireballs[i].timer.recordTime(&g.fireballs[i].timer.timeCurrent);
	    double timeSpan = g.fireballs[i].timer.timeDiff(&g.fireballs[i].timer.animTime, &g.fireballs[i].timer.timeCurrent);
	    if (timeSpan > g.fireballs[i].delay) {
		++g.fireballs[i].spriteFrame;
		if(g.fireballs[i].spriteFrame >= g.fireballs[i].FRAMECOUNT)
		    g.fireballs[i].spriteFrame = 0;
		g.fireballs[i].timer.recordTime(&g.fireballs[i].timer.animTime);
	    }
	}

    }

    // Bullet physics
    for (int i = 0; i < g.nbullets; i++) {

	// Constant direction
	g.bullets[i].pos[0] += g.bullets[i].dir[0];
	g.bullets[i].pos[1] += g.bullets[i].dir[1];
	g.bullets[i].pos[2] += g.bullets[i].dir[2];

	// collision with walls
	if (g.bullets[i].pos[0] < -22.4f || //
		g.bullets[i].pos[0] > 22.4f  || //
		g.bullets[i].pos[1] < 0	   || // floor
		g.bullets[i].pos[2] < -37.4f || //
		g.bullets[i].pos[2] > 7.4f)     //
	{
	    makeExplosion(g.bullets[i].pos[0], g.bullets[i].pos[1], g.bullets[i].pos[2]);

	    // Copy last fireball to current possition and decrement nbullets
	    if (g.nbullets > 1) {
		g.bullets[i].pos[0] = g.bullets[g.nbullets-1].pos[0];
		g.bullets[i].pos[1] = g.bullets[g.nbullets-1].pos[1];
		g.bullets[i].pos[2] = g.bullets[g.nbullets-1].pos[2];
		g.bullets[i].dir[0] = g.bullets[g.nbullets-1].dir[0];
		g.bullets[i].dir[1] = g.bullets[g.nbullets-1].dir[1];
		g.bullets[i].dir[2] = g.bullets[g.nbullets-1].dir[2];
	    }
	    --g.nbullets;

	    // Move change current fireball if there are still bullets
	    if (g.nbullets > 0) {
		g.bullets[i].pos[0] += g.bullets[i].dir[0];
		g.bullets[i].pos[1] += g.bullets[i].dir[1];
		g.bullets[i].pos[2] += g.bullets[i].dir[2];
	    }
	} else {

	    for (int j = 0; j < g.nbrutes; j++) {
		if (g.bullets[i].pos[0] < g.brutes[j].pos[0] + 1 && // right of player
			g.bullets[i].pos[0] > g.brutes[j].pos[0] - 1 && // left of player
			g.bullets[i].pos[1] < g.brutes[j].pos[1] + 2 && // top of player
			g.bullets[i].pos[1] > g.brutes[j].pos[1] - 2 && // bottom of player
			g.bullets[i].pos[2] < g.brutes[j].pos[2] + 1 && // front of player
			g.bullets[i].pos[2] > g.brutes[j].pos[2] - 1)   // back of player
		{
		    makeExplosion(g.bullets[i].pos[0], g.bullets[i].pos[1], g.bullets[i].pos[2]);
		    g.brutes[j].health -= 50;
		    //printf("Brute #%d has %f health left\n", j, g.brutes[j].health);

		    // Copy last fireball to current possition and decrement nbrutes
		    if (g.brutes[j].health < 1) {
			g.player.score += 1;
			if (g.nbrutes > 1) {
			    g.brutes[j].pos[0] = g.brutes[g.nbrutes-1].pos[0];
			    g.brutes[j].pos[1] = g.brutes[g.nbrutes-1].pos[1];
			    g.brutes[j].pos[2] = g.brutes[g.nbrutes-1].pos[2];
			    g.brutes[j].vel[0] = g.brutes[g.nbrutes-1].vel[0];
			    g.brutes[j].vel[1] = g.brutes[g.nbrutes-1].vel[1];
			    g.brutes[j].vel[2] = g.brutes[g.nbrutes-1].vel[2];
			    g.brutes[j].health = g.brutes[g.nbrutes-1].health;
			}
			--g.nbrutes;

			// Move change current fireball if there are still brutes
			if (g.nbrutes > 0) {
			    g.brutes[j].pos[0] += g.brutes[j].vel[0];
			    g.brutes[j].pos[1] += g.brutes[j].vel[1];
			    g.brutes[j].pos[2] += g.brutes[j].vel[2];
			}
		    }

		    // Copy last fireball to current possition and decrement nbullets
		    if (g.nbullets > 1) {
			g.bullets[i].pos[0] = g.bullets[g.nbullets-1].pos[0];
			g.bullets[i].pos[1] = g.bullets[g.nbullets-1].pos[1];
			g.bullets[i].pos[2] = g.bullets[g.nbullets-1].pos[2];
			g.bullets[i].dir[0] = g.bullets[g.nbullets-1].dir[0];
			g.bullets[i].dir[1] = g.bullets[g.nbullets-1].dir[1];
			g.bullets[i].dir[2] = g.bullets[g.nbullets-1].dir[2];
		    }
		    --g.nbullets;

		    // Move change current fireball if there are still bullets
		    if (g.nbullets > 0) {
			g.bullets[i].pos[0] += g.bullets[i].dir[0];
			g.bullets[i].pos[1] += g.bullets[i].dir[1];
			g.bullets[i].pos[2] += g.bullets[i].dir[2];
		    }
		}
	    }

	    // collision with fliers 
	    for (int j = 0; j < g.nfliers; j++) {
		if (g.bullets[i].pos[0] < g.fliers[j].pos[0] + .5 && // right of player
			g.bullets[i].pos[0] > g.fliers[j].pos[0] - .5 && // left of player
			g.bullets[i].pos[1] < g.fliers[j].pos[1] + .5 && // top of player
			g.bullets[i].pos[1] > g.fliers[j].pos[1] - .5 && // bottom of player
			g.bullets[i].pos[2] < g.fliers[j].pos[2] + .5 && // front of player
			g.bullets[i].pos[2] > g.fliers[j].pos[2] - .5)   // back of player
		{
		    makeExplosion(g.bullets[i].pos[0], g.bullets[i].pos[1], g.bullets[i].pos[2]);
		    g.fliers[j].health -= 50;
		    //printf("Flier #%d has %f health left\n", j, g.fliers[j].health);

		    // Copy last fireball to current possition and decrement nbrutes
		    if (g.fliers[j].health < 1) {
			g.player.score += 1;
			if (g.nfliers > 1) {
			    g.fliers[j].pos[0] = g.fliers[g.nfliers-1].pos[0];
			    g.fliers[j].pos[1] = g.fliers[g.nfliers-1].pos[1];
			    g.fliers[j].pos[2] = g.fliers[g.nfliers-1].pos[2];
			    g.fliers[j].vel[0] = g.fliers[g.nfliers-1].vel[0];
			    g.fliers[j].vel[1] = g.fliers[g.nfliers-1].vel[1];
			    g.fliers[j].vel[2] = g.fliers[g.nfliers-1].vel[2];
			    g.fliers[j].health = g.fliers[g.nfliers-1].health;
			}
			--g.nfliers;

			// Move change current fireball if there are still fliers
			if (g.nfliers > 0) {
			    g.fliers[j].pos[0] += g.fliers[j].vel[0];
			    g.fliers[j].pos[1] += g.fliers[j].vel[1];
			    g.fliers[j].pos[2] += g.fliers[j].vel[2];
			}
		    }



		    // Copy last fireball to current possition and decrement nbullets
		    if (g.nbullets > 1) {
			g.bullets[i].pos[0] = g.bullets[g.nbullets-1].pos[0];
			g.bullets[i].pos[1] = g.bullets[g.nbullets-1].pos[1];
			g.bullets[i].pos[2] = g.bullets[g.nbullets-1].pos[2];
			g.bullets[i].dir[0] = g.bullets[g.nbullets-1].dir[0];
			g.bullets[i].dir[1] = g.bullets[g.nbullets-1].dir[1];
			g.bullets[i].dir[2] = g.bullets[g.nbullets-1].dir[2];
		    }
		    --g.nbullets;

		    // Move change current fireball if there are still bullets
		    if (g.nbullets > 0) {
			g.bullets[i].pos[0] += g.bullets[i].dir[0];
			g.bullets[i].pos[1] += g.bullets[i].dir[1];
			g.bullets[i].pos[2] += g.bullets[i].dir[2];
		    }
		}
	    }
	    if (g.nbullets > 0) {

		// Sprite Animation
		g.bullets[i].timer.recordTime(&g.bullets[i].timer.timeCurrent);
		double timeSpan = g.bullets[i].timer.timeDiff(&g.bullets[i].timer.animTime, &g.bullets[i].timer.timeCurrent);
		if (timeSpan > g.bullets[i].delay) {
		    ++g.bullets[i].spriteFrame;
		    if(g.bullets[i].spriteFrame >= g.bullets[i].FRAMECOUNT)
			g.bullets[i].spriteFrame = 0;
		    g.bullets[i].timer.recordTime(&g.bullets[i].timer.animTime);
		}
	    }
	}

    }


    // Portal physics
    for (int i = 0; i < g.nportals; i++) {

	// Sprite Animation
	g.portals[i].timer.recordTime(&g.portals[i].timer.timeCurrent);
	double timeSpan = g.portals[i].timer.timeDiff(&g.portals[i].timer.animTime, &g.portals[i].timer.timeCurrent);
	if (timeSpan > g.portals[i].delay) {
	    ++g.portals[i].spriteFrame;
	    if(g.portals[i].spriteFrame >= g.portals[i].FRAMECOUNT)
		g.portals[i].spriteFrame = 0;
	    g.portals[i].timer.recordTime(&g.portals[i].timer.animTime);
	}
    }
}

void init_image(char * imagePath, Ppmimage * image, GLuint * texture)
{
    image = ppm6GetImage(imagePath);
    glGenTextures(1, texture);

    // Image
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
	    image->width, image->height,
	    0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
}

unsigned char *buildAlphaData(Ppmimage *img) {
    int i, a, b, c;
    unsigned char *newdata, *ptr;
    unsigned char *data = (unsigned char *) img->data;
    newdata = (unsigned char *)malloc(img->width * img->height * 4);
    ptr = newdata;
    for (i = 0; i<img->width * img->height * 3; i+=3) {
	a = *(data+0);
	b = *(data+1);
	c = *(data+2);
	*(ptr+0) = a;
	*(ptr+1) = b;
	*(ptr+2) = c;
	*(ptr+3) = (a|b|c);
	ptr += 4;
	data += 3;
    }
    return newdata;
}

void init_alpha_image(char * imagePath, Ppmimage * image,
	GLuint * texture, GLuint * silhouette)
{
    image = ppm6GetImage(imagePath);
    glGenTextures(1, silhouette);
    glGenTextures(1, texture);

    // Image
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
	    image->width, image->height,
	    0, GL_RGB, GL_UNSIGNED_BYTE, image->data);

    // Alpha
    glBindTexture(GL_TEXTURE_2D,*silhouette);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

    unsigned char *silhouetteData = buildAlphaData(image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	    image->width, image->height,
	    0, GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
    free(silhouetteData);
}


void render()
{
    //Clear the depth buffer and screen.
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    //
    //Render a 3D scene
    //
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0f, g.aspectRatio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();



    // Camera that can move on x&z and look on x,y,&z
    gluLookAt(g.player.pos[0] , 0.0f , g.player.pos[2],
	    g.player.view[0], g.player.view[1], g.player.view[2],
	    g.player.upv[0],  g.player.upv[1], g.player.upv[2]);

    //
    drawFloor();
    drawWall();
    drawPortals();
    drawBrutes();
    drawFliers();
    drawFireballs();
    // Show bullet trajectory for debugging
    drawBullets();
    drawExplosions();
    //
    //switch to 2D mode
    //
    Rect r;
    glViewport(0, 0, g.xres, g.yres);
    glMatrixMode(GL_MODELVIEW);   glLoadIdentity();
    glMatrixMode (GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, g.xres, 0, g.yres);
    glDisable(GL_LIGHTING);
    drawGun();
    r.center = 0;

    // FPS
    r.bot = g.yres-5;
    r.left = g.xres-50;
    ggprint8b(&r, 16, 0x00887766, "");
    ggprint8b(&r, 16, 0x00887766, "FPS: %d", g.fps);

    // Health/score
    r.bot = g.yres - 35;
    r.left = 10;
    if (g.player.health > 1) {
	ggprint16(&r, 32, 0x00887766, "Health: %d", (int)g.player.health);
	ggprint16(&r, 32, 0x00887766, "Score: %d", g.player.score);
    } else {
	r.bot = g.yres/2+100;
	r.left = g.xres/2-95;
	ggprint40(&r, 40, 0x00887766, "Game Over");
	r.left = g.xres/2-70;
	ggprint40(&r, 40, 0x00887766, "Score: %d", g.player.score);
	ggprint16(&r, 32, 0x00887766, "Pres ESC to exit");
    }

    //    ggprint8b(&r, 16, 0x00887766, "Camera Info:");
    //    ggprint8b(&r, 16, 0x00887766, "    Position: [%.2f, %.2f, %.2f]", g.player.pos[0], g.player.pos[1], g.player.pos[2]);
    //    ggprint8b(&r, 16, 0x00887766, "    Direction: [%.2f, %.2f, %.2f]", g.player.view[0], g.player.view[1], g.player.view[2]);
    //    ggprint8b(&r, 16, 0x00887766, "    Velocity: [%.2f, %.2f]", g.player.vel[0], g.player.vel[2]);
    //
    //    ggprint8b(&r, 16, 0x00887766, "Controls:");
    //    ggprint8b(&r, 16, 0x00887766, "    Mouse: Look Around");
    //    ggprint8b(&r, 16, 0x00887766, "    w/s: Move Forward/Backward");
    //    ggprint8b(&r, 16, 0x00887766, "    a/d: Look Left/Right");
    //    ggprint8b(&r, 16, 0x00887766, "    e/q: Strafe Left/Right");
    //    ggprint8b(&r, 16, 0x00887766, "    c/z: Strafe Up/Down");
}

void unitTest () {

    Vec test1Vec;
    MakeVector(3.0, 1.0, 2.0, test1Vec);
    vecNormalize(test1Vec);
    printf("Expected:\n\tX: 0.802000, Y: 0.267000, Z: 0.535000\n");
    printf("Got:\n\tX: %f, Y: %f, Z: %f\n\n",  round( test1Vec[0] * 1000.0 ) / 1000.0,
	    round( test1Vec[1] * 1000.0 ) / 1000.0,
	    round( test1Vec[2] * 1000.0 ) / 1000.0);
}


