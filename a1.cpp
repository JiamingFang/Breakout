// CS 349 Fall 2018
// A1: Breakout code sample
// You may use any or all of this code in your assignment!
// See makefile for compiling instructions

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <list>
#include <sstream>
#include <sys/time.h>
#include <vector>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

// X11 structures
Display* display;
Window window;

Colormap cmap;
XColor red,purple,blue,green,yellow;

struct XInfo {
	Display*  display;
	Window   window;
	GC       gc;
	Pixmap pixmap;
};

// fixed frames per second animation


// get current time
unsigned long now() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

/*
 * An abstract class representing displayable things.
 */
class Displayable {
public:
	virtual void paint(XInfo& xinfo) = 0;
};

/*
 * A text displayable
 */
class Text : public Displayable {
public:
	virtual void paint(XInfo& xinfo) {
		XDrawImageString( xinfo.display, xinfo.pixmap, xinfo.gc,
			this->x, this->y, this->s.c_str(), this->s.length() );
	}

  // constructor
	Text(int x, int y, string s): x(x), y(y), s(s)  {}

private:
	int x;
	int y;
  string s; // string to show
};

class Rectangle : public Displayable {
public:
	virtual void paint(XInfo& xinfo){
		XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc, this->x, this->y, this->width, this->height);
	}
	virtual void fill(XInfo& xinfo){
		XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc, this->x, this->y, this->width, this->height);
	}

	Rectangle(int x, int y, int width, int height): x(x), y(y), width(width), height(height) {}

	int x;
	int y;
	int width;
	int height;
};

void repaint( list<Displayable*> dList, XInfo& xinfo) {
	list<Displayable*>::const_iterator begin = dList.begin();
	list<Displayable*>::const_iterator end = dList.end();

	XClearWindow(xinfo.display, xinfo.window);
	while ( begin != end ) {
		Displayable* d = *begin;
		d->paint(xinfo);
		begin++;
	}
	XFlush(xinfo.display);
}

/*
void collision(list<Displayable*> dList, int x, int y，int ballSize, int &dirx, int &diry){
	list<Displayable*>::const_iterator begin = dList.begin();
  	list<Displayable*>::const_iterator end = dList.end();
	while ( begin != end ) {
	    Rectangle* d = *begin;
	    if(y + ballSize/2 >=d->y && y -ballSize/2 <= d->y+d->height &&
				((x + ballSize/2 >= d->x && x + ballSize/2 <= d->x+3) ||
					(x - ballSize/2 <= d->x+d->width && x - ballSize/2 >= d->x+(d->width-3)))){
	    	dirx = - dirx;
	    	dList.erase(begin);
	    }
	    if(x + ballSize/2 >= d->x && x - ballSize/2 <= d->x+d->width &&
					y + ballSize/2 >= d->y && y + ballSize/2 <= d->y+3){
	    	diry = -diry;
	    	dList.erase(begin);
	    }
	    	
	    begin++;
	}
}
*/


