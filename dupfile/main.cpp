#include "mainwindow.h"
#include <locale.h>
#include <QApplication>
#include <fcntl.h>

#if defined(__linux__)

int main(int argc, char *argv[], char *env[])
{
  setlocale(LC_ALL, "");

  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}

#elif defined(_MSC_VER)

#include <tchar.h>
#include <io.h>

// �ɹ����� argc��ʧ�ܷ���-1
static int get_argv(int argc, /* const */ wchar_t *argv[], char ***char_argv);
static void destroy_argv(char ***char_argv);

int wmain(int argc, wchar_t *argv[], wchar_t *env[])
{
  char **char_argv, **p;
  int res;

  _setmode(_fileno(stdout), _O_WTEXT);
  _setmode(_fileno(stderr), _O_U16TEXT);
  _setmode(_fileno(stdin), _O_U16TEXT);
  _wsetlocale(LC_ALL, L"chs_china.936");

  argc = get_argv(argc, argv, &p);

  char_argv = p;
  QApplication a(argc, char_argv);
  MainWindow w;
  w.show();

  res = a.exec();
  destroy_argv(&p);
  return res;
}

static int get_argv(int argc, /* const */ wchar_t *argv[], char ***char_argv) {
  char **pchar_argv;
  if (argv == NULL || char_argv == NULL)
    return -1;

  *char_argv = (char **)malloc(sizeof(char *) * (argc + 1));
  memset(*char_argv, 0, sizeof(char *) * (argc + 1));

  pchar_argv = *char_argv;
  while(*argv) {
     long long unsigned int chars;
     if ((chars = wcstombs(NULL, *argv, -1) + 1) == 0u) {
       --argc;
       continue;
     }
     if ((*pchar_argv = (char *)malloc(chars)) == NULL) {
       --argc;
       continue;
     }
     memset(*pchar_argv, 0, chars);
     // must let wcstombs encounter the wide-character null character
     wcstombs(*pchar_argv, *argv, wcslen(*argv) + 1);
     ++argv;
     ++pchar_argv;
  }
  return argc;
}

static void destroy_argv(char ***char_argv) {

  if(char_argv == NULL || *char_argv == NULL)
    return;
  for (char **p = *char_argv; *p; ++p)
    free(*p);
  free(*char_argv);
}
#endif
