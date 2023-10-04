﻿#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;


/*
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀X⡀⠀⠀⠀⠀⠀0,1,1⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣴⣾⣿⣿⣷⣦⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
0,1,0⠀⢀⣠⣴⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣦⣄⡀⠀⠀⠀⠀⠀
⠀⠀⠀X⠰⢾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡷X⠆⠀⠀⠀1,1,1
⠀⠀⢸⣷⣦⣈⠙⠛⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠛⠋⣁⣴⣾⡇⠀⠀
⠀⠀⢸⣿⣿⣿⣿⣷⣦⣄⣉⠛⠻⢿⣿⣿⡿⠟⠛⣉⣠⣴⣾⣿⣿⣿⣿⡇⠀⠀
⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣤⡌X⢡⣤⣶⣿⣿⣿⣿⣿⣿⣿⣿⠀1,1,0
⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀
⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀
⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀0,0,1
⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿X⡇⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀
⠀⠀X⠘⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃X⠀⠀1,0,1
⠀⠀⠀⠀⠀⠉⠛⠿⣿⣿⣿⣿⣿⣿⡇⢸⣿⣿⣿⣿⣿⣿⠿⠛⠉⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠛⠿⣿⡇X⢸⣿⠿⠛⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀0,0,0⠀⠀⠀⠀⠀⠀⠀⠁1,0,0⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀

*/


/*Notes: Strcut is used to group several related variables into one place.
    Used for lightweight objects*/
struct vec3d
{
    //Coords for 3d space
    float x, y, z;
};

//Contains 3 vertices and define the outline of a triangle
struct triangle
{
    vec3d p[3];

    wchar_t sym;
    short col;

};

//Represents object its a bunch of triangles
struct mesh
{
    vector <triangle> tris;

    bool LoadFromObjectFile(string sFilename)
    {
        ifstream f(sFilename);
        if (!f.is_open())
            return false;

        //Local cache of verts
        vector<vec3d> verts;

        while(!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            strstream s;
            s << line;

            //temp
            char junk;

            //Reading in ver
            if (line[0] == 'v')
            {
                vec3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }

            //Making a triangle
            if (line[0] == 'f')
            {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            }
        }

        return true;
    }
};

//Matrix struct
struct mat4x4
{
    //Row followed by column
    float m[4][4] = { 0 };
};


/*Notes: Normal - An object that is perpendicular to a given object (usually a plane). It is invisible*/
class olcEngine3D : public olcConsoleGameEngine
{
    public:
        olcEngine3D()
        {
            /*Note: L before "" makes it a wide string*/
            m_sAppName = L"3D Demo";
        }

private:
    mesh meshCube;
    mat4x4 matProj;

    vec3d vCamera;

    float fTheta;

    //For matrix vector multiplication
    //Input, output, matrix
    void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
    {
        //x,y,z written directly to the output vector
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        //This is implied as 1 so it can sum the final matrix element
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];


