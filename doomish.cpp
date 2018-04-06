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
// From smoke lab
const int MAX_SMOKES = 400;


Ppmimage * impImage;
GLuint impTexture;
GLuint impSilhouette;

Ppmimage * flierImage;
GLuint flierTexture;
GLuint flierSilhouette;

Ppmimage * portalImage;
GLuint portalTexture;
GLuint portalSilhouette;

Ppmimage * floor1Image=NULL;
GLuint floor1Texture;

Ppmimage * wall1Image=NULL;
GLuint wall1Texture;

class Camera {
	public:
		Vec pos;
		Vec vel;
		Vec view;
		Flt angleH;
		Flt angleV;
		Vec upv;
		Flt moveSpeed;
		Flt strafeSpeed;

		Camera() {
			MakeVector(0.0, 0.0, 5.0, pos);
			MakeVector(0.0, 0.0, 0.0, view);
			MakeVector(0.0, 0.0, 0.0, vel);
			MakeVector(0.0, 1.0, 0.0, upv);
			angleH = 0.0f;
			angleV = 0.0f;
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
			angleV -= 0.02f;
			if (angleV < -1.0)
				angleV = -1.0;

			view[1] = sin(angleV);
		}

		void lookDown() 
		{
			angleV += 0.02f;
			if (angleV > 1.0)
				angleV = 1.0;

			view[1] = sin(angleV);
		}

		void lookLeft() 
		{
			angleH += 0.02f;
			view[0] = pos[0] + sin(angleH);
			view[2] = pos[2] + -cos(angleH);
		}

		void lookRight() 
		{
			angleH -= 0.02f;
			view[0] = pos[0] + sin(angleH);
			view[2] = pos[2] + -cos(angleH);
		}


};

class Brute {
	public:
		Vec pos;
		Vec vel;
};

class Flier {
	public:
		Vec pos;
		Vec vel;
};

// From smoke lab
class Smoke {
	public:
		Vec pos;
		Vec vert[16];
		Flt radius;
		Flt camDist;
		Flt groundDist;
		int n;
		struct timespec tstart;
		Flt maxtime;
		Flt alpha;
		bool separate;

		Smoke() { }
};

class Portal {
	public:
		Vec pos;
		struct timespec smokeStart, smokeTime;
		Smoke * smoke;
		int nsmokes;
		~Portal() {
			if (smoke)
				delete [] smoke;
		}

		Portal() {
			clock_gettime(CLOCK_REALTIME, &smokeStart);
			nsmokes = 0;
			smoke = new Smoke[MAX_SMOKES];
		}


};

//-----------------------------------------------------------------------------
////Setup timers
const double OOBILLION = 1.0 / 1e9;
extern struct timespec timeStart, timeCurrent;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
////-----------------------------------------------------------------------------

class Global {
	public:
		int xres, yres;
		Flt aspectRatio;
		Camera cam;
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
		Portal * portals;
		int nportals;

		Global() {
			//constructor
			xres = 1024; 
			yres = 768;
			aspectRatio = (GLfloat)xres / (GLfloat)yres;
			Camera cam;
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
			MakeVector(50.0f, 0.0f, 25.0f, lightPosition);
			lightPosition[3] = 1.0f;

			brutes = new Brute[100];
			nbrutes = 0;
			fliers = new Flier[100];
			nfliers = 0;
			portals = new Portal[10];
			nportals = 0;


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
	remove("./images/flier.ppm");
	remove("./images/portal.ppm");

	//
	//    //convert images to ppm
	system("mogrify -format ppm ./images/floor1.jpg");
	system("mogrify -format ppm ./images/wall1.jpg");
	system("mogrify -format ppm ./images/imp.jpg");
	system("mogrify -format ppm ./images/flier.jpg");
	system("mogrify -format ppm ./images/portal.jpg");
}

void imageClean()
{
	//clean up all images in master folder
	remove("./images/floor1.ppm");
	remove("./images/wall1.ppm");
	remove("./images/imp.ppm");
	remove("./images/imp.ppm");
	remove("./images/flier.ppm");
	remove("./images/portal.ppm");
}

Flt toDegrees(Flt radians) {
	return radians * (180.0 / M_PI);
}

Flt toRadians(Flt degrees) {
	return (degrees * M_PI ) / 180;
}

void init_opengl();
void init_enemies();
void init_portals();

void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

void init_image(char * imagePath, Ppmimage * image, GLuint * texture);
unsigned char *buildAlphaData(Ppmimage *img);
void init_alpha_image(char * imagePath, Ppmimage * image,
		GLuint * texture, GLuint * silhouette);

Flt distance(Smoke s) {

	Flt dist = ((s.pos[0] - g.cam.pos[0])*(s.pos[0] - g.cam.pos[0])) +
		((s.pos[1] - g.cam.pos[1])*(s.pos[1] - g.cam.pos[1])) +
		((s.pos[2] - g.cam.pos[2])*(s.pos[2] - g.cam.pos[2]));


	return dist;
}

int main()
{
	imageConvert();
	init_opengl();
	init_enemies();
	init_portals();
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
		//printf("AngX: %f\n", toDegrees(g.cameraAng[0]));
	}
	cleanup_fonts();
	imageClean();
	return 0;
}

