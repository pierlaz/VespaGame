#include <math.h>
#include <iostream>
#include <string>

using namespace std;

#include <GL/glut.h>

#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else

#include <SDL2/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <sstream>
#include <fstream>

#endif

#include "vespa/vespa.h"
#include "mesh/mesh.h"
#include "text/GLText.h"


#define CAMERA_BACK_CAR 3
#define CAMERA_TOP_CAR 1
#define CAMERA_PILOT 2
#define CAMERA_MOUSE 0
#define CAMERA_TYPE_MAX 4

#define FINE_STRADA 10
#define SIZE_FLOOR 250
#define K_FLOOR 200

#define ZROCCIA 215

//vari font
TTF_Font *font;
TTF_Font *font26;
TTF_Font *font20;
TTF_Font *font17;

GLText *scoreDynamic = new GLText(32);
GLText *textMenu = new GLText(18);

SDL_Color colorRed = {255, 0, 0, 0};
SDL_Color colorBlack = {0, 0, 0, 0};
SDL_Color colorWhite = {255, 255, 255, 255};

float viewAlpha = 0, viewBeta = 20; // angoli che definiscono la vista
float eyeDist = 5.0; // distanza dell'occhio dall'origine
int scrH = 750, scrW = 750; // altezza e larghezza viewport (in pixels)
bool useWireframe = false;
bool useRadar = false;
bool useHeadlight = false;
bool useShadow = true;
bool useSabbiaTempesta = false;

double record = 1000.0; //record letto da file inizializzato a un valore altissimo

//array di mesh
Mesh rocce[100];

extern void drawLineaStradale();

extern void drawPalloncini();

extern Mesh drawRoccia();

extern void drawQuestionMarks();

extern void drawBillboard();

extern void drawBillboardPoster();

int cameraType = 0;
Vespa vespa; // la nostra macchina

int nstep = 0; // numero di passi di FISICA fatti fin'ora
const int PHYS_SAMPLING_STEP = 10; // numero di millisec che un passo di fisica simula

// Frames Per Seconds
const int fpsSampling = 3000; // lunghezza intervallo di calcolo fps
float fps = 0; // valore di fps dell'intervallo precedente
int fpsNow = 0; // quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale
Uint32 timeLastInterval = 0; // quando e' cominciato l'ultimo intervallo
double tempoPercorso = 0.0; //tempoPercorso finali per il time-attack


// setta le matrici di trasformazione in modo che le coordinate in spazio oggetto siano le coord del pixel sullo schemo
void SetCoordToPixel() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1, -1, 0);
    glScalef(2.0 / scrW, 2.0 / scrH, 1);
}

// funzione che carica la testure
bool LoadTexture(int textbind, char *filename) {
    GLenum texture_format;

    SDL_Surface *s = IMG_Load(filename);
    if (!s) return false;

    if (s->format->BytesPerPixel == 4) {     // contiene canale alpha
        if (s->format->Rmask == 0x000000ff) {
            texture_format = GL_RGBA;
        } else {
            texture_format = GL_BGRA;
        }
    } else if (s->format->BytesPerPixel == 3) {     // non contiene canale alpha
        if (s->format->Rmask == 0x000000ff)
            texture_format = GL_RGB;
        else
            texture_format = GL_BGR;
    } else {
        printf("[ERROR] the image is not truecolor\n");
        exit(1);
    }

    glBindTexture(GL_TEXTURE_2D, textbind);
    gluBuild2DMipmaps(
            GL_TEXTURE_2D,
            3,
            s->w, s->h,
            texture_format,
            GL_UNSIGNED_BYTE,
            s->pixels
    );
    return true;
}

