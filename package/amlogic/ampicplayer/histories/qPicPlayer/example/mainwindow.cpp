#include "mainwindow.h"
#include "picture_player_interface.h"
#include "qexifimageheader.h"

MainWindow::MainWindow(int screen_width, int screen_height, QWidget *parent)
    : QMainWindow(parent)
{
#ifdef USE_SYSTEM_DBUS
    picture_player = new PicturePlayerInterface("com.amlogic.player", "/Picture",
                           QDBusConnection::systemBus(), this);
#else
    picture_player = new PicturePlayerInterface("com.amlogic.player", "/Picture",
                           QDBusConnection::sessionBus(), this);
#endif

    if (picture_player->isValid()){
        qDebug() << QString("Connected");
        openDirectory();
        startTimer(10000); // slideshow duration 10 seconds
    }
    else{
        qDebug() << QString("Not connected");
    }
	
	picture_player->setDesktop(screen_width,screen_height); /* set display rect for picture. */
    scene_width = screen_width;
    scene_height = screen_height;
    degree=0;
    scale=0;
    prepared=0;
    last_prepared=0;
}
/* test code for single mode. */
#if 0
void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    hide();
    if (degree==90){
        picture_player->adjustDisplayScene(270, 100.0/last_ratio, 100.0/last_ratio, scene_width/2, scene_height/2, "");
        degree=0;
        return;
    }
    if (scale==2){
        picture_player->adjustDisplayScene(0, 50, 50, scene_width/2, scene_height/2, "");
        scale = 0;
        return;
    }
    /*(if (prepared){
        picture_player->displayScene(0, "effect_blend",0);
        last_prepared = prepared;
        prepared = 0;
    }*/
    picture_player->createScene(0, scene_width, 0, scene_width, scene_height);
    //if (qrand()%2){
    if(1) {
        QExifImageHeader exif(photos[index]);
        QImageReader image;
        image.setFileName(photos[index]);
        qDebug() << "Single Picture";
        qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
        int rotation_degree = qrand()%7-3;
        if (rotation_degree < 0)
            rotation_degree += 360;
        picture_player->createPicture(0, photos[index], 0, 0, scene_width, scene_height, QString("rotation = ") + QString::number(rotation_degree));
        //qDebug()<<"start display\n";
        while(picture_player->ScenceStatus(0)==0) {
            qDebug()<<"Wait for display\n";
        }
        picture_player->displayScene(0, "effect_over",0);
        index++;
        if (index==photos.count())
            index = 0;
        prepared=1;
        last_ratio = (qreal)image.size().height()/image.size().width();
    }
    else
    {
        if (qrand()%2){
            int look_ahead_index = index;
            int vertical_flag[4] = {0,0,0,0};
            for (int i=0;i<4;i++){
                QImageReader image;
                image.setFileName(photos[look_ahead_index]);
                if (image.size().height()>image.size().width()){
                    vertical_flag[i]=1;
                }
                look_ahead_index++;
                if (look_ahead_index==photos.count())
                    look_ahead_index = 0;
            }
            if (vertical_flag[0]&&vertical_flag[1]){
                qDebug() << "Two Vertical Picture";
                for (int j=0;j<2;j++){
                    QExifImageHeader exif(photos[index]);
                    QImageReader image;
                    int rotation_degree = qrand()%7-3;
                    if (rotation_degree < 0)
                        rotation_degree += 360;
                    image.setFileName(photos[index]);
                    qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                    picture_player->createPicture(0, photos[index], j*scene_width/2, 0, scene_width/2, scene_height, QString("rotation = ") + QString::number(rotation_degree));
                    index++;
                    if (index==photos.count())
                        index = 0;
                }
                prepared=2;
            }
            else if (vertical_flag[0]+vertical_flag[1]+vertical_flag[2]==1){
                int horizontal_flag = 0;
                qDebug() << "Two horizontal and One Vertical Picture";
                for (int j=0;j<3;j++){
                    QExifImageHeader exif(photos[index]);
                    QImageReader image;
                    int rotation_degree = qrand()%7-3;
                    if (rotation_degree < 0)
                        rotation_degree += 360;
                    image.setFileName(photos[index]);
                    qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                    if (vertical_flag[j]){
                        picture_player->createPicture(0, photos[index], scene_width/2, 0, scene_width/2, scene_height, QString("rotation = ") + QString::number(rotation_degree));
                    }
                    else{
                        if (horizontal_flag){
                            picture_player->createPicture(0, photos[index], 0, scene_height/2, scene_width/2, scene_height/2, QString("rotation = ") + QString::number(rotation_degree));
                        }
                        else{
                            picture_player->createPicture(0, photos[index], 0, 0, scene_width/2, scene_height/2, QString("rotation = ") + QString::number(rotation_degree));
                            horizontal_flag = 1;
                        }
                    }
                    index++;
                    if (index==photos.count())
                        index = 0;
                }
                prepared=3;
            }
            else if (vertical_flag[0]+vertical_flag[1]+vertical_flag[2]+vertical_flag[3]==0){
                qDebug() << "Four Horizontal Picture";
                for (int i=0;i<2;i++){
                    for (int j=0;j<2;j++){
                        QExifImageHeader exif(photos[index]);
                        QImageReader image;
                        int rotation_degree = qrand()%7-3;
                        if (rotation_degree < 0)
                            rotation_degree += 360;
                        image.setFileName(photos[index]);
                        qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                        picture_player->createPicture(0, photos[index], i*scene_width/2, j*scene_height/2, scene_width/2, scene_height/2, QString("rotation = ") + QString::number(rotation_degree));
                        index++;
                        if (index==photos.count())
                            index = 0;
                    }
                }
                prepared=4;
            }
            else{
                qDebug() << "One Picture";
                QExifImageHeader exif(photos[index]);
                QImageReader image;
                int rotation_degree = qrand()%7-3;
                if (rotation_degree < 0)
                    rotation_degree += 360;
                image.setFileName(photos[index]);
                qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                picture_player->createPicture(0, photos[index], 0, 0, scene_width, scene_height, QString("rotation = ") + QString::number(rotation_degree));
                index++;
                if (index==photos.count())
                    index = 0;
                prepared=1;
                last_ratio = (qreal)image.size().height()/image.size().width();
            }

        }
        else{
            if ((qrand()%2)&&(last_prepared==1)){
                qDebug() << "Rotate Picture";
                picture_player->adjustDisplayScene(90, 100*last_ratio, 100*last_ratio, scene_width/2, scene_height/2, "");
                degree = 90;
            }
            else
            {
                qDebug() << "Zoom Picture";
                picture_player->adjustDisplayScene(0, 200, 200, scene_width/2, scene_height/2, "");
                scale = 2;
            }
        }
    }
}

