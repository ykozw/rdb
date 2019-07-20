
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

template<typename Type>
struct RingBuffer
{
public:
    //
    void setRingSize(int32_t ringSize)
    {
        ringSize_ = ringSize;
        buffer_.reserve(ringSize);
        clear();
    }
    //
    void clear()
    {
        buffer_.clear();
        cursor_ = 0;
    }
    //
    int32_t size() const
    {
        return int32_t(buffer_.size());
    }
    int32_t ringSize() const
    {
        return ringSize_;
    }
    //
    template<typename Type>
    void add(const Type& value)
    {
        if ((ringSize_ == -1) ||
            (buffer_.size() < ringSize_))
        {
            buffer_.push_back(value);
        }
        else
        {
            buffer_[cursor_] = value;
            cursor_ = (cursor_ + 1) % ringSize_;
        }
    }
    //
    template<typename Type>
    Type& operator [](int32_t index)
    {
        if (ringSize_ > 0)
        {
            index = (index + cursor_) % ringSize_;
        }
        return buffer_[index];
    }
    const Type& operator [](int32_t index) const
    {
        if (ringSize_ > 0)
        {
            index = (index + cursor_) % ringSize_;
        }
        return buffer_[index];
    }
private:
    int32_t ringSize_ = -1;
    int32_t cursor_ = 0;
    std::vector<Type> buffer_;
    
};

//
struct RdbLabel
{
public:
    const char* label;
    int32_t group;
};
//
struct RdbColor
{
public:
    float r;
    float g;
    float b;
    int32_t group;
};
//
struct RdbPoint
{
public:
    float x;
    float y;
    float z;
    int32_t group;
};
struct RdbLine
{
    float x0;
    float y0;
    float z0;
    float x1;
    float y1;
    float z1;
    int32_t group;
};
struct RdbNormal
{
    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nz;
    int32_t group;
};
struct RdbTriangle
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
};

//
struct RdbTask
{
    RdbTaskType type;
    union
    {
        RdbLabel rdbLabel;
        RdbColor rdbColor;
        RdbPoint rdbPoint;
        RdbLine  rdbLine;
        RdbNormal rdbNormal;
        RdbTriangle rdbTriangle;
    };
};
Concurrency::concurrent_queue<RdbTask> g_rdbTasks;
//

class Window
{
public:
    GLFWwindow* window_;
    int32_t windowWidth_;
    int32_t windowHeight_;
public:
    static void resize_callback(GLFWwindow* window, int width, int height)
    {
        Window* this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
        this_->onWindowResize(width, height);
    }
    void onWindowResize(int32_t width, int32_t height)
    {
        windowWidth_ = width;
        windowHeight_ = height;
    }
    Window()
    {
        // Setup window_
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return;
        const int32_t width = 1280;
        const int32_t height = 720;
        window_ = glfwCreateWindow(width, height, "Ray Debugger", NULL, NULL);
        if (window_ == NULL)
        {
            return;
        }
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable vsync

        //
        onWindowResize(width, height);
        glfwSetWindowUserPointer(window_, this);
        glfwSetWindowSizeCallback(window_, resize_callback);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL2_Init();

        Style();

        //
        points_.setRingSize(128);
    }
    // https://github.com/ocornut/imgui/issues/707#issuecomment-468798935
    inline void Style()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        /// 0 = FLAT APPEARENCE
        /// 1 = MORE "3D" LOOK
        int is3D = 0;

        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
        colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
        colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

        style.PopupRounding = 3;

        style.WindowPadding = ImVec2(4, 4);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(6, 2);

        style.ScrollbarSize = 18;

        style.WindowBorderSize = 1;
        style.ChildBorderSize = 1;
        style.PopupBorderSize = 1;
        style.FrameBorderSize = is3D;

