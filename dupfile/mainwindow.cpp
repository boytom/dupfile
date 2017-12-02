#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QList>
#include <QDebug>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QCloseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <climits>
#include <cstdlib>

#if defined(_MSC_VER)
#pragma execution_character_set("utf-8")
#endif

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  setupUi(this);
  //setWindowFlags(windowFlags()^Qt::WindowMaximizeButtonHint);
  disable_something_when_boot();
  //Ui_MainWindow::statusBar->addPermanentWidget(progressBar);
  Ui_MainWindow::statusBar->addWidget(progressBar, 0);
  Ui_MainWindow::statusBar->addWidget(label, 1);

  tree_model_ = new QStandardItemModel(this);
  //tree_model_->setHeaderData(1, Qt::Horizontal, QStringLiteral(u"Hash Value"));
  //QString::fromStdWString(L":/img/4.gif")
  resultWidget->setModel(tree_model_);

  filePathLineEdit->setText(QStringLiteral(u"abc"));
  filePathLabelTip->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

  connect(filePathLineEdit, &QLineEdit::editingFinished, this, &MainWindow::showFilePath);
  showFilePath();

#if defined(__linux__)
  setAppStyleSheet(QString::fromStdWString(L":/css/dupfilelinux.css"));
#elif defined(_MSC_VER)
  setAppStyleSheet(QString::fromStdWString(L":/css/dupfilewin.css"));
#endif
}

MainWindow::~MainWindow()
{
  if (tree_model_)
    delete tree_model_;
}

void MainWindow::setAppStyleSheet(const QString &styleSheetResource)
{
  QFile file(styleSheetResource);
  file.open(QFile::ReadOnly);
  qApp->setStyleSheet(file.readAll());
  setStyleSheet(file.readAll());
  qApp->setPalette(QPalette(QColor("#F0F0F0")));
}


void MainWindow::on_addButton_clicked()
{
  //QMessageBox::information(this, "提示", "中文文字",  QMessageBox::Yes | QMessageBox::No);
  QString dir = QFileDialog::getExistingDirectory(this);
  if(dir.isEmpty())
    return;
  QList<QListWidgetItem *> item_list = listWidget->findItems(dir, Qt::MatchExactly);
  if(item_list.empty()) {
    listWidget->addItem(dir);
    listWidget->sortItems();
  }
  else {
    QMessageBox mess(this);
    mess.setWindowTitle("提示");
    mess.setIcon(QMessageBox::Information);
    mess.setText(QString("\"") + dir + QString("\" 已放入列表。"));
    mess.addButton("确定", QMessageBox::AcceptRole);
    mess.exec();
  }
  enable_something_disabled_when_boot();
}

void MainWindow::on_delButton_clicked()
{
  int row = listWidget->currentRow();

  if(row == -1) {
    QMessageBox msg(this);
    msg.setWindowTitle("警告");
    msg.setIcon(QMessageBox::Warning);
    msg.setText("请首先选择要删除的目录，再点击删除按钮（只从列表中删除，不会删除磁盘上的目录）。");
    msg.addButton("确定", QMessageBox::AcceptRole);
    msg.exec();
  }
  QListWidgetItem *item = listWidget->takeItem(listWidget->currentRow());
  delete item;
  if(listWidget->count() == 0)
    disable_something_when_boot();
}

void MainWindow::on_startButton_clicked()
{
  //QMessageBox::information(this, "提示", "中文文字",  QMessageBox::Yes | QMessageBox::No);
  QStringList dir_list;
  int i = 0;
  QListWidgetItem *item;
  if(listWidget->count() == 0)
    return;
  while((item = listWidget->item(i++)) != NULL)
    dir_list.push_back(item->text());
  if(dir_list.isEmpty())
    return;
  disable_something_when_start();
  startWorkInAThread(dir_list);
}

void MainWindow::on_stopButton_clicked()
{
  //QMessageBox::information(this, "提示", "中文文字",  QMessageBox::Yes | QMessageBox::No);
  if(worker_thread_) {
    if(worker_thread_->get_pause()) {
      emit resume_worker();
      pauseButton->setText("暂停");
    }
    stopButton->setEnabled(false);
    emit stop_worker();
    worker_thread_->wait();
    stopButton->setEnabled(true);
  }
}

