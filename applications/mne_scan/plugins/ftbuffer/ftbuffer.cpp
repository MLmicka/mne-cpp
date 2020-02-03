/**
* @file     ftbuffer.cpp
* @author   Gabriel B Motta <gbmotta@mgh.harvard.edu>;
*           Lorenz Esch <lorenz.esch@tu-ilmenau.de>
*           Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     January, 2020
*
* @section  LICENSE
*
* Copyright (C) 2020, Christoph Dinh and Matti Hamalainen. All rights reserved.
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
* @brief    Definition of the FtBuffer class.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "ftbuffer.h"
#include "ftbuffproducer.h"

//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace SCSHAREDLIB;
using namespace FTBUFFERPLUGIN;
using namespace SCMEASLIB;

//*************************************************************************************************************

FtBuffer::FtBuffer()
: m_bIsRunning(false)
, m_pFtBuffProducer(QSharedPointer<FtBuffProducer>::create(this))
, m_pFiffInfo(QSharedPointer<FiffInfo>::create())
, m_pRTMSA_BufferOutput(PluginOutputData<RealTimeMultiSampleArray>::create(this, "FtBuffer", "Output data"))
, m_bBuffOutputSet(false)
, m_bCustomFiff(false)
{
    qDebug() << "Constructing FtBuffer Object";

    m_pActionShowYourWidget = new QAction(QIcon(":/images/options.png"), tr("FieldTrip Buffer Widget"),this);
    m_pActionShowYourWidget->setShortcut(tr("F12"));
    m_pActionShowYourWidget->setStatusTip(tr("FieldTrip Buffer Widget"));

    connect(m_pActionShowYourWidget, &QAction::triggered, this, &FtBuffer::showYourWidget);

    addPluginAction(m_pActionShowYourWidget);
}

//*************************************************************************************************************

FtBuffer::~FtBuffer() {
    if(this->isRunning()){
        stop();
    }
}

//*************************************************************************************************************

QSharedPointer<IPlugin> FtBuffer::clone() const {
    QSharedPointer<FtBuffer> pFtBufferClone(new FtBuffer);
    return pFtBufferClone;
}

//*************************************************************************************************************

void FtBuffer::init() {
    qDebug() << "Initializing FtBuffer plugin...";
    m_outputConnectors.append(m_pRTMSA_BufferOutput);
}

//*************************************************************************************************************

void FtBuffer::unload() {}

//*************************************************************************************************************

bool FtBuffer::start() {

    qDebug() << "Starting FtBuffer...";
    m_bIsRunning = true;


    QThread::start();

    qRegisterMetaType<QPair<int,float>>("QPair<int,float>");
    qRegisterMetaType<SimpleStorage>("SimpleStorage");

    // FtProducer in it's own thread and connect communications signals/slots
    m_pFtBuffProducer->moveToThread(&m_pProducerThread);
    connect(m_pFtBuffProducer.data(), &FtBuffProducer::newDataAvailable, this, &FtBuffer::onNewDataAvailable, Qt::DirectConnection);
    connect(m_pFtBuffProducer.data(), &FtBuffProducer::extendedHeaderChunks, this, &FtBuffer::parseHeader);
    connect(this, &FtBuffer::workCommand, m_pFtBuffProducer.data(),&FtBuffProducer::doWork);
    connect(m_pFtBuffProducer.data(), &FtBuffProducer::bufferParameters, this, &FtBuffer::setParams, Qt::DirectConnection);
    m_pProducerThread.start();

    qDebug() << "Producer thread created, sending work command...";
    emit workCommand();

    return true;
}

//*************************************************************************************************************

bool FtBuffer::stop() {

    qDebug() << "Stopping...";

    m_mutex.lock();
    m_bIsRunning = false;
    m_mutex.unlock();

    m_pRTMSA_BufferOutput->data()->clear();

    //stops separate producer/client thread
    m_pProducerThread.quit();
    m_pProducerThread.wait();


    //Reset ftproducer and sample received list
    m_pFtBuffProducer.clear();
    m_pFtBuffProducer = QSharedPointer<FtBuffProducer>::create(this);

    return true;
}

//*************************************************************************************************************

IPlugin::PluginType FtBuffer::getType() const {
    return _ISensor;
}

//*************************************************************************************************************

QString FtBuffer::getName() const {
    return "FtBuffer";
}

//*************************************************************************************************************

QWidget* FtBuffer::setupWidget() {
    FtBufferSetupWidget* setupWidget = new FtBufferSetupWidget(this);
    return setupWidget;
}

//*************************************************************************************************************

void FtBuffer::run() {}

//*************************************************************************************************************

void FtBuffer::showYourWidget() {
    m_pYourWidget = FtBufferYourWidget::SPtr(new FtBufferYourWidget());
    m_pYourWidget->show();
}

//*************************************************************************************************************

void FtBuffer::setUpFiffInfo()
{
    //
    //Clear old fiff info data
    //
    m_pFiffInfo->clear();


    if (m_bCustomFiff) {
        //
        //Set number of channels, sampling frequency and high/-lowpass
        //
        qDebug() << "Custom Fiff";
        m_pFiffInfo->nchan = m_iNumChannels;
        m_pFiffInfo->sfreq = m_iSampFreq;
        m_pFiffInfo->highpass = 0.001f;
        m_pFiffInfo->lowpass = m_iSampFreq/2;
    } else {
        //
        //Set number of channels, sampling frequency and high/-lowpass
        //
        qDebug() << "Default Fiff";
        m_pFiffInfo->nchan = m_iNumChannels;
        m_pFiffInfo->sfreq = m_iSampFreq;
        m_pFiffInfo->highpass = 0.001f;
        m_pFiffInfo->lowpass = m_iSampFreq/2;
    }

    //
    //Set up the channel info
    //
    QStringList QSLChNames;
    m_pFiffInfo->chs.clear();

    for(int i = 0; i < m_pFiffInfo->nchan; ++i)
    {
        //Create information for each channel
        QString sChType;
        FiffChInfo fChInfo;

//        //EEG Channels
//        if(i <= m_pFiffInfo->nchan-2)
//        {
            //Set channel name
            sChType = QString("CH. ");
            if(i<10) {
                sChType.append("00");
            }

            if(i>=10 && i<100) {
                sChType.append("0");
            }

            fChInfo.ch_name = sChType.append(sChType.number(i));

            //Set channel type
            fChInfo.kind = FIFFV_EEG_CH;

            //Set logno
            fChInfo.logNo = i;

            //Set coord frame
            fChInfo.coord_frame = FIFFV_COORD_HEAD;

            //Set unit
            fChInfo.unit = FIFF_UNIT_V;

            //Set EEG electrode location - Convert from mm to m
            fChInfo.eeg_loc(0,0) = 0;
            fChInfo.eeg_loc(1,0) = 0;
            fChInfo.eeg_loc(2,0) = 0;

            //Set EEG electrode direction - Convert from mm to m
            fChInfo.eeg_loc(0,1) = 0;
            fChInfo.eeg_loc(1,1) = 0;
            fChInfo.eeg_loc(2,1) = 0;

            //Also write the eeg electrode locations into the meg loc variable (mne_ex_read_raw() matlab function wants this)
            fChInfo.chpos.r0(0) = 0;
            fChInfo.chpos.r0(1) = 0;
            fChInfo.chpos.r0(2) = 0;

            fChInfo.chpos.ex(0) = 1;
            fChInfo.chpos.ex(1) = 0;
            fChInfo.chpos.ex(2) = 0;

            fChInfo.chpos.ey(0) = 0;
            fChInfo.chpos.ey(1) = 1;
            fChInfo.chpos.ey(2) = 0;

            fChInfo.chpos.ez(0) = 0;
            fChInfo.chpos.ez(1) = 0;
            fChInfo.chpos.ez(2) = 1;
//        }

//        //Digital input channel
//        if(i == m_pFiffInfo->nchan-1)
//        {
//            //Set channel type
//            fChInfo.kind = FIFFV_STIM_CH;

//            sChType = QString("STIM");
//            fChInfo.ch_name = sChType;
//        }

        QSLChNames << sChType;

        m_pFiffInfo->chs.append(fChInfo);
    }

    //Set channel names in fiff_info_base
    m_pFiffInfo->ch_names = QSLChNames;

    //
    //Set head projection
    //
    m_pFiffInfo->dev_head_t.from = FIFFV_COORD_DEVICE;
    m_pFiffInfo->dev_head_t.to = FIFFV_COORD_HEAD;
    m_pFiffInfo->ctf_head_t.from = FIFFV_COORD_DEVICE;
    m_pFiffInfo->ctf_head_t.to = FIFFV_COORD_HEAD;
}

//*************************************************************************************************************

bool FtBuffer::isRunning() {
    return m_bIsRunning;
}

//*************************************************************************************************************

void FtBuffer::onNewDataAvailable(const Eigen::MatrixXd &matData) {
    qDebug() << "Appending matrix";
    m_mutex.lock();
    if(m_bIsRunning) {
        if (!m_bBuffOutputSet) {
            qDebug() << "Setting up buffer output";
            setupRTMSA();
            m_bBuffOutputSet = true;
        }
        m_pRTMSA_BufferOutput->data()->setValue(matData);
    }
    m_mutex.unlock();
    qDebug() << "Done.";

}

//*************************************************************************************************************

void FtBuffer::setParams(QPair<int,float> val) {
    m_iNumChannels = val.first;
    m_iSampFreq = val.second;
    qDebug() << "Parameters set";
}

//*************************************************************************************************************

void FtBuffer::setupRTMSA() {
    //Setup fiff info before setting up the RMTSA because we need it to init the RTMSA
    qDebug() << "Setting up RTMSA...";
    if(!m_bCustomFiff){
        qDebug() << "Default Fiff:";
        setUpFiffInfo();
    }

    //Set the channel size of the RMTSA
    m_pRTMSA_BufferOutput->data()->initFromFiffInfo(m_pFiffInfo);
    m_pRTMSA_BufferOutput->data()->setMultiArraySize(1);
}

//*************************************************************************************************************

void FtBuffer::parseHeader(QBuffer* chunkData) {

    qDebug() << "Chunk data received";
    qDebug() << "";


    char ch;
    chunkData->open(QBuffer::ReadWrite);
    chunkData->seek(0);
    qDebug() << "test read:" << chunkData->getChar(&ch) << ch
                             << chunkData->getChar(&ch) << ch
                             << chunkData->getChar(&ch) << ch;

    chunkData->close();

    m_pNeuromagHeadChunkData = QSharedPointer<FIFFLIB::FiffRawData>(new FiffRawData(*chunkData));

}

//*************************************************************************************************************

//void FtBuffer::updateBufferAddress(QString address) {}

//*************************************************************************************************************

//bool FtBuffer::connectToBuffer(QString addr){
//    //this->m_FtBuffClient.setAddress(addr);
//    //updateBufferAddress(addr);

//    m_pTempAddress = new char[(addr.toLocal8Bit().size()) + 1];
//    strcpy(m_pTempAddress, addr.toLocal8Bit().constData());

//    delete m_pFtBuffClient;

//    m_pFtBuffClient = new FtBuffClient(m_pTempAddress);
//    return this->m_pFtBuffClient->startConnection();
//}

//*************************************************************************************************************

//bool FtBuffer::disconnectFromBuffer(){
//    return this->m_pFtBuffClient->stopConnection();
//}

//*************************************************************************************************************

//Eigen::MatrixXd FtBuffer::getData() {
//    qDebug() << "FtBuffer::getData()";


//    //int i = 0;
//    while(m_bIsRunning) {

//        //pushData();

//        //qDebug() << "Loop" << i;
//        m_pFtBuffClient->getData();



//        if (m_pFtBuffClient->newData()) {
//            m_pFtBuffClient->reset();
//            qDebug() << "Returning mat";
//            return m_pFtBuffClient->dataMat();
//        }

//        //i++;
//    }
//}

//*************************************************************************************************************

//void FtBuffer::pushData(){
//    qDebug() <<  "pushData()";
//    m_mutex.lock();
//    qDebug() << "mutex locked";
//    if(!m_pListReceivedSamples->isEmpty()) {
//        qDebug() << "In loop";
//        MatrixXd matData = m_pListReceivedSamples->takeFirst();
//        qDebug() << matData.size();
//        m_pRTMSA_BufferOutput->data()->setValue(matData);
//        qDebug() << "value set";
//    }
//    m_mutex.unlock();
//    qDebug() << "mutex unlocked";
//}

//*************************************************************************************************************

//void FtBuffer::run() {
//    qDebug() << "Ft Buffer run()";
//    emit workCommand();
//    while (m_bIsRunning) {

//    }
//    while(m_bIsRunning) {
//        m_mutex.lock();
//        if(m_bIsRunning){break;}

//        qDebug() << "FtBuffer run() loop running";
//        pushData();
//        QThread::usleep(100);
//    }
//}

//*************************************************************************************************************

//void FtBuffer::parseHeader(SimpleStorage chunkData) {
//    //code to parse header chunks
//    m_bCustomFiff = true;

//    uint32_t *iData = (uint32_t *) chunkData.data();

//    qDebug() << "Header Type:" << iData[0];

//    //In Fieldtrip, FT_CHUNK_NEUROMAG_HEADER = 8
//    if (iData[0] == 8) {
//        qDebug() << "Size:" << iData[1];

//        char* cData_NEUROMAG_HEADER = new char[iData[1]];
//        char* cData_NEUROMAG_ISOTRAK = new char[*((iData + 3 * sizeof(uint32_t)) + (sizeof (char) * iData[1]))]; //2* size of uint32 to account for iData[0] and iData[1], plus size of fiff file, plus 1* size of uint32 to account for 'type' field of isotrak header

//        //extract the data from the associated fiff files, based on type and size of chunk headers, formatted to be made into a QBuffer
//        memcpy(cData_NEUROMAG_HEADER,
//               iData + 2 * sizeof(uint32_t),
//               sizeof(char) * iData[1]);
//        memcpy(cData_NEUROMAG_ISOTRAK,
//               iData + 4 * sizeof(uint32_t) + sizeof (char) * iData[1],
//               *((iData + 3 * sizeof(uint32_t)) + (sizeof (char) * iData[1])) * sizeof (char));

//        QBuffer bData_NEUROMAG_HEADER;
//        QBuffer bData_NEUROMAG_ISOTRAK;


//        bData_NEUROMAG_HEADER.setData(cData_NEUROMAG_HEADER, iData[1]);

////         = new FiffRawData(bData_NEUROMAG_HEADER);

//    } else {
//        qDebug() << "Unable to recognize header type.";
//    }

//}

//other version of parseHeader

//qDebug() << "Hey, we got here :)";
//const ft_chunk_t* chanNames = find_chunk(chunkData.data(), 0, chunkData.size(), FT_CHUNK_CHANNEL_NAMES);
//const ft_chunk_t* neuromagHead = find_chunk(chunkData.data(), 0, chunkData.size(), FT_CHUNK_NEUROMAG_HEADER);
////    const ft_chunk_t* neuromagIso = find_chunk(chunkData.data(), 0, chunkData.size(), FT_CHUNK_NEUROMAG_ISOTRAK);
////    const ft_chunk_t* neuromagHPI = find_chunk(chunkData.data(), 0, chunkData.size(), FT_CHUNK_NEUROMAG_HPIRESULT);

//QBuffer bData_CHANNEL_NAMES;
//QBuffer bData_NEUROMAG_HEADER;
////    QBuffer bData_NEUROMAG_ISOTRAK;
////    QBuffer bData_NEUROMAG_HPIRESULT;


//if (chanNames != Q_NULLPTR) {
//    qDebug() << "Channel name chunk found, size" << chanNames->def.size;
//    bData_CHANNEL_NAMES.setData(chanNames->data, chanNames->def.size);

//    m_bCustomFiff = true;
//}

//if (neuromagHead != Q_NULLPTR) {
//    qDebug () << "Neuromag header found, size" << neuromagHead->def.size;
//    bData_NEUROMAG_HEADER.setData(neuromagHead->data, neuromagHead->def.size);

//    m_bCustomFiff = true;
//}

////    if (neuromagIso != Q_NULLPTR) {
////        qDebug () << "Neuromag Isotrak found, size" << neuromagIso->def.size;
////    }

////    if (neuromagHPI != Q_NULLPTR) {
////        qDebug () << "Neuromag HPI found, size" << neuromagHPI->def.size;
////    }
