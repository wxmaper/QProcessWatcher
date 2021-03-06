# QProcessWatcher
Simple interface for monitoring state of process **for Windows**.

Use *[addProcess()](https://github.com/wxmaper/QProcessWatcher/blob/master/qprocesswatcher.h#L77)* method for add a process for monitoring by passing a case insensitive process name.
 
The signal *[processStarted()](https://github.com/wxmaper/QProcessWatcher/blob/master/qprocesswatcher.h#L103)* will be emitted in two cases:
1. If the monitored process is already started before calling addProcess() method.
2. When the first copy of process is started.
 
The signal *[processFinished()](https://github.com/wxmaper/QProcessWatcher/blob/master/qprocesswatcher.h#L109)* will be emitted when all copies of monitored process is closed.

You can define a PROCESS_WATCHER_DEBUG macro in project file for view a small debug information:
```qmake
DEFINES += PROCESS_WATCHER_DEBUG
```

## Usage
```c++
#include "qprocesswatcher.h"
...
QProcessWatcher watcher;
watcher.addProcess("MSPaint.exe");
```

## Examples

### Example 1

```c++
#include <QApplication>
#include <QWidget>
#include "qprocesswatcher.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create widget
    QWidget w;

    // Create watcher and add monitoring for "MSPaint.exe" process
    QProcessWatcher watcher;
    watcher.addProcess("MSPaint.exe");

    // Connect signals
    QObject::connect(&watcher, SIGNAL(processStarted(QString,QString)), &w, SLOT(show()));
    QObject::connect(&watcher, SIGNAL(processFinished(QString)), &w, SLOT(close()));

    return a.exec();
}
```

This windget-based application wait for starting "MSPaint.exe" process and then shows the Widget form.  
Widget has closed automatically when "MSPaint.exe" is closed.

### Example 2

```c++
#include <QApplication>
#include <QMessageBox>
#include "qprocesswatcher.h"

void onProcessStarted(const QString &processName,
                      const QString &originalProcessName)
{
    const QString title = QObject::tr("Process started");
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
    watcher.addProcess("CMD.EXE");

    QObject::connect(&watcher, &QProcessWatcher::processStarted, &onProcessStarted);
    QObject::connect(&watcher, &QProcessWatcher::processFinished, &onProcessFinished);

    return a.exec();
}
```

This application shows information message when processes "MSPaint.exe" and "CMD.EXE" starts or finished.

![cmd.exe](https://pp.userapi.com/c850728/v850728114/d2798/f_ZxaGbJDIE.jpg)


