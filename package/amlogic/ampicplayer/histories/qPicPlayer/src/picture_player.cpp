#include "picture_player.h"
#include "picture_player_adapter.h"
#include "effect/show_effect.h"

picture_player::picture_player(int screen_width, int screen_height, QGraphicsView *parent)
    : QGraphicsView(parent)
{
	/* get positon parameter. */
	display_area=QRect(0,0,screen_width,screen_height);
	prepare_area=QRect(screen_width,0,screen_width,screen_height);
	
    setGeometry(0,0,screen_width,screen_height);
    for (int i=0; i<MAX_PREPARE_SCENE_NUMBER; i++){
        prepare_scene[i] = NULL;
        prepare_view[i] = NULL;
    }
    display_scene = new(QGraphicsScene);
    if (display_scene){
        display_scene->setSceneRect(0, 0, screen_width, screen_height);
        display_scene->setBackgroundBrush(Qt::black);
        setScene(display_scene);
        scaling_x = 1.0;
        scaling_y = 1.0;
        current_degree = 0;
    }
	
	/* register transition effect. */
	pic_transiion.register_effect(name_effect_over,(unsigned)effect_over);
	pic_transiion.register_effect(name_effect_blend,(unsigned)effect_blend);
	pic_transiion.register_effect(name_effect_wave,(unsigned)effect_wave);
	pic_transiion.register_effect(name_effect_cornercover,(unsigned)effect_cornercover);
	pic_transiion.register_effect(name_effect_move,(unsigned)effect_move);
}

picture_player::~picture_player()
{
    for (int i=0; i<MAX_PREPARE_SCENE_NUMBER; i++){
        if (prepare_scene[i]){
            prepare_scene[i]->clear();
            delete(prepare_scene[i]);
        }
        if (prepare_view[i])
            delete(prepare_view[i]);
    }
    if (display_scene){
        display_scene->clear();
        delete(display_scene);
    }
}

void picture_player::setDesktop(int w, int h)
{
	/* get positon parameter. */
	display_area=QRect(0,0,w,h);
	prepare_area=QRect(w,0,w,h);
	
    setGeometry(0,0,w,h);
    if (display_scene){
        display_scene->setSceneRect(0, 0, w, h);
        display_scene->setBackgroundBrush(Qt::black);
        setScene(display_scene);
    }
}

void picture_player::createPicture(int id, const QString &pictureURL, int x, int y, int w, int h, const QString &parameter)
{
    qDebug() << "createPicture" << id << pictureURL << x << y << w << h;
    if ((id>=MAX_PREPARE_SCENE_NUMBER)||(!prepare_view[id])||(!prepare_scene[id]))
        return;
    int mode =0 ;        
    int rotation = 0;

    if (!parameter.isEmpty()){
        QStringList param_name;
        QStringList param_value;
        QRegExp rx1("(\\s*\\w+\\s*=\\s*\\w+\\s*,?)");
        QRegExp rx2("(\\w+)");
        int pos = 0;
        while (pos>=0){
            pos = rx1.indexIn(parameter, pos);
            if (pos>=0){
                int index = rx2.indexIn(rx1.cap(1), 0);
                param_name.append(rx2.cap(1));
                index += rx2.matchedLength();
                rx2.indexIn(rx1.cap(1), index);
                param_value.append(rx2.cap(1));
                pos += rx1.matchedLength();
            }
        }
        pos  =  param_name.indexOf(QRegExp("mode"));
        if(pos >=0){
            mode = param_value.at(pos).toInt();
        }         
        pos = param_name.indexOf(QRegExp("rotation"));
        if (pos>=0){
            rotation = param_value.at(pos).toInt();           
        }
    }
    QImageReader reader;
    reader.setFileName(pictureURL);
    QSize size = reader.size();
#if QT_VERSION >= 0x040602
    if (size.height()*w > size.width()*h)
        size.scale(QSize(w,h-abs(2*h*sin(rotation*3.1415926/180))), Qt::KeepAspectRatio);
    else
        size.scale(QSize(w-abs(2*w*sin(rotation*3.1415926/180)),h), Qt::KeepAspectRatio);
#else
    switch(mode){
        case 0:
        size.scale(QSize(w,h), Qt::KeepAspectRatio);
        break;
        case 1:
        size.scale(QSize(w,h), Qt::IgnoreAspectRatio);
        break;
        default:
        size.scale(QSize(w,h), Qt::KeepAspectRatio);
        break;    
    }  
#endif
    reader.setScaledSize(size);
    QImage pic = reader.read();
    pic.convertToFormat(QImage::Format_RGB16);
    QPixmap pix = QPixmap::fromImage(pic);
    QGraphicsPixmapItem *image = prepare_scene[id]->addPixmap(pix);
#if QT_VERSION >= 0x040602
    if (rotation){
        image->setTransformOriginPoint(pix.width()/2, pix.height()/2);
        image->setRotation(rotation);
    }
#endif
    image->setPos(QPointF(x+w/2,y+h/2) + prepare_view[id]->sceneRect().topLeft() - QPointF(pix.width()/2.0, pix.height()/2.0));
}

