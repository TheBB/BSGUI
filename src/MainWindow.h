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

    inline ObjectSet *objectSet() { return _objectSet; }
    inline GLWidget *glWidget() { return _glWidget; }
    inline ToolBox *toolBox() { return _toolBox; }

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    ObjectSet *_objectSet;
    GLWidget *_glWidget;
    ToolBox *_toolBox;
};

#endif /* _MAINWINDOW_H_ */
