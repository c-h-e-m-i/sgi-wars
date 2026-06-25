/***************************************************
*
*	SGI - Práctica 9
*	José María Rodríguez Charco
*
***************************************************/

#define PROYECTO "SGI Wars"

#include <iostream>	
#include <codebase.h>
#include <vector>
#include <random>

using namespace cb;
using namespace std;

// Dimensiones de la ventana:
static float
	aspect;

// Plataforma:
static const int
	DIM_PLATAFORMA = 50,
	SEMILADO_PLATAFORMA = DIM_PLATAFORMA / 2;

static GLuint
	modeloPlataforma;

// Dimensiones del fondo espacial:
static const int
	DIM_FONDO = 3000,
	LIM_MOV = 1000;

// Coordenadas anteriores del ratón:
static int
	xAnterior = -1,
	yAnterior = -1;

// Grados de guiñada y cabeceo:
static float
	guinyada = 0.0f,
	cabeceo = 0.0f,
	cabeceoAcumulado = 0.0f,
	pixelesAGrados = 0.5f; // Equivalencia de píxeles de movimiento del ratón a grados de cabeceo/guiñada

// Grados de alabeo:
static float
	alabeo = 0.0f,
	alabeoAcumulado = 0.0f,
	gradosPorSegundo = 180.0f; // Grados de alabeo y exploración de cabina (horario o antihorario) por cada segundo que pulsan las teclas correspondientes

// Ancho y alto de pantalla:
static float
	ancho = 640,
	alto = 360;

// Velocidad y aceleración de la nave:
static float
	vel = 0,
	vel0 = 0,
	acc = 8,
	maxVel = 40,
	accTrasColision = 20;

// Nave:
static const GLfloat
	RADIO_NAVE = 3.0f;

// Posición y vista de la cámara:
static const GLfloat
	Z0 = RADIO_NAVE;

static Sistema3d
	camara(Vec3(1, 0, 0), Vec3(0, 0, 1), Vec3(0, -1, 0), Vec3(0, 0, Z0));

static Vec3
	upGlobal(0, 0, 1),
	centroCaja; // Centro de la caja del espacio en la que está ubicada la cámara actualmente

// Variables de teclas:
static bool
	aPulsada = false,
	zPulsada = false,
	sPulsada = false,
	dPulsada = false,

	// > Flechas direccionales:
	arrPulsada = false,
	abjPulsada = false,
	izqPulsada = false,
	derPulsada = false;

static int
	iniAceleracion,
	iniAlabeo;

// Variables de animación:
static const int
	TASA_FPS = 60;

// Tiempo entre fotogramas:
static float
	fotoDelta; // Diferencia (en s) entre el fotograma anterior y el actual

// Asteroides:
static const GLfloat
	MIN_RADIO_ASTEROIDES = 6.0f,
	MAX_RADIO_ASTEROIDES = 10.0f,
	MIN_ESCALADO_ASTEROIDES = 0.5f,
	MAX_ESCALADO_ASTEROIDES = 1.5f;

static const GLint
	NUM_MODELOS_ASTEROIDE = 10,
	NUM_TEXTURAS_ASTEROIDE = 4,
	NUM_ASTEROIDES_CAJA = 100,
	DIM_CAJA_ASTEROIDES = 96,
	DIM_SUBCAJA_ASTEROIDES = 32;

static GLint
	modelosAsteroide[NUM_MODELOS_ASTEROIDE];

static vector<Vec3>
	centrosAsteroides;

// Cristales de energía:
static const GLfloat
	RADIO_CRISTAL = 2.0f;

static const GLint
	NUM_CRISTALES_CAJA = 1,
	/* > Un "cinturón de cristales" se define recursivamente como el conjunto de cajas vecinas de otro cinturón,
	 * siendo el cinturón base una única caja con centro en (0, 0, 0).
	 * 
	 * Si tenemos 2 cinturones de cristales, tendremos un total de 27 cajas (la de centro (0, 0, 0) y sus 26 vecinas).
	 * Con 3 cinturones, tendremos 125 cajas (las 27 que ya teníamos más las 98 vecinas de esas 27), y así sucesivamente: */
	NUM_CINTURONES_CRISTALES = 3,
	LADO_CINTURONES = 1 + 2 * (NUM_CINTURONES_CRISTALES - 1),
	NUM_CAJAS = LADO_CINTURONES * LADO_CINTURONES * LADO_CINTURONES,
	TOTAL_CRISTALES = NUM_CAJAS * NUM_CRISTALES_CAJA;

static GLint
	modeloCristal;

static unsigned int
	numCristalesRecogidos = 0;

static bool
	cristalesRecogidos[TOTAL_CRISTALES];

// Materiales:
static GLuint
	metal,
	roca,
	cristal,
	oro,
	plata,
	vidrio,
	farol;

// Luces:
static bool
	focosActivos = false;

// Texturas:
static GLuint
	plataforma[2],
	meteorito[NUM_TEXTURAS_ASTEROIDE],
	espacio[6],
	cabina[6],
	platillo;

static bool
	mostrarCabina = false;

// Exploración de cabina:
static const int
	DIM_CABINA = 1;

static GLfloat
	anguloExpEjeY = 0.0f,
	anguloExpEjeX = 0.0f;
	// > Usaremos la variable 'gradosPorSegundo' de la sección del alabeo para medir también los grados de giro aquí

static bool
	restablecerVistaCabina = false;

// Pantalla roja de daño:
static bool
	tintarPantalla;

static GLfloat
	opacidadRojoIni = 0.3f,
	desvanecimiento = 0.5f;

static int
	iniTintado;

// Puntos de vida:
static const int
	DANYO_COLISION = 10,
	DANYO_DISPARO = 20,
	CURACION_CRISTAL = 5;

static int
	puntosDeVida = 100;

// Disparos:
struct Bala {
	Vec3 pos, dir;
	int tiempo; // Momento en que fue disparada
	float vel = 80.0f;
	bool
		delJugador = true,
		activa = true;

	Bala(const Vec3& pos, const Vec3& dir) : pos(pos), dir(dir) {
		this->tiempo = glutGet(GLUT_ELAPSED_TIME);
	}
};

static const unsigned int
	MAX_DISPAROS_POR_SEG = 2,
	MAX_BALAS_EN_PANTALLA = 50;

static const float
	SEG_ENTRE_DISPAROS = 1.0f / MAX_DISPAROS_POR_SEG,
	MAX_SEG_BALA_ACTIVA = 2.0f,
	RADIO_BALA = 0.7f;

static GLuint
	modeloBala;

static vector<Bala>
	balas;

// Naves enemigas:
static const GLfloat
	RADIO_OVNI = RADIO_NAVE * 2,
	CAMPO_DE_VISION = 40.0f,
	VEL_ENEMIGO = 4.0f;

struct Enemigo {
	Vec3 pos, dir = Vec3(0, 0, 0);
	GLfloat vel = VEL_ENEMIGO;
	int pv = 60; // Puntos de vida
	bool activo = true;

	Enemigo(const Vec3& pos) : pos(pos) {}
};

static const unsigned int
	FRECUENCIA_ENEMIGOS = 3,
	MAX_ENEMIGOS_EN_PANTALLA = 10;

static GLuint
	modeloOvni;

static bool
	enemigosEnCaja[NUM_CAJAS];

static vector<Enemigo>
	enemigos;

// Explosiones:
struct Particula {
	Vec3 pos, vel;
	float vida;
	bool activa = false;
};

static const int
	MAX_PARTICULAS = 500;

static vector<Particula>
	particulas;

// Aleatoriedad:
static unsigned int
	semillaGlobal;

// Texto en pantalla:
static char
	bufferVida[30],
	bufferCristales[30];

// Fin del juego:
static bool
	gameOver = false;


// FUNCIONES AUXILIARES: Generación de números aleatorios con un generador basado en MT19937:
int enteroAleatorio(int min, int max, mt19937& generador)
{
	uniform_int_distribution<int> dist(min, max);
	return dist(generador);
}

float realAleatorio(float min, float max, mt19937& generador)
{
	uniform_real_distribution<float> dist(min, max);
	return dist(generador);
}

// FUNCIÓN AUXILIAR: Cálculo de puntos de una circunferencia con ruido:
vector<Vec3> puntosCircunferenciaDeformada(float radio, unsigned int numeroPuntos, float z, bool aleatoriedad = true)
{
	vector<Vec3> puntos;
	float delta = 2 * PI / numeroPuntos;
	for (int i = 0; i < numeroPuntos; i++) {
		float
			a = i * delta,
			ruido = aleatoriedad ? random(80.0f, 120.0f) : 100,
			radioAleatorio = radio * ruido / 100;

		puntos.push_back(Vec3(radioAleatorio * cosf(a), radioAleatorio * sinf(a), z));
	}

	return puntos;
}

// FUNCIÓN AUXILIAR: Cálculo de un color en escala [0, 1] pasando su código RGB:
const GLfloat* rgb(int r, int g, int b, int alpha = 255) {
	static GLfloat c[4];
	c[0] = r / 255.0f;
	c[1] = g / 255.0f;
	c[2] = b / 255.0f;
	c[3] = alpha / 255.0f;
	return c; // Devolvemos el puntero al primer elemento
}

