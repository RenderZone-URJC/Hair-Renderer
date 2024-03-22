#include "hair_renderer.h"

void HairRenderer::init()
{

    Renderer::init();

    chdir("/home/tony/Dev/OpenGL-Hair/");

    m_camera = new Camera(m_window.extent.width, m_window.extent.height, {0.0f, 0.0f, -10.0f});

    m_controller = new Controller(m_camera);

    m_light = new PointLight();
    m_light->set_position({5.0f, 3.0f, -7.0f});

    m_hair = new Mesh();
    m_head = new Mesh();

    const size_t CAMERA_MAT_NUM = 3;
    const size_t NUM_OBJS = 2;
    m_cameraBuffer = new UniformBuffer(sizeof(glm::mat4) * CAMERA_MAT_NUM);
    m_cameraBuffer->generate();

    GraphicPipeline hpipeline{};
    hpipeline.shader = new Shader("resources/shaders/cook-torrance.glsl", ShaderType::LIT);
    hpipeline.shader->set_uniform_block("Camera", CAMERA_LAYOUT);
    Material *headMaterial = new Material(hpipeline);
    m_head->set_material(headMaterial);

    GraphicPipeline spipeline{};
    spipeline.shader = new Shader("resources/shaders/strand-kajiya.glsl", ShaderType::LIT);
    spipeline.shader->set_uniform_block("Camera", CAMERA_LAYOUT);
    Material *hairMaterial = new Material(spipeline);
    m_hair->set_material(hairMaterial);

#define YUKSEL
#ifdef YUKSEL
    // CEM YUKSEL MODELS
    {
        std::thread loadThread1(loaders::load_PLY, m_head, "resources/models/woman.ply", true, true, false);
        loadThread1.detach();
        m_head->set_rotation({180.0f, -90.0f, 0.0f});
        std::thread loadThread2(hair_loaders::load_cy_hair, m_hair, "resources/models/natural.hair");
        loadThread2.detach();
        m_hair->set_scale(0.054f);
        m_hair->set_position({0.015f, -0.09f, 0.2f});
        m_hair->set_rotation({-90.0f, 0.0f, 16.7f});
    }
#else
    // NEURAL HAIRCUT MODELS
    {
        loaders::load_PLY(m_head, "resources/models/head_blender.ply", true, true, false);
        std::thread loadThread1(hair_loaders::load_neural_hair, m_hair, "resources/models/2000000.ply", m_head, true, true, false);
        loadThread1.detach();
    }
    #endif
}

void HairRenderer::update()
{
    if (!user_interface_wants_to_handle_input())
        m_controller->handle_keyboard(m_window.ptr, 0, 0, m_time.delta);
}

void HairRenderer::draw()
{
    // ---- Update global uniform buffers ----
    struct CameraUniforms
    {
        glm::mat4 vp;
        glm::mat4 mv;
        glm::mat4 v;
    };
    CameraUniforms camu;
    camu.vp = m_camera->get_projection() * m_camera->get_view();
    camu.mv = m_camera->get_view() * m_head->get_model_matrix();
    camu.v = m_camera->get_view();
    m_cameraBuffer->cache_data(sizeof(CameraUniforms), &camu);

    // ----- Draw ----
    clearColorDepthBit();

    MaterialUniforms headu;
    headu.mat4Types["u_model"] = m_head->get_model_matrix();
    headu.vec3Types["u_skinColor"] = m_headSettings.skinColor;
    m_light->cache_uniforms(headu);
    m_head->get_material()->set_uniforms(headu);

    m_head->draw();

    MaterialUniforms hairu;
    hairu.mat4Types["u_model"] = m_hair->get_model_matrix();
    hairu.vec3Types["u_albedo"] = m_hairSettings.color;
    hairu.vec3Types["u_spec1"] = m_hairSettings.specColor1;
    hairu.floatTypes["u_specPwr1"] = m_hairSettings.specPower1;
    hairu.vec3Types["u_spec2"] = m_hairSettings.specColor2;
    hairu.floatTypes["u_specPwr2"] = m_hairSettings.specPower2;
    hairu.floatTypes["u_thickness"] = m_hairSettings.thickness;
    m_light->cache_uniforms(hairu);
    m_hair->get_material()->set_uniforms(hairu);

    m_hair->draw(GL_LINES);
}

void HairRenderer::setup_user_interface_frame()
{
    Renderer::setup_user_interface_frame();

    ImGui::NewFrame();

    // ImGui::ShowDemoWindow(&m_globalSettings.showUI);

    ImGui::Begin("Settings", &m_globalSettings.showUI); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::SeparatorText("Profiler");
    ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::SeparatorText("Global Settings");
    if (ImGui::Checkbox("V-Sync", &m_settings.vSync))
    {
        set_v_sync(m_settings.vSync);
    }
    ImGui::Separator();
    ImGui::SeparatorText("Hair Settings");
    gui::draw_transform_widget(m_hair);
    ImGui::DragFloat("Strand thickness", &m_hairSettings.thickness, 0.001f, 0.001f, 0.05f);
    ImGui::ColorEdit3("Strand color", (float *)&m_hairSettings.color);
    ImGui::Separator();
    ImGui::SeparatorText("Head Settings");
    gui::draw_transform_widget(m_head);
    ImGui::ColorEdit3("Skin color", (float *)&m_headSettings.skinColor);
    ImGui::Separator();
    ImGui::SeparatorText("Lighting Settings");
    gui::draw_transform_widget(m_light);
    ImGui::Separator();

    ImGui::End();

    ImGui::Render();
}

void HairRenderer::setup_window_callbacks()
{
    glfwSetWindowUserPointer(m_window.ptr, this);

    glfwSetKeyCallback(m_window.ptr, [](GLFWwindow *w, int key, int scancode, int action, int mods)
                       { static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->key_callback(w, key, scancode, action, mods); });
    glfwSetCursorPosCallback(m_window.ptr, [](GLFWwindow *w, double x, double y)
                             { static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->mouse_callback(w, x, y); });
    glfwSetFramebufferSizeCallback(m_window.ptr, [](GLFWwindow *w, int width, int height)
                                   { static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->resize_callback(w, width, height); });
}
