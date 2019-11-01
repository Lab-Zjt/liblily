#include "controller_view.h"

ControllerView::ControllerView()
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
  m_timer.start(tick);
  QObject::connect(&m_start, &QPushButton::clicked, this, &ControllerView::onStartButtonClick);
  QObject::connect(&m_stop, &QPushButton::clicked, this, &ControllerView::onStopButtonClick);
  QObject::connect(&m_timer, &QTimer::timeout, this, &ControllerView::onTick);
  m_ctl.RegisterHandler<proto::LogNotify>([this](const proto::LogNotify &notify) {
    std::lock_guard lock(m_mtx);
    m_actions += [log = notify.log, this]() { m_log.append(QString::fromStdString(log)); };
    return NoError;
  }
  );
}

void ControllerView::onStartButtonClick(bool) {
  if (auto err = m_ctl.StartNewServer(m_addr.text().toStdString(), m_port.text().toUShort());err != NoError) {
    m_log.append("Start server failed: ");
    m_log.append(err.desc);
  } else {
    m_running = true;
    m_start.hide();
    m_stop.show();
  }
}

void ControllerView::onStopButtonClick(bool) {
  if (auto err = m_ctl.StopServer(); err != NoError) {
    m_log.append("Stop server failed: ");
    m_log.append(err.desc);
  } else {
    m_running = false;
    m_stop.hide();
    m_start.show();
  }
}

void ControllerView::onTick() {
  Delegate delegate;
  {
    std::lock_guard lock(m_mtx);
    delegate.swap(m_actions);
  }
  for (auto &&action : delegate) {
    action();
  }
  m_timer.setInterval(tick);
}