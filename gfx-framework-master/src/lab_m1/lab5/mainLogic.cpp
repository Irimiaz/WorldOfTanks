#include "lab_m1/lab5/mainLogic.h"

#include <vector>
#include <string>
#include <iostream>
#include "lab_m1/lab5/movement.h"
#include <math.h>
#include "components/text_renderer.h"
using namespace std;
using namespace m1;

#define CAMERASENSITIVITY 0.001f
#define TURRETSENSITIVITY 0.008f
#define CANNONBALLSPEED 7
#define SCALE 0.005f
#define MOVESPEED 3
#define ENEMIESNUMBER 5
#define ENEMIESMOVEMENTTIMER 4
#define ENEMYHITRADIUS 10
#define WORLDBORDER 48
/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */



Lab5::Lab5()
{
}


Lab5::~Lab5()
{
}

float distanceCalculator(float x1, float z1, float x2, float z2) {
    return sqrt((x1 - x2) * (x1 - x2) + (z1 - z2) * (z1 - z2));
}


void Lab5::Init()
{
    renderCameraTarget = false;
    projectionType = true;

    camera = new implemented::Camera();
    camera->Set(glm::vec3(0, 3, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 2, 0));

    ///text ---------------------------------------------------
    textRenderer = new gfxc::TextRenderer(window->props.selfDir, 1280, 720);
    textRenderer->Load(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab5", "fonts", "Hack-Bold.ttf"), 10);
    
    ///shader + meshes ----------------------------------------
    myTank playerTank;
    playerTank.radius = 150 * SCALE;
    tanks.push_back(playerTank);
    {
        Shader* shader = new Shader("CustomShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab5", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab5", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    {
        Mesh* mesh = new Mesh("tankBody");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tanks/tankParts"), "body.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tankRightWheels");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tanks/tankParts"), "senila_dreapta.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tankLeftWheels");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tanks/tankParts"), "senila_stanga.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tankBarrel");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tanks/tankParts"), "teava.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tankHead");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tanks/tankParts"), "head.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("cannonball");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "cannonball"), "cannonball.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("ground");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "ground/source"), "ground.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("building");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "building"), "untitled.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // TODO(student): After you implement the changing of the projection
    // parameters, remove hardcodings of these parameters
    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

    ///generate buildings coordonates------------------------------------
    int numberOfBuildings = 8 + ( rand() % 15);
    for (int i = 0; i < numberOfBuildings; i++) {
        
        int x = 10 + (rand() % 20);
        int signX = 1 + (rand() % 2); 
        if (signX == 1) {
            x *= (-1);
        }
        float size = (float)(2 + (rand() % 8)) / 4;
        int z = 10 + (rand() % 20);
        int signZ = 1 + (rand() % 2);
        if (signZ == 1) {
            z *= (-1);
        }
        myBuilding newBuilding;
        newBuilding.x = x;
        newBuilding.z = z;
        newBuilding.length = 5.43 * size;
        newBuilding.size = size;
        buildings.push_back(newBuilding);
    }
    ///generate enemies--------------------------------------------
    int numberOfEnemies = ENEMIESNUMBER;
    for (int i = 0; i < numberOfEnemies; i++) {
        int x = 8 + (rand() % 20);
        int signX = 1 + (rand() % 2);
        if (signX == 1) {
            x *= (-1);
        }
        int z = 8 + (rand() % 20);
        int signZ = 1 + (rand() % 2);
        if (signZ == 1) {
            z *= (-1);
        }

        int ok = 1;
        for (int j = 0; j < numberOfBuildings; j++) {
            if (x + 4 >= buildings.at(j).x - buildings.at(j).length / 2 && x - 4 <= buildings.at(j).x + buildings.at(j).length / 2) {
                if (z + 4 >= buildings.at(j).z - buildings.at(j).length / 2 && z - 4 <= buildings.at(j).z + buildings.at(j).length / 2) {
                    ok = 0;
                }
            }
        }
        for (int j = 0; j < tanks.size(); j++) {
            myTank tank = tanks.at(j);
            if (distanceCalculator(x, z, tank.x, tank.z) <= tank.radius + 150 * SCALE) {
                ok = 0;
            }
        }
        if (ok == 1) {
            float orientation = (float)(0 + (rand() % 9)) / M_PI;
            myTank newTank;
            newTank.radius = 150 * SCALE;
            newTank.z = z;
            newTank.x = x;
            newTank.orientation = orientation;
            tanks.push_back(newTank);
        }
        else {
            i--;
        }

    }
    game.numberOfBuildings = numberOfBuildings;
    game.numberOfTanks = numberOfEnemies;
    tanks.at(0).cooldown = 1;
    
    
}

