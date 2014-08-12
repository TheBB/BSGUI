/*
 * Copyright (C) 2014 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information:
 * E-mail: eivind.fonn@sintef.no
 * SINTEF ICT, Department of Applied Mathematics,
 * P.O. Box 4760 Sluppen,
 * 7045 Trondheim, Norway.
 *
 * This file is part of BSGUI.
 *
 * BSGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * BSGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with GoTools. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using GoTools.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the GoTools library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT.
 */

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
