// vespa.cpp
// implementazione dei metodi definiti in vespa.h

#include <stdio.h>
#include <math.h>
#include <ctime>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else

#include <GL/gl.h>
#include <GL/glu.h>

#endif

#include <vector> // la classe vector di STL 

#include "vespa.h"
#include "../mesh/mesh.h"

#define FLOOR_LIMIT 249.3 //limite del floor del main (decimale usato per non far sporgere nulla della moto)
#define FINE_STRADA 10


clock_t startTime;


// var globale di tipo mesh
Mesh moto((char *) "models/vespa_nowheels_nomanubrio_nocopri_nosella.obj"); // chiama il costruttore
Mesh manubrio((char *) "models/vespa_manubrio_p.obj");
Mesh sella((char *) "models/vespa_sella.obj");
Mesh copriRuota((char *) "models/vespa_copri.obj");
Mesh faroAnteriore((char *) "models/vespa_front_light.obj");
Mesh wheelBR1((char *) "models/vespa_wheel_gomma.obj");
Mesh wheelFR1((char *) "models/vespa_wheel_gomma.obj");
Mesh wheelBR2((char *) "models/vespa_wheel_metal.obj");
Mesh wheelFR2((char *) "models/vespa_wheel_metal.obj");

Mesh striscia((char *) "models/street_line.obj");
Mesh baloon((char *) "models/baloon.obj");
Mesh baseBaloon((char *) "models/base_baloon.obj");
Mesh roccia((char *) "models/rock.obj");
Mesh questionMark((char *) "models/question_mark.obj");
Mesh billboard((char *) "models/billboard.obj");


extern bool useRadar; // var globale esterna: per usare l'evnrionment mapping
extern bool useHeadlight; // var globale esterna: per usare i fari
extern bool useShadow; // var globale esterna: per generare l'ombra


void Controller::Init() {
    for (int i = 0; i < NKEYS; i++) key[i] = false;
}

void Controller::startTimeCounting() {
    startTime = clock();
}

double Controller::getSeconds() {
    clock_t currentTime = clock();
    clock_t clockTicksTaken = currentTime - startTime;
    return clockTicksTaken / (double) CLOCKS_PER_SEC;
}

void Vespa::Init() {
    // inizializzo lo stato della vespa
    px = py = facing = 0; // posizione e orientamento

    pz = FLOOR_LIMIT - 20;
    mozzoA = mozzoP = sterzo = 0;   // stato
    vx = vy = vz = 0;      // velocita' attuale
    // inizializzo la struttura di controllo
    controller.Init();

    velSterzo = 2.0;         // A
    velRitornoSterzo = 0.93; // B, sterzo massimo = A*B / (1-B)

    accMax = 0.0015;

    // attriti: percentuale di velocita' che viene mantenuta
    // 1 = no attrito
    // <<1 = attrito grande
    attritoZ = 0.991;  // piccolo attrito sulla Z (nel senso di rotolamento delle ruote)
    attritoX = 0.8;  // grande attrito sulla X (per non fare slittare la vespa)
    attritoY = 1.0;  // attrito sulla y nullo

    // Nota: vel max = accMax*attritoZ / (1-attritoZ)

    raggioRuotaA = 0.35;
    raggioRuotaP = 0.35;

    grip = 0.35; // quanto il facing vespa si adegua velocemente allo sterzo

    //Variabili utili per gli esiti di quando viene colpito il punto interrogativo in-game
    comandiInvertiti = false;
    aumentaAcc = false;
    diminuisciAcc = false;
}

// da invocare quando e' stato premuto/rilasciato il tasto numero "keycode"
void Controller::EatKey(int keycode, int *keymap, bool pressed_or_released) {
    for (int i = 0; i < NKEYS; i++) {
        if (keycode == keymap[i]) key[i] = pressed_or_released;
    }
}