// FUNCIÓN AUXILIAR: Generamos una skybox cúbica con la textura y el lado indicados:
void generarCubo(GLfloat l, GLuint* textura, GLfloat h = 0, int numTexturas = 6, int m = 10, int n = 10, int caraIni = 0, int caraFin = 5, bool dentro = true /* Indica si estamos dentro o fuera del cubo */)
{
	// Empezamos creando un array con todas las esquinas del cubo:
	if (h < 0.0001f) h = l;

	GLfloat esquinas[8][3] =
	{ { -l, l,  h }, { l, l,  h }, { -l, -l,  h }, { l, -l,  h },
		{ -l, l, -h }, { l, l, -h }, { -l, -l, -h }, { l, -l, -h }
	};

	// A continuación, definimos un array que recoja los índices de las esquinas que forman parte de cada cara:
	GLint caras[6][4] =
	{ { 0, 1, 3, 2 }, // Cara superior
		{ 6, 7, 5, 4 }, // Cara inferior
		{ 6, 4, 0, 2 }, // Cara izquierda
		{ 4, 5, 1, 0 }, // Cara trasera
		{ 5, 7, 3, 1 }, // Cara derecha
		{ 7, 6, 2, 3 }  // Cara delantera
	};

	// Generamos los 'quadtex' correspondientes:
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (numTexturas == 1) glBindTexture(GL_TEXTURE_2D, *textura);

	for (int i = caraIni; i < caraFin + 1; i++) {
		if (numTexturas > 1) glBindTexture(GL_TEXTURE_2D, textura[i % numTexturas]);

		quadtex(esquinas[caras[i][dentro ? 0 : 3]],
			esquinas[caras[i][dentro ? 1 : 2]],
			esquinas[caras[i][dentro ? 2 : 1]],
			esquinas[caras[i][dentro ? 3 : 0]],
			0, 1, 0, 1, m, n);
	}

	glPopAttrib();
}

// Función para generar la lista de dibujo de la plataforma base:
GLuint hangar() {
	GLuint hangar = glGenLists(1);
	glNewList(hangar, GL_COMPILE);

	// Abrimos una zona limitada para los atributos:
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT, GL_FILL);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Le damos un material metálico:
	glCallList(metal);

	// Creamos un cubo para los lados:
	GLfloat altura = SEMILADO_PLATAFORMA / 2.0f;

	// > Lo desplazamos para que su parte de arriba esté en el plano XY:
	glPushMatrix();
	glTranslatef(0, 0, -altura);

	generarCubo(SEMILADO_PLATAFORMA, &plataforma[1], altura, 1, 1, 2, 2, 5, false);

	// Generamos otro para las partes de arriba y abajo:
	generarCubo(SEMILADO_PLATAFORMA, &plataforma[0], altura, 1, 2, 2, 0, 1, false);

	glPopMatrix();
	glPopAttrib();

	glEndList();
	return hangar;
}

// Función para generar listas de dibujo de asteroides:
GLuint asteroide()
{
	/* La idea es la siguiente: vamos a generar una esfera con un número de meridianos y paralelos aleatorio.
	 * A cada punto de las circunferencias que conformarán los paralelos se les añadirá un ruido, también aleatorio,
	 * sobre el valor del radio, de modo que tendremos un conjunto de circunferencias deformes que uniremos formando una pseudoesfera: */

	 // > Fijaremos el mínimo de meridianos y paralelos en 5, y el máximo en 15.
	 // Para el radio el rango estará entre 'MIN_RADIO_ASTEROIDES' y 'MAX_RADIO_ASTEROIDES' (ambas variables globales):
	static int
		minSecciones = 5,
		maxSecciones = 15;

	GLint
		meridianos = random(minSecciones, maxSecciones + 1),
		paralelos = random(minSecciones, maxSecciones + 1);

	static float radio = 1.0f;

	GLfloat anguloPorParalelo = 180.0f / paralelos;

	// Creamos la lista de dibujo:
	GLuint asteroide = glGenLists(1);
	glNewList(asteroide, GL_COMPILE);

	// Generamos las circunferencias:
	vector<Vec3> circ1(meridianos, Vec3(0, 0, -radio));

	for (int i = 1; i <= paralelos; i++) {
		GLfloat
			angulo = rad((anguloPorParalelo * i - 90)),
			r = cosf(angulo) * radio,
			h = sinf(angulo) * radio;

		vector<Vec3> circ2 = (i < paralelos)
			? puntosCircunferenciaDeformada(r, meridianos, h)
			: vector<Vec3>(meridianos, Vec3(0, 0, radio));

		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= meridianos; j++) {
			// Calculamos las coordenadas de textura en función del paralelo y meridiano en los que esté cada vértice:
			GLfloat
				s = (float)j / meridianos,
				t1 = (float)(i - 1) / paralelos,
				t2 = (float)i / paralelos;

			// Empezamos por los vértices del parelelo superior para que el orden de los vértices de las caras sigan un sentido antihorario (y así poder aplicar back-face culling):
			Vec3 v2 = circ2[j % meridianos];
			glTexCoord2f(s, t2);
			glNormal3fv(v2);
			glVertex3fv(v2);

			Vec3 v1 = circ1[j % meridianos];
			glTexCoord2f(s, t1);
			glNormal3fv(v1); // Las normales serán aproximadamente los vectores que van del centro (0, 0, 0) al vértice; es decir, el propio vértice
			glVertex3fv(v1);
		}
		glEnd();

		circ1 = circ2;
	}
	glEndList();

	return asteroide;
}

// Función para generar listas de dibujo de cristales de energía:
GLuint cristalDeEnergia()
{
	GLuint cristalDeEnergia = glGenLists(1);
	glNewList(cristalDeEnergia, GL_COMPILE);

	glPushMatrix();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_TEXTURE_2D);

	glCallList(cristal);

	// Como el tamaño de los cristales será constante, incluimos el escalado en la lista de dibujo:
	glScalef(RADIO_CRISTAL, RADIO_CRISTAL, RADIO_CRISTAL);

	// Nuestros cristales de energía serán octaedros, ya definidos como primitivas en GLUT:
	glutSolidOctahedron();

	glPopAttrib();
	glPopMatrix();
	glEndList();

	return cristalDeEnergia;
}

// Función para generar listas de dibujo de ovnis (naves enemigas):
GLuint ovni()
{
	GLuint ovni = glGenLists(1);
	glNewList(ovni, GL_COMPILE);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	// Su cúpula será una esfera de vidrio sin textura:
	glCallList(vidrio);
	const GLfloat RADIO_ESFERA = RADIO_OVNI / 2;
	glutSolidSphere(RADIO_ESFERA, 20, 20);

	// La cubierta estará formada por los vértices de 3 circunferencias unidas:
	const GLint PUNTOS = 40;
	vector<Vec3>
		circ1 = puntosCircunferenciaDeformada(RADIO_ESFERA / 2,	PUNTOS, -RADIO_ESFERA,			false),
		circ2 = puntosCircunferenciaDeformada(RADIO_OVNI,		PUNTOS, -RADIO_ESFERA / 2,		false),
		circ3 = puntosCircunferenciaDeformada(RADIO_ESFERA,		PUNTOS, 0,						false);

	// Dibujamos luces en la cubierta en forma de esferas luminosas:
	glCallList(farol);

	vector<Vec3> faroles = puntosCircunferenciaDeformada(RADIO_OVNI * 0.9f, PUNTOS, -RADIO_ESFERA / 2 * 0.95f, false);
	
	for (Vec3& p : faroles) {
		glPushMatrix();
		glTranslatef(p.x, p.y, p.z);
		glutSolidSphere(RADIO_ESFERA / 12, 10, 10);
		glPopMatrix();
	}

	// Le aplicamos textura a la cubierta:
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, platillo);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Ahora le aplicamos material:
	glCallList(metal);

	// Finalmente, la dibujamos:
	glBegin(GL_POLYGON);
	glNormal3f(0, 0, -1);
	for (int i = PUNTOS - 1; i >= 0; i--) {
		// Mapeamos cada punto dentro del rango [0, 1]:
		GLfloat
			s = (circ1[i].x / RADIO_ESFERA) + 0.5f,
			t = (circ1[i].y / RADIO_ESFERA) + 0.5f;

		glTexCoord2f(s, t);

		glVertex3fv(circ1[i]);
	}
	glEnd();

	const GLfloat
		T_ARR = 1.0f,
		T_MED = 0.5f,
		T_ABJ = 0.0f;

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= PUNTOS; i++) {
		int id = i % PUNTOS;

		GLfloat s = (GLfloat)i / PUNTOS * 2;

		// Las normales serán los vectores que van desde el centro de la cubierta hasta cada vértice:
		Vec3
			n2 = circ2[id],
			n1 = circ1[id];

		n2.z += RADIO_ESFERA / 2;
		n1.z += RADIO_ESFERA / 2;

		glTexCoord2f(s, T_MED);
		glNormal3fv(n2);
		glVertex3fv(circ2[id]);

		glTexCoord2f(s, T_ABJ);
		glNormal3fv(n1);
		glVertex3fv(circ1[id]);
	}
	glEnd();

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= PUNTOS; i++) {
		int id = i % PUNTOS;

		GLfloat s = (GLfloat)i / PUNTOS * 2;

		Vec3
			n3 = circ3[id],
			n2 = circ2[id];

		n3.z += RADIO_ESFERA / 2;
		n2.z += RADIO_ESFERA / 2;

		glTexCoord2f(s, T_ARR);
		glNormal3fv(n3);
		glVertex3fv(circ3[id]);

		glTexCoord2f(s, T_MED);
		glNormal3fv(n2);
		glVertex3fv(circ2[id]);
	}
	glEnd();

	glPopAttrib();
	glEndList();

	return ovni;
}

