#include "Renderer.h"

void draw_center(void)
{
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); /* R */
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.2f, 0.0f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'x');

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f); /* G */
	glVertex3f(0.0f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.2f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'y');

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f); /* B */
	glVertex3f(0.0f, 0.0f, -0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.0f, -0.2f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'z');
}

void idle() {
	static GLuint previousClock = glutGet(GLUT_ELAPSED_TIME);
	static GLuint currentClock = glutGet(GLUT_ELAPSED_TIME);
	static GLfloat deltaT;

	currentClock = glutGet(GLUT_ELAPSED_TIME);
	deltaT = currentClock - previousClock;
	if (deltaT < 1000.0 / 20.0) { return; }
	else { previousClock = currentClock; }

	//char buff[256];
	//sprintf_s(buff, "Frame Rate = %f", 1000.0 / deltaT);
	//frameRate = buff;

	glutPostRedisplay();
}

void close()
{
	glDeleteTextures(1, &dispBindIndex);
	glutLeaveMainLoop();
	CloseHandle(hMutex);
}

void add_quats(float q1[4], float q2[4], float dest[4])
{
	static int count = 0;
	float t1[4], t2[4], t3[4];
	float tf[4];

	vcopy(q1, t1);
	vscale(t1, q2[3]);

	vcopy(q2, t2);
	vscale(t2, q1[3]);

	vcross(q2, q1, t3);
	vadd(t1, t2, tf);
	vadd(t3, tf, tf);
	tf[3] = q1[3] * q2[3] - vdot(q1, q2);

	dest[0] = tf[0];
	dest[1] = tf[1];
	dest[2] = tf[2];
	dest[3] = tf[3];

	if (++count > RENORMCOUNT) {
		count = 0;
		normalize_quat(dest);
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(58, (double)width / height, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	GLfloat spin_quat[4];
	float gain;
	gain = 2.0; /* trackball gain */

	if (drag_state == GLUT_DOWN)
	{
		if (button_state == GLUT_LEFT_BUTTON)
		{
			trackball(spin_quat,
				(gain * rot_x - 500) / 500,
				(500 - gain * rot_y) / 500,
				(gain * x - 500) / 500,
				(500 - gain * y) / 500);
			add_quats(spin_quat, quat, quat);
		}
		else if (button_state == GLUT_RIGHT_BUTTON)
		{
			t[0] -= (((float)trans_x - x) / 500);
			t[1] += (((float)trans_y - y) / 500);
		}
		else if (button_state == GLUT_MIDDLE_BUTTON)
			t[2] -= (((float)trans_z - y) / 500 * 4);
		else if (button_state == 3 || button_state == 4) // scroll
		{

		}
		//glutPostRedisplay();
	}

	rot_x = x;
	rot_y = y;

	trans_x = x;
	trans_y = y;
	trans_z = y;
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rot_x = x;
			rot_y = y;

			//t[0] = t[0] + 1;


		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			trans_x = x;
			trans_y = y;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{
			//trcon = trcon + 1;
			trans_z = y;
		}
		else if (button == 3 || button == 4)
		{
			const float sign = (static_cast<float>(button)-3.5f) * 2.0f;
			t[2] -= sign * 500 * 0.00015f;
		}
	}

	drag_state = state;
	button_state = button;
}

void vzero(float* v)
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
	register int i;
	for (i = 0; i < 3; i++)
		v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

float vlength(const float *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vnormal(float *v)
{
	vscale(v, 1.0f / vlength(v));
}

float vdot(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
	float a[3]; /* Axis of rotation */
	float phi;  /* how much to rotate about axis */
	float p1[3], p2[3], d[3];
	float t;

	if (p1x == p2x && p1y == p2y) {
		/* Zero rotation */
		vzero(q);
		q[3] = 1.0;
		return;
	}

	/*
	 * First, figure out z-coordinates for projection of P1 and P2 to
	 * deformed sphere
	 */
	vset(p1, p1x, p1y, tb_project_to_sphere(TRACKBALLSIZE, p1x, p1y));
	vset(p2, p2x, p2y, tb_project_to_sphere(TRACKBALLSIZE, p2x, p2y));

	/*
	 *  Now, we want the cross product of P1 and P2
	 */
	vcross(p2, p1, a);

	/*
	 *  Figure out how much to rotate around that axis.
	 */
	vsub(p1, p2, d);
	t = vlength(d) / (2.0f*TRACKBALLSIZE);

	/*
	 * Avoid problems with out-of-control values...
	 */
	if (t > 1.0) t = 1.0;
	if (t < -1.0) t = -1.0;
	phi = 2.0f * asin(t);

	axis_to_quat(a, phi, q);
}

void axis_to_quat(float a[3], float phi, float q[4])
{
	vnormal(a);
	vcopy(a, q);
	vscale(q, sin(phi / 2.0f));
	q[3] = cos(phi / 2.0f);
}

float tb_project_to_sphere(float r, float x, float y)
{
	float d, t, z;

	d = sqrt(x*x + y*y);
	if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
		z = sqrt(r*r - d*d);
	}
	else {           /* On hyperbola */
		t = r / 1.41421356237309504880f;
		z = t*t / d;
	}
	return z;
}

void normalize_quat(float q[4])
{
	int i;
	float mag;

	mag = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	for (i = 0; i < 4; i++) q[i] /= mag;
}

void build_rotmatrix(float m[4][4], float q[4])
{
	m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
	m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
	m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
	m[0][3] = 0.0f;

	m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
	m[1][1] = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
	m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
	m[1][3] = 0.0f;

	m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
	m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
	m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void InitializeWindow(int argc, char* argv[])
{
	// initialize glut settings
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000 / 2, 1000 / 2);

	glutInitWindowPosition(0, 0);

	dispWindowIndex = glutCreateWindow("3D Model");

	trackball(quat, 90.0, 0.0, 0.0, 0.0);

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutCloseFunc(close);
	//GLuint image = load   ("./my_texture.bmp");
	
	//glBindTexture(1,)

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// bind textures
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	reshape(1000, 1000);

	/*glGenTextures(1, &dispBindIndex);
	glBindTexture(GL_TEXTURE_2D, dispBindIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 0.1, 200);
	
	
	GLfloat m[4][4], m1[4][4];
	build_rotmatrix(m, quat);
	gluLookAt(0, 15.0, 25.0, 0, 0, 0, 0, 1.0, 0);

	float cosval = cosf(trcon / 50.0);
	float sinval = sinf(trcon / 50.0);
	float cosval2 = cosf(trcon2 / 1.85);
	float sinval2 = sinf(trcon2 / 1.85);

	m1[0][0] = cosval;
	m1[0][1] = 0.0f;
	m1[0][2] = 0 - sinval;
	m1[0][3] = 0.0f;

	m1[1][0] = 0.0f;
	m1[1][1] = 1.0f;
	m1[1][2] = 0.0f;
	m1[1][3] = 0.0f;

	m1[2][0] = sinval;
	m1[2][1] = 0.0f;
	m1[2][2] = cosval;
	m1[2][3] = 0.0f;

	m1[3][0] = 0.0f;
	m1[3][1] = 0.0f;
	m1[3][2] = 0.0f;
	m1[3][3] = 1.0f;

	glScalef(1, 1, 1);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, earthtexels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
	glMultMatrixf(&m[0][0]);

	glEnable(GL_TEXTURE_2D);
	
	glBegin(GL_QUADS);

	
	float xx1, yy1, zz1, xx2, yy2, zz2, xx3, yy3, zz3, xx4, yy4, zz4;
	for (int jj = 0; jj < 1470; jj = jj + 1)
	{
		glTexCoord2d(earth_color[earthmesh1[jj].T1 - 1].X, earth_color[earthmesh1[jj].T1 - 1].Y);
		xx1 = m1[0][0] * earth[earthmesh1[jj].V1 - 1].X + m1[0][1] * earth[earthmesh1[jj].V1 - 1].Y + m1[0][2] * earth[earthmesh1[jj].V1 - 1].Z;
		yy1 = m1[1][0] * earth[earthmesh1[jj].V1 - 1].X + m1[1][1] * earth[earthmesh1[jj].V1 - 1].Y + m1[1][2] * earth[earthmesh1[jj].V1 - 1].Z;
		zz1 = m1[2][0] * earth[earthmesh1[jj].V1 - 1].X + m1[2][1] * earth[earthmesh1[jj].V1 - 1].Y + m1[2][2] * earth[earthmesh1[jj].V1 - 1].Z;
		glVertex3f(xx1, yy1, zz1);

		glTexCoord2d(earth_color[earthmesh1[jj].T2 - 1].X, earth_color[earthmesh1[jj].T2 - 1].Y);
		xx2 = m1[0][0] * earth[earthmesh1[jj].V2 - 1].X + m1[0][1] * earth[earthmesh1[jj].V2 - 1].Y + m1[0][2] * earth[earthmesh1[jj].V2 - 1].Z;
		yy2 = m1[1][0] * earth[earthmesh1[jj].V2 - 1].X + m1[1][1] * earth[earthmesh1[jj].V2 - 1].Y + m1[1][2] * earth[earthmesh1[jj].V2 - 1].Z;
		zz2 = m1[2][0] * earth[earthmesh1[jj].V2 - 1].X + m1[2][1] * earth[earthmesh1[jj].V2 - 1].Y + m1[2][2] * earth[earthmesh1[jj].V2 - 1].Z;
		glVertex3f(xx2, yy2, zz2);

		glTexCoord2d(earth_color[earthmesh1[jj].T3 - 1].X, earth_color[earthmesh1[jj].T3 - 1].Y);
		xx3 = m1[0][0] * earth[earthmesh1[jj].V3 - 1].X + m1[0][1] * earth[earthmesh1[jj].V3 - 1].Y + m1[0][2] * earth[earthmesh1[jj].V3 - 1].Z;
		yy3 = m1[1][0] * earth[earthmesh1[jj].V3 - 1].X + m1[1][1] * earth[earthmesh1[jj].V3 - 1].Y + m1[1][2] * earth[earthmesh1[jj].V3 - 1].Z;
		zz3 = m1[2][0] * earth[earthmesh1[jj].V3 - 1].X + m1[2][1] * earth[earthmesh1[jj].V3 - 1].Y + m1[2][2] * earth[earthmesh1[jj].V3 - 1].Z;
		glVertex3f(xx3, yy3, zz3);

		glTexCoord2d(earth_color[earthmesh1[jj].T4 - 1].X, earth_color[earthmesh1[jj].T4 - 1].Y);
		xx4 = m1[0][0] * earth[earthmesh1[jj].V4 - 1].X + m1[0][1] * earth[earthmesh1[jj].V4 - 1].Y + m1[0][2] * earth[earthmesh1[jj].V4 - 1].Z;
		yy4 = m1[1][0] * earth[earthmesh1[jj].V4 - 1].X + m1[1][1] * earth[earthmesh1[jj].V4 - 1].Y + m1[1][2] * earth[earthmesh1[jj].V4 - 1].Z;
		zz4 = m1[2][0] * earth[earthmesh1[jj].V4 - 1].X + m1[2][1] * earth[earthmesh1[jj].V4 - 1].Y + m1[2][2] * earth[earthmesh1[jj].V4 - 1].Z;
		glVertex3f(xx4, yy4, zz4);
	}
	glEnd();
	
	glBegin(GL_TRIANGLES);
	
	for (int jj = 0; jj < 70; jj = jj + 1)
	{
		glTexCoord2d(earth_color[earthmesh2[jj].T1 - 1].X, earth_color[earthmesh2[jj].T1 - 1].Y);
		xx1 = m1[0][0] * earth[earthmesh2[jj].V1 - 1].X + m1[0][1] * earth[earthmesh2[jj].V1 - 1].Y + m1[0][2] * earth[earthmesh2[jj].V1 - 1].Z;
		yy1 = m1[1][0] * earth[earthmesh2[jj].V1 - 1].X + m1[1][1] * earth[earthmesh2[jj].V1 - 1].Y + m1[1][2] * earth[earthmesh2[jj].V1 - 1].Z;
		zz1 = m1[2][0] * earth[earthmesh2[jj].V1 - 1].X + m1[2][1] * earth[earthmesh2[jj].V1 - 1].Y + m1[2][2] * earth[earthmesh2[jj].V1 - 1].Z;
		glVertex3f(xx1, yy1, zz1);

		glTexCoord2d(earth_color[earthmesh2[jj].T2 - 1].X, earth_color[earthmesh2[jj].T2 - 1].Y);
		xx2 = m1[0][0] * earth[earthmesh2[jj].V2 - 1].X + m1[0][1] * earth[earthmesh2[jj].V2 - 1].Y + m1[0][2] * earth[earthmesh2[jj].V2 - 1].Z;
		yy2 = m1[1][0] * earth[earthmesh2[jj].V2 - 1].X + m1[1][1] * earth[earthmesh2[jj].V2 - 1].Y + m1[1][2] * earth[earthmesh2[jj].V2 - 1].Z;
		zz2 = m1[2][0] * earth[earthmesh2[jj].V2 - 1].X + m1[2][1] * earth[earthmesh2[jj].V2 - 1].Y + m1[2][2] * earth[earthmesh2[jj].V2 - 1].Z;
		glVertex3f(xx2, yy2, zz2);

		glTexCoord2d(earth_color[earthmesh2[jj].T3 - 1].X, earth_color[earthmesh2[jj].T3 - 1].Y);
		xx3 = m1[0][0] * earth[earthmesh2[jj].V3 - 1].X + m1[0][1] * earth[earthmesh2[jj].V3 - 1].Y + m1[0][2] * earth[earthmesh2[jj].V3 - 1].Z;
		yy3 = m1[1][0] * earth[earthmesh2[jj].V3 - 1].X + m1[1][1] * earth[earthmesh2[jj].V3 - 1].Y + m1[1][2] * earth[earthmesh2[jj].V3 - 1].Z;
		zz3 = m1[2][0] * earth[earthmesh2[jj].V3 - 1].X + m1[2][1] * earth[earthmesh2[jj].V3 - 1].Y + m1[2][2] * earth[earthmesh2[jj].V3 - 1].Z;
		glVertex3f(xx3, yy3, zz3);
	}
	
	glEnd();
	
	glPushMatrix();
		glRotatef(trcon2, 0, 1, 0);

	glTranslatef(15.0f, 0.0f, 0.0f); // 실제로는 지구 지름의 30배 정도 떨어진 거리지만 축소하여 표현
	glRotatef(180, 0.0f, 1.0f, 0.0f); // 지구가 달의 밝은 면을 보도록
	glScalef(20, 20, 20); // 달의 지름 * 4 = 지구의 지름처럼 보이게
	
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, moontexels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D);

	
	
	glBegin(GL_QUADS);
	
	for (int jj = 0; jj < 1984; jj = jj + 1)
	{
		glTexCoord2d(moon_color[moonmesh[jj].T1 - 1].X, moon_color[moonmesh[jj].T1 - 1].Y);
		glVertex3f(moon[moonmesh[jj].V1 - 1].X, moon[moonmesh[jj].V1 - 1].Y, moon[moonmesh[jj].V1 - 1].Z);
		glTexCoord2d(moon_color[moonmesh[jj].T2 - 1].X, moon_color[moonmesh[jj].T2 - 1].Y);
		glVertex3f(moon[moonmesh[jj].V2 - 1].X, moon[moonmesh[jj].V2 - 1].Y, moon[moonmesh[jj].V2 - 1].Z);
		glTexCoord2d(moon_color[moonmesh[jj].T3 - 1].X, moon_color[moonmesh[jj].T3 - 1].Y);
		glVertex3f(moon[moonmesh[jj].V3 - 1].X, moon[moonmesh[jj].V3 - 1].Y, moon[moonmesh[jj].V3 - 1].Z);
		glTexCoord2d(moon_color[moonmesh[jj].T4 - 1].X, moon_color[moonmesh[jj].T4 - 1].Y);
		glVertex3f(moon[moonmesh[jj].V4 - 1].X, moon[moonmesh[jj].V4 - 1].Y, moon[moonmesh[jj].V4 - 1].Z);
	}
	
	glEnd();
	glPopMatrix();
	

	trcon = trcon - 5; // 달이 1회 공전하는 동안, 지구는 27회 자전하지만 너무 빠르기 때문에 조정하였음.
	trcon2 = trcon2 - 1; 

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	earth = new Vertex[100000];
	earth_color = new Vertex[100000];
	earthmesh1 = new MMesh[100000];
	earthmesh2 = new MMesh2[100000];

	int i, j, k = 0;
	FILE* f = fopen("earth.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	// extract image height and width from header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];

	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);
	for (i = 0; i < width; i++)
		for (j = 0; j < height; j++)
		{
			earthtexels[i][j][0] = data[k * 3 + 2];
			earthtexels[i][j][1] = data[k * 3 + 1];
			earthtexels[i][j][2] = data[k * 3];
			k++;
		}
	
	FILE* fp;
	fp = fopen("earth.obj", "r");
	int count = 0;
	int num = 0;
	float x, y, z;

	for (j = 0; j < 1507; j = j + 1)
	{
		count = fscanf(fp, "v %f %f %f /n", &x, &y, &z);
		if (count == 3)
		{
			earth[j].X = x / scale;
			earth[j].Y = y / scale;
			earth[j].Z = z / scale;
			if (earth[j].Z < zmin)
				zmin = earth[j].Z;
			if (earth[j].Z > zmax)
				zmax = earth[j].Z;
		}
		else
			break;
	}

	for (j = 0; j < 1618; j = j + 1)
	{
		count = fscanf(fp, "vt %f %f /n", &x, &y);
		if (count == 2)
		{
			earth_color[j].X = x;
			earth_color[j].Y = y;
			earth_color[j].Z = 0.0;
		}
		else
			break;
	}

	float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

	for (j = 0; j < 1470; j = j + 1)
	{
		count = fscanf(fp, "f %f/%f/%f %f/%f/%f %f/%f/%f %f/%f/%f /n", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4);
		if (count == 12)
		{
			earthmesh1[j].V1 = x1;
			earthmesh1[j].V2 = x2;
			earthmesh1[j].V3 = x3;
			earthmesh1[j].V4 = x4;
			earthmesh1[j].T1 = y1;
			earthmesh1[j].T2 = y2;
			earthmesh1[j].T3 = y3;
			earthmesh1[j].T4 = y4;
		}
		else
			break;
	}

	for (j = 0; j < 70; j = j + 1)
	{
		count = fscanf(fp, "f %f/%f/%f %f/%f/%f %f/%f/%f /n", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3);
		if (count == 9)
		{
			earthmesh2[j].V1 = x1;
			earthmesh2[j].V2 = x2;
			earthmesh2[j].V3 = x3;
			earthmesh2[j].T1 = y1;
			earthmesh2[j].T2 = y2;
			earthmesh2[j].T3 = y3;
		}
		else
			break;
	}

	fclose(fp);
	
	delete[] data;

	moon = new Vertex[100000];
	moon_color = new Vertex[100000];
	moonmesh = new MMesh[100000];

	f = fopen("moon.bmp", "rb");
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	// extract image height and width from header
	width = *(int*)&info[18];
	height = *(int*)&info[22];

	size = 3 * width * height;
	data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);
	k = 0;
	for (i = 0; i < width; i++)
		for (j = 0; j < height; j++)
		{
			moontexels[i][j][0] = data[k * 3];
			moontexels[i][j][1] = data[k * 3 + 1];
			moontexels[i][j][2] = data[k * 3 + 2];
			k++;
		}

	fp = fopen("moon.obj", "r");

	for (j = 0; j < 1986; j = j + 1)
	{
		count = fscanf(fp, "v %f %f %f /n", &x, &y, &z);
		if (count == 3)
		{
			moon[j].X = x / scale;
			moon[j].Y = y / scale;
			moon[j].Z = z / scale;
			if (moon[j].Z < zmin)
				zmin = moon[j].Z;
			if (moon[j].Z > zmax)
				zmax = moon[j].Z;
		}
		else
			break;
	}

	for (j = 0; j < 2152; j = j + 1)
	{
		count = fscanf(fp, "vt %f %f /n", &x, &y);
		if (count == 2)
		{
			moon_color[j].X = x;
			moon_color[j].Y = y;
			moon_color[j].Z = 0.0;
		}
		else
			break;
	}

	for (j = 0; j < 1984; j = j + 1)
	{
		count = fscanf(fp, "f %f/%f/%f %f/%f/%f %f/%f/%f %f/%f/%f /n", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4);
		if (count == 12)
		{
			moonmesh[j].V1 = x1;
			moonmesh[j].V2 = x2;
			moonmesh[j].V3 = x3;
			moonmesh[j].V4 = x4;
			moonmesh[j].T1 = y1;
			moonmesh[j].T2 = y2;
			moonmesh[j].T3 = y3;
			moonmesh[j].T4 = y4;
		}
		else
			break;
	}

	fclose(fp);
	
	InitializeWindow(argc, argv);

	display();
	glutMainLoop();
	delete[] earth;
	delete[] earthmesh1;
	delete[] earthmesh2;
	delete[] earth_color;
	delete[] moon;
	delete[] moonmesh;
	delete[] moon_color;
	return 0;
}