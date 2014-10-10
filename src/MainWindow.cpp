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

#include <algorithm>

#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "MainWindow.h"


MainWindow::MainWindow()
{
    ogreLogManager = new Ogre::LogManager();
    ogreLogManager->createLog("Ogre.log", true, false, false);
    ogreRoot = new Ogre::Root("ogre/plugins.cfg");

    setupResources();
    setupRenderSystem();

    createWidgets();
}


MainWindow::~MainWindow()
{
    delete ogreRoot;
}


void MainWindow::setupResources()
{
    Ogre::ConfigFile config;
    config.load("ogre/resources.cfg");

    auto seci = config.getSectionIterator();
    while (seci.hasMoreElements())
    {
        auto secName = seci.peekNextKey();
        auto settings = seci.getNext();
        for (auto i : *settings)
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(i.second, i.first, secName);
    }
}


void MainWindow::setupRenderSystem()
{
    auto renderers = ogreRoot->getAvailableRenderers();
    auto sys = std::find_if(renderers.begin(), renderers.end(),
                            [] (Ogre::RenderSystem *sys) {
                                return sys->getName() == "OpenGL Rendering Subsystem";
                            });

    if (sys == renderers.end())
        throw std::runtime_error("Unable to find OpenGL renderer");

    ogreRenderSystem = *sys;
    ogreRoot->setRenderSystem(ogreRenderSystem);
    ogreRenderWindow = ogreRoot->initialise(false);
}


void MainWindow::createWidgets()
{
    QGroupBox *mainGroup = new QGroupBox;
    setCentralWidget(mainGroup);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainGroup->setLayout(mainLayout);

    QPushButton *btnZoomIn = new QPushButton("Zoom in");
    QPushButton *btnZoomOut = new QPushButton("Zoom out");
    mainLayout->addWidget(btnZoomIn);
    mainLayout->addWidget(btnZoomOut);
    QObject::connect(btnZoomIn, &QPushButton::released, this, &MainWindow::onZoomIn);
    QObject::connect(btnZoomOut, &QPushButton::released, this, &MainWindow::onZoomOut);

    ogreWidget = new QOgreWidget(ogreRoot);
    mainLayout->addWidget(ogreWidget);
}


void MainWindow::onZoomIn()
{
    std::cout << "Zoom in" << std::endl;
}


void MainWindow::onZoomOut()
{
    std::cout << "Zoom out" << std::endl;
}
