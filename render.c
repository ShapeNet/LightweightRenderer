/*
 * Off-screen Mesa rendering using assimp loader
 *
 * Modified from osdemo (demo of offscreen rendering from Mesa3D) and Assimp sample program SimpleTexturedOpenGL
 *
 * If you want to render BIG images you'll probably have to increase
 * MAX_WIDTH and MAX_Height in src/config.h.
 *
 * This program is in the public domain.
 *
 * Hao Su
 *
 * PPM output provided by Joerg Schmalzl.
 * ASCII PPM output added by Brian Paul.
 *
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GL/osmesa.h"
#include "gl_wrap.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <png++/png.hpp>
#include <fstream>
#include <IL/il.h>
#include <libgen.h>
#include <GL/glu.h>   

//to map image filenames to textureIds
#include <map>

#include <assimp/cimport.h>
#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

static int Width = 400;
static int Height = 400;

//////////////////////////////////////////
char *modelname;
char *pngname;

GLfloat        camx = 0.0, camy = 1.0, camz = 4.0;
GLfloat        centerx = 0.0, centery = 0.0, centerz = 0.0;
GLfloat        upx = 0.0, upy = 1.0, upz = 0.0;
GLfloat        fovy = 45.0;

GLfloat LightAmbient[]= { 0.1f, 0.1f, 0.1f, 1.0f };
GLfloat LightDiffuse[]= { 0.5f, 0.5f, 0.5f, 1.0f };

GLfloat Light1Position[]= { 15.0f, 15.0f, 15.0f, 1.0f };
GLfloat Light2Position[]= { 15.0f, 15.0f, -15.0f, 1.0f };
GLfloat Light3Position[]= { -15.0f, 15.0f, 15.0f, 1.0f };
GLfloat Light4Position[]= { -15.0f, 15.0f, -15.0f, 1.0f };
GLfloat Light5Position[]= { 15.0f, -15.0f, 15.0f, 1.0f };
GLfloat Light6Position[]= { 15.0f, -15.0f, -15.0f, 1.0f };
GLfloat Light7Position[]= { -15.0f, -15.0f, 15.0f, 1.0f };
GLfloat Light8Position[]= { -15.0f, -15.0f, -15.0f, 1.0f };

// the global Assimp scene object
const aiScene* scene = NULL;
GLuint scene_list = 0;
aiVector3D scene_min, scene_max, scene_center;

// images / texture
std::map<uint32_t, GLuint*> textureIdMap;    // map image filenames to textureIds
std::map<uint32_t, char*> textureName;    // map image filenames to textureIds

GLuint*        textureIds;                            // pointer to texture Array

// Create an instance of the Importer class
Assimp::Importer importer;

uint32_t hash(char * s) {
    uint32_t hash = 0;

    for(; *s; ++s)
    {
        hash += *s;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

/* return a new string with every instance of ch replaced by repl */
char *replace(const char *s, char ch, const char *repl) {
    int count = 0;
    const char *t;
    for(t=s; *t; t++)
        count += (*t == ch);

    size_t rlen = strlen(repl);
    char *res = (char*)malloc(strlen(s) + (rlen-1)*count + 1);
    char *ptr = res;
    for(t=s; *t; t++) {
        if(*t == ch) {
            memcpy(ptr, repl, rlen);
            ptr += rlen;
        } else {
            *ptr++ = *t;
        }
    }
    *ptr = 0;
    return res;
}

bool Import3DFromFile( const char * pFile)
{
    // Check if file exists
    std::ifstream fin(pFile);
    if(!fin.fail())
    {
        fin.close();
    }
    else
    {
        printf("Couldn't open file: %s\n", pFile);
        return false;
    }

    scene = importer.ReadFile( pFile, aiProcessPreset_TargetRealtime_Quality);

    // If the import failed, report it
    if( !scene)
    {
        return false;
    }

    // Now we can access the file's contents.
    char result[1000];
    sprintf(result, "Import of scene %s succeeded.", pFile);

    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}

