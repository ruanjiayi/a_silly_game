//使用说明：
//'1'-切换环境光开关。
//'n'-放大聚光灯照射角度。
//'m'-减小聚光灯照射角度。
//'h','f’-增减x轴上的聚光灯照射方向。
//'g','t’-增减z轴上的聚光灯照射方向。
//点击风景画-改变纹理。
//左击抽屉-打开抽屉。
//右击抽屉-合上抽屉。
//aswd方向键-移动视线。
//          -点击物体，选中后物体会泛红，再按aswd方向键即可移动物体。
//'z','c’-走进（放大）和远离物体（缩小）。

#include <GL/glut.h>

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#define BITMAP_ID 0x4D42
#define INFINITY 100000
#define BUFFER_SIZE 1024
#define N 30
#define F:/viewed/CG/final project/pr/newobj/ ./newobj/

float whRatio;
int size[N];
GLfloat *vertices[N],*normals[N],*textures[N];
GLuint texture[N];
GLfloat fTrans[N][2];
int objName;
GLfloat angle=8;

bool bPersp = true;
bool bAnim = false;
bool bWire = false;
bool bTrans = false;
bool bcolor = true;
int bCard=0;
int bKey=0;
int wHeight = 0;
int wWidth = 0;
double dRotate    = 0.0;
float center[] = {0.0, 0.0, -0.5};
float eye[] = {0.0, 0.2, 1.1};
GLfloat lightDir[]={0,-1,0,1};
GLfloat position[]={0,1.3,0,1};
unsigned char* introduction;
BITMAPINFOHEADER bitmapInfoHeader;

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr;    // 文件指针
    BITMAPFILEHEADER bitmapFileHeader;    // bitmap文件头
    unsigned char    *bitmapImage;        // bitmap图像数据
    int    imageIdx = 0;        // 图像位置索引
    unsigned char    tempRGB;    // 交换变量

                                // 以“二进制+读”模式打开文件filename
    filePtr = fopen(filename, "rb");
    if (filePtr == NULL) {
        printf("file %s not open\n",filename);
        exit(1);
        return NULL;
    }
    // 读入bitmap文件图
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    // 验证是否为bitmap文件
    if (bitmapFileHeader.bfType != BITMAP_ID) {
        fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
        return NULL;
    }
    // 读入bitmap信息头
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
    // 将文件指针移至bitmap数据
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
    // 为装载图像数据创建足够的内存
    bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage];
    // 验证内存是否创建成功
    if (!bitmapImage) {
        fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
        return NULL;
    }

    // 读入bitmap图像数据
    fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
    // 确认读入成功
    if (bitmapImage == NULL) {
        fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
        return NULL;
    }
    //由于bitmap中保存的格式是BGR，下面交换R和B的值，得到RGB格式
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }
    // 关闭bitmap图像文件
    fclose(filePtr);
    return bitmapImage;
}

void texload(int i, char *filename)
{
    BITMAPINFOHEADER bitmapInfoHeader;                                 // bitmap信息头
    unsigned char*   bitmapData;                                       // 纹理数据

    bitmapData = LoadBitmapFile(filename, &bitmapInfoHeader);
    glBindTexture(GL_TEXTURE_2D, texture[i]);
    // 指定当前纹理的放大/缩小过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D,
        0,         //mipmap层次(通常为，表示最上层)
        GL_RGB,    //我们希望该纹理有红、绿、蓝数据
        bitmapInfoHeader.biWidth, //纹理宽带，必须是n，若有边框+2
        bitmapInfoHeader.biHeight, //纹理高度，必须是n，若有边框+2
        0, //边框(0=无边框, 1=有边框)
        GL_RGB,    //bitmap数据的格式
        GL_UNSIGNED_BYTE, //每个颜色数据的类型
        bitmapData);    //bitmap数据指针
}

