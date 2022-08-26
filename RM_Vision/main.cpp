#include <QThread>
#include <QTimer>

#include "Common.h"
#include "QtSingleApplication.h"
#include "CameraProcessor.h"
#include "CommProcessor.h"
#include "UI/mainwindow.h"
#include "VisionProcessor.h"


int main(int argc, char *argv[])
{
    SharedTools::QtSingleApplication app((QLatin1String("RM Vision")), argc, argv);
    if (app.isRunning())
    {
        app.sendMessage("raies");
        return EXIT_SUCCESS;
    }
    CommonInit();
    qRegisterMetaType<VisionResult_t>("VisionResult_t");
    qRegisterMetaType<uint16_t>("uint16_t");


    CameraProcessor* camera = CameraProcessor::Instance();
    CommProcessor* comm = CommProcessor::Instance();
    VisionProcessor* vision = VisionProcessor::Instance();
    QThread* thread = new QThread;
    comm->moveToThread(thread);

    vision->getCameraObj(camera->m_objCamera);
    comm->getVisionObj(vision);

    thread->start(QThread::NormalPriority);
    vision->start(QThread::HighestPriority);

    MainWindow w;
    w.show();

    w.getCameraObj(camera->m_objCamera);
    w.getVisionObj(vision);




    return app.exec();
}