void init_enemies()
{
	//MakeVector(0.0, 0.0, 0.5, g.brutes[0].pos);
	//MakeVector(1.0, 0.0, 0.5, g.brutes[1].pos);
	//MakeVector(-1.0, 0.0, 0.5, g.brutes[2].pos);
	MakeVector(12.5, 0.0, -2.5, g.brutes[0].pos);
	MakeVector(12.5, 0.0, -27.5, g.brutes[1].pos);
	MakeVector(-12.5, 0.0, -27.5, g.brutes[2].pos);

	MakeVector(0.0, 0.0, 0.0, g.brutes[0].vel);
	MakeVector(0.0, 0.0, 0.0, g.brutes[1].vel);
	MakeVector(0.0, 0.0, 0.0, g.brutes[2].vel);

	g.nbrutes = 3;

	MakeVector(0.0, 1.2, 0.5, g.fliers[0].pos);
	MakeVector(1.0, 1.2, 0.5, g.fliers[1].pos);
	MakeVector(-12.5, 1.2, -2.5, g.fliers[2].pos);

	MakeVector(0.0, 0.0, 0.0, g.fliers[0].vel);
	MakeVector(0.0, 0.0, 0.0, g.fliers[1].vel);
	MakeVector(0.0, 0.0, 0.0, g.fliers[2].vel);


	g.nfliers = 3;

}

void init_portals()
{
	MakeVector(12.5, 0.0, -2.5, g.portals[0].pos);
	MakeVector(12.5, 0.0, -27.5, g.portals[1].pos);
	MakeVector(-12.5, 0.0, -2.5, g.portals[2].pos);
	MakeVector(-12.5, 0.0, -27.5, g.portals[3].pos);
	g.nportals = 4;

	//    //Add positioned light
	//    GLfloat lightColor1[] = {0.75f, 0.0f, 0.0f, 1.0f};
	//    GLfloat lightPos1[] = {g.portals[0].pos[0], g.portals[0].pos[1], g.portals[0].pos[2], 0.0f};
	//    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor1);
	//    glLightfv(GL_LIGHT0, GL_POSITION, lightPos1);
	//    glEnable(GL_LIGHT0);
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
	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f}; //Color(0.2, 0.2, 0.2)
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();

	// Initialize brute image
	init_alpha_image((char *)"./images/imp.ppm",
			impImage, &impTexture, &impSilhouette);

	// Initialize flier image
	init_alpha_image((char *)"./images/flier.ppm",
			flierImage, &flierTexture, &flierSilhouette);

	// Initialize portal image
	init_alpha_image((char *)"./images/portal.ppm",
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
}