Mesh* Lab5::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    // Create the VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create the VBO and bind it
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // Create the IBO and bind it
    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));
    // ========================================================================

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    CheckOpenGLError();

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
    meshes[name]->vertices = vertices;
    meshes[name]->indices = indices;
    return meshes[name];
}

void Lab5::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int health)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // TODO(student): Get shader location for uniform mat4 "Model"
    int location = glGetUniformLocation(shader->GetProgramID(), "Model");

    // TODO(student): Set shader uniform "Model" to modelMatrix
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // TODO(student): Get shader location for uniform mat4 "View"
    int location2 = glGetUniformLocation(shader->GetProgramID(), "View");

    // TODO(student): Set shader uniform "View" to viewMatrix
    //glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    //glUniformMatrix4fv(location2, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    // TODO(student): Get shader location for uniform mat4 "Projection"
    int location3 = glGetUniformLocation(shader->GetProgramID(), "Projection");
    shader->Use();
    glUniformMatrix4fv(location2, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    //glUniformMatrix4fv(location3, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
    // Bonus
    double time = Engine::GetElapsedTime();
    int location5 = glGetUniformLocation(shader->GetProgramID(), "Health");

    glUniform1f(location5, health + 2);

    // TODO(student): Get shader location for uniform mat4 "Projection"

    // TODO(student): Set shader uniform "Projection" to projectionMatrix
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    glUniformMatrix4fv(location3, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Lab5::FrameStart()
{
    //color the sky ------------------------------------------------------------------------
    glClearColor(0.6, 0.9, 0.9, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}
///collision check for enemies ------------------------------------------------------------------------
bool Lab5::collisionCheck(int tankNumber, float deltaTimeSeconds) {
    myTank tank = tanks.at(tankNumber);
    if (tank.movement == "FORWARDS") {
        
        int ok = 1;
        int tankok = 1;
        int borderok = 1;
        for (int i = 0; i < buildings.size(); i++) {

            if (tank.x + tank.radius + 0.25 >= buildings.at(i).x - buildings.at(i).length / 2 && tank.x - tank.radius - 0.25 <= buildings.at(i).x + buildings.at(i).length / 2) {
                if (tank.z + tank.radius + 0.25 >= buildings.at(i).z - buildings.at(i).length / 2 && tank.z - tank.radius - 0.25 <= buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 0;
                }
            }
            if (tank.x + tank.radius + 0.25 >= WORLDBORDER || tank.z + tank.radius + 0.25 >= WORLDBORDER || tank.x + tank.radius - 0.25 <= -WORLDBORDER || tank.z + tank.radius - 0.25 <= -WORLDBORDER) {
                borderok = 0;
            }
            int collisionTank = -1;
            for (int j = 0; j < tanks.size() && j != tankNumber; j++) {
                myTank collidedTank = tanks.at(j);
                if (distanceCalculator(collidedTank.x, collidedTank.z, tank.x, tank.z) <= collidedTank.radius + tank.radius) {
                    tankok = 0;
                    collisionTank = j;
                }
            }


            float nextPozX = tank.x + MOVESPEED * deltaTimeSeconds * (sin(tank.orientation)) * 1.5;
            float nextPozZ = tank.z + MOVESPEED * deltaTimeSeconds * cos(tank.orientation) * 1.5;
            if (nextPozX + tank.radius >= buildings.at(i).x - buildings.at(i).length / 2 && nextPozX - tank.radius <= buildings.at(i).x + buildings.at(i).length / 2) {
                if (nextPozZ + tank.radius < buildings.at(i).z - buildings.at(i).length / 2 || nextPozZ - tank.radius> buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (nextPozZ + tank.radius >= buildings.at(i).z - buildings.at(i).length / 2 && nextPozZ - tank.radius <= buildings.at(i).z + buildings.at(i).length / 2) {
                if (nextPozX + tank.radius < buildings.at(i).x - buildings.at(i).length / 2 || nextPozX - tank.radius> buildings.at(i).x + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (collisionTank != -1 && distanceCalculator(nextPozX, nextPozZ, tanks.at(collisionTank).x, tanks.at(collisionTank).z) > tanks.at(tankNumber).radius + tanks.at(collisionTank).radius) {
                tankok = 1;
            }
            if (nextPozX + tank.radius > -WORLDBORDER && nextPozX + tank.radius < WORLDBORDER && nextPozZ + tank.radius <WORLDBORDER && nextPozZ + tank.radius > -WORLDBORDER) {
                borderok = 1;
            }
            if (ok == 0 || tankok == 0 || borderok == 0) {
                break;
            }
        }
        if (ok == 1 && tankok == 1 && borderok == 1) {
            return false;
        }
        return true;
    }
    else if (tank.movement == "BACKWARDS") {
        int ok = 1;
        int tankok = 1;
        int borderok = 1;
        for (int i = 0; i < buildings.size(); i++) {

            if (tank.x + tank.radius + 0.25 >= buildings.at(i).x - buildings.at(i).length / 2 && tank.x - tank.radius - 0.25 <= buildings.at(i).x + buildings.at(i).length / 2) {
                if (tank.z + tank.radius + 0.25 >= buildings.at(i).z - buildings.at(i).length / 2 && tank.z - tank.radius - 0.25 <= buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 0;
                }
            }
            if (tank.x + tank.radius + 0.25 >= WORLDBORDER || tank.z + tank.radius + 0.25 >= WORLDBORDER || tank.x + tank.radius - 0.25 <= -WORLDBORDER || tank.z + tank.radius - 0.25 <= -WORLDBORDER) {
                borderok = 0;
            }


            int collisionTank = -1;
            for (int j = 0; j < tanks.size() && j != tankNumber; j++) {
                myTank collidedTank = tanks.at(j);
                if (distanceCalculator(collidedTank.x, collidedTank.z, tank.x, tank.z) <= collidedTank.radius + tank.radius) {
                    tankok = 0;
                    collisionTank = j;
                }
            }


            float nextPozX = tank.x - MOVESPEED * deltaTimeSeconds * (sin(tank.orientation)) * 1.5;
            float nextPozZ = tank.z - MOVESPEED * deltaTimeSeconds * cos(tank.orientation) * 1.5;
            if (nextPozX + tank.radius >= buildings.at(i).x - buildings.at(i).length / 2 && nextPozX - tank.radius <= buildings.at(i).x + buildings.at(i).length / 2) {
                if (nextPozZ + tank.radius < buildings.at(i).z - buildings.at(i).length / 2 || nextPozZ - tank.radius> buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (nextPozZ + tank.radius >= buildings.at(i).z - buildings.at(i).length / 2 && nextPozZ - tank.radius <= buildings.at(i).z + buildings.at(i).length / 2) {
                if (nextPozX + tank.radius < buildings.at(i).x - buildings.at(i).length / 2 || nextPozX - tank.radius> buildings.at(i).x + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (collisionTank != -1 && distanceCalculator(nextPozX, nextPozZ, tanks.at(collisionTank).x, tanks.at(collisionTank).z) > tanks.at(tankNumber).radius + tanks.at(collisionTank).radius) {
                tankok = 1;
            }
            if (nextPozX + tank.radius > -WORLDBORDER && nextPozX + tank.radius < WORLDBORDER && nextPozZ + tank.radius <WORLDBORDER && nextPozZ + tank.radius > -WORLDBORDER) {
                borderok = 1;
            }
            if (ok == 0 || tankok == 0 || borderok == 0) {
                break;
            }
        }
        if (ok == 1 && tankok == 1 && borderok == 1) {
            return false;
        }
        return true;
    }
    return false;
}


void Lab5::Update(float deltaTimeSeconds)
{

    glm::mat4 modelMatrix;
    
    //enemies ----------------
    for (int i = 0; i < tanks.size(); i++) {
        if (tanks.at(i).hp == 0) {
            tanks.at(i).dead = true;
            tanks.at(i).hp = -1;
            game.score++;
        }
        else {
            string shaderType;
            shaderType = "CustomShader";

            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
            modelMatrix *= transform3D::Scale(INT_MIN, INT_MIN, INT_MIN);
            RenderMesh(meshes["tankBody"], shaders["Simple"], modelMatrix);

            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(tanks.at(i).x, 0, tanks.at(i).z));
            modelMatrix *= transform3D::RotateOY(tanks.at(i).orientation);
            modelMatrix *= transform3D::Scale(SCALE, SCALE, SCALE);
            RenderSimpleMesh(meshes["tankBody"], shaders[shaderType], modelMatrix, tanks.at(i).hp);

            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(tanks.at(i).x, 0, tanks.at(i).z));
            modelMatrix *= transform3D::RotateOY(tanks.at(i).orientation);
            modelMatrix *= transform3D::RotateOY(tanks.at(i).turretOrientation);
            modelMatrix *= transform3D::Scale(SCALE, SCALE, SCALE);
            RenderSimpleMesh(meshes["tankHead"], shaders[shaderType], modelMatrix, tanks.at(i).hp);

            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(tanks.at(i).x, 0, tanks.at(i).z));
            modelMatrix *= transform3D::RotateOY(tanks.at(i).orientation);
            modelMatrix *= transform3D::RotateOY(tanks.at(i).turretOrientation);
            modelMatrix *= transform3D::Scale(SCALE, SCALE, SCALE);
            RenderSimpleMesh(meshes["tankBarrel"], shaders[shaderType], modelMatrix, tanks.at(i).hp);

            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(tanks.at(i).x, 0, tanks.at(i).z));
            modelMatrix *= transform3D::RotateOY(tanks.at(i).orientation);
            modelMatrix *= transform3D::Scale(SCALE, SCALE, SCALE);
            RenderSimpleMesh(meshes["tankRightWheels"], shaders[shaderType], modelMatrix, tanks.at(i).hp);

            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(tanks.at(i).x, 0, tanks.at(i).z));
            modelMatrix *= transform3D::RotateOY(tanks.at(i).orientation);
            modelMatrix *= transform3D::Scale(SCALE, SCALE, SCALE);
            RenderSimpleMesh(meshes["tankLeftWheels"], shaders[shaderType], modelMatrix, tanks.at(i).hp);
        }
    }

    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
    modelMatrix *= transform3D::Scale(50, 1, 50);
    RenderMesh(meshes["ground"], shaders["Simple"], modelMatrix);
    //shooting cannonballs -------------------------------------
    for (int i = 0; i < cannonBalls.size(); i ++) {
        modelMatrix = glm::mat4(1);
        int ok = 0;
        for (int j = 0; j < buildings.size() && ok == 0; j++) {
            if (cannonBalls.at(i).realX >= buildings.at(j).x - buildings.at(j).length/2 && cannonBalls.at(i).realX <= buildings.at(j).x + buildings.at(j).length/2) {
                if (cannonBalls.at(i).realZ >= buildings.at(j).z - buildings.at(j).length/2 && cannonBalls.at(i).realZ <= buildings.at(j).z + buildings.at(j).length/2) {
                    cannonBalls.erase(cannonBalls.begin() + i);
                    i--;
                    ok = 1;
                }
            } 
        }
        if (ok == 0) {
            if (cannonBalls.at(i).dissappearTimer <= 0) {
                cannonBalls.erase(cannonBalls.begin() + i);
                i--;
            }
            else {
                cannonBalls.at(i).dissappearTimer -= deltaTimeSeconds;
                modelMatrix = glm::translate(modelMatrix, glm::vec3(cannonBalls.at(i).x, 0, cannonBalls.at(i).z));
                modelMatrix *= transform3D::RotateOY(cannonBalls.at(i).orientation + cannonBalls.at(i).turretOrientation);
                modelMatrix *= transform3D::Scale(2 *SCALE, SCALE, 2*SCALE);
                RenderMesh(meshes["cannonball"], shaders["Simple"], modelMatrix);
                cannonBalls.at(i).x += deltaTimeSeconds * CANNONBALLSPEED * sin(cannonBalls.at(i).orientation + cannonBalls.at(i).turretOrientation);
                cannonBalls.at(i).z += deltaTimeSeconds * CANNONBALLSPEED * cos(cannonBalls.at(i).orientation + cannonBalls.at(i).turretOrientation);
                cannonBalls.at(i).realX += deltaTimeSeconds * CANNONBALLSPEED * sin(cannonBalls.at(i).orientation + cannonBalls.at(i).turretOrientation);
                cannonBalls.at(i).realZ += deltaTimeSeconds * CANNONBALLSPEED * cos(cannonBalls.at(i).orientation + cannonBalls.at(i).turretOrientation);
            }
        }
    }
    //set cooldowns ----------
    for (int i = 0; i < tanks.size(); i++) {
        tanks.at(i).cooldown -= deltaTimeSeconds;

    }

    //buildings ------------------
    for (int i = 0; i < buildings.size(); i++) {
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(buildings.at(i).x, 0, buildings.at(i).z));
        modelMatrix *= transform3D::Scale(buildings.at(i).size, buildings.at(i).size, buildings.at(i).size);
        RenderMesh(meshes["building"], shaders["Simple"], modelMatrix);
    }
    
    
    //enemies movement --------------------------------
    for (int i = 1; i < tanks.size(); i++) {
        tanks.at(i).movementTimer -= deltaTimeSeconds;
        if (tanks.at(i).dead == true) tanks.at(i).movement = "STILL";
        if (tanks.at(i).movementTimer <= 0 && tanks.at(i).dead == false) {
            tanks.at(i).movementTimer = ENEMIESMOVEMENTTIMER;
            int movementType = rand() % 4;
            if (movementType == 0) tanks.at(i).movement = "FORWARDS";
            else if (movementType == 1) tanks.at(i).movement = "BACKWARDS";
            else if (movementType == 2) tanks.at(i).movement = "RIGHT";
            else if (movementType == 3) tanks.at(i).movement = "LEFT";
        }
        if (collisionCheck(i, deltaTimeSeconds) == false) {
            if (tanks.at(i).movement == "FORWARDS") {
                tanks.at(i).x += MOVESPEED * deltaTimeSeconds * sin(tanks.at(i).orientation);
                tanks.at(i).z += MOVESPEED * deltaTimeSeconds * cos(tanks.at(i).orientation);
            }
            else if (tanks.at(i).movement == "BACKWARDS") {
                tanks.at(i).x -= MOVESPEED * deltaTimeSeconds * sin(tanks.at(i).orientation);
                tanks.at(i).z -= MOVESPEED * deltaTimeSeconds * cos(tanks.at(i).orientation);
            }
            else if (tanks.at(i).movement == "RIGHT") {
                tanks.at(i).orientation -= deltaTimeSeconds;
            }
            else if (tanks.at(i).movement == "LEFT") {
                tanks.at(i).orientation += deltaTimeSeconds;
            }
        }
        else {
            tanks.at(i).movementTimer = 0;
        }
    }
    ///hit enemies
    for (int i = 0; i < tanks.size(); i++) {
        for (int j = 0; j < cannonBalls.size(); j++) {
            myCannonball ball = cannonBalls.at(j);
            myTank tank = tanks.at(i);
            if (distanceCalculator(ball.realX, ball.realZ, tank.x, tank.z) <= tank.radius && ball.shotBy != i) {
                cannonBalls.erase(cannonBalls.begin() + j);
                j--;
                if (tanks.at(i).hp > 0) {
                   tanks.at(i).hp--;
                }
            }
        }
    }
    ///end game
    game.timer -= deltaTimeSeconds;
    if (game.timer < 0 && game.endScreen == 1) {
        game.displayScore = true;
        //cout <<endl<< "You have " << game.score << " points!" << endl;
        for(int i = 0; i < tanks.size(); i++) {
            tanks.at(i).dead = true;
        }
        game.endScreen = 0;
    }
    if (tanks.at(0).hp == 0) {
        for (int i = 0; i < tanks.size(); i++) {
            tanks.at(i).dead = true;
        }
        game.displayDeath = true;
        //cout << endl << "You have died! :(" << endl;
    }
    if (game.score == ENEMIESNUMBER && game.endScreen == 1) {
        game.displayScore = true;
        //cout << endl << "You have " << game.score << " points!" << endl;
        for (int i = 0; i < tanks.size(); i++) {
            tanks.at(i).dead = true;
        }
        game.endScreen = 0;
    }


    //enemy turn
    for (int i = 1; i < tanks.size(); i++) {
        myTank tank = tanks.at(i);
        if (distanceCalculator(tank.x, tank.z, tanks.at(0).x, tanks.at(0).z) <= ENEMYHITRADIUS && tank.dead == false) {
            float angle = atan2(tank.x - tanks.at(0).x, tank.z - tanks.at(0).z);
            
            float angle1 = tanks.at(i).turretOrientation;
            float angle2 = angle - tanks.at(i).orientation + M_PI;
            while (angle1 >= 2 * M_PI) angle1 -= 2 * M_PI;
            while (angle2 >= 2 * M_PI) angle2 -= 2 * M_PI;
            while (angle1 <= -2 * M_PI) angle1 += 2 * M_PI;
            while (angle2 <= -2 * M_PI) angle2 += 2 * M_PI;

            
             if (tanks.at(i).turretOrientation > angle - tanks.at(i).orientation + M_PI ) {
                tanks.at(i).turretOrientation -= 2 * deltaTimeSeconds;
                 if (abs(angle1 - angle2) <= 0.1 || abs(angle2 - angle1) <= 0.1) {
                     tanks.at(i).turretOrientation = angle - tanks.at(i).orientation + M_PI;
                 }
            }
            else if (tanks.at(i).turretOrientation < angle - tanks.at(i).orientation + M_PI ) {
                tanks.at(i).turretOrientation += 2 * deltaTimeSeconds;
                if (abs(angle1 - angle2) <= 0.1 || abs(angle2 - angle1) <= 0.1) {
                    tanks.at(i).turretOrientation = angle - tanks.at(i).orientation + M_PI;
                }
            }
             ///enemies shoot projectiles ------------------------------------------------------------------------
             if (abs (angle1 - angle2) <= 0.01) {
                 if (tanks.at(i).cooldown <= 0) {
                     myCannonball newBall;
                     newBall.x = tanks.at(i).x;
                     newBall.z = tanks.at(i).z;
                     newBall.shotBy = i;
                     newBall.orientation = tanks.at(i).orientation;
                     newBall.turretOrientation = tanks.at(i).turretOrientation;
                     newBall.realX = newBall.x + sin(newBall.orientation + newBall.turretOrientation) * SCALE * 120;
                     newBall.realZ = newBall.z + cos(newBall.orientation + newBall.turretOrientation) * SCALE * 120;
                     cannonBalls.push_back(newBall);
                     tanks.at(i).cooldown = 2;
                 }
             }
            
        }
    }
}

///display text ------------------------------------------------------------------------
void Lab5::FrameEnd()
{
    glm::ivec2 resolution = window->GetResolution();
    //DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
    if (game.displayDeath == false && game.displayScore == false) {
        textRenderer->RenderText("Health: " + std::to_string(tanks.at(0).hp), 20, 20, 3, glm::vec3(0));
        textRenderer->RenderText("Score: " + std::to_string(game.score), 20, 80, 3, glm::vec3(0));
        textRenderer->RenderText("Time: " + std::to_string((int)game.timer), 20, 140, 3, glm::vec3(0));
    }
    if (game.displayDeath == true) {
        textRenderer->RenderText("You died!",400, 250, 10, glm::vec3(0));
    }
    if (game.displayScore == true) {
        textRenderer->RenderText("Score: " + std::to_string(game.score), 400, 250, 10, glm::vec3(0));

    }
}


void Lab5::RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab5::OnInputUpdate(float deltaTime, int mods)
{
    // player movement ------------------------------------------------------------------------

    if (window->KeyHold(GLFW_KEY_A)) {
        if (tanks.at(0).dead == false) {
            tanks.at(0).orientation += deltaTime;
        }
        camera->RotateThirdPerson_OY(deltaTime );
    }
    if (window->KeyHold(GLFW_KEY_D)) {
        if (tanks.at(0).dead == false) {
            tanks.at(0).orientation -= deltaTime;
        }
        camera->RotateThirdPerson_OY(-deltaTime);

    }
    if (window->KeyHold(GLFW_KEY_W) && tanks.at(0).dead == false) {
        int ok = 1;
        int tankok = 1;
        int borderok = 1;
        for (int i = 0; i < buildings.size(); i++) {
            
            if (tanks.at(0).x + tanks.at(0).radius + 0.25 >= buildings.at(i).x - buildings.at(i).length / 2 && tanks.at(0).x - tanks.at(0).radius - 0.25<= buildings.at(i).x + buildings.at(i).length / 2) {
                if (tanks.at(0).z + tanks.at(0).radius + 0.25 >= buildings.at(i).z - buildings.at(i).length / 2 && tanks.at(0).z - tanks.at(0).radius - 0.25 <= buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 0;
                }
            }
            if (tanks.at(0).x + tanks.at(0).radius + 0.25 >= WORLDBORDER || tanks.at(0).z + tanks.at(0).radius + 0.25 >= WORLDBORDER || tanks.at(0).x + tanks.at(0).radius - 0.25 <= -WORLDBORDER || tanks.at(0).z + tanks.at(0).radius - 0.25 <= -WORLDBORDER) {
                borderok = 0;
            }
            myTank player = tanks.at(0);
            int collisionTank = -1;
            for (int j = 1; j < tanks.size(); j++) {
                myTank tank = tanks.at(j);
                if (distanceCalculator(player.x, player.z, tank.x, tank.z) <= player.radius + tank.radius) {
                    tankok = 0;
                    collisionTank = j;
                }
            }


            float nextPozX = tanks.at(0).x + MOVESPEED * deltaTime * (sin(tanks.at(0).orientation)) * 1.5;
            float nextPozZ = tanks.at(0).z + MOVESPEED * deltaTime * cos(tanks.at(0).orientation) * 1.5;
            if (nextPozX + tanks.at(0).radius  >= buildings.at(i).x - buildings.at(i).length / 2 && nextPozX - tanks.at(0).radius <= buildings.at(i).x + buildings.at(i).length / 2) {
                if (nextPozZ + tanks.at(0).radius  < buildings.at(i).z - buildings.at(i).length / 2 || nextPozZ - tanks.at(0).radius > buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (nextPozZ + tanks.at(0).radius  >= buildings.at(i).z - buildings.at(i).length / 2 && nextPozZ - tanks.at(0).radius <= buildings.at(i).z + buildings.at(i).length / 2) {
                if (nextPozX + tanks.at(0).radius < buildings.at(i).x - buildings.at(i).length / 2 || nextPozX - tanks.at(0).radius> buildings.at(i).x + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (collisionTank != -1 && distanceCalculator(nextPozX, nextPozZ, tanks.at(collisionTank).x, tanks.at(collisionTank).z) > player.radius + tanks.at(collisionTank).radius) {
                tankok = 1;
            }
            if (nextPozX + tanks.at(0).radius > -WORLDBORDER && nextPozX + tanks.at(0).radius < WORLDBORDER && nextPozZ + tanks.at(0).radius <WORLDBORDER && nextPozZ + tanks.at(0).radius > -WORLDBORDER) {
                borderok = 1;
            }
            if (ok == 0 || tankok == 0 || borderok == 0) {
                break;
            }
        }
        if (ok == 1 && tankok == 1 && borderok == 1) {
            tanks.at(0).x += MOVESPEED * deltaTime * sin(tanks.at(0).orientation);
            tanks.at(0).z += MOVESPEED * deltaTime * cos(tanks.at(0).orientation);

            camera->MoveForward(MOVESPEED * deltaTime * (cos(tanks.at(0).orientation)));
            camera->MoveRight(MOVESPEED * deltaTime * (sin(tanks.at(0).orientation)));
        }
        
    }
    if (window->KeyHold(GLFW_KEY_S) && tanks.at(0).dead == false) {
        int ok = 1;
        int tankok = 1;
        int borderok = 1;
        for (int i = 0; i < buildings.size(); i++) {

            if (tanks.at(0).x + tanks.at(0).radius + 0.25 >= buildings.at(i).x - buildings.at(i).length / 2 && tanks.at(0).x - tanks.at(0).radius - 0.25 <= buildings.at(i).x + buildings.at(i).length / 2) {
                if (tanks.at(0).z + tanks.at(0).radius + 0.25 >= buildings.at(i).z - buildings.at(i).length / 2 && tanks.at(0).z - tanks.at(0).radius - 0.25 <= buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 0;
                }
            }
            if (tanks.at(0).x + tanks.at(0).radius + 0.25 >= WORLDBORDER || tanks.at(0).z + tanks.at(0).radius + 0.25 >= WORLDBORDER || tanks.at(0).x + tanks.at(0).radius - 0.25 <= -WORLDBORDER || tanks.at(0).z + tanks.at(0).radius - 0.25 <= -WORLDBORDER) {
                borderok = 0;
            }
            myTank player = tanks.at(0);
            int collisionTank = -1;
            for (int j = 1; j < tanks.size(); j++) {
                myTank tank = tanks.at(j);
                if (distanceCalculator(player.x, player.z, tank.x, tank.z) <= player.radius + tank.radius) {
                    tankok = 0;
                    collisionTank = j;
                }
            }


            float nextPozX = tanks.at(0).x - MOVESPEED * deltaTime * (sin(tanks.at(0).orientation)) * 1.5;
            float nextPozZ = tanks.at(0).z - MOVESPEED * deltaTime * cos(tanks.at(0).orientation) * 1.5;
            if (nextPozX + tanks.at(0).radius >= buildings.at(i).x - buildings.at(i).length / 2 && nextPozX - tanks.at(0).radius <= buildings.at(i).x + buildings.at(i).length / 2) {
                if (nextPozZ + tanks.at(0).radius  < buildings.at(i).z - buildings.at(i).length / 2 || nextPozZ - tanks.at(0).radius > buildings.at(i).z + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (nextPozZ + tanks.at(0).radius >= buildings.at(i).z - buildings.at(i).length / 2 && nextPozZ - tanks.at(0).radius <= buildings.at(i).z + buildings.at(i).length / 2) {
                if (nextPozX + tanks.at(0).radius < buildings.at(i).x - buildings.at(i).length / 2 || nextPozX - tanks.at(0).radius> buildings.at(i).x + buildings.at(i).length / 2) {
                    ok = 1;
                }
            }
            if (collisionTank != -1 && distanceCalculator(nextPozX, nextPozZ, tanks.at(collisionTank).x, tanks.at(collisionTank).z) > player.radius + tanks.at(collisionTank).radius) {
                tankok = 1;
            }
            if (nextPozX + tanks.at(0).radius > -WORLDBORDER && nextPozX + tanks.at(0).radius < WORLDBORDER && nextPozZ + tanks.at(0).radius <WORLDBORDER && nextPozZ + tanks.at(0).radius > -WORLDBORDER) {
                borderok = 1;
            }
            if (ok == 0 || tankok == 0 || borderok == 0) {
                break;
            }
        }
        if (ok == 1 && tankok == 1 && borderok == 1) {
            tanks.at(0).x -= MOVESPEED * deltaTime * sin(tanks.at(0).orientation);
            tanks.at(0).z -= MOVESPEED * deltaTime * cos(tanks.at(0).orientation);

            camera->MoveForward(-MOVESPEED * deltaTime * (cos(tanks.at(0).orientation)));
            camera->MoveRight(-MOVESPEED * deltaTime * (sin(tanks.at(0).orientation)));
        }
    }
}


void Lab5::OnKeyPress(int key, int mods)
{

}


void Lab5::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    ///move camera ------------------------------------------------------------------------
    if (tanks.at(0).mouseRightClick) {
        renderCameraTarget = true;
        camera->RotateThirdPerson_OX(CAMERASENSITIVITY * -deltaY);
        camera->RotateThirdPerson_OY(CAMERASENSITIVITY * -deltaX);
        
    }
    else {
        if (tanks.at(0).shouldIgnoreClick) {
            tanks.at(0).shouldIgnoreClick = false;
            return;
        } 
        tanks.at(0).turretOrientation -= TURRETSENSITIVITY * deltaX;

        
    }
}


void Lab5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    /// shoot projectiles ------------------------------------------------------------------------
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT))
    {
        tanks.at(0).mouseRightClick = true;
        tanks.at(0).shouldIgnoreClick = true;
    }
    else { // shoot cannonballs
        if (tanks.at(0).cooldown <= 0 && tanks.at(0).dead == false) {
            myCannonball newBall;
            newBall.x = tanks.at(0).x;
            newBall.z = tanks.at(0).z;
            newBall.shotBy = 0;
            newBall.orientation = tanks.at(0).orientation;
            newBall.turretOrientation = tanks.at(0).turretOrientation;
            newBall.realX = newBall.x + sin(newBall.orientation + newBall.turretOrientation) * SCALE * 120;
            newBall.realZ = newBall.z + cos(newBall.orientation + newBall.turretOrientation) * SCALE * 120;
            cannonBalls.push_back(newBall);
            tanks.at(0).cooldown = 1;
        }
        
    }
}


void Lab5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT))
    {
        tanks.at(0).mouseRightClick = false;
    }
}


void Lab5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab5::OnWindowResize(int width, int height)
{
}
