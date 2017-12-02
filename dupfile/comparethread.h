#ifndef COMPARETHREAD_H
#define COMPARETHREAD_H

#include <QThread>
#include <QStringList>
#include <QCryptographicHash>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>

class QProgressBar;
class QLabel;

class CompareThread : public QThread
{
  Q_OBJECT
public:
  explicit CompareThread(QObject *parent = 0);
  virtual ~CompareThread();

  void run() Q_DECL_OVERRIDE;
  void add_dir(QString &dir);
  void add_dir_list(QStringList &dir_list);
  void set_algorithm(QCryptographicHash::Algorithm  algorithm);

  bool get_pause();

signals:
  void resultReady(QMap<QByteArray, QString> *result);
  void update_text_msg(const QString &msg);
  void update_current_dir_msg(const QString &msg);
  void update_progressbar(int value);

public slots:
  void stop();
  void pause();
  void resume();


private:
  QMap<QByteArray, QString> *result_{nullptr};
  bool stop_{false}, pause_{false};
  double file_total_{0.0};
  QCryptographicHash::Algorithm algorithm_{QCryptographicHash::Sha3_512};
  QMutex mutex_;
  QWaitCondition condition_;
  QStringList dir_list_;

  void file_total();
};

#endif // COMPARETHREAD_H