// funzione che disegna lo skybox
void drawSkybox(float size) {
    glPushMatrix();
    bool b1 = glIsEnabled(GL_TEXTURE_2D); //new, we left the textures turned on, if it was turned on
    glDisable(GL_LIGHTING);               //turn off lighting, when making the skybox
    glDisable(GL_DEPTH_TEST);             //turn off depth texting
    glEnable(GL_TEXTURE_2D);              //and turn on texturing
    glBindTexture(GL_TEXTURE_2D, 20);      //use the texture we want
    glBegin(GL_QUADS);                    //and draw a face
    //back face
    glTexCoord2f(0, 0);                       //use the correct texture coordinate
    glVertex3f(size / 2, size / 2, size / 2); //and a vertex
    glTexCoord2f(1, 0);                       //and repeat it...
    glVertex3f(-size / 2, size / 2, size / 2);
    glTexCoord2f(1, 1);
    glVertex3f(-size / 2, -size / 2, size / 2);
    glTexCoord2f(0, 1);
    glVertex3f(size / 2, -size / 2, size / 2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 21);
    glBegin(GL_QUADS);
    //left face
    glTexCoord2f(0, 0);
    glVertex3f(-size / 2, size / 2, size / 2);
    glTexCoord2f(1, 0);
    glVertex3f(-size / 2, size / 2, -size / 2);
    glTexCoord2f(1, 1);
    glVertex3f(-size / 2, -size / 2, -size / 2);
    glTexCoord2f(0, 1);
    glVertex3f(-size / 2, -size / 2, size / 2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 23);
    glBegin(GL_QUADS);
    //front face
    glTexCoord2f(1, 0);
    glVertex3f(size / 2, size / 2, -size / 2);
    glTexCoord2f(0, 0);
    glVertex3f(-size / 2, size / 2, -size / 2);
    glTexCoord2f(0, 1);
    glVertex3f(-size / 2, -size / 2, -size / 2);
    glTexCoord2f(1, 1);
    glVertex3f(size / 2, -size / 2, -size / 2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 22);
    glBegin(GL_QUADS);
    //right face
    glTexCoord2f(0, 0);
    glVertex3f(size / 2, size / 2, -size / 2);
    glTexCoord2f(1, 0);
    glVertex3f(size / 2, size / 2, size / 2);
    glTexCoord2f(1, 1);
    glVertex3f(size / 2, -size / 2, size / 2);
    glTexCoord2f(0, 1);
    glVertex3f(size / 2, -size / 2, -size / 2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 24);
    glBegin(GL_QUADS); //top face
    glTexCoord2f(1, 0);
    glVertex3f(size / 2, size / 2, size / 2);
    glTexCoord2f(0, 0);
    glVertex3f(-size / 2, size / 2, size / 2);
    glTexCoord2f(0, 1);
    glVertex3f(-size / 2, size / 2, -size / 2);
    glTexCoord2f(1, 1);
    glVertex3f(size / 2, size / 2, -size / 2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 25);
    glBegin(GL_QUADS);
    //bottom face
    glTexCoord2f(1, 1);
    glVertex3f(size / 2, -size / 2, size / 2);
    glTexCoord2f(0, 1);
    glVertex3f(-size / 2, -size / 2, size / 2);
    glTexCoord2f(0, 0);
    glVertex3f(-size / 2, -size / 2, -size / 2);
    glTexCoord2f(1, 0);
    glVertex3f(size / 2, -size / 2, -size / 2);
    glEnd();
    glEnable(GL_LIGHTING); //turn everything back, which we turned on, and turn everything off, which we have turned on.
    glEnable(GL_DEPTH_TEST);
    if (!b1)
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

// carica texture per skybox
bool loadTextureSky(int textbind, char *filename) {
    GLenum texture_format;

    SDL_Surface *s = IMG_Load(filename);
    if (!s)
        return false;

    if (s->format->BytesPerPixel == 4) { // contiene canale alpha
        if (s->format->Rmask == 0x000000ff) {
            texture_format = GL_RGBA;
        } else {
            texture_format = GL_BGRA;
        }
    } else if (s->format->BytesPerPixel == 3) { // non contiene canale alpha
        if (s->format->Rmask == 0x000000ff)
            texture_format = GL_RGB;
        else
            texture_format = GL_BGR;
    } else {
        printf("[ERROR] the image is not truecolor\n");
        exit(1);
    }

    glBindTexture(GL_TEXTURE_2D, textbind);
    gluBuild2DMipmaps(
            GL_TEXTURE_2D,
            3,
            s->w, s->h,
            texture_format,
            GL_UNSIGNED_BYTE,
            s->pixels);
    glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR);
    glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
    SDL_FreeSurface(s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);    //if the texture is smaller, than the image, we get the avarege of the pixels next to it
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    //same if the image bigger
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE); //we repeat the pixels in the edge of the texture, it will hide that 1px wide line at the edge of the cube, which you have seen in the video
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return true;
}

// disegno la strada
void drawStrada() {
    if (!useWireframe) {
        const float S = SIZE_FLOOR;
        const float H = 0;
        const int K = K_FLOOR;
        // disegno il terreno ripetendo una texture su di esso
        glBindTexture(GL_TEXTURE_2D, 6);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        // disegna KxK quads
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        for (int x = 0; x < K; x++)
            for (int z = 0; z < K; z++) {
                float x0 = -S + 2 * (x + 0) * S / K;
                float x1 = -S + 2 * (x + 1) * S / K;
                float z0 = -S + 2 * (z + 0) * S / K;
                float z1 = -S + 2 * (z + 1) * S / K;
                // disegno solo i quadrati relativi alla strada
                if ((x0 <= FINE_STRADA) && (x0 >= -FINE_STRADA) && ((z0 < -240) || (z0 > -238))) {
                    glTexCoord2f(0.0, 0.0);
                    glVertex3d(x0, H, z0);
                    glTexCoord2f(1.0, 0.0);
                    glVertex3d(x1, H, z0);
                    glTexCoord2f(1.0, 1.0);
                    glVertex3d(x1, H, z1);
                    glTexCoord2f(0.0, 1.0);
                    glVertex3d(x0, H, z1);
                }
            }
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    return;
}


// disegno l'asfalto in prossimità dell'arrivo con una scacchiera texture
void drawArrivoTexture() {
    const float S = SIZE_FLOOR;
    const float H = 0;
    const int K = K_FLOOR;

    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, 8);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    for (int x = 50; x <= 150; x++)
        for (int z = 0; z <= 8; z++) {
            float x0 = -S + 2 * (x + 0) * S / K;
            float x1 = -S + 2 * (x + 1) * S / K;
            float z0 = -S + 2 * (z + 0) * S / K;
            float z1 = -S + 2 * (z + 1) * S / K;
            // sono in prossimità di fine pista texturo con la bandiera a scacchi
            if ((x0 <= FINE_STRADA) && (x0 >= -FINE_STRADA) && (z0 >= -240) && (z0 <= -238)) {
                glTexCoord2f(0.0, 0.0);
                glVertex3d(x0, H, z0);
                glTexCoord2f(1.0, 0.0);
                glVertex3d(x1, H, z0);
                glTexCoord2f(1.0, 1.0);
                glVertex3d(x1, H, z1);
                glTexCoord2f(0.0, 1.0);
                glVertex3d(x0, H, z1);
            }
        }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    return;
}

