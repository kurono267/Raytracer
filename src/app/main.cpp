//
// Created by kurono267 on 2018-12-07.
//

#include "App.hpp"

int main(){
    spMainApp main = MainApp::instance();
    spBaseApp app = std::make_shared<App>(main);

    main->create("Raytracer",1280,720);
    main->setBaseApp(app);

    main->run();

    return 0;
}