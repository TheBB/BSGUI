#include <QTabWidget>
#include <QTextEdit>

#include "ObjectSet.h"

#ifndef INFOBOX_H
#define INFOBOX_H

class InfoBox : public QTabWidget
{
    Q_OBJECT

public:
    InfoBox(ObjectSet *objectSet, QWidget *parent = NULL);
    ~InfoBox() { }

public slots:
    void log(QString, LogLevel);

private:
    ObjectSet *_objectSet;

    QTextEdit *logBox;
};

#endif /* INFOBOX_H */