// Función para generar listas de dibujo de balas:
GLuint bala()
{
	GLuint bala = glGenLists(1);
	glNewList(bala, GL_COMPILE);

	glPushMatrix();

	// Nuestras balas serán esferas, ya definidas como primitivas en GLUT:
	glutSolidSphere(RADIO_BALA, 10, 10);

	glPopMatrix();
	glEndList();

	return bala;
}

// Definición de materiales:
void materiales()
{
	metal = glGenLists(1);
	roca = glGenLists(1);
	cristal = glGenLists(1);
	oro = glGenLists(1);
	plata = glGenLists(1);
	vidrio = glGenLists(1);
	farol = glGenLists(1);

	// Si no hacemos esto, el último material se aplicará a lo que venga después:
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// > Metal:
	glNewList(metal, GL_COMPILE);
	GLfloat grisMedio[] = { 0.5f, 0.5f, 0.5f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);		// Cuando no le dé el sol, el metal se verá casi negro
	glMaterialfv(GL_FRONT, GL_DIFFUSE, grisMedio);			// Expuesto al sol, el gris se verá claro
	glMaterialfv(GL_FRONT, GL_SPECULAR, NEGRO);			// La plataforma no reflejará el sol
	glEndList();

	// > Roca:
	glNewList(roca, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, BLANCO);
	glMaterialfv(GL_FRONT, GL_SPECULAR, NEGRO);
	glEndList();

	// > Cristal:
	glNewList(cristal, GL_COMPILE);
	GLfloat azulClaro[] = { 0.0f, 0.8f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, azulClaro);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, azulClaro);
	glMaterialfv(GL_FRONT, GL_SPECULAR, BLANCO);
	glMaterialf(GL_FRONT, GL_SHININESS, 100);		// Brillo muy concentrado
	glEndList();

	// > Oro (material de las balas del jugador):
	glNewList(oro, GL_COMPILE);
	GLfloat grisMuyOscuro[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, grisMuyOscuro);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, BRONCE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, ORO);
	glMaterialf(GL_FRONT, GL_SHININESS, 10);
	glEndList();

	// > Plata (material de las balas del enemigo):
	glNewList(plata, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISCLARO);
	glMaterialfv(GL_FRONT, GL_SPECULAR, BLANCO);
	glMaterialf(GL_FRONT, GL_SHININESS, 90);
	glEndList();

	// > Vidrio (material de la cúpula de los ovnis):
	glNewList(vidrio, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, rgb(7, 30, 60)); // Azul oscuro
	glMaterialfv(GL_FRONT, GL_SPECULAR, BLANCO);
	glMaterialf(GL_FRONT, GL_SHININESS, 20);
	glEndList();

	// > Faroles (luces en el hangar y los ovnis):
	glNewList(farol, GL_COMPILE);
	const GLfloat* amarilloOscuro = rgb(180, 180, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, amarilloOscuro); // Amarillo muy oscuro
	glMaterialfv(GL_FRONT, GL_DIFFUSE, amarilloOscuro); // Amarillo oscuro
	glMaterialfv(GL_FRONT, GL_SPECULAR, ORO);
	glMaterialf(GL_FRONT, GL_SHININESS, 5);
	glEndList();

	glPopAttrib();
}

// Definición de luces:
void luces()
{
	// Luz del sol:
	GLfloat
		solAmbiente[] = { 0.5f, 0.5f, 0.5f, 1.0f },	// Poca luz ambiental (sin sol no se vería apenas nada)
		solDifuso[] = { 1.0f, 1.0f, 0.9f, 1.0f };	// Amarillo muy suave para el color dufuso

	glLightfv(GL_LIGHT0, GL_AMBIENT, solAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, solDifuso);
	glLightfv(GL_LIGHT0, GL_SPECULAR, BLANCO);		// Componente especular totalmente blanca

	// Luz del foco derecho de la nave:
	glLightfv(GL_LIGHT1, GL_AMBIENT, AMARILLO);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT1, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 15.0f);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 10.0f);

	// > Les añadimos atenuación para que se vayan debilitando con la distancia:
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.0005f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.001f);

	// Luz del foco izquierdo de la nave:
	glLightfv(GL_LIGHT2, GL_AMBIENT, AMARILLO);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT2, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 15.0f);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 10.0f);

	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.005f);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.01f);

	// Activamos iluminación y luces:
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH); // Modelo de sombreado suave
}

// Definición de texturas:
void texturas()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Texturas de la plataforma:
	for (int i = 0; i < 2; i++) {
		glGenTextures(1, &plataforma[i]);
		glBindTexture(GL_TEXTURE_2D, plataforma[i]);
		char imagen[32];
		sprintf_s(imagen, "img/plataforma/plataforma%d.jpg", i + 1);
		loadImageFile((char*)imagen);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Minificación
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Magnificación (lineal en este caso para que se vea bien la plataforma de partida)

		// > Teselamos la plataforma repitiendo su textura una y otra vez:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// Texturas de los meteoritos:
	for (int i = 0; i < NUM_TEXTURAS_ASTEROIDE; i++) {
		glGenTextures(1, &meteorito[i]);
		glBindTexture(GL_TEXTURE_2D, meteorito[i]);
		char imagen[32];
		sprintf_s(imagen, "img/asteroide/asteroide%d.jpg", i + 1);
		loadImageFile((char*)imagen);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	// Texturas del espacio:
	for (int i = 0; i < 6; i++) {
		glGenTextures(1, &espacio[i]);
		glBindTexture(GL_TEXTURE_2D, espacio[i]);
		char imagen[32];
		sprintf_s(imagen, "img/espacio/espacio%d.png", i + 1); // Las imágenes 1 y 2 son las tapas de arriba y abajo de la skybox, respectivamente, siendo las 3-6 las caras laterales
		loadImageFile((char*)imagen);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// > Habilitamos 'GL_CLAMP_TO_EDGE' para evitar costuras entre planos del cubo:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// Texturas de la cabina:
	for (int i = 0; i < 6; i++) {
		glGenTextures(1, &cabina[i]);
		glBindTexture(GL_TEXTURE_2D, cabina[i]);
		char imagen[32];
		sprintf_s(imagen, "img/cabina/cabina%d.png", i + 1);
		loadImageFile((char*)imagen);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// Textura de la cubierta de los ovnis:
	glGenTextures(1, &platillo);
	glBindTexture(GL_TEXTURE_2D, platillo);
	loadImageFile((char*)"img/otros/ovni.jpg");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPopAttrib();

	// Normalizamos los vectores normales y habilitamos las texturas:
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
}

// Imprimir controles por consola:
void controles() {
	cout << "\n=========================================" << endl;
	cout << "          CONTROLES DE VUELO             " << endl;
	cout << "=========================================" << endl;

	cout << "\n[ NAVE ]" << endl;
	cout << " - A / Z           : Acelerar / Frenar" << endl;
	cout << " - S / D           : Alabeo" << endl;
	cout << " - Raton           : Guinada y Cabeceo" << endl;
	cout << " - Espacio         : Disparar" << endl;
	cout << " - L               : Focos de la nave (On/Off)" << endl;

	cout << "\n[ VISTA ]" << endl;
	cout << " - C               : Entrar/Salir de Cabina" << endl;
	cout << " - Flechas         : Explorar cabina" << endl;
	cout << " - Enter           : Resetear vista de cabina" << endl;
	cout << " - F               : Pantalla completa" << endl;

	cout << "\n[ ESTADO ]" << endl;
	cout << " - ESC             : Salir del programa" << endl;

	cout << "\n[ OBJETIVO ]" << endl;
	cout << " Recoge todos los cristales de energia evitando asteroides" << endl;
	cout << " y las naves enemigas!!" << endl;
	cout << "=========================================\n" << endl;
}

// Inicialización:
void init()
{
	// Inicializamos la semilla de aleatoriedad global a la hora actual:
	semillaGlobal = (unsigned int)time(NULL);
	srand(semillaGlobal);

	// Configuramos el motor de color:
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST); /* Si no lo activas, no tiene en cuenta la profundidad de los objetos.
							  * Entonces, aunque uno esté detrás de otro, si el primero se dibuja después, se verá delante. */

							  // Ocultamos el cursor:
	glutSetCursor(GLUT_CURSOR_NONE);

	// Activamos el back-face culling:
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Definimos el blending (lo activaremos cuando sea necesario):
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Mostramos controles por consola:
	controles();

	// Definición de materiales:
	materiales();

	// Definición de luces:
	luces();

	// Definición de texturas:
	texturas();

	// Genemos el modelo de la plataforma:
	modeloPlataforma = hangar();

	// Generamos 10 modelos aleatorios de asteroides:
	for (int i = 0; i < NUM_MODELOS_ASTEROIDE; i++)
		modelosAsteroide[i] = asteroide();

	// Generamos el modelo de los cristales de energía:
	modeloCristal = cristalDeEnergia();

	// Generamos el modelo de las balas:
	modeloBala = bala();

	// Generamos el modelo de las naves enemigas:
	modeloOvni = ovni();

	// Reservamos memoria para los vectores de balas y enemigos (para evitar redimensionamientos):
	balas.reserve(MAX_BALAS_EN_PANTALLA);
	enemigos.reserve(MAX_ENEMIGOS_EN_PANTALLA);
}

// FUNCIÓN AUXILIAR: Funciona como 'clamp' de C++:
float restringir(float v, float inf, float sup)
{
	return max(min(v, sup), inf);
}

// Aceleración o frenado de la nave:
void acelerarOFrenar()
{
	// Calculamos el momento actual y la diferencia de tiempo (en s) desde que empezamos a pulsar:
	int ahora = glutGet(GLUT_ELAPSED_TIME);
	float delta = (ahora - iniAceleracion) / 1000.0f;

	if (vel0 >= 0)
		vel = aPulsada ? min(vel0 + acc * delta, maxVel) : max(vel0 - acc * delta, 0);

	// Si la velocidad es negativa significa que hemos colisionado y estamos retrocediendo.
	// Esperaremos a que la nave pare completamente y acto seguido restableceremos los valores que correspondan:
	else {
		vel = min(vel0 + accTrasColision * delta, 0);
		if (vel == 0) {
			vel0 = 0;
			iniAceleracion = glutGet(GLUT_ELAPSED_TIME);
		}
	}
}

// Función de activacion de los focos de la nave:
void focos()
{
	GLfloat posD[] = { 0.5f, 0.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, posD);

	GLfloat dirD[] = { 0.05f, -0.2f, -1.0f };
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dirD);

	GLfloat posI[] = { -0.5f, 0.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT2, GL_POSITION, posI);

	GLfloat dirI[] = { -0.05f, -0.2f, -1.0f };
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, dirI);
}