int LoadGLTextures(const aiScene * scene)
{
    ILboolean success;

    /* Before calling ilInit() version should be checked. */
    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
    {
        /// wrong DevIL version ///
        char err_msg[] = "Wrong DevIL version. Old devil.dll in system32/SysWow64?";
        return -1;
    }

    ilInit(); /* Initialization of DevIL */

    // if (scene->HasTextures()) abortGLInit("Support for meshes with embedded textures is not implemented");

    /* getTexture Filenames and Numb of Textures */
    for (unsigned int m=0; m<scene->mNumMaterials; m++)
    {
        int texIndex = 0;
        aiReturn texFound = AI_SUCCESS;

        aiString path;    // filename

        while (true)
        {
            texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
            if (texFound != AI_SUCCESS)
                break;
            
            char * filename_unix = replace(path.data, '\\', "/");
            textureIdMap[hash(filename_unix)] = NULL; //fill map with textures, pointers still NULL yet
            textureName[hash(filename_unix)] = filename_unix; //fill map with textures, pointers still NULL yet
            texIndex++;
        }
    }


    int numTextures = textureIdMap.size();

    /* array with DevIL image IDs */
    ILuint* imageIds = NULL;
    imageIds = new ILuint[numTextures];

    /* generate DevIL Image IDs */
    ilGenImages(numTextures, imageIds); /* Generation of numTextures image names */

    /* create and fill array with GL texture ids */
    textureIds = new GLuint[numTextures];
    glGenTextures(numTextures, textureIds); /* Texture name generation */

    /* get iterator */
    std::map<uint32_t, GLuint*>::iterator itr = textureIdMap.begin();

    char basepath[1000];
    strcpy(basepath, modelname);
    dirname(basepath);
    for (int i=0; i<numTextures; i++)
    {

        //save IL image ID
        char filename[1000];
        char * filename_unix = textureName[(*itr).first];
        (*itr).second =  &textureIds[i];      // save texture id for filename in map
        itr++;                                  // next texture

        ilBindImage(imageIds[i]); /* Binding of DevIL image name */
        char fileloc[1000];
        sprintf(fileloc, "%s/%s", basepath, filename_unix);    /* Loading of image */
        success = ilLoadImage(fileloc);

        if (success) /* If no error occured: */
        {
            // Convert every colour component into unsigned byte.If your image contains 
            // alpha channel you can replace IL_RGB with IL_RGBA
            success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
            if (!success)
            {
                /* Error occured */
                // abortGLInit("Couldn't convert image");
                return -1;
            }
            // Binding of texture name
            glBindTexture(GL_TEXTURE_2D, textureIds[i]); 
            // redefine standard texture values
            // We will use linear interpolation for magnification filter
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            // We will use linear interpolation for minifying filter
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            // Texture specification
            glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
                    ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
                    ilGetData()); 
            // we also want to be able to deal with odd texture dimensions
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
            glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
            glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
        }
        else
        {
            /* Error occured */
            printf("Couldn't load Image: %s\n", fileloc);
        }
    }
    // Because we have already copied image data into texture data  we can release memory used by image.
    ilDeleteImages(numTextures, imageIds); 

    // Cleanup
    delete [] imageIds;
    imageIds = NULL;

    return true;
}

// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
void Color4f(const aiColor4D *color)
{
    glColor4f(color->r, color->g, color->b, color->a);
}

void set_float4(float f[4], float a, float b, float c, float d)
{
    f[0] = a;
    f[1] = b;
    f[2] = c;
    f[3] = d;
}

void color4_to_float4(const aiColor4D *c, float f[4])
{
    f[0] = c->r;
    f[1] = c->g;
    f[2] = c->b;
    f[3] = c->a;
}