// da invocare quando e' stato premuto/rilasciato un jbutton
void Controller::Joy(int keymap, bool pressed_or_released) {
    key[keymap] = pressed_or_released;
}

// disegno la linea stradale continua sui lati
void drawLineaStradale() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glScalef(0.1, 1, FLOOR_LIMIT + 0.03);
    glPushMatrix();
    glTranslatef(-100, 0.001, 0);
    striscia.RenderNxF();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(125, 0.001, 0);
    striscia.RenderNxF();
    glPopMatrix();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}


// funzione che prepara tutto per creare le coordinate texture (s,t) da (x,y,z)
// Mappo l'intervallo [ minY , maxY ] nell'intervallo delle T [0..1]
//     e l'intervallo [ minZ , maxZ ] nell'intervallo delle S [0..1]
void setupPartTexture(Point3 min, Point3 max, int texture) {
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    // ulilizzo le coordinate OGGETTO
    // cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
    // in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    float sz = 1.0 / (max.Z() - min.Z());
    float ty = 1.0 / (max.Y() - min.Y());
    float s[4] = {0, 0, sz, -min.Z() * sz};
    float t[4] = {0, ty, 0, -min.Y() * ty};
    glTexGenfv(GL_S, GL_OBJECT_PLANE, s);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, t);
    glPopMatrix();
}


// disegno i palloncini oltre il traguardo (utili per far vedere dove si trova la fine del percoso)
void drawPalloncini() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    //glScalef(0.1, 1, FLOOR_LIMIT + 0.03);
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslatef(FLOOR_LIMIT - 5, 28, -FINE_STRADA + 3);
    setupPartTexture(baloon.bbmin, baloon.bbmax, 9);
    baloon.RenderNxF();
    glTranslatef(0, -28, 0);
    baseBaloon.RenderNxF();
    glPopMatrix();

    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslatef(FLOOR_LIMIT - 5, 28, +FINE_STRADA);
    setupPartTexture(baloon.bbmin, baloon.bbmax, 9);
    baloon.RenderNxF();
    glTranslatef(0, -28, 0);
    baseBaloon.RenderNxF();
    glPopMatrix();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// disegno una roccia
Mesh drawRoccia() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glPushMatrix();
    setupPartTexture(roccia.bbmin, roccia.bbmax, 10);
    roccia.RenderNxV();
    glPopMatrix();
    glPopMatrix();
    glEnable(GL_LIGHTING);
    return roccia;
}

// applico la mia immagine al billboard a fine strada
void drawBillboardPoster() {
    glPushMatrix();
    float billboardPoster[4][3] = {
            {27.7, 1.2, 0.9}, // lower sx vertex
            {20.4, 1.2, 0.9}, // lower dx vertex
            {27.7, 14,  0.9}, // higher sx vertex
            {20.4, 14,  0.9}, // higher dx vertex
    };
    glTranslatef(-22.25,0,-246); //posizionare il poster sul billboard alla fine del percorso
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, 13);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // render original colors of the texture
    glBegin(GL_QUADS);

    glTexCoord2f(1.0, 0.0);
    glVertex3fv(billboardPoster[2]);
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(billboardPoster[3]);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(billboardPoster[1]);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(billboardPoster[0]);

    glEnd();
    glPopMatrix();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // back to the default value of GL_TEXTURE_ENV_MODE
    glPopMatrix();
}

// disegno il billboard a fine strada
void drawBillboard() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(0.5, 0.5, 0.5);
    glScalef(3, 3, 3);
    setupPartTexture(billboard.bbmin, billboard.bbmax, 12);
    glTranslatef(0.6,0,-82); // billboard a fine strada
    billboard.RenderNxF();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// disegno i 3 punti interrogativi in posizioni pre-fissate del percorso