// Función para detectar colisiones con la plataforma inicial:
void colisionesPlataforma(Vec3& pos) {
	float margen = RADIO_NAVE;

	bool
		enX = abs(pos.x) < SEMILADO_PLATAFORMA + margen,
		enY = abs(pos.y) < SEMILADO_PLATAFORMA + margen,
		enZ = (pos.z < margen) && (pos.z > -SEMILADO_PLATAFORMA - margen);

	// Si los tres booleanos anteriores son ciertos, la nave está intentando entrar en la plataforma:
	if (enX && enY && enZ) {
		// > Obtenemos la posición donde estábamos justo antes de movernos:
		Vec3 posAnterior = camara.geto();

		// > Comprobamos por dónde intentamos entrar para bloquear ese eje específico:
		if (posAnterior.z >= margen || posAnterior.z <= -SEMILADO_PLATAFORMA - margen) {
			pos.z = (posAnterior.z >= margen) ? margen : -SEMILADO_PLATAFORMA - margen;
		}
		else {
			pos.x = posAnterior.x;
			pos.y = posAnterior.y;
		}
	}
}

// Función de movimiento de la nave:
void movimiento()
{
	// Giramos el sistema de referencia de la cámara según la guiñada, el cabeceo y el alabeo:
	// upGlobal.rotate(rad(alabeo), camara.getw());
	camara.rotar(-rad(guinyada), upGlobal); // La guiñada se aplica sobre el vector 'up' del mundo, no de la cámara
	camara.rotar(rad(cabeceo), camara.getu());
	camara.rotar(rad(alabeo), camara.getw());

	// Actualizamos la posición de la cámara en función de dónde mira y la velocidad del avión:
	Vec3
		pos = camara.geto(),
		eye = camara.getw() * (-1),
		up = camara.getv();

	pos.x += eye.x * vel * fotoDelta;
	pos.y += eye.y * vel * fotoDelta;
	pos.z += eye.z * vel * fotoDelta;

	// > Fijamos los límites del espacio:
	pos.x = restringir(pos.x, -LIM_MOV, LIM_MOV);
	pos.y = restringir(pos.y, -LIM_MOV, LIM_MOV);
	pos.z = restringir(pos.z, -LIM_MOV, LIM_MOV);

	// > Comprobamos colisiones con la plataforma inicial:
	colisionesPlataforma(pos);

	// > Actualizamos el origen de la cámara:
	camara.seto(pos);

	// > Fijamos los parámetros de vista de la cámara:
	gluLookAt(pos.x, pos.y, pos.z,
		pos.x + eye.x, pos.y + eye.y, pos.z + eye.z,
		up.x, up.y, up.z);

	// Nos guardamos el centro de la caja donde está la nave:
	centroCaja = Vec3(	round(pos.x / DIM_CAJA_ASTEROIDES),
						round(pos.y / DIM_CAJA_ASTEROIDES),
						round(pos.z / DIM_CAJA_ASTEROIDES));
}

// Función de generación del fondo espacial:
void generarFondo()
{
	GLint l = DIM_FONDO / 2; // Mitad de lo que valdrá un lado de la skybox

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Deshabilitamos el Z-buffer para que, si quisiéramos dibujar algo detrás de la skybox, se siga viendo (es decir, queremos dar ese efecto de que el fondo está en el infinito):
	glDisable(GL_DEPTH_TEST);

	// Deshabilitamos también las luces para que la iluminación de los elementos de la escena no afecte al fondo:
	glDisable(GL_LIGHTING);

	// Generamos el cubo:
	generarCubo(l, espacio);

	glPopAttrib();
}

// Función de generación del escenario:
void generarPlataforma()
{
	glCallList(modeloPlataforma);
}

// FUNCIÓN AUXILIAR: Creamos un generador MT19937 a partir de unas coordenadas dadas:
mt19937 crearGenerador(Vec3 coord)
{
	// Solo lo declaramos la primera vez que llamemos a esta función. A partir de ahí, solo actualizaremos la semilla (por motivos de eficiencia):
	static mt19937 generador;

	// Calculamos una semilla de aleatoriedad local en función de las coordenadas dadas en 'coord':
	unsigned int semilla = (unsigned int)((int)coord.x * 73856093 ^ (int)coord.y * 19349663 ^ (int)coord.z * 83492791);

	// Combinamos la semilla local con la global para que cada partida sea distinta:
	semilla ^= semillaGlobal;

	generador.seed(semilla);

	return generador;
}

// FUNCIÓN AUXILIAR: Genera un vector de 'k' números aleatorios (sin repetición) dentro del rango ['min', 'max']:
vector<int> listaAleatoria(int k, int min, int max, mt19937& generador)
{
	// Si 'min' es mayor que 'max', los intercambiamos:
	if (min > max) {
		swap(min, max);
	}

	int tam = max - min + 1;

	// Si no podemos tener 'k' números sin repetición, al menos generaremos todos los que podamos:
	if (k > tam) k = tam;

	// Creamos un vector de 'tam' elementos, que recoja todos los valores enteros en ['min', 'max']:
	vector<int> valores;
	valores.reserve(tam);

	for (int i = min; i <= max; i++) {
		valores.push_back(i);
	}

	/* Vamos a aplicar el algoritmo de Fisher-Yates:
	 *
	 * 1. Empezamos tomando una posición aleatoria del vector 'valores'.
	 * 2. Metemos el valor que haya en esa posición dentro de otro vector de tamaño 'k' que será el que retornaremos al final.
	 * 3. Acto seguido, intercambiamos dicha posición con la última de 'valores'.
	 * 4. En la siguiente iteración, decrementaremos en 1 el rango de búsqueda, de modo que, si al principio generamos una posición entre 0 y 1000,
	 *   ahora lo haremos entre 0 y 999.
	 * 5. Una vez decrementado el rango de búsqueda, repetimos los pasos 1-3 hasta haber obtenido 'k' valores.
	 */
	vector<int> sol;
	sol.reserve(k);

	int rango = tam;

	for (int i = 0; i < k; i++) {
		rango--;
		int pos = enteroAleatorio(0, rango, generador);
		sol.push_back(valores[pos]);

		swap(valores[pos], valores[rango]);
	}

	return sol;
}

