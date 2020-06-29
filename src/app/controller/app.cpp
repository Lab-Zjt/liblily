#include "../proto/proto.h"
#include "controller_view.h"
#include <QtWidgets/QApplication>

using namespace lily;
using namespace reflect;

int main(int argc, char *argv[]) {
  auto[server, err] = TCPServer::ListenTCP("/tmp/http_server_controller");
  if (err != NoError) {
    std::cerr << "Create Controller Failed. " << err.desc << "\n";
    return -1;
  }
  QApplication app(argc, argv);
  auto view = New<ControllerView>();
  view->show();
  try {
    return app.exec();
  } catch (...) {
    view->onStopButtonClick(false);
    return -1;
  }
}
