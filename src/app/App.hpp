//
// Created by kurono267 on 2018-12-07.
//

#ifndef MANGO_EXAMPLE_APP_HPP
#define MANGO_EXAMPLE_APP_HPP

#define MANGO_VULKAN
#include <mango.hpp>
#include "scene/Camera.hpp"
#include "../scene/Scene.hpp"
#include "../cpu/PathTracer.hpp"
#include "../cpu/BDPT.hpp"

using namespace mango;
using namespace mango::scene;

class App : public BaseApp {
public:
    explicit App(const spMainApp& app) : BaseApp(app) {}
    ~App() final = default;

    bool init() final;
    bool draw() final;
    bool update() final;

    bool onKey(const GLFWKey& key) final;
    bool onMouse(const GLFWMouse& mouse) final;
    bool onExit() final;
    bool onScroll(const glm::vec2& offset) final;
protected:
    std::unique_ptr<Instance> _instance;

    spPipeline _main;
    std::vector<spCommandBuffer> _cmdScreen;

    spMesh _cube;
    spMesh _quad;

    spIntegrator _integrator;

    spTexture _texture;
    spScene _scene;
    std::vector<spMesh> _sceneGPU;

    std::shared_ptr<Camera> _camera;

    // For camera rotate
    bool _isPressed = false;
    bool _isFirst = false;
    glm::vec2 _prev_mouse;
    float _dt;
    std::chrono::steady_clock::time_point _prevFrameTime;

    spDescSet _descSet;

    spSemaphore _screenAvailable;
    spSemaphore _renderFinish;
};


#endif //MANGO_APP_HPP
