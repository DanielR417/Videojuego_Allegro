#include <iostream>
#include <allegro.h>
#include <cmath>

using namespace std;

void pintar_nave(float cx, float cy, BITMAP *buffer);
void mover_nave(float &cx, float &cy, float &vx, float &vy);
void rotar(float &x, float &y, float cx, float cy, float da);
void aceleracion(float da, float &vx, float &vy); // da = angulo
void pintar_motor(float da, float cx, float cy, BITMAP *buffer);
void medidor_combustible(bool gastar_combustible, float &combustible, BITMAP *buffer);
void pintar_nivel(int num_nivel, BITMAP *buffer);
void explosion(float cx, float cy, BITMAP *buffer, int num_nivel);
bool GameOver(float cx, float cy, BITMAP *buffer, int num_nivel);

int main()
{
    //buffer es la estampa principal donde se arma todo el escenario
    allegro_init();
    install_keyboard();
    set_color_depth(32); //bits a ocupar
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 740, 500, 0, 0); //tamaño de la pantalla donde se mostrará el juegoi
    BITMAP *buffer = create_bitmap(740, 500); // 740 -> coordenadas en x
                                              // 500 -> coordenadas en y
    //vertices del ractángulo
    float cx, cy; //centro geometrico de la nave
    float vx, vy;
    cx = 100; cy = 100;
    float combustible = 100;
    int num_nivel = 2;

    while(!key[KEY_ESC] && !GameOver(cx, cy, buffer, num_nivel)){  // mientras no se presione la tecla esc se ejecuta esto
        bool gastar_combustible = false;
        clear_to_color(buffer, 0x000000);
        pintar_nivel(num_nivel, buffer);
        mover_nave(cx, cy, vx, vy);

        // Aceleración hacia arriba
        if(key[KEY_UP] && combustible > 0){
            aceleracion(0, vx, vy); // angulo = 0 aceleración hacia arriba
            pintar_motor(0, cx, cy, buffer); //angulo=0 fuego hacia abajo
            gastar_combustible = true;
        }

        if(key[KEY_RIGHT] && combustible > 0){
            aceleracion(-90, vx, vy); //
            pintar_motor(-90, cx, cy, buffer);
            gastar_combustible = true;
        }

        if(key[KEY_LEFT] && combustible > 0){
            aceleracion(90, vx, vy); //
            pintar_motor(90, cx, cy, buffer);
            gastar_combustible = true;
        }

        medidor_combustible(gastar_combustible, combustible, buffer);
        pintar_nave(cx, cy, buffer);
        blit(buffer, screen, 0, 0, 0, 0, 740, 500); //impresion en pantalla del rectangulo

        rest(20);
    }

    return 0;
}
END_OF_MAIN();

/** @brief Termina el juego si existe una colisión con los límites de la pantalla,
 * se valida si la parte derecha, izquierda, superior y baja de la nave se empalman con
 * los límites de la pantalla.
 * @param coordenada cx de la nave
 * @param coornenada cy de la nave
 * @param *buffer El área sobre la que se pintará el nivel
 * @param num_nivel es un dato entero que indica el nivel en el que te encuentras
 * @return true se devuelve este valor si existe una colisión con los límites de la pantalla
 * @return false se devuelve cuando no existe ninguna colisión y te encuentras dentro de la pantalla
 */
bool GameOver(float cx, float cy, BITMAP *buffer, int num_nivel){
    // veremos la nave como un rectangulo para el área de colisión
    if(cx+20 >= 740 || cx-20 <= 0 || cy-15 <= 0 || cy + 20 >= 500){ //cx+20 limite de la parte derecha de la nave
        explosion(cx, cy, buffer, num_nivel);                       //cx-20 representa la parte izquierda de la nave
        return true;                                                //cy-15 representa la parte superior de la nave
    }                                                               //cy+20 representa la parte inferior de la nave
    return false;
}

/** @brief Simula la explosión de la nave al entrar en colisión con ciertas coordenadas
 * @param cx es la coordenada respecto del centro en x de la nave
 * @param cy es la coordenada respecto del centro en y de la nave
 * @param *buffer El área sobre la que se pintará el nivel
 * @param num_nivel es un dato entero que indica el nivel en el que te encuentras
 */
