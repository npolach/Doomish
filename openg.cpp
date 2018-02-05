//
//modified by: Nick Polach
//date: 01/23/18
//
//program: openg.cpp
//author:  Gordon Griesel
//date:    Spring 2018
//
//An OpenGL 3D framework for students.
//
//Goal:
//     1. setup the keyboard input to move the camera location
//        and camera viewing direction.
//     2. move the camera down and to the left to line-up with the hole
//     3. move the camera smoothly through the hole in the wall
//
//
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "fonts.h"
#include <iostream>

typedef float Flt;
typedef Flt Vec[3];
typedef Flt	Matrix[4][4];
//some macros
#define MakeVector(x, y, z, v) (v)[0]=(x);(v)[1]=(y);(v)[2]=(z)
#define rnd() (Flt)rand() / (Flt)RAND_MAX
//some constants
const Flt PI = 3.141592653589793;

class Global {
    public:
	int xres, yres;
	Flt aspectRatio;
	Flt boxRotate;
	//Vec camera;
	Flt angleH;
	Flt angleV;
	// Needed to move camera
	Flt lx,ly,lz;
	Flt x,y,z;
	int savex, savey;

	GLfloat lightPosition[4];
	Global() {
	    //constructor
	    xres = 640;
	    yres = 480;
	    aspectRatio = (GLfloat)xres / (GLfloat)yres;
	    boxRotate = 0.0;
	    //MakeVector(6.0, 5.3, 8.0, camera);
	    angleH = -0.55f;
	    angleV = -0.55f;
	    lx = sin(angleH);
	    ly = sin(angleV);
	    lz = -cos(angleH);
	    x = 6.0;
	    y = 5.3;
	    z = 8.0;
	    savex = 0;
	    savey = 0;

	    //light is up high, right a little, toward a little
	    MakeVector(100.0f, 240.0f, 40.0f, lightPosition);
	    lightPosition[3] = 1.0f;
	}
} g;

//X Windows wrapper class
class X11_wrapper {
    private:
	Display *dpy;
	Window win;
	GLXContext glc;
    public:
	X11_wrapper() {
	    //Look here for information on XVisualInfo parameters.
	    //http://www.talisman.org/opengl-1.1/Reference/glXChooseVisual.html
	    //
	    GLint att[] = { GLX_RGBA,
		GLX_STENCIL_SIZE, 2,
		GLX_DEPTH_SIZE, 24,
		GLX_DOUBLEBUFFER, None };
	    XSetWindowAttributes swa;
	    setup_screen_res(g.xres, g.yres);
	    dpy = XOpenDisplay(NULL);
	    if (dpy == NULL) {
		printf("\ncannot connect to X server\n\n");
		exit(EXIT_FAILURE);
	    }
	    Window root = DefaultRootWindow(dpy);
	    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	    if (vi == NULL) {
		printf("\nno appropriate visual found\n\n");
		exit(EXIT_FAILURE);
	    }
	    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	    swa.colormap = cmap;
	    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		StructureNotifyMask | SubstructureNotifyMask | PointerMotionMask;
	    win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
		    vi->depth, InputOutput, vi->visual,
		    CWColormap | CWEventMask, &swa);
	    set_title("4490 OpenGL Lab-1");
	    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	    glXMakeCurrent(dpy, win, glc);
	}
	~X11_wrapper() {
	    XDestroyWindow(dpy, win);
	    XCloseDisplay(dpy);
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

} x11;

void init_opengl();
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

int main()
{
    init_opengl();
    int done=0;
    while (!done) {
	while (x11.getXPending()) {
	    XEvent e = x11.getXNextEvent();
	    x11.check_resize(&e);
	    check_mouse(&e);
	    done = check_keys(&e);
	}
	physics();
	render();
	x11.swapBuffers();
    }
    cleanup_fonts();
    return 0;
}