// Posicionamiento de los asteroides:
vector<Vec3> posAsteroides(Vec3 centroCaja, mt19937& generador)
{
	/* Nuestros asteroides estarán distribuidos aleatoriamente dentro de una caja de tamaño 'DIM_CAJA_ASTEROIDES' ^ 3.
	 * Dividiremos esta caja en subcajas de tamaño 'DIM_SUBCAJA_ASTEROIDES' ^ 3.
	 *
	 * De esas subcajas, rellenaremos con un asteroide 'NUM_ASTEROIDES_CAJA' de ellas, mostrando como resultado 'k' asteroides
	 * que no colisionarán entre sí: */
	static const int
		SUBCAJAS_1D = DIM_CAJA_ASTEROIDES / DIM_SUBCAJA_ASTEROIDES,
		SUBCAJAS_2D = SUBCAJAS_1D * SUBCAJAS_1D,
		NUM_SUBCAJAS = pow(SUBCAJAS_1D, 3),
		MAX_RUIDO = DIM_SUBCAJA_ASTEROIDES - MAX_RADIO_ASTEROIDES * 2 * MAX_ESCALADO_ASTEROIDES;

	// Vamos a generar asteroides solo en subcajas pares (para poder poner cristales en las impares):
	vector<int> subcajas = listaAleatoria(NUM_ASTEROIDES_CAJA, 0, ceil(NUM_SUBCAJAS / 2.0f) - 1, generador);

	// Transformamos los índices de las subcajas en coordenadas:
	static const GLfloat COORDS_SUP_IZQ = (DIM_CAJA_ASTEROIDES - DIM_SUBCAJA_ASTEROIDES) / 2.0f;

	static const Vec3 ESQUINA_SUP_IZQ(-COORDS_SUP_IZQ, COORDS_SUP_IZQ, COORDS_SUP_IZQ);
	vector<Vec3> centros;
	centros.reserve(NUM_ASTEROIDES_CAJA);

	for (int subcaja : subcajas) {
		subcaja *= 2;

		// Añadiremos un ruido aleatorio para que los asteroides no estén exactamente en el centro de sus subcajas:
		Vec3 ruido(	realAleatorio(-MAX_RUIDO, MAX_RUIDO, generador),
					realAleatorio(-MAX_RUIDO, MAX_RUIDO, generador),
					realAleatorio(-MAX_RUIDO, MAX_RUIDO, generador));

		Vec3 centro(ESQUINA_SUP_IZQ.x + (subcaja % SUBCAJAS_1D) * DIM_SUBCAJA_ASTEROIDES,
					ESQUINA_SUP_IZQ.y - (subcaja % SUBCAJAS_2D / SUBCAJAS_1D) * DIM_SUBCAJA_ASTEROIDES,
					ESQUINA_SUP_IZQ.z - (subcaja / SUBCAJAS_2D) * DIM_SUBCAJA_ASTEROIDES);

		centro = centro + ruido + centroCaja;

		float radioSeguridad = SEMILADO_PLATAFORMA + MAX_RADIO_ASTEROIDES;

		// Si el asteroide está cerca del origen de coordenadas (0, 0, 0), no lo generamos (si no se solaparía con la plataforma):
		if (abs(centro.x) < radioSeguridad &&
			abs(centro.y) < radioSeguridad &&
			abs(centro.z) < radioSeguridad)
			continue;

		centros.push_back(centro);
	}

	return centros;
}

// Función para detectar colisiones entre dos objetos esféricos dados sus centros y sus radios:
bool colision(const Vec3& pos1, GLfloat r1, const Vec3& pos2, GLfloat r2)
{
	Vec3 dist = pos2 - pos1;
	GLfloat distCuadrado = dist.dot(dist),
		radioSuma = r1 + r2;

	return distCuadrado < radioSuma * radioSuma;
}

// FUNCIÓN AUXILIAR: Realizamos las asignaciones necesarias para que la nave retroceda:
void setupRetroceso()
{
	// Aplicamos un pequeño retroceso para que no se quede en un bucle de colisionado:
	Vec3 retroceso = camara.geto() + camara.getw() * 2;
	camara.seto(retroceso);

	// Actualizamos 'vel0' e 'iniAceleracion' para desencadenar el retroceso en la función 'acelerarOFrenar()':
	vel0 = -abs(vel / 2); // Ponemos el abs() y luego el menos por si se producen 2 colisiones seguidas
	iniAceleracion = glutGet(GLUT_ELAPSED_TIME);

	// Si hemos retrocedido, significa que hemos colisionado contra un objeto estelar o una nave enemiga, así que restamos 'DANYO_COLISION' puntos de vida:
	puntosDeVida -= DANYO_COLISION;
}

// FUNCIÓN AUXILIAR: Realizamos las asignaciones necesarias para poner la pantalla en rojo:
void setupPantallaRoja()
{
	tintarPantalla = true;
	iniTintado = glutGet(GLUT_ELAPSED_TIME);
}

// Función de generación de los asteroides:
void generarAsteroides()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPolygonMode(GL_FRONT, GL_FILL);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glCallList(roca);

	// Calculamos los centros de las cajas vecinas a aquella en la que se encuentre la cámara:
	Vec3 posCamara = camara.geto();

	for (int i : {-1, 0, 1})
		for (int j : {-1, 0, 1})
			for (int k : {-1, 0, 1}) {
				Vec3 centroCajaVecina = (centroCaja + Vec3(i, j, k)) * DIM_CAJA_ASTEROIDES;

				// Inicializamos un generador basado en el Marsenne Twister usando las coordenadas de 'centroCajaVecina':
				mt19937 generador = crearGenerador(centroCajaVecina);

				// Generamos las posiciones de los centros de los asteroides de la caja:
				vector<Vec3> posiciones = posAsteroides(centroCajaVecina, generador);

				// Generamos un asteroide para cada posición:
				for (Vec3& pos : posiciones) {
					glPushMatrix();

					// Trasladamos cada asteroide a los centros generados:
					glTranslatef(pos.x, pos.y, pos.z);

					// Generamos un eje de rotación aleatorio para cada asteroide:
					Vec3 eje(	realAleatorio(-1.0f, 1.0f, generador),
								realAleatorio(-1.0f, 1.0f, generador),
								realAleatorio(-1.0f, 1.0f, generador));

					// Normalizamos el eje:
					if (eje.norm() > 0.001f) eje.normalize();

					// Si el eje era muy cercano a (0, 0, 0), usaremos el eje (0, 1, 0) por defecto:
					else eje = Vec3(0, 1, 0);

					// Calculamos un ángulo base de rotación:
					GLfloat anguloBase = realAleatorio(0, 360, generador);

					// Actualizamos dicho ángulo base con el tiempo:
					GLfloat delta = glutGet(GLUT_ELAPSED_TIME) * realAleatorio(0.01f, 0.03f, generador);
					glRotatef(fmodf(anguloBase + delta, 360.0f), eje.x, eje.y, eje.z);

					// Aplicamos un escalado aleatorio:
					GLfloat
						escaladoZ = realAleatorio(MIN_ESCALADO_ASTEROIDES, MAX_ESCALADO_ASTEROIDES, generador),
						radio = realAleatorio(MIN_RADIO_ASTEROIDES, MAX_RADIO_ASTEROIDES, generador);
					glScalef(radio, radio, radio * escaladoZ);

					// Escogemos aleatoriamente una de las texturas de asteroide disponibles:
					glBindTexture(GL_TEXTURE_2D, meteorito[enteroAleatorio(0, NUM_TEXTURAS_ASTEROIDE - 1, generador)]);

					// Llamamos a la lista de dibujo de uno de los modelos de asteroides de forma aleatoria:
					glCallList(modelosAsteroide[enteroAleatorio(0, NUM_MODELOS_ASTEROIDE - 1, generador)]);

					glPopMatrix();

					// Comprobamos si se dan colisiones entre la nave y el asteroide actual:
					GLfloat radioColision = radio * max(1.0f, escaladoZ);
					if (colision(posCamara, RADIO_NAVE, pos, radioColision)) {
						// Hacemos que la nave retroceda:
						setupRetroceso();

						// Tintamos la pantalla de rojo indicando daño:
						setupPantallaRoja();
					}

					// También comprobamos si este colisiona con alguna bala:
					for (Bala& b : balas)
						if (b.activa && colision(b.pos, RADIO_BALA, pos, radioColision))
							b.activa = false;
				}
			}

	glPopAttrib();
}

