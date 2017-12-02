#include "comparethread.h"
#include <QString>
#include <QDirIterator>
#include <QMap>
#include <QProgressBar>
#include <QDebug>
#include <QLabel>

#if defined(_MSC_VER)
#pragma execution_character_set("utf-8")
#endif

CompareThread::CompareThread(QObject *parent) :  QThread(parent)
{

}

CompareThread::~CompareThread()
{
 qDebug() << "be deleted.";
}

#if 0
void CompareThread::run()
{
  FILE *fp;
#if defined(linux)
  fp = fopen("/home/whg/abc.txt", "a");
#else
  fp = fopen("D:\\abc.txt", "at");
#endif

  QStringListIterator iter(dir_list_);
  QCryptographicHash hash(algorithm_);
  QFile file;

  while(iter.hasNext()) {
    QDirIterator dir_iter(iter.next(), QDirIterator::Subdirectories);
    while(dir_iter.hasNext()) {
      QString file_path = dir_iter.next();
      if(dir_iter.fileInfo().isDir())
        continue;
      file.setFileName(file_path);
      file.open(QIODevice::ReadOnly);

      if(hash.addData(&file))
        fprintf(fp, "%s %s\n", QString(hash.result().toHex()).toStdString()
                .c_str(), file_path.toStdString().c_str());
      else
        fprintf(fp, "error %s\n", file_path.toStdString().c_str());
      file.close();
      hash.reset();
    }
  }
  fclose(fp);
}
#endif

void CompareThread::run()
{
  QStringListIterator iter(dir_list_);
  QCryptographicHash hash(algorithm_);
  QFile file;
  QMap<QByteArray, QString> *result = new QMap<QByteArray, QString>;
  double file_count = 0.0;
  char msg[BUFSIZ];

  file_total();
  if(file_total_ == 0.0)
    return;

  emit update_text_msg("开始计算……");
  while(iter.hasNext() && !(volatile bool)stop_) {
    QDirIterator dir_iter(iter.next(), QDirIterator::Subdirectories);
    while(dir_iter.hasNext() && !(volatile bool)stop_) {
      mutex_.lock();
      while((volatile bool)pause_) {
        emit update_text_msg("计算暂停……");
        condition_.wait(&mutex_);
      }
      QString file_path = dir_iter.next();
      if(dir_iter.fileInfo().isDir()) {
        mutex_.unlock();
        continue;
      }
      file.setFileName(file_path);
      file.open(QIODevice::ReadOnly);
      if(hash.addData(&file))
        result->insertMulti(hash.result(), file_path);
      emit update_current_dir_msg(dir_iter.filePath());
#if defined(_MSC_VER)
      _snprintf(msg, sizeof(msg) - 1, "%.0lf/%.0lf %s", ++file_count,
                file_total_, dir_iter.fileName().toStdString().c_str());
#else
      snprintf(msg, sizeof(msg), "%.0lf/%.0lf %s", ++file_count, file_total_,
               dir_iter.fileName().toStdString().c_str());
#endif
      emit update_text_msg(msg);
      emit update_progressbar(file_count * 100.0 / file_total_);
      file.close();
      hash.reset();
      mutex_.unlock();
    }
  }
  emit update_text_msg("计算完成。");
  emit resultReady(result);
}

void CompareThread::add_dir(QString &dir)
{
  dir_list_.push_back(dir);
}

void CompareThread::add_dir_list(QStringList &dir_list)
{
  dir_list_ += dir_list;
}

void CompareThread::set_algorithm(QCryptographicHash::Algorithm algorithm)
{
  algorithm_ = algorithm;
}

void CompareThread::stop()
{
  stop_ = true;
}

bool CompareThread::get_pause()
{
  return pause_;
}

void CompareThread::pause()
{
  pause_ = true;
  mutex_.lock();
}

void CompareThread::resume()
{
  pause_ = false;
  condition_.wakeOne();
  mutex_.unlock();
}

void CompareThread::file_total()
{
  char buf[128];

  QStringListIterator iter(dir_list_);

  file_total_ = 0.0;

  emit update_text_msg("计算文件总数……");
  while(iter.hasNext() && !(volatile bool)stop_) {
    QDirIterator dir_iter(iter.next(), QDirIterator::Subdirectories);
    while(dir_iter.hasNext() && !(volatile bool)stop_) {
      mutex_.lock();
      while((volatile bool)pause_)
        condition_.wait(&mutex_);
      QString file_path = dir_iter.next();
      if(dir_iter.fileInfo().isDir()) {
        mutex_.unlock();
        continue;
      }
      file_total_++;
#if defined(_MSC_VER)
      _snprintf(buf, sizeof(buf) - 1, "计算文件总数…… %.0lf %s", file_total_,
                dir_iter.fileName().toStdString().c_str());
#else
      snprintf(buf, sizeof(buf), "计算文件总数…… %.0lf %s", file_total_,
               dir_iter.fileName().toStdString().c_str());
#endif
      emit update_text_msg(buf);
      mutex_.unlock();
    }
  }
#if defined(_MSC_VER)
  _snprintf(buf, sizeof(buf) - 1, "计算文件总数完毕。 %.0lf", file_total_);
#else
  snprintf(buf, sizeof(buf), "计算文件总数完毕。 %.0lf", file_total_);
#endif
  emit update_text_msg(buf);
}