void readObj(string filename){
    char line[128];
    static int j=0;//控制存储顶点数组的指针下标。
    vector<GLfloat> vertex;
    vector<GLfloat> normal;
    vector<GLfloat> texture;
    vector<GLint> vertexi;
    vector<GLint> normali;
    vector<GLint> texturei;

    FILE * file = fopen(filename.c_str(), "r"); //打开文件
    if (file == NULL) //检测是否成功打开
    {
        printf("Can't open the file %s\n",filename.c_str());
        exit(1);
    }

    int x = fscanf(file, "%s", line);
    while(x>0)
{      // printf("%s\n",line);
        if (strcmp(line, "v")==0) //读入顶点坐标数据
        {
            GLfloat v[3];
            fscanf(file, " %f %f %f\n", &v[0], &v[1], &v[2]);
            vertex.push_back(v[0]); vertex.push_back(v[1]); vertex.push_back(v[2]);

        }
        else if (strcmp(line, "vt")==0) //读入纹理坐标数据
        {
            GLfloat t[2];
            fscanf(file, "%f %f\n", &t[0], &t[1]);
            //printf("%f %f\n",t[0],t[1]);
            texture.push_back(t[0]);texture.push_back(t[1]);
        }
        else if (strcmp(line, "vn")==0) //读入法向向量数据
        {
            GLfloat n[3];
            fscanf(file, "%f %f %f\n", &n[0], &n[1], &n[2]);
            normal.push_back(n[0]); normal.push_back(n[1]); normal.push_back(n[2]);
        }
        else if (strcmp(line, "f")==0)
        {

            GLint vi[3], ti[3], ni[3];
            //fscanf会如果成功读取则会返回读取的参数个数，也就是%d的个数，或者后面参数列表的个数
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vi[0], &ti[0], &ni[0],
                &vi[1], &ti[1], &ni[1], &vi[2], &ti[2], &ni[2]);

            if (matches != 9) //上面fscanf中有多少个%d，就写成多少个，以检测格式是否正确
            {
                printf("File can't be read by this parser :-( Try exporting with other options\n");
                return ;
            }
            vertexi.push_back(vi[0]);
            vertexi.push_back(vi[1]);
            vertexi.push_back(vi[2]);
            texturei.push_back(ti[0]);
            texturei.push_back(ti[1]);
            texturei.push_back(ti[2]);
            normali.push_back(ni[0]);
            normali.push_back(ni[1]);
            normali.push_back(ni[2]);
          }
          x = fscanf(file, "%s", line);
}

    int i;
    size[j]=vertexi.size();
    vertices[j]=new GLfloat[size[j]*3];
    normals[j]=new GLfloat[size[j]*3];
    textures[j]=new GLfloat[size[j]*2];

    for (i=0; i<size[j];i++ ){
        vertices[j][3*i]=vertex[3*(vertexi[i]-1)];
        vertices[j][3*i+1]=vertex[3*(vertexi[i]-1)+1];
        vertices[j][3*i+2]=vertex[3*(vertexi[i]-1)+2];
        normals[j][3*i]=normal[3*(normali[i]-1)];
        normals[j][3*i+1]=normal[3*(normali[i]-1)+1];
        normals[j][3*i+2]=normal[3*(normali[i]-1)+2];

        textures[j][2*i]=texture[2*(texturei[i]-1)];
        textures[j][2*i+1]=texture[2*(texturei[i]-1)+1];
    }

    vertex.clear();
    normal.clear();
    texture.clear();
    vertexi.clear();
    normali.clear();
    texturei.clear();

    j++;

    return;
}

void color(int name){
    GLfloat diffuse[] = { 1.0f, 0.0f, 0.0f };
    GLfloat diffuse1[] = { 1.0f, 1.0f, 1.0f };
    if(name==objName&&bTrans)
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    else
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse1);
}

void drawVA(int i,int ti){
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);


    glVertexPointer(3,GL_FLOAT,0,vertices[i]);
    glNormalPointer(GL_FLOAT,0,normals[i]);


    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[ti]);
    glTexCoordPointer(2, GL_FLOAT, 0, textures[i]);

    glDrawArrays(GL_TRIANGLES,0,size[i]);

}