// Posicionamiento de los cristales de energía:
vector<Vec3> posCristales(Vec3 centroCaja, mt19937& generador)
{
	static const int
		SUBCAJAS_1D = DIM_CAJA_ASTEROIDES / DIM_SUBCAJA_ASTEROIDES,
		SUBCAJAS_2D = SUBCAJAS_1D * SUBCAJAS_1D,
		NUM_SUBCAJAS = pow(SUBCAJAS_1D, 3),
		MAX_RUIDO = (DIM_SUBCAJA_ASTEROIDES - MAX_RADIO_ASTEROIDES) / 2.0f;

	// Vamos a generar cristales solo en subcajas impares (para poder poner asteroides en las pares):
	vector<int> subcajas = listaAleatoria(	NUM_CRISTALES_CAJA + 1 /* Este valor adicional será la posición de la nave enemiga en la caja */,
											0, ceil(NUM_SUBCAJAS / 2.0f) - 1, generador);

	// Transformamos los índices de las subcajas en coordenadas:
	static const GLfloat COORDS_SUP_IZQ = (DIM_CAJA_ASTEROIDES - DIM_SUBCAJA_ASTEROIDES) / 2.0f;

	static const Vec3 ESQUINA_SUP_IZQ(-COORDS_SUP_IZQ, COORDS_SUP_IZQ, COORDS_SUP_IZQ);
	vector<Vec3> centros;
	centros.reserve(NUM_CRISTALES_CAJA);

	for (int subcaja : subcajas) {
		subcaja = subcaja * 2 + 1;

		Vec3 centro(ESQUINA_SUP_IZQ.x + (subcaja % SUBCAJAS_1D) * DIM_SUBCAJA_ASTEROIDES,
					ESQUINA_SUP_IZQ.y - (subcaja % SUBCAJAS_2D / SUBCAJAS_1D) * DIM_SUBCAJA_ASTEROIDES,
					ESQUINA_SUP_IZQ.z - (subcaja / SUBCAJAS_2D) * DIM_SUBCAJA_ASTEROIDES);

		centro += centroCaja;

		centros.push_back(centro);
	}

	return centros;
}

// Función de generación de los cristales de energía:
void generarCristales()
{
	// Calculamos los centros de las cajas vecinas a aquella en la que se encuentre la cámara:
	for (int i : {-1, 0, 1})
		for (int j : {-1, 0, 1})
			for (int k : {-1, 0, 1}) {
				Vec3 centroCajaVecina = (centroCaja + Vec3(i, j, k));

				// Si la caja vecina actual se sale del número de cinturones de cristales prefijados en 'NUM_CINTURONES_CRISTALES', no colocamos cristales en ella:
				if (abs(centroCajaVecina.x) >= NUM_CINTURONES_CRISTALES ||
					abs(centroCajaVecina.y) >= NUM_CINTURONES_CRISTALES ||
					abs(centroCajaVecina.z) >= NUM_CINTURONES_CRISTALES)
					continue;

				// Si el cristal de esta caja ya está recogido, tampoco lo generamos:
				unsigned int
					semilado = LADO_CINTURONES / 2,
					indiceCaja =	((int)centroCajaVecina.x + semilado) +
									((int)centroCajaVecina.y + semilado) * LADO_CINTURONES +
									((int)centroCajaVecina.z + semilado) * LADO_CINTURONES * LADO_CINTURONES,
					indiceCristalesCaja = indiceCaja * NUM_CRISTALES_CAJA;

				centroCajaVecina = centroCajaVecina * DIM_CAJA_ASTEROIDES;

				// Inicializamos un generador basado en el Marsenne Twister usando las coordenadas de 'centroCajaVecina':
				mt19937 generador = crearGenerador(centroCajaVecina);

				// Generamos las posiciones de los centros de los cristales de la caja:
				vector<Vec3> posiciones = posCristales(centroCajaVecina, generador);

				// Guardamos la posición de la nave enemiga en esta caja (si es múltiplo de 'FRECUENCIA_ENEMIGOS'):
				if (!enemigosEnCaja[indiceCaja] && indiceCaja % FRECUENCIA_ENEMIGOS == 0) {
					enemigos.emplace_back(posiciones.back());

					// Lo activamos cuando ya se ha generado previamente un enemigo en la caja actual:
					enemigosEnCaja[indiceCaja] = true;
				}

				// Generamos un cristal en cada posición:
				for (int i = 0; i < posiciones.size() - 1; i++) {
					Vec3 &pos = posiciones[i];

					// Comprobamos si el cristal ya ha sido recogido:
					unsigned int indiceCristal = indiceCristalesCaja + i % NUM_CRISTALES_CAJA;

					if (cristalesRecogidos[indiceCristal])
						continue;

					// Si no ha sido recogido, comprobamos si la nave está colisionando con él en este momento:
					if (colision(camara.geto(), RADIO_NAVE, pos, RADIO_CRISTAL)) {
						cristalesRecogidos[indiceCristal] = true;
						numCristalesRecogidos++;
						puntosDeVida = min(puntosDeVida + CURACION_CRISTAL, 100); // Si lo hemos recogido, nos curamos 'CURACION_CRISTAL' puntos de vida
						continue;
					}

					glPushMatrix();

					// Trasladamos cada cristal a los centros generados:
					glTranslatef(pos.x, pos.y, pos.z);

					// Llamamos a la lista de dibujo de los cristales:
					glCallList(modeloCristal);

					glPopMatrix();
				}
			}
}

// Función para actualizar las partículas:
void actualizarParticulas(float delta) {
	for (Particula& p : particulas) {
		if (!p.activa) continue;

		p.pos += p.vel * delta;
		p.vida -= delta * 1.0f;

		if (p.vida <= 0)
			p.activa = false;
	}
}

// Función para dibujar las partículas activas:
void dibujarParticulas() {
	// Deshabilitamos la iluminación y las texturas:
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	// Activamos el blending:
	glEnable(GL_BLEND);

	// Dibujamos las partículas como puntos:
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	for (Particula& p : particulas) {
		if (!p.activa) continue;

		// Cambiamos el color según la vida (de amarillo a rojo):
		glColor4f(1.0f, p.vida, 0.0f, p.vida);
		glVertex3f(p.pos.x, p.pos.y, p.pos.z);
	}
	glEnd();
	
	glPopAttrib();
}

// Función para generar explosiones de partículas:
void explosion(Vec3& posicion, unsigned int numParticulas) {
	int creadas = 0;

	for (int i = 0; i < MAX_PARTICULAS && creadas < numParticulas; i++) {
		// Exploramos tanto las activas/inactivas que ya hay en el vector como las posiciones entre esas y MAX_PARTICULAS:
		if (i >= particulas.size()) particulas.push_back(Particula());

		// Usamos las partículas inactivas para generar la explosión:
		if (!particulas[i].activa) {
			particulas[i].pos = posicion;

			// Les damos una velocidad aleatoria en todas direcciones:
			particulas[i].vel = Vec3(random(-5.0f, 5.0f), random(-5.0f, 5.0f), random(-5.0f, 5.0f));

			// Las activamos y aumentamos el número de partículas creadas:
			particulas[i].vida = 1.0f;
			particulas[i].activa = true;
			creadas++;
		}
	}
}

// Función de generación de las naves enemigas:
void generarEnemigos()
{
	int ahora = glutGet(GLUT_ELAPSED_TIME);
	float delta = fmodf(ahora / 1000.0f, 360.0f);
	Vec3 posCamara = camara.geto();
	for (Enemigo& e : enemigos) {
		// Solo mostramos los enemigos que entren dentro de las cajas de asteroides que se muestran actualmente:
		if (!colision(posCamara, RADIO_NAVE, e.pos, RADIO_OVNI + DIM_CAJA_ASTEROIDES * 1.5f))
			continue;

		glPushMatrix();
		glTranslatef(e.pos.x, e.pos.y, e.pos.z);
		glRotatef(delta * gradosPorSegundo / 2, 0, 0, 1);
		glCallList(modeloOvni);
		glPopMatrix();
	}
}

// Función para controlar las colisiones de los enemigos con las naves y balas:
void colisionesEnemigos(float delta)
{
	Vec3 posCamara = camara.geto();
	for (int i = 0; i < enemigos.size(); i++) {
		Enemigo& e = enemigos[i];

		// Solo comprobamos los enemigos que entren dentro de las cajas de asteroides que se muestran actualmente:
		if (!colision(posCamara, RADIO_NAVE, e.pos, RADIO_OVNI + DIM_CAJA_ASTEROIDES * 1.5f))
			continue;

		// Comprobamos si la nave está dentro del campo de visión del enemigo, en cuyo caso lo trasladamos hacia ella:
		if (colision(posCamara, RADIO_NAVE, e.pos, RADIO_OVNI + CAMPO_DE_VISION)) {
			// Solo normalizamos el vector de dirección si no es (0, 0, 0):
			Vec3 dif = posCamara - e.pos;
			GLfloat dist = dif.norm();
			if (dist > 0.001f)
				e.dir = dif / dist;
			else
				e.dir = Vec3(0, 0, 0);

			e.pos += e.dir * e.vel * delta;

			// Comprobamos además si la nave ha colisionado con el enemigo:
			if (colision(posCamara, RADIO_NAVE, e.pos, RADIO_OVNI)) {
				// Hacemos que la nave y el enemigo retrocedan:
				setupRetroceso();
				e.pos = e.pos - e.dir * 2;

				// Tintamos la pantalla de rojo indicando daño:
				setupPantallaRoja();
			}
		}

		// Comprobamos las colisiones con las balas:
		for (Bala& b : balas) {
			if (!b.activa || !b.delJugador) continue;

			if (colision(b.pos, RADIO_BALA, e.pos, RADIO_OVNI)) {
				e.pv -= DANYO_DISPARO;

				// Desactivamos la bala que le ha dado:
				b.activa = false;

				// Si el enemigo se queda sin vida, lo borramos de la lista:
				if (e.pv <= 0) {
					explosion(e.pos, 200);
					e.activo = false;
					break;
				}
				else {
					Vec3 posExp = e.pos - b.dir * RADIO_OVNI;
					explosion(posExp, 20);
				}
			}
		}

		// Si hemos matado al enemigo actual, lo eliminamos de la lista de enemigos activos:
		if (!e.activo) {
			swap(enemigos[i], enemigos.back());
			enemigos.pop_back();

			// > Decrementamos 'i' para procesar el nuevo enemigo que hemos traído:
			i--;
		}
	}
}