// disegno terreno texturato dove non ho strada
void drawFuoriStrada() {
    if (!useWireframe) {
        const float S = SIZE_FLOOR;
        const float H = 0;
        const int K = K_FLOOR;

        glBindTexture(GL_TEXTURE_2D, 7);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        for (int x = 0; x < K; x++)
            for (int z = 0; z < K; z++) {
                float x0 = -S + 2 * (x + 0) * S / K;
                float x1 = -S + 2 * (x + 1) * S / K;
                float z0 = -S + 2 * (z + 0) * S / K;
                float z1 = -S + 2 * (z + 1) * S / K;
                if ((x0 > FINE_STRADA) || (x0 < -FINE_STRADA)) {
                    glTexCoord2f(0.0, 0.0);
                    glVertex3d(x0, H, z0);
                    glTexCoord2f(1.0, 0.0);
                    glVertex3d(x1, H, z0);
                    glTexCoord2f(1.0, 1.0);
                    glVertex3d(x1, H, z1);
                    glTexCoord2f(0.0, 1.0);
                    glVertex3d(x0, H, z1);
                }
            }
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
}


// setto la posizione della camera
double *setCameraPreSky() {
    double px = vespa.px;
    double py = vespa.py;
    double pz = vespa.pz;
    double angle = vespa.facing;
    float camH = 0.7;
    glTranslatef(0, camH, -eyeDist);
    glRotatef(viewBeta, 1, 0, 0);
    glRotatef(-angle, 0, 1, 0);

    //rotazione camera asse z
    if (vespa.controller.key[Controller::CAMERA_ROT_DX]) {
        viewAlpha -= 2;
        glRotatef(viewAlpha, 0, 1, 0);
    } else {
        glRotatef(viewAlpha, 0, 1, 0);
    }
    if (vespa.controller.key[Controller::CAMERA_ROT_SX]) {
        viewAlpha += 2;
        glRotatef(viewAlpha, 0, 1, 0);
    } else {
        glRotatef(viewAlpha, 0, 1, 0);
    }

    //ripristino telecamera standard
    if (vespa.controller.key[Controller::CAMERA_RESET]) {
        viewAlpha = 0;
        viewBeta = 20;
        eyeDist = 5;
    }

    static double p[3];
    p[0] = px;
    p[1] = py;
    p[2] = pz;
    return p;
}

// setto camera iniziale
void setCameraPostSky(double px, double py, double pz) {
    glTranslatef(-px, -py - 1.5, -pz);

    //TODO vedere se mettere le altre camere (con rotazione da vedere)
    /* switch (cameraType) {
         case CAMERA_MOUSE:
             glTranslatef(-px, -py - 1.5, -pz);
             break;
         case CAMERA_BACK_CAR:
             glTranslatef(-px, -py - 1.5, -pz - 3);
             break;
         case CAMERA_PILOT:
             glTranslatef(-px, -py , -pz+3);
             break;
         case CAMERA_TOP_CAR:
             glTranslatef(-px+30, -py , -pz);
             break;
     }*/

}

// simulo una tempesta di sabbia
void tempestaSabbia() {
    GLfloat fogColor[4] = {0.6, 0.6, 0.4, 1.0};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 0.0);
    glFogf(GL_FOG_END, 14.0);
    glClearColor(0.5, 0.5, 0.5, 1.0);
}

/* //TODO vedere se eliminare, penso di si
// setto la posizione della camera
void setCamera() {

    double px = vespa.px;
    double py = vespa.py;
    double pz = vespa.pz;
    double angle = vespa.facing;
    double cosf = cos(angle * M_PI / 180.0);
    double sinf = sin(angle * M_PI / 180.0);
    double camd, camh, ex, ey, ez, cx, cy, cz;
    double cosff, sinff;

// controllo la posizione della camera a seconda dell'opzione selezionata
    switch (cameraType) {
        case CAMERA_BACK_CAR:
            camd = 4.5;
            camh = 2;
            ex = px + camd * sinf;
            ey = py + camh;
            ez = pz + camd * cosf;
            cx = px - camd * sinf;
            cy = py + camh;
            cz = pz - camd * cosf;
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_TOP_FIXED:
            camd = 0.5;
            camh = 0.55;
            angle = vespa.facing + 40.0;
            cosff = cos(angle * M_PI / 180.0);
            sinff = sin(angle * M_PI / 180.0);
            ex = px + camd * sinff;
            ey = py + camh;
            ez = pz + camd * cosff;
            cx = px - camd * sinf;
            cy = py + camh;
            cz = pz - camd * cosf;
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_TOP_CAR:
            camd = 2.5;
            camh = 1.0;
            ex = px + camd * sinf;
            ey = py + camh;
            ez = pz + camd * cosf;
            cx = px - camd * sinf;
            cy = py + camh;
            cz = pz - camd * cosf;
            gluLookAt(ex, ey + 5, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_PILOT:
            camd = 0.2;
            camh = 0.55;
            ex = px + camd * sinf;
            ey = py + camh;
            ez = pz + camd * cosf;
            cx = px - camd * sinf;
            cy = py + camh;
            cz = pz - camd * cosf;
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_MOUSE:
            glTranslatef(0, 0, -eyeDist);
            glRotatef(viewBeta, 1, 0, 0);
            glRotatef(viewAlpha, 0, 1, 0);

            break;
    }
}*/

