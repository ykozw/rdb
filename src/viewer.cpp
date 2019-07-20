/*
- window_の作成と簡単なUIを追加する
- ラインを描画する
- クライアントが落ちても、接続しなおすようにする
- クライアントが落ちた時にrecvで激しく回らないようにする
- haderonlyにする
- glfwを配置するようにする
  https://github.com/glfw/glfw/releases/download/3.3/glfw-3.3.bin.WIN64.zip
*/
#define WIN32_LEAN_AND_MEAN
#include <chrono>
#include <thread>
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
#define BUFFER_SIZE (64 * 1024)

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
    void update()
    {
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        // Main loop
        while (!glfwWindowShouldClose(window_))
        {
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

            // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!), 
            // you may need to backup/reset/restore current shader using the commented lines below.
            //GLint last_program; 
            //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
            //glUseProgram(0);
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            //glUseProgram(last_program);

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
public:
    void init()
    {
        threawd_ = std::thread([this]() {socketMain(); });
    }
    void socketMain()
    {
        puts("0");
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        {
            exit(1);
        }
        puts("1");
        SOCKET sockd = ::socket(AF_INET, SOCK_STREAM, 0);
        //avoid address in use error that occur if we quit with a client connected
        int t = 1;
        int status = ::setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, (const char*)& t, sizeof(int));
        if (status == -1)
        {
            exit(1);
        }
        puts("2");
        struct sockaddr_in name;
        name.sin_family = AF_INET;
        name.sin_addr.s_addr = INADDR_ANY;
        name.sin_port = ::htons(10000);

        if (sockd == -1)
        {
            exit(1);
        }

        status = bind(sockd, (struct sockaddr*) & name, sizeof(name));
        puts("3");
        if (status == -1)
        {
            exit(1);
        }
        puts("4");
        status = ::listen(sockd, 5);
        if (status == -1)
        {
            exit(1);
        }
        puts("wait for connection");
        struct sockaddr_in peer_name;
        int32_t addrlen = sizeof(peer_name);
        SOCKET sock2 = ::accept(sockd, (struct sockaddr*) & peer_name, &addrlen);
        printf("6(%d)\n", sock2);

        //
        int32_t start = 0;
        int32_t end = 0;
        char data[BUFFER_SIZE];
        while (true)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms);

            //
            if (start != 0)
            {
                size_t size = end - start;
                memmove(data, &data[start], size);
                start = 0;
                end = size;
                puts("7");
            }
            puts("9");
            int r = recv(sock2, &data[end], BUFFER_SIZE - end, 0);
            puts("10");
            if (r > 0)
            {
                end += r;
                puts("8");

                // デバッグ出力
                data[16] = '\0';
                printf("%s", data);
                start = 0;
                end = 0;
            }
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