void explosion(float cx, float cy, BITMAP *buffer, int num_nivel){
    float x[12] = {cx-10, cx+10, cx   , cx   , cx+15, cx-15, cx+5, cx-10, cx+10, cx-5, cx-10, cx+10};// par de
    float y[12] = {cy   , cy   , cy-15, cy+15, cy-15, cy+15, cy+5, cy-10, cy-10, cy+5, cy   , cy   };// coordenadas
    float dx[6] = {7, 7, 0, -7, -7, 0};
    float dy[6] = {0, -7, -7, -7, 0, 7};


    clear(screen); //limpiar la pantalla
    do{
        clear(buffer);
        pintar_nivel(num_nivel, buffer);
        int j = 0;
        for(int i = 0; i <= 10; i+=2){
            line(buffer, x[i], y[i], x[i+1], y[i+1], 0x999999);
            rotar(x[i+1], y[i+1], x[i], y[i], 15);

            x[i] += dx[j]; // movimiento del pibote
            y[i] += dy[j];
            x[i+1] += dx[j]; // movimiento del punto que orbita
            y[i+1] += dy[j];
            j++;
        }
        blit(buffer, screen, 0, 0, 0, 0, 740, 500);
        rest(20);
    } while(!key[KEY_ESC]);
}

/** @brief Pinta los niveles, es decir, los obstaculos y el área de aterrizaje
 * @param num_nivel el numero del nivel
 * @param *buffer El área sobre la que se pintará el nivel
 */
void pintar_nivel(int num_nivel, BITMAP *buffer){
    if (num_nivel == 1){
        rectfill(buffer, 10, 450, 100, 500, 0x999999);
    }
    if(num_nivel == 2){
        triangle(buffer, 400, 500, 300, 500, 300, 200, 0x00FF1F);
        triangle(buffer, 300, 0, 500, 0, 500, 400, 0x00FF1F);
        triangle(buffer, 620, 500, 700, 500, 620, 230, 0x00FF1F);
        rectfill(buffer, 10, 450, 100, 500, 0x9B00FF);
    }
}

void medidor_combustible(bool gastar_combustible, float &combustible, BITMAP *buffer){
    textout_centre_ex(buffer, font, "Combustible", 100, 30, 0x999999, 0x000000); // color de letras y cuadro
    rectfill(buffer, 50, 50, 50+combustible, 55, 0x999999);
    if(gastar_combustible == true) combustible -= .2;

}

void pintar_motor(float da, float cx, float cy, BITMAP *buffer){
    float c1, c2;
    c1 = cx;
    c2 = cy;
    if(da != 0) c2+=9;
    float fuego[14] = {c1-5, c2+5, c1-10, c2+20, c1-5, c2+20, c1, c2+35, c1+5, c2+20, c1+10, c2+20, c1+5, c2+5};
    for(int i = 0; i<=12; i += 2){
        rotar(fuego[i], fuego[i+1], cx, cy, da);
    }

    for(int i = 0; i<=10; i += 2){
        line(buffer, fuego[i], fuego[i+1], fuego[i+2], fuego[i+3], 0x000CFF);
    }
}

void aceleracion(float da, float &vx, float &vy){
    // vector anclado en el origen con una longitud de 0.15 px que apunta hacia arriba
    float ax = 0, ay = -0.15;
    rotar(ax, ay, 0, 0, da); // si da=0 el vector aceleración queda intacto
    vx += ax;
    vy += ay;
}

void pintar_nave(float cx, float cy, BITMAP *buffer){
    line(buffer, cx-20, cy+20, cx-20, cy+10, 0x000CFF);
    line(buffer, cx-20, cy+10, cx-10, cy, 0x000CFF);
    line(buffer, cx-10, cy, cx-10, cy-10, 0x000CFF);
    line(buffer, cx-10, cy-10, cx, cy-15, 0x000CFF);
    line(buffer, cx, cy-15, cx+10, cy-10, 0x000CFF);
    line(buffer, cx+10, cy-10, cx+10, cy, 0x000CFF);
    line(buffer, cx+10, cy, cx+20, cy+10, 0x000CFF);
    line(buffer, cx+20, cy+10, cx+20, cy+20, 0x000CFF);
    line(buffer, cx-10, cy, cx+10, cy, 0x000CFF);

}

void mover_nave(float &cx, float &cy, float &vx, float &vy){
    float ax, ay;
    ax = 0.0;
    ay = 0.1; // gravedad que llevará hacia abajo a la nave 0.05 px

    vx += ax;
    vy += ay;

    cx += vx;
    cy += vy;
}

/*

*/

void rotar(float &x, float &y, float cx, float cy, float da){
    // da es un ángulo en grados
    float dx = x - cx;
    float dy = y - cy;
    float r = sqrt(dx*dx+dy*dy);  // longitud del vector
    float a = atan2(dy,dx);// valor del ángulo del vector
    // convertimos grados a radianes
    float da_rad = da/180 * M_PI;
    a -= da_rad; //a = suma de los ángulos

    x = cx + r*cos(a);
    y = cy + r*sin(a);
}
