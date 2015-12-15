//=============================================================================================================
/**
* @file     brainhemispheretreeitem.cpp
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     November, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    BrainHemisphereTreeItem class definition.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "brainhemispheretreeitem.h"


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace DISP3DNEWLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

BrainHemisphereTreeItem::BrainHemisphereTreeItem(const int& iType, const QString& text)
: AbstractTreeItem(iType, text)
{
}


//*************************************************************************************************************

BrainHemisphereTreeItem::~BrainHemisphereTreeItem()
{
}


//*************************************************************************************************************

QVariant BrainHemisphereTreeItem::data(int role) const
{
    switch(role) {
        case BrainHemisphereTreeItemRoles::SurfaceHemi:
            return QVariant();
    }

    return QStandardItem::data(role);
}


//*************************************************************************************************************

void  BrainHemisphereTreeItem::setData(const QVariant& value, int role)
{
    QStandardItem::setData(value, role);
}


//*************************************************************************************************************

bool BrainHemisphereTreeItem::addData(const Surface& tSurface, const Annotation& tAnnotation, Qt3DCore::QEntity *p3DEntityParent)
{
    //Set name of this item based on the hemispehre information
    switch (tSurface.hemi()) {
    case 0:
        this->setText("Left");
        break;
    case 1:
        this->setText("Right");
        break;
    default:
        this->setText("Unknown");
        break;
    }

    //Add childs
    bool state = false;

    //Add surface child
    BrainSurfaceTreeItem* pSurfaceItem = new BrainSurfaceTreeItem(BrainTreeModelItemTypes::SurfaceItem);
    *this<<pSurfaceItem;
    state = pSurfaceItem->addData(tSurface, p3DEntityParent);

    //Add annotation child
    if(!tAnnotation.isEmpty()) {
        BrainAnnotationTreeItem* pAnnotItem = new BrainAnnotationTreeItem(BrainTreeModelItemTypes::AnnotationItem);
        *this<<pAnnotItem;
        state = pAnnotItem->addData(tSurface, tAnnotation);
    }

    return state;
}
