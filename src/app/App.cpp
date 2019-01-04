//
// Created by kurono267 on 2018-12-07.
//

#include "App.hpp"
#include "scene/lights/PointLight.hpp"
#include "scene/lights/EnvLight.hpp"

bool App::init() {
    auto mainWnd = mainApp.lock();

    _instance = std::make_unique<vulkan::InstanceVK>();
    _instance->init("Raytracer", mainWnd->window(), mainWnd->wndSize());

    auto device = _instance->device();
    std::cout << device->device_name() << std::endl;

    RenderPattern rp;
    rp.viewport(Viewport(glm::vec2(0), mainWnd->wndSize()));
    rp.scissor(glm::ivec2(0), mainWnd->wndSize());

    _camera = std::make_shared<CameraAtPoint>(device,glm::vec3(11, 4, -6));
    _camera->initProj(glm::radians(45.0f),(float)(1280)/(float)(720),0.1f,1000.0f);

    _scene = std::make_shared<Scene>();
    _scene->loadGLTF("models/room/scene.gltf");
    //_scene->load("models/lambo/lambo");
    for(auto model : _scene->models()){
        _sceneGPU.push_back(createMesh(device,model->mesh()));
    }
    _scene->add(std::make_shared<PointLight>(glm::translate(glm::vec3(0.f,10.0f,0.0f)),glm::vec3(100.0f)));

    // Load environment map
    spImage4f env = loadImageHDRI("envs/spruit_sunrise_2k.hdr",true);
    _scene->add(std::make_shared<EnvLight>(env,glm::vec3(10.0f)));

    _pt = std::make_shared<PathTracer>(device,_scene);
    _pt->init();

    _texture = checkboardTexture(device, 1280, 720, 100);
    auto texView = _pt->getTexture()->createTextureView();

    _descSet = device->createDescSet();
    _descSet->setUniformBuffer(_camera->getCameraUniform(), 0, ShaderStage::Vertex);
    _descSet->setTexture(texView, Sampler(), 1, ShaderStage::Fragment);
    _descSet->create();

    _main = device->createPipeline(rp);
    _main->addShader(ShaderStage::Vertex, "../glsl/quad.vert");
    _main->addShader(ShaderStage::Fragment, "../glsl/quad.frag");
    _main->setDescSet(_descSet);

    _cube = createCube(device);
    _quad = createQuad(device);

    spRenderPass renderPass = device->getScreenRenderPass();

    _main->setRenderPass(renderPass);
    _main->create();

    auto screenBuffers = device->getScreenbuffers();
    for (const auto &screen : screenBuffers) {
        std::cout << screen->info() << std::endl;
    }
    _cmdScreen.resize(screenBuffers.size());
    for (int i = 0; i < _cmdScreen.size(); ++i) {
        _cmdScreen[i] = device->createCommandBuffer();
        _cmdScreen[i]->begin();

        _cmdScreen[i]->setClearColor(0, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        _cmdScreen[i]->setClearDepthStencil(1, 1.0f, 0.0f);

        _cmdScreen[i]->beginRenderPass(renderPass, screenBuffers[i],
                                       RenderArea(screenBuffers[i]->getSize(), glm::ivec2(0)));
        _cmdScreen[i]->bindPipeline(_main);
        _cmdScreen[i]->bindDescriptorSet(_main, _descSet);
        //_cube->draw(_cmdScreen[i]);
        /*for(int m = 0;m<_sceneGPU.size();++m){
            _sceneGPU[m]->draw(_cmdScreen[i]);
        }*/
        _quad->draw(_cmdScreen[i]);
        _cmdScreen[i]->endRenderPass();

        _cmdScreen[i]->end();
    }

    _screenAvailable = device->createSemaphore();
    _renderFinish = device->createSemaphore();

    _pt->nextFrame(_camera);
    _pt->run();

    return true;
}

bool App::draw() {
    auto currFrameTime = std::chrono::steady_clock::now();
    _dt = std::chrono::duration_cast<std::chrono::duration<float> >(currFrameTime-_prevFrameTime).count();

    auto device = _instance->device();
    auto imageIndex = device->nextScreen(_screenAvailable);

    device->submit(_cmdScreen[imageIndex],_screenAvailable,_renderFinish);
    device->present(imageIndex,_renderFinish);

    _prevFrameTime = currFrameTime;

    return true;
}

bool App::update() {
    std::cout << "CameraPos: " << glm::to_string(_camera->getPos()) << std::endl;
    _camera->updateUniform();
    if(_pt->isUpdateFinish()){
        _pt->sync();
        _pt->nextFrame(_camera);
    }
    return true;
}

bool App::onKey(const GLFWKey& key) {
    return true;
}
bool App::onMouse(const GLFWMouse& mouse) {
    if(mouse.callState == GLFWMouse::onMouseButton){
        if(mouse.button == GLFW_MOUSE_BUTTON_1){
            if(mouse.action == GLFW_PRESS){
                _isPressed = true;
                _isFirst = true;
            } else _isPressed = false;
        }
    } else if (mouse.callState == GLFWMouse::onMousePosition){
        if(_isPressed){
            if(!_isFirst){
                glm::vec2 dp = glm::vec2(mouse.x,mouse.y)-_prev_mouse;
                _camera->rotate(dp,_dt);
            }
            _prev_mouse = glm::vec2(mouse.x,mouse.y);
            _isFirst = false;
        }
    }
    return true;
}
bool App::onExit() {
    return true;
}

bool App::onScroll(const glm::vec2 &offset) {
    _camera->scale(offset.y,_dt);
    return true;
}