void apply_material(const aiMaterial *mtl)
{
    float c[4];

    GLenum fill_mode;
    int ret1, ret2;
    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D ambient;
    aiColor4D emission;
    float shininess, strength;
    int two_sided;
    int wireframe;
    unsigned int max;    // changed: to unsigned

    int texIndex = 0;
    aiString texPath;    //contains filename of texture

    {
        aiString path;    // filename
        int texFound = mtl->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        char * filename_unix = replace(path.data, '\\', "/");
    }

    if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath))
    {
        //bind texture
        char * filename_unix = replace(texPath.data, '\\', "/");
        unsigned int texId = *textureIdMap[hash(filename_unix)];
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        color4_to_float4(&diffuse, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

    set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
        color4_to_float4(&specular, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

    set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
        color4_to_float4(&ambient, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
        color4_to_float4(&emission, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

    max = 1;
    ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
    max = 1;
    ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
    if((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
    else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
        set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
    }

    max = 1;
    if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
        fill_mode = wireframe ? GL_LINE : GL_FILL;
    else
        fill_mode = GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

    max = 1;
    if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    glDisable(GL_CULL_FACE); ///////////////
}


void recursive_render(const struct aiScene * sc, const struct aiNode * nd, float scale)
{
    unsigned int i;
    unsigned int n=0, t;
    aiMatrix4x4 m = nd->mTransformation;

    aiMatrix4x4 m2;
    aiMatrix4x4::Scaling(aiVector3D(scale, scale, scale), m2);
    m = m * m2;

    // update transform
    m.Transpose();
    glPushMatrix();
    glMultMatrixf((float*)&m);

    // draw all meshes assigned to this node
    for (; n < nd->mNumMeshes; ++n)
    {
        const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

        apply_material(sc->mMaterials[mesh->mMaterialIndex]); 

        if(mesh->mNormals == NULL)
        {
            glDisable(GL_LIGHTING);
        }
        else
        {
            glEnable(GL_LIGHTING);            
        }

        if(mesh->mColors[0] != NULL)
        {
            glEnable(GL_COLOR_MATERIAL);
        }
        else
        {
            glDisable(GL_COLOR_MATERIAL);
        }

        for (t = 0; t < mesh->mNumFaces; ++t) {
            const struct aiFace* face = &mesh->mFaces[t];
            GLenum face_mode;

            switch(face->mNumIndices)
            {
                case 1: face_mode = GL_POINTS; break;
                case 2: face_mode = GL_LINES; break;
                case 3: face_mode = GL_TRIANGLES; break;
                default: face_mode = GL_POLYGON; break;
            }

            glBegin(face_mode);
            int v0 = face->mIndices[0];
            int v1 = face->mIndices[1];
            int v2 = face->mIndices[2];
            glm::vec3 p0(mesh->mVertices[v0].x, mesh->mVertices[v0].y, mesh->mVertices[v0].z);
            glm::vec3 p1(mesh->mVertices[v1].x, mesh->mVertices[v1].y, mesh->mVertices[v1].z);
            glm::vec3 p2(mesh->mVertices[v2].x, mesh->mVertices[v2].y, mesh->mVertices[v2].z);
            
            glm::vec3 res = glm::cross(p1-p0, p2-p0);
            res = -glm::normalize(res);            
            
            for(i = 0; i < face->mNumIndices; i++)        // go through all vertices in face
            {
                int vertexIndex = face->mIndices[i];    // get group index for current index
                if(mesh->mColors[0] != NULL)
                    Color4f(&mesh->mColors[0][vertexIndex]);
                if(mesh->mNormals != NULL) 
                    if(mesh->HasTextureCoords(0))        //HasTextureCoords(texture_coordinates_set)
                    {
                        glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y); //mTextureCoords[channel][vertex]
                    }

                glNormal3fv(&res[0]);            
                // glColor3fv(&res[0]);            
                glVertex3fv(&mesh->mVertices[vertexIndex].x);
            }
            glEnd();
        }
    }

    // draw all children
    for (n = 0; n < nd->mNumChildren; ++n)
    {
        recursive_render(sc, nd->mChildren[n], scale);
    }

    glPopMatrix();
}


void drawAiScene(const aiScene* scene)
{
    recursive_render(scene, scene->mRootNode, 1);
}


//////////////////////////////////////////
float camDist = 4.0f;

// All Setup For OpenGL goes here
int InitGL(int width, int height)
{
    if (!LoadGLTextures(scene))
    {
        return false;
    }

    glViewport(0, 0, width, height);                    // Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glLoadIdentity();                            // Reset The Projection Matrix

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(fovy,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
    gluLookAt(camx, camy, camz,
              centerx, centery, centerz,
              upx, upy, upz);

    glMatrixMode(GL_MODELVIEW);                        // Select The Modelview Matrix
    glLoadIdentity();       

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);         // Enables Smooth Shading
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);                // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);            // The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    // Really Nice Perspective Calculation

    glEnable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    glEnable(GL_LIGHT0);    // Uses default lighting parameters        
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, Light8Position);
    glEnable(GL_NORMALIZE);
    
    glEnable(GL_LIGHT1);    
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, Light1Position);
    
    glEnable(GL_LIGHT2);    
    glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT2, GL_POSITION, Light2Position);
    
    glEnable(GL_LIGHT3);    
    glLightfv(GL_LIGHT3, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT3, GL_POSITION, Light3Position);
    
    glEnable(GL_LIGHT4);    
    glLightfv(GL_LIGHT4, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT4, GL_POSITION, Light4Position);
    
    glEnable(GL_LIGHT5);    
    glLightfv(GL_LIGHT5, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT5, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT5, GL_POSITION, Light5Position);
    
    glEnable(GL_LIGHT6);    
    glLightfv(GL_LIGHT6, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT6, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT6, GL_POSITION, Light6Position);
    
    glEnable(GL_LIGHT7);    
    glLightfv(GL_LIGHT7, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT7, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT7, GL_POSITION, Light7Position);
    

    

    return true;                    // Initialization Went OK
}