void init(){
    glGenTextures(N, texture);// 第一参数是需要生成标示符的个数, 第二参数是返回标示符的数组
    texload(0,"F:/viewed/CG/final project/pr/newobj/chuanglian.bmp");
    texload(1,"F:/viewed/CG/final project/pr/newobj//fangding.bmp");
    texload(2,"F:/viewed/CG/final project/pr/newobj//qiang3.bmp");
    texload(3,"F:/viewed/CG/final project/pr/newobj//deng.bmp");
    texload(4,"F:/viewed/CG/final project/pr/newobj//diban.bmp");
    texload(5,"F:/viewed/CG/final project/pr/newobj//men.bmp");
    texload(6,"F:/viewed/CG/final project/pr/newobj//dalishi.bmp");//桌子椅子
    texload(7,"F:/viewed/CG/final project/pr/newobj//fengjinghua.bmp");
    texload(8,"F:/viewed/CG/final project/pr/newobj//huanpen.bmp");
    texload(9,"F:/viewed/CG/final project/pr/newobj//key.bmp");
    texload(10,"F:/viewed/CG/final project/pr/newobj//mucai2.bmp");//抽屉和窗户、画框
    texload(11,"F:/viewed/CG/final project/pr/newobj//tips.bmp");
    texload(12,"F:/viewed/CG/final project/pr/newobj//xiaoyuanka.bmp");
    texload(13,"F:/viewed/CG/final project/pr/newobj//mucai.bmp");
    texload(14,"F:/viewed/CG/final project/pr/newobj//fengjinghua2.bmp");
    texload(15,"F:/viewed/CG/final project/pr/newobj//ball.bmp");
    texload(16,"F:/viewed/CG/final project/pr/newobj//cola.bmp");

//	以下命名为一个物体，点击不做变换。
    readObj("F:/viewed/CG/final project/pr/newobj//fangding.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//simianqiang.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//diban.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//deng.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//chuanghu.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//chuanglian.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//chuangliangan.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//tips.obj");

    //点击打开大门（若找到校园卡和钥匙）
    readObj("F:/viewed/CG/final project/pr/newobj//men.obj");


    readObj("F:/viewed/CG/final project/pr/newobj//yizi1.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//yizi2.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//yizi3.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//yizi4.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//zhuozi.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//xiaoyuanka.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//key.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//huakuang.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//huapen.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//guizi.obj");//柜子的移动要带动抽屉。
    readObj("F:/viewed/CG/final project/pr/newobj//chouti1.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//chouti2.obj");//抽屉的移动要带动钥匙移动。
    readObj("F:/viewed/CG/final project/pr/newobj//chouti3.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//fengjinghua.obj");//左转，右转。
    readObj("F:/viewed/CG/final project/pr/newobj//ball.obj");
    readObj("F:/viewed/CG/final project/pr/newobj//cola.obj");
//    readObj("F:/viewed/CG/final project/pr/newobj//13.obj");


    introduction = LoadBitmapFile("F:/viewed/CG/final project/pr/newobj//shuoming.bmp", &bitmapInfoHeader);
}