// Función de generación de la textura de la cabina de la nave (solidaria a la cámara):
void generarCabina()
{
	// Deshabilitamos luces para que la cabina no salga iluminada:
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);

	// Deshabilitamos también el test de profundidad para que la cabina se dibuje delante del todo:
	glDisable(GL_DEPTH_TEST);

	// Habilitamos el blending:
	glEnable(GL_BLEND);

	// Apilamos la cámara perspectiva actual temporalmente y cargamos otra local a la cabina:
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// > Vamos a usar una perspectiva (pese a las recomendaciones del boletín) para hacer más natural la exploración de la cabina:
	gluPerspective(90, aspect, 0.1, 10);

	// Cargamos de nuevo la matriz del modelo:
	glMatrixMode(GL_MODELVIEW);		
	glPushMatrix();
	glLoadIdentity();
	
	// Rotamos la cámara local a la cabina:
	glRotatef(anguloExpEjeX, 1, 0, 0); // Rotación sobre el eje X (mirar arriba/abajo)
	glRotatef(anguloExpEjeY, 0, 1, 0); // Rotación sobre el eje Y (mirar hacia los lados)

	// Como 'generarCubo()' está en el sistema de coordenadas de la cámara global (el eje Z es "arriba"),
	// debemos realizar un giro "correctivo" de -90º para la cámara local respecto del eje X:
	glRotatef(-90.0f, 1, 0, 0);

	// Generamos el cubo de la cabina con el tamaño especificado en 'DIM_CABINA':
	generarCubo(DIM_CABINA, cabina);

	// Restauramos la matriz del modelo anterior:
	glPopMatrix();

	// Restauramos también las matrices de proyección y textura anteriores:
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Volvemos a la matriz del modelo:
	glMatrixMode(GL_MODELVIEW);

	// Finalmente, restauramos los atributos:
	glPopAttrib();
}

// Tintamos la pantalla de rojo cuando la nave ha sufrido daño:
void pantallaRoja()
{
	// Habilitamos/deshabilitamos todo lo necesario:
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	// Cargamos la cámara perspectiva actual y nos la guardamos temporalmente:
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// Definimos una cámara ortográfica cuyo volumen sea un cubo centrado en el origen de la cámara de dimensiones 2x2x2:
	glOrtho(-1, 1, -1, 1, -1, 1);

	// Cargamos la matriz del modelo actual y nos la guardamos temporalmente:
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	// Dibujamos un rectángulo rojo frente a la cámara:
	int ahora = glutGet(GLUT_ELAPSED_TIME);
	GLfloat
		delta = (ahora - iniTintado) / 1000.0f,
		opacidadRojo = max(opacidadRojoIni - desvanecimiento * delta, 0);

	glColor4f(1.0f, 0.0f, 0.0f, opacidadRojo);
	glBegin(GL_QUADS);
	glVertex3f(-1, -1, 0);
	glVertex3f( 1, -1, 0);
	glVertex3f( 1,  1, 0);
	glVertex3f(-1,  1, 0);
	glEnd();

	// Si el color ya se ha desvanecido, dejamos de llamar a la función:
	if (opacidadRojo == 0)
		tintarPantalla = false;

	// Restauramos los atributos y matrices correspondientes:
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

// Función para controlar los disparos de la nave:
void disparar()
{
	// Momento (en ms) donde se produjo el último disparo:
	static int ultimoDisparo = 0;

	// Si el tiempo entre disparos es mayor que 'SEG_ENTRE_DISPAROS', disparamos:
	int ahora = glutGet(GLUT_ELAPSED_TIME);
	float delta = (ahora - ultimoDisparo) / 1000.0f;

	if (delta >= SEG_ENTRE_DISPAROS) {
		balas.emplace_back(camara.geto(), camara.getw() * (-1));
		ultimoDisparo = ahora;
	}
}

// Función para actuar la posición de las balas y eliminar las que ya no estén activas:
void actualizarBalas(float delta)
{
	for (int i = 0; i < balas.size(); i++) {
		Bala& b = balas[i];

		// Actualizamos la posición de la bala:
		b.pos += b.dir * b.vel * delta;

		// Comprobamos si ya se le ha agotado el tiempo de vida en pantalla:
		int ahora = glutGet(GLUT_ELAPSED_TIME);
		float segundos = (ahora - b.tiempo) / 1000.0f;
		if (segundos > MAX_SEG_BALA_ACTIVA || !b.activa) {
			// > Si se ha acabado, la cambiamos por la última del vector y la eliminamos:
			swap(balas[i], balas.back());
			balas.pop_back();

			// > Decrementamos 'i' para procesar la nueva bala que hemos traído:
			i--;
		}
	}
}

// Función para mostrar por pantalla las balas activas:
void mostrarBalas()
{
	for (Bala& b : balas) {
		if (!b.activa) continue;

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPolygonMode(GL_FRONT, GL_FILL);
		glDisable(GL_TEXTURE_2D);

		glPushMatrix();
		glTranslatef(b.pos.x, b.pos.y, b.pos.z);

		// Las balas del jugador serán doradas y las del enemigo plateadas:
		glCallList(b.delJugador ? oro : plata);
		glCallList(modeloBala);

		glPopMatrix();
		glPopAttrib();
	}
}

// Función para invocar la pantalla del final del juego:
void finDelJuego() {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (puntosDeVida > 0)	glColor3f(0.0f, 0.2f, 0.1f);
	else					glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	float
		cX = ancho / 2.0f,
		cY = alto / 2.0f;

	// Texto:
	if (puntosDeVida > 0) texto(cX - 280, cY + 20, (char*)"ENHORABUENA, HAS RECOGIDO TODOS LOS CRISTALES", AMARILLO, GLUT_BITMAP_TIMES_ROMAN_24, false);
	else texto(cX - 80, cY + 20, (char*)"F I N  D E L  J U E G O", ROJO, GLUT_BITMAP_TIMES_ROMAN_24, false);

	texto(cX - 30, cY - 40, (char*)"Presiona 'ESC' para salir", BLANCO, GLUT_BITMAP_HELVETICA_12, false);

	glPopAttrib();
	glutSwapBuffers();
}

void estadisticas() {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// Formateamos las cadenas con los valores actuales:
	sprintf_s(bufferVida, "Vida: %d %%", (int)puntosDeVida);
	sprintf_s(bufferCristales, "Cristales: %d / %d", numCristalesRecogidos, TOTAL_CRISTALES);

	texto(20, alto - 40, bufferVida, BLANCO, GLUT_BITMAP_HELVETICA_18, false);
	texto(20, alto - 70, bufferCristales, BLANCO, GLUT_BITMAP_HELVETICA_18, false);

	glPopAttrib();
}

// Función de atención al evento de dibujo
void display()
{
	// Restablecemos los búferes de color y profundidad:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Cargamos la matriz del modelo:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (gameOver) {
		finDelJuego();
		return;
	}

	// Activamos los focos de la nave (si cabe):
	if (focosActivos) focos();

	// Llamamos a la función del movimiento de la nave:
	movimiento();

	// Generamos el fondo espacial:
	generarFondo();

	// Activamos la luz del sol:
	/* > Si la colocamos, por ejemplo en la posición (0, 0, 1), significa que los rayos irán en la dirección del vector
	 * que va desde (0, 0, 1) hasta (0, 0, 0). Es decir, la luz irá desde arriba hacia abajo. 
	 *
	 * En nuestro caso, también vamos a darle valores distintos de 0 a las componentes X e Y para que no venga exactamente de arriba: */
	static GLfloat posLuzSolar[] = { 0.1f, -0.3f, 1, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, posLuzSolar);

	// Generamos la plataforma:
	generarPlataforma();

	// Generamos los asteroides:
	generarAsteroides();

	// Generamos los cristales de energía:
	generarCristales();

	// Mostramos las balas:
	mostrarBalas();

	// Generamos las naves enemigas:
	generarEnemigos();

	// Dibujamos las partículas de las explosiones:
	dibujarParticulas();

	// Mostramos (si cabe) la cabina de la nave:
	if (mostrarCabina) generarCabina();

	// Tintamos (si cabe) la pantalla de rojo al recibir daño:
	if (tintarPantalla) pantallaRoja();

	// Mostramos los PV y cristales que llevamos:
	estadisticas();

	glutSwapBuffers();
}

