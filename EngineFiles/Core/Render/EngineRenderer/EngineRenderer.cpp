#include "EngineRenderer.hpp"
#include "Engine.hpp"
#include "Core/WindowManager/WindowManager.hpp"
#include "Core/WindowManager/Window/Window.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <glm/glm.hpp>

std::unique_ptr<ShaderManager> EngineRenderer::shaderManager = nullptr;

EngineRenderer::EngineRenderer(SDL_GPUDevice* device) {
	this->device = device;
}

bool EngineRenderer::Initialize() {
    ShaderManager* rawSM = new ShaderManager(device);
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manager");
        return false;
    }

    shaderManager.reset(rawSM);

    Window* window = Engine::GetEngine().GetWindowManager().GetMainWindow();
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = device;
    init_info.ColorTargetFormat = window->GetGPUSwapchainTextureFormat();
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;

    if (!ImGui_ImplSDLGPU3_Init(&init_info)) {
        SDL_Log("Failed to initialize ImGui SDLGPU3 backend");
        return false;
    }

    if (!window->InitImGUI()) {
        SDL_Log("Failed to initialize ImGui SDL3 backend");
        return false;
    }

    std::string vertShader = "shaders/TexturedQuadWithMatrix.vert.hlsl";
    std::string fragShader = "shaders/TexturedQuad.frag.hlsl";

    ShaderOptions optionsFrag = {
        .num_samplers = 1,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0
    };

    ShaderOptions optionsVert = {
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 1
    };

    SDL_GPUShader* vert = shaderManager.get()->LoadShader(vertShader, &optionsVert);
    SDL_GPUShader* frag = shaderManager.get()->LoadShader(fragShader, &optionsFrag);

    if (!InitializePipelines(window, vert, frag)) {
        SDL_Log("Failed to create Pipelines");
        return false;
    }

    if (msaaEnabled) {
        if (!CreateMSAATexture(window)) {
            SDL_Log("Failed to create MSAA Texture");
            return false;
        }
    }

    if (!CreateDepthStencil(window)) {
        SDL_Log("Failed to create Depth Stencils");
        return false;
    }

    if (!InitializeSamplers()) {
        SDL_Log("Failed to create Samplers");
        return false;
    }

    return true;
}

void EngineRenderer::Render() {
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    if (!command_buffer) {
        SDL_Log("Failed to acquire command buffer");
        return;
    }

    SDL_GPUTexture* swapChainTexture = nullptr;
    Window* window = Engine::GetEngine().GetWindowManager().GetMainWindow();
    window->WaitAndAquireGPUGwapchainTexture(command_buffer, &swapChainTexture, nullptr, nullptr);

    if (swapChainTexture && !is_minimized) {
        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

        SDL_GPUColorTargetInfo colorTarget = {};
        colorTarget.clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.mip_level = 0;
        colorTarget.layer_or_depth_plane = 0;
        colorTarget.cycle = false;

        if (msaaEnabled) {
            colorTarget.texture = msaaTexture;
            colorTarget.store_op = SDL_GPU_STOREOP_RESOLVE;
            colorTarget.resolve_texture = swapChainTexture;
            colorTarget.resolve_mip_level = 0;
            colorTarget.resolve_layer = 0;
        }
        else {
            colorTarget.texture = swapChainTexture;
            colorTarget.store_op = SDL_GPU_STOREOP_STORE;
        }

        SDL_GPUDepthStencilTargetInfo depthTarget = {};
        depthTarget.texture = depthStencilTexture;
        depthTarget.clear_depth = 1.0f;
        depthTarget.clear_stencil = 0;
        depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;
        depthTarget.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
        depthTarget.cycle = false;

        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &colorTarget, 1, &depthTarget);

        SDL_EndGPURenderPass(render_pass);

        if (msaaEnabled) {
            SDL_GPUColorTargetInfo imguiColorTarget = {};
            imguiColorTarget.texture = swapChainTexture;
            imguiColorTarget.clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };
            imguiColorTarget.load_op = SDL_GPU_LOADOP_LOAD;
            imguiColorTarget.store_op = SDL_GPU_STOREOP_STORE;
            imguiColorTarget.cycle = false;

            SDL_GPURenderPass* imgui_pass = SDL_BeginGPURenderPass(command_buffer, &imguiColorTarget, 1, nullptr);
            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, imgui_pass);
            SDL_EndGPURenderPass(imgui_pass);
        }
        else {
            SDL_GPUColorTargetInfo imguiColorTarget = {};
            imguiColorTarget.texture = swapChainTexture;
            imguiColorTarget.load_op = SDL_GPU_LOADOP_LOAD;
            imguiColorTarget.store_op = SDL_GPU_STOREOP_STORE;
            imguiColorTarget.cycle = false;

            SDL_GPURenderPass* imgui_pass = SDL_BeginGPURenderPass(command_buffer, &imguiColorTarget, 1, nullptr);
            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, imgui_pass);
            SDL_EndGPURenderPass(imgui_pass);
        }
    }

    SDL_SubmitGPUCommandBuffer(command_buffer);
}