void draw(){

    GLfloat mat[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, mat);
	glMateriali(GL_FRONT,GL_SHININESS,128);//设置材质的镜面指数来确定光斑的大小和聚焦程度。可以对镜面指数进行设定。如果N值越大，则光斑尺寸越小，物体越有光泽，反之越大。N值可 取1-128之间的任意整数。

    glInitNames();
    glPushName(0);

    glLoadName(1);

	drawVA(1,2);
	drawVA(0,1);
	drawVA(2,4);
	drawVA(3,3);
	drawVA(4,10);
	drawVA(5,0);
	drawVA(6,1);
    drawVA(7,11);
    drawVA(16,10);
	drawVA(18,13);

    glPushMatrix();
    color(2);glLoadName(2);
	drawVA(8,5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(fTrans[1][0],0,fTrans[1][1]);
    color(3);glLoadName(3);
	drawVA(9,10);
	glPopMatrix();


    glPushMatrix();
    glTranslatef(fTrans[2][0],0,fTrans[2][1]);
	color(4);glLoadName(4);
	drawVA(10,10);
	glPopMatrix();


    glPushMatrix();
    glTranslatef(fTrans[3][0],0,fTrans[3][1]);
	color(5);
    glLoadName(5);
    drawVA(11,10);
	glPopMatrix();


    glPushMatrix();
    glTranslatef(fTrans[4][0],0,fTrans[4][1]);
	color(6);glLoadName(6);
	drawVA(12,10);
	glPopMatrix();


    glPushMatrix();
    glTranslatef(fTrans[5][0],0,fTrans[5][1]);
	color(7);glLoadName(7);
	drawVA(13,6);
	glPopMatrix();

	//校园卡
    glPushMatrix();
    glTranslatef(0,0,-0.2);
    color(8);glLoadName(8);
	drawVA(14,12);
	glPopMatrix();

    //key
    glPushMatrix();
    glTranslatef(fTrans[7][0],0,fTrans[7][1]+0.4);
	color(9);glLoadName(9);
	drawVA(15,9);
	glPopMatrix();

    glPushMatrix();
    glTranslatef(fTrans[9][0],0,fTrans[9][1]);
	color(11);glLoadName(11);
	drawVA(17,8);
	glPopMatrix();

    glPushMatrix();
    glTranslatef(fTrans[11][0],0,fTrans[11][1]);
    color(13);    glLoadName(13);
	drawVA(19,10);
	glPopMatrix();


    color(14);
    glPushMatrix();
    glTranslatef(fTrans[12][0],0,fTrans[12][1]);glLoadName(14);
	drawVA(20,10);
	glPopMatrix();


    glPushMatrix();
    glTranslatef(fTrans[13][0],0,fTrans[13][1]);
    color(15); glLoadName(15);
	drawVA(21,10);
	glPopMatrix();


    glPushMatrix();
    glLoadName(16);
    if(objName==16&&bTrans)
        drawVA(22,7);
    else
        drawVA(22,14);
	glPopMatrix();

    glPushMatrix();
    glTranslatef(fTrans[15][0],0,fTrans[15][1]);
    color(17); glLoadName(17);
    drawVA(23,15);
	glPopMatrix();

	glPushMatrix();
    glTranslatef(fTrans[16][0],0,fTrans[16][1]);
    color(18); glLoadName(18);
    drawVA(24,16);
	glPopMatrix();

//	drawVA(25,9);

}

BOOL WriteBitmapFile(char * filename,int width,int height,unsigned char * bitmapData)
{
    //填充BITMAPFILEHEADER
    BITMAPFILEHEADER bitmapFileHeader;
    memset(&bitmapFileHeader,0,sizeof(BITMAPFILEHEADER));
    bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER);
    bitmapFileHeader.bfType = BITMAP_ID;
    bitmapFileHeader.bfOffBits =sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //填充BITMAPINFOHEADER
    BITMAPINFOHEADER bitmapInfoHeader;
    memset(&bitmapInfoHeader,0,sizeof(BITMAPINFOHEADER));
    bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfoHeader.biWidth = width;
    bitmapInfoHeader.biHeight = height;
    bitmapInfoHeader.biPlanes = 1;
    bitmapInfoHeader.biBitCount = 24;
    bitmapInfoHeader.biCompression = BI_RGB;
    bitmapInfoHeader.biSizeImage = width * abs(height) * 3;

    //////////////////////////////////////////////////////////////////////////
    FILE * filePtr;         //连接要保存的bitmap文件用
    unsigned char tempRGB;  //临时色素
    int imageIdx;

    //交换R、B的像素位置,bitmap的文件放置的是BGR,内存的是RGB
    for (imageIdx = 0;imageIdx < bitmapInfoHeader.biSizeImage;imageIdx +=3)
    {
        tempRGB = bitmapData[imageIdx];
        bitmapData[imageIdx] = bitmapData[imageIdx + 2];
        bitmapData[imageIdx + 2] = tempRGB;
    }

    filePtr = fopen(filename,"wb");
    if (NULL == filePtr)
    {
        return FALSE;
    }

    fwrite(&bitmapFileHeader,sizeof(BITMAPFILEHEADER),1,filePtr);

    fwrite(&bitmapInfoHeader,sizeof(BITMAPINFOHEADER),1,filePtr);

    fwrite(bitmapData,bitmapInfoHeader.biSizeImage,1,filePtr);

    fclose(filePtr);
    return TRUE;
}


void drawPic(){
    static unsigned char* imgBuffer = (unsigned char*)malloc(wWidth * wHeight * 3);
    memset(imgBuffer,0,wWidth * wHeight * 3);
    glReadPixels(0, 0,  wHeight,wWidth, GL_RGB, GL_UNSIGNED_BYTE, imgBuffer);
    WriteBitmapFile("F:/viewed/CG/final project/pr/newobj//1.bmp",wHeight,wWidth,imgBuffer);
}


void output()
{
	static char buffer[256];
    sprintf(buffer,"get key:%d,get card:%d",bKey,bCard);
	glutSetWindowTitle(buffer);
}


