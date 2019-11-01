#ifndef LILY_SRC_APP_CONTROLLER_CONTROLLER_VIEW_H_
#define LILY_SRC_APP_CONTROLLER_CONTROLLER_VIEW_H_
#include "controller.h"
#include <QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QLineEdit>
#include <QtCore/QTimer>
#include <queue>
#include <mutex>

using Action = std::function<void()>;
using Delegate = std::vector<Action>;
inline static Delegate &operator+=(Delegate &d, Action &&action) {
  d.emplace_back(std::move(action));
  return d;
}

class ControllerView : public QWidget {
 public:
  constexpr static int tick = 100; // ms
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
  Delegate m_actions;
  std::mutex m_mtx;
  bool m_running = false;
 public:
  ControllerView();
  void onStartButtonClick(bool);
  void onStopButtonClick(bool);
  void onTick();
};
#endif //LILY_SRC_APP_CONTROLLER_CONTROLLER_VIEW_H_
