
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include "cube.h"

#define DIM_X_MAX_TARGET 10;
#define DIM_X_MIN_TARGET 10;
#define DIM_Y_MAX_TARGET 10;
#define DIM_Y_MIN_TARGET 10;
#define DIM_Z_MAX_TARGET 10;
#define DIM_Z_MIN_TARGET 10;


void drawPolygon(bool texture, bool border) {

    printf("%d\n",texture);
    float x_min_polygon, x_max_polygon, y_min_polygon, y_max_polygon, z_min_polygon, z_max_polygon;

    //dimensioni per target
    x_min_polygon = DIM_X_MIN_TARGET;
    x_max_polygon = DIM_X_MAX_TARGET;
    y_min_polygon = DIM_Y_MIN_TARGET;
    y_max_polygon = DIM_Y_MAX_TARGET;
    z_min_polygon = DIM_Z_MIN_TARGET;
    z_max_polygon = DIM_Z_MAX_TARGET;

    glBegin(GL_QUADS);
    /* Front. */
    if (texture) { glTexCoord2f(1.0, 1.0); }
    glVertex3f(x_min_polygon, y_min_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(0.0, 1.0); }
    glVertex3f(x_max_polygon, y_min_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(0.0, 0.0); }
    glVertex3f(x_max_polygon, y_max_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(1.0, 0.0); }
    glVertex3f(x_min_polygon, y_max_polygon, z_max_polygon);

    /* Down. */
    if (texture) { glTexCoord2f(0.0, 0.0); }
    glVertex3f(x_min_polygon, y_min_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(1.0, 0.0); }
    glVertex3f(x_max_polygon, y_min_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(1.0, 1.0); }
    glVertex3f(x_max_polygon, y_min_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(0.0, 1.0); }
    glVertex3f(x_min_polygon, y_min_polygon, z_max_polygon);

    /* Back. */
    if (texture) { glTexCoord2f(0.0, 0.0); }
    glVertex3f(x_min_polygon, y_max_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(1.0, 0.0); }
    glVertex3f(x_max_polygon, y_max_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(1.0, 1.0); }
    glVertex3f(x_max_polygon, y_min_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(0.0, 1.0); }
    glVertex3f(x_min_polygon, y_min_polygon, z_min_polygon);

    /* Up. */
    if (texture) { glTexCoord2f(0.0, 0.0); }
    glVertex3f(x_min_polygon, y_max_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(1.0, 0.0); }
    glVertex3f(x_max_polygon, y_max_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(1.0, 1.0); }
    glVertex3f(x_max_polygon, y_max_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(0.0, 1.0); }
    glVertex3f(x_min_polygon, y_max_polygon, z_min_polygon);

    /* SideLeft. */
    if (texture) { glTexCoord2f(0.0, 0.0); }
    glVertex3f(x_min_polygon, y_max_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(1.0, 0.0); }
    glVertex3f(x_min_polygon, y_max_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(1.0, 1.0); }
    glVertex3f(x_min_polygon, y_min_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(0.0, 1.0); }
    glVertex3f(x_min_polygon, y_min_polygon, z_min_polygon);

    /* SideRight. */
    if (texture) { glTexCoord2f(0.0, 0.0); }
    glVertex3f(x_max_polygon, y_max_polygon, z_min_polygon);
    if (texture) { glTexCoord2f(1.0, 0.0); }
    glVertex3f(x_max_polygon, y_max_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(1.0, 1.0); }
    glVertex3f(x_max_polygon, y_min_polygon, z_max_polygon);
    if (texture) { glTexCoord2f(0.0, 1.0); }
    glVertex3f(x_max_polygon, y_min_polygon, z_min_polygon);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    if (border) {
        glLineWidth(1);
        glColor3f(0, 0, 0);
        glBegin(GL_LINE_LOOP);
        glVertex3f(x_min_polygon, y_min_polygon, z_max_polygon);
        glVertex3f(x_max_polygon, y_min_polygon, z_max_polygon);
        glVertex3f(x_max_polygon, y_max_polygon, z_max_polygon);
        glVertex3f(x_min_polygon, y_max_polygon, z_max_polygon);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3f(x_min_polygon, y_max_polygon, z_min_polygon);
        glVertex3f(x_max_polygon, y_max_polygon, z_min_polygon);
        glVertex3f(x_max_polygon, y_min_polygon, z_min_polygon);
        glVertex3f(x_min_polygon, y_min_polygon, z_min_polygon);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(x_min_polygon, y_min_polygon, z_max_polygon);
        glVertex3f(x_min_polygon, y_min_polygon, z_min_polygon);

        glVertex3f(x_max_polygon, y_min_polygon, z_max_polygon);
        glVertex3f(x_max_polygon, y_min_polygon, z_min_polygon);

        glVertex3f(x_min_polygon, y_max_polygon, z_max_polygon);
        glVertex3f(x_min_polygon, y_max_polygon, z_min_polygon);

        glVertex3f(x_max_polygon, y_max_polygon, z_max_polygon);
        glVertex3f(x_max_polygon, y_max_polygon, z_min_polygon);
        glEnd();
    }
    glPopMatrix();
}

void drawCube(bool shadow, bool useWireframe){
    glPushMatrix();
    /*if (isTnt)
        glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_TNT);
    else*/
        glBindTexture(GL_TEXTURE_2D, 3);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_GEN_MODE, GL_REPLACE);

    /* disegno il cubo target */
    //glTranslatef(targetPoint.X(), targetPoint.Y(), targetPoint.Z());
    glColor3f(1, 1, 1);
    glDisable(GL_LIGHTING);
    drawPolygon(true, true);

    /* disegno l'ombra del cubo target */
    if(shadow) {
        bool b = false;
        glPushMatrix();
       // glTranslatef(targetPoint.X(), targetPoint.Y(), targetPoint.Z());
        glColor3f(0.1, 0.1, 0.1); // colore fisso
        glTranslatef(0, 0.01, 0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
        glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
        glDisable(GL_LIGHTING); // niente lighing per l'ombra
        glColor3f(1, 1, 1);
        if(!useWireframe) { glColor3f(0.05, 0.05, 0.05); } else { glColor3f(1, 1, 1); b = true;}
        drawPolygon(false,  b);
        glPopMatrix();
    }
}