        style.WindowRounding = 3;
        style.ChildRounding = 3;
        style.FrameRounding = 3;
        style.ScrollbarRounding = 2;
        style.GrabRounding = 3;

#ifdef IMGUI_HAS_DOCK 
        style.TabBorderSize = is3D;
        style.TabRounding = 3;

        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
#endif
    }
    //
    void simpleWindow()
    {
        //
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        //window_flags |= ImGuiWindowFlags_NoBackground;

        const int32_t guiWidth = 300;
        ImGui::SetNextWindowPos(ImVec2(windowWidth_-guiWidth, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(
            guiWidth,
            windowHeight_));

        ImGui::Begin("MainWindow", nullptr, window_flags);
        {
            // -------------------------------------------------------
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Stats");
            ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
            // -------------------------------------------------------
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Basic");
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Accept ");
            ImGui::SameLine();
            static bool accept = false;
            ImGui::Checkbox("##ACCEPT_CHECKBOX", &accept);
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                labels_.clear();
                colors_.clear();
                points_.clear();
                lines_.clear();
                normals_.clear();
                triangles_.clear();
            }
            
            static char portNoBuffer[64] = "";
            ImGui::Text("Port   ");
            ImGui::SetNextItemWidth(120);
            ImGui::SameLine();
            ImGui::InputText("##PORT_NO", portNoBuffer, 64, ImGuiInputTextFlags_CharsDecimal);
            // -------------------------------------------------------
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Filter");
            ImGui::Text("Used | Max | Lower | Upper");
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Point    %d  %d", points_.size(), points_.ringSize());
            ImGui::SameLine(); 
            ImGui::SetNextItemWidth(120);
            static int pointRange_[2] = { -1, -1 };
            ImGui::DragInt2("##POINT_RANGE", pointRange_, 1, -1, 255);
            //
            static int lineRange[6] = { -1, -1, -1, -1, -1, -1 };
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Line     %d  %d", lines_.size(), lines_.ringSize());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120);
            ImGui::DragInt2("##LINE_RANGE", lineRange, 1, -1, 64);
            //
            static int triangleRange[2] = { -1, -1 };
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Triangle %d  %d", triangles_.size(), triangles_.ringSize());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120);
            ImGui::DragInt2("##TRIANGLE_RANGE", triangleRange, 1, -1, 255);
            //
            static int normalRange[2] = { -1, -1 };
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Normal   %d  %d", normals_.size(), normals_.ringSize());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120);
            ImGui::DragInt2("##NORMAL_RANGE", normalRange, 1, -1, 255);
        }
        ImGui::End();
    }

    //
    void drawLine()
    {
        //
        // set up view
        glViewport(0, 0, 400, 400);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // see https://www.opengl.org/sdk/docs/man2/xhtml/glOrtho.xml
        glOrtho(0.0, 400.0, 0.0, 400.0, 0.0, 1.0); // this creates a canvas you can do 2D drawing on
        //
        for (int32_t i = 0; i < points_.size(); ++i)
        {
            const RdbPoint& point = points_[i];
            glPointSize(1);
            glBegin(GL_POINTS); // TODO: まとめて実行する
            glColor4f(1, 1, 1, 1);
            glVertex3f(point.x, point.y, point.z);
            glEnd();
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
        ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
        // TODO: リングバッファーにする
        
        // Main loop
        while (!glfwWindowShouldClose(window_))
        {

            // 全ての積まれているタスクを描画に変換する
            while (!g_rdbTasks.empty())
            {
                RdbTask task;
                if (g_rdbTasks.try_pop(task))
                {
                    switch (task.type)
                    {
                    case RdbTaskType::LABEL:
                        labels_.add(task.rdbLabel);
                        break;
                    case RdbTaskType::COLOR:
                        colors_.add(task.rdbColor);
                        break;
                    case RdbTaskType::POINT:
                        points_.add(task.rdbPoint);
                        break;
                    case RdbTaskType::LINE:
                        lines_.add(task.rdbLine);
                        break;
                    case RdbTaskType::NORMAL:
                        normals_.add(task.rdbNormal);
                        break;
                    case RdbTaskType::TRIANGLE:
                        triangles_.add(task.rdbTriangle);
                        break;
                    }                    
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

            // GUI
            simpleWindow();
            ImGui::ShowDemoWindow(&show_demo_window);

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
            //
            drawLine();

            glfwMakeContextCurrent(window_);
            glfwSwapBuffers(window_);
        }
    }
private:
    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }
private:
    RingBuffer<RdbLabel> labels_;
    RingBuffer<RdbColor> colors_;
    RingBuffer<RdbPoint> points_;
    RingBuffer<RdbLine> lines_;
    RingBuffer<RdbNormal> normals_;
    RingBuffer<RdbTriangle> triangles_;
    //
    
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
                char data[BUFFER_SIZE-1];
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