/*
*  Compilation trough:
*     gcc -o starfield starfield.c -O2 -lglut -lGL -lGLU -lSDL -lSDL_image
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL/SDL.h"
#include <SDL/SDL_image.h>

#define SCREEN_BPP 16

#define TRUE  1
#define FALSE 0

struct {
    GLfloat rotationZaxis;
    GLfloat speed;
    int spaceDepth;
    unsigned int stars;
    int width, height;
    int fullscreen, movie;
    int fsw, fsh;
    unsigned int fps;
    GLuint galaxy;
} cfg;


struct {
    GLuint texture[4];
    GLfloat *x, *y, *z;
} star;

GLfloat galaxyCoords[4] = {0.0f, 1.0f, 1.0f, 0.0f};
GLfloat galaxySize[2] = {1.0, 1.0};
SDL_Surface *surface;

void quit(int returnCode) {
    SDL_Quit();
    exit(returnCode);
}

void configure(int argc, char **argv)
{
    cfg.width = 768;
    cfg.height = 480;
    cfg.fullscreen = FALSE;
    cfg.movie = FALSE;
    cfg.rotationZaxis = 0.0f;
    cfg.speed = 2.0f;
    cfg.spaceDepth = 1500;
    cfg.stars = 512;
    cfg.fps = 0;
    cfg.galaxy = 0;
    int fwd = FALSE;
    for (int i = 1; i < argc; ++i) {
        if (strstr(argv[i], "speed=") == argv[i])
            cfg.speed = atof(argv[i] + 6);
        else if (strstr(argv[i], "stars=") == argv[i])
            cfg.stars = atof(argv[i] + 6);
        else if (strstr(argv[i], "spaceDepth=") == argv[i])
            cfg.spaceDepth = atoi(argv[i] + 11);
        else if (strstr(argv[i], "width=") == argv[i])
            cfg.width = atoi(argv[i] + 6);
        else if (strstr(argv[i], "height=") == argv[i])
            cfg.height = atoi(argv[i] + 7);
        else if (strstr(argv[i], "fps=") == argv[i])
            cfg.fps = atoi(argv[i] + 4);
        else if (!strcmp(argv[i], "forward"))
            fwd = TRUE;
        else if (!strcmp(argv[i], "backward"))
            fwd = FALSE;
        else if (!strcmp(argv[i], "fullscreen"))
            cfg.fullscreen = TRUE;
        else if (!strcmp(argv[i], "galaxy"))
            cfg.galaxy = 1;
        else if (!strcmp(argv[i], "movie"))
            cfg.movie = TRUE;
        else if (!strcmp(argv[i], "help")) {
            printf("\nSupported parameters and defaults\n---------------------------------\n"
                   "speed=2.0 forward backward : speed and direction\n"
                   "width=768 height=480 fullscreen : window geometry\n"
                   "stars=512 spaceDepth=1500 : how many stars at the same time and deep you can see\n"
                   "fps=0 : fps (0 = unlimited)\n"
                   "galaxy : see the center of the galaxy\n"
                   "movie : random speed fluctuation\n"
                   "keys : print supported controls\n"
                   "help : this help\n\n");
            exit(0);
        } else if (!strcmp(argv[i], "keys")) {
            printf("\nF1 : toggle fullscreen\n"
                   "arrow up/down : faster/slower\n"
                   "arrow left/right : rotation\n"
                   "d : flip direction\n"
                   "escape : quit\n"
                   "space : toggle breaks\n\n");
            exit(0);
        }
    }
    if (fwd)
        cfg.speed = -cfg.speed;
    if (cfg.movie) {
        cfg.fps = 30;
        cfg.fullscreen = TRUE;
    }
    if (cfg.fps)
        cfg.speed *= 60.0f/cfg.fps;
}


int loadTextures() {
    int Status = FALSE;

    SDL_Surface *textureImage;
    char filename[11];
    
    if (cfg.galaxy && (textureImage = IMG_Load("galaxy.jpg"))) {
        galaxySize[0] = textureImage->w;
        galaxySize[1] = textureImage->h;
        glGenTextures(1, &cfg.galaxy);
        glBindTexture(GL_TEXTURE_2D, cfg.galaxy);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     textureImage->w, textureImage->h,
                     0, GL_RGB, GL_UNSIGNED_BYTE, textureImage->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        SDL_FreeSurface(textureImage);
    } else {
        cfg.galaxy = 0; // file failure
    }

//    printf("%s/star_%d.jpg\n", FILEPATH, 1);
    for (int i = 0; i < 4; ++i) {
        sprintf(filename, "star_%d.jpg", i);
        if ((textureImage = IMG_Load(filename))) {
            glGenTextures(1, &star.texture[i]);
            glBindTexture(GL_TEXTURE_2D, star.texture[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         textureImage->w, textureImage->h,
                         0, GL_RGB, GL_UNSIGNED_BYTE, textureImage->pixels);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            SDL_FreeSurface(textureImage);
        } else {
            return FALSE;
        }
    }

    return TRUE;
}

int resizeSky(int width, int height) {
    if (height == 0)
        height = 1;
    if (width == 0)
        width = 1;
    GLfloat gw = galaxySize[0]/width;
    GLfloat gh = galaxySize[1]/height;
    if (gw > gh) {
            float r = (1.0f-gh/gw)/2.0f;
            galaxyCoords[0] = r;
            galaxyCoords[1] = 1.0f - r;
            galaxyCoords[2] = 1.0f;
            galaxyCoords[3] = 0.0;
        } else if (gw < gh) {
            float r = (1.0-gw/gh)/2.0f;
            galaxyCoords[0] = 0.0;
            galaxyCoords[1] = 1.0f;
            galaxyCoords[2] = 1.0f - r;
            galaxyCoords[3] = r;
        }

    GLfloat ratio = (GLfloat)width / (GLfloat)height;
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, cfg.spaceDepth);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return(TRUE);
}

void toggleFullscreen()
{
    int flags = surface->flags;
    int w = cfg.fsw, h= cfg.fsh;
    if (flags & SDL_FULLSCREEN) {
        surface = SDL_SetVideoMode(cfg.width, cfg.height, 0, flags & ~SDL_FULLSCREEN);
        w = cfg.width;
        h = cfg.height;
    } else {
        const SDL_VideoInfo *info = SDL_GetVideoInfo();
        cfg.width = info->current_w;
        cfg.height = info->current_h;
        surface = SDL_SetVideoMode(cfg.fsw, cfg.fsh, 0, flags | SDL_FULLSCREEN);
        if (!surface) {
            surface = SDL_SetVideoMode(cfg.width, cfg.height, 0, flags);
            w = cfg.width;
            h = cfg.height;
        }
    }

    if (!surface)
        quit(1); // bad bad error*/
    resizeSky(w,h);
}