// disegno la bandiera a scacchi nel radar
void drawFinishFlagRadar() {
    float color = 0;
    for (int x = 25; x <= 225; x += 5) {
        glColor3f(color, color, color);
        glBegin(GL_POLYGON);
        glVertex2d(x, SIZE_FLOOR - 10 + 15);
        glVertex2d(x, SIZE_FLOOR - 5 + 15);
        glVertex2d(x - 5, SIZE_FLOOR - 5 + 15);
        glVertex2d(x - 5, SIZE_FLOOR - 10 + 15);
        glEnd();
        if (color == 0)
            color = 1;
        else
            color = 0;
    }
}

// disegno il Radar in basso a sx
void drawRadar() {
    int x0 = 0, x1 = SIZE_FLOOR;
    int y0 = 0, y1 = SIZE_FLOOR + 20;
    glPushMatrix();

    //Indicatore Vespa
    glColor3f(1.0, 0, 0);
    glBegin(GL_TRIANGLES);
    int xVespa = 0;
    //radar scalato
    int posizioneFuoriStrada = (18 * vespa.px) + x1;
    if (vespa.px < 4.5 + FINE_STRADA && vespa.px > -2 - FINE_STRADA) {
        xVespa = (18 * vespa.px) + x1 - 20;
    } else {
        xVespa = (posizioneFuoriStrada - x1) + x1;
        if (xVespa > SIZE_FLOOR / 2) {
            xVespa = SIZE_FLOOR * 2;
        } else {
            xVespa = 0;
        }
    }
    int zVespa = -vespa.pz + y1;
    float xM = xVespa / 2; //SPAZIO MAPPA
    float zM = zVespa / 2;
    glVertex2f(xM - 6, zM);
    glVertex2f(xM + 6, zM);
    glVertex2f(xM, zM + 10);
    glEnd();

    drawFinishFlagRadar();

    //DISEGNO STRADA
    glColor3f(0.7, 0.7, 0.7);
    glBegin(GL_QUADS);
    glVertex2d(SIZE_FLOOR / 2 - FINE_STRADA - 97, y0);
    glVertex2d(SIZE_FLOOR / 2 + FINE_STRADA + 90, y0);
    glVertex2d(SIZE_FLOOR / 2 + FINE_STRADA + 90, y1);
    glVertex2d(SIZE_FLOOR / 2 - FINE_STRADA - 97, y1);
    glRotatef(90, 1, 0, 0);
    glColor3f(1, 1, 1); //reset color

    //SFONDO MAPPA
    glColor3f(0.8, 0.5, 0);
    glBegin(GL_QUADS);
    glVertex2d(x0, y0);
    glVertex2d(x1, y0);
    glVertex2d(x1, y1);
    glVertex2d(x0, y1);
    glEnd();
    glRotatef(90, 1, 0, 0);
    glColor3f(1, 1, 1); //reset color
    glPopMatrix();
}

//funzione che disegna le rocce
void drawRocce() {
    int counter = 0;
    Mesh rocciaTemp;
    for (int i = 0; i < 440; i += 23) {
        glPushMatrix();
        glTranslatef(1, 0, ZROCCIA - i);
        rocciaTemp = drawRoccia();
        rocce[counter++] = rocciaTemp;
        glPopMatrix();
        int sign = -1;
        if (i % 2 == 0) {
            sign = 1;
        }
        glPushMatrix();
        glTranslatef(sign * 3.5, 0, ZROCCIA - i);
        rocciaTemp = drawRoccia();
        rocce[counter++] = rocciaTemp;
        glPopMatrix();
        glPushMatrix();
        glTranslatef(sign * 6, 0, ZROCCIA - i);
        rocciaTemp = drawRoccia();
        rocce[counter++] = rocciaTemp;
        glPopMatrix();
        glPushMatrix();
        glTranslatef(sign * 8.5, 0, ZROCCIA - i);
        rocciaTemp = drawRoccia();
        rocce[counter++] = rocciaTemp;
        glPopMatrix();
        glPushMatrix();
        glTranslatef(11, 0, ZROCCIA - i);
        rocciaTemp = drawRoccia();
        rocce[counter++] = rocciaTemp;
        glPopMatrix();
    }
    for (int i = 20; i < 440; i += 11) {
        glPushMatrix();
        glTranslatef(-1, 0, ZROCCIA - i);
        drawRoccia();
        glPopMatrix();
    }

}