        if (w != 0.0f)
        {
            o.x /= w; o.y /= w; o.z /= w;
        }
    
    }

    CHAR_INFO GetColour(float lum)
    {
        short bg_col, fg_col;
        wchar_t sym;
        int pixel_bw = (int)(13.0f * lum);
        switch (pixel_bw)
        {
        case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

        case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
        case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
        case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
        case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

        case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
        case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
        case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
        case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

        case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
        case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
        case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
        case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
        default:
            bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
        }

        CHAR_INFO c;
        c.Attributes = bg_col | fg_col;
        c.Char.UnicodeChar = sym;
        return c;
    }

    public:
        bool OnUserCreate() override
        {
            //Populates mesh to create a cube made out of triangles
            //meshCube.tris =
            //{
                /*Notes: Sub initializer list initializes the triangle with 3 vectors. While outside initializer initializes the standard vector*/
                // SOUTH
                //{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
                //{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

                // EAST                                                      
                //{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
               // { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

                // NORTH                                                     
               // { 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
               // { 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

                // WEST                                                      
               // { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
               // { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

                // TOP                                                       
               // { 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
                //{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

                // BOTTOM                                                    
               // { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
               // { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

           // };

            //loads object file
            meshCube.LoadFromObjectFile("VideoShip.obj");
            
            //Projection Matrix
            //These are a bunch of calculations and yes they hurt my head
            float fNear = 0.1f;
            float fFar = 1000.0f;
            float fFov = 90.0f;
            float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
            float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

            matProj.m[0][0] = fAspectRatio * fFovRad;
            matProj.m[1][1] = fFovRad;
            matProj.m[2][2] = fFar / (fFar - fNear);
            matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
            matProj.m[2][3] = 1.0f;
            matProj.m[3][3] = 0.0f;

            return true;
        }

        /*Notes: Projection concept -> Condensing a 3d space to a 2d screen*/

        bool OnUserUpdate(float fElapsedTime) override
        {
            //Clears screen
            Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

            mat4x4 matRotZ, matRotX;
            fTheta += 1.0f * fElapsedTime;

            // Rotation Z
            matRotZ.m[0][0] = cosf(fTheta);
            matRotZ.m[0][1] = sinf(fTheta);
            matRotZ.m[1][0] = -sinf(fTheta);
            matRotZ.m[1][1] = cosf(fTheta);
            matRotZ.m[2][2] = 1;
            matRotZ.m[3][3] = 1;

            // Rotation X
            matRotX.m[0][0] = 1;
            matRotX.m[1][1] = cosf(fTheta * 0.5f);
            matRotX.m[1][2] = sinf(fTheta * 0.5f);
            matRotX.m[2][1] = -sinf(fTheta * 0.5f);
            matRotX.m[2][2] = cosf(fTheta * 0.5f);
            matRotX.m[3][3] = 1;

            vector<triangle> vecTrianglesToRaster;

            //Draw triangles
            /*Notes: auto in a for loop is used to avoid long initializations*/
            for (auto tri : meshCube.tris)
            {
                triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

                /*Order of translations matter!!!!!*/

                // Rotate in Z-Axis
                MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
                MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
                MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

                // Rotate in X-Axis
                MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
                MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
                MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

                /*Notes: Painters algorithm, draw the things furthest away first then closest last*/
                // Offset into the screen
                triTranslated = triRotatedZX;
                triTranslated.p[0].z = triRotatedZX.p[0].z + 8.0f;
                triTranslated.p[1].z = triRotatedZX.p[1].z + 8.0f;
                triTranslated.p[2].z = triRotatedZX.p[2].z + 8.0f;

                // Use Cross-Product to get surface normal
                //Revisit this math it hurts me head
                vec3d normal, line1, line2;
                line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
                line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
                line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

                line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
                line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
                line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

                normal.x = line1.y * line2.z - line1.z * line2.y;
                normal.y = line1.z * line2.x - line1.x * line2.z;
                normal.z = line1.x * line2.y - line1.y * line2.x;

                //Normalize the normal
                float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
                normal.x /= l; normal.y /= l; normal.z /= l;

                //Calculating the Dot product (how similar two vectors are)
                if (normal.x * (triTranslated.p[0].x - vCamera.x) +
                    normal.y * (triTranslated.p[0].y - vCamera.y) +
                    normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f)
                {
                    //Shade the triangle if it can be seen
                    vec3d light_direction = { 0.0f, 0.0f, -1.0f };

                    //Normalize it
                    float l = sqrtf(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);
                    light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;

                    //Calculate the Dot Product
                    float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;

                    //Extract character and symbol info to represent a shade of grey
                    CHAR_INFO c = GetColour(dp);
                    triTranslated.col = c.Attributes;
                    triTranslated.sym = c.Char.UnicodeChar;

                    //Triangles from 3D to 2D
                    MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
                    MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
                    MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
                    triProjected.col = triTranslated.col;
                    triProjected.sym = triTranslated.sym;

                    //Scale into view
                    triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
                    triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
                    triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

                    triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
                    triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
                    triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
                    triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
                    triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
                    triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

                    //Store triangle
                    vecTrianglesToRaster.push_back(triProjected);

                    /*FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
                        triProjected.p[1].x, triProjected.p[1].y,
                        triProjected.p[2].x, triProjected.p[2].y,
                        triProjected.sym, triProjected.col);*/
                }

                //Sort triangles using painters algorithm
                sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
                    {
                        float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
                        float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
                        return z1 > z2;
                    });

                for (auto& triProjected : vecTrianglesToRaster)
                {
                    // Rasterize triangle
                    FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
                        triProjected.p[1].x, triProjected.p[1].y,
                        triProjected.p[2].x, triProjected.p[2].y,
                        triProjected.sym, triProjected.col);

                }
            }


            return true;
        }
};

int main()
{
    //Instance of the class created
    olcEngine3D demo;

    //Constructs the window if it fails it prints error
    if (demo.ConstructConsole(256, 240, 4, 4))
        demo.Start();
    else
        std::cout << "Error";


    return 0;
}
