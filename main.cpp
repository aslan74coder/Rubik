/*???
Перемешать - Меню
Показ победу - Большая надпись поверх куба когда собран
Краткий Help в углу поверх куба (вращ, +-)
*/

#include <gl/glut.h>
#include <math.h>
#include <ata/vec.h>
#include <stdlib.h>

#define RVAL(c) ((c)&0xff)
#define GVAL(c) (((c)>>8)&0xff)
#define BVAL(c) (((c)>>16)&0xff)

using namespace ata;

float dist=5; //???
float fov=60;
int sw,sh;
int mx,my,state=0;
mat3f mat; // camera mat
int sf=-1; // выдел грань ???
// red orange white yellow green marine
int color0=0;
int color[]={0x000040,0x0080FF,0xffffff,0x00FFFF,0x004000,0x804000};
float ra; // face rotation angle
vec3f q1,q2;

struct Cube { // small cube
    int c[3];
};

Cube cubes[27];

int d[]={1,3,9};

void InitCubes()
{
    for(int i=0;i<27;++i)
        for(int j=0;j<3;++j)
            cubes[i].c[j]=(i/d[j]%3!=1)? (j<<1)|((i/d[j]%3)>>1) : -1;
}

void InitMat()
{
    mat[2]=vec3f(0.7f,0.7f,1.f).unit();
    mat[0]=mat[2].ort().unit();
    mat[1]=mat[2]^mat[0];
}

void Init()
{
    InitCubes();
    InitMat();
}

void DrawSquare(const vec3f& a,const vec3f& b,const vec3f& c,int color)
{
    glColor3ub(RVAL(color),GVAL(color),BVAL(color));
    glBegin(GL_QUADS);
    glVertex3fv(&(c-a-b).x);
    glVertex3fv(&(c+a-b).x);
    glVertex3fv(&(c+a+b).x);
    glVertex3fv(&(c-a+b).x);
    glEnd();
}

void DrawFace(const vec3f& a,const vec3f& b,const vec3f& c,int color)
{
    DrawSquare(a,b,c,0);
    DrawSquare(a*0.9f,b*0.9f,c*1.01f,color);
}

void DrawCube()
{
    for(int i=0;i<27;++i) {
        vec3f p(2.f/3*(i%3-1),2.f/3*(i/3%3-1),2.f/3*(i/9%3-1)); // cube center
        bool rf=(sf>=0&&i/d[sf>>1]%3==((sf&1)<<1)); // cube belongs to rotated face
        for(int j=0;j<3;++j) {
            int r=i/d[j]%3;
            if(r==1) continue;
            vec3f ex,ey,q=p;ex[(j+1)%3]=1./3;ey[(j+2)%3]=1./3;q[j]+=1./3*(r-1);
            glPushMatrix();
            if(rf) { vec3f l;l[sf>>1]=1;glRotatef(ra,l.x,l.y,l.z); }
            DrawFace(ex,ey,q,color[cubes[i].c[j]]);
            glPopMatrix();
        }
    }
}

void Display()
{
    glViewport(0,0,sw,sh);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//???    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE); //???

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //??? float zn=::dist-sqrt(3),zf=::dist+sqrt(3),w=3,h=w*sh/sw;
    //??? glOrtho(-w,+w,-h,+h,zn,zf);

    float zn=::dist-sqrt(3),zf=::dist+sqrt(3),w=zn*tan(fov*M_PI/360),h=w*sh/sw;
    glFrustum(-w,+w,-h,+h,zn,zf);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat lightPos[4]={0,0,-1,0};
    glLightfv(0,GL_POSITION,lightPos);

    vec3f pos=-::dist*mat[2],up=mat[1];
    gluLookAt(pos.x,pos.y,pos.z,0,0,0,up.x,up.y,up.z);

    DrawCube();

    glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y)
{
    if(key=='+')
        ::dist*=0.9;
    else
    if(key=='-')
        ::dist*=1.1;
    glutPostRedisplay(); // call for screen redraw
}

void Reshape(int w, int h)
{
    sw=w;sh=h;
    glutPostRedisplay();
}

vec3f Ray(float x,float y)
{
    float k=tan(fov*M_PI/360);
    return (1.f-x*2.f/sw)*k*mat.ex()+(sh-y*2.f)/sw*k*mat.ey()+mat.ez();
}

// (p+r*t-c,c)=0
// t=(1-(p,c))/(r,c)=(1-p[m]*c[m])/(r[m]*c[m])=(c[m]-p[m])/r[m]
// |(p+r*t-c,d)|<=1
bool Ray2Face(const vec3f& p,const vec3f& r,int f,vec3f& q)
{
    int m=f>>1;
    vec3f c;c[m]=((f&1)<<1)-1; // центр грани
    int i=f>>1,j=(i+1)%3,k=3-i-j;
    float t=(c[m]-p[m])/r[m];
    if(fabs(p[j]+r[j]*t-c[j])>1||fabs(p[k]+r[k]*t-c[k])>1) return false;
    q=p+r*t-c;
    return true;
}

//??? сдвинуть кубики по кругу
void Rotate(int f,int n)
{
    Cube tmp[9];
    for(;--n>=0;) {
        int a=f>>1,b=(a+1)%3,c=3-a-b,i,j,k,m=d[a]*((f&1)<<1),mm;
        for(i=-1,k=0;i<=1;++i)
            for(j=-1;j<=1;++j)
                tmp[k++]=cubes[m+d[b]*(1+i)+d[c]*(1+j)];
        for(i=-1,k=0;i<=1;++i)
            for(j=-1;j<=1;++j,++k) {
                mm=m+d[b]*(1-j)+d[c]*(1+i);
                cubes[mm].c[a]=tmp[k].c[a];
                cubes[mm].c[b]=tmp[k].c[c];
                cubes[mm].c[c]=tmp[k].c[b];
            }
    }
}    

void AAA()
{
    for(int i=10000;--i>=0;)
        Rotate(random(6),random(4));
}

void MouseFunc(int button, int _state, int x, int y)
{
    mx=x;my=y;state=(button+1)&-!_state;
    vec3f p=-::dist*mat[2],r=Ray(x,y); // луч на мышь
    int i;
    if(state==1) { // left down
        for(i=0;i<3&&!Ray2Face(p,r,(i<<1)|(p[i]>=0),q1);++i);
        sf=(i<3)?(i<<1)|(p[i]>=0):-1;ra=0;
    }
    else
    if(state==0 && sf>=0) { // up
        Rotate(sf,ra/90+0.5);
        sf=-1;
    }
    glutPostRedisplay();//???
}

void MotionFunc(int x, int y)
{
    if(state==1) { // left mouse button
        if(sf<0) return;
        vec3f p=-::dist*mat[2],r=Ray(x,y); // луч на мышь
        if(!Ray2Face(p,r,sf,q2)) return;
        ra=ata::ang(q1,q2)*180./M_PI;
        if((q1^q2)[sf>>1]<0) ra=360-ra; // dot((q1^q2),l)
    }
    else
    if(state==3) { // right mouse button
        mat[2]=(mat[2]+3.f/sw*(mx-x)*mat[0]+3.f/sw*(my-y)*mat[1]).unit();
        mat[0]=(mat[1]^mat[2]).unit();
        mat[1]=mat[2]^mat[0];
    }
    mx=x;my=y;
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(640,480);
    glutCreateWindow("RUBIK");
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
	glutReshapeFunc(Reshape);
    glutMouseFunc(MouseFunc);
    glutMotionFunc(MotionFunc);
    Init();
    randomize();
    AAA();
    glutMainLoop();
    return 0;
}