// render testo 2d
void RenderText(std::string message, SDL_Color color, int x, int y, TTF_Font *font) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(0, scrW, 0, scrH);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    SDL_Surface *sFont;
    sFont = TTF_RenderText_Blended(font, message.c_str(), color);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, sFont->pixels);

    glBegin(GL_QUADS);
    {
        glTexCoord2f(0, 1);
        glVertex2f(x, y);
        glTexCoord2f(1, 1);
        glVertex2f(x + sFont->w, y);
        glTexCoord2f(1, 0);
        glVertex2f(x + sFont->w, y + sFont->h);
        glTexCoord2f(0, 0);
        glVertex2f(x, y + sFont->h);
    }
    glEnd();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // valore di default di GL_TEXTURE:ENV:MODE
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(sFont);
}

// render della schermata iniziale
void renderingPreGame(SDL_Window *win) {

    int w = scrW, h = scrH;
    fpsNow++;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClearColor(0.9, 0.6, 0.2, 0.5);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    RenderText("\"Vespa on the Road! - Time attack edition\"", colorBlack, w * 1 / 20 + 10, h - 70, font);
    string regole[7] = {
            "OBIETTIVO DEL GIOCO",
            "Il gioco consiste nel raggiungere il traguardo alla fine della strada nel minor tempo possibile.",
            "Vi sono presenti degli ostacoli da evitare (rocce) che altrimenti comportano la fine del gioco.",
            "Vi sono presenti 3 blocchi con i punti interrogativi che possono:",
            "    1) Invertire il comando dello sterzo (A diventa D e viceversa)",
            "    2) Dimezzare la velocita'",
            "    3) Raddoppiare la velocita'"
    };
    int paddingTop = 180;

    for (int i = 0; i < 7; i++) {
        if (i == 0) {
            RenderText(regole[i], colorBlack, w * 1 / 3, h - paddingTop + 20, font20);
        } else {
            RenderText(regole[i], colorBlack, w * 1 / 30, h - paddingTop, font17);
        }
        paddingTop += 25;
    }
    paddingTop += 30;

    RenderText("Buon divertimento e batti il record!", colorBlack, w * 1 / 4, h - paddingTop, font20);
    //testo sui comandi del gioco
    string instr[10] = {
            "COMANDI DI GIOCO",
            "W   =  Accelerazione",
            "S   =  Freno/Retromarcia",
            "D   =  Destra",
            "S   =  Sinistra",
            "<-  =  Ruota telecamera in senso orario",
            "->  =  Ruota telecamera in senso antiorario",
            "T   =  Reset della telecamera",
            "R   =  Ricominciare il gioco",
            "ESC =  Esci"};
    paddingTop = 460;
    for (int i = 0; i < 10; i++) {
        if (i == 0) {
            RenderText(instr[i], colorBlack, w * 1 / 30, h - paddingTop + 20, font20);
        } else {
            RenderText(instr[i], colorBlack, w * 1 / 30, h - paddingTop, font17);
        }

        paddingTop += 25;
    }
    paddingTop += 40;

    RenderText("Premere un tasto qualsiasi per iniziare a giocare!", colorBlack, w * 1 / 8, h - paddingTop, font20);
    glFinish();
    vespa.controller.startTimeCounting(); //il cronometro inizia dopo la chiusura della schermata iniziale

    SDL_GL_SwapWindow(win);
}


//workaround per tostring (non funzionava altrimenti)
template<typename T>
std::string to_string(T value) {
    //create an output string stream
    std::ostringstream os;

    //throw the value into the string stream
    os << value;

    //convert the string stream into a string and return
    return os.str();
}

// funzione che permette di leggere il record da file
double getRecordDaFile() {
    std::ifstream ifile("record.txt", std::ios::in);
    std::vector<double> records;


    if (!ifile.is_open()) {
        std::cerr << "Errore nell'apertura del file dei record!\n";
        exit(1);
    }
    double num = 0.0;

    while (ifile >> num) {
        records.push_back(num);
    }
    ifile.close();
    return records[0];
}

// funzione che salva il record, se cambiato, su file
void salvaRecordSuFile(double record) {
    ofstream myfile("record.txt");
    if (myfile.is_open()) {
        myfile << record;
        myfile.close();
    }
}


//funzione che mostra in alto il tempo dell'attuale corsa e il record da battere
void displayTime() {
    SDL_Color colorBlack = {0, 0, 0, 0};
    string tempoAttuale = "Time: ";
    tempoAttuale.append(to_string(vespa.controller.getSeconds()));
    RenderText(tempoAttuale, colorBlack, 50, scrH - 60, font);

    double record = getRecordDaFile();
    string recordString = "Record: ";
    recordString.append(to_string(record));
    RenderText(recordString, colorBlack, scrW - 300, scrH - 60, font);
}

//funzione che mostra in basso l'esito dell'impatto con il punto interrogativo
void displayEsitoPuntoInterrogativo() {
    if (vespa.comandiInvertiti) {
        RenderText("Sterzo invertito!", colorRed, scrW / 4, scrH / 20, font);
    } else if (vespa.aumentaAcc) {
        RenderText("La velocita' e' raddoppiata!", colorRed, scrW / 4, scrH / 20, font);

    } else if (vespa.diminuisciAcc) {
        RenderText("La velocita' e' dimezzata!", colorRed, scrW / 4, scrH / 20, font);

    }
}


