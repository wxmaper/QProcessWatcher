# QProcessWatcher
Simple interface for monitoring state of process for Windows.

# Usage
```c++
QProcessWatcher watcher;
watcher.addProcess("MSPaint.exe");
```

# Examples

# Example 1

```c++
#include <QApplication>
#include <QProcessWatcher/qprocesswatcher.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create widget
    QWidget w;

    // Create watcher and add monitoring for "MSPaint.exe" process
    QProcessWatcher watcher;
    watcher.addProcess("MSPaint.exe");

    // Connect signals
    QObject::connect(&watcher, SIGNAL(processFinished(QString)), &w, SLOT(close()));
    QObject::connect(&watcher, SIGNAL(processStarted(QString,QString)), &w, SLOT(show()));

    return a.exec();
}
```

This windget-based application wait for starting "MSPaint.exe" process and then shows the Widget form.  
Widget has closed automatically when "MSPaint.exe" is closed.

# Example 2

```c++
#include <QApplication>
#include <QMessageBox>
#include <QProcessWatcher/qprocesswatcher.h>

void onProcessStarted(const QString &processName,
                      const QString &originalProcessName)
{
    const QString title = QObject::tr("Process finished");
    const QString message = QObject::tr("Process `%1` is started with name `%2`!")
            .arg(processName)
            .arg(originalProcessName);

    QMessageBox::information(nullptr, title, message);
}

void onProcessFinished(const QString &processName)
{
    const QString title = QObject::tr("Process finished");
    const QString message = QObject::tr("Process `%1` is finished!")
            .arg(processName);

    QMessageBox::information(nullptr, title, message);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QProcessWatcher watcher;
    watcher.addProcess("MSPaint.exe");
    watcher.addProcess("cmd.exe");

    QObject::connect(&watcher, &QProcessWatcher::processStarted, &onProcessStarted);
    QObject::connect(&watcher, &QProcessWatcher::processFinished, &onProcessFinished);

    return a.exec();
}
```

This application shows information message when processes "MSPaint.exe" and "cmd.exe" starts or finished.