void handleKeyPress(SDL_keysym *keysym)
{
    switch (keysym->sym) {
    case SDLK_UP: cfg.speed *= 2.0f; break;
    case SDLK_DOWN: cfg.speed /= 2.0f; break;
    case SDLK_LEFT: cfg.rotationZaxis += 0.1; break;
    case SDLK_RIGHT: cfg.rotationZaxis -= 0.1; break;
    case SDLK_d: cfg.speed = -cfg.speed; break;
    case SDLK_ESCAPE: quit(0); break;
    case SDLK_F1: toggleFullscreen(); break;
    case SDLK_SPACE: cfg.speed = cfg.speed == 0.0f ? 1.0f : 0.0f; break;
    default: break;
    }
    return;
}

int initGL(GLvoid)
{
    if (!loadTextures())
        return FALSE;
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glDisable(GL_DEPTH_TEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    return TRUE;
}

inline void createStar(int idx, GLfloat zpos)
{
    star.x[idx] = 0.1 * (rand() % (10*cfg.spaceDepth) + 1) - cfg.spaceDepth/2;
    star.y[idx] = 0.1 * (rand() % (10*cfg.spaceDepth) + 1) - cfg.spaceDepth/2;
    if (zpos == 0.0f)
        star.z[idx] = - (0.1 * (rand() % (10*cfg.spaceDepth) + 1));
    else
        star.z[idx] = zpos;
}

void initStars() {
    star.x = malloc(cfg.stars * sizeof(GLfloat));
    star.y = malloc(cfg.stars * sizeof(GLfloat));
    star.z = malloc(cfg.stars * sizeof(GLfloat));
    for (int i = 0; i < cfg.stars; i++) {
       createStar(i, 0.0f);
    }
 
}

float scale = 4.0f;

inline void drawStar() {
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 1.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 1.0f);
      glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 1.0f);
    glEnd();
}