// Actualización de la escena:
void update()
{
	// Nos guardamos el tiempo transcurrido al momento de lanzarse este fotograma:
	int fotoActual = glutGet(GLUT_ELAPSED_TIME);

	// > Solo en el primer fotograma, la diferencia entre el fotograma actual y el anterior será de 0:
	static int fotoAnterior = fotoActual;
	fotoDelta = (fotoActual - fotoAnterior) / 1000.0f;
	fotoAnterior = fotoActual;

	// En cada fotograma devolvemos el ratón al centro de la pantalla:
	glutWarpPointer(ancho / 2, alto / 2);

	// En cada fotograma aceleramos o frenamos, y aplicamos un alabeo:
	if (aPulsada || zPulsada || vel0 < 0) acelerarOFrenar();
	if (sPulsada || dPulsada) alabeo = gradosPorSegundo * fotoDelta * (sPulsada ? 1 /* Sentido antihorario */ : -1 /* Sentido horario */);

	// También actualizamos, si cabe, el punto de vista dentro de la cabina:
	if (mostrarCabina) {
		if (arrPulsada) anguloExpEjeX -= gradosPorSegundo * fotoDelta;
		if (abjPulsada) anguloExpEjeX += gradosPorSegundo * fotoDelta;
		if (izqPulsada) anguloExpEjeY -= gradosPorSegundo * fotoDelta;
		if (derPulsada) anguloExpEjeY += gradosPorSegundo * fotoDelta;

		// > Restringimos el ángulo de exploración hacia arriba/abajo en 90º:
		anguloExpEjeX = restringir(anguloExpEjeX, -90.0f, 90.0f);

		// > Restablecemos del punto de vista inicial si cabe:
		if (restablecerVistaCabina) {
			anguloExpEjeX = 0.0f;
			anguloExpEjeY = 0.0f;
			restablecerVistaCabina = false;
		}
	}

	// Actualizamos el vector de balas en pantalla:
	actualizarBalas(fotoDelta);

	// Comprobamos las colisiones de los enemigos:
	colisionesEnemigos(fotoDelta);

	// Actualizamos las partículas:
	actualizarParticulas(fotoDelta);

	// Comprobamos si ya tenemos todos los cristales:
	if (numCristalesRecogidos >= NUM_CRISTALES_CAJA * NUM_CAJAS || puntosDeVida <= 0) {
		gameOver = true;
	}

	// Refrescamos la ventana:
	glutPostRedisplay();
}

// Funcion de atencion al redimensionamiento
void reshape(GLint w, GLint h)
{
	// Definir el marco:
	// --- Los dos primeros parámetros son la esquina inferior izquierda, y los dos últimos la sup. derecha.
	glViewport(0, 0, w, h); // Lienzo entero.
	// glViewport(0, h / 2, w / 2, h / 2); // Cuadrante superior izquierdo del lienzo.
	aspect = (float)w / (float)h;

	ancho = w;
	alto = h;

	// Si cambia el tamaño de la ventana, tenemos que volver a centrar el ratón:
	glutWarpPointer(ancho / 2, alto / 2);

	// Definir la cámara:
	glMatrixMode(GL_PROJECTION); // Selecciono la matriz donde voy a almacenar los parámetros que definen mi cámara.
	glLoadIdentity(); // Cargo la identidad en esta matriz para que en cada frame se refresque y no se vayan multiplicando sus valores.

	// Cámara perspectiva:
	gluPerspective(80, aspect, 0.1, 3000);
}

// Función de atención al movimiento pasivo del ratón:
void ratonMovido(int x, int y)
{
	// Calculamos el centro de la ventana:
	float
		cx = ancho / 2,
		cy = alto / 2;

	// Calculamos los grados de guiñada en función de cuánto se ha movido el ratón en el eje X respecto del centro:
	guinyada = fmod(pixelesAGrados * (x - cx), 360.0f);

	// Calculamos los grados de cabeceo en función de cuánto se ha movido el ratón en el eje Y respecto del centro:
	cabeceo = pixelesAGrados * (cy - y);
}

// Función de atención al pulsado de teclas:
void onKeyPressed(unsigned char tecla, int x, int y)
{
	switch (tecla) {

		// Tecla A - Acelerar:
		case 'a':
		case 'A':
			// Registramos la velocidad y el momento en que hemos empezado a presionar la tecla:
			if (!aPulsada && vel0 >= 0) {
				iniAceleracion = glutGet(GLUT_ELAPSED_TIME);
				vel0 = vel;
			}

			aPulsada = true;
			zPulsada = false;
			break;

		// Tecla Z - Frenar:
		case 'z':
		case 'Z':
			if (!zPulsada && vel0 >= 0) {
				iniAceleracion = glutGet(GLUT_ELAPSED_TIME);
				vel0 = vel;
			}

			zPulsada = true;
			aPulsada = false;
			break;

		// Tecla S - Alabeo antihorario:
		case 's':
		case 'S':
			if (!sPulsada) {
				iniAlabeo = glutGet(GLUT_ELAPSED_TIME);
			}

			sPulsada = true;
			dPulsada = false;
			break;

		// Tecla D - Alabeo horario:
		case 'd':
		case 'D':
			if (!dPulsada) {
				iniAlabeo = glutGet(GLUT_ELAPSED_TIME);
			}

			dPulsada = true;
			sPulsada = false;
			break;

		// Tecla F - Activar/desactivar pantalla completa:
		case 'f':
		case 'F':
			glutFullScreenToggle();
			break;

		// Tecla L - Activar/desactivar focos de la nave:
		case 'l':
		case 'L':
			focosActivos = !focosActivos;
			if (focosActivos) {
				glEnable(GL_LIGHT1);
				glEnable(GL_LIGHT2);
			}
			else {
				glDisable(GL_LIGHT1);
				glDisable(GL_LIGHT2);
			}
			break;

		// Tecla ESPACIO - Disparar:
		case ' ':
			disparar();
			break;

		// Tecla C - Activar/desactivar cabina de la nave:
		case 'c':
		case 'C':
			mostrarCabina = !mostrarCabina;

			// > Cada vez que activemos la cabina, restablecemos su punto de vista al inicial:
			if (mostrarCabina) restablecerVistaCabina = true;
			break;

		// Tecla ENTER - Restablecer posición de la vista dentro de la cabina:
		case 13:
			restablecerVistaCabina = true;
			break;

		// Tecla ESC - Cerrar ventana:
		case 27: exit(0);

	}
}

// Función de atención al soltado de teclas:
void onKeyReleased(unsigned char tecla, int x, int y)
{
	switch (tecla) {
		case 'a':
			aPulsada = false;
			break;

		case 'z':
			zPulsada = false;
			break;

		case 's':
			sPulsada = false;
			alabeo = 0;
			break;

		case 'd':
			dPulsada = false;
			alabeo = 0;
			break;
	}
}

// Función de atención al pulsado de teclas especiales:
void onSpecialPressed(int tecla, int x, int y)
{
	switch (tecla) {
		case GLUT_KEY_UP:    arrPulsada = true; break;
        case GLUT_KEY_DOWN:  abjPulsada = true; break;
        case GLUT_KEY_LEFT:  izqPulsada = true; break;
        case GLUT_KEY_RIGHT: derPulsada = true; break;
	}
}

// Función de atención al soltado de teclas especiales:
void onSpecialReleased(int tecla, int x, int y)
{
	switch (tecla) {
		case GLUT_KEY_UP:    arrPulsada = false; break;
		case GLUT_KEY_DOWN:  abjPulsada = false; break;
		case GLUT_KEY_LEFT:  izqPulsada = false; break;
		case GLUT_KEY_RIGHT: derPulsada = false; break;
	}
}

// Función de atención al cambio de fotogramas:
void onTimer(int valor)
{
	update();
	glutTimerFunc(valor, onTimer, valor);
}

int main(int argc, char** argv)
{
	// Inicializaciones
	glutInit(&argc, argv);
	glutInitDisplayMode(
		GLUT_DOUBLE		// Doble buffer
		| GLUT_RGB			// Buffer de color
		| GLUT_DEPTH		// Z-BUFFER: Guarda la profundidad de los píxeles
		| GLUT_MULTISAMPLE	/* Anti-aliasing con multimuestreo */);
	glutInitWindowSize(ancho, alto);
	glutInitWindowPosition(50, 50);

	// Crear ventana
	glutCreateWindow(PROYECTO);

	// Registrar callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	// > Callback del movimiento pasivo del ratón:
	glutPassiveMotionFunc(ratonMovido);

	// > Callback de teclas pulsadas:
	glutKeyboardFunc(onKeyPressed);

	// > Callback de teclas soltadas:
	glutKeyboardUpFunc(onKeyReleased);

	// > Callback al pulsar teclas especiales:
	glutSpecialFunc(onSpecialPressed);

	// > Callback al soltar teclas especiales:
	glutSpecialUpFunc(onSpecialReleased);

	// > Callback de animación:
	glutTimerFunc(1000 / TASA_FPS, onTimer, 1000 / TASA_FPS);

	init();

	// Bucle de atencion a eventos
	glutMainLoop();
}
