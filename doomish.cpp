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
#include "ppm.h"

typedef float Flt;
typedef Flt Vec[3];
typedef Flt	Matrix[4][4];
//some macros
#define MakeVector(x, y, z, v) (v)[0]=(x);(v)[1]=(y);(v)[2]=(z)
#define rnd() (Flt)rand() / (Flt)RAND_MAX
//some constants
const Flt PI = 3.141592653589793;


Ppmimage * impImage;
GLuint impTexture;
GLuint impSilhouette;

Ppmimage * floor1Image=NULL;
GLuint floor1Texture;

Ppmimage * wall1Image=NULL;
GLuint wall1Texture;

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
	Flt velx;
	Flt velz;
	unsigned char key_states;
	unsigned char w_mask;
	unsigned char a_mask;
	unsigned char s_mask;
	unsigned char d_mask;


	GLfloat lightPosition[4];
	Global() {
	    //constructor
	    xres = 1024; 
	    yres = 768;
	    aspectRatio = (GLfloat)xres / (GLfloat)yres;
	    boxRotate = 0.0;
	    //MakeVector(6.0, 5.3, 8.0, camera);
	    // Initial player position and view direction
	    angleH = 0.0;
	    angleV = 0.0;
	    lx = sin(angleH);
	    ly = sin(angleV);
	    lz = -cos(angleH);

	    x = 0.0;
	    y = 0.0;
	    z = 5.0;
	    velx = 0.0;
	    velz = 0.0;
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
	    show_mouse_cursor(0);
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
    remove("./images/imp.ppm");
    //
    //    //convert images to ppm
    system("mogrify -format ppm ./images/floor1.jpg");
    system("mogrify -format ppm ./images/wall1.jpg");
    system("mogrify -format ppm ./images/imp.jpg");
}

void imageClean()
{
    //clean up all images in master folder
    remove("./images/floor1.ppm");
    remove("./images/wall1.ppm");
    remove("./images/imp.ppm");
}

void init_opengl();
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

void init_image(char * imagePath, Ppmimage * image, GLuint * texture);
unsigned char *buildAlphaData(Ppmimage *img);
void init_alpha_image(char * imagePath, Ppmimage * image,
	GLuint * texture, GLuint * silhouette);

int main()
{
    imageConvert();
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
    imageClean();
    return 0;
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

    // Initialize rocket image
    init_alpha_image((char *)"./images/imp.ppm",
	    impImage, &impTexture, &impSilhouette);

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
}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?

    static int savex = 0;
    static int savey = 0;
    bool skip = false;
    static int ct=0;
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
    //if (savex != e->xbutton.x || savey != e->xbutton.y) {
    if ((savex != e->xbutton.x || savey != e->xbutton.y) && !skip) {
	//Mouse moved
	int xdiff = savex - e->xbutton.x;
	int ydiff = savey - e->xbutton.y;
	//if (++ct < 10)
	//	return;
	if (xdiff < 0) {
	    g.angleH += 0.015f;
	    g.lx = sin(g.angleH);
	    g.lz = -cos(g.angleH);

	}
	else if (xdiff > 0) {
	    g.angleH -= 0.015f;
	    g.lx = sin(g.angleH);
	    g.lz = -cos(g.angleH);

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
	x11.set_mouse_position(g.xres/2,g.yres/2);
	savex = e->xbutton.x;
	savey = e->xbutton.y;
	skip = !skip;
    }
}

