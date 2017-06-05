//=============================================================================================================
/**
* @file     subjecttreeitem.cpp
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     May, 2016
*
* @section  LICENSE
*
* Copyright (C) 2016, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    SubjectTreeItem class definition.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "subjecttreeitem.h"
#include "../measurement/measurementtreeitem.h"
#include "../mri/mritreeitem.h"
#include "../bem/bemsurfacetreeitem.h"

//*************************************************************************************************************
//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================

#include <Eigen/Core>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace FSLIB;
using namespace DISP3DLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

SubjectTreeItem::SubjectTreeItem(int iType, const QString& text)
: AbstractTreeItem(iType, text)
{
    initItem();
}


//*************************************************************************************************************

void SubjectTreeItem::initItem()
{
    this->setEditable(false);
    this->setCheckable(true);
    this->setCheckState(Qt::Checked);
    this->setToolTip("Subject");
}


//*************************************************************************************************************

void SubjectTreeItem::connectMeasurementToMriItems(MeasurementTreeItem* pMeasurementItem)
{
    //Connect mri item with the measurement tree items in case the real time color changes (i.e. rt source loc)
    //or the user changes the color origin
    QList<QStandardItem*> mriItemList = this->findChildren(Data3DTreeModelItemTypes::MriItem);

    for(int i = 0; i < mriItemList.size(); ++i) {
        if(MriTreeItem* pMriItem = dynamic_cast<MriTreeItem*>(mriItemList.at(i))) {
            connect(pMeasurementItem, &MeasurementTreeItem::vertColorChanged,
                pMriItem, &MriTreeItem::setRtVertColor);

            connect(pMriItem, &MriTreeItem::colorOriginChanged,
                pMeasurementItem, &MeasurementTreeItem::setColorOrigin);
        }
    }
}


//*************************************************************************************************************

void SubjectTreeItem::connectMeasurementToBemHeadItems(MeasurementTreeItem* pMeasurementItem)
{
    //Connect bem head item with the measurement tree items in case the real time color changes (i.e. rt source loc)
    QList<QStandardItem*> bemItemList = this->findChildren(Data3DTreeModelItemTypes::BemItem);
    QList<QStandardItem*> bemSurfacesItemList;

    for(int i = 0; i < bemItemList.size(); ++i) {
        if(MriTreeItem* pMriItem = dynamic_cast<MriTreeItem*>(bemItemList.at(i))) {
            bemSurfacesItemList = pMriItem->findChildren(Data3DTreeModelItemTypes::BemSurfaceItem);

            for(int k = 0; k < bemSurfacesItemList.size(); ++k) {
                if(BemSurfaceTreeItem* pBemSurfItem = dynamic_cast<BemSurfaceTreeItem*>(bemSurfacesItemList.at(i))) {
                    if(pBemSurfItem->text() == "Head") {
                        connect(pMeasurementItem, &MeasurementTreeItem::vertColorChanged,
                            pBemSurfItem, &BemSurfaceTreeItem::setVertColor);
                    }
                }
            }
        }
    }
}


//*************************************************************************************************************

void SubjectTreeItem::connectMeasurementToSensorItems(MeasurementTreeItem* pMeasurementItem)
{
    //Connect bem sensor surface item with the measurement tree items in case the real time color changes (i.e. rt source loc)
    QList<QStandardItem*> bemItemList = this->findChildren(Data3DTreeModelItemTypes::BemItem);
    QList<QStandardItem*> bemSurfacesItemList;

    for(int i = 0; i < bemItemList.size(); ++i) {
        if(MriTreeItem* pMriItem = dynamic_cast<MriTreeItem*>(bemItemList.at(i))) {
            bemSurfacesItemList = pMriItem->findChildren(Data3DTreeModelItemTypes::BemSurfaceItem);

            for(int k = 0; k < bemSurfacesItemList.size(); ++k) {
                if(BemSurfaceTreeItem* pBemSurfItem = dynamic_cast<BemSurfaceTreeItem*>(bemSurfacesItemList.at(i))) {
                    if(pBemSurfItem->text() == "Sensor Surface") {
                        connect(pMeasurementItem, &MeasurementTreeItem::vertColorChanged,
                            pBemSurfItem, &BemSurfaceTreeItem::setVertColor);
                    }
                }
            }
        }
    }
}
