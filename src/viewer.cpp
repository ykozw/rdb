﻿
#define WIN32_LEAN_AND_MEAN
//
#include <glfw/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
//
#include <Windows.h>
#include <WinSock2.h>
#include <concurrent_queue.h>
#pragma comment(lib, "Ws2_32.lib")
//
#include <chrono>
#include <thread>
#include <vector>
#include <array>
//
#define BUFFER_SIZE (64 * 1024)

enum class RdbTaskType : int32_t
{
    LABEL,
    COLOR,
    POINT,
    LINE,
    NORMAL,
    TRIANGLE,
};
//
struct RdbTask
{
    RdbTaskType type;
    union
    {
        struct
        {
            const char* label;
            int32_t group;
        }rdbLabel;
        struct
        {
            float r;
            float g;
            float b;
            int32_t group;
        }rdbColor;
        struct
        {
            float x;
            float y;
            float z;
            int32_t group;
        }rdbPoint;
        struct
        {
            float x0;
            float y0;
            float z0;
            float x1;
            float y1;
            float z1;
            int32_t group;
        }rdbLine;
        struct
        {
            float x;
            float y;
            float z;
            float nx;
            float ny;
            float nz;
            int32_t group;
        }rdbNormal;
        struct
        {
            float x0;
            float y0;
            float z0;
            float x1;
            float y1;
            float z1;
            float x2;
            float y2;
            float z2;
            int32_t group;
        }rdbTriangle;
    };
};
Concurrency::concurrent_queue<RdbTask> g_rdbTasks;
//

class Window
{
public:
    GLFWwindow* window_;
public:
    Window()
    {
        // Setup window_
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return;
        window_ = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);
        if (window_ == NULL)
            return;
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL2_Init();
    }
    //
    void drawLine(const std::vector<RdbTask>& tasks)
    {
        //
        // set up view
        glViewport(0, 0, 400, 400);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // see https://www.opengl.org/sdk/docs/man2/xhtml/glOrtho.xml
        glOrtho(0.0, 400.0, 0.0, 400.0, 0.0, 1.0); // this creates a canvas you can do 2D drawing on
        //
        for (const RdbTask& task : tasks)
        {
            switch (task.type)
            {
            case RdbTaskType::LABEL:
                break;
            case RdbTaskType::POINT:
            {
                auto& t = task.rdbPoint;
                glPointSize(1);
                glBegin(GL_POINTS); // TODO: まとめて実行する
                glColor4f(1, 1, 1, 1);
                glVertex3f(t.x, t.y, t.z);
                glEnd();
            }
            break;
            }
        }


        glPointSize(10);
        glLineWidth(2.5);
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_LINES);
        glVertex3f(10.0, 10.0, 0.0);
        glVertex3f(200.0, 200.0, 0.0);
        glEnd();
    }
    //
    void update()
    {
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        // TODO: リングバッファーにする
        std::vector<RdbTask> tasks;
        // Main loop
        while (!glfwWindowShouldClose(window_))
        {

            // 全ての積まれているタスクを描画に変換する
            while (!g_rdbTasks.empty())
            {
                RdbTask task;
                if (g_rdbTasks.try_pop(task))
                {
                    tasks.push_back(task);
                }
            }


            // Poll and handle events (inputs, window_ resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. Show the big demo window_ (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window_ that we create ourselves. We use a Begin/End pair to created a named window_.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window_ called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window_ open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("clear color", (float*)& clear_color); // Edit 3 floats representing a color

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            // 3. Show another simple window_.
            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window_ will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window_!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window_, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            //
            //glfwMakeContextCurrent(window_);
            //drawLine();

            // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!), 
            // you may need to backup/reset/restore current shader using the commented lines below.
            //GLint last_program; 
            //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
            //glUseProgram(0);
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            //glUseProgram(last_program);
            //
            drawLine(tasks);

            glfwMakeContextCurrent(window_);
            glfwSwapBuffers(window_);
        }
    }
private:
    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }
};