void MainWindow::on_pauseButton_clicked()
{
  if(!worker_thread_)
    return;
  if(!worker_thread_->get_pause()) {
    pauseButton->setEnabled(false);
    emit pause_worker();
    pauseButton->setText("继续");
    pauseButton->setEnabled(true);
  }
  else {
    pauseButton->setEnabled(false);
    emit resume_worker();
    pauseButton->setText("暂停");
    pauseButton->setEnabled(true);
  }
}

void MainWindow::on_loadButton_clicked()
{
  // disable any buttons
#if 0
  disable_something_when_start();
  pauseButton->setEnabled(false);
  stopButton->setEnabled(false);
#endif

  loadButton->setEnabled(false);

  QMap<QByteArray, QString> map;
  load_map(&map, get_realpath(filePathLineEdit->text()));
  show_map(&map);

  loadButton->setEnabled(true);
#if 0
  pauseButton->setEnabled(true);
  stopButton->setEnabled(true);
  enable_something_disabled_when_boot();
#endif
}

void MainWindow::handleResults(QMap<QByteArray, QString> *result)
{
#if defined(__linux__)
   delete worker_thread_;
#elif defined(_MSC_VER)

  /*
   *
  delete worker_thread_; //QThread: Destroyed while thread is still running 2016-07-20 06:45 am on windows
  */
#endif
  worker_thread_ = NULL;

  dump_map(result, get_realpath(filePathLineEdit->text().trimmed()));
  show_map(result);
  write_script(result, get_realpath(filePathLineEdit->text().trimmed()));

  delete result;

  enalbe_anything_disabled_when_start();
}

void MainWindow::showFilePath()
{
  QString rp = get_realpath(filePathLineEdit->text().trimmed());
  filePathLabelTip->setText(QStringLiteral(u"二进制绝对文件路径：") + rp + QStringLiteral(u".bin\n")
                     + QStringLiteral(u"重复文件列表绝对路径：") + rp + QStringLiteral(u".txt\n")
                     + QStringLiteral(u"zsh 脚本绝对路径：") + rp + QStringLiteral(u".sh\n")
                            + QStringLiteral(u"bat 脚本绝对路径：") + rp + QString::fromWCharArray(L".bat"));
}

void MainWindow::on_resultWidget_collapsed(const QModelIndex &index)
{
  resultWidget->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
}

void MainWindow::on_resultWidget_expanded(const QModelIndex &index)
{
  resultWidget->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
}

void MainWindow::dump_map(QMap<QByteArray, QString> *result, const QString &file_path)
{
  FILE *fp, *fbin;
#if defined(linux)
  fp = fopen((file_path + QString::fromWCharArray(L".txt")).toStdString().c_str(), "w");
  fbin = fopen((file_path + QString::fromWCharArray(L".bin")).toStdString().c_str(), "w");
#else
  fp = _wfopen((file_path + QString::fromWCharArray(L".txt")).toStdWString().c_str(), L"wt");
  fbin = _wfopen((file_path + QString::fromWCharArray(L".bin")).toStdWString().c_str(), L"wb");
#endif

#if 0
  QMap<QByteArray, QString>::iterator iter;
  for(iter = result->begin(); iter != result->end(); iter++) {
    fprintf(fp, "%s %s\n", QString(iter.key().toHex()).toStdString().c_str(), iter.value().toStdString().c_str());
  }
#endif
  QList<QByteArray> keys;
  QList<QString> values;
  keys = result->uniqueKeys();
  QList<QByteArray>::iterator key_iter;

  for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++) {
    values = result->values(*key_iter);
    MapFile map_file;
    memset(&map_file, 0, sizeof(MapFile));
    memcpy(map_file.key_data, key_iter->data(), map_file.key_length = key_iter->size());
    map_file.file_number = values.size();
    fwrite(&map_file, sizeof(MapFile), 1U, fbin);

    if(values.size() > 1)
      fwprintf(fp, L"%ls\nfile numbers = %d\n", QString((*key_iter).toHex()).toStdWString().c_str(), values.size());
    QList<QString>::iterator string_iter;
    for(string_iter = values.begin(); string_iter != values.end(); ++string_iter) {
      if(values.size() > 1)
        fwprintf(fp, L"\t%ls\n", string_iter->toStdWString().c_str());

      long long unsigned int length = string_iter->toStdString().length();
      fwrite(&length, sizeof(length), 1U, fbin);
      fwrite(string_iter->toStdString().c_str(), 1U, length, fbin);
    }
    if(values.size() > 1)
      fputwc(L'\n', fp);
  }
  fclose(fp);
  fclose(fbin);
}

