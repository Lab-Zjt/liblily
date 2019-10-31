#include <QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QLineEdit>
#include <queue>
#include <mutex>
#include <QtCore/QTimer>
#include "proto/proto.h"

using namespace lily;
using namespace reflect;

class Controller {
 private:
  const char *m_path;
  TCPServer m_server;
  Ref<Client> m_conn;
  proto::ProtoHandler m_handler;
 public:
  Controller(const char *path) : m_path(path), m_server(m_path) {}
  Error StartNewServer(const std::string &ip, uint16_t port) {
    if (system("./http_server &") < 0) {
      perror("http_server");
      return Error{"create sub process failed"};
    }
    std::thread([this, ip = ip, port]() -> Error {
      auto[conn, err] = m_server.Accept();
      if (err != NoError && conn == nullptr) {
        return err;
      }
      m_conn = conn;
      proto::StartServerNotify notify;
      notify.addr = ip;
      notify.port = port;
      err = proto::SendNotify(conn, notify);
      if (err != NoError) {
        return err;
      }
      while (true) {
        auto err = m_handler.Handle(conn);
        if (err != NoError) {
          return NoError;
        }
      }
    }).detach();
    return NoError;
  }
  Error StopServer() {
    proto::StopServerNotify notify;
    notify.reason = "Stop by command";
    if (auto err = proto::SendNotify(m_conn, notify); err != NoError) {
      return err;
    } else {
      m_conn = nullptr;
      return NoError;
    }
  }
  template<typename Notify, typename Fn>
  void RegisterHandler(int cmd_id, Fn &&fn) {
    m_handler.RegisterHandler<Notify>(cmd_id, fn);
  }
};

class ControllerView : public QWidget {
 private:
  QLineEdit m_addr;
  QLineEdit m_port;
  QPushButton m_start;
  QPushButton m_stop;
  QLabel m_addr_label;
  QLabel m_port_label;
  QLabel m_log_label;
  QTextBrowser m_log;
  QTimer m_timer;
  Controller m_ctl;
  std::queue<std::string> m_log_queue;
  std::mutex m_mtx;
  bool m_running = false;
 public:
  ControllerView()
      : m_addr(this),
        m_port(this),
        m_start(this),
        m_stop(this),
        m_addr_label(this),
        m_port_label(this),
        m_log_label(this),
        m_log(this),
        m_timer(this),
        m_ctl("/tmp/http_server_controller") {
    this->resize(1024, 768);
    m_addr_label.setText("地址");
    m_port_label.setText("端口");
    m_log_label.setText("日志");
    m_start.setText("启动");
    m_stop.setText("停止");
    m_addr.setGeometry(100, 20, 200, 25);
    m_port.setGeometry(100, 55, 200, 25);
    m_start.setGeometry(400, 55, 100, 25);
    m_stop.setGeometry(400, 55, 100, 25);
    m_addr_label.setGeometry(20, 20, 60, 25);
    m_port_label.setGeometry(20, 55, 60, 25);
    m_log_label.setGeometry(20, 90, 60, 25);
    m_log.setGeometry(100, 90, 600, 600);
    m_addr.show();
    m_port.show();
    m_start.show();
    m_stop.hide();
    m_addr_label.show();
    m_port_label.show();
    m_log_label.show();
    m_log.show();
    m_timer.start(100);
    QObject::connect(&m_start, &QPushButton::clicked, this, &ControllerView::onStartButtonClick);
    QObject::connect(&m_stop, &QPushButton::clicked, this, &ControllerView::onStopButtonClick);
    QObject::connect(&m_timer, &QTimer::timeout, this, &ControllerView::onAppendLog);
    m_ctl.REGISTER_NOTIFY_HANDLER(Log, [this](const proto::LogNotify &notify) {
      std::lock_guard lock(m_mtx);
      m_log_queue.push(notify.log);
      return NoError;
    });
  }
  void onStartButtonClick(bool) {
    if (auto err = m_ctl.StartNewServer(m_addr.text().toStdString(), m_port.text().toUShort());err != NoError) {
      m_log.append("Start server failed: ");
      m_log.append(err.desc);
    } else {
      m_running = true;
      m_start.hide();
      m_stop.show();
    }
  }
  void onStopButtonClick(bool) {
    if (auto err = m_ctl.StopServer(); err != NoError) {
      m_log.append("Stop server failed: ");
      m_log.append(err.desc);
    } else {
      m_running = false;
      m_stop.hide();
      m_start.show();
    }
  }
  void onAppendLog() {
    std::lock_guard lock(m_mtx);
    while (!m_log_queue.empty()) {
      m_log.append(QString::fromStdString(std::move(m_log_queue.front())));
      m_log_queue.pop();
    }
    m_timer.setInterval(100);
  }
};

int main(int argc, char *argv[]) {
  auto server = New<TCPServer>("/tmp/http_server_controller");
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