void rendering(SDL_Window *win) {

    fpsNow++;
    glLineWidth(3); // linee larghe

    // settiamo il viewport
    glViewport(0, 0, scrW, scrH);

    // colore sfondo = bianco
    glClearColor(1, 1, 1, 1);

    // settiamo la matrice di proiezione
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70,                   //fovy,
                   ((float) scrW) / scrH, //aspect Y/X,
                   0.2,                  //distanza del NEAR CLIPPING PLANE in coordinate vista
                   1000                  //distanza del FAR CLIPPING PLANE in coordinate vista
    );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // riempe tutto lo screen buffer di pixel color sfondo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    double *p;
    p = setCameraPreSky();
    drawSkybox(20);
    static float tmpcol[4] = {1, 1, 1, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmpcol);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 127);
    glEnable(GL_LIGHTING);
    setCameraPostSky(p[0], p[1], p[2]);

    // setto la posizione luce in alto (mezzogiorno)
    float tmpv[4] = {0, 10, 0, 0}; // ultima comp=0 => luce direzionale
    glLightfv(GL_LIGHT0, GL_POSITION, tmpv);

    drawBillboardPoster();
    drawStrada();
    drawFuoriStrada();
    drawLineaStradale();
    drawArrivoTexture();
    drawPalloncini();
    drawRocce();
    drawQuestionMarks();
    drawBillboard();

    //tempesta di sabbia, effetto carino
    if (useSabbiaTempesta) {
        tempestaSabbia();
        glEnable(GL_FOG);

    } else {
        glDisable(GL_FOG);

    }
    vespa.Render();
    if (useRadar) {
        SetCoordToPixel();
        drawRadar();

    }
    //mostra tempo attuale e record
    displayTime();

    //mostra cosa è accaduto se si è colpito il punto interrogativo
    displayEsitoPuntoInterrogativo();

    SDL_GL_SwapWindow(win);
}

//TODO
void renderingGameOver(SDL_Window *win) {
    int w = scrW, h = scrH;
    fpsNow++;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    string instr[6] = {
            "GAME OVER!",
            "Hai perso perche' hai colpito una roccia!",
            " ",
            " ",
            "Premi 'R' per ricominciare il gioco. ",
            "Premi 'ESC' per uscire ..."
    };
    int paddingTop = 100;

    for (int i = 0; i < 6; i++) {
        RenderText(instr[i], colorWhite, w * 1 / 15, h - paddingTop + 20, font);
        //aumento padding prima dell'ultima frase
        if (i == 5) {
            paddingTop += 250;
        } else {
            paddingTop += 50;
        }
    }

    glFinish();

    SDL_GL_SwapWindow(win);
}


//TODO
void renderingFineStrada(SDL_Window *win) {

    int w = scrW, h = scrH;
    fpsNow++;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClearColor(0.8, 1, 0.8, 0.8);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    RenderText("\"Vespa on the Road! - Time attack edition\"", colorBlack, w * 1 / 20 + 10, h - 70, font);

    //testo sul record del gioco
    string instr[5] = {
            "Complimenti! Hai completato il percorso in: ",
            "",
            "",
            "Premi 'R' per ricominciare il gioco. ",
            "Premere 'ESC' per uscire..."};

    if (tempoPercorso ==
        0.0) { //controllo per avvalorare la variabile solo 1 volta, ovvero appena la vespa taglia il traguardo
        tempoPercorso = vespa.controller.getSeconds();
    }
    instr[0].append(to_string(tempoPercorso));
    instr[0].append(" secondi");

    //se vero hai stabilito un nuovo record
    if (tempoPercorso <= record) {
        instr[1].append("Hai stabilito un nuovo record!");
        instr[2].append("Il record precedente era di: ");
        instr[2].append(to_string(record));
        instr[2].append(" secondi");
        salvaRecordSuFile(tempoPercorso);
    } else {
        instr[1].append("Il record del percorso e': ");
        instr[1].append(to_string(record));
        instr[1].append(" secondi");
        instr[2].append("Gioca nuovamente per cercare di battere il record!");
    }

    int paddingTop = 300;
    for (int i = 0; i < 5; i++) {

        RenderText(instr[i], colorBlack, w * 1 / 15, h - paddingTop + 20, font26);
        paddingTop += 80;
    }

    SDL_GL_SwapWindow(win);
}