class Socket
{
    std::thread threawd_;
    char datum_[BUFFER_SIZE] = {'\0'};
public:
    void init()
    {
        threawd_ = std::thread([this]() {socketMain(); });
    }
    void socketMain()
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        {
            exit(1);
        }
        SOCKET sockd = ::socket(AF_INET, SOCK_STREAM, 0);
        //avoid address in use error that occur if we quit with a client connected
        int t = 1;
        int status = ::setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, (const char*)& t, sizeof(int));
        if (status == -1)
        {
            exit(1);
        }
        struct sockaddr_in name;
        name.sin_family = AF_INET;
        name.sin_addr.s_addr = INADDR_ANY;
        name.sin_port = ::htons(10000);

        if (sockd == -1)
        {
            exit(1);
        }

        status = bind(sockd, (struct sockaddr*) & name, sizeof(name));
        if (status == -1)
        {
            exit(1);
        }
        status = ::listen(sockd, 5);
        if (status == -1)
        {
            exit(1);
        }

        while (true)
        {
            puts("wait for connection");
            struct sockaddr_in peer_name;
            int32_t addrlen = sizeof(peer_name);
            SOCKET sock2 = ::accept(sockd, (struct sockaddr*) & peer_name, &addrlen);
            puts("connected");
            //
            size_t dataLen = 0;
            while (true)
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(10ms);
                char data[BUFFER_SIZE];
                int r = recv(sock2, data, BUFFER_SIZE, 0);
                if (r < 0)
                {
                    puts("disconnect");
                    break;
                }
                
                if (r > 0)
                {
                    //
                    dataLen += r;
                    data[dataLen] = '\0';
                    strcat(datum_, data);

                    // "\n"まで言っているか確認
                    if (datum_[dataLen - 1] == '\n')
                    {
                        puts("------------------");
                        const char* token = strtok(datum_, "\n");
                        processToken(token);
                        while (const char* token = strtok(nullptr, "\n"))
                        {
                            processToken(token);
                        }
                        dataLen = 0;
                        datum_[0] = '\0';
                    }
                }
            }
        }
    }
    void processToken(const char* token)
    {
        int32_t group;
        RdbTask task;
        
        switch (token[0])
        {
        case 'E':
        {
            char label[0xff];
            if (sscanf(token, "E %s,%d\n", label, &group) == 2)
            {
                printf("LABEL %s\n", label);
            }
        }
        break;
        case 'C':
        {
            float r, g, b;
            if(sscanf(token, "C %f,%f,%f,%d\n", &r, &g, &b, &group) == 4)
            {
                printf("COLOR %f,%f,%f\n", r, g, b);
            }
        }
        break;
        case 'P':
        {
            //float x, y, z;
            task.type = RdbTaskType::POINT;
            auto& t = task.rdbPoint;
            if (sscanf(token, "P %f,%f,%f,%d\n", &t.x, &t.y, &t.z, &t.group) == 4)
            {
                g_rdbTasks.push(task);
                //printf("POINT %f,%f,%f\n", x, y, z);
            }
        }
        break;
        case 'L':
        {
            float x0, y0, z0, x1, y1, z1;
            if (sscanf(token, "L %f,%f,%f,%f,%f,%f,%d\n",
                &x0, &y0, &z0,
                &x1, &y1, &z1, &group) == 7)
            {
                printf("LINE (%f,%f,%f) (%f,%f,%f)\n", x0, y0, z0, x1, y1, z1 );
            }

        }
        break;
        case 'N':
        {
            float x, y, z, nx, ny, nz;
            if( sscanf(token, "N %f,%f,%f,%f,%f,%f,%d\n",
                &x, &y, &z, &nx, &ny, &nz, &group) == 7)
            {
                printf("NORM (%f,%f,%f) (%f,%f,%f)\n", x, y, z, nx, ny, nz);
            }
        }
        break;
        case 'T':
        {
            float x0, y0, z0, x1, y1, z1, x2, y2, z2;
            if (sscanf(token, "T %f,%f,%f,%f,%f,%f,%f,%f,%f,%d\n",
                &x0, &y0, &z0,
                &x1, &y1, &z1,
                &x2, &y2, &z2,
                &group) == 10)
            {
                printf("TRI (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)\n",
                    x0, y0, z0, x1, y1, z1, x2, y2, z2);
            }
        }
        break;
        }
    }
};

//
int main()
{
    Socket socket;
    socket.init();
    Window window;
    window.update();
}