void check_mouse(XEvent *e)
{
	//Did the mouse move?
	//Was a mouse button clicked?

	static int savex = 0;
	static int savey = 0;
	//bool skip = false;
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
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//if ((savex != e->xbutton.x || savey != e->xbutton.y) && !skip) {
		//Mouse moved
		int xdiff = savex - e->xbutton.x;
		int ydiff = savey - e->xbutton.y;
		//if (++ct < 10)
		//	return;
		if (xdiff < 0) {
			g.cam.lookLeft();
			//			g.angleH += 0.02f;
			//			g.cameraAng[0] = sin(g.angleH);
			//			g.cameraAng[2] = -cos(g.angleH);

		}
		else if (xdiff > 0) {
			g.cam.lookRight();
			//			g.angleH -= 0.02f;
			//			g.cameraAng[0] = sin(g.angleH);
			//			g.cameraAng[2] = -cos(g.angleH);

		}
		if (ydiff < 0) {
			g.cam.lookUp();
			//			g.angleV -= 0.02f;
			//			if (g.angleV < -1.0)
			//				g.angleV = -1.0;
			//
			//			g.cameraAng[1] = sin(g.angleV);

		}
		if (ydiff > 0) {
			g.cam.lookDown();
			//			g.angleV += 0.02f;
			//			if (g.angleV > 1.0)
			//				g.angleV = 1.0;
			//			g.cameraAng[1] = sin(g.angleV);

		}
		//x11.set_mouse_position(g.xres/2,g.yres/2);
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		//skip = !skip;
	}
	}

	int check_keys(XEvent *e)
	{
		//static int shift = false;
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
				//shift = true;
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

				case XK_Escape:
					return 1;
			}
		}
		return 0;
	}

	//    void vecNormalize(Vec v)
	//    {
	//	Flt len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	//	if (len == 0.0)
	//	    return;
	//	len = 1.0 / sqrt(len);
	//	v[0] *= len;
	//	v[1] *= len;
	//	v[2] *= len;
	//    }
	//
	//    void vecScale(Vec v, Flt s)
	//    {
	//	v[0] *= s;
	//	v[1] *= s;
	//	v[2] *= s;
	//    }


	void drawFloor()
	{
		Flt w = 2.5;
		Flt d = 2.5;
		Flt h = -1.0;

		glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
		glPushMatrix();
		//glTranslated(0, 0, 0);
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

	void vecScale(Vec v, Flt s)
	{
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}

	void drawBrutes()
	{
		Flt w = 0.5;
		Flt d = 0.75;
		Flt h = 0.0;

		glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
		//	glPushMatrix();
		//	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		//	glBindTexture(GL_TEXTURE_2D, impSilhouette);
		//	glEnable(GL_ALPHA_TEST);
		//	glAlphaFunc(GL_GREATER, 0.0f); //Alpha

		for (int i = 0; i < g.nbrutes; i++) {

			glPushMatrix();
			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D, impSilhouette);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.0f); //Alpha

			glTranslated(g.brutes[i].pos[0], g.brutes[i].pos[1]-.5, g.brutes[i].pos[2]);
			///// Billboarding
			//Setup camera rotation matrix
			//
			Vec v;
			VecSub(g.brutes[i].pos, g.cam.pos, v);
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

			glRotatef(90, 1, 0, 0);
			//glTranslated(0.0, 0.0, 0.5);
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
		for (int i = 0; i < g.nbrutes; i++) {

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
			VecSub(g.fliers[i].pos, g.cam.pos, v);
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

			glRotatef(90, 1, 0, 0);
			//glTranslated(0.0, 0.0, 0.5);
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
			VecSub(g.portals[i].pos, g.cam.pos, v);
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

			glRotatef(90, 1, 0, 0);
			//glTranslated(0.0, 0.0, 0.5);
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
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_ALPHA_TEST);
	}

	//	    void make_a_smoke()
	//	    {
	//		if (g.nsmokes < MAX_SMOKES) {
	//		    Smoke *s = &g.smoke[g.nsmokes];
	//		    s->pos[0] = rnd() * 5.0 - 2.5;
	//		    s->pos[2] = rnd() * 5.0 - 2.5;
	//		    s->pos[1] = rnd() * 0.1 + 0.1;
	//		    s->separate = true; 
	//		    s->radius = rnd() * 1.0 + 0.5;
	//		    s->n = rand() % 5 + 5;
	//		    Flt angle = 0.0;
	//		    Flt inc = (PI*2.0) / (Flt)s->n;
	//		    for (int i=0; i<s->n; i++) {
	//			s->vert[i][0] = cos(angle) * s->radius;
	//			s->vert[i][1] = sin(angle) * s->radius;
	//			s->vert[i][2] = 0.0;
	//			angle += inc;
	//		    }
	//		    s->maxtime = 8.0;
	//		    s->alpha = 65.0;
	//		    clock_gettime(CLOCK_REALTIME, &s->tstart);
	//		    ++g.nsmokes;
	//		}
	//	    }
	//
	//	    // Used to make child smokes
	//	    void make_a_smoke(Flt x, Flt y, Flt z, Flt r, Flt m, struct timespec t)
	//	    {
	//		if (g.nsmokes < MAX_SMOKES) {
	//		    Smoke *s = &g.smoke[g.nsmokes];
	//		    s->pos[0] = rnd()* 5.0 + x;
	//		    s->pos[2] = rnd()* 5.0 + z;
	//		    s->pos[1] = y;
	//		    s->separate = false;
	//		    s->radius = r/2;
	//		    s->n = rand() % 5 + 5;
	//		    Flt angle = 0.0;
	//		    Flt inc = (PI*2.0) / (Flt)s->n;
	//		    for (int i=0; i<s->n; i++) {
	//			s->vert[i][0] = cos(angle) * s->radius;
	//			s->vert[i][1] = sin(angle) * s->radius;
	//			s->vert[i][2] = 0.0;
	//			angle += inc;
	//		    }
	//		    s->maxtime = m;
	//		    s->alpha = 65.0;
	//		    s->tstart = t;
	//		    ++g.nsmokes;
	//		}
	//	    }


	void physics()
	{

		// Player movement
		if (g.key_states & g.w_mask) {
			if (g.cam.pos[2] > -37.4f) {
				g.cam.moveForward();
			}
		}
		if (g.key_states & g.a_mask) {
			if (g.cam.pos[0] > -22.4f) {
				g.cam.moveLeft();
			}
		}
		if (g.key_states & g.s_mask) {
			if (g.cam.pos[2] < 7.4f) {
				g.cam.moveBackward();
			}
		}
		if (g.key_states & g.d_mask) {
			if (g.cam.pos[0] < 22.4f) {
				g.cam.moveRight();
			}
		}

		//		// Left/Right wall collision
		//		if (g.cam.pos[0] < -22.4f) {
		//			g.cam.pos[0] = -22.4f;
		//			g.cam.vel[0] = 0;
		//		} else if (g.cam.pos[0] > 22.4f) {
		//			g.cam.pos[0] = 22.4;
		//			g.cam.vel[0] = 0;
		//		}
		//
		//		// Back/Front wall collision
		//		if (g.cam.pos[2] < -37.4f) {
		//			g.cam.pos[2] = -37.4f;
		//			g.cam.vel[2] = 0;
		//		} else if (g.cam.pos[2] > 7.4f) {
		//			g.cam.pos[2] = 7.4;
		//			g.cam.vel[2] = 0;
		//		}


		//	g.cam.pos[0] += g.cameraAng[0] * g.cam.vel[0];
		//	g.cam.pos[2] += g.cameraAng[2] * g.cam.vel[2];
		//		if (g.cam.vel[0] < 0.0f)
		//			g.cam.vel[0] += 0.004f;
		//		else if (g.cam.vel[0] > 0.0f)
		//			g.cam.vel[0] -= 0.004f;
		//
		//		if (g.cam.vel[2] < 0.0f)
		//			g.cam.vel[2] += 0.004f;
		//		else if (g.cam.vel[2] > 0.0f)
		//			g.cam.vel[2] -= 0.004f;
		//
		//		if (g.cam.vel[0] >= -0.005f && g.cam.vel[0] <= 0.005f)
		//			g.cam.vel[0] = 0.0f;
		//
		//		if (g.cam.vel[2] >= -0.005f && g.cam.vel[2] <= 0.005f)
		//			g.cam.vel[2] = 0.0f;

		for (int i = 0; i < g.nbrutes; i++) {
			// Camera center - brute center
			Vec v;
			v[0] = g.cam.pos[0] - g.brutes[i].pos[0];
			v[1] = g.cam.pos[1] - g.brutes[i].pos[1];
			v[2] = g.cam.pos[2] - g.brutes[i].pos[2];

			// Normalize vector
			Flt len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
			if (len > 1.5) {
				Vec norm = {v[0]/len, v[1]/len, v[2]/len};

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



		}

		for (int i = 0; i < g.nfliers; i++) {
			// Camera center - fliers center
			Vec v;
			v[0] = g.cam.pos[0] - g.fliers[i].pos[0];
			v[1] = g.cam.pos[1] - g.fliers[i].pos[1];
			v[2] = g.cam.pos[2] - g.fliers[i].pos[2];

			// Normalize vector
			Flt len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
			Vec norm = {v[0]/len, v[1]/len, v[2]/len};

			g.fliers[i].vel[0] = norm[0]*.04;
			g.fliers[i].vel[1] = norm[1]*.04;
			g.fliers[i].vel[2] = norm[2]*.04;

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


		/////// From smoke lab
		//		clock_gettime(CLOCK_REALTIME, &g.smokeTime);
		//		double d = timeDiff(&g.smokeStart, &g.smokeTime);
		//		if (d > 0.05) {
		//		    //time to make another smoke particle
		//		    make_a_smoke();
		//		    timeCopy(&g.smokeStart, &g.smokeTime);
		//		}
		//		//move smoke particles
		//		for (int i=0; i<g.nsmokes; i++) {
		//		    //smoke rising
		//		    g.smoke[i].pos[1] += 0.015;
		//		    g.smoke[i].pos[1] += ((g.smoke[i].pos[1]*0.24) * (rnd() * 0.075));
		//
		//		    //expand particle as it rises
		//		    g.smoke[i].radius += g.smoke[i].pos[1]*0.002;
		//		    //wind might blow particle
		//		    if (g.smoke[i].pos[1] > 10.0) {
		//			g.smoke[i].pos[0] -= rnd() * 0.1;
		//		    }
		//		    // break apart
		//		    if ((g.smoke[i].pos[1] > rnd() * 20 + 20) & g.smoke[i].separate) {
		//			// Save vars from parent smoke
		//			Flt x = g.smoke[i].pos[0];
		//			Flt y = g.smoke[i].pos[1];
		//			Flt z = g.smoke[i].pos[2];
		//			Flt r = g.smoke[i].radius;
		//			Flt m = g.smoke[i].maxtime;
		//			struct timespec t = g.smoke[i].tstart;
		//			// Delete parent smoke
		//			--g.nsmokes;
		//			g.smoke[i] = g.smoke[g.nsmokes];
		//			// Generate two new child smokes
		//			make_a_smoke(x, y, z, r, m, t);
		//			make_a_smoke(x, y, z, r, m, t);
		//		    }
		//		}
		//		//check for smoke out of time
		//		int i=0;
		//		while (i < g.nsmokes) {
		//		    struct timespec bt;
		//		    clock_gettime(CLOCK_REALTIME, &bt);
		//		    double d = timeDiff(&g.smoke[i].tstart, &bt);
		//		    if (d > g.smoke[i].maxtime - 3.0) {
		//			g.smoke[i].alpha *= 0.95;
		//			if (g.smoke[i].alpha < 1.0)
		//			    g.smoke[i].alpha = 1.0;
		//		    }
		//		    if (d > g.smoke[i].maxtime) {
		//			//delete this smoke
		//			--g.nsmokes;
		//			g.smoke[i] = g.smoke[g.nsmokes];
		//			continue;
		//		    }
		//		    ++i;
		//		}
		//		//
		//		if (g.circling) {
		//		    Flt rad = 80 + sin(g.cameraAngle) * 50.0;
		//		    Flt x = cos(g.cameraAngle) * rad;
		//		    Flt z = sin(g.cameraAngle) * rad;
		//		    Flt y = 25.0;
		//		    MakeVector(x, y, z, g.cam.pos);
		//		    g.cameraAngle -= 0.01;
		//		}



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
		gluLookAt(g.cam.pos[0] , 0.0f , g.cam.pos[2],
				g.cam.view[0], g.cam.view[1], g.cam.view[2],
				g.cam.upv[0],  g.cam.upv[1], g.cam.upv[2]);

		glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
		//
		drawFloor();
		drawWall();
		drawPortals();
		drawBrutes();
		drawFliers();
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
		ggprint8b(&r, 16, 0x00887766, "    Position: [%.2f, %.2f, %.2f]", g.cam.pos[0], g.cam.pos[1], g.cam.pos[2]);
		ggprint8b(&r, 16, 0x00887766, "    Direction: [%.2f, %.2f, %.2f]", g.cam.view[0], g.cam.view[1], g.cam.view[2]);
		ggprint8b(&r, 16, 0x00887766, "    Velocity: [%.2f, %.2f]", g.cam.vel[0], g.cam.vel[2]);

		ggprint8b(&r, 16, 0x00887766, "Controls:");
		ggprint8b(&r, 16, 0x00887766, "    Mouse: Look Around");
		ggprint8b(&r, 16, 0x00887766, "    w/s: Move Forward/Backward");
		ggprint8b(&r, 16, 0x00887766, "    a/d: Look Left/Right");
		ggprint8b(&r, 16, 0x00887766, "    e/q: Strafe Left/Right");
		ggprint8b(&r, 16, 0x00887766, "    c/z: Strafe Up/Down");
	}