bool EngineRenderer::InitializePipelines(Window* window, SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader) {

    SDL_GPUColorTargetDescription colorTargetDescription{};
    colorTargetDescription.format = window->GetGPUSwapchainTextureFormat();
    colorTargetDescription.blend_state.enable_blend = true;
    colorTargetDescription.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    colorTargetDescription.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorTargetDescription.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    colorTargetDescription.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    colorTargetDescription.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorTargetDescription.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

    std::vector colorTargetDescriptions{ colorTargetDescription };

    SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
    targetInfo.color_target_descriptions = colorTargetDescriptions.data();
    targetInfo.num_color_targets = colorTargetDescriptions.size();
    targetInfo.has_depth_stencil_target = true;
    targetInfo.depth_stencil_format = GetDepthStencilFormat();

    SDL_GPUDepthStencilState depthStencilState{};
    depthStencilState.enable_depth_test = true;
    depthStencilState.enable_depth_write = true;
    depthStencilState.compare_op = SDL_GPU_COMPAREOP_LESS;

    std::vector<SDL_GPUVertexAttribute> vertexAttributes{};
    std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions{};

    vertexAttributes.emplace_back(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Vertex, position));
    vertexAttributes.emplace_back(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Vertex, uv));

    vertexBufferDescriptions.emplace_back(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0);

    SDL_GPUVertexInputState vertexInputState{};
    vertexInputState.vertex_attributes = vertexAttributes.data();
    vertexInputState.num_vertex_attributes = vertexAttributes.size();
    vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions.data();
    vertexInputState.num_vertex_buffers = vertexBufferDescriptions.size();

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.vertex_shader = vertexShader;
    pipelineCreateInfo.fragment_shader = fragmentShader;
    pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipelineCreateInfo.target_info = targetInfo;
    pipelineCreateInfo.vertex_input_state = vertexInputState;
    pipelineCreateInfo.depth_stencil_state = depthStencilState;

    if (msaaEnabled) {
        SDL_GPUMultisampleState multisampleState{};
        multisampleState.sample_count = sampleCount;
        pipelineCreateInfo.multisample_state = multisampleState;
    }

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);

    if (!pipeline) {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_Log("Pipeline created successfully!");
    pipelines[PIPELINE_TYPE::FILL_PIPELINE] = pipeline;

    return true;
}

SDL_GPUTextureFormat EngineRenderer::GetDepthStencilFormat() {
    if (!device) {
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    if (SDL_GPUTextureSupportsFormat(device,
        SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    }
    else if (SDL_GPUTextureSupportsFormat(device,
        SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        return SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
    }
    else {
        SDL_Log("Stencil formats not supported!");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }
}

bool EngineRenderer::InitializeSamplers() {
    SDL_GPUSamplerCreateInfo samplerCreateInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
    };

    defaultSampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);

    if (!defaultSampler) {
        SDL_Log("Failed to create default sampler, %s", SDL_GetError());
        return false;
    }
    return true;
}

bool EngineRenderer::CreateDepthStencil(Window* window) {
    int windowWidth, windowHeight;
    window->GetWindowSize(&windowWidth, &windowHeight);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, 1.0f);
    SDL_SetNumberProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, 0);

    SDL_GPUTextureCreateInfo depthStencilTextureCreateInfo = {};

    depthStencilTextureCreateInfo.format = GetDepthStencilFormat();
    depthStencilTextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    depthStencilTextureCreateInfo.width = static_cast<uint32_t>(windowWidth);
    depthStencilTextureCreateInfo.height = static_cast<uint32_t>(windowHeight);
    depthStencilTextureCreateInfo.layer_count_or_depth = 1;
    depthStencilTextureCreateInfo.num_levels = 1;
    depthStencilTextureCreateInfo.props = props;
    if (msaaEnabled) {
        depthStencilTextureCreateInfo.sample_count = sampleCount;
    }

    depthStencilTexture = SDL_CreateGPUTexture(device, &depthStencilTextureCreateInfo);
    SDL_DestroyProperties(props);

    if (!depthStencilTexture) {
        SDL_Log("Failed to create default depth stencil texture, %s", SDL_GetError());
        return false;
    }

    if (msaaEnabled) {
        SDL_SetGPUTextureName(device, depthStencilTexture, "MSAA Depth Stencil Texture");
    }
    else {
        SDL_SetGPUTextureName(device, depthStencilTexture, "Depth Stencil Texture");
    }
    return true;
}

bool EngineRenderer::CreateMSAATexture(Window* window) {
    int windowWidth, windowHeight;
    window->GetWindowSize(&windowWidth, &windowHeight);

    SDL_PropertiesID props = SDL_CreateProperties();

    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, 0.1f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, 0.1f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, 0.1f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, 1.0f);

    SDL_GPUTextureCreateInfo msaaTextureCreateInfo{
        .format = window->GetGPUSwapchainTextureFormat(),
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
        .width = static_cast<uint32_t>(windowWidth),
        .height = static_cast<uint32_t>(windowHeight),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = sampleCount,
        .props = props,
    };

    msaaTexture = SDL_CreateGPUTexture(device, &msaaTextureCreateInfo);
    SDL_DestroyProperties(props);

    if (!msaaTexture) {
        SDL_Log("Failed to create default MSAA texture, %s", SDL_GetError());
        return false;
    }

    SDL_SetGPUTextureName(device, msaaTexture, "MSAA Texture");
    return true;
}

void EngineRenderer::Shutdown() {
    SDL_WaitForGPUIdle(device);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
}