void MainWindow::load_map(QMap<QByteArray, QString> *map, const QString &file_path)
{
  if(map == NULL)
    return;
  MapFile map_file;
  FILE *fbin;
#if defined(linux)
  fbin = fopen((file_path + QString::fromWCharArray(L".bin")).toStdString().c_str(), "r");
#else
  fbin = _wfopen((file_path + QString::fromWCharArray(L".bin")).toStdWString().c_str(), L"rb");
#endif

  QByteArray key;

  while(!feof(fbin)
        && !ferror(fbin)
        && fread(&map_file, sizeof(MapFile), 1U, fbin) == 1) {
    key.clear();
    key.append(map_file.key_data, map_file.key_length);
    for(unsigned int i = 0U; i < map_file.file_number; ++i) {
      long long unsigned int length;
      char *buf;
      fread(&length, sizeof(length), 1U, fbin);
      if((buf = (char *)malloc(length + 1)) == NULL)
        return;
      fread(buf, 1U, length, fbin);
      buf[length] = '\0';
      map->insertMulti(key, QString(buf));
      free(buf);
    }
  }
  fclose(fbin);
}

void MainWindow::show_map(QMap<QByteArray, QString> *result)
{
  QList<QByteArray> keys;
  QList<QString> values;
  keys = result->uniqueKeys();

  QList<QByteArray>::iterator key_iter;

  tree_model_->clear();
  tree_model_->setHorizontalHeaderLabels(QStringList() << QString::fromStdWString(L"Hash value/File Name")
                                         << QString::fromStdWString(L"File Count/File Size(bytes)")
                                         << QString::fromStdWString(L"File Size(bytes)"));

  QStandardItem *parentItem = tree_model_->invisibleRootItem();

  unsigned long long int total_bytes{0llu}, total_bytes_exclude{0llu};
  for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++) {
    unsigned long long int bytes;

    values = result->values(*key_iter);

    if(values.size() == 1)
      continue;

    QStandardItem *key_item = new QStandardItem(QString((*key_iter).toHex()));

    parentItem->appendRow(key_item);

    key_item->setEditable(false);

    //key_item->appendColumn(QList<QStandardItem *>() << new QStandardItem(QString("%0").arg(values.size())));
    tree_model_->setItem(tree_model_->indexFromItem(key_item).row(), 1, new QStandardItem(QString("%0").arg(values.size())));

    QList<QString>::iterator string_iter = values.begin();
    tree_model_->setItem(key_item->index().row(), 2, new QStandardItem(QString("%0").arg(QFile(*string_iter).size())));

    total_bytes_exclude -= QFile(*string_iter).size();

    for(string_iter = values.begin(); string_iter != values.end(); ++string_iter) {
      QStandardItem *value_item;
      total_bytes += bytes = QFile(*string_iter).size();
      total_bytes_exclude += bytes;

      value_item = new QStandardItem(*string_iter);

      key_item->appendRow(value_item);

      //value_item->appendColumn(QList<QStandardItem *>() << new QStandardItem(QString("%0").arg(values.size())));
      //tree_model_->setItem(value_item->index().row(), 1, new QStandardItem(QString("%0").arg(values.size())));
      //key_item->appendColumn(QList<QStandardItem *>() << new QStandardItem(QString("%0").arg(values.size())));

      key_item->setChild(value_item->index().row(), 1, new QStandardItem(QString("%0").arg(bytes)));
      key_item->setChild(value_item->index().row(), 2, new QStandardItem(QString("%0").arg(bytes)));
      value_item->setEditable(false);
    }
  }
  QStandardItem *total_item = new QStandardItem(QString::fromWCharArray(L"总计"));
  total_item->setEditable(false);
  parentItem->appendRow(total_item);

  QStandardItem *item = new QStandardItem(QString("%0（所有）").arg(total_bytes));
  item->setEditable(false);
  tree_model_->setItem(total_item->index().row(), 1, item);

  item = new QStandardItem(QString("%0（去一个重复）").arg(total_bytes_exclude));
  item->setEditable(false);
  tree_model_->setItem(total_item->index().row(), 2, item);

  QStandardItem *size_item = new QStandardItem(QString::fromWCharArray(L"总大小 KiB"));
  total_item->appendRow(size_item);
  item = new QStandardItem(QString("%0（所有）").arg((double)total_bytes / 1024.0));
  total_item->setChild(size_item->index().row(), 1, item);
  item = new QStandardItem(QString("%0（去一个重复）").arg((double)total_bytes_exclude / 1024.0));
  total_item->setChild(size_item->index().row(), 2, item);

  size_item = new QStandardItem(QString::fromWCharArray(L"总大小 MiB"));
  total_item->appendRow(size_item);
  item = new QStandardItem(QString("%0（所有）").arg((double)total_bytes / (1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 1, item);
  item = new QStandardItem(QString("%0（去一个重复）").arg((double)total_bytes_exclude / (1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 2, item);

  size_item = new QStandardItem(QString::fromWCharArray(L"总大小 GiB"));
  total_item->appendRow(size_item);
  item = new QStandardItem(QString("%0（所有）").arg((double)total_bytes / (1024.0 * 1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 1, item);
  item = new QStandardItem(QString("%0（去一个重复）").arg((double)total_bytes_exclude / (1024.0 * 1024.0 * 1024.0)));
  total_item->setChild(size_item->index().row(), 2, item);

  resultWidget->header()->resizeSections(/*QHeaderView::Interactive*/QHeaderView::ResizeToContents);
  resultWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::write_script(QMap<QByteArray, QString> *result, const QString &file_path)
{
  FILE *script;
#if defined(linux)
  if((script = fopen((file_path + QString::fromWCharArray(L".sh")).toStdString().c_str(), "w")) == NULL)
    return;
  fputws(L"#!/bin/zsh\n", script);

  const wchar_t *comment = L"# ";
  const wchar_t *command = L"rm -f ";
#elif defined(_MSC_VER)
  if((script = _wfopen((file_path + QString::fromWCharArray(L".bat")).toStdWString().c_str(), L"wt")) == NULL)
    return;
  fputws(L"@echo off\n", script);
  fputws(L"setlocal enabledelayedexpansion ENABLEEXTENSIONS\n", script);

  const wchar_t *comment = L"rem ";
  const wchar_t *command = L"del ";
#endif

  QList<QByteArray> keys;
  QList<QString> values;
  keys = result->uniqueKeys();

  QList<QByteArray>::iterator key_iter;

  for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++) {
    values = result->values(*key_iter);

    if(values.size() == 1)
      continue;

    QList<QString>::iterator string_iter = values.begin();

    fwprintf(script, L"%ls%ls\n%lsfile numbers = %d\n",
             comment,
             QString((*key_iter).toHex()).toStdWString().c_str(),
             comment,
             values.size());

    fwprintf(script, L"%ls\"%ls\"\n",
             comment,
             (*string_iter).toStdWString().c_str());

    while(++string_iter != values.end())
      fwprintf(script, L"%ls\"%ls\"\n", command, (*string_iter).toStdWString().c_str());
  }
#if defined(linux)
  fputws(L"exit 0\n", script);
#elif defined(_MSC_VER)
  fputws(L"@echo on\n", script);
#endif
  fclose(script);
}

QString MainWindow::get_realpath(const QString &relative_path)
{
  QString real_path;
#if defined(__linux__)
  char resolved[PATH_MAX]{0};
  realpath(relative_path.toStdString().c_str(), resolved);
  real_path = resolved;
#elif defined(_MSC_VER)
  wchar_t resolved[_MAX_PATH]{0};
  _wfullpath(resolved, relative_path.toStdWString().c_str(), sizeof(resolved) / sizeof(resolved[0]));
  real_path = QString::fromWCharArray(resolved);
#endif
  return real_path;
}

void  MainWindow::disable_something_when_start()
{
  Md4->setEnabled(false);
  Md5->setEnabled(false);
  Sha1->setEnabled(false);
  Sha224->setEnabled(false);
  Sha384->setEnabled(false);
  Sha256->setEnabled(false);
  Sha512->setEnabled(false);
  Sha3_224->setEnabled(false);
  Sha3_384->setEnabled(false);
  Sha3_256->setEnabled(false);
  Sha3_512->setEnabled(false);
  delButton->setEnabled(false);
  addButton->setEnabled(false);
  startButton->setEnabled(false);
  loadButton->setEnabled(false);

  pauseButton->setEnabled(true);
  stopButton->setEnabled(true);
  //resultWidget->clear();
}

void  MainWindow::enalbe_anything_disabled_when_start()
{
  Md4->setEnabled(true);
  Md5->setEnabled(true);
  Sha1->setEnabled(true);
  Sha224->setEnabled(true);
  Sha384->setEnabled(true);
  Sha256->setEnabled(true);
  Sha512->setEnabled(true);
  Sha3_224->setEnabled(true);
  Sha3_384->setEnabled(true);
  Sha3_256->setEnabled(true);
  Sha3_512->setEnabled(true);
  delButton->setEnabled(true);
  addButton->setEnabled(true);
  startButton->setEnabled(true);
  loadButton->setEnabled(true);

  pauseButton->setEnabled(false);
  stopButton->setEnabled(false);
}

void MainWindow::disable_something_when_boot()
{
  delButton->setEnabled(false);
  startButton->setEnabled(false);
  pauseButton->setEnabled(false);
  stopButton->setEnabled(false);
}

void MainWindow::enable_something_disabled_when_boot()
{
  delButton->setEnabled(true);
  startButton->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if(worker_thread_ == NULL)
    return;
  bool itsme = false;
  QMessageBox msg(this);
  msg.setWindowTitle("问题");
  msg.setIcon(QMessageBox::Question);
  msg.setText("正在比较文件,真的要退出吗?");
  QAbstractButton *sureButton = msg.addButton("确定", QMessageBox::AcceptRole);
  msg.addButton("取消", QMessageBox::RejectRole);
  if(!worker_thread_->get_pause()) {
    on_pauseButton_clicked();
    itsme = true;
  }
  msg.exec();
  if(msg.clickedButton() == sureButton)
    on_stopButton_clicked();
  else {
    if(itsme)
      on_pauseButton_clicked();
    event->ignore();
  }
}

void MainWindow::startWorkInAThread(QStringList &dir_list)
{
  worker_thread_ = new CompareThread(this);
  worker_thread_->add_dir_list(dir_list);
  worker_thread_->set_algorithm(algorithm_);
  connect(this, &MainWindow::stop_worker, worker_thread_, &CompareThread::stop);
  connect(this, &MainWindow::pause_worker, worker_thread_, &CompareThread::pause);
  connect(this, &MainWindow::resume_worker, worker_thread_, &CompareThread::resume);
  connect(worker_thread_, &CompareThread::resultReady, this, &MainWindow::handleResults);
  connect(worker_thread_, &CompareThread::finished, worker_thread_, &QObject::deleteLater);
  connect(worker_thread_, &CompareThread::update_text_msg, label, &QLabel::setText);
  //connect(worker_thread_, &CompareThread::update_current_dir_msg, labelDir, &QLabel::setText);
  connect(worker_thread_, &CompareThread::update_progressbar, progressBar, &QProgressBar::setValue);
  //QObject::connect(*(_DWORD *)(v1 + 220), "2actionEffectsOptions()", v1, "1actionEffectsOptions()", 0);
  worker_thread_->start();
}

void MainWindow::on_Md4_clicked()
{
  algorithm_ = QCryptographicHash::Md4;
}

void MainWindow::on_Md5_clicked()
{
  algorithm_ = QCryptographicHash::Md5;
}

void MainWindow::on_Sha1_clicked()
{
  algorithm_ = QCryptographicHash::Sha1;
}

void MainWindow::on_Sha224_clicked()
{
  algorithm_ = QCryptographicHash::Sha224;
}

void MainWindow::on_Sha256_clicked()
{
  algorithm_ = QCryptographicHash::Sha256;
}

void MainWindow::on_Sha384_clicked()
{
  algorithm_ = QCryptographicHash::Sha384;
}

void MainWindow::on_Sha512_clicked()
{
  algorithm_ = QCryptographicHash::Sha512;
}

void MainWindow::on_Sha3_224_clicked()
{
  algorithm_ = QCryptographicHash::Sha3_224;
}

void MainWindow::on_Sha3_256_clicked()
{
  algorithm_ = QCryptographicHash::Sha3_256;
}

void MainWindow::on_Sha3_384_clicked()
{
  algorithm_ = QCryptographicHash::Sha3_384;
}

void MainWindow::on_Sha3_512_clicked()
{
  algorithm_ = QCryptographicHash::Sha3_512;
}

//http://bastian.rieck.ru/blog/posts/2014/selections_qt_osg/
