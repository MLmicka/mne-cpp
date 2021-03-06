//=============================================================================================================
/**
 * @file     rawdataviewer.cpp
 * @author   Lorenz Esch <lesch@mgh.harvard.edu>;
 *           Lars Debor <Lars.Debor@tu-ilmenau.de>;
 *           Simon Heinke <Simon.Heinke@tu-ilmenau.de>
 * @since    0.1.0
 * @date     October, 2018
 *
 * @section  LICENSE
 *
 * Copyright (C) 2018, Lorenz Esch, Lars Debor, Simon Heinke. All rights reserved.
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
 * @brief    Definition of the RawDataViewer class.
 *
 */

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "rawdataviewer.h"
#include "fiffrawview.h"
#include "fiffrawviewdelegate.h"
#include "../dataloader/dataloader.h"

#include <anShared/Management/analyzedata.h>
#include <anShared/Utils/metatypes.h>
#include <anShared/Management/communicator.h>
#include <anShared/Model/annotationmodel.h>

#include <disp/viewers/fiffrawviewsettings.h>
#include <disp/viewers/scalingview.h>

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QSpacerItem>

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace RAWDATAVIEWERPLUGIN;
using namespace ANSHAREDLIB;
using namespace DISPLIB;

//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

RawDataViewer::RawDataViewer()
: m_iVisibleBlocks(10)
, m_iBufferBlocks(10)
{
}

//=============================================================================================================

RawDataViewer::~RawDataViewer()
{
}

//=============================================================================================================

QSharedPointer<IPlugin> RawDataViewer::clone() const
{
    QSharedPointer<RawDataViewer> pRawDataViewerClone = QSharedPointer<RawDataViewer>::create();
    return pRawDataViewerClone;
}

//=============================================================================================================

void RawDataViewer::init()
{
    m_pCommu = new Communicator(this);

    // Create viewer
    m_pFiffRawView = new FiffRawView();
    m_pFiffRawView->setMinimumSize(256, 256);
    m_pFiffRawView->setFocusPolicy(Qt::TabFocus);
    m_pFiffRawView->setAttribute(Qt::WA_DeleteOnClose, false);
}

//=============================================================================================================

void RawDataViewer::unload()
{
}

//=============================================================================================================

QString RawDataViewer::getName() const
{
    return "Signal Viewer";
}

//=============================================================================================================

QMenu *RawDataViewer::getMenu()
{
    return Q_NULLPTR;
}

//=============================================================================================================

QDockWidget *RawDataViewer::getControl()
{
    QDockWidget* pControlDock = new QDockWidget(getName());
    pControlDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    pControlDock->setObjectName("Data View Controls");

    QVBoxLayout* pLayout = new QVBoxLayout;

    //Scaling Widget
    m_pScalingWidget = new ScalingView("mne_analyze/SignalViewer/Scaling");
    pLayout->addWidget(m_pScalingWidget);

    //View Widget
    m_pSettingsViewWidget = new FiffRawViewSettings("mne_analyze/SignalViewer/ViewSettings");
    m_pSettingsViewWidget->setWidgetList();
    pLayout->addWidget(m_pSettingsViewWidget);

    QWidget* pWidget = new QWidget();
    pWidget->setLayout(pLayout);
    pControlDock->setWidget(pWidget);

    return pControlDock;
}

//=============================================================================================================

QWidget *RawDataViewer::getView()
{
    return m_pFiffRawView;
}

//=============================================================================================================

void RawDataViewer::handleEvent(QSharedPointer<Event> e)
{
    switch (e->getType()) {
    case TRIGGER_REDRAW:
        m_pFiffRawView->updateView();
        break;
    case TRIGGER_VIEWER_MOVE:
        m_pFiffRawView->updateScrollPosition();
        break;
    case SELECTED_MODEL_CHANGED:
        onModelChanged(e->getData().value<QSharedPointer<ANSHAREDLIB::AbstractModel> >());
        break;
    default:
        qWarning() << "[RawDataViewer::handleEvent] Received an Event that is not handled by switch cases.";
    }
}

//=============================================================================================================

QVector<EVENT_TYPE> RawDataViewer::getEventSubscriptions(void) const
{
    QVector<EVENT_TYPE> temp = {};
    temp.push_back(TRIGGER_REDRAW);
    temp.push_back(TRIGGER_VIEWER_MOVE);
    temp.push_back(SELECTED_MODEL_CHANGED);

    return temp;
}

//=============================================================================================================

void RawDataViewer::onModelChanged(QSharedPointer<AbstractModel> pNewModel)
{
    if(pNewModel->getType() == MODEL_TYPE::ANSHAREDLIB_FIFFRAW_MODEL) {
        if(m_pFiffRawView->getModel() == pNewModel) {
            return;
        }

        if(!m_pFiffRawView->getDelegate()) {
            m_pFiffRawView->setDelegate(QSharedPointer<FiffRawViewDelegate>::create());
        }

        m_pFiffRawView->setModel(qSharedPointerCast<FiffRawViewModel>(pNewModel));

        updateControls();
    }
}

//=============================================================================================================

void RawDataViewer::updateControls()
{
    if(m_pScalingWidget && m_pSettingsViewWidget) {
        // Setup scaling widget
        connect(m_pScalingWidget.data(), &DISPLIB::ScalingView::scalingChanged,
                m_pFiffRawView.data(), &FiffRawView::setScalingMap);

        // Setup view settings widget
        connect(m_pSettingsViewWidget.data(), &DISPLIB::FiffRawViewSettings::signalColorChanged,
                m_pFiffRawView.data(), &FiffRawView::setSignalColor);
        connect(m_pSettingsViewWidget.data(), &DISPLIB::FiffRawViewSettings::backgroundColorChanged,
                m_pFiffRawView.data(), &FiffRawView::setBackgroundColor);
        connect(m_pSettingsViewWidget.data(), &DISPLIB::FiffRawViewSettings::zoomChanged,
                m_pFiffRawView.data(), &FiffRawView::setZoom);
        connect(m_pSettingsViewWidget.data(), &DISPLIB::FiffRawViewSettings::timeWindowChanged,
                m_pFiffRawView.data(), &FiffRawView::setWindowSize);
        connect(m_pSettingsViewWidget.data(), &DISPLIB::FiffRawViewSettings::distanceTimeSpacerChanged,
                m_pFiffRawView.data(), &FiffRawView::setDistanceTimeSpacer);
        connect(m_pSettingsViewWidget.data(), &DISPLIB::FiffRawViewSettings::makeScreenshot,
                m_pFiffRawView.data(), &FiffRawView::onMakeScreenshot);

        // Preserve settings between different file sessions
        m_pFiffRawView->setWindowSize(m_pSettingsViewWidget->getWindowSize());
        m_pFiffRawView->setSignalColor(m_pSettingsViewWidget->getSignalColor());
        m_pFiffRawView->setBackgroundColor(m_pSettingsViewWidget->getBackgroundColor());
        m_pFiffRawView->setZoom(m_pSettingsViewWidget->getZoom());
        m_pFiffRawView->setDistanceTimeSpacer(m_pSettingsViewWidget->getDistanceTimeSpacer());

        connect(m_pFiffRawView.data(), &FiffRawView::sendSamplePos,
                this, &RawDataViewer::onSendSamplePos);
    }
}

//=============================================================================================================

void RawDataViewer::onSendSamplePos(int iSample)
{
    QVariant data;
    data.setValue(iSample);

    m_pCommu->publishEvent(EVENT_TYPE::NEW_ANNOTATION_ADDED, data);
}