void redraw(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();									// Reset The Current Modelview Matrix

	gluLookAt(eye[0], eye[1], eye[2],
		center[0], center[1], center[2],
		0, 1, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
    GLfloat white[] = { 1.0, 1.0, 1.0, 1.0};
    GLfloat yellow[]={1.0,1.0,0.2,1.0};

	GLfloat light_pos[] = {0,1,0,0.2};

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, white);

    if(bcolor)
        glEnable(GL_LIGHT0);
    else
        glDisable(GL_LIGHT0);

    glLightfv(GL_LIGHT1, GL_AMBIENT,yellow);
    glLightfv(GL_LIGHT1, GL_SPECULAR,yellow);                    //设置镜面光成分
	glLightfv(GL_LIGHT1, GL_DIFFUSE,yellow);                     //设置漫射光成分

    glLightfv(GL_LIGHT1, GL_POSITION,position);
	glLightf(GL_LIGHT1,  GL_SPOT_CUTOFF,angle);             //裁减角度
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDir);          //光源方向
	glLightf(GL_LIGHT1,GL_SPOT_EXPONENT,2);                    //聚集度

	glEnable(GL_LIGHT1);

    if(bAnim){
        dRotate+=0.2f;
    }

    glPushMatrix();
	glRotated(dRotate, 0, 1, 0);			// Rotate around Y axis
	glRotatef(-90, 0, 1.0f, 0);			// Rotate around Y axis
	glTranslatef(0.5,0,0.2);
	glScalef(0.5,0.5,0.5);
    draw();
    glPopMatrix();

    output();
	glutSwapBuffers();
}

void idle()
{
	glutPostRedisplay();
}


void updateView(int width, int height)
{
	glViewport(200,0,height,width);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	whRatio = (GLfloat)height/(GLfloat)width;

	if (bPersp){
		gluPerspective(45,whRatio,0.3,4.0);
	}
	else
		glOrtho(-3, 3, -3, 3, -5, 5);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
}

void writeObj(){
    int i;
    int obj=objName+6;
    int objsize=size[obj];
    char filename[64];
    sprintf(filename,"F:/viewed/CG/final project/pr/newobj//%d.obj",obj);
    FILE * file = fopen(filename, "w");
     if (file == NULL) //检测是否成功打开
    {
        printf("Can't open the file %s\n",filename);
        exit(1);
    }

    for (i=0;i<objsize;i++)
        fprintf(file,"v %.6f %.6f %.6f\n",vertices[obj][3*i],vertices[obj][3*i+1],vertices[obj][3*i+2]);
    for (i=0;i<objsize;i++)
        fprintf(file,"vt %.6f %.6f\n",textures[obj][2*i],textures[obj][2*i+1]);
    for (i=0;i<objsize;i++)
        fprintf(file,"vn %.6f %.6f %.6f\n",normals[obj][3*i],normals[obj][3*i+1],normals[obj][3*i+2]);
    for (i=0;i<objsize;i++)
        fprintf(file,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",3*i+1,3*i+1,3*i+1,3*i+2,3*i+2,3*i+2,3*i+3,3*i+3,3*i+3);

    if(fclose(file)==0)
        MessageBox (NULL, "writeObj() success!", "notice", MB_OK);
}

void reshape(int width, int height)
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	wHeight = height;
	wWidth = width-200;

	updateView(wHeight, wWidth);
}

void key(unsigned char k, int x, int y)
{
	switch(k)
	{
	case 27:
	case 'q': {exit(0); break; }
	case 'p': {bPersp = !bPersp; break; }
	case 'o': {bWire = !bWire; break;}
    case ' ': {bAnim = !bAnim;break;}
	case 'a': {
	    if(bTrans)
            fTrans[objName-2][0]+=0.2f;
        else{
            if(eye[0]>-1.3){
                eye[0] -= 0.2f;center[0] -= 0.2f;
            }
        }

		break;
			  }
	case 'd': {
        if(bTrans)
            fTrans[objName-2][0]-=0.2f;
        else{
            if(eye[0]<1.2){
                eye[0] += 0.2f;center[0] += 0.2f;
            }
        }

		break;
			  }
	case 's': {
	    if(bTrans)
            fTrans[objName-2][1]+=0.2f;
        else{
            if(eye[1]>-0.5f){
                //eye[1] -= 0.2f;
                center[1]-= 0.2f;
            }
        }

		break;
			  }
	case 'w': {
	    if(bTrans)
            fTrans[objName-2][1]-=0.2f;
        else
            if(eye[1]<0.5f){
                //eye[1] += 0.2f;
                center[1]+= 0.2f;
            }

		break;
			  }
	case 'z': {
	    if(eye[2]>-0.5){
            eye[2] -= 0.1f;
            center[2] -= 0.1f;
	    }

		break;
			  }
	case 'c': {
	    if(eye[2]<1.6){
            eye[2] += 0.1f;
            center[2] += 0.1f;
	    }

		break;
			  }
    case 'f': {
        if(lightDir[0]>-0.8)
            lightDir[0]-=0.2f;
        break;
    }
    case 'h': {
        if(lightDir[0]<0.8)
            lightDir[0]+=0.2f;
        break;
    }
    case 't': {
        if(lightDir[2]>-1.8)
            lightDir[2]-=0.2f;
        break;
    }
    case 'g': {
        if(lightDir[2]<0.8)
            lightDir[2]+=0.2f;
        break;
    }
    case 'n':{
        if(angle<20) angle++;break;
    }
    case 'm':{
        if(angle>0) angle--;break;
    }
    case '1':{
        bcolor = ! bcolor;
        break;
    }
    case '0':{
        if(bTrans)
            writeObj();
        break;
    }
    case '9':{
        drawPic();
        break;
    }
	}

	updateView(wHeight, wWidth);
}


