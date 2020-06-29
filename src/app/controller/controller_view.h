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
using ActionList = std::vector<Action>;
inline static ActionList &operator+=(ActionList &d, Action &&action) {
  d.emplace_back(std::move(action));
  return d;
}

class ControllerView : public QWidget {
 public:
  constexpr static int tick = 100; // ms
 private:
  QLineEdit m_addr;
  QLineEdit m_port;
  QLineEdit m_root;
  QPushButton m_start;
  QPushButton m_stop;
  QLabel m_addr_label;
  QLabel m_port_label;
  QLabel m_root_label;
  QLabel m_log_label;
  QTextBrowser m_log;
  QTimer m_timer;
  Controller m_ctl;
  ActionList m_actions;
  std::mutex m_mtx;
  bool m_running = false;
 public:
  ControllerView();
  void onStartButtonClick(bool);
  void onStopButtonClick(bool);
  void onTick();
};
#endif //LILY_SRC_APP_CONTROLLER_CONTROLLER_VIEW_H_