void drawQuestionMarks() {
    //punto interrogativo 1
    glPushMatrix();
    glEnable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glPushMatrix();
    glScalef(0.7, 0.7, 0.7);
    glRotatef(90, 0, 1, 0);
    glTranslatef(-210, 2.5, 6);
    setupPartTexture(questionMark.bbmin, questionMark.bbmax, 11);
    questionMark.RenderNxF();
    glPopMatrix();
    if (useShadow) {
        glPushMatrix();
        glDisable(GL_LIGHTING); // niente lighing per l'ombra
        glScalef(0.7, 0.7, 0.7);
        glRotatef(90, 0, 1, 0);
        glTranslatef(-210, 0, 6);
        glColor3f(0.1, 0.1, 0.1); // colore fisso
        glTranslatef(0, 0.01, 0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
        glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
        glDisable(GL_TEXTURE_2D);
        questionMark.RenderNxF();
        glColor3f(1, 1, 1);
        glPopMatrix();
    }

    //punto interrogativo 2
    glPushMatrix();
    glEnable(GL_LIGHTING);
    glScalef(0.7, 0.7, 0.7);
    glRotatef(90, 0, 1, 0);
    glTranslatef(-76, 2.5, 6);
    setupPartTexture(questionMark.bbmin, questionMark.bbmax, 11);
    questionMark.RenderNxF();
    glPopMatrix();
    if (useShadow) {
        glPushMatrix();
        glDisable(GL_LIGHTING); // niente lighing per l'ombra
        glScalef(0.7, 0.7, 0.7);
        glRotatef(90, 0, 1, 0);
        glTranslatef(-76, 0, 6);
        glColor3f(0.1, 0.1, 0.1); // colore fisso
        glTranslatef(0, 0.01, 0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
        glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
        glDisable(GL_TEXTURE_2D);
        questionMark.RenderNxF();
        glColor3f(1, 1, 1);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    //punto interrogativo 3
    glPushMatrix();
    glEnable(GL_LIGHTING);
    glScalef(0.7, 0.7, 0.7);
    glRotatef(90, 0, 1, 0);
    glTranslatef(33, 2.5, -6);
    setupPartTexture(questionMark.bbmin, questionMark.bbmax, 11);
    questionMark.RenderNxF();
    glPopMatrix();

    if (useShadow) {
        glPushMatrix();
        glDisable(GL_LIGHTING); // niente lighing per l'ombra
        glScalef(0.7, 0.7, 0.7);
        glRotatef(90, 0, 1, 0);
        glTranslatef(33, 0, -6);
        glColor3f(0.1, 0.1, 0.1); // colore fisso
        glTranslatef(0, 0.01, 0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
        glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
        glDisable(GL_TEXTURE_2D);
        questionMark.RenderNxF();
        glColor3f(1, 1, 1);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
}

// funzione che viene inverte i comandi dello sterzo
void Vespa::invertiComandi() {
    if (controller.key[Controller::LEFT]) sterzo -= velSterzo; //inverto sterzo post passaggio su punto interrogativo
    if (controller.key[Controller::RIGHT]) sterzo += velSterzo; //inverto sterzo post passaggio su punto interrogativo
    sterzo *= velRitornoSterzo;
    comandiInvertiti = true;
    aumentaAcc = false;
    diminuisciAcc = false;
}

// funzione che setta i flag in base all'esito del passaggio sul punto interrogativo
void Vespa::aumentaAccelerazione() {
    aumentaAcc = true;
    diminuisciAcc = false;
    comandiInvertiti = false;
}

// funzione che setta i flag in base all'esito del passaggio sul punto interrogativo
void Vespa::diminuisciAccelerazione() {

    diminuisciAcc = true;
    aumentaAcc = false;
    comandiInvertiti = false;

}

// funzione che fa un passo di fisica (a delta_t costante)
void Vespa::DoStep() {
    // computiamo l'evolversi della macchina

    float vxm, vym, vzm; // velocita' in spazio macchina

    // da vel frame mondo a vel frame macchina
    float cosf = cos(facing * M_PI / 180.0);
    float sinf = sin(facing * M_PI / 180.0);
    vxm = +cosf * vx - sinf * vz;
    vym = vy;
    vzm = +sinf * vx + cosf * vz;

    // gestione dello sterzo e delle accelerazioni (normale o cambiamenti dovuti ai punti interrogativi)
    if (px < 2.5 + FINE_STRADA && px > -FINE_STRADA && comandiInvertiti == false && aumentaAcc == false &&
        diminuisciAcc == false) {
        if (controller.key[Controller::ACC]) vzm -= accMax; // accelerazione in avanti
        if (controller.key[Controller::LEFT]) sterzo += velSterzo;
        if (controller.key[Controller::RIGHT]) sterzo -= velSterzo;
        sterzo *= velRitornoSterzo; // ritorno a volante dritto
    } else if (comandiInvertiti == true) {
        if (px < 2.5 + FINE_STRADA && px > -FINE_STRADA) {
            if (controller.key[Controller::ACC]) vzm -= accMax; // accelerazione in avanti
        } else {
            if (controller.key[Controller::ACC])
                vzm -= accMax / 4; // accelerazione in avanti rallentata perché fuoripista
        }
        invertiComandi();
    } else if (aumentaAcc == true) {
        if (px < 2.5 + FINE_STRADA && px > -FINE_STRADA) {
            if (controller.key[Controller::ACC]) vzm -= accMax * 2; // accelerazione in avanti
        } else {
            if (controller.key[Controller::ACC])
                vzm -= accMax / 4; // accelerazione in avanti rallentata perché fuoripista
        }
        if (controller.key[Controller::LEFT]) sterzo += velSterzo;
        if (controller.key[Controller::RIGHT]) sterzo -= velSterzo;
        sterzo *= velRitornoSterzo; // ritorno a volante dritto
    } else if (diminuisciAcc == true) {
        if (px < 2.5 + FINE_STRADA && px > -FINE_STRADA) {
            if (controller.key[Controller::ACC]) vzm -= accMax / 2; // accelerazione in avanti
        } else {
            if (controller.key[Controller::ACC])
                vzm -= accMax / 4; // accelerazione in avanti rallentata perché fuoripista
        }
        if (controller.key[Controller::LEFT]) sterzo += velSterzo;
        if (controller.key[Controller::RIGHT]) sterzo -= velSterzo;
        sterzo *= velRitornoSterzo; // ritorno a volante dritto
    } else {
        if (controller.key[Controller::ACC]) vzm -= accMax / 4; // accelerazione in avanti rallentata perché fuoripista
        if (controller.key[Controller::LEFT]) sterzo += velSterzo;
        if (controller.key[Controller::RIGHT]) sterzo -= velSterzo;
        sterzo *= velRitornoSterzo;
        // ritorno a volante dritto
    }
    if (controller.key[Controller::DEC]) vzm += accMax / 8; // accelerazione indietro (molto lenta nella vespa)

    // attirti (semplificando)
    vxm *= attritoX;
    vym *= attritoY;
    vzm *= attritoZ;

    // l'orientamento della vespa segue quello dello sterzo (a seconda della velocita' sulla z)
    facing = facing - (vzm * grip) * sterzo;

    // rotazione mozzo ruote (a seconda della velocita' sulla z)
    float da; //delta angolo
    da = (360.0 * vzm) / (2.0 * M_PI * raggioRuotaA);
    mozzoA += da;
    da = (360.0 * vzm) / (2.0 * M_PI * raggioRuotaP);
    mozzoP += da;

    // ritorno a vel coord mondo
    vx = +cosf * vxm + sinf * vzm;
    vy = vym;
    vz = -sinf * vxm + cosf * vzm;

    // posizione = posizione + velocita * delta t (ma delta t e' costante)
    px += vx;
    py += vy;
    pz += vz;

    // metto dei limiti di posizione alla vespa per non uscire dallo skybox
    if (pz > FLOOR_LIMIT)
        pz = FLOOR_LIMIT;
    if (pz < -FLOOR_LIMIT)
        pz = -FLOOR_LIMIT;
    if (px > FLOOR_LIMIT)
        px = FLOOR_LIMIT;
    if (px < -FLOOR_LIMIT) {
        px = -FLOOR_LIMIT;
    }

    // printf("%f px, %f pz\n", px, pz); //debug
}

// attiva una luce di openGL per simulare un faro della vespa
void Vespa::DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) const {
    int usedLight = GL_LIGHT1 + lightN;

    if (useHeadlight) {
        glEnable(usedLight);

        float col0[4] = {0.8, 0.8, 0.0, 1};
        glLightfv(usedLight, GL_DIFFUSE, col0); //luce faro

        float col1[4] = {0.5, 0.5, 0.0, 1};
        glLightfv(usedLight, GL_AMBIENT, col1); //luce ambientale

        float tmpPos[4] = {x, y, z, 1}; // ultima comp=1 => luce posizionale
        glLightfv(usedLight, GL_POSITION, tmpPos);

        float tmpDir[4] = {0, -0.5, -1, 0}; // ultima comp=1 => luce posizionale
        glLightfv(usedLight, GL_SPOT_DIRECTION, tmpDir);

        glLightf(usedLight, GL_SPOT_CUTOFF, 30);
        glLightf(usedLight, GL_SPOT_EXPONENT, 5);

        glLightf(usedLight, GL_CONSTANT_ATTENUATION, 0);
        glLightf(usedLight, GL_LINEAR_ATTENUATION, 0.5);
    } else
        glDisable(usedLight);
}

// funzione che disegna il manubrio della vespa
void renderManubrio(bool usecolor, float sterzo) {
    glPushMatrix();
    glRotatef(sterzo / 1.5, 0, 1, 0); //piega in curva
    if (usecolor) glColor3f(1, 1, 1);
    if (usecolor) setupPartTexture(manubrio.bbmin, manubrio.bbmax, 1);
    manubrio.RenderNxF(); // rendering della mesh usando normali per faccia
    glPopMatrix();
}

// funzione che disegna la maggior parte della vespa (senza le parti separate per fare le rotazioni/inclinazioni)
void renderMostOfVespa(bool usecolor, float sterzo) {
    if (usecolor) glColor3f(1, 1, 1);
    if (usecolor) setupPartTexture(moto.bbmin, moto.bbmax, 1);
    if (!usecolor) {
        glRotatef(-sterzo / 2, 0, 0, 1);
    }
    moto.RenderNxV(); // rendering della mesh usando normali per vertice
}

// funzione che disegna la sella della vespa
void renderSella(bool usecolor) {
    glPushMatrix();
    if (usecolor) glColor3f(1, 1, 1);
    if (usecolor) setupPartTexture(sella.bbmin, sella.bbmax, 4);
    sella.RenderNxF(); // rendering della mesh usando normali per faccia
    glPopMatrix();
}

// funzione che disegna il copriruota anteriore della vespa
void renderCopriRuotaAnteriore(bool usecolor, float sterzo) {
    glPushMatrix();
    glTranslatef(-0.8, 9.8, 7.5); //posizionare il copri ruota anteriore in linea
    glRotatef(sterzo / 1.5, 0, 1, 0);
    if (usecolor) glColor3f(1, 1, 1);
    if (usecolor) setupPartTexture(copriRuota.bbmin, copriRuota.bbmax, 5);
    copriRuota.RenderNxF(); // rendering della mesh usando normali per faccia
    glPopMatrix();
}

// funzione che disegna il faro anteriore della vespa
void renderFaroAnteriore(bool usecolor, float sterzo) {
    glPushMatrix();
    glRotatef(sterzo / 1.5, 0, 1, 0);
    if (usecolor) glColor3f(0.8, 0.8, 0.8);     // colore grigio
    if (usecolor) setupPartTexture(faroAnteriore.bbmin, faroAnteriore.bbmax, 3);
    faroAnteriore.RenderNxF(); // rendering della mesh usando normali per faccia
    glPopMatrix();
}

// funzione che disegna le ruote della vespa
void renderRuote(bool usecolor, float sterzo, float mozzoA) {
    glPushMatrix();
    glScalef(0.90, 0.90, 0.90);
    glTranslate(wheelFR1.Center());
    glTranslatef(-2.2, 7, 9); //posizionare la ruota anteriore in linea
    glRotatef(sterzo / 1.5, 0, 1, 0);
    glRotatef(-mozzoA, 1, 0, 0);
    glTranslate(-wheelFR1.Center());
    if (usecolor) glColor3f(.6, .6, .6);
    if (usecolor) setupPartTexture(wheelFR1.bbmin, wheelFR1.bbmax, 0);
    wheelFR1.RenderNxF(); // rendering della mesh usando normali per faccia
    glDisable(GL_TEXTURE_2D);
    if (usecolor) glColor3f(0.9, 0.9, 0.9);
    wheelFR2.RenderNxF(); // rendering della mesh usando normali per faccia
    glPopMatrix();

    glPushMatrix();
    glScalef(0.90, 0.90, 0.90);
    glTranslate(wheelBR1.Center());
    glTranslatef(-2, 7, -31); //posizionare la ruota posteriore in linea
    glRotatef(-mozzoA, 1, 0, 0);
    glTranslate(-wheelBR1.Center());
    if (usecolor) glColor3f(.6, .6, .6);
    if (usecolor) setupPartTexture(wheelBR1.bbmin, wheelBR1.bbmax, 0);
    wheelBR1.RenderNxF(); // rendering della mesh usando normali per faccia
    glDisable(GL_TEXTURE_2D);
    if (usecolor) glColor3f(0.9, 0.9, 0.9);
    wheelBR2.RenderNxF(); // rendering della mesh usando normali per faccia
    glPopMatrix();
}

// funzione che disegna la moto completa con tutti i suoi pezzi
void Vespa::RenderAllParts(bool usecolor) const {

    // disegna la vespa con una mesh
    glPushMatrix();
    if (usecolor) glEnable(GL_LIGHTING);

    //piega sull'asse z solo se non sei ombra
    if (usecolor) {
        glRotatef(sterzo / 2, 0, 0, 1);
    }
    glScalef(-0.05, 0.05, -0.05); // patch: riscaliamo la mesh di 1/10
    renderMostOfVespa(usecolor, sterzo);
    renderManubrio(usecolor, sterzo);
    renderSella(usecolor);
    renderCopriRuotaAnteriore(usecolor, sterzo);
    renderFaroAnteriore(usecolor, sterzo);
    renderRuote(usecolor, sterzo, mozzoA);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glPopMatrix();
}

// disegna a schermo la vespa
void Vespa::Render() const {
    // sono nello spazio mondo


    glTranslatef(px, py, pz);
    glRotatef(facing, 0, 1, 0);

    glPushMatrix();
    glRotatef(sterzo / 2, 0, 0, 1);
    glRotatef(sterzo / 1.5, 0, 1, 0);
    DrawHeadlight(0.1, 1.6, 0, 1, useHeadlight); // accendi faro anteriore centrale

    glPopMatrix();
    RenderAllParts(true);

    // ombra
    if (useShadow) {
        glColor3f(0.1, 0.1, 0.1); // colore fisso
        glTranslatef(0, 0.01, 0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
        glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
        glDisable(GL_LIGHTING); // niente lighing per l'ombra
        RenderAllParts(false);  // disegno la vespa appiattita
        glColor3f(1, 1, 1);
        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
}
