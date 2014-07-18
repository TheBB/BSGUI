#include <QMainWindow>

#include "ObjectSet.h"

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~MainWindow();

private:
    ObjectSet *objectSet;
};

#endif /* _MAINWINDOW_H_ */
