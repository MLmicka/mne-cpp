//=============================================================================================================
/**
 * @file     main.cpp
 * @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
 *           Lorenz Esch <lesch@mgh.harvard.edu>
 * @since    0.1.0
 * @date     January, 2017
 *
 * @section  LICENSE
 *
 * Copyright (C) 2017, Christoph Dinh, Lorenz Esch. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 * the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @brief    Implements the mne_analyze GUI application.
 *
 */

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include <stdio.h>
#include <utils/generics/applicationlogger.h>

#include "info.h"
#include "analyzecore.h"

//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================

#include <QApplication>
#include <QFontDatabase>
#include <QtPlugin>
#include <QSurfaceFormat>

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNEANALYZE;

//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

//=============================================================================================================
// MAIN
//=============================================================================================================

#ifdef STATICBUILD
Q_IMPORT_PLUGIN(DataLoader)
Q_IMPORT_PLUGIN(DataManager)
Q_IMPORT_PLUGIN(RawDataViewer)
Q_IMPORT_PLUGIN(AnnotationManager)
#endif

//=============================================================================================================

AnalyzeCore *pAnalyzeCore;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set default font
    int id = QFontDatabase::addApplicationFont(":/fonts/Roboto-Light.ttf");
    app.setFont(QFont(QFontDatabase::applicationFontFamilies(id).at(0)));

    //set application settings
    QCoreApplication::setOrganizationName(CInfo::OrganizationName());
    QCoreApplication::setApplicationName(CInfo::AppNameShort());

    QSurfaceFormat fmt;
    fmt.setSamples(4);
    QSurfaceFormat::setDefaultFormat(fmt);

    //New main window instance
    pAnalyzeCore = new AnalyzeCore();
    pAnalyzeCore->showMainWindow();

    return app.exec();
}
