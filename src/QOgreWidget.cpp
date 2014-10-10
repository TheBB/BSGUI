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

#include "QOgreWidget.h"

#include <QtX11Extras/QX11Info>

QOgreWidget::QOgreWidget(Ogre::Root *ogreRoot, QWidget *parent) : QGLWidget(parent)
                                                                , ogreRoot(ogreRoot)
{
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setFocusPolicy(Qt::StrongFocus);

    Ogre::String winHandle;
    winHandle =  Ogre::StringConverter::toString((unsigned long)QX11Info::display()) + ":";
    winHandle += Ogre::StringConverter::toString((unsigned int)QX11Info::appScreen()) + ":";
    winHandle += Ogre::StringConverter::toString((unsigned int)winId());

    Ogre::NameValuePairList params;
    params["parentWindowHandle"] = winHandle;
    params["FSAA"] = Ogre::String("8");

    ogreRenderWindow = ogreRoot->createRenderWindow("OgreWidget_RenderWindow",
                                                    qMax(width(), 640),
                                                    qMax(height(), 480),
                                                    false, &params);
    ogreRenderWindow->setActive(true);
    ogreRenderWindow->setVisible(true);

    WId ogreWinId = 0x0;
    ogreRenderWindow->getCustomAttribute("WINDOW", &ogreWinId);
    assert(ogreWinId);

    QWidget::create(ogreWinId);
    setAttribute(Qt::WA_OpaquePaintEvent);
}
