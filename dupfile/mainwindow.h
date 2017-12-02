#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QCryptographicHash>
#include <QPainter>
#include "comparethread.h"

#ifdef linux
enum {MY_PATH_MAX = PATH_MAX};
#else
enum {MY_PATH_MAX = 4096};
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4200)
#endif

class QStandardItemModel;

struct FilePath {
  unsigned int length;
  char path[0];
};

struct MapFile {
  unsigned int key_length;
  unsigned int file_number;
  char key_data[64];
};

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();
    void startWorkInAThread(QStringList &dir_list);
    void setAppStyleSheet(const QString &styleSheetResource);

signals:
    void stop_worker();
    void pause_worker();
    void resume_worker();

private slots:
    void on_addButton_clicked();
    void on_delButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_pauseButton_clicked();
    void on_loadButton_clicked();

    void on_Md4_clicked();
    void on_Md5_clicked();
    void on_Sha1_clicked();
    void on_Sha224_clicked();
    void on_Sha256_clicked();
    void on_Sha384_clicked();
    void on_Sha512_clicked();
    void on_Sha3_224_clicked();
    void on_Sha3_256_clicked();
    void on_Sha3_384_clicked();
    void on_Sha3_512_clicked();

    void handleResults(QMap<QByteArray, QString> *result);
    void showFilePath(void);

    void on_resultWidget_collapsed(const QModelIndex &index);
    void on_resultWidget_expanded(const QModelIndex &index);

private:
    enum QCryptographicHash::Algorithm algorithm_{QCryptographicHash::Sha3_512};
    CompareThread *worker_thread_{nullptr};
    QStandardItemModel *tree_model_{nullptr};

    void dump_map(QMap<QByteArray, QString> *result, const QString &file_path);
    void load_map(QMap<QByteArray, QString> *map, const QString &file_path);

    void show_map(QMap<QByteArray, QString> *result);

    void write_script(QMap<QByteArray, QString> *result, const QString &file_path);

    QString get_realpath(const QString &relative_path);

    void disable_something_when_start();
    void enalbe_anything_disabled_when_start();

    void disable_something_when_boot();
    void enable_something_disabled_when_boot();

    QRect makeRectangle( const QPoint& first, const QPoint& second );
    void closeEvent(QCloseEvent *event) final;
};

#endif // MAINWINDOW_H