static void
render_image(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear The Screen And The Depth Buffer
    // glLoadIdentity();                // Reset MV Matrix
    // glTranslatef(0.0f, 0.0f, -camDist);    // Move 40 Units And Into The Screen

    drawAiScene(scene);

    /* This is very important!!!
     * Make sure buffered commands are finished!!!
     */
    glFinish();
}

    int
main(int argc, char *argv[])
{
    OSMesaContext ctx;
    void *buffer;

    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  render modelname pngname [width height] [camx camy camz] [centerx centerz centerz] [upx upy upz] [fovy]\n");
        fprintf(stderr, "Default: width=%d height=%d cam=[%0.4f %0.4f %0.4f] center=[%0.4f %0.4f %0.4f] up=[%0.4f %0.4f %0.4f] fovy=%0.4f\n", Width, Height, camx, camy, camz, centerx, centery, centerz, upx, upy, upz, fovy);
        return 0;
    }

    modelname = argv[1];
    pngname = argv[2];

    if (argc >= 5) {
        Width = atoi(argv[3]);
        Height = atoi(argv[4]);
    }

    if (argc >= 8) {
        camx = atoi(argv[5]);
        camy = atoi(argv[6]);
        camz = atoi(argv[7]);
    }

    if (argc >= 11) {
        centerx = atoi(argv[8]);
        centery = atoi(argv[9]);
        centerz = atoi(argv[10]);
    }

    if (argc >= 14) {
        upx = atoi(argv[11]);
        upy = atoi(argv[12]);
        upz = atoi(argv[13]);
    }
        
    if (argc >= 15) {
        fovy = atoi(argv[14]);
    }

    if (!Import3DFromFile(modelname)) {
        fprintf(stderr, "model cannot be loaded!\n");
        return 0;
    }    

    /* Create an RGBA-mode context */
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
    /* specify Z, stencil, accum sizes */
    ctx = OSMesaCreateContextExt( OSMESA_RGBA, 16, 0, 0, NULL );
#else
    ctx = OSMesaCreateContext( OSMESA_RGBA, NULL );
#endif
    if (!ctx) {
        printf("OSMesaCreateContext failed!\n");
        return 0;
    }

    /* Allocate the image buffer */
    buffer = malloc( Width * Height * 4 * sizeof(GLubyte) );
    if (!buffer) {
        printf("Alloc image buffer failed!\n");
        return 0;
    }

    /* Bind the buffer to the context and make it current */
    if (!OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, Width, Height )) {
        printf("OSMesaMakeCurrent failed!\n");
        return 0;
    }

    {
        int z, s, a;
        glGetIntegerv(GL_DEPTH_BITS, &z);
        glGetIntegerv(GL_STENCIL_BITS, &s);
        glGetIntegerv(GL_ACCUM_RED_BITS, &a);
        printf("Depth=%d Stencil=%d Accum=%d\n", z, s, a);
    }

    InitGL(Width, Height);
    render_image();

    if (pngname != NULL) {
        png::image< png::rgba_pixel > image(Width, Height);
        GLubyte * p_buffer = (GLubyte*)buffer;
        for (png::uint_32 y = 0; y < Height; ++y)
        {
            for (png::uint_32 x = 0; x < Width; ++x)
            {
                png::uint_32 r, g, b, a;
                r = *(p_buffer++);
                g = *(p_buffer++);
                b = *(p_buffer++);
                a = *(p_buffer++);
                image[Height-1-y][x] = png::rgba_pixel(r, g, b, a);
            }
        }
        image.write(pngname);
    }
    else {
        printf("Specify a filename if you want to make an image file\n");
    }

    printf("all done\n");

    /* free the image buffer */
    free( buffer );

    // *** cleanup ***
    textureIdMap.clear(); //no need to delete pointers in it manually here. (Pointers point to textureIds deleted in next step)
    textureName.clear(); //no need to delete pointers in it manually here. (Pointers point to textureIds deleted in next step)

    if (textureIds)
    {
        delete[] textureIds;
        textureIds = NULL;
    }

    /* destroy the context */
    OSMesaDestroyContext( ctx );

    return 0;
}