void mouse(int btn, int state, int x, int y){
     if((btn == GLUT_LEFT_BUTTON||btn==GLUT_RIGHT_BUTTON)&& state == GLUT_UP){
        GLint viewport[4];
        GLuint buffer[BUFFER_SIZE];
        glSelectBuffer(BUFFER_SIZE,buffer);

        glGetIntegerv(GL_VIEWPORT, viewport);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        glRenderMode(GL_SELECT);
        glLoadIdentity();
        gluPickMatrix((GLdouble)x, (GLdouble) (viewport[3] - y), 0.01f, 0.01f, viewport);
         if (bPersp) {
                gluPerspective(45,whRatio,0.3,4.0);
        }
        else {
            glOrtho(-3, 3, -3, 3, -5, 5);  //正投影
        }

         glMatrixMode(GL_MODELVIEW);

         redraw(); //重绘

         int hits = glRenderMode(GL_RENDER); //进入渲染模式，得到选中物体个数


            if (hits > 0)//如果选中物体个数大于1
            {
                GLint name = buffer[3];
                GLint depth = buffer[1];

               if(name==1&&hits>1){
                    name=buffer[7];
                    depth=buffer[5];
                    hits--;
                }

                if(name==1&&hits==1)
                    name=0;

                for (int i = 1; i < hits; i++)
                {
                    if ((GLint)buffer[i * 4 + 1] < depth)//获取深度最小的物体(selectBuff是按照ID从小到大排列的)
                    {
                        name = buffer[i * 4 + 3];
                        depth = buffer[i * 4 + 1];
                        printf("after,name=%d,depth=%d\n",name,depth);
                    }
                }

                objName = name;
                bTrans = !bTrans;


                if(name==13||name==14||name==15){
                    if(btn==GLUT_LEFT_BUTTON&&fTrans[name-2][1]>-0.8){
                        fTrans[name-2][1]-=0.2;
                        if(name==14)
                            fTrans[7][1]-=0.2;
                    }

                    else if(btn==GLUT_RIGHT_BUTTON&&fTrans[name-2][1]<-0.2){
                        fTrans[name-2][1]+=0.2;
                        if(name==14)
                            fTrans[7][1]+=0.2;
                    }

                    bTrans = !bTrans;
                    objName=0;
                }

                if(name==9)
                    bKey=1;
                if(name==8)
                    bCard=1;
                if(bKey&&bCard&&name==2)
                    MessageBox (NULL, "Now you can walk out of the room!", "Game Over", MB_OK);

                printf("get object%d\n",name);
            }



            glMatrixMode(GL_PROJECTION);
            glPopMatrix();//抛出矩阵
            //设置矩阵模式为模型
            glMatrixMode(GL_MODELVIEW);
        }
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

    glDrawPixels(bitmapInfoHeader.biWidth,bitmapInfoHeader.biHeight,GL_RGB,GL_UNSIGNED_BYTE,introduction);

	glutSwapBuffers();
}


int main (int argc,  char *argv[])
{

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(700, 700);
	int window=glutCreateWindow("Simple GLUT App");

    init();

	glutDisplayFunc(redraw);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutMouseFunc(mouse);
    glutIdleFunc(idle);


    int world=glutCreateSubWindow(window, 0, 0, 200, 750);
    glutDisplayFunc(display);

    glutMainLoop();
	return 0;
}