int drawGLScene(GLvoid)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    if (cfg.galaxy) {    
        glPushMatrix();
        glLoadIdentity();
    //    glOrtho(0, 100.0, 0.0, 100.0, 1.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, cfg.galaxy);
    //    glDepthMask(GL_FALSE);
    //    drawStar();
        glBegin(GL_QUADS);
          glTexCoord2f(galaxyCoords[0], galaxyCoords[2]); glVertex3f(-1.0f, -1.0f, 1.0f);
          glTexCoord2f(galaxyCoords[1], galaxyCoords[2]); glVertex3f( 1.0f, -1.0f, 1.0f);
          glTexCoord2f(galaxyCoords[1], galaxyCoords[3]); glVertex3f( 1.0f,  1.0f, 1.0f);
          glTexCoord2f(galaxyCoords[0], galaxyCoords[3]); glVertex3f(-1.0f,  1.0f, 1.0f);
        glEnd();
        glPopMatrix();
    }
    glRotatef(cfg.rotationZaxis, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
//    glTranslatef(-0.0f, -0.0f, -5.0f);
//    glRotatef(cfg.rotationZaxis, 0.0f, 0.0f, 1.0f);


    int i = 0;
    int n = cfg.stars/2;
    for (int t = 0; t < 4; ++t) {
        glBindTexture(GL_TEXTURE_2D, star.texture[t]);
        for (; i < n; ++i) {
            glPushMatrix();
            glTranslatef(star.x[i],star.y[i],star.z[i]);
            drawStar();
            glPopMatrix();

            if (cfg.speed > 0.0f && star.z[i] < -cfg.spaceDepth) {
                createStar(i, star.z[i] + cfg.spaceDepth + 0.5f + cfg.speed);
            } else if (cfg.speed < 0.0f && star.z[i] > -1.0f) {
                createStar(i, star.z[i] - cfg.spaceDepth - 0.5f - cfg.speed);
            } else {
                star.z[i] -= cfg.speed;
            }
        }
        if (t == 0)
            n += cfg.stars/3;
        else if (t == 1)
            n += cfg.stars/8;
        else
            n = cfg.stars;
    }

    SDL_GL_SwapBuffers();

    return(TRUE);
}

int main(int argc, char **argv)
{
    configure(argc, argv);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Video initialization failed: %sn", SDL_GetError());
        quit(1);
    }

    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    if (!videoInfo) {
        fprintf(stderr, "Video query failed: %sn", SDL_GetError());
        quit(1);
    }
    cfg.fsw = videoInfo->current_w;
    cfg.fsh = videoInfo->current_h;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int videoFlags = SDL_OPENGL|SDL_GL_DOUBLEBUFFER|SDL_HWPALETTE|SDL_RESIZABLE;

    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    if (videoInfo->blit_hw)
        videoFlags |= SDL_HWACCEL;

    int width = cfg.width, height = cfg.height;
    if (cfg.fullscreen) {
        videoFlags |= SDL_FULLSCREEN;
        width = videoInfo->current_w;
        height = videoInfo->current_h;
    }

    surface = SDL_SetVideoMode(width, height, SCREEN_BPP, videoFlags);
    if (!surface) {
        fprintf(stderr,  "Video mode set failed: %sn", SDL_GetError());
        quit(1);
    }

    SDL_WM_SetCaption("Starfield", 0);
    initGL();
    initStars();
    resizeSky(width, height);
    SDL_ShowCursor(0);

    SDL_Event event;
    int active = TRUE;
    while (active) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_VIDEORESIZE:
                surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 0,
                                           surface->flags);
                if (!surface) {
                    fprintf(stderr, "Could not get a surface after resize: %sn",
                            SDL_GetError());
                    quit(1);
                }
                resizeSky(event.resize.w, event.resize.h);
                break;
            case SDL_KEYDOWN:
                handleKeyPress(&event.key.keysym);
                break;
            case SDL_QUIT:
                active = FALSE;
                break;
            default:
                break;
            }
        }
        if (cfg.movie) {
            int r = rand() % 100;
            if (!r) {
                r = rand() % 2;
                switch (r) {
                    case 0: cfg.speed *= 2.0f; break;
                    case 1: cfg.speed /= 2.0f; break;
                    case 2: cfg.rotationZaxis += 0.1; break;
                    case 3: cfg.rotationZaxis -= 0.1; break;
                    default: break;
                }
            }
        }
        drawGLScene();
        if (cfg.fps)
            SDL_Delay(1000/cfg.fps);
    }

    quit(0);

    return(0);
}