#endif
void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    hide();
    if (degree==90){
        picture_player->adjustDisplayScene(270, 100.0/last_ratio, 100.0/last_ratio, scene_width/2, scene_height/2, "");
        degree=0;
        return;
    }
    if (scale==2){
        picture_player->adjustDisplayScene(0, 50, 50, scene_width/2, scene_height/2, "");
        scale = 0;
        return;
    }
    if (prepared){
        picture_player->displayScene(0, "effect_blend",0);
        last_prepared = prepared;
        prepared = 0;
    }
    picture_player->createScene(0, scene_width, 0, scene_width, scene_height);
    if (qrand()%2){
        QExifImageHeader exif(photos[index]);
        QImageReader image;
        image.setFileName(photos[index]);
        qDebug() << "Single Picture";
        qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
        int rotation_degree = qrand()%7-3;
        if (rotation_degree < 0)
            rotation_degree += 360;
        picture_player->createPicture(0, photos[index], 0, 0, scene_width, scene_height, QString("rotation = ") + QString::number(rotation_degree));
        /******
		** if you want display scence immediately please add the follow code before
		**  display method.
		** qDebug()<<"start display\n";
        while(picture_player->ScenceStatus(0)==0) {
            qDebug()<<"Wait for display\n";
        }
		*/
        picture_player->displayScene(0, "effect_over",0);
        index++;
        if (index==photos.count())
            index = 0;
        prepared=1;
        last_ratio = (qreal)image.size().height()/image.size().width();
    }
    else
    {
        if (qrand()%2){
            int look_ahead_index = index;
            int vertical_flag[4] = {0,0,0,0};
            for (int i=0;i<4;i++){
                QImageReader image;
                image.setFileName(photos[look_ahead_index]);
                if (image.size().height()>image.size().width()){
                    vertical_flag[i]=1;
                }
                look_ahead_index++;
                if (look_ahead_index==photos.count())
                    look_ahead_index = 0;
            }
            if (vertical_flag[0]&&vertical_flag[1]){
                qDebug() << "Two Vertical Picture";
                for (int j=0;j<2;j++){
                    QExifImageHeader exif(photos[index]);
                    QImageReader image;
                    int rotation_degree = qrand()%7-3;
                    if (rotation_degree < 0)
                        rotation_degree += 360;
                    image.setFileName(photos[index]);
                    qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                    picture_player->createPicture(0, photos[index], j*scene_width/2, 0, scene_width/2, scene_height, QString("rotation = ") + QString::number(rotation_degree));
                    index++;
                    if (index==photos.count())
                        index = 0;
                }
                prepared=2;
            }
            else if (vertical_flag[0]+vertical_flag[1]+vertical_flag[2]==1){
                int horizontal_flag = 0;
                qDebug() << "Two horizontal and One Vertical Picture";
                for (int j=0;j<3;j++){
                    QExifImageHeader exif(photos[index]);
                    QImageReader image;
                    int rotation_degree = qrand()%7-3;
                    if (rotation_degree < 0)
                        rotation_degree += 360;
                    image.setFileName(photos[index]);
                    qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                    if (vertical_flag[j]){
                        picture_player->createPicture(0, photos[index], scene_width/2, 0, scene_width/2, scene_height, QString("rotation = ") + QString::number(rotation_degree));
                    }
                    else{
                        if (horizontal_flag){
                            picture_player->createPicture(0, photos[index], 0, scene_height/2, scene_width/2, scene_height/2, QString("rotation = ") + QString::number(rotation_degree));
                        }
                        else{
                            picture_player->createPicture(0, photos[index], 0, 0, scene_width/2, scene_height/2, QString("rotation = ") + QString::number(rotation_degree));
                            horizontal_flag = 1;
                        }
                    }
                    index++;
                    if (index==photos.count())
                        index = 0;
                }
                prepared=3;
            }
            else if (vertical_flag[0]+vertical_flag[1]+vertical_flag[2]+vertical_flag[3]==0){
                qDebug() << "Four Horizontal Picture";
                for (int i=0;i<2;i++){
                    for (int j=0;j<2;j++){
                        QExifImageHeader exif(photos[index]);
                        QImageReader image;
                        int rotation_degree = qrand()%7-3;
                        if (rotation_degree < 0)
                            rotation_degree += 360;
                        image.setFileName(photos[index]);
                        qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                        picture_player->createPicture(0, photos[index], i*scene_width/2, j*scene_height/2, scene_width/2, scene_height/2, QString("rotation = ") + QString::number(rotation_degree));
                        index++;
                        if (index==photos.count())
                            index = 0;
                    }
                }
                prepared=4;
            }
            else{
                qDebug() << "One Picture";
                QExifImageHeader exif(photos[index]);
                QImageReader image;
                int rotation_degree = qrand()%7-3;
                if (rotation_degree < 0)
                    rotation_degree += 360;
                image.setFileName(photos[index]);
                qDebug() << photos[index] << image.size().width() << image.size().height() << exif.value(QExifImageHeader::DateTime).toDateTime().toString();
                picture_player->createPicture(0, photos[index], 0, 0, scene_width, scene_height, QString("rotation = ") + QString::number(rotation_degree));
                index++;
                if (index==photos.count())
                    index = 0;
                prepared=1;
                last_ratio = (qreal)image.size().height()/image.size().width();
            }

        }
        else{
            if ((qrand()%2)&&(last_prepared==1)){
                qDebug() << "Rotate Picture";
                picture_player->adjustDisplayScene(90, 100*last_ratio, 100*last_ratio, scene_width/2, scene_height/2, "");
                degree = 90;
            }
            else
            {
                qDebug() << "Zoom Picture";
                picture_player->adjustDisplayScene(0, 200, 200, scene_width/2, scene_height/2, "");
                scale = 2;
            }
        }
    }
}

void MainWindow::openDirectory()
{
#ifdef PIC_FOLDER_SELECT
    QString path = QFileDialog::getExistingDirectory(this);
#else
    QString path = "/usr/picplay/Picture";
#endif
    if (!path.isEmpty()) {
        QDir dir(path);
        QFileInfoList listElem = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

        foreach (const QString &fileName, dir.entryList(QDir::Files)) {
            QString absolutePath = dir.absoluteFilePath(fileName);
            photos.append(absolutePath);
        }
        index = 0;
    }
}

MainWindow::~MainWindow()
{

}