// entry point
int main( int argc, char *argv[] ) {

	XInfo xinfo;
	xinfo.display = display;
	xinfo.window = window;

	float FPS = 30;
	float speed = 6;
	int SCORE = 0;

	if(argc == 2){
		FPS = atoi(argv[1]);
	}
	else if(argc >= 3){
		FPS =atoi(argv[1]);
		speed = atoi(argv[2]);
	}

	if(speed > 10){
		cerr << "The speed you entered is too large, please enter a speed between 1 and 10" << endl;
		exit(1);
	}
	if(speed < 1){
		cerr << "The speed you entered is too small, please enter a speed between 1 and 6" << endl;
		exit(1);
	}

	speed = speed /(FPS/30);
	if (speed <= 1){
		speed = 1;
	}


	// create window
	xinfo.display = XOpenDisplay("");
	if (xinfo.display == NULL) exit (-1);
	int screennum = DefaultScreen(xinfo.display);
	long background = WhitePixel(xinfo.display, screennum);
	long foreground = BlackPixel(xinfo.display, screennum);
	xinfo.window = XCreateSimpleWindow(xinfo.display, DefaultRootWindow(xinfo.display),
		10, 10, 1280, 800, 2, foreground, background);

	// set events to monitor and display window
	XSelectInput(xinfo.display, xinfo.window, ButtonPressMask | KeyPressMask);
	XMapRaised(xinfo.display, xinfo.window);
	XFlush(xinfo.display);

	// ball postition, size, and velocity
	

	

	XPoint ballDir;
	ballDir.x = 0;
	ballDir.y = 0;

	// block position, size
	XPoint rectPos;
	rectPos.x = 615;
	rectPos.y = 750;

	//set width and height of rectangle block
	int recwidth = 100;
	int recheight = 8;

	XPoint ballPos;
	ballPos.x = rectPos.x + recwidth/2;
	ballPos.y = 745;
	int ballSize = 10;

	// create gc for drawing
	xinfo.gc = XCreateGC(xinfo.display, xinfo.window, 0, 0);
	XWindowAttributes w;
	XGetWindowAttributes(xinfo.display, xinfo.window, &w);

	int depth = DefaultDepth(xinfo.display, DefaultScreen(xinfo.display));
	
	xinfo.pixmap = XCreatePixmap(xinfo.display, xinfo.window, 1280, 800, depth); 
	int screen = DefaultScreen( xinfo.display );
	XSetForeground(xinfo.display, xinfo.gc, BlackPixel(xinfo.display, screen));
	XSetBackground(xinfo.display, xinfo.gc, WhitePixel(xinfo.display, screen));
	// save time of last window paint
	unsigned long lastRepaint = 0;

	//color
	cmap = DefaultColormap(xinfo.display, DefaultScreen(xinfo.display));
	XAllocNamedColor(xinfo.display,cmap,"red",&red,&red);
	XAllocNamedColor(xinfo.display,cmap,"purple",&purple,&purple);
	XAllocNamedColor(xinfo.display,cmap,"blue",&blue,&blue);
	XAllocNamedColor(xinfo.display,cmap,"green",&green,&green);
	XAllocNamedColor(xinfo.display,cmap,"yellow",&yellow,&yellow);

	// list of Displayables

	list<Displayable*> dList;
	list<Displayable*> dList2;
	vector<Rectangle*> blocks;
	ostringstream fps;
	fps << FPS;
	bool start = False;
	bool ending = False;
	bool eject = False;


	list<Displayable*> dList3;
	dList3.push_back(new Text(550, 300, "BREAKOUT"));
	dList3.push_back(new Text(550, 320, "Created by Jiaming Fang, ID#20650159"));
	dList3.push_back(new Text(550, 340, "Press A and D to move the paddle "));
	dList3.push_back(new Text(550, 360, "Press W and S to change the width of the paddle "));
	dList3.push_back(new Text(550, 380, "Press T to start the game!"));


	ostringstream score;
	score << SCORE;

	dList.push_back(new Text(10, 10, "FPS: "+fps.str()));
	dList.push_back(new Text(200, 10, "Press q to quite the game"));
	dList.push_back(new Text(200, 30, "Press r to restart the game"));
	dList.push_back(new Text(400, 10, "Press w to increase the width of the paddle"));
	dList.push_back(new Text(400, 30, "Press s to decrease the width of the paddle"));
	dList.push_back(new Text(400, 50, "Press j to shoot the ball"));
	dList.push_back(new Text(10, 30, "Score: "+score.str()));

	dList2.push_back(new Text(550, 300, "Press q to quite the game"));
	dList2.push_back(new Text(550, 320, "Press r to restart the game"));
	

	blocks.push_back(new Rectangle(140,200, 80, 25));
	blocks.push_back(new Rectangle(240,200, 80, 25));
	blocks.push_back(new Rectangle(340,200, 80, 25));
	blocks.push_back(new Rectangle(440,200, 80, 25));
	blocks.push_back(new Rectangle(540,200, 80, 25));
	blocks.push_back(new Rectangle(640,200, 80, 25));
	blocks.push_back(new Rectangle(740,200, 80, 25));
	blocks.push_back(new Rectangle(840,200, 80, 25));
	blocks.push_back(new Rectangle(940,200, 80, 25));
	blocks.push_back(new Rectangle(1040,200, 80, 25));

	blocks.push_back(new Rectangle(140,250, 80, 25));
	blocks.push_back(new Rectangle(240,250, 80, 25));
	blocks.push_back(new Rectangle(340,250, 80, 25));
	blocks.push_back(new Rectangle(440,250, 80, 25));
	blocks.push_back(new Rectangle(540,250, 80, 25));
	blocks.push_back(new Rectangle(640,250, 80, 25));
	blocks.push_back(new Rectangle(740,250, 80, 25));
	blocks.push_back(new Rectangle(840,250, 80, 25));
	blocks.push_back(new Rectangle(940,250, 80, 25));
	blocks.push_back(new Rectangle(1040,250, 80, 25));

	blocks.push_back(new Rectangle(140,300, 80, 25));
	blocks.push_back(new Rectangle(240,300, 80, 25));
	blocks.push_back(new Rectangle(340,300, 80, 25));
	blocks.push_back(new Rectangle(440,300, 80, 25));
	blocks.push_back(new Rectangle(540,300, 80, 25));
	blocks.push_back(new Rectangle(640,300, 80, 25));
	blocks.push_back(new Rectangle(740,300, 80, 25));
	blocks.push_back(new Rectangle(840,300, 80, 25));
	blocks.push_back(new Rectangle(940,300, 80, 25));
	blocks.push_back(new Rectangle(1040,300, 80, 25));

	blocks.push_back(new Rectangle(140,350, 80, 25));
	blocks.push_back(new Rectangle(240,350, 80, 25));
	blocks.push_back(new Rectangle(340,350, 80, 25));
	blocks.push_back(new Rectangle(440,350, 80, 25));
	blocks.push_back(new Rectangle(540,350, 80, 25));
	blocks.push_back(new Rectangle(640,350, 80, 25));
	blocks.push_back(new Rectangle(740,350, 80, 25));
	blocks.push_back(new Rectangle(840,350, 80, 25));
	blocks.push_back(new Rectangle(940,350, 80, 25));
	blocks.push_back(new Rectangle(1040,350, 80, 25));

	blocks.push_back(new Rectangle(140,400, 80, 25));
	blocks.push_back(new Rectangle(240,400, 80, 25));
	blocks.push_back(new Rectangle(340,400, 80, 25));
	blocks.push_back(new Rectangle(440,400, 80, 25));
	blocks.push_back(new Rectangle(540,400, 80, 25));
	blocks.push_back(new Rectangle(640,400, 80, 25));
	blocks.push_back(new Rectangle(740,400, 80, 25));
	blocks.push_back(new Rectangle(840,400, 80, 25));
	blocks.push_back(new Rectangle(940,400, 80, 25));
	blocks.push_back(new Rectangle(1040,400, 80, 25));

	// event handle for current event
	XEvent event;


	// event loop
	while ( true ) {

		// process if we have any events
		if (XPending(xinfo.display) > 0) { 
			XNextEvent( xinfo.display, &event ); 

			switch ( event.type ) {

				// mouse button press
				case ButtonPress:
				cout << "Click" << endl;
				break;

				case KeyPress: // any keypress
				KeySym key;
				char text[10];
				int i = XLookupString( (XKeyEvent*)&event, text, 10, &key, 0 );

					// move right
				if ( i == 1 && text[0] == 'd' ) {
					if(rectPos.x + recwidth + 20 <= 1280){
						rectPos.x += 20;
					}
					else{
						rectPos.x = 1280 - recwidth;
					}
				}

					// move left
				if ( i == 1 && text[0] == 'a' ) {
					if(rectPos.x - 20 >= 0){
						rectPos.x -= 20;
					}
					else{
						rectPos.x = 0;
					}

				}

				if ( i == 1 && text[0] == 'w' ) {
					if(rectPos.x + recwidth + 50 <=1280){
						recwidth+=50;
					}
				}

				if ( i == 1 && text[0] == 's' ) {
					if(recwidth - 50 >= 100){
						recwidth-=50;
					}

				}


				if ( i == 1 && text[0] == 't') {
					start = True;
				}

				if ( i == 1 && text[0] == 'j' && eject == False) {
					eject = True;
					int v1 = rand() % 2;
					if(v1 == 1){
						ballDir.x = speed;
					}
					else{
						ballDir.x = -speed;
					}
					ballDir.y = -speed;
				}

					// quit game
				if (( i == 1 && text[0] == 'q') && (start == True || (start == False && ending == True))) {
					XCloseDisplay(xinfo.display);
					exit(0);
				}

				if((i == 1 && text[0] == 'r') && (start == True || (start == False && ending == True))) {
					XClearWindow(xinfo.display, xinfo.window);
						//XClearWindow(xinfo.display, xinfo.pixmap);
					SCORE = 0;
					blocks.clear();
					rectPos.x = 615;
					rectPos.y = 750;
					dList.push_back(new Text(10, 10, "FPS: "+fps.str()));
					dList.push_back(new Text(200, 10, "Press q to quite the game"));
					dList.push_back(new Text(200, 30, "Press r to restart the game"));
					dList.push_back(new Text(400, 10, "Press w to increase the width of the paddle"));
					dList.push_back(new Text(400, 30, "Press s to decrease the width of the paddle"));
					dList.push_back(new Text(400, 50, "Press j to shoot the ball"));
					dList.push_back(new Text(10, 30, "Score: "+score.str()));
					start = True;
					ending = False;
					eject = False;
					ballDir.x = 0;
					ballDir.y = 0;
				}

				break;
			}
		}

		unsigned long end = now();	// get current time in microsecond

		if (end - lastRepaint > 1000000 / FPS) { 
			// clear background
			//XClearWindow(xinfo.display, xinfo.window);
			xinfo.pixmap = xinfo.window;
			XClearWindow(xinfo.display, xinfo.pixmap);
			if(start == True){
				

				if(dList.size() != 0){
					dList.pop_back();
				}
				
				score.str("");
				score.clear();
				score << SCORE;
				dList.push_back(new Text(10, 30, "Score: "+score.str()));

				repaint(dList, xinfo);

				if(blocks.size() == 0){
					blocks.push_back(new Rectangle(140,200, 80, 25));
					blocks.push_back(new Rectangle(240,200, 80, 25));
					blocks.push_back(new Rectangle(340,200, 80, 25));
					blocks.push_back(new Rectangle(440,200, 80, 25));
					blocks.push_back(new Rectangle(540,200, 80, 25));
					blocks.push_back(new Rectangle(640,200, 80, 25));
					blocks.push_back(new Rectangle(740,200, 80, 25));
					blocks.push_back(new Rectangle(840,200, 80, 25));
					blocks.push_back(new Rectangle(940,200, 80, 25));
					blocks.push_back(new Rectangle(1040,200, 80, 25));

					blocks.push_back(new Rectangle(140,250, 80, 25));
					blocks.push_back(new Rectangle(240,250, 80, 25));
					blocks.push_back(new Rectangle(340,250, 80, 25));
					blocks.push_back(new Rectangle(440,250, 80, 25));
					blocks.push_back(new Rectangle(540,250, 80, 25));
					blocks.push_back(new Rectangle(640,250, 80, 25));
					blocks.push_back(new Rectangle(740,250, 80, 25));
					blocks.push_back(new Rectangle(840,250, 80, 25));
					blocks.push_back(new Rectangle(940,250, 80, 25));
					blocks.push_back(new Rectangle(1040,250, 80, 25));

					blocks.push_back(new Rectangle(140,300, 80, 25));
					blocks.push_back(new Rectangle(240,300, 80, 25));
					blocks.push_back(new Rectangle(340,300, 80, 25));
					blocks.push_back(new Rectangle(440,300, 80, 25));
					blocks.push_back(new Rectangle(540,300, 80, 25));
					blocks.push_back(new Rectangle(640,300, 80, 25));
					blocks.push_back(new Rectangle(740,300, 80, 25));
					blocks.push_back(new Rectangle(840,300, 80, 25));
					blocks.push_back(new Rectangle(940,300, 80, 25));
					blocks.push_back(new Rectangle(1040,300, 80, 25));

					blocks.push_back(new Rectangle(140,350, 80, 25));
					blocks.push_back(new Rectangle(240,350, 80, 25));
					blocks.push_back(new Rectangle(340,350, 80, 25));
					blocks.push_back(new Rectangle(440,350, 80, 25));
					blocks.push_back(new Rectangle(540,350, 80, 25));
					blocks.push_back(new Rectangle(640,350, 80, 25));
					blocks.push_back(new Rectangle(740,350, 80, 25));
					blocks.push_back(new Rectangle(840,350, 80, 25));
					blocks.push_back(new Rectangle(940,350, 80, 25));
					blocks.push_back(new Rectangle(1040,350, 80, 25));

					blocks.push_back(new Rectangle(140,400, 80, 25));
					blocks.push_back(new Rectangle(240,400, 80, 25));
					blocks.push_back(new Rectangle(340,400, 80, 25));
					blocks.push_back(new Rectangle(440,400, 80, 25));
					blocks.push_back(new Rectangle(540,400, 80, 25));
					blocks.push_back(new Rectangle(640,400, 80, 25));
					blocks.push_back(new Rectangle(740,400, 80, 25));
					blocks.push_back(new Rectangle(840,400, 80, 25));
					blocks.push_back(new Rectangle(940,400, 80, 25));
					blocks.push_back(new Rectangle(1040,400, 80, 25));
				}

				for (int i = 0; i < blocks.size();i++){
					if(i >= 0 & i <= 9){
						XSetForeground(xinfo.display, xinfo.gc, red.pixel);
					}
					else if(i >= 10 & i <= 19){
						XSetForeground(xinfo.display, xinfo.gc, purple.pixel);
					}
					else if(i >= 20 & i <= 29){
						XSetForeground(xinfo.display, xinfo.gc, blue.pixel);
					}
					else if(i >= 30 & i <= 39){
						XSetForeground(xinfo.display, xinfo.gc, green.pixel);
					}
					else if(i >= 40 & i <= 49){
						XSetForeground(xinfo.display, xinfo.gc, yellow.pixel);
					}
					blocks[i]->fill(xinfo);
				}
				XSetForeground(xinfo.display, xinfo.gc, BlackPixel(xinfo.display, screen));

				// draw rectangle
				XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc, rectPos.x, rectPos.y, recwidth, recheight);

				// draw ball from centre
				XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc, 
					ballPos.x - ballSize/2, 
					ballPos.y - ballSize/2, 
					ballSize, ballSize,
					0, 360*64);

				// update ball position
				ballPos.x += ballDir.x;
				ballPos.y += ballDir.y;

				if(eject == False){
					ballPos.x = rectPos.x + recwidth/2;
					ballPos.y = 745;
				}

				//end game
				if(ballPos.y + ballSize/2 > w.height){
					XClearWindow(xinfo.display, xinfo.window);
					XClearWindow(xinfo.display, xinfo.pixmap);
					dList.clear();
					repaint(dList2, xinfo);
					start = False;
					ending = True;
				}

				// bounce ball
				if (ballPos.x + ballSize/2 > w.width ||
					ballPos.x - ballSize/2 < 0 || (ballPos.y + ballSize/2 >=rectPos.y && ballPos.y -ballSize/2 <= rectPos.y+recheight &&
						((ballPos.x + ballSize/2 >= rectPos.x && ballPos.x + ballSize/2 <= rectPos.x+speed && ballDir.x > 0) ||
							(ballPos.x - ballSize/2 <= rectPos.x+recwidth && ballPos.x - ballSize/2 >= rectPos.x+(recwidth-speed) && ballDir.x < 0))))
					ballDir.x = -ballDir.x;
				if (ballPos.y - ballSize/2 < 0 || (ballPos.x + ballSize/2 >= rectPos.x && ballPos.x - ballSize/2 <= rectPos.x+recwidth &&
					ballPos.y + ballSize/2 >= rectPos.y && ballPos.y + ballSize/2 <= rectPos.y+speed))
					ballDir.y = -ballDir.y;
			 //test			collision(dList, ballPos.x, ballPos.y, ballSize, &ballDir.x, &ballDir.y);


				int erase = false;
				for (int i = 0; i < blocks.size();i++){
					if((ballPos.y + ballSize/2 >=blocks[i]->y && ballPos.y -ballSize/2 <= blocks[i]->y+blocks[i]->height &&
						((ballPos.x + ballSize/2 >= blocks[i]->x && ballPos.x + ballSize/2 <= blocks[i]->x+speed && ballDir.x > 0) ||
							(ballPos.x - ballSize/2 <= blocks[i]->x+blocks[i]->width && ballPos.x - ballSize/2 >= blocks[i]->x+(blocks[i]->width-speed) && ballDir.x < 0)))){
						//cout << "x:" << ballPos.x << " y:" << ballPos.y << " blockx:" << blocks[i]->x << "blocky:" << blocks[i]->y << endl;
						ballDir.x = -ballDir.x;
						erase = true;
					}

					if(ballPos.x + ballSize/2 >= blocks[i]->x && ballPos.x - ballSize/2 <= blocks[i]->x+blocks[i]->width &&
						((ballPos.y + ballSize/2 >= blocks[i]->y && ballPos.y + ballSize/2 <= blocks[i]->y+speed && ballDir.y > 0) || 
						(ballPos.y - ballSize/2 <= blocks[i]->y+blocks[i]->height && ballPos.y - ballSize/2 >= blocks[i]->y+blocks[i]->height-speed && ballDir.y < 0))){
						//cout << "x:" << ballPos.x << " y:" << ballPos.y << " blockx:" << blocks[i]->x << "blocky:" << blocks[i]->y << endl;
					ballDir.y = -ballDir.y;
				erase = true;
			}
			if(erase == true){
				blocks.erase(blocks.begin()+i);
				i--;
				SCORE++;
				erase = false;
			}


		}
	}
	else{
		if(ending == False){
			repaint(dList3, xinfo);
			XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc, rectPos.x, rectPos.y, recwidth, recheight);
		}
		else{
			repaint(dList2, xinfo);
		}
		
	}
	XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window,
		xinfo.gc,
					0, 0, 1280, 800, // pixmap region to copy
					0, 0); 

	XFlush( xinfo.display );

				lastRepaint = now(); // remember when the paint happened
				
			}

		// IMPORTANT: sleep for a bit to let other processes work
			if (XPending(xinfo.display) == 0) {
				usleep(1000000 / FPS - (now() - lastRepaint));
			}
		}
		XCloseDisplay(xinfo.display);
	}