// funzione che gestisce le dinamiche di gioco
int gameplay() {
    srand(time(NULL)); //seme che cambia per il random fatto bene

    float limiteRocciaSx = -1.2; //estremo sx di una roccia qualsiasi
    float limiteRocciaDx = 1.2; //estremo dx di una roccia qualsiasi
    float centroRocce[5] = {1, 3.5, 6, 8.5,
                            11}; //array contenente il centro di ognuna delle 5 rocce che si trovano sulla stessa linea

    if ((-vespa.pz >= SIZE_FLOOR - 10) && (abs(vespa.px) < FINE_STRADA + 1)) {
        //arrivo al traguardo
        return 1;
    } else {
        for (int i = 0; i < 440; i += 23) {
            int sign = -1;

            if (static_cast<int>(vespa.pz) == ZROCCIA - i &&
                ((vespa.px > centroRocce[0] + limiteRocciaSx && vespa.px < centroRocce[0] + limiteRocciaDx))) {
                return 2; //gameover per impatto con roccia
            }
            if (static_cast<int>(vespa.pz) == ZROCCIA - i &&
                ((vespa.px > centroRocce[4] + limiteRocciaSx && vespa.px < centroRocce[4] + limiteRocciaDx))) {
                return 2; //gameover per impatto con roccia
            }
            for (int j = 1; j < 4; j++) {
                if (i % 2 == 0) {
                    sign = 1;
                }
                // printf("%f vespa\n", (vespa.px));
                if (static_cast<int>(vespa.pz) == ZROCCIA - i && ((vespa.px > sign * centroRocce[j] + limiteRocciaSx &&
                                                                   vespa.px <
                                                                   sign * centroRocce[j] + limiteRocciaDx))) {
                    return 2; //gameover per impatto con roccia
                }
            }
        }
        for (int i = 20; i < 440; i += 11) {
            // printf("%f vespa\n", (vespa.px));
            if (static_cast<int>(vespa.pz) == ZROCCIA - i &&
                ((vespa.px) > -1 + limiteRocciaSx && vespa.px < -1 + limiteRocciaDx)) {
                return 2; //gameover per impatto con roccia
            }
        }


        int random = 0;

        string esitoPuntoInterrogativo[3] = {"Comandi sterzo invertiti!", "Velocità dimezzata!",
                                             "Velocità raddoppiata!"};

        //controllo punto interrogativo 1
        if ((vespa.px >= 2.6 && vespa.px <= 5) && (vespa.pz <= 147.2 && vespa.pz >= 146.5)) {
            glEnable(GL_LIGHTING);
            random = rand() % 3; //numero da 0 a 2
            switch (random) {

                case 0:
                    vespa.invertiComandi(); //inverte sterzo
                    RenderText(esitoPuntoInterrogativo[random], colorRed, 50, scrH - 160, font);
                    break;
                case 1:
                    vespa.diminuisciAccelerazione(); //diminuisce velocità
                    RenderText(esitoPuntoInterrogativo[random], colorRed, 50, scrH - 160, font);
                    break;
                case 2:
                    vespa.aumentaAccelerazione(); //raddoppia velocità
                    RenderText(esitoPuntoInterrogativo[random], colorRed, 50, scrH - 160, font);
                    break;
            }
        }
        //controllo punto interrogativo 2
        if (((vespa.px >= 2.6 && vespa.px <= 5) &&
             (vespa.pz >= 52.8 && vespa.pz <= 53.3))) {// initialize random seed:
            random = rand() % 3; //numero da 0 a 2
            switch (random) {
                case 0:
                    vespa.invertiComandi(); //inverte sterzo
                    break;
                case 1:
                    vespa.diminuisciAccelerazione(); //diminuisce velocità
                    break;
                case 2:
                    vespa.aumentaAccelerazione(); //raddoppia velocità
            }
        }
        //controllo punto interrogativo 3
        if (((vespa.px <= -2.6 && vespa.px >= -5) && (vespa.pz >= -23.4 && vespa.pz <= -22.5))) {
            random = rand() % 3; //numero da 0 a 2
            switch (random) {
                case 0:
                    vespa.invertiComandi(); //inverte sterzo
                    break;
                case 1:
                    vespa.diminuisciAccelerazione(); //diminuisce velocità
                    break;
                case 2:
                    vespa.aumentaAccelerazione(); //raddoppia velocità
            }
        }

        return -1;
    }
}

