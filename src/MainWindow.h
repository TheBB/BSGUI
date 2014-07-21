#include <QKeyEvent>
#include <QMainWindow>

#include "ObjectSet.h"
#include "GLWidget.h"
#include "ToolBox.h"

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    ObjectSet *objectSet;

    GLWidget *glWidget;
    ToolBox *toolBox;
};

#endif /* _MAINWINDOW_H_ */