void init()
{

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
    //Turn on a light
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    glEnable(GL_LIGHT0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
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
}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?
    //
    //static int savex = 0;
    //static int savey = 0;
    //static int ct=0;
    //	if (e->type == ButtonRelease) {
    //		return;
    //	}
    //	if (e->type == ButtonPress) {
    //		if (e->xbutton.button==1) {
    //			//Left button is down
    //		}
    //		if (e->xbutton.button==3) {
    //			//Right button is down
    //		}
    //	}
    if (g.savex != e->xbutton.x || g.savey != e->xbutton.y) {
	//Mouse moved
	int xdiff = g.savex - e->xbutton.x;
	int ydiff = g.savey - e->xbutton.y;
	//if (++ct < 10)
	//	return;
	if (xdiff < 0) {
	    g.angleH += 0.015f;
	    g.lx = sin(g.angleH);
	    g.lz = -cos(g.angleH);
	    //if (g.ship.angle >= 360.0f)
	    //    g.ship.angle -= 360.0f;
	}
	else if (xdiff > 0) {
	    g.angleH -= 0.015f;
	    g.lx = sin(g.angleH);
	    g.lz = -cos(g.angleH);
	    //if (g.ship.angle < 0.0f)
	    //    g.ship.angle += 360.0f;
	}
	if (ydiff < 0) {
	    g.angleV -= 0.015f;
		if (g.angleV < -1.0)
			g.angleV = -1.0;

	    g.ly = sin(g.angleV);

	}
	if (ydiff > 0) {
	    g.angleV += 0.015f;
		if (g.angleV > 1.0)
			g.angleV = 1.0;
	    g.ly = sin(g.angleV);

	}
    }
    //x11.set_mouse_position(100,100);
    g.savex = e->xbutton.x;
    g.savey = e->xbutton.y;
}

int check_keys(XEvent *e)
{
    static int shift = false;
    //Was there input from the keyboard?
    if (e->type != KeyPress && e->type != KeyRelease)
	return 0;

    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyRelease) {
	if (key == XK_Shift_L || key == XK_Shift_R) {
	    shift = false;
	    return 0;
	}
    }

    if (e->type == KeyPress) {
	if (key == XK_Shift_L || key == XK_Shift_R) {
	    shift = true;
	    return 0;
	}
	//Was there input from the keyboard?
	switch(key) {
	    // Camera angle
	    case XK_1:
		break;
	    case XK_w:
		if (shift) {
		    g.x += g.lx * 0.20f;
		    g.y += g.ly * 0.20f;
		    g.z += g.lz * 0.20f;
		} else {
		    g.angleV += 0.020f;
			if (g.angleV > 1.0)
				g.angleV = 1.0;

		    g.ly = sin(g.angleV);
		}
		break;
	    case XK_a:
		g.angleH -= 0.020f;
		g.lx = sin(g.angleH);
		g.lz = -cos(g.angleH);
		break;
	    case XK_s:
		if (shift) {
		    g.x -= g.lx * 0.20f;
		    g.y -= g.ly * 0.20f;
		    g.z -= g.lz * 0.20f;
		} else {
		    g.angleV -= 0.020f;
			if (g.angleV < -1.0)
				g.angleV = -1.0;

		    g.ly = sin(g.angleV);
		}
		break;
	    case XK_d:
		g.angleH += 0.020f;
		g.lx = sin(g.angleH);
		g.lz = -cos(g.angleH);
		break;

	    case XK_e:
		g.x += 0.02f;
		break;
	    case XK_q:
		g.x -= 0.02f;
		break;
	    case XK_c:
		g.y += 0.02f;
		break;
	    case XK_z:
		g.y -= 0.02f;
		break;

	    case XK_Escape:
		return 1;
	}
    }
    return 0;
}

void box(float w1, float h1, float d1)
{
    //
    //   1================2
    //   |\              /|
    //   | \            / |
    //   |  \          /  |
    //   |   \        /   |
    //   |    5------6    |
    //   |    |      |    |
    //   |    |      |    |
    //   |    |      |    |
    //   |    4------7    |
    //   |   /        \   |
    //   |  /          \  |
    //   | /            \ |
    //   |/              \|
    //   0================3
    //
    const Flt vert[][3] = {
	-1.0, -1.0,  1.0,
	-1.0,  1.0,  1.0,
	1.0,  1.0,  1.0,
	1.0, -1.0,  1.0,
	-1.0, -1.0, -1.0,
	-1.0,  1.0, -1.0,
	1.0,  1.0, -1.0,
	1.0, -1.0, -1.0 };
    //left,top,right,bottom,front,back.
    const int face[][4] = {
	0,1,5,4,
	1,2,6,5,
	2,3,7,6,
	0,4,7,3,
	2,1,0,3,
	4,5,6,7 };
    const Flt norm[][3] = {
	-1.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	1.0, 0.0, 0.0,
	0.0,-1.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 0.0,-1.0 };
    //half the width from center.
    Flt w = w1 * 0.5;
    Flt d = d1 * 0.5;
    Flt h = h1 * 0.5;
    //Normals are required for any lighting.
    glBegin(GL_QUADS);
    for (int i=0; i<6; i++) {
	glNormal3fv(norm[i]);
	for (int j=0; j<4; j++) {
	    int k = face[i][j];
	    glVertex3f(vert[k][0]*w, vert[k][1]*h, vert[k][2]*d);
	}
    }
    glEnd();
}

