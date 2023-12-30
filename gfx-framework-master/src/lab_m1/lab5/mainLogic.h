#pragma once

#include "components/simple_scene.h"
#include "lab_m1/lab5/camera.h"
#include "components/text_renderer.h"
namespace m1
{
    class Lab5 : public gfxc::SimpleScene
    {
     public:
        Lab5();
        ~Lab5();

        void Init() override;
        class myTank {
        public:
            float orientation = M_PI;
            float x = 0;
            float z = 0;
            float turretOrientation = 0;
            bool mouseRightClick = true;
            bool shouldIgnoreClick = false;
            float cooldown = 2;
            float radius;
            int hp = 3;
            bool dead = false;
            std::string movement;
            float movementTimer = 0;
        };
        class myGame {
        public:
            int numberOfBuildings;
            int numberOfTanks;
            int score = 0;
            float timer = 100;
            int endScreen = 1;
            bool displayScore = false;
            bool displayDeath = false;
        };
        

        class myCannonball {
        public:
            float x; 
            float z;
            int shotBy;
            float orientation = M_PI;
            float turretOrientation = 0;
            float dissappearTimer = 3;
            float realX;
            float realZ;
        };

        class myBuilding {
        public:
            float x;
            float z;
            float length = 5.43;
            float size = 1;
        };


     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int health);
        Mesh* CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix) override;
        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
        bool collisionCheck(int tankNumber, float deltaTimeSeconds);

     protected:
        implemented::Camera *camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;

        bool projectionType;
        GLfloat right;
        GLfloat left;
        GLfloat bottom;
        GLfloat top;
        GLfloat fov = 90;
        
        std::vector <myTank> tanks;
        std::vector <myCannonball> cannonBalls;
        std::vector <myBuilding> buildings;
        myGame game;
        gfxc::TextRenderer *textRenderer;
        // TODO(student): If you need any other class variables, define them here.

    };
}   // namespace m1