void picture_player::createScene(int id, int x, int y, int w, int h)
{
    qDebug() << "createScene" << id << x << y << w << h;
    if (id>=MAX_PREPARE_SCENE_NUMBER)
        return;
    if (!prepare_view[id]){
        prepare_view[id] = new(QUsrGraphicsView);
        if (prepare_view[id]){
            prepare_view[id]->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            prepare_view[id]->setGeometry(x,y,w,h);
            prepare_view[id]->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            prepare_view[id]->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            prepare_view[id]->setFrameShape(QFrame::NoFrame);
        }
    }
    if (!prepare_scene[id]){
        prepare_scene[id] = new(QGraphicsScene);
        if (prepare_scene[id]){
            prepare_scene[id]->setSceneRect(x,y,w,h);
            prepare_scene[id]->setBackgroundBrush(Qt::black);
            if (prepare_view[id]){
                prepare_view[id]->setScene(prepare_scene[id]);
                prepare_view[id]->show();
            }
        }
    }
    else{
        prepare_scene[id]->clear();
    }
    prepare_view[id]->reset();
}

void picture_player::displayScene(int id, const QString &transitionName, int args)
{
	screen_area_t src_rct;
	screen_area_t dst_rct;
	
	dst_rct.x		=	display_area.x();
	dst_rct.y		=	display_area.y();
	dst_rct.width	=	display_area.width();
	dst_rct.height	=	display_area.height();
	
	src_rct.x		=	prepare_area.x();
	src_rct.y		=	prepare_area.y();
	src_rct.width	=	prepare_area.width();
	src_rct.height	=	prepare_area.height();
	
	//display pictures with transition effect.
	pic_transiion.set_src_dst(&src_rct,&dst_rct);
    pic_transiion.run_effect(transitionName.toLatin1().data(),(unsigned)args);
	
    qDebug() << "displayScene" << id << transitionName;
    if ((id>=MAX_PREPARE_SCENE_NUMBER)||(!prepare_view[id])||(!prepare_scene[id]))
        return;
    // reset the view scale to 1:1
    if ((scaling_x!=1.0)||(scaling_y!=1.0)){
        QRectF unity = this->matrix().mapRect(QRectF(0,0,1,1));
        scale(1/unity.width(),1/unity.height());
        scaling_x = 1.0;
        scaling_y = 1.0;
    }
    // reset the view rotation to 0
    if (current_degree){
        rotate(360 - current_degree);
        current_degree = 0;
    }
    // now copy items in prepare_scene[id] to display_scene, will be changed to use ge2d to copy and do transitions
    display_scene->clear();
    QList <QGraphicsItem *> image_list = prepare_scene[id]->items();
    foreach (QGraphicsItem *image, image_list){
        QGraphicsPixmapItem *pix = (QGraphicsPixmapItem*)image;
        QGraphicsPixmapItem *item = display_scene->addPixmap(pix->pixmap());
#if QT_VERSION >= 0x040602
        if (pix->rotation()){
            item->setTransformOriginPoint(item->pixmap().width()/2, item->pixmap().height()/2);
            item->setRotation(pix->rotation());
        }
#endif
        item->setPos(display_scene->sceneRect().x()+pix->pos().x()-prepare_scene[id]->sceneRect().x(), display_scene->sceneRect().y()+pix->pos().y()-prepare_scene[id]->sceneRect().y());
    }

}

void picture_player::adjustDisplayScene(int degree, int scale_x, int scale_y, int center_x, int center_y, const QString &parameter)
{
    qDebug() << "adjustDisplayScene" << degree << scale_x << scale_y << center_x << center_y;
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    if ((scale_x != 100)||(scale_y != 100)){
        scale((qreal)scale_x/100.0,(qreal)scale_y/100);
        scaling_x *= (qreal)scale_x/100.0;
        scaling_y *= (qreal)scale_y/100.0;
    }
    if (degree){
        rotate(degree);
        current_degree += degree;
        current_degree %= 360;
    }
    centerOn(center_x, center_y);
    update();
}
int picture_player::ScenceStatus(int id){
    prepare_view[id]->status;
}