int main(int argc, char *argv[]) {
    static int schermata = 0;

    SDL_Window *win;
    SDL_GLContext mainContext;
    Uint32 windowID;
    static int keymap[Controller::NKEYS] = {SDLK_a, SDLK_d, SDLK_w, SDLK_s, SDLK_RIGHT, SDLK_LEFT, SDLK_t};

    // inizializzazione di SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // facciamo una finestra di scrW x scrH pixels
    win = SDL_CreateWindow(argv[0], 0, 0, scrW, scrH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    //Create our opengl context and attach it to our window
    mainContext = SDL_GL_CreateContext(win);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // rinormalizza le normali (non viene fatto in automatico da opengl)
    glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 1); // indietro di 1

    //caricamento font
    font = TTF_OpenFont("FreeSans.ttf", 35);
    font26 = TTF_OpenFont("FreeSans.ttf", 26);
    font20 = TTF_OpenFont("FreeSans.ttf", 20);
    font17 = TTF_OpenFont("FreeSans.ttf", 17);


    //Caricamento delle texture
    if (!LoadTexture(0, (char *) "images/gomma.jpg")) return 0;
    if (!LoadTexture(1, (char *) "images/vespa.jpg")) return 0;
    if (!LoadTexture(2, (char *) "images/sky_ok.jpg")) return -1;
    if (!LoadTexture(3, (char *) "images/lightF.jpg")) return 0;
    if (!LoadTexture(4, (char *) "images/sella.jpg")) return 0;
    if (!LoadTexture(5, (char *) "images/copri.jpg")) return 0;
    if (!LoadTexture(6, (char *) "images/asfalto.jpg")) return 0;
    if (!LoadTexture(7, (char *) "images/sand.jpg")) return 0;
    if (!LoadTexture(8, (char *) "images/racing_flag.jpg")) return 0;
    if (!LoadTexture(9, (char *) "images/racing_flag_baloon.jpg")) return 0;
    if (!LoadTexture(10, (char *) "images/rock.jpg")) return 0;
    if (!LoadTexture(11, (char *) "images/question.jpg")) return 0;
    if (!LoadTexture(12, (char *) "images/billboard.jpg")) return 0;
    if (!LoadTexture(13, (char *) "images/miafoto.jpg")) return 0;


    loadTextureSky(20, (char *) "images/skybox/back.png");
    loadTextureSky(22, (char *) "images/skybox/left.png");
    loadTextureSky(23, (char *) "images/skybox/front.png");
    loadTextureSky(21, (char *) "images/skybox/right.png");
    loadTextureSky(24, (char *) "images/skybox/top.png");
    loadTextureSky(25, (char *) "images/skybox/bottom.png");

    record = getRecordDaFile();

    bool done = 0;
    bool preGame = 1;
    while (!done) {

        SDL_Event e;

        // guardo se c'e' un evento:
        if (SDL_PollEvent(&e)) {
            // se si: processa evento
            switch (e.type) {
                case SDL_KEYDOWN:
                    vespa.controller.EatKey(e.key.keysym.sym, keymap, true);
                    if (e.key.keysym.sym == SDLK_F1) cameraType = (cameraType + 1) % CAMERA_TYPE_MAX; //TODO vedere se mantenere le camere
                    if (e.key.keysym.sym == SDLK_F2) useWireframe = !useWireframe;
                    if (e.key.keysym.sym == SDLK_F3) useRadar = !useRadar;
                    if (e.key.keysym.sym == SDLK_F4) useHeadlight = !useHeadlight;
                    if (e.key.keysym.sym == SDLK_F5) useShadow = !useShadow;
                    if (e.key.keysym.sym == SDLK_F6) useSabbiaTempesta = !useSabbiaTempesta;

                    if (e.key.keysym.sym == SDLK_ESCAPE) done = 1;

                    //ricominciare il gioco
                    if (e.key.keysym.sym == SDLK_r) {
                        schermata = 0;
                        preGame = 1;
                        vespa.Init();
                        useWireframe = false;
                        useRadar = false;
                        useHeadlight = false;
                        useShadow = true;
                        useSabbiaTempesta = false;
                        viewAlpha = 0;
                        viewBeta = 20;
                        eyeDist = 5;
                    } else {
                        preGame = 0; //variabile che tiene traccia del fatto di trovarsi prima di iniziare  il gioco
                    }

                    break;
                case SDL_KEYUP:
                    vespa.controller.EatKey(e.key.keysym.sym, keymap, false);
                    break;
                case SDL_QUIT:
                    done = 1;
                    break;
                case SDL_WINDOWEVENT:
                    // dobbiamo ridisegnare la finestra
                    if (e.window.event == SDL_WINDOWEVENT_EXPOSED)
                        rendering(win);
                    else {
                        windowID = SDL_GetWindowID(win);
                        if (e.window.windowID == windowID) {
                            switch (e.window.event) {
                                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                                    scrW = e.window.data1;
                                    scrH = e.window.data2;
                                    glViewport(0, 0, scrW, scrH);
                                    rendering(win);
                                    break;
                                }
                            }
                        }
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (!preGame) {
                        if (e.motion.state & SDL_BUTTON(1) /*& cameraType == CAMERA_MOUSE*/) {
                            viewAlpha += e.motion.xrel;
                            viewBeta += e.motion.yrel;
                            if (viewBeta < +5) viewBeta = +5; //per non andare sotto la vespa
                            if (viewBeta > +90) viewBeta = +90;
                        }
                    }
                    break;

                    //TODO ELIMINARE
                case SDL_MOUSEWHEEL:
                    if (!preGame) {
                        if (e.wheel.y > 0) {
                            // avvicino il punto di vista (zoom in)
                            eyeDist = eyeDist * 0.9;
                            if (eyeDist < 1) eyeDist = 1;
                        };
                        if (e.wheel.y < 0) {
                            // allontano il punto di vista (zoom out)
                            eyeDist = eyeDist / 0.9;
                        };
                    }
                    break;
            }
        } else {
            // nessun evento: siamo IDLE
            Uint32 timeNow = SDL_GetTicks(); // che ore sono?

            bool doneSomething = false;
            int guardia = 0; // sicurezza da loop infinito

            // finche' il tempo simulato e' rimasto indietro rispetto al tempo reale...
            while (nstep * PHYS_SAMPLING_STEP < timeNow) {
                vespa.DoStep();
                nstep++;
                doneSomething = true;
                timeNow = SDL_GetTicks();
                if (guardia++ > 1000) {
                    done = true;
                    break;
                } // siamo troppo lenti!
            }

            if (doneSomething) {

                if (preGame) {
                    renderingPreGame(win);
                } else {
                    if (schermata != 1 &&
                        schermata != 2) { //se colpisci roccia o finisci quella è l'ultima schermata che vedi
                        schermata = gameplay();
                    }
                    switch (schermata) {
                        case 1: // traguardo
                            renderingFineStrada(win);
                            break;
                        case 2: // roccia colpita -> gameover
                            renderingGameOver(win);
                            break;
                        default: // gioco in corso
                            rendering(win);
                            break;

                    }
                }
            } else {
                // tempo libero!!!
            }
        }
    }
    SDL_GL_DeleteContext(mainContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return (0);
}