void drawFloor()
{
    glColor3f(0.6f, 0.6f, 0.6f);
    Flt w = 5.0 * 0.5;
    Flt d = 5.0 * 0.5;
    Flt h = -1.0;
    glBegin(GL_QUADS);
    //top
    glNormal3f( 0.0f, 1.0f, 0.0f);
    glVertex3f( w, h,-d);
    glVertex3f(-w, h,-d);
    glVertex3f(-w, h, d);
    glVertex3f( w, h, d);
    glEnd();
}

void drawBox()
{
    glColor3f(0.2f, 0.7f, 1.0f);
    box(3.0, 0.2, 3.0);
}

void drawWall()
{
    //
    //   dimensions are 1.0 x 1.0
    //
    //   0--------------1--------2--------------3
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   4--------------5--------6--------------7
    //   |              |        |              |
    //   |              |  hole  |              |
    //   |              |        |              |
    //   |              |        |              |
    //   8--------------9--------10-------------11
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   |              |        |              |
    //   12-------------13-------14-------------15
    //
    //
    //draw a wall with a hole in it.
    float vert[16][3] = {
	-.5, .5, 0,
	-.1, .5, 0,
	.1, .5, 0,
	.5, .5, 0,
	-.5, .1, 0,
	-.1, .1, 0,
	.1, .1, 0,
	.5, .1, 0,
	-.5,-.1, 0,
	-.1,-.1, 0,
	.1,-.1, 0,
	.5,-.1, 0,
	-.5,-.5, 0,
	-.1,-.5, 0,
	.1,-.5, 0,
	.5,-.5, 0 };
    const int n = 21;
    int idx[n] = {0,4,1,5,2,6,3,7,6,11,10,15,14,10,13,9,12,8,9,4,5};
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslated(0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f( 0.0f, 0.0f, 1.0f);
    for (int i=0; i<n; i++)
	glVertex3fv(vert[idx[i]]);
    glEnd();
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslated(0.0, 1.0, -0.1);
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f( 0.0f, 0.0f, 1.0f);
    for (int i=0; i<n; i++)
	glVertex3fv(vert[idx[i]]);
    glEnd();
    glPopMatrix();



}

void physics()
{
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
    // Camera that can move
    gluLookAt(g.x, g.y, g.z,
	    g.x+g.lx, g.y+g.ly, g.z+g.lz,
	    0.0f, 1.0f, 0.0f);
    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    //
    drawFloor();
    drawBox();
    drawWall();
    //
    //
    //switch to 2D mode
    //
    Rect r;
    glViewport(0, 0, g.xres, g.yres);
    glMatrixMode(GL_MODELVIEW);   glLoadIdentity();
    glMatrixMode (GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, g.xres, 0, g.yres);
    glDisable(GL_LIGHTING);
    r.bot = g.yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint8b(&r, 16, 0x00887766, "4490 OpenGL");
    ggprint8b(&r, 16, 0x00887766, "Camera Info:");
    ggprint8b(&r, 16, 0x00887766, "    Position: [%.2f, %.2f, %.2f]", g.x, g.y, g.z);
    ggprint8b(&r, 16, 0x00887766, "    Direction: [%.2f, %.2f, %.2f]", g.lx, g.ly, g.lz);
    ggprint8b(&r, 16, 0x00887766, "Controls:");
    ggprint8b(&r, 16, 0x00887766, "    Mouse: Look Around");
    ggprint8b(&r, 16, 0x00887766, "    w/s: Look Up/Down");
    ggprint8b(&r, 16, 0x00887766, "    a/d: Look Left/Right");
    ggprint8b(&r, 16, 0x00887766, "    e/q: Strafe Left/Right");
    ggprint8b(&r, 16, 0x00887766, "    c/z: Strafe Up/Down");
    ggprint8b(&r, 16, 0x00887766, "    Shift + w/s: Move Forward/Backward");
}