int check_keys(XEvent *e)
{
    static int shift = false;
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

//int check_keys(XEvent *e)
//{
//    static int shift = false;
//    //Was there input from the keyboard?
//    if (e->type != KeyPress && e->type != KeyRelease)
//	return 0;
//
//    int key = XLookupKeysym(&e->xkey, 0);
//    if (e->type == KeyRelease) {
//	if (key == XK_Shift_L || key == XK_Shift_R) {
//	    shift = false;
//	    return 0;
//	}
//    }
//
//    if (e->type == KeyPress) {
//	if (key == XK_Shift_L || key == XK_Shift_R) {
//	    shift = true;
//	    return 0;
//	}
//	//Was there input from the keyboard?
//	switch(key) {
//	    // Camera angle
//	    case XK_1:
//		break;
//	    case XK_w:
//		//if (shift) {
//		g.velx += 0.02f;
//		if (g.velx > 0.08f)
//		    g.velx = 0.08f;
//		g.velz += 0.02f;
//		if (g.velz > 0.08f)
//		    g.velz = 0.08f;
//		break;
//	    case XK_a:
//		g.angleH -= 0.020f;
//		g.lx = sin(g.angleH);
//		g.lz = -cos(g.angleH);
//		break;
//	    case XK_s:
//		g.velx -= 0.02f;
//		if (g.velx < -0.08f)
//		    g.velx = -0.08f;
//		g.velz -= 0.02f;
//		if (g.velz < -0.08f)
//		    g.velz = -0.08f;
//		break;
//	    case XK_d:
//		g.angleH += 0.020f;
//		g.lx = sin(g.angleH);
//		g.lz = -cos(g.angleH);
//		break;
//
//	    case XK_e:
//		g.x += 0.02f;
//		break;
//	    case XK_q:
//		g.x -= 0.02f;
//		break;
//	    case XK_c:
//		g.y += 0.02f;
//		break;
//	    case XK_z:
//		g.y -= 0.02f;
//		break;
//
//	    case XK_Escape:
//		return 1;
//	}
//	}
//	return 0;
//    }

void drawFloor()
{
    Flt w = 2.5;
    Flt d = 2.5;
    Flt h = -1.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(0, 0, 0);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, floor1Texture);
    glBegin(GL_QUADS);


    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( w, h,-d);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-w, h,-d);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-w, h, d);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( w, h, d);


    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawWall()
{
    Flt w = 2.5;
    Flt d = 2.5;
    Flt h = -1.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(0, 0, -2);
    glRotatef(90, 1, 0, 0);
    //	glRotatef(90, 0, 0, 1);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, wall1Texture);
    glBegin(GL_QUADS);

    //	glTexCoord2f(0.0f, 1.0f);
    //	glVertex3f( w, h,-d);
    //
    //	glTexCoord2f(0.0f, 0.0f);
    //	glVertex3f(-w, h,-d);
    //
    //	glTexCoord2f(1.0f, 0.0f);
    //	glVertex3f(-w, h, d);
    //
    //	glTexCoord2f(1.0f, 1.0f);
    //	glVertex3f( w, h, d);

    //
    glTranslated(0, 1, 0);
    glRotatef(-90, 0, 0, 1);

    //glTexCoord2f(0.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( w, h,-d);

    //glTexCoord2f(0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-w, h,-d);

    //glTexCoord2f(1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-w, h, d);

    //glTexCoord2f(1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( w, h, d);

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawEnemy()
{
    Flt w = 0.5;
    Flt d = 0.5;
    Flt h = -1.0;

    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(0, -0.5, 0);
    glRotatef(90, 1, 0, 0);
    //glRotatef(270, 0, 1, 0);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, impSilhouette);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f); //Alpha
    glBegin(GL_QUADS);

    //glTexCoord2f(0.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( w, h,-d);

    //glTexCoord2f(0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-w, h,-d);

    //glTexCoord2f(1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-w, h, d);

    //glTexCoord2f(1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( w, h, d);

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}



//void drawBox()
//{
//    glColor3f(0.2f, 0.7f, 1.0f);
//    box(3.0, 0.2, 3.0);
//}

//void drawWall()
//{
//    //
//    //   dimensions are 1.0 x 1.0
//    //
//    //   0--------------1--------2--------------3
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   4--------------5--------6--------------7
//    //   |              |        |              |
//    //   |              |  hole  |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   8--------------9--------10-------------11
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   |              |        |              |
//    //   12-------------13-------14-------------15
//    //
//    //
//    //draw a wall with a hole in it.
//    float vert[16][3] = {
//	-.5, .5, 0,
//	-.1, .5, 0,
//	.1, .5, 0,
//	.5, .5, 0,
//	-.5, .1, 0,
//	-.1, .1, 0,
//	.1, .1, 0,
//	.5, .1, 0,
//	-.5,-.1, 0,
//	-.1,-.1, 0,
//	.1,-.1, 0,
//	.5,-.1, 0,
//	-.5,-.5, 0,
//	-.1,-.5, 0,
//	.1,-.5, 0,
//	.5,-.5, 0 };
//    const int n = 21;
//    int idx[n] = {0,4,1,5,2,6,3,7,6,11,10,15,14,10,13,9,12,8,9,4,5};
//    glColor3f(1.0f, 0.0f, 0.0f);
//    glPushMatrix();
//    glTranslated(0.0, 1.0, 0.0);
//    glBegin(GL_TRIANGLE_STRIP);
//    glNormal3f( 0.0f, 0.0f, 1.0f);
//    for (int i=0; i<n; i++)
//	glVertex3fv(vert[idx[i]]);
//    glEnd();
//    glPopMatrix();
//
//    glColor3f(1.0f, 1.0f, 0.0f);
//    glPushMatrix();
//    glTranslated(0.0, 1.0, -0.1);
//    glBegin(GL_TRIANGLE_STRIP);
//    glNormal3f( 0.0f, 0.0f, 1.0f);
//    for (int i=0; i<n; i++)
//	glVertex3fv(vert[idx[i]]);
//    glEnd();
//    glPopMatrix();
//}

void physics()
{

    if (g.key_states & g.w_mask) {
	g.velx += 0.02f;
	if (g.velx > 0.08f)
	    g.velx = 0.08f;
	g.velz += 0.02f;
	if (g.velz > 0.08f)
	    g.velz = 0.08f;
	g.x += g.lx * g.velx;
	g.z += g.lz * g.velz;
    }
    if (g.key_states & g.a_mask) {
	//		g.angleH -= 0.020f;
	//		g.lx = sin(g.angleH);
	//		g.lz = -cos(g.angleH);
	//g.x += (g.lx-1.0f) * g.velx;
	g.velx += 0.02f;
	if (g.velx > 0.08f)
	    g.velx = 0.08f;
	g.velz += 0.02f;
	if (g.velz > 0.08f)
	    g.velz = 0.08f;

	g.x += g.lx * g.velx;
	g.z += g.lz * g.velz;
    }
    if (g.key_states & g.s_mask) {
	g.velx -= 0.02f;
	if (g.velx < -0.08f)
	    g.velx = -0.08f;
	g.velz -= 0.02f;
	if (g.velz < -0.08f)
	    g.velz = -0.08f;
	g.x += g.lx * g.velx;
	g.z += g.lz * g.velz;

    }
    if (g.key_states & g.d_mask) {
	//		g.angleH += 0.020f;
	//		g.lx = sin(g.angleH);
	//		g.lz = -cos(g.angleH);
	//g.x += (g.lx+1.0f) * g.velx;
	g.velx -= 0.02f;
	if (g.velx < -0.08f)
	    g.velx = -0.08f;
	g.velz -= 0.02f;
	if (g.velz < -0.08f)
	    g.velz = -0.08f;

	g.x += g.lx * g.velx;
	g.z += g.lz * g.velz;
    }

    //	g.x += g.lx * g.velx;
    //	g.z += g.lz * g.velz;
    if (g.velx < 0.0f)
	g.velx += 0.004f;
    else if (g.velx > 0.0f)
	g.velx -= 0.004f;

    if (g.velz < 0.0f)
	g.velz += 0.004f;
    else if (g.velz > 0.0f)
	g.velz -= 0.004f;

    if (g.velx >= -0.005f && g.velx <= 0.005f)
	g.velx = 0.0f;

    if (g.velz >= -0.005f && g.velz <= 0.005f)
	g.velz = 0.0f;



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
    gluLookAt(g.x, 0.0f, g.z,
	    g.x+(g.lx), g.y+g.ly, g.z+g.lz,
	    0.0f, 1.0f, 0.0f);

    glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
    //
    drawFloor();
    drawWall();
    drawEnemy();
    //    drawBox();
    //    drawWall();
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
    ggprint8b(&r, 16, 0x00887766, "    Velocity: [%.2f, %.2f]", g.velx, g.velz);
    ggprint8b(&r, 16, 0x00887766, "Controls:");
    ggprint8b(&r, 16, 0x00887766, "    Mouse: Look Around");
    ggprint8b(&r, 16, 0x00887766, "    w/s: Move Forward/Backward");
    ggprint8b(&r, 16, 0x00887766, "    a/d: Look Left/Right");
    ggprint8b(&r, 16, 0x00887766, "    e/q: Strafe Left/Right");
    ggprint8b(&r, 16, 0x00887766, "    c/z: Strafe Up/Down");
}



