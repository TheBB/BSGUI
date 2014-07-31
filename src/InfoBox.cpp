#include <iostream>
#include <QColor>
#include <QDateTime>
#include <QFont>
#include <QTabWidget>

#include "InfoBox.h"

InfoBox::InfoBox(ObjectSet *objectSet, QWidget *parent)
    : QTabWidget(parent)
    , _objectSet(objectSet)
{
    installEventFilter(parent);

    addTab(new QWidget(), "Selection");
    addTab(new QWidget(), "Pick");
    addTab(new QWidget(), "Files");

    logBox = new QTextEdit();
    logBox->setReadOnly(true);
    logBox->setFont(QFont("Source Code Pro, monospace", 10));
    addTab(logBox, "Log");

    setCurrentIndex(3);

    QObject::connect(objectSet, &ObjectSet::log, this, &InfoBox::log);
}


void InfoBox::log(QString text, LogLevel level)
{
    QString logString = "<span style=\"color:#a9a9a9;\">[%1]</span> %2 %4 %3";

    logString = logString.arg(QDateTime::currentDateTime().toString("HH:mm:ss"));

    switch (level)
    {
    case LL_NORMAL:
        logString = logString.arg("", "");;
        break;

    case LL_WARNING:
        logString = logString.arg("<span style=\"color:#ffa500; font-weight: bold;\">WARNING</span>", "");
        break;

    case LL_ERROR:
        logString = logString.arg("<span style=\"color:#ff4500; font-weight: bold;\">ERROR</span>", "");
        std::cerr << "ERROR: " << text.toStdString() << std::endl;
        break;

    case LL_FATAL:
        logString = logString.arg("<span style=\"color:#ff0000; font-weight: bold;\">FATAL!", "</span>");
        std::cerr << "FATAL ERROR: " << text.toStdString() << std::endl;
        break;
    }

    logString = logString.arg(text);

    logBox->append(logString);
}
