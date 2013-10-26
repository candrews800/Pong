/* Pong
by Clinton Andrews

Gameplay:

Pong is played using similar rules to tennis.

A ball is randomly launched from the servers side of the playing field towards the opponents paddle.

The opponent can either hit the ball back by placing his paddle in the balls path or miss the ball.

If the opponent hits the ball back to the server, the same options are presented to the server as the opponent.

This continues until a ball is missed and a point is credited to the player opposite the miss.

The points are displayed in the middle of the screen, in between the points  the neutral line or 'net' is displayed.

The structure of the playing field is simply a rectangular field.

Walls line both the top and bottom of the field and reflect the balls in the opposite vertical direction if a collision is detected.

The paddles are representd by rectangles and are the only piece each player can move within the game.

This is a two player game with no AI and thus controls will be setup to use each paddle seperately.

Once struck, a paddle will send the ball in the opposite direction that it was hit. It will act in essence as a movable wall.

The ball is a circle with collision detection as if it is a square.

*/ 

// To Do:
// 1. Get Up Background Screen
// 2. Motion for Paddles
// 3. Ball Motion
// 4. Collision Detection
// 5. Score System

// Misc:
// 1. clean_up() needs to release all files
// 2. Change Motion Controls to Key States -- Current Bug is when you Press w-UP-w, etc real quick it moves on up auto after you release.

//Headers
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <string>
#include <sstream>
#include <vector>

//Screen attributes
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 32;

//The frame rate
const int FRAMES_PER_SECOND = 60;

//The surfaces
SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *playerone = NULL;
SDL_Surface *playertwo = NULL;

TTF_Font *font = NULL;

Mix_Chunk *bounce = NULL;

SDL_Color textColor = {255,255,255};

//The event structure
SDL_Event event;

int player_one_score = 0;
int player_two_score = 0;



//SDL Functions
SDL_Surface *load_image( std::string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }

	//Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        return false;    
    }

	//Initialize SDL_mixer
    if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
    {
        return false;    
    }

	background = load_image("background.png");
	font = TTF_OpenFont("5thAgent.ttf", 150);
	bounce = Mix_LoadWAV("medium.wav");

    //Set the window caption
    SDL_WM_SetCaption( "Pong by Clinton Andrews", NULL );

    //If everything initialized fine
    return true;
}

void clean_up()
{
    //Free the surface
  //  SDL_FreeSurface( );

    //Quit SDL
    SDL_Quit();
}

//The timer
class Timer
{
    private:
    //The clock time when the timer started
    int startTicks;

    //The ticks stored when the timer was paused
    int pausedTicks;

    //The timer status
    bool paused;
    bool started;

    public:
    //Initializes variables
    Timer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //Gets the timer's time
    int get_ticks();

    //Checks the status of the timer
    bool is_started();
    bool is_paused();
};
Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::start()
{
    //Start the timer
    started = true;

    //Unpause the timer
    paused = false;

    //Get the current clock time
    startTicks = SDL_GetTicks();
}

void Timer::stop()
{
    //Stop the timer
    started = false;

    //Unpause the timer
    paused = false;
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;

        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;

        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;

        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }
    }

    //If the timer isn't running
    return 0;
}

bool Timer::is_started()
{
    return started;
}

bool Timer::is_paused()
{
    return paused;
}
/* Wall Class:

Variables - Collision box, velocities

Functions-
-Handle Input - Only paddles move and they can only move up and down
		Update velocities
-Move - Check dimensions of paddle in order to keep it on screen
	Update the new coordinates
-Show - Show the wall on the screen */

class Wall
{
private:
	// Coordinates + Speed
	int x, y, w, h;
	int yVel;


public:
	SDL_Surface *image;

	//Initialize Variables
	Wall( int x, int y, int w, int h );

	//Handle Input
	void handle_input(int player);
	
	//Move
	void move();

	//Show
	void show();

	int get_x();
	int get_y();
};

int Wall::get_x()
{
	return x;
}

int Wall::get_y()
{
	return y;
}

Wall::Wall( int X, int Y, int W, int H)
{
	x = X;
	y = Y;
	w = W;
	h = H;

	yVel=0;
}

void Wall::handle_input(int player)
{
	//Player 1 Controls
	if(player == 1)
	{
		//If a key was pressed
		if( event.type == SDL_KEYDOWN )
		{
	        //Adjust the velocity
	        switch( event.key.keysym.sym )
	        {
	           case SDLK_w: yVel = -10; break;
	           case SDLK_s: yVel = 10; break;
	       }
	   }
		//If a key was released
		else if( event.type == SDL_KEYUP )
		{
		    //Adjust the velocity
		    switch( event.key.keysym.sym )
		    {
		        case SDLK_w: yVel = 0; break;
		        case SDLK_s: yVel = 0; break;
		    }
		}
	}
	else if(player == 2)
	{
		//If a key was pressed
		if( event.type == SDL_KEYDOWN )
		{
	        //Adjust the velocity
	        switch( event.key.keysym.sym )
	        {
	           case SDLK_UP: yVel = -10; break;
	           case SDLK_DOWN: yVel = 10; break;
	       }
	   }
		//If a key was released
		else if( event.type == SDL_KEYUP )
		{
		    //Adjust the velocity
		    switch( event.key.keysym.sym )
		    {
		        case SDLK_UP: yVel = 0; break;
		        case SDLK_DOWN: yVel = 0; break;
		    }
		}
	}

}

void Wall::move()
{
	y=y+yVel;

	if(y<50)
	{
		y=50;
	}

	if(y+h>550)
	{
		y=500;
	}
}

void Wall::show()
{
	//Show the image of Wall
	apply_surface(x,y,image,screen);
}

/*Ball Class:

Variables- Collision box, Velocities

Functions-
-Move - Check dimensions of ball to determine if it has hit a wall, paddle, or edge of playing surface.
	Update the new coordinates
	Change score
-Show - Show the ball on the screen */

class Ball
{
private:
	int x, y, w, h;
	int xVel, yVel;
	
public:
	SDL_Surface *image;

	Ball(int x, int y, int w, int h);
	void move(int YPADONE, int YPADTWO);
	void show();
	void send_vel(int XVEL, int YVEL);
	void send_coord(int X, int Y);
	bool check_collision(int YPAD_ONE, int YPAD_TWO);
	int get_xcoord();
};

Ball::Ball(int X, int Y, int W, int H)
{
	x=X;
	y=Y;
	w=W;
	h=H;

	xVel=0;
	yVel=0;
}

void Ball::move(int YPADONE, int YPADTWO){
	//Move Ball
	x=x+xVel;
	y=y+yVel;

	//Check for Walls
	if(y<50)
	{
		y=y-yVel;
		yVel=yVel*-1;
	}
	if(y+h>550)
	{
		y=y-yVel;
		yVel=yVel*-1;
	}
	if(check_collision(YPADONE,YPADTWO))
	{
		xVel=xVel*-1;
		Mix_PlayChannel( -1, bounce, 0 );
	}
	if(x<0)
	{
		x=0;
		xVel=0;
		yVel=0;
	}
	if(x+w>800)
	{
		x=780;
		xVel=0;
		yVel=0;
	}
}

bool Ball::check_collision(int YPAD_ONE, int YPAD_TWO)
{
	if(x==75)
	{
		//Check if the Corners of the Ball are within the Corners of the paddle
		if(y>YPAD_ONE-h && y<YPAD_ONE+75)
		{
			return true;
		}
	}

	if(x==705)
	{
		//Check if the Corners of the Ball are within the Corners of the paddle
		if(y>YPAD_TWO-h && y<YPAD_TWO+75)
		{
			return true;
		}
	}
	return false;
}

void Ball::show(){
	apply_surface(x,y,image,screen);
}

void Ball::send_vel(int XVEL, int YVEL)
{
	xVel=XVEL;
	yVel=YVEL;
}

void Ball::send_coord(int X, int Y)
{
	x=X;
	y=Y;
}

int Ball::get_xcoord()
{
	return x;
}

void score_screen(int player)
{
	std::stringstream out;
	std::string ss;
	if(player==1)
	{
		player_one_score += 1;
		out << player_one_score;
		playerone = TTF_RenderText_Solid(font,out.str().c_str(),textColor);
	}
	if(player==2)
	{
		player_two_score += 1;
		out << player_two_score;
		playertwo = TTF_RenderText_Solid(font,out.str().c_str(),textColor);
	}
}

int main( int argc, char* args[] )
{
    //Quit flag
    bool quit = false;
	Timer fps;

    //Initialize
    if( init() == false )
    {
        return 1;
    }

	//Paddles
	Wall paddle(50,263,25,50);
	Wall paddle_two(725,263,25,50);
	paddle.image = load_image("paddle.png");
	paddle_two.image = load_image("paddle.png");

	//Top and Bottom Walls + Net
	Wall top(50,25,700,25);
	Wall bottom(50,550,700,25);
	Wall net(395,50,10,500);
	top.image = load_image("wall.png");
	bottom.image = load_image("wall.png");
	net.image = load_image("net.png");

	//Ball
	Ball ball(85,300,20,20);
	ball.image = load_image("ball.png");
	ball.send_vel(5,5);

	playerone = TTF_RenderText_Solid(font, "0", textColor);
	playertwo = TTF_RenderText_Solid(font, "0", textColor);
	
    //While the user hasn't quit
    while( quit == false )
    {
		//Start the frame timer
        fps.start();
		//Show the background + playing field
		apply_surface(0,0,background,screen);
		if(player_one_score>9){
			apply_surface(225,50,playerone,screen);
		}
		else
		{
			apply_surface(275,50,playerone,screen);
		}
		apply_surface(450,50,playertwo,screen);
		top.show();
		bottom.show();		
		net.show();

        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }

		paddle.handle_input(1);
		paddle_two.handle_input(2);
		paddle.move();
		paddle_two.move();
		ball.move(paddle.get_y(), paddle_two.get_y());
		paddle.show();
		paddle_two.show();
		ball.show();

        //Update the screen
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }
		//Cap the frame rate
        if( fps.get_ticks() < 1000 / FRAMES_PER_SECOND )
        {
            SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps.get_ticks() );
        }

		//Check Status of the Ball
		//If it has passed a paddel, delay 1 sec, Update Score, Set new ball coords and vel for winner 

		if(ball.get_xcoord() == 0)
		{	
			SDL_Delay(1000);
			//Player 2 has won
			ball.send_coord(705,300);
			ball.send_vel(-5,5);
			score_screen(2);
		}

		if(ball.get_xcoord() == 780)
		{
			SDL_Delay(1000);
			//Player 1 has won
			ball.send_coord(85,300);
			ball.send_vel(5,5);
			score_screen(1);
		}
    }

    //Clean up
    clean_up();

    return